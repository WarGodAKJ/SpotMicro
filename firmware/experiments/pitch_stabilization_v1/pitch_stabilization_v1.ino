/**
 * @file pitch_stabilization_v1.ino
 * @brief First hardware-tested pitch stabilization experiment.
 *
 * Known limitations: the integral term can delay recovery after a sustained
 * tilt, the hip joints are not actively held, and the resting IMU bias is not
 * calibrated. Retained to document the tested development milestone.
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

constexpr int8_t kServoOffsets[kServoCount] = {
    -3, 7, -4, -3, -2, -2, 3, -1, 3, 2, 4, 7};
constexpr uint8_t kLeftThighs[] = {1, 4};
constexpr uint8_t kLeftKnees[] = {2, 5};
constexpr uint8_t kRightThighs[] = {7, 10};
constexpr uint8_t kRightKnees[] = {8, 11};

Adafruit_PWMServoDriver pwm(kPwmAddress);
Adafruit_MPU6050 mpu;

double controllerKp = 0.5;
double controllerKi = 0.05;
double controllerKd = 0.02;
PID_v2 pitchPid(controllerKp, controllerKi, controllerKd, PID::Direct);

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

  pitchPid.SetOutputLimits(-30, 30);
  pitchPid.Start(0.0, 0.0, 0.0);

  delay(1000);
  Serial.println("SpotMicro: pitch stabilization v1 ready");
  updateStance(0.0);
  delay(2000);
}

void loop() {
  const float pitchDeg = readPitchDeg();
  const double correctionDeg = pitchPid.Run(pitchDeg);
  updateStance(correctionDeg);

  Serial.print("Pitch: ");
  Serial.print(pitchDeg);
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
