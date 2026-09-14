# Emulator History/Progress remap regression — 2026-09-14

- Source HEAD: `d06c3e6b7cc393b3486f383bc275984611d9815a` (`945643d` remap fix plus `e62230f` test coverage)
- App UUID: `8e2d6c6f-54ee-4f4a-ae6c-7e6cf6b4c8c1`
- Fixture-free bundle SHA-256: `bbecd4e9937064f2fadb3edefd31890770c2a471cc4bd385499e6a05f657f5bb`
- WSL2 preflight, full tests, build, and separate emulator installs: PASS
- No physical install or emulator reset was performed.

## Results

| Platform | History | Progress | Evidence |
| --- | --- | --- | --- |
| Flint | PASS — request id 2, response, accepted, populated calendar render | PASS — request id 1, accepted, graph render points=3 | `s12-remap-flint-history.log`, `s12-remap-flint-history-result.png`, `s12-remap-flint.log`, `s12-remap-flint-result.png` |
| Emery | PASS — request id 2, response, accepted, populated calendar render | PASS — request id 1, both chunks accepted, graph render points=6 | `s12-remap-emery-history.log`, `s12-remap-emery-history-result.png`, `s12-remap-emery.log`, `s12-remap-emery-result.png` |
| Gabbro | PASS — request id 2, response, accepted, populated calendar render | PASS — request id 1, both chunks accepted, graph render points=7 | `s12-remap-gabbro-history.log`, `s12-remap-gabbro-history-result.png`, `s12-remap-gabbro.log`, `s12-remap-gabbro-result.png` |

All Progress responses were normal retained emulator data, not fixtures. The remap change did not regress History or Progress in the emulator. These results do not establish physical phone/watch behavior; the physical gate remains governed by the separate physical diagnostics.
