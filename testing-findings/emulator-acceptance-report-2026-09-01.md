# Emulator Acceptance Findings

Date: 2026-09-01  
Build: normal fixture-free production build  
Platforms: Flint, Emery, Gabbro

## Preparation

- WSL2 preflight passed: Ubuntu running under WSL 2; `WSL_OK` returned.
- `npm test` passed.
- Clean `pebble build` passed.
- `STRONGLIFTS_VISUAL_FIXTURES` was absent.
- The normal build installed successfully on Flint, Emery, and Gabbro.
- No source files were modified and no commits were made.

## Results

| Test | Flint | Emery | Gabbro | Notes |
|---|---|---|---|---|
| E1 Home | PASS | PASS | PASS | Home shows `New Workout`, `Setup`, `History`, and `Progress`; `Continue` is absent. |
| E2 Home navigation | PASS | PASS | PASS | Short Up/Down navigation and child-screen return were exercised. |
| E3 Setup separation | PASS | PASS | PASS | Setup shows `Exercise Weights` and `Plate Inventory`; both open. |
| E4 Plate defaults | PARTIAL | PARTIAL | PARTIAL | `45 lb`, `Count / side: 2` was captured; the complete default inventory was not recorded. |
| E5 Setup Back traversal | PARTIAL | PARTIAL | PARTIAL | Basic editor return was exercised; full multi-item traversal was not completed. |
| E6 Short adjustment | PASS | PASS | PASS | Up and Down changed the exercise weight by one increment. |
| E7 Hold-to-repeat | PARTIAL | PARTIAL | PARTIAL | Held Up reached a higher value; full comparable-direction timing and release checks were not completed. |
| E8 Setup persistence | NOT TESTED | NOT TESTED | NOT TESTED | No valid completion of the full persistence scenario. |
| E9 Workout A/B choice | PASS | PASS | PASS | Selector shows `Workout A` and `Workout B`; Workout B reaches its specific flow. |
| E10 Back confirmation | FAIL | FAIL | FAIL | No concise abandonment confirmation was observed. |
| E11 Continue | NOT TESTED | NOT TESTED | NOT TESTED | Full valid relaunch/Continue scenario was not completed. |
| E12 Exercise header | PARTIAL | PARTIAL | PARTIAL | Work screens show headers such as `Squat 5x5 45 lb`; Flint warm-up footer is cramped. |
| E13 Set circles | PASS | PASS | PASS | Five circles appear; completed circles visibly change state. |
| E14 Failed set entry | PASS | PASS | PASS | Three- and zero-repetition sets were visibly recorded; circles show actual counts. |
| E15 Final-set transition | FAIL | FAIL | FAIL | After the final set, the app advanced without a verified visible final-circle completion state. |
| E16 Rest timer | PASS | PASS | PARTIAL | Rest counted upward; Gabbro’s lower-right clock is clipped. |
| E17 Vibration invocation | NOT TESTED | NOT TESTED | NOT TESTED | Emulator tactile behavior and invocation trace were not fully verified. |
| E18 Next-set completion | PASS | PASS | PASS | Rest ended and the next set updated the corresponding circle. |
| E19 Workout A completion | FAIL | FAIL | FAIL | Completion displayed `Save failed`. |
| E20 Workout B / Deadlift | FAIL | FAIL | FAIL | Deadlift reached `Deadlift 1x5 95 lb`, but completion displayed `Workout A` and `Save failed`. |
| E21 Failed-weight progression | NOT TESTED | NOT TESTED | NOT TESTED | Blocked by failed durable completion. |
| E22 Synchronization | BLOCKED | BLOCKED | BLOCKED | No durable completed record was produced because completion displayed `Save failed`. |
| E23 History | PASS (empty) | PASS (empty) | PASS (empty) | Clean state displays `No History`; populated-history behavior was blocked. |
| E24 Progress | FAIL | FAIL | FAIL | Clean state opens `Progress Picker` with `Squat` instead of an empty/no-progress state. |

## Evidence

Screenshots were captured in the repository root during testing and visually inspected before use. Representative evidence includes:

- `final-clean-home-flint.png`
- `final-clean-home-emery.png`
- `final-clean-home-gabbro.png`
- `run-setup-flint.png`, `run-setup-emery.png`, `run-setup-gabbro.png`
- `run-work-flint.png`, `run-work-emery.png`, `run-work-gabbro.png`
- `e14-flint-before.png`, `e14-flint-after.png`
- `e14-emery-before.png`, `e14-emery-after.png`
- `e14-gabbro-before.png`, `e14-gabbro-after.png`
- `e20-deadliftwork-flint.png`, `e20-deadliftwork-emery.png`, `e20-deadliftwork-gabbro.png`
- `e22-complete-flint.png`, `e22-complete-emery.png`, `e22-complete-gabbro.png`
- `clean-e23-history-flint.png`, `clean-e23-history-emery.png`, `clean-e23-history-gabbro.png`
- `clean-e24-progress-flint.png`, `clean-e24-progress-emery.png`, `clean-e24-progress-gabbro.png`

Some intermediate screenshots were intentionally discarded because they showed the Pebble timeline (`No events`) or another invalid state rather than the application.

## Gate decision

`EMULATOR GATE: REJECTED`

The cross-platform `Save failed` result blocks durable completion, synchronization, populated History, Progress data, and progression verification. Physical-watch testing must not proceed under the emulator-first acceptance process.

## Findings by subject

### Workout/UI

- No Back abandonment confirmation was observed.
- The final exercise set advances before a verified final-circle completion state is visible.
- Gabbro clips the lower-right clock.
- Flint’s warm-up footer is cramped.

### Rest/vibration

- Rest elapsed visibly on all platforms, but Gabbro’s clock presentation is clipped.
- Vibration invocation timing/pattern selection was not fully instrumented.

### Setup

- Setup separation and one-step adjustment worked.
- Complete default-inventory and hold-repeat acceptance coverage remains partial.

### Persistence/synchronization

- Workout completion consistently displays `Save failed` on all three platforms.
- This prevents durable completion and blocks synchronization/progression verification.

### History/Progress

- Empty History correctly displays `No History`.
- Empty Progress incorrectly presents `Progress Picker` with `Squat`.
- Populated History and Progress could not be tested without a durable completed record.
