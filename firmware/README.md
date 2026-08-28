# Firmware guide

Every directory containing an `.ino` file is a standalone Arduino sketch. Open the sketch file directly in Arduino IDE; its parent directory deliberately has the same name as the sketch.

## Sketches

| Sketch | Purpose | Hardware status |
| --- | --- | --- |
| `tools/servo_calibration` | Center one servo at a time and record offsets | Tested |
| `demos/push_ups` | Perform five synchronized push-ups | Tested |
| `diagnostics/imu_readout` | Print pitch and roll from the MPU6050 | Tested |
| `experiments/pitch_stabilization_v1` | Preserve the first successful pitch controller | Tested with known limitations |
| `experiments/pitch_stabilization_v2` | Auto-zero and filter pitch while holding all hips | Untested |
| `reference/nova_sm3_teensy_v4_2` | Preserve the original Nova SM3 v4.2 firmware | Upstream reference only |

## Arduino dependencies

Install these through Arduino IDE's Library Manager unless noted otherwise:

- Adafruit PWM Servo Driver Library
- Adafruit MPU6050
- Adafruit Unified Sensor
- PID_v2 for the stabilization experiments
- Teensy board support for Teensy 4.0

The upstream reference firmware has additional dependencies and is intentionally not the recommended starting point for this robot.

## Upload settings

- Board: Teensy 4.0
- USB type: Serial
- Serial monitor: `19200` baud
- PCA9685 PWM frequency: `60 Hz`
- PCA9685 oscillator setting: `25 MHz`

## Configuration ownership

The human-readable source of truth is [the hardware guide](../docs/hardware.md), and the machine-readable source is [robot.json](../config/robot.json). Each standalone sketch embeds the required constants so it remains easy to open in Arduino IDE. The repository check prevents the final offsets and channel map from silently drifting apart.

Do not copy servo assignments from the upstream reference firmware into the experimental sketches. The two layouts are not compatible.
