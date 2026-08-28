/**
 * @file push_ups.ino
 * @brief Performs five synchronized push-ups after a five-second delay.
 *
 * Hardware status: tested on the project robot.
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

constexpr uint8_t kPwmAddress = 0x40;
constexpr uint16_t kServoFrequencyHz = 60;
constexpr uint32_t kOscillatorFrequencyHz = 25000000;
constexpr uint16_t kServoMinPulseUs = 500;
constexpr uint16_t kServoMaxPulseUs = 2500;
constexpr int kServoAngleRangeDeg = 180;
constexpr uint8_t kServoCount = 12;
constexpr uint8_t kPushUpCount = 5;
constexpr int kPushUpTravelDeg = 30;

constexpr int8_t kServoOffsets[kServoCount] = {
    -3, 7, -4, -3, -2, -2, 3, -1, 3, 2, 4, 7};
constexpr uint8_t kLeftThighs[] = {1, 4};
constexpr uint8_t kLeftKnees[] = {2, 5};
constexpr uint8_t kRightThighs[] = {7, 10};
constexpr uint8_t kRightKnees[] = {8, 11};

Adafruit_PWMServoDriver pwm(kPwmAddress);

void setServoAngle(uint8_t channel, int targetAngleDeg);

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 3000) {}

  pwm.begin();
  pwm.setOscillatorFrequency(kOscillatorFrequencyHz);
  pwm.setPWMFreq(kServoFrequencyHz);

  delay(1000);
  Serial.println("SpotMicro: Calibrated Synchronous Push-Ups");

  Serial.println("Moving to Home position...");
  for (uint8_t channel = 0; channel < kServoCount; ++channel) {
    setServoAngle(channel, 90);
  }

  Serial.println("Holding Home for 5 seconds...");
  delay(5000);

  Serial.println("Starting Push-ups...");
  for (uint8_t repetition = 1; repetition <= kPushUpCount; ++repetition) {
    Serial.print("Push-up #"); Serial.println(repetition);

    // --- GOING DOWN ---
    for (int offset = 0; offset <= kPushUpTravelDeg; ++offset) {
      for (uint8_t leg = 0; leg < 2; ++leg) {
        setServoAngle(kLeftThighs[leg], 90 + offset);
        setServoAngle(kLeftKnees[leg], 90 - offset);
        setServoAngle(kRightThighs[leg], 90 - offset);
        setServoAngle(kRightKnees[leg], 90 + offset);
      }
      delay(20);
    }

    delay(500);

    for (int offset = kPushUpTravelDeg; offset >= 0; --offset) {
      for (uint8_t leg = 0; leg < 2; ++leg) {
        setServoAngle(kLeftThighs[leg], 90 + offset);
        setServoAngle(kLeftKnees[leg], 90 - offset);
        setServoAngle(kRightThighs[leg], 90 - offset);
        setServoAngle(kRightKnees[leg], 90 + offset);
      }
      delay(15);
    }

    delay(1000);
  }

  Serial.println("Push-ups complete. Holding stance.");
}

void loop() {}

void setServoAngle(uint8_t channel, int targetAngleDeg) {
  if (channel >= kServoCount) {
    return;
  }
  const int calibratedAngle = constrain(
      targetAngleDeg + kServoOffsets[channel], 0, kServoAngleRangeDeg);
  const long pulseUs = map(
      calibratedAngle,
      0,
      kServoAngleRangeDeg,
      kServoMinPulseUs,
      kServoMaxPulseUs);
  pwm.writeMicroseconds(channel, pulseUs);
}
