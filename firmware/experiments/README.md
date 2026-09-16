# Stabilization history and wiring compatibility

| Sketch | Wiring | Status |
| --- | --- | --- |
| pitch_stabilization_v1 | Legacy: rear-left, front-left, rear-right, front-right | Earlier pitch response reported; recovery/hip limitations |
| pitch_stabilization_v2 | Legacy: rear-left, front-left, rear-right, front-right | Historical untested pitch-only proposal |
| pitch_roll_stabilization_v3 | Current: rear-left, rear-right, front-left, front-right | User-reported hardware-tested pitch/roll proof of concept |

Each group contains hip, thigh, knee. Legacy v1/v2 must not be uploaded to the
rewired robot. Their source files and original offsets remain unchanged and are
hash-checked to preserve the earlier milestones.

V3 is the exact sketch supplied in the 2026-09-15 update handoff. The original
header calls it "SpotMicro stabilization proof of concept v2"; the repository
uses v3 to avoid confusing it with the old pitch-only v2. No new controller
tuning or runtime behavior changes were made during import.

See [test results and limits](../../docs/testing.md) and
[control architecture](../../docs/architecture.md). High-frequency shaking
eventually caused tipping; the controller is not a complete fall-prevention system.
