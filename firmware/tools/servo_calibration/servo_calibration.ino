/**
 * @file servo_calibration.ino
 * @brief Serial calibration utility for the 12-servo SpotMicro layout.
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

Adafruit_PWMServoDriver pwm(kPwmAddress);

// Calibration intentionally begins at zero. Copy the reported values into the
// project configuration only after physically centering every joint.
int servoOffsets[kServoCount] = {};

void printHelp();
void printOffsets();
void resetOffsets();
void updateServo(uint8_t channel);

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 3000);

  pwm.begin();
  pwm.setOscillatorFrequency(kOscillatorFrequencyHz);
  pwm.setPWMFreq(kServoFrequencyHz);

  printHelp();

  for (uint8_t channel = 0; channel < kServoCount; ++channel) {
    updateServo(channel);
  }
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.equalsIgnoreCase("h")) {
    printHelp();
    return;
  }
  if (input.equalsIgnoreCase("p")) {
    printOffsets();
    return;
  }
  if (input.equalsIgnoreCase("r")) {
    resetOffsets();
    return;
  }
  if (input.length() < 2) {
    Serial.println("Invalid command. Type 'h' for help.");
    return;
  }

  const char action = input.charAt(input.length() - 1);
  const int channel = input.substring(0, input.length() - 1).toInt();
  if (channel < 0 || channel >= kServoCount || (action != '+' && action != '-')) {
    Serial.println("Invalid command. Type 'h' for help.");
    return;
  }

  servoOffsets[channel] += action == '+' ? 1 : -1;
  servoOffsets[channel] = constrain(servoOffsets[channel], -90, 90);
  updateServo(static_cast<uint8_t>(channel));
  printOffsets();
}

void updateServo(uint8_t channel) {
  const int calibratedAngle = constrain(90 + servoOffsets[channel], 0, 180);
  const long pulseUs = map(
      calibratedAngle,
      0,
      kServoAngleRangeDeg,
      kServoMinPulseUs,
      kServoMaxPulseUs);
  pwm.writeMicroseconds(channel, pulseUs);
}

void printOffsets() {
  Serial.println("\nCurrent offsets (copy these values):");
  Serial.print("int servoOffsets[] = {");
  for (uint8_t channel = 0; channel < kServoCount; ++channel) {
    Serial.print(servoOffsets[channel]);
    if (channel + 1 < kServoCount) {
      Serial.print(", ");
    }
  }
  Serial.println("};");
}

void resetOffsets() {
  for (uint8_t channel = 0; channel < kServoCount; ++channel) {
    servoOffsets[channel] = 0;
    updateServo(channel);
  }
  Serial.println("All offsets reset to zero.");
  printOffsets();
}

void printHelp() {
  Serial.println("\n--- SPOTMICRO SERVO CALIBRATION ---");
  Serial.println("<channel>+  increase a channel offset by 1 degree");
  Serial.println("<channel>-  decrease a channel offset by 1 degree");
  Serial.println("p           print all offsets");
  Serial.println("r           reset all offsets to zero");
  Serial.println("h           show this help");
  Serial.println("Examples: 0+, 5-, 11+");
  Serial.println("-----------------------------------");
}
