# SpotMicro Quadruped Robot

[![Repository checks](https://github.com/WarGodAKJ/SpotMicro/actions/workflows/repository-checks.yml/badge.svg)](https://github.com/WarGodAKJ/SpotMicro/actions/workflows/repository-checks.yml)

A Teensy 4.0-based quadruped robotics project derived from the Nova SM3 Spot-Mini Micro clone. This repository records the progression from servo bring-up to automatic pitch and roll leveling with an MPU6050.

> [!WARNING]
> This is a hardware research prototype. Twelve high-torque servos powered by a LiPo battery can move suddenly and with enough force to cause injury or damage. Read [Hardware and electrical safety](docs/hardware.md) before uploading a sketch or applying servo power.

## Current state

The latest [pitch/roll stabilization v3](firmware/experiments/pitch_roll_stabilization_v3/pitch_roll_stabilization_v3.ino) is a **hardware-tested proof of concept**, according to the supplied development handoff. Pitch, roll, diagonal correction, and recovery toward level worked. Increasingly high-frequency shaking eventually caused overreaction and a tip-over; vibration/fall handling remains future work.

| Component | Status | Notes |
| --- | --- | --- |
| Servo calibration tool | Hardware-tested | Produces per-channel angle offsets |
| Five-push-up demo | Remapped; retest pending | Previous routine tested; current channels and offsets updated |
| MPU6050 readout | Updated; retest pending | Original diagnostic tested; now follows confirmed axis signs |
| Pitch stabilization v1 | Legacy wiring | Earlier pitch milestone with known limitations |
| Pitch stabilization v2 | Legacy wiring; untested | Historical pitch-only proposal |
| Pitch/roll stabilization v3 | Hardware-tested, user-reported | Adaptive fusion, diagonal mixing, joint/slew limits; high-frequency tipping remains |
| Original Nova SM3 v4.2 firmware | Reference only | Unmodified upstream snapshot with a different servo-channel map |

## Hardware summary

- Teensy 4.0 main controller
- PCA9685 16-channel PWM servo driver at I2C address `0x40`
- 8 x 20 kg digital servos for hip and thigh joints
- 4 x 35 kg digital servos for knee joints
- MPU6050 IMU at I2C address `0x68` on Teensy `Wire1`
- 11.1 V 3S LiPo with separate regulated logic and servo power rails
- Arduino Nano and additional sensors from the Nova SM3 design, not yet integrated into the experimental sketches

Current channel groups are `0-2` rear left, `3-5` rear right, `6-8` front left, and `9-11` front right. Each group is hip, thigh, knee. See the [servo map](docs/hardware.md#servo-channel-map) and [legacy compatibility guide](firmware/experiments/README.md).

## Quick start

1. Read the [hardware safety guide](docs/hardware.md).
2. Install Arduino IDE with Teensy 4.0 board support.
3. Install the libraries listed in [firmware/README.md](firmware/README.md).
4. Raise the robot so every leg can move without contacting the bench.
5. Run the [calibration procedure](docs/calibration.md).
6. Validate the IMU with the readout diagnostic.
7. Follow the staged [test plan](docs/testing.md) before trying stabilization.

Use v3 for the current wiring after the staged checks. V1/v2 and the upstream snapshot target different wiring. V3 uses 115200 baud; utilities use 19200.

## Repository layout

```text
config/                 Machine-readable hardware configuration
docs/                   Architecture, hardware, calibration, and test guides
firmware/
  demos/                Motion demonstrations (remapped demo awaits retest)
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
