# Development roadmap

## Implemented in the reported v3 proof of concept

- Pitch and roll stabilization, including diagonal mixing and return-to-level.
- Accelerometer/gyroscope complementary orientation filtering and gyro bias calibration.
- Startup attitude zeroing, filtered rates/errors, and deadbands.
- Uniform corner-output scaling and servo slew-rate limiting.
- Requested and calibrated joint-angle limits.
- Adaptive accelerometer trust from gravity magnitude and angle disagreement.

These are implemented features with user-reported proof-of-concept results.
They do not establish performance under all disturbances.

## Immediate work

- Retest the remapped repository push-up demo and revised IMU diagnostic.
- Record quantified angles, timing, repetition counts, and controller telemetry.
- Detect sustained high-frequency vibration and reduce gains or hold a stable crouch.
- Add servo acceleration limiting and a latched fall/fault state machine.
- Handle sensor-read failures, invalid samples, and calibration while moving.
- Validate excessive-tilt behavior and physical power-off / output-enable provisions.

## Longer term

- Inverse kinematics and a body-position controller.
- Gait generation and controlled locomotion.
- Battery/current monitoring and appropriate shutdown behavior.
- Nano-side sensors, display, and other peripherals.
- Reproducible mechanical build and verified wiring revision.

Record physical evidence using [testing.md](testing.md) before upgrading test status.
