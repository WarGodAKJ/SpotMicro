# SpotMicro Quadruped Robot

[![Repository checks](https://github.com/WarGodAKJ/SpotMicro/actions/workflows/repository-checks.yml/badge.svg)](https://github.com/WarGodAKJ/SpotMicro/actions/workflows/repository-checks.yml)

A Teensy 4.0-based quadruped robotics project derived from the Nova SM3 Spot-Mini Micro clone. This repository records the progression from servo bring-up and calibration to an MPU6050-assisted pitch stabilization experiment.

> [!WARNING]
> This is a hardware research prototype. Twelve high-torque servos powered by a LiPo battery can move suddenly and with enough force to cause injury or damage. Read [Hardware and electrical safety](docs/hardware.md) before uploading a sketch or applying servo power.

## Current state

The project has verified basic actuation, per-servo calibration, a five-push-up demonstration, MPU6050 pitch/roll readings, and a first pitch stabilization controller. The newest stabilization revision is **not yet hardware-tested**.

| Component | Status | Notes |
| --- | --- | --- |
| Servo calibration tool | Hardware-tested | Produces per-channel angle offsets |
| Five-push-up demo | Hardware-tested | Uses the final recorded offsets |
| MPU6050 readout | Hardware-tested | Reports pitch and roll over USB serial |
| Pitch stabilization v1 | Hardware-tested | Corrects pitch but has recovery and stability limitations |
| Pitch stabilization v2 | Experimental | Adds auto-zeroing, filtering, hip hold, and removes integral windup |
| Original Nova SM3 v4.2 firmware | Reference only | Unmodified upstream snapshot with a different servo-channel map |

## Hardware summary

- Teensy 4.0 main controller
- PCA9685 16-channel PWM servo driver at I2C address `0x40`
- 8 x 20 kg digital servos for hip and thigh joints
- 4 x 35 kg digital servos for knee joints
- MPU6050 IMU at I2C address `0x68` on Teensy `Wire1`
- 11.1 V 3S LiPo with separate regulated logic and servo power rails
- Arduino Nano and additional sensors from the Nova SM3 design, not yet integrated into the experimental sketches

The project robot uses PCA9685 channels `0-11` continuously. See the authoritative [servo map](docs/hardware.md#servo-channel-map).

## Quick start

1. Read the [hardware safety guide](docs/hardware.md).
2. Install Arduino IDE with Teensy 4.0 board support.
3. Install the libraries listed in [firmware/README.md](firmware/README.md).
4. Raise the robot so every leg can move without contacting the bench.
5. Run the [calibration procedure](docs/calibration.md).
6. Validate the IMU with the readout diagnostic.
7. Follow the staged [test plan](docs/testing.md) before trying stabilization.

Do not begin with the full upstream firmware or the untested stabilization sketch.

## Repository layout

```text
config/                 Machine-readable hardware configuration
docs/                   Architecture, hardware, calibration, and test guides
firmware/
  demos/                Hardware-tested motion demonstrations
  diagnostics/          Read-only sensor diagnostics
  experiments/          Stabilization controller milestones
  reference/            Unmodified upstream Nova SM3 source snapshot
  tools/                Setup and calibration utilities
scripts/                Repository consistency checks
```

## Documentation

- [Firmware guide](firmware/README.md)
- [System architecture](docs/architecture.md)
- [Hardware and electrical safety](docs/hardware.md)
- [Servo calibration](docs/calibration.md)
- [Test procedure](docs/testing.md)
- [Development roadmap](docs/roadmap.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)

## Upstream relationship

This work is based on Chris Locke's Nova SM3 project. The repository also references the community mirror maintained at [MKme/quadrupedal-robot](https://github.com/MKme/quadrupedal-robot). The exact Nova SM3 Teensy v4.2 files retained under `firmware/reference/` match the upstream Git blobs from [cguweb-com/Arduino-Projects](https://github.com/cguweb-com/Arduino-Projects/tree/main/Nova-SM3).

The upstream snapshot uses a different servo-channel map from this physical robot. It is retained for provenance and reference, not as a drop-in replacement.

## License status

No repository-wide license has been selected. The bundled upstream snapshot did not include a license file in its source directory when reviewed. Do not assume redistribution or reuse rights; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
