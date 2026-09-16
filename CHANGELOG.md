# Changelog

All notable changes to this project are documented here.

## Unreleased

### Rewired robot and dual-axis stabilization

- Imported the exact user-supplied hardware-tested pitch/roll controller as v3.
- Swapped rear-right/front-left channel groups and moved offsets with their motors.
- Updated active configuration, push-up mapping, MPU wiring/axis documentation, and readout signs.
- Preserved legacy v1/v2 source and added explicit wiring/status labels and hash checks.
- Documented reported pitch/roll/diagonal recovery, telemetry, and high-frequency tip-over.
- Updated the roadmap and staged tests; remapped demo/readout retests remain pending.

## Initial repository rework

### Added

- Professional project overview and staged hardware-safety documentation.
- Machine-readable robot configuration with the verified contiguous channel map.
- Automated checks for configuration drift, Arduino sketch layout, documentation links, and upstream file provenance.
- Contribution, security, testing, calibration, architecture, and roadmap guides.

### Changed

- Organized firmware by purpose: tools, diagnostics, demos, experiments, and upstream reference.
- Added Arduino-compatible filenames and matching sketch directories.
- Refactored project sketches for consistent constants, naming, input validation, and channel bounds checks.
- Documented tested and untested firmware status from the recorded hardware sessions.

### Preserved

- Retained the original Nova SM3 Teensy v4.2 source content byte-for-byte after line-ending normalization.
- Retained the final recorded servo offsets and behavior of the project experiments.
