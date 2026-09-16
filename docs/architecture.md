# System architecture

## Active controller: pitch/roll stabilization v3

The Teensy 4.0 reads the MPU6050 and controls twelve PCA9685-connected servos.
The user reports successful pitch, roll, diagonal, and return-to-level tests.
This is an automatic-leveling proof of concept; gait and body-position control remain future work.

```text
MPU accelerometer + gyroscope
        |
adaptive complementary filter (gravity magnitude + angle agreement)
        |
pitch and roll error filters
        |
two PD controllers
        |
four-corner correction mixer
        |
uniform scaling + leg slew limiting
        |
mirrored thigh/knee commands + calibrated hip hold
```

## Estimation and timing

Startup waits five seconds before motion, centers channels sequentially, enters a
25-degree crouch, settles for two seconds, then averages 250 samples at 4 ms
intervals. This establishes pitch/roll targets and gyro biases, including the
approximately 5-degree mounting roll offset.

The nominal controller period is 20 ms (50 Hz); telemetry is scheduled every
100 ms (10 Hz). Measured dt drives gyro integration and slew limiting, with a
20 ms fallback for invalid or greater-than-100 ms intervals. Telemetry is decimated,
but Serial writes are not guaranteed nonblocking on every host connection.

Pitch = atan2(ax, sqrt(ay² + az²)), roll = atan2(ay, az).
Pitch rate = -gyroY and roll rate = gyroX, converted to degrees per second.
Accelerometer angle alpha is 0.12, gyro-rate alpha 0.10, error alpha 0.12.
Complementary accelerometer weight is 0.035 multiplied by trust.

Trust is the product of gravity-magnitude and angle-agreement factors. Magnitude
trust falls from one to zero as gravity deviation increases from 0.30 to 2.00 m/s²;
agreement trust falls from one to zero between 2 and 10 degrees of disagreement.
Low trust reduces accelerometer influence; it does not detect a fall or guarantee stability.

## Control geometry

Pitch gains: Kp 2.2, Kd 0.06. Roll gains: Kp 1.8, Kd 0.06.
The actual output is Kp * attitude error + Kd * angular rate, with configured sign
+1 on both axes. Deadbands are 0.40 degrees and 1.20 degrees/second.
Axis outputs are limited to +/-20 degrees.

| Leg | Correction |
| --- | --- |
| Rear left | +pitch - roll |
| Rear right | +pitch + roll |
| Front left | -pitch - roll |
| Front right | -pitch + roll |

If any corner exceeds +/-20 degrees, all four are scaled together to preserve
the diagonal ratios. Leg corrections slew at at most 60 degrees/second.
Base compression 25 degrees plus correction gives 5-45 degrees of compression.
Left thigh/knee commands are 90+c / 90-c; right commands are mirrored.
Hips hold nominal 90 degrees plus calibration.

Requested angles are limited to 45-135 degrees; after offsets, commands are
limited again to 40-140 degrees. Pulse mapping retains fractional angles until
rounding microseconds for the driver.

## Excessive tilt and remaining limits

When either fused angle differs from its startup target by more than 35 degrees,
desired corrections become zero and slew toward the base crouch. Servos remain
powered. This is not a latched emergency stop: correction resumes when the estimate
returns inside the threshold.

The supplied startup ramp requests 0-25 degrees, but its shared compression helper
clamps values below 5 degrees to 5. This detail is retained with the tested code.
Runtime sensor-read failures and non-finite samples have no explicit fault state.

Increasingly rapid support-surface shaking eventually caused overreaction and a
tip-over. Translational acceleration can resemble gravity tilt and servos cannot
track arbitrarily fast motion. A vibration mode, acceleration limiting, and fall
state machine remain pending. No maximum safe disturbance frequency was measured.

## Historical controllers

V1/v2 retain their exact source and old mapping; see [firmware history](../firmware/experiments/README.md).
The original Nova source remains a separate upstream reference. Nano peripherals,
radio, displays, and autonomous gait are outside v3's implemented scope.
