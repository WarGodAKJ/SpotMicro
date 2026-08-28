# Contributing

## Principles

- Preserve the separation between upstream reference code and project-specific firmware.
- Treat hardware safety and tested status as part of the implementation, not optional documentation.
- Keep each Arduino sketch self-contained and in a directory matching its filename.
- Update `config/robot.json`, documentation, and embedded sketch constants together when hardware changes.

## Workflow

1. Create a focused branch.
2. Make the smallest coherent change.
3. Run `python scripts/check_repository.py`.
4. Describe the hardware used, the exact test procedure, and the observed result.
5. Open a pull request and leave it as draft until required hardware tests are complete.

## Hardware test evidence

For a motion or control change, include:

- firmware sketch and commit tested;
- battery, regulator, controller, servo driver, IMU, and servo configuration;
- starting pose and support arrangement;
- motion direction and maximum test angle;
- serial output relevant to the result;
- pass/fail behavior, including recovery to the initial pose;
- any heating, reset, oscillation, noise, or mechanical-stop event.

Do not change an experiment's status from untested to tested based only on compilation or simulation.

## Upstream files

Do not edit files under `firmware/reference/` in place. Add a new versioned snapshot with its source URL and Git blob identifiers if an upstream version must be imported.
