/**
 * @file pitch_stabilization_v2.ino
 * @brief Filtered, auto-zeroing pitch stabilization experiment.
 *
 * Hardware status: untested. This revision responds to the last recorded test
 * observations by holding the hip joints, removing integral windup, filtering
 * IMU noise, and calibrating the resting pitch during startup.
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <PID_v2.h>

constexpr uint8_t kPwmAddress = 0x40;
constexpr uint8_t kMpuAddress = 0x68;
constexpr uint8_t kMpuSdaPin = 17;
constexpr uint8_t kMpuSclPin = 16;
constexpr uint16_t kServoFrequencyHz = 60;
constexpr uint32_t kOscillatorFrequencyHz = 25000000;
constexpr uint16_t kServoMinPulseUs = 500;
constexpr uint16_t kServoMaxPulseUs = 2500;
constexpr int kServoAngleRangeDeg = 180;
constexpr uint8_t kServoCount = 12;
constexpr int kCrouchOffsetDeg = 20;
constexpr uint8_t kBaselineSampleCount = 50;
constexpr float kFilterAlpha = 0.15F;

constexpr int8_t kServoOffsets[kServoCount] = {
    -3, 7, -4, -3, -2, -2, 3, -1, 3, 2, 4, 7};
constexpr uint8_t kLeftThighs[] = {1, 4};
constexpr uint8_t kLeftKnees[] = {2, 5};
constexpr uint8_t kRightThighs[] = {7, 10};
constexpr uint8_t kRightKnees[] = {8, 11};
constexpr uint8_t kHips[] = {0, 3, 6, 9};

Adafruit_PWMServoDriver pwm(kPwmAddress);
Adafruit_MPU6050 mpu;

double controllerKp = 0.6;
double controllerKi = 0.0;
double controllerKd = 0.0;
PID_v2 pitchPid(controllerKp, controllerKi, controllerKd, PID::Direct);

float baselinePitchDeg = 0.0F;
float filteredPitchDeg = 0.0F;

float readPitchDeg();
void setServoAngle(uint8_t channel, double targetAngleDeg);
void updateStance(double correctionDeg);

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 3000) {}

  pwm.begin();
  pwm.setOscillatorFrequency(kOscillatorFrequencyHz);
  pwm.setPWMFreq(kServoFrequencyHz);

  Wire1.begin();
  Wire1.setSDA(kMpuSdaPin);
  Wire1.setSCL(kMpuSclPin);
  if (!mpu.begin(kMpuAddress, &Wire1)) {
    Serial.println("Failed to find MPU6050 chip.");
    while (true) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  for (uint8_t hip : kHips) {
    setServoAngle(hip, 90.0);
  }
  updateStance(0.0);

  Serial.println("Robot in stance. Waiting 2 seconds to settle...");
  delay(2000);

  Serial.println("Calibrating level baseline. Do not move the robot.");
  float pitchSum = 0.0F;
  for (uint8_t sample = 0; sample < kBaselineSampleCount; ++sample) {
    pitchSum += readPitchDeg();
    delay(20);
  }
  baselinePitchDeg = pitchSum / kBaselineSampleCount;
  filteredPitchDeg = baselinePitchDeg;

  Serial.print("Calibration complete. Baseline pitch: ");
  Serial.println(baselinePitchDeg);

  pitchPid.SetOutputLimits(-30, 30);
  pitchPid.Start(baselinePitchDeg, 0.0, baselinePitchDeg);
  Serial.println("SpotMicro: pitch stabilization v2 ready");
}

void loop() {
  const float rawPitchDeg = readPitchDeg();
  filteredPitchDeg =
      kFilterAlpha * rawPitchDeg + (1.0F - kFilterAlpha) * filteredPitchDeg;

  const double correctionDeg = pitchPid.Run(filteredPitchDeg);
  updateStance(correctionDeg);

  Serial.print("Raw: ");
  Serial.print(rawPitchDeg);
  Serial.print(" | Filtered: ");
  Serial.print(filteredPitchDeg);
  Serial.print(" | Correction: ");
  Serial.println(correctionDeg);
  delay(20);
}

float readPitchDeg() {
  sensors_event_t acceleration;
  sensors_event_t gyroscope;
  sensors_event_t temperature;
  mpu.getEvent(&acceleration, &gyroscope, &temperature);

  return atan2(
             -acceleration.acceleration.x,
             sqrt(acceleration.acceleration.y * acceleration.acceleration.y +
                  acceleration.acceleration.z * acceleration.acceleration.z)) *
         180.0 / PI;
}

void updateStance(double correctionDeg) {
  const int leftThighBase = 90 + kCrouchOffsetDeg;
  const int leftKneeBase = 90 - kCrouchOffsetDeg;
  const int rightThighBase = 90 - kCrouchOffsetDeg;
  const int rightKneeBase = 90 + kCrouchOffsetDeg;

  setServoAngle(kLeftThighs[1], leftThighBase + correctionDeg);
  setServoAngle(kLeftKnees[1], leftKneeBase - correctionDeg);
  setServoAngle(kLeftThighs[0], leftThighBase - correctionDeg);
  setServoAngle(kLeftKnees[0], leftKneeBase + correctionDeg);

  setServoAngle(kRightThighs[1], rightThighBase - correctionDeg);
  setServoAngle(kRightKnees[1], rightKneeBase + correctionDeg);
  setServoAngle(kRightThighs[0], rightThighBase + correctionDeg);
  setServoAngle(kRightKnees[0], rightKneeBase - correctionDeg);

  for (uint8_t hip : kHips) {
    setServoAngle(hip, 90.0);
  }
}

void setServoAngle(uint8_t channel, double targetAngleDeg) {
  if (channel >= kServoCount) {
    return;
  }
  const int calibratedAngle = constrain(
      static_cast<int>(round(targetAngleDeg)) + kServoOffsets[channel],
      0,
      kServoAngleRangeDeg);
  const long pulseUs = map(
      calibratedAngle,
      0,
      kServoAngleRangeDeg,
      kServoMinPulseUs,
      kServoMaxPulseUs);
  pwm.writeMicroseconds(channel, pulseUs);
}
