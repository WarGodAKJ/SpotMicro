# Test procedure and recorded results

## Reported hardware results

Source: the user-provided repository update handoff, incorporated 2026-09-15.
These are reported physical observations, not tests run by the repository checker.
The exact supplied controller is stored as
[the v3 sketch](../firmware/experiments/pitch_roll_stabilization_v3/pitch_roll_stabilization_v3.ino).
Its original header says "proof of concept v2"; repository v3 distinguishes it from
the earlier pitch-only v2.

| Milestone | Reported result |
| --- | --- |
| Sequential servo activation after rewiring | All twelve channels worked |
| Pitch, both directions | Successful |
| Roll, both directions | Successful |
| Diagonal pitch/roll response | Successful |
| Return toward level | Stable recovery |
| Vibration rejection | Improved over previous controller |
| Increasing high-frequency shaking | Overreaction and tip-over |

The handoff does not provide angle/frequency logs, repetition counts, or separately
logged results for every diagonal. Do not infer quantified stability margins.
The remapped repository push-up demo and revised readout still require a retest.

## Bring-up

Follow [hardware precautions](hardware.md), verify power-off wiring and rails,
and support the chassis before initial servo motion. Confirm current groups:
0-2 rear left, 3-5 rear right, 6-8 front left, 9-11 front right.
Within each group: hip, thigh, knee.

The reported sequential diagnostic centered with reordered offsets, then moved
each channel 90 -> 102 -> 78 -> 90, one channel at a time from 0 through 11.
The exact diagnostic source was not supplied; this records its procedure, not
an additional delivered sketch. Verify both joint identity and direction.

Run [calibration](calibration.md), then the readout diagnostic at 19200 baud.
Rear raised should give positive pitch; right raised should give positive roll.
If the Serial Monitor is blank, verify board, USB port, and baud first: the reported
blank-output incident was a Windows/Arduino COM-port selection issue.

## Push-up retest

The demo now uses the new mapping and offsets. It enters home, waits five seconds,
performs five repetitions, then holds home. Confirm all legs compress together
with mirrored left/right commands. Record a new result for this repository revision.

## V3 acceptance procedure

Use a supported robot on a rigid movable surface, with accessible battery disconnect.
Open the v3 Serial Monitor at 115200 baud. Allow the five-second motion countdown,
stance transition, two-second settling delay, and calibration to finish undisturbed.

1. Rear raised: rear legs compress, front legs extend; then return to level.
2. Front raised: front legs compress, rear legs extend; then return to level.
3. Right raised: right legs compress, left legs extend; then return to level.
4. Left raised: left legs compress, right legs extend; then return to level.
5. Front-right raised: front-right compresses most, rear-left extends most.
6. Front-left raised: front-left compresses most, rear-right extends most.
7. Rear-right raised: rear-right compresses most, front-left extends most.
8. Rear-left raised: rear-left compresses most, front-right extends most.
9. Apply slow combined disturbances and check repeatable recovery without growing oscillation.

Keep the initial motions small. Stop on growing oscillation, overheating,
unexpected reset, or joint-stop contact. Do not reproduce the known high-frequency
tip-over as a routine acceptance test. Any future frequency sweep needs physical
fall restraint and a defined stop condition.

## Telemetry

Startup reports PitchTarget and RollTarget. At nominal 10 Hz the controller reports:

| Field | Meaning |
| --- | --- |
| Pitch, Roll | Fused orientation in degrees |
| PitchError, RollError | Filtered deviations from startup targets |
| Trust | Accelerometer trust, 0 to 1 |
| LR, RR, FL, FR | Applied leg correction in degrees, before base compression |

Trust should generally be higher at rest when magnitude and predicted attitude
agree. Translational disturbances or large disagreement reduce it; low trust
does not prove vibration recognition. Log raw test action, firmware commit,
calibration targets, telemetry, recovery, and failures.

The >35-degree deviation behavior returns toward base crouch with slew limiting;
it neither disables servos nor latches a fall state. Hardware validation of that
threshold is not separately established in the handoff.

## Legacy firmware

Do not upload pitch-only v1/v2 onto current wiring. Their old mappings and recorded
limitations are retained in [the history guide](../firmware/experiments/README.md).
