/**
 * @file imu_readout.ino
 * @brief Prints MPU6050 pitch and roll values over USB serial.
 *
 * Hardware status: tested on the project robot.
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

constexpr uint8_t kMpuAddress = 0x68;
constexpr uint8_t kMpuSdaPin = 17;
constexpr uint8_t kMpuSclPin = 16;
constexpr uint32_t kSerialBaud = 19200;
constexpr uint16_t kSamplePeriodMs = 100;

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(kSerialBaud);
  while (!Serial && millis() < 3000);

  Serial.println("Initializing MPU6050...");

  Wire1.begin();
  Wire1.setSDA(kMpuSdaPin);
  Wire1.setSCL(kMpuSclPin);

  if (!mpu.begin(kMpuAddress, &Wire1)) {
    Serial.println("Failed to find MPU6050 chip. Check wiring!");
    while (1) {
      delay(10); // Halt if not found
    }
  }
  Serial.println("MPU6050 Found and Ready!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  const float roll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  const float pitch = atan2(
      -a.acceleration.x,
      sqrt(a.acceleration.y * a.acceleration.y +
           a.acceleration.z * a.acceleration.z)) *
      180.0 / PI;

  // Print out the values
  Serial.print("Pitch (Front/Back tilt): ");
  Serial.print(pitch);
  Serial.print("   |   Roll (Left/Right tilt): ");
  Serial.println(roll);

  delay(kSamplePeriodMs);
}
