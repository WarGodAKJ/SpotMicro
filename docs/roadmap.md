# Development roadmap

## Immediate

- Hardware-test pitch stabilization v2 in both directions.
- Record repeatable acceptance criteria and controller telemetry.
- Add explicit output-enable or emergency-stop handling if supported by the final PCB wiring.
- Verify the complete power schematic and document Teensy VIN/VUSB isolation.

## Near term

- Add correction deadband, slew-rate limiting, and joint-specific safe angle limits.
- Add left/right roll stabilization after pitch behavior is repeatable.
- Replace accelerometer-only tilt with fused accelerometer/gyroscope orientation.
- Separate hardware configuration from control algorithms in a reusable library.
- Add hardware-in-the-loop smoke tests for servo order and motion direction.

## Long term

- Implement a stable standing state machine and fault states.
- Add inverse kinematics and controlled gait generation.
- Integrate the Nano-side sensors and display only after the motion controller is stable.
- Add battery, current, and thermal monitoring with automatic motion shutdown.
- Document a reproducible mechanical build and wiring revision.

Features move from experimental to tested only after the procedure in [testing.md](testing.md) is completed and the observed result is recorded.
