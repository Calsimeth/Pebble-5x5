# Emulator acceptance audit — 2026-09-14

## Build and environment

- Tested commit: `9e105cc178a5790828ecb1aa451323455b893bb8`
- Build: normal Pebble production build; `STRONGLIFTS_DEBUG` unset; no fixture mode enabled.
- WSL2 preflight: PASS (`Default Version: 2`, Ubuntu running, `WSL_OK`).
- `npm test`: PASS. Output included transaction matrix, Workout A/B persistence and ACK reload, sync JS, screen transition guards, production Back/abandonment routing, exercise handoff acknowledgement, setup recognizer registration, and five-rep/zero-rep selection.
- `pebble build`: PASS (`build finished successfully`).
- Emulator installs: Flint PASS, Emery PASS, Gabbro PASS; each printed `Installing app...` and `App install succeeded.`
- No source files were modified by this audit.

## Current-build evidence

### First-set default and lower-rep selection

| Platform | First working-set screenshot | Lower-rep screenshot | Result |
| --- | --- | --- | --- |
| Flint | [current-audit-flint-active.png](current-audit-flint-active.png) — `Squat 5x5 150 lb`, five circles each displaying `5` | Not separately captured | PASS for default; lower-rep current run unverified |
| Emery | [current-audit-emery-active2.png](current-audit-emery-active2.png) — `Squat 5x5 210 lb`, five `5` values | [current-audit-emery-reps4.png](current-audit-emery-reps4.png) — first circle `4` | PASS |
| Gabbro | [current-audit-gabbro-active2.png](current-audit-gabbro-active2.png) — `Squat 5x5 185 lb`, five `5` values | [current-audit-gabbro-reps4.png](current-audit-gabbro-reps4.png) — first circle `4` | PASS |

The production test suite separately passed the Flint zero-rep selection path. No emulator result contradicted the five-rep default.

### Back / Continue and exercise handoff

- Flint: after recording five Squat sets, [current-audit-flint-post-squat.png](current-audit-flint-post-squat.png) visibly showed `Next Exercise`, `Select to begin`, and `Bench`. This is PASS for the explicit first boundary handoff.
- Emery and Gabbro: one recorded first Squat set produced Home with `Continue`, and selecting Continue restored the same `Squat 5x5` state with the first circle showing `4`; evidence: [current-audit-emery-home-continue.png](current-audit-emery-home-continue.png), [current-audit-emery-continue.png](current-audit-emery-continue.png), [current-audit-gabbro-home-continue.png](current-audit-gabbro-home-continue.png), and [current-audit-gabbro-continue.png](current-audit-gabbro-continue.png). PASS for Back/Continue restoration.
- Emery/Gabbro full first-boundary handoff was not cleanly captured: retained state entered Deload and warmup screens during the attempted completion sequence. Result: UNVERIFIED, not FAIL.
- Relaunch while paused at the handoff was not performed in this run. Result: UNVERIFIED on all platforms.

The active emulator log sessions produced production lines including `SELECT ... reps=5`, `SET_COMPLETE ... reps=5`, and `SYNC_RECORD_COMMITTED ... q=0`; the captured streams also contained repeated PHONESIM footer warnings. Those warnings are emulator observability noise. No current-run completion record/ACK was generated because the workouts were not completed.

### History and Progress

- Flint History: [current-audit-flint-history.png](current-audit-flint-history.png) visibly rendered `History`, `9/2026`, and a populated calendar. PASS for opening/rendering History in the current build.
- Current-run Progress was not cleanly isolated: navigation left a retained active-session/confirmation flow, and the final capture was inactive Home. Result: UNVERIFIED on Flint, Emery, and Gabbro for this current-build run.
- Existing production evidence from earlier integrated commits remains historical support only; it is not substituted for a current-build populated query where the current run did not establish a valid completed record.

## E1–E24 status for this run

| Case | Flint | Emery | Gabbro | Current evidence |
| --- | --- | --- | --- | --- |
| E1–E3 setup entry/back | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun cleanly in this audit |
| E4 plate defaults | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun cleanly in this audit |
| E5 setup traversal | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun cleanly in this audit |
| E6 discrete adjustment | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun cleanly in this audit |
| E7 held adjustment | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun with current build |
| E8 setup persistence | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun with current build |
| E9 workout choice | PASS | PASS | PASS | Selector screenshots and selected Workout A path |
| E10 confirmation | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun cleanly in this audit |
| E11 Continue/relaunch | PASS | PASS | PASS | Visible Back/Continue restoration; relaunch portion unverified |
| E12 active summary/circles | PASS | PASS | PASS | Active screenshots show summaries and circles |
| E13 completed circle | UNVERIFIED | UNVERIFIED | UNVERIFIED | No clean current final-set run |
| E14 failed/lower reps | UNVERIFIED | PASS | PASS | Emery/Gabbro visible `4` circle |
| E15 final-set transition | UNVERIFIED | UNVERIFIED | UNVERIFIED | Flint handoff observed, full matrix not cleanly completed |
| E16 rest timer | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun |
| E17 vibrations | UNVERIFIED | UNVERIFIED | UNVERIFIED | No current invocation run |
| E18 next-set completion | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun |
| E19 Workout A completion | UNVERIFIED | UNVERIFIED | UNVERIFIED | No current completion |
| E20 Workout B / Deadlift | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun |
| E21 mixed progression | UNVERIFIED | UNVERIFIED | UNVERIFIED | Not rerun |
| E22 synchronization/ACK | UNVERIFIED | UNVERIFIED | UNVERIFIED | No current completed record |
| E23 History | PASS | UNVERIFIED | UNVERIFIED | Flint current calendar screenshot |
| E24 Progress | UNVERIFIED | UNVERIFIED | UNVERIFIED | No clean current query result |

## Gate decision

**EMULATOR GATE: UNVERIFIED / NOT PASSED.**

The current build passes compilation, production tests, installation, first-set defaults, and the observed Flint handoff plus Emery/Gabbro Continue restoration. It does not provide a clean current-build run for every E1–E24 case, particularly handoff relaunch, completion/ACK, and populated Progress. No application defect is assigned from this audit because no clean reproducible current-build failure was established; retained emulator state is the limiting condition.
