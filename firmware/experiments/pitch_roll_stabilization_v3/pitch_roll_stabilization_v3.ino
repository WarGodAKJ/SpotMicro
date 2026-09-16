/**
 * SpotMicro stabilization proof of concept v2
 *
 * New channel order:
 *   0-2   left rear
 *   3-5   right rear
 *   6-8   front left
 *   9-11  front right
 *
 * Joint order within each leg:
 *   hip, thigh, knee
 *
 * MPU orientation:
 *   +X = rear
 *   +Y = right
 *   +Z = up
 *
 * Serial Monitor: 115200 baud
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// =============================================================================
// Hardware
// =============================================================================

constexpr uint8_t kPwmAddress = 0x40;
constexpr uint8_t kMpuAddress = 0x68;

constexpr uint8_t kMpuSdaPin = 17;
constexpr uint8_t kMpuSclPin = 16;

constexpr uint16_t kServoFrequencyHz = 60;
constexpr uint32_t kOscillatorFrequencyHz = 25000000;

constexpr float kServoMinPulseUs = 500.0F;
constexpr float kServoMaxPulseUs = 2500.0F;

constexpr uint8_t kServoCount = 12;

constexpr int8_t kServoOffsets[kServoCount] = {
    -3,  7, -4,  // left rear
     3, -1,  3,  // right rear
    -3, -2, -2,  // front left
     2,  4,  7   // front right
};

// Left rear
constexpr uint8_t kLeftRearHip = 0;
constexpr uint8_t kLeftRearThigh = 1;
constexpr uint8_t kLeftRearKnee = 2;

// Right rear
constexpr uint8_t kRightRearHip = 3;
constexpr uint8_t kRightRearThigh = 4;
constexpr uint8_t kRightRearKnee = 5;

// Front left
constexpr uint8_t kFrontLeftHip = 6;
constexpr uint8_t kFrontLeftThigh = 7;
constexpr uint8_t kFrontLeftKnee = 8;

// Front right
constexpr uint8_t kFrontRightHip = 9;
constexpr uint8_t kFrontRightThigh = 10;
constexpr uint8_t kFrontRightKnee = 11;

constexpr uint8_t kHips[] = {
    kLeftRearHip,
    kRightRearHip,
    kFrontLeftHip,
    kFrontRightHip
};

Adafruit_PWMServoDriver pwm(kPwmAddress);
Adafruit_MPU6050 mpu;

// =============================================================================
// Movement range
// =============================================================================

constexpr float kBaseCompressionDeg = 25.0F;
constexpr float kMaximumLegCorrectionDeg = 20.0F;
constexpr float kLegSlewRateDegPerSec = 60.0F;

constexpr float kRequestedAngleMinDeg = 45.0F;
constexpr float kRequestedAngleMaxDeg = 135.0F;

constexpr float kCalibratedAngleMinDeg = 40.0F;
constexpr float kCalibratedAngleMaxDeg = 140.0F;

constexpr float kEmergencyTiltDeg = 35.0F;

// =============================================================================
// Control settings
// =============================================================================

constexpr bool kEnablePitch = true;
constexpr bool kEnableRoll = true;

constexpr float kPitchOutputSign = 1.0F;
constexpr float kRollOutputSign = 1.0F;

constexpr float kPitchKp = 2.2F;
constexpr float kPitchKd = 0.06F;

constexpr float kRollKp = 1.8F;
constexpr float kRollKd = 0.06F;

constexpr float kMaximumAxisCorrectionDeg = 20.0F;

constexpr float kAngleDeadbandDeg = 0.40F;
constexpr float kRateDeadbandDegPerSec = 1.20F;

// =============================================================================
// Filtering
// =============================================================================

constexpr uint32_t kControlPeriodUs = 20000;
constexpr uint32_t kTelemetryPeriodUs = 100000;

constexpr uint16_t kCalibrationSamples = 250;
constexpr uint16_t kCalibrationDelayMs = 4;

constexpr float kAccelerometerFilterAlpha = 0.12F;
constexpr float kComplementaryAccelWeight = 0.035F;
constexpr float kGyroRateFilterAlpha = 0.10F;
constexpr float kControlErrorFilterAlpha = 0.12F;

constexpr float kGravity = 9.80665F;

// =============================================================================
// State
// =============================================================================

enum LegIndex : uint8_t {
  LEFT_REAR = 0,
  RIGHT_REAR = 1,
  FRONT_LEFT = 2,
  FRONT_RIGHT = 3
};

struct ImuReading {
  float accelPitchDeg;
  float accelRollDeg;

  float pitchRateDegPerSec;
  float rollRateDegPerSec;

  float accelerationMagnitude;
};

float targetPitchDeg = 0.0F;
float targetRollDeg = 0.0F;

float gyroPitchBiasDegPerSec = 0.0F;
float gyroRollBiasDegPerSec = 0.0F;

float filteredAccelPitchDeg = 0.0F;
float filteredAccelRollDeg = 0.0F;

float fusedPitchDeg = 0.0F;
float fusedRollDeg = 0.0F;

float filteredPitchRateDegPerSec = 0.0F;
float filteredRollRateDegPerSec = 0.0F;

float filteredPitchErrorDeg = 0.0F;
float filteredRollErrorDeg = 0.0F;

float appliedLegCorrectionDeg[4] = {
    0.0F,
    0.0F,
    0.0F,
    0.0F
};

uint32_t lastControlUs = 0;
uint32_t nextControlUs = 0;
uint32_t lastTelemetryUs = 0;

// =============================================================================
// Helpers
// =============================================================================

float clampFloat(float value, float minimum, float maximum) {
  if (value < minimum) {
    return minimum;
  }

  if (value > maximum) {
    return maximum;
  }

  return value;
}

float mapFloat(
    float value,
    float inputMinimum,
    float inputMaximum,
    float outputMinimum,
    float outputMaximum
) {
  const float ratio =
      (value - inputMinimum) /
      (inputMaximum - inputMinimum);

  return outputMinimum +
      ratio * (outputMaximum - outputMinimum);
}

// =============================================================================
// MPU
// =============================================================================

ImuReading readImu() {
  sensors_event_t acceleration;
  sensors_event_t gyroscope;
  sensors_event_t temperature;

  mpu.getEvent(
      &acceleration,
      &gyroscope,
      &temperature
  );

  const float ax = acceleration.acceleration.x;
  const float ay = acceleration.acceleration.y;
  const float az = acceleration.acceleration.z;

  ImuReading result;

  // +X points rearward. Positive pitch means rear raised.
  result.accelPitchDeg =
      atan2f(
          ax,
          sqrtf(ay * ay + az * az)
      ) * RAD_TO_DEG;

  // +Y points right. Positive roll means right side raised.
  result.accelRollDeg =
      atan2f(ay, az) * RAD_TO_DEG;

  result.pitchRateDegPerSec =
      -gyroscope.gyro.y * RAD_TO_DEG;

  result.rollRateDegPerSec =
      gyroscope.gyro.x * RAD_TO_DEG;

  result.accelerationMagnitude =
      sqrtf(ax * ax + ay * ay + az * az);

  return result;
}

void calibrateImu() {
  Serial.println("Calibrating. Keep the table completely still.");

  float pitchSum = 0.0F;
  float rollSum = 0.0F;
  float pitchRateSum = 0.0F;
  float rollRateSum = 0.0F;

  for (uint16_t sample = 0;
       sample < kCalibrationSamples;
       ++sample) {
    const ImuReading reading = readImu();

    pitchSum += reading.accelPitchDeg;
    rollSum += reading.accelRollDeg;

    pitchRateSum += reading.pitchRateDegPerSec;
    rollRateSum += reading.rollRateDegPerSec;

    delay(kCalibrationDelayMs);
  }

  const float count =
      static_cast<float>(kCalibrationSamples);

  targetPitchDeg = pitchSum / count;
  targetRollDeg = rollSum / count;

  gyroPitchBiasDegPerSec = pitchRateSum / count;
  gyroRollBiasDegPerSec = rollRateSum / count;

  filteredAccelPitchDeg = targetPitchDeg;
  filteredAccelRollDeg = targetRollDeg;

  fusedPitchDeg = targetPitchDeg;
  fusedRollDeg = targetRollDeg;

  filteredPitchRateDegPerSec = 0.0F;
  filteredRollRateDegPerSec = 0.0F;

  filteredPitchErrorDeg = 0.0F;
  filteredRollErrorDeg = 0.0F;

  Serial.print("PitchTarget:");
  Serial.print(targetPitchDeg, 3);

  Serial.print(" RollTarget:");
  Serial.println(targetRollDeg, 3);
}

// =============================================================================
// Servos
// =============================================================================

void setServoAngle(
    uint8_t channel,
    float requestedAngleDeg
) {
  if (channel >= kServoCount) {
    return;
  }

  requestedAngleDeg = clampFloat(
      requestedAngleDeg,
      kRequestedAngleMinDeg,
      kRequestedAngleMaxDeg
  );

  const float calibratedAngleDeg = clampFloat(
      requestedAngleDeg +
          static_cast<float>(kServoOffsets[channel]),
      kCalibratedAngleMinDeg,
      kCalibratedAngleMaxDeg
  );

  const float pulseUs = mapFloat(
      calibratedAngleDeg,
      0.0F,
      180.0F,
      kServoMinPulseUs,
      kServoMaxPulseUs
  );

  pwm.writeMicroseconds(
      channel,
      static_cast<uint16_t>(lroundf(pulseUs))
  );
}

void holdHips() {
  for (uint8_t hip : kHips) {
    setServoAngle(hip, 90.0F);
  }
}

void setLegCompression(
    uint8_t thigh,
    uint8_t knee,
    bool rightSide,
    float compressionDeg
) {
  compressionDeg = clampFloat(
      compressionDeg,
      kBaseCompressionDeg -
          kMaximumLegCorrectionDeg,
      kBaseCompressionDeg +
          kMaximumLegCorrectionDeg
  );

  if (rightSide) {
    setServoAngle(thigh, 90.0F - compressionDeg);
    setServoAngle(knee, 90.0F + compressionDeg);
  } else {
    setServoAngle(thigh, 90.0F + compressionDeg);
    setServoAngle(knee, 90.0F - compressionDeg);
  }
}

void applyStance() {
  setLegCompression(
      kLeftRearThigh,
      kLeftRearKnee,
      false,
      kBaseCompressionDeg +
          appliedLegCorrectionDeg[LEFT_REAR]
  );

  setLegCompression(
      kRightRearThigh,
      kRightRearKnee,
      true,
      kBaseCompressionDeg +
          appliedLegCorrectionDeg[RIGHT_REAR]
  );

  setLegCompression(
      kFrontLeftThigh,
      kFrontLeftKnee,
      false,
      kBaseCompressionDeg +
          appliedLegCorrectionDeg[FRONT_LEFT]
  );

  setLegCompression(
      kFrontRightThigh,
      kFrontRightKnee,
      true,
      kBaseCompressionDeg +
          appliedLegCorrectionDeg[FRONT_RIGHT]
  );

  holdHips();
}

void enterCrouch() {
  for (uint8_t channel = 0;
       channel < kServoCount;
       ++channel) {
    setServoAngle(channel, 90.0F);
    delay(120);
  }

  for (float compression = 0.0F;
       compression <= kBaseCompressionDeg;
       compression += 0.5F) {
    setLegCompression(
        kLeftRearThigh,
        kLeftRearKnee,
        false,
        compression
    );

    setLegCompression(
        kRightRearThigh,
        kRightRearKnee,
        true,
        compression
    );

    setLegCompression(
        kFrontLeftThigh,
        kFrontLeftKnee,
        false,
        compression
    );

    setLegCompression(
        kFrontRightThigh,
        kFrontRightKnee,
        true,
        compression
    );

    holdHips();
    delay(20);
  }

  applyStance();
}

// =============================================================================
// Adaptive vibration rejection
// =============================================================================

float calculateAccelerometerTrust(
    const ImuReading &imu,
    float gyroPredictedPitchDeg,
    float gyroPredictedRollDeg
) {
  const float gravityDeviation =
      fabsf(imu.accelerationMagnitude - kGravity);

  const float magnitudeTrust =
      1.0F -
      clampFloat(
          (gravityDeviation - 0.30F) / 1.70F,
          0.0F,
          1.0F
      );

  const float pitchDisagreement =
      fabsf(
          filteredAccelPitchDeg -
          gyroPredictedPitchDeg
      );

  const float rollDisagreement =
      fabsf(
          filteredAccelRollDeg -
          gyroPredictedRollDeg
      );

  const float largestDisagreement =
      max(pitchDisagreement, rollDisagreement);

  const float agreementTrust =
      1.0F -
      clampFloat(
          (largestDisagreement - 2.0F) / 8.0F,
          0.0F,
          1.0F
      );

  return magnitudeTrust * agreementTrust;
}

// =============================================================================
// Control
// =============================================================================

float calculateAxisOutput(
    float errorDeg,
    float rateDegPerSec,
    float kp,
    float kd,
    float outputSign
) {
  if (fabsf(errorDeg) < kAngleDeadbandDeg) {
    errorDeg = 0.0F;
  }

  if (fabsf(rateDegPerSec) <
      kRateDeadbandDegPerSec) {
    rateDegPerSec = 0.0F;
  }

  const float output =
      outputSign *
      (
          kp * errorDeg +
          kd * rateDegPerSec
      );

  return clampFloat(
      output,
      -kMaximumAxisCorrectionDeg,
      kMaximumAxisCorrectionDeg
  );
}

void mixCorrections(
    float pitchCorrectionDeg,
    float rollCorrectionDeg,
    float desiredLegCorrectionDeg[4]
) {
  desiredLegCorrectionDeg[LEFT_REAR] =
      pitchCorrectionDeg - rollCorrectionDeg;

  desiredLegCorrectionDeg[RIGHT_REAR] =
      pitchCorrectionDeg + rollCorrectionDeg;

  desiredLegCorrectionDeg[FRONT_LEFT] =
      -pitchCorrectionDeg - rollCorrectionDeg;

  desiredLegCorrectionDeg[FRONT_RIGHT] =
      -pitchCorrectionDeg + rollCorrectionDeg;

  float largestMagnitude = 0.0F;

  for (uint8_t leg = 0; leg < 4; ++leg) {
    largestMagnitude = max(
        largestMagnitude,
        fabsf(desiredLegCorrectionDeg[leg])
    );
  }

  if (largestMagnitude >
      kMaximumLegCorrectionDeg) {
    const float scale =
        kMaximumLegCorrectionDeg /
        largestMagnitude;

    for (uint8_t leg = 0; leg < 4; ++leg) {
      desiredLegCorrectionDeg[leg] *= scale;
    }
  }
}

void slewLegCorrections(
    const float desiredLegCorrectionDeg[4],
    float dtSec
) {
  const float maximumChange =
      kLegSlewRateDegPerSec * dtSec;

  for (uint8_t leg = 0; leg < 4; ++leg) {
    const float difference =
        desiredLegCorrectionDeg[leg] -
        appliedLegCorrectionDeg[leg];

    appliedLegCorrectionDeg[leg] +=
        clampFloat(
            difference,
            -maximumChange,
            maximumChange
        );
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  pwm.begin();
  pwm.setOscillatorFrequency(
      kOscillatorFrequencyHz
  );
  pwm.setPWMFreq(kServoFrequencyHz);

  Wire1.setSDA(kMpuSdaPin);
  Wire1.setSCL(kMpuSclPin);
  Wire1.begin();
  Wire1.setClock(400000);

  if (!mpu.begin(kMpuAddress, &Wire1)) {
    Serial.println("MPU6050 not found.");

    while (true) {
      delay(1000);
    }
  }

  mpu.setAccelerometerRange(
      MPU6050_RANGE_8_G
  );

  mpu.setGyroRange(
      MPU6050_RANGE_500_DEG
  );

  mpu.setFilterBandwidth(
      MPU6050_BAND_21_HZ
  );

  Serial.println("Movement begins in five seconds.");
  delay(5000);

  enterCrouch();

  Serial.println("Waiting for movement to settle...");
  delay(2000);

  calibrateImu();

  lastControlUs = micros();
  nextControlUs =
      lastControlUs + kControlPeriodUs;
  lastTelemetryUs = lastControlUs;

  Serial.println("Stabilization active.");
}

// =============================================================================
// Main loop
// =============================================================================

void loop() {
  const uint32_t nowUs = micros();

  if (static_cast<int32_t>(
          nowUs - nextControlUs
      ) < 0) {
    return;
  }

  float dtSec =
      static_cast<float>(
          nowUs - lastControlUs
      ) * 0.000001F;

  lastControlUs = nowUs;
  nextControlUs += kControlPeriodUs;

  if (static_cast<int32_t>(
          nowUs - nextControlUs
      ) > static_cast<int32_t>(
          2 * kControlPeriodUs
      )) {
    nextControlUs =
        nowUs + kControlPeriodUs;
  }

  if (dtSec <= 0.0F || dtSec > 0.10F) {
    dtSec = 0.020F;
  }

  const ImuReading imu = readImu();

  const float pitchRate =
      imu.pitchRateDegPerSec -
      gyroPitchBiasDegPerSec;

  const float rollRate =
      imu.rollRateDegPerSec -
      gyroRollBiasDegPerSec;

  filteredAccelPitchDeg +=
      kAccelerometerFilterAlpha *
      (
          imu.accelPitchDeg -
          filteredAccelPitchDeg
      );

  filteredAccelRollDeg +=
      kAccelerometerFilterAlpha *
      (
          imu.accelRollDeg -
          filteredAccelRollDeg
      );

  filteredPitchRateDegPerSec +=
      kGyroRateFilterAlpha *
      (
          pitchRate -
          filteredPitchRateDegPerSec
      );

  filteredRollRateDegPerSec +=
      kGyroRateFilterAlpha *
      (
          rollRate -
          filteredRollRateDegPerSec
      );

  const float predictedPitchDeg =
      fusedPitchDeg + pitchRate * dtSec;

  const float predictedRollDeg =
      fusedRollDeg + rollRate * dtSec;

  const float accelerometerTrust =
      calculateAccelerometerTrust(
          imu,
          predictedPitchDeg,
          predictedRollDeg
      );

  const float adaptiveAccelWeight =
      kComplementaryAccelWeight *
      accelerometerTrust;

  fusedPitchDeg =
      (1.0F - adaptiveAccelWeight) *
          predictedPitchDeg +
      adaptiveAccelWeight *
          filteredAccelPitchDeg;

  fusedRollDeg =
      (1.0F - adaptiveAccelWeight) *
          predictedRollDeg +
      adaptiveAccelWeight *
          filteredAccelRollDeg;

  const float rawPitchErrorDeg =
      fusedPitchDeg - targetPitchDeg;

  const float rawRollErrorDeg =
      fusedRollDeg - targetRollDeg;

  filteredPitchErrorDeg +=
      kControlErrorFilterAlpha *
      (
          rawPitchErrorDeg -
          filteredPitchErrorDeg
      );

  filteredRollErrorDeg +=
      kControlErrorFilterAlpha *
      (
          rawRollErrorDeg -
          filteredRollErrorDeg
      );

  float pitchCorrectionDeg = 0.0F;
  float rollCorrectionDeg = 0.0F;

  if (kEnablePitch) {
    pitchCorrectionDeg =
        calculateAxisOutput(
            filteredPitchErrorDeg,
            filteredPitchRateDegPerSec,
            kPitchKp,
            kPitchKd,
            kPitchOutputSign
        );
  }

  if (kEnableRoll) {
    rollCorrectionDeg =
        calculateAxisOutput(
            filteredRollErrorDeg,
            filteredRollRateDegPerSec,
            kRollKp,
            kRollKd,
            kRollOutputSign
        );
  }

  float desiredLegCorrectionDeg[4];

  const bool emergencyTilt =
      fabsf(rawPitchErrorDeg) >
          kEmergencyTiltDeg ||
      fabsf(rawRollErrorDeg) >
          kEmergencyTiltDeg;

  if (emergencyTilt) {
    for (uint8_t leg = 0; leg < 4; ++leg) {
      desiredLegCorrectionDeg[leg] = 0.0F;
    }
  } else {
    mixCorrections(
        pitchCorrectionDeg,
        rollCorrectionDeg,
        desiredLegCorrectionDeg
    );
  }

  slewLegCorrections(
      desiredLegCorrectionDeg,
      dtSec
  );

  applyStance();

  if (nowUs - lastTelemetryUs >=
      kTelemetryPeriodUs) {
    lastTelemetryUs = nowUs;

    Serial.print("Pitch:");
    Serial.print(fusedPitchDeg, 2);

    Serial.print(" PitchError:");
    Serial.print(filteredPitchErrorDeg, 2);

    Serial.print(" Roll:");
    Serial.print(fusedRollDeg, 2);

    Serial.print(" RollError:");
    Serial.print(filteredRollErrorDeg, 2);

    Serial.print(" Trust:");
    Serial.print(accelerometerTrust, 2);

    Serial.print(" LR:");
    Serial.print(
        appliedLegCorrectionDeg[LEFT_REAR],
        2
    );

    Serial.print(" RR:");
    Serial.print(
        appliedLegCorrectionDeg[RIGHT_REAR],
        2
    );

    Serial.print(" FL:");
    Serial.print(
        appliedLegCorrectionDeg[FRONT_LEFT],
        2
    );

    Serial.print(" FR:");
    Serial.println(
        appliedLegCorrectionDeg[FRONT_RIGHT],
        2
    );
  }
}
