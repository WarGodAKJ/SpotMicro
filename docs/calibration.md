# Servo calibration

Calibration aligns each mechanically installed servo with the software's 90-degree reference.

## Before starting

1. Read [hardware.md](hardware.md), especially the USB service-mode warning.
2. Support the chassis so all four legs can move freely.
3. Inspect every horn, fastener, extension cable, and PCA9685 connector.
4. Verify servo-rail voltage and polarity with a multimeter.
5. Keep the battery disconnect within reach.

## Procedure

1. Open `firmware/tools/servo_calibration/servo_calibration.ino` in Arduino IDE.
2. Select Teensy 4.0 and upload over USB with battery power disconnected.
3. Open Serial Monitor at `19200` baud.
4. Apply servo power using the verified service-mode wiring.
5. Adjust one channel at a time:

   - `0+` increases channel 0 by one degree.
   - `5-` decreases channel 5 by one degree.
   - `p` prints all offsets.
   - `r` resets all offsets to zero.
   - `h` prints help.

6. Center the hip axes and make the thigh/knee linkages visually symmetric without forcing a joint against a mechanical stop.
7. Copy the printed array and update [robot.json](../config/robot.json) plus every project sketch that embeds `kServoOffsets`.
8. Run `python scripts/check_repository.py` to confirm the copies agree.

## Recorded calibration

The final values from the documented hardware session are:

```cpp
{-3, 7, -4, -3, -2, -2, 3, -1, 3, 2, 4, 7}
```

Offsets are specific to the installed horns and servos. Recalibrate after replacing a servo, moving a horn, changing linkage geometry, or rebuilding a leg.
