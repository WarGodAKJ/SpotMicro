# Firmware guide

Every directory containing an `.ino` file is a standalone Arduino sketch. Open the sketch file directly in Arduino IDE; its parent directory deliberately has the same name as the sketch.

## Sketches

| Sketch | Purpose | Hardware status |
| --- | --- | --- |
| `tools/servo_calibration` | Center one servo at a time and record offsets | Tested |
| `demos/push_ups` | Five push-ups using current wiring | Remapped; retest pending |
| `diagnostics/imu_readout` | Confirmed-axis pitch/roll readout | Updated; retest pending |
| `experiments/pitch_stabilization_v1` | Historical pitch milestone, legacy wiring | Previously tested with limitations |
| `experiments/pitch_stabilization_v2` | Historical pitch-only proposal, legacy wiring | Untested |
| `experiments/pitch_roll_stabilization_v3` | Current pitch/roll leveling with adaptive fusion | Hardware-tested proof of concept (user-reported) |
| `reference/nova_sm3_teensy_v4_2` | Preserve the original Nova SM3 v4.2 firmware | Upstream reference only |

## Arduino dependencies

Install these through Arduino IDE's Library Manager unless noted otherwise:

- Adafruit PWM Servo Driver Library
- Adafruit MPU6050
- Adafruit Unified Sensor
- PID_v2 only for legacy v1/v2; v3 has no PID library dependency
- Teensy board support for Teensy 4.0

The upstream reference firmware has additional dependencies and is intentionally not the recommended starting point for this robot.

## Upload settings

- Board: Teensy 4.0
- USB type: Serial
- Serial monitor: `115200` for v3; `19200` for utilities and legacy v1/v2
- PCA9685 PWM frequency: `60 Hz`
- PCA9685 oscillator setting: `25 MHz`

## Configuration ownership

The human-readable source of truth is [the hardware guide](../docs/hardware.md), and the machine-readable source is [robot.json](../config/robot.json). Active v3 and the push-up demo embed this configuration. V1/v2 remain unchanged legacy snapshots with separate hash checks. See [compatibility and provenance](experiments/README.md).

V3 is imported exactly from the final tested handoff sketch, including its original
"proof of concept v2" header. Its repository name distinguishes it from pitch-only v2.
The reported high-frequency shaking failure remains documented in [testing](../docs/testing.md).

Do not copy servo assignments from the upstream reference firmware into the experimental sketches. The two layouts are not compatible.
