# Test procedure

Run the tests in order. A later stage assumes every earlier stage passed.

## 1. Power-off inspection

- Confirm no short exists between each power rail and ground.
- Confirm PCA9685 signal, V+, and ground orientation for all twelve connectors.
- Confirm the robot is supported with unobstructed leg travel.
- Confirm the battery and regulators remain cool before servo motion begins.

## 2. Controller-only test

Disconnect the battery and verify that the Teensy accepts a simple upload over USB. Do not proceed if the Teensy repeatedly disconnects, fails to enumerate, or becomes warm.

## 3. Servo calibration

Run the calibration tool and move only one channel at a time. Stop immediately if a different joint moves, a servo hits a mechanical stop, or the power system heats unexpectedly.

## 4. IMU diagnostic

Run `firmware/diagnostics/imu_readout/imu_readout.ino`.

- Pitch should change smoothly when the front or rear of the chassis is raised.
- Roll should change smoothly when the left or right side is raised.
- Values should return near their starting values when the robot returns to the same surface.

## 5. Push-up demonstration

Run the push-up sketch with the robot supported. It waits five seconds, completes five cycles, and returns to its calibrated home stance. Verify that front and rear legs move synchronously and that left/right motion is visually mirrored.

## 6. Stabilization v1 reference

V1 is retained because it produced a successful pitch response during the recorded test. Known issues were delayed recovery after lowering the test board, free hip joints, resting bias, and vibration-driven twitching.

## 7. Stabilization v2 experiment

V2 is untested. Use a rigid board, a spotter, conservative battery access, and small angles.

1. Keep the board level during startup.
2. Allow the two-second settling delay and one-second baseline sample period to finish without touching the robot.
3. Confirm all four hips are actively held and the initial crouch is symmetric.
4. Raise the rear edge only a few degrees; the rear legs should compress while the front legs extend.
5. Return to level and confirm recovery.
6. Repeat with the front edge.
7. Stop if correction increases the tilt, oscillation grows, or any joint nears its limit.

Record baseline pitch, raw pitch, filtered pitch, correction output, test direction, recovery behavior, and any oscillation. Do not mark v2 as tested until both directions and return-to-level behavior succeed repeatedly.
