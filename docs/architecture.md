# System architecture

## Scope

The current experimental firmware controls the twelve leg servos and reads the MPU6050 from a Teensy 4.0. The Arduino Nano, ultrasonic sensors, PIR sensors, displays, audio, radio, and autonomous gait features present in the broader Nova SM3 design are outside the active controller scope.

## Control flow

```text
MPU6050 acceleration
        |
        v
pitch calculation --> optional low-pass filter --> pitch controller
                                                     |
                                                     v
                                  mirrored stance correction
                                                     |
                                                     v
                         calibration offset + angle constraint
                                                     |
                                                     v
                                  PCA9685 --> 12 servos
```

## Pitch stabilization v2

The latest experiment performs four steps:

1. Command a symmetric crouch and actively hold all four hip joints.
2. Average 50 pitch samples after a two-second settling period and use that average as the level reference.
3. Apply an exponential moving-average filter with `alpha = 0.15` to reduce vibration-induced noise.
4. Apply proportional correction with a gain of `0.6`, limited to `+/-30 degrees`, to opposing front and rear leg pairs.

Although the sketch uses the PID_v2 interface, both integral and derivative gains are zero in v2. It therefore behaves as a proportional controller. This deliberately avoids the integral windup observed in v1 and the derivative response to servo vibration.

## Coordinate and mirroring conventions

- Front/rear correction is pitch-only.
- Left/right roll correction is not implemented.
- The left and right mechanisms are mirrored about the chassis centerline.
- The project robot's physical channel order is rear-left, front-left, rear-right, front-right.

## Provenance boundary

`firmware/reference/nova_sm3_teensy_v4_2` is an exact upstream source snapshot. All other firmware directories describe the project-specific, contiguous-channel robot. Keeping the two areas separate makes provenance and configuration differences explicit.
