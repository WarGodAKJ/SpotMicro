# Hardware and electrical safety

## Safety rules

- Keep hands, cables, and tools outside the leg workspace whenever servo power is enabled.
- Support the chassis on a rigid stand so no foot can catch on the bench during initial tests.
- Keep the LiPo battery accessible for immediate disconnection.
- Power off before changing any PCA9685, servo, Teensy, Nano, or IMU connection.
- Never power a servo from the Teensy USB or logic rail.
- Confirm polarity and rail voltage with a multimeter before reconnecting electronics.
- Treat a hot regulator, hot battery, missing status LED, unexpected reset, or burning smell as a fault. Disconnect power immediately.

The development history includes a short circuit and a failed servo driver. Do not bypass the staged checks in [testing.md](testing.md).

## Electrical architecture

The documented build uses an 11.1 V 3S LiPo and separate regulated rails:

```text
3S LiPo
  +-- regulated servo rail (approximately 6.8 V) --> PCA9685 V+ --> 12 servos
  +-- regulated logic rail (approximately 5.4 V) --> controller electronics

Teensy 4.0 -- I2C/Wire --> PCA9685 logic (0x40)
Teensy 4.0 -- I2C/Wire1 --> MPU6050 (0x68, SDA 17, SCL 16)
```

All communicating devices need a common ground, but the high-current servo rail must not be routed through the Teensy.

### USB service mode

The robot previously used USB to power and program the Teensy while the battery independently powered the servo rail. The logic-rail lead to Teensy/Nano VIN was disconnected in that service configuration to avoid back-powering the laptop.

Do not copy this arrangement blindly: verify the exact PCB trace and Teensy VIN/VUSB isolation on the physical build before connecting USB and battery at the same time. If the power topology is uncertain, disconnect the battery and resolve it with a continuity test first.

## Servo channel map

This table is authoritative for the experimental sketches:

| PCA9685 channel | Leg | Joint | Servo class | Final offset |
| ---: | --- | --- | --- | ---: |
| 0 | Rear left | Hip | 20 kg | -3 deg |
| 1 | Rear left | Thigh | 20 kg | +7 deg |
| 2 | Rear left | Knee | 35 kg | -4 deg |
| 3 | Front left | Hip | 20 kg | -3 deg |
| 4 | Front left | Thigh | 20 kg | -2 deg |
| 5 | Front left | Knee | 35 kg | -2 deg |
| 6 | Rear right | Hip | 20 kg | +3 deg |
| 7 | Rear right | Thigh | 20 kg | -1 deg |
| 8 | Rear right | Knee | 35 kg | +3 deg |
| 9 | Front right | Hip | 20 kg | +2 deg |
| 10 | Front right | Thigh | 20 kg | +4 deg |
| 11 | Front right | Knee | 35 kg | +7 deg |

The right-side thigh and knee servos are mechanically mirrored. A visually symmetric crouch therefore uses opposite angle directions on the left and right sides.

## Upstream map incompatibility

The unmodified Nova SM3 v4.2 reference uses the channel groups `0-2`, `4-6`, `8-10`, and `12-14`, and it assigns leg names differently. The project robot uses `0-11` continuously. Uploading upstream motion code without recalibration and remapping can actuate the wrong joints.
