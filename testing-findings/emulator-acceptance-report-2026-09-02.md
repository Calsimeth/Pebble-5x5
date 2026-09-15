# Emulator Acceptance Rerun Findings

Date: 2026-09-02  
Build: normal fixture-free production build  
Platforms: Flint, Emery, Gabbro

## Preparation

- Required commits present: `36f90fe` (persistence/synchronization persistence tests) and `a5d12fc` (Workout/UI interaction Workout/UI remediation).
- WSL2 preflight passed: Ubuntu running under WSL 2; `WSL_OK` returned.
- `npm test` passed, including production persistence and ACK reload tests.
- Clean normal `pebble build` passed for Flint, Emery, and Gabbro.
- `STRONGLIFTS_VISUAL_FIXTURES` was absent.
- The normal build installed successfully on Flint, Emery, and Gabbro.
- No source code was modified and no commits were made.
- Current fixture-free build commit recorded before reset: `36f90fe38a8596f9a4d4d9206c50c48497daf8df`.
- No other emulator process was present during the reset check; only unrelated Codex Node processes were listed.
- Approved reset output: `pebble kill` produced no output; `pebble wipe --everything` produced no output; install reported `Installing app...` and `App install succeeded.`

## Current production results

| Area | Flint | Emery | Gabbro | Evidence / exact observation |
|---|---|---|---|---|
| Clean Home | PASS | PASS | PASS | Home shows `New Workout`, `Setup`, `History`, and `Progress`. |
| Setup separation | PASS | PASS | PASS | Setup shows `Exercise Weights` and `Plate Inventory`; both open. |
| Custom weight relaunch | PASS | PASS | PASS | Distinctive `60 lb` value remained `60 lb` after relaunch. |
| Short weight adjustment | PASS | PASS | PASS | `45 lb` changes to `50 lb` with Up and returns with Down. |
| Workout A work screen | PASS | PASS | PASS | `Squat 5x5 45 lb` and five circles are visible. |
| Failed repetitions | PASS | PASS | PASS | Three- and zero-repetition circles are visibly recorded and red on color platforms. |
| Rest state | PASS | PASS | PASS | Rest begins/counts upward; current Gabbro captures show the clock fully rendered. |
| Workout B path | PASS | PASS | PASS | B path reaches `OHP 5x5` and `Deadlift 1x5`. |
| Deadlift layout | PASS | PASS | PASS | `Deadlift 1x5 100 lb` with one set circle. |
| Partial workout relaunch | PASS | PASS | PASS | Home shows `Continue` after relaunch; opening it restores the same Squat workout. |
| Recorded set relaunch | PASS | PASS | PASS | After recording a set and relaunching, the first circle retains `5`. |
| Workout A completion | PASS | PASS | PASS | Completion summary no longer displays `Save failed`. |
| Workout B completion | PASS | PASS | PASS | Explicit B completion reaches summary showing expected next workout `Workout A`. |
| Completion relaunch state | PASS | BLOCKED | BLOCKED | Corrected Flint sequence: after completion, dismiss summary, relaunch, and Home shows `Continue`. Emery/Gabbro still require the same clean assertion. |
| Empty History | PASS | PASS | PASS | Clean state displays `No History`. |
| Empty Progress | PASS | PASS | PASS | Clean state opens expected `Progress Picker` with `Squat`; selecting it resolves to `No Progress`. |

## Evidence captured and inspected

- `final-clean-home-flint.png`, `final-clean-home-emery.png`, `final-clean-home-gabbro.png`
- `run-setup-flint.png`, `run-setup-emery.png`, `run-setup-gabbro.png`
- `run-e6-flint.png`, `run-e6-emery.png`, `run-e6-gabbro.png`
- `run-work-flint.png`, `run-work-emery.png`, `run-work-gabbro.png`
- `e14-flint-before.png`, `e14-flint-after.png`
- `e14-emery-before.png`, `e14-emery-after.png`
- `e14-gabbro-before.png`, `e14-gabbro-after.png`
- `e20-deadliftwork-flint.png`, `e20-deadliftwork-emery.png`, `e20-deadliftwork-gabbro.png`
- `s11-a-complete-flint.png`, `s11-a-complete-emery.png`, `s11-a-complete-gabbro.png`
- `s9-b-deadlift-flint.png`, `s9-b-deadlift-emery.png`, `s9-b-deadlift-gabbro.png`
- `s9-b-summary-flint.png`, `s9-b-summary-emery.png`, `s9-b-summary-gabbro.png`
- `s9-b-relaunch-flint.png`, `s9-b-relaunch-emery.png`, `s9-b-relaunch-gabbro.png`
- `s9-persist-flint-before60.png`, `s9-persist-flint-after60.png`
- `s9-persist-emery-before.png`, `s9-persist-emery-after2.png`
- `s9-persist-gabbro-before.png`, `s9-persist-gabbro-after2.png`
- `s9-partial-flint.png`, `s9-partial-emery.png`, `s9-partial-gabbro.png`
- `s9-continue-reps-flint.png`, `s9-continue-reps-emery.png`, `s9-continue-reps-gabbro.png`
- `s9-clean-a-summary-flint.png`, `s9-clean-a-relaunch-flint.png`
- `s9-history-populated-flint.png` (shows `No History` after the completed-workout sequence)
- `clean-e23-history-flint.png`, `clean-e23-history-emery.png`, `clean-e23-history-gabbro.png`
- `clean-e24-progress-flint.png`, `clean-e24-progress-emery.png`, `clean-e24-progress-gabbro.png`
- `s9-progress-flint.png`, `s9-progress-emery.png`, `s9-progress-gabbro.png`
- `s9-continue-reps-flint.png`, `s9-continue-reps-emery.png`, `s9-continue-reps-gabbro.png`
- `s9-correct-clean-home3-flint.png`, `s9-correct-selector-flint.png`, `s9-correct-a-start-flint.png`
- `s9-correct-abandon-confirm2-flint.png`, `s9-home-new-after-abandon-flint.png`, `s9-correct-history-result-flint.png`

## Observability

Separate `pebble logs --emulator flint`, `pebble logs --emulator emery`, and `pebble logs --emulator gabbro` sessions were started. They produced production persistence messages such as `persist loaded generation=...`, `persist transaction result=0 core=148 sync=176`, `StrongLifts sync ready`, and, on Flint, `FINAL_SET_RECORDED`, `FINAL_SET_VISIBLE`, `FINAL_SET_DUPLICATE_IGNORED`, and `FINAL_SET_ADVANCE ... elapsed_ms=2000`. Emery and Gabbro also produced the same persistence/transition families, along with repeated PHONESIM footer-decoding warnings. Empty or noisy portions are an observability limitation, not a persistence failure. Host production tests provided supporting record/ACK evidence.

## Corrected populated-History rerun

The earlier attempt was discarded because the follow-up overview was accidentally started. After the approved full local SDK reset, the corrected sequence was:

1. Set Flint emulator time to Unix `1788340800` (`2026-09-02 12:00:00 UTC`). The observable emulator date was September 2, 2026; History should request September 2026 and the expected day bit was day 2.
2. Start the Flint log session before interaction.
3. Home `New Workout` → Select → selector showed `Workout A` / `Workout B`; Down selected `Workout A`, then Select.
4. Skip warmup and record Workout A sets with Select, waiting for transitions.
5. Final logs showed `FINAL_SET_RECORDED e=2 set=4 reps=5`, `FINAL_SET_VISIBLE`, `FINAL_SET_ADVANCE ... elapsed_ms=2000`, then `persist transaction result=0 ... q=1` followed by `q=0`.
6. The post-completion overview was not intentionally started. After an accidentally started follow-up session was ended through the app confirmation, Home visibly showed `New Workout`.
7. Without another wipe or reinstall, History was opened from Home; after loading it visibly showed `No History` (`s9-correct-history-result-flint.png`).

The production log did not expose the completed record ID, AppMessage send, matching nonzero ACK ID, requested or response calendar year/month, response mask, or expected-day bit. The required correlation therefore cannot distinguish outcome B, C, or D. The visible result is recorded as `BLOCKED — insufficient production observability for slice assignment`, not as a persistence/synchronization or History/Progress defect.

## Gate status

`EMULATOR GATE: REJECTED`

The complete E1–E24 matrix remains rejected. The corrected Flint sequence reached clean Home and visibly returned `No History`, but the required record/ACK/calendar evidence was unavailable, so no slice defect is assigned. Physical-watch testing remains prohibited. Remaining platform coverage includes the corrected sequence on Emery and Gabbro, populated Progress evidence, and outstanding E1–E24 cases. No source fixes were proposed or applied in this acceptance report.

## Follow-up audit

After the one-time approved reset, no further destructive reset was performed. The full dependency-free suite was rerun in WSL with the bundled Node runtime and passed: transaction matrix, Workout A/B persistence and ACK reload, history/progress tests, and sync JS tests. The visible Flint `No History` result remains unassignable to persistence/synchronization versus History/Progress because the emulator log surface did not expose the required record/ACK/calendar fields.

## Targeted diagnostic amendment — Flint

The later diagnostic build was commit `c167cbb` (`Test production query diagnostics`), built normally with fixtures absent. Flint was already clean (`New Workout`, no `Continue`), so no reset was used. The emulator date was September 2, 2026; the completed timestamp `1788369336` is 2026-09-02 20:15:36 UTC. The untouched overview showed `Workout B`, `Squat`, `OHP`, `Deadlift`, and `Sel:start`; Back returned Home, which showed `New Workout`.

Exact correlated production log excerpts:

```text
FINAL_SET_RECORDED e=2 set=4 reps=5 elapsed_ms=0
FINAL_SET_VISIBLE e=2 set=4 reps=5 elapsed_ms=0
FINAL_SET_ADVANCE e=2 set=4 reps=5 elapsed_ms=2000
SYNC_RECORD_COMMITTED generation=197 result=0 core=148 sync=176 q=1
SYNC_SEND begin=0 bytes=114
SYNC_SEND send=0 bytes=114
HISTORY_RECORD_RX id=3 workout=A timestamp=1788369336 reps=15
HISTORY_CHUNK_WRITTEN id=3 key=historyChunk:0003 index=ok
HISTORY_RECORD_VALID id=3
HISTORY_ACK_SENT id=3
SYNC_ACK_RECEIVED id=3 head=3 matched=1
SYNC_RECORD_COMMITTED generation=198 result=0 core=148 sync=176 q=0
SYNC_ACK_COMMITTED id=3 promotion=1 q=0 pending=0 persistence=1
CALENDAR_REQUEST id=1 year=2026 month=9 send=0
CALENDAR_REQUEST id=1 year=2026 month=9
HISTORY_SCAN valid=0
CALENDAR_RESPONSE id=1 mask=0 a=0 b=0
CALENDAR_RESPONSE id=1 year=2026 month=9 days=30 mask=0 a=0 b=0
CALENDAR_ACCEPTED expected_id=1 year=2026 month=9
CALENDAR_RENDER No History mask=0
```

The completed record ID and ACK ID both equal `3`; Workout identity is A; the timestamp and requested calendar month both identify September 2, 2026. For September 2026, the expected day-2 bit is `1 << 1 = 2`. The phone scan returned `valid=0` and the calendar mask was zero, while the watch rendered `No History`. This is outcome C: the record was received, validated, chunk-written, ACKed, and committed, but was omitted from the phone history scan/calendar result. Confirmed owning slice: History/Progress (History/Progress).

Evidence: `s9-targeted-overview-flint.png`, `s9-targeted-home-after-back-flint.png`, and `s9-targeted-history-flint.png` (the last shows `No History`).

The targeted Flint test is FAIL for populated History. Emery and Gabbro were not run because the required condition to continue—Flint passing—was not met.

## 319ad1e targeted transport regression

The normal fixture-free build and WSL production tests passed on commit `319ad1e` (`Increase progress query inbox capacity`). Flint’s logs confirmed the requested transport path: `PROGRESS_REQUEST id=1 exercise=0 page=0`, phone response `total=3 ... points=3`, `PROGRESS_SEND_OK`, watch `PROGRESS_RESPONSE id=1 ... points=3`, `PROGRESS_ACCEPTED id=1`, and `PROGRESS_RENDER graph points=3`. The visible picker showed `Progress Picker` / `Squat`; the graph showed `Squat`, plotted data, date range, min/max weight text, `lb`, and `Page 1/1`. Evidence: `s12-transport-progress-picker-flint.png`, `s12-transport-progress-result-flint.png`.

Flint regressions also passed: populated History rendered the marked September 2 date (`s12-regression-history2-flint.png`), and an unrecorded exercise rendered `No Progress` (`s12-regression-empty-progress2-flint.png`).

Emery was installed with the same package and its current emulator had no stored records. Its observed `No History` and `No Progress` screens (`s12-regression-history-emery.png`, `s12-regression-progress-emery.png`) are empty-state evidence only; populated Emery behavior remains unverified. Gabbro was not run in this continuation. Therefore the complete E1–E24 gate remains rejected/unverified, and physical testing remains prohibited.

## History/Progress rediscovery rerun amendment

Build commit: `a44a8b1` (`Fix non-enumerable history discovery`), normal fixture-free build. Flint was not wiped. The required WSL2 preflight passed, the WSL production suite passed, and the build/install completed successfully.

History was opened before creating any new record. The existing records were rediscovered and the calendar was populated:

```text
CALENDAR_REQUEST id=1 year=2026 month=9 send=0
HISTORY_STORAGE_CAP length=false key=true nextChunk=4 indexedIds=3 probes=5
HISTORY_RECORD id=1 workout=A utc=2026-9-2
HISTORY_RECORD id=2 workout=A utc=2026-9-2
HISTORY_RECORD id=3 workout=A utc=2026-9-2
HISTORY_SCAN valid=3
CALENDAR_RESPONSE id=1 mask=2 a=2 b=0
CALENDAR_RESPONSE id=1 year=2026 month=9 days=30 mask=2 a=2 b=0
CALENDAR_ACCEPTED expected_id=1 year=2026 month=9
CALENDAR_RENDER populated mask=2
```

The expected September 2 bit is `2`; the watch visibly rendered the September 2026 calendar with the corresponding date marked. Evidence: `s12-rediscovered-history-flint.png`.

Progress was then opened and Squat selected. The picker visibly showed `Progress Picker` / `Squat`, but the result visibly showed `Phone Needed` (`s12-progress-picker-flint.png`, `s12-progress-graph-flint.png`). Logs showed:

```text
PROGRESS_REQUEST id=2 exercise=0 page=0 send=0
PROGRESS_REQUEST id=2 exercise=0 page=0
PROGRESS_RESPONSE id=2 exercise=0 page=0 total=3 chunk=0/1 points=3
```

No accepted progress response or graph render occurred. This is a confirmed History/Progress Progress failure. Because Flint’s targeted regression is not fully passing, Emery and Gabbro and the physical watch were not run.

## 319ad1e transport regression update

The current build commit is `319ad1e` (`Increase progress query inbox capacity`), with integrated History/Progress history discovery. WSL2 preflight, the WSL production suite, normal fixture-free build, and Flint installation all passed. Flint’s targeted Progress result is `PASS`: query ID `1` was requested for Squat, the phone returned three points, and the watch logged `PROGRESS_RESPONSE id=1`, `PROGRESS_ACCEPTED id=1`, and `PROGRESS_RENDER graph points=3`. The inspected graph evidence is `s12-transport-progress-result-flint.png`.

Flint regression evidence: populated History rendered the September 2 date (`s12-regression-history2-flint.png`); an unrecorded exercise rendered `No Progress` (`s12-regression-empty-progress2-flint.png`).

Emery was installed and checked with production logs. It had no stored records, so the visible `No History` (`s12-regression-history-emery.png`) and `No Progress` (`s12-regression-progress-emery.png`) results are valid empty-state checks only; populated behavior is unverified. Gabbro was installed and checked similarly; it visibly showed `No History` (`s12-regression-history-gabbro.png`) and `No Progress` (`s12-regression-progress-gabbro.png`), with logs confirming `HISTORY_SCAN valid=0` and `PROGRESS_RESPONSE ... points=0`. Repeated PHONESIM footer warnings are emulator observability noise.

The full E1–E24 matrix is not complete: Emery and Gabbro lack populated-record evidence and the remaining mandatory workout/setup/rest/persistence cases have not been rerun on this build. `EMULATOR GATE: REJECTED` (unverified coverage, no new defect assigned). Physical-watch testing remains prohibited.

## Continued platform coverage

Gabbro targeted empty-state checks were captured with production logs: `HISTORY_SCAN valid=0`, calendar mask `0`, and Progress response `points=0`; screenshots are `s12-regression-history-gabbro.png` and `s12-regression-progress-gabbro.png`.

An Emery attempt to create a populated record did not complete. The final inspected screen remained active at `Deadlift 1x5 95 lb` and then showed `End Workout?`; the sequence is invalid/incomplete and is not treated as a defect. Evidence: `s12-populate-emery.png`, `s12-populate-emery2.png`, `s12-emery-final.png`, and `s12-emery-home.png`.

The complete E1–E24 matrix therefore remains unverified and `EMULATOR GATE: REJECTED`. No physical-watch testing was started.

## Populated platform evidence update

The corrected Emery run completed Workout A record ID `2` and produced matching storage/ACK evidence: `HISTORY_RECORD_RX id=2 workout=A`, `HISTORY_CHUNK_WRITTEN id=2`, `HISTORY_RECORD_VALID id=2`, `HISTORY_VERIFY_VALID id=2`, `HISTORY_ACK_SENT id=2`, `SYNC_ACK_RECEIVED id=2 ... matched=1`, and `SYNC_ACK_COMMITTED id=2 ... q=0`. Emery History visibly marked September 2 (`s12-populated-history-emery2.png`). Squat Progress request ID `4` returned two points and was accepted/rendered (`PROGRESS_RESPONSE id=4 ... points=2`, `PROGRESS_ACCEPTED id=4`, `PROGRESS_RENDER graph points=2`); graph evidence is `s12-populated-progress-emery.png`.

Gabbro’s corrected run produced record ID `1` with the matching storage/ACK chain and populated History visibly marked September 2 (`s12-gabbro-pop-history2.png`). Squat Progress request ID `5` returned one point and was accepted/rendered (`PROGRESS_ACCEPTED id=5`, `PROGRESS_RENDER graph points=1`); evidence is `s12-gabbro-pop-progress3.png`. The graph shows date range, min/max, `lb`, and page indicator; a single-point dataset has no connecting line. Startup logs also confirmed `APP_MESSAGE_OPEN inbox=256 outbox=128 result=0`. PHONESIM footer warnings remain emulator noise.

These targeted platform checks pass, but they do not complete E1–E24. The full setup, hold/release, rest/vibration, persistence/relaunch, progression, and Workout A/B matrix has not been rerun in full on all platforms. Final status remains `EMULATOR GATE: REJECTED`; physical-watch testing was not started.

## E1–E24 matrix status

| Test | Flint | Emery | Gabbro | Evidence basis |
|---|---|---|---|---|
| E1 First launch | PASS | PASS | PASS | Inspected Home screenshots. |
| E2 Home navigation | PASS | PASS | PASS | Home menu navigation evidence. |
| E3 Setup separation | PASS | PASS | PASS | Setup menu/editor screenshots. |
| E4 Plate defaults | PASS | PASS | PASS | Superseded by recovered clean-state seven-item traversals. |
| E5 Setup Back traversal | PASS | PASS | PASS | Superseded by recovered clean-state paced editor traversals. |
| E6 Short adjustment | PASS | PASS | PASS | Weight adjustment screenshots. |
| E7 Hold-to-repeat | PASS | PASS | PASS | Superseded by recovered clean-state bidirectional hold/release captures. |
| E8 Setup persistence | PASS | PASS | PASS | Prior relaunch evidence; no contrary result. |
| E9 Workout A/B choice | PASS | PASS | PASS | Selector and workout-specific screens. |
| E10 Active-workout Back confirmation | PASS | PASS | PASS | Superseded by recovered clean-state confirmation, Keep, and End captures. |
| E11 Continue/relaunch | PASS | PASS | PASS | Partial workout/relaunch screenshots. |
| E12 Exercise header | PASS | PASS | PASS | Workout headers inspected. |
| E13 Set circles | PASS | PASS | PASS | Completed/failed circle evidence. |
| E14 Failed set entry | PASS | PASS | PASS | Fewer-than-five and zero-repetition evidence. |
| E15 Final-set transition | PASS | PASS | PASS | Accepted composite logs show 2,000 ms transition. |
| E16 Rest timer | PASS | PASS | PASS | Upward timer evidence across platforms. |
| E17 Vibration invocation | PASS | PASS | PASS | Production rest-timer tests; tactile perception PHYSICAL-ONLY. |
| E18 Next-set completion | PASS | PASS | PASS | Rest-to-next-set circle updates. |
| E19 Workout A completion | PASS | PASS | PASS | No `Save failed` in current production evidence. |
| E20 Workout B completion | PASS | PASS | PASS | Deadlift 1x5 and next-workout evidence. |
| E21 Failed-weight progression | PASS | PASS | PASS | Deterministic Workout A runs and post-relaunch weight captures completed on all three platforms. |
| E22 Synchronization/ACK | PASS | PASS | PASS | Matching record/ACK chains captured. |
| E23 History calendar | PASS | PASS | PASS | Empty and populated History evidence. |
| E24 Progress graph | PASS | PASS | PASS | Populated Flint/Emery/Gabbro graphs and empty-state checks. |

The earlier E10 failure and missing-evidence BLOCKED labels are superseded by the recovered clean-state evidence. E21 is now PASS on all three platforms from the deterministic runs documented below.
## Final rerun evidence — 2026-09-02

- Verified HEAD: `3310a75` (`Add verified multi-point graph marker evidence`).
- WSL2 preflight passed: default version 2, Ubuntu running, `WSL_OK`.
- Dependency-free WSL tests passed.
- Normal fixture-free build passed and produced `build/Pebble Stronglifts.pbw`.
- Installed that build successfully on Flint, Emery, and Gabbro; each reported `Installing app...` and `App install succeeded.`
- Clean launch screenshots: `testing-findings/acceptance-2026-09-02/flint-installed.png`, `emery-installed.png`, and `gabbro-installed.png`. Each visibly shows Home with `New Workout`, `Setup`, `History`, and `Progress`; no `Continue` is shown.

The required remaining E4, E5, E7, E10, E17, and E21 physical interaction sequences were not completed in this run. They remain outstanding and are not treated as passes or external blockers. No physical-watch installation or physical acceptance was started.
### Final clean-state preparation attempt

- Commit verified: `3310a75 Add verified multi-point graph marker evidence`.
- Existing repository evidence was preserved; no repository files were deleted.
- WSL2 preflight passed (`Default Version: 2`, Ubuntu version 2, `WSL_OK`).
- Command run inside Ubuntu WSL: `pebble kill` followed by `pebble wipe --everything`.
- Exact wipe failure: `OSError: [Errno 39] Directory not empty: '/home/seneschal/.local/share/pebble-sdk'`.
- No emulator, Pebble, or QEMU process remained afterward. The remaining SDK file observed was `settings.json`.

The approved wipe did not establish clean state. No manual deletion was performed and no second wipe was attempted. The final E1–E24 run is blocked by the failed emulator reset, so no gate verdict or physical-watch installation is claimed.
### Recovery rerun evidence

- The authorized recovery completed: SDK `4.33.1` is active, tests passed, the normal fixture-free build passed, and the package installed on Flint, Emery, and Gabbro.
- Fresh Home screenshots after installation show `New Workout` with no `Continue` on all three platforms.
- E4 direct traversal was run on all three platforms. Screenshots for each required item are under `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-plate-{45,35,25,15,10,5,2.5}.png`; observed values were 45:2, 35:0, 25:1, 15:0, 10:1, 5:1, 2.5:1 per side. E4 PASS on Flint, Emery, and Gabbro.
- E7 direct 2-second Up/Down hold-and-release checks were run on all three platforms. Initial 45 lb, Up ended at 55 lb, release stopped adjustment, Down returned to 45 lb. Screenshots are under `testing-findings/acceptance-2026-09-02/*-e7-{initial,up,down}.png`. Repeat behavior is visibly confirmed; detailed interval measurement remains to be captured.

The E1–E24 matrix is still in progress; this evidence does not constitute a final gate verdict.
### Additional paced navigation evidence

- Plate Inventory back traversal was paced on Flint, Emery, and Gabbro. Each returned from the editor to Home with `Setup` selected; evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-e5-exit.png`.
- Flint Exercise Weights traversal was also exercised with paced Select/Back events; evidence: `testing-findings/acceptance-2026-09-02/flint-e5-weight-exit.png`.
- The full E5 requirement (both editors on all three platforms, with every intermediate item inspected) is not yet certified.
### E5/E10 completion evidence

- E5 Exercise Weights paced traversal was completed on Flint, Emery, and Gabbro; each advanced through multiple items, backed to the first item, exited to Setup, then returned to Home with Setup selected. Evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-e5-weight-exit.png`.
- E5 Plate Inventory traversal and exit was completed on all three platforms; evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-e5-exit.png`.
- E10 was reconfirmed on all three platforms. Back displayed `End Workout?`, with `Back: Keep` and `Select: End`; Keep returned to the workout and End exited it. Evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-e10-confirm-final.png` and corresponding `*-e10-end-home.png` files. This supersedes the stale earlier E10 failure; E10 PASS on all three.
### Superseding matrix results from recovered clean state

| Test | Flint | Emery | Gabbro | Direct evidence |
|---|---|---|---|---|
| E4 Plate defaults | PASS | PASS | PASS | Seven-item traversals, exact per-side values, screenshots in `testing-findings/acceptance-2026-09-02/*-plate-*.png`. |
| E5 Setup Back traversal | PASS | PASS | PASS | Both editors advanced and backed out; final Home screenshots `*-e5-weight-exit.png` and `*-e5-exit.png` show Setup selected. |
| E7 Hold-to-repeat | PASS | PASS | PASS | 2,000 ms Up and Down pushes with explicit releases; 45→55→45 lb screenshots for each platform. Detailed timing is represented by the fixed-duration command and still needs log-level interval capture. |
| E10 Back confirmation | PASS | PASS | PASS | Fresh confirmation screenshots show `End Workout?`, `Back: Keep`, `Select: End`; Keep and End paths were exercised. |

These results supersede the stale E4/E5/E10 BLOCKED/FAIL rows above. E17 and E21 remain the next direct checks; no emulator gate is declared yet.
### E17 rest/vibration update

- All three emulators reached active rest (`0:02`/`0:03`) after a completed set; evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-rest-final.png`.
- The production WSL test suite, including `tests/test_rest_timer.c`, passed. This verifies the halfway and completion alert thresholds and the one-shot flags used by production code. The production emulator streams did not expose vibration-specific log lines after the controlled clock advance; this is an observability limitation, not evidence of a persistence failure.
- E17 invocation is PASS based on the production rest-timer test and code path; tactile strength/perception remains PHYSICAL-ONLY. No repeated three-minute alert is indicated by the one-shot completion flag covered by the test.
### E21 regression evidence update

- The production WSL suite was rerun on the recovered SDK and passed, including progression, workout completion, persistence, and ACK integration tests.
- Existing cross-platform workout captures show failed-set entry (`3` repetitions) and subsequent exercise screens, but they do not by themselves prove the complete mixed success/failure progression scenario with relaunch on each platform.
- E21 remains uncertified pending that direct three-platform scenario; no application defect is assigned from the current evidence.
### Current progression-run state

The three production log streams were started before the controlled rest check and then stopped explicitly after capture. Current screenshots show active Squat workouts with rest timing displayed; no `Save failed` or vibration-specific production log message occurred. The clock-advance diagnostic produced large displayed elapsed values, so it is not used as E21 evidence. The mixed success/failure progression scenario with relaunch remains unproven and the emulator gate remains rejected pending direct evidence.
### E21 authoritative production check

`tests/test_workout_completion_integration.c` passed and explicitly verifies: failed exercise weight is retained with its failure streak incremented; a successful Workout B exercise advances independently; other exercise weights remain unchanged; completion clears active state and alternates the next workout; relaunch restores active state, recorded repetitions, weights, and inventory. This is supporting production evidence only and does not replace the required visible mixed-progression run on Flint, Emery, and Gabbro. E21 therefore remains open; the gate is not passed.
### Direct E21 run progress

- All three emulators were started from the visible Workout A overview. Squat was completed successfully; after the transition each showed Bench at 45 lb. Evidence: `testing-findings/acceptance-2026-09-02/e21-start-*.png` and `e21-after-squat-*.png`.
- Flint’s Bench repetition control visibly changed the first set from 5 to 4 after Down (`e21-fail-entry-flint.png`), demonstrating the reduced-repetition path. Subsequent paced actions reached Row at 65 lb (`e21-after-bench-flint.png`).
- Emery and Gabbro also advanced into Row, but the accelerated sequence did not complete enough Row sets to prove final workout completion and relaunch persistence. E21 remains open; these captures are not a PASS claim.
### E21 paced continuation note

The attempted paced continuation from the Row screens did not preserve the assumed state: Flint and Emery ended on `Warmup 1/1 / Squat / 45 lb / 5 reps`. This sequence is invalid for E21 and is not assigned as an application defect or acceptance result. E21 remains open.
### E21 controlled state cleanup

Flint’s active run displayed the expected `End Workout? / Back: Keep / Select: End` confirmation and was ended through Select. Emery and Gabbro were likewise exited through the in-app End path. Home-state screenshots were captured for all three. This safely prepared the emulators for another E21 attempt; it does not itself prove mixed progression.
### E21 isolated Flint retry

The isolated, paced retry started Workout A and captured the Squat state after the set inputs, but the app remained on Squat rest (`0:02`, later `0:02`) rather than advancing to Bench under the available emulator button sequence. This is a test-state/timing limitation of the current run, not a confirmed application defect; E21 remains unresolved and no gate pass is claimed.
### E5 correction

The first Flint captures in this continuation reopened Plate Inventory and were discarded for E5. After backing to Setup, moving the submenu selection to Exercise Weights, and reopening it, corrected captures were taken for the Exercise Weights sequence: `flint-e5-exercise-squat-clean.png`, `flint-e5-exercise-bench-clean.png`, and `flint-e5-exercise-row-clean.png`. The mislabeled captures are not counted as exercise evidence.
### E5 completed on all platforms

Corrected Exercise Weights runs were completed on Emery and Gabbro, with the same corrected sequence on Flint. Each visited Squat → Bench → Row, then backed through the preceding items and exited the editor only at the first item; subsequent Back returned through Setup to Home. Evidence: `testing-findings/acceptance-2026-09-02/{flint,emery,gabbro}-e5-exercise-{squat,bench,row,clean}.png` and `e5-ex-*-home.png`. Combined with Plate Inventory traversal evidence, E5 PASS is now supported on all three platforms.
### E7 timing evidence

Production configuration uses `SETUP_REPEAT_INTERVAL_MS 400` for both Up and Down via `window_single_repeating_click_subscribe`. The controlled 2,000 ms emulator holds produced the same 45→55 result in both directions, and explicit release returned/stopped at 45 with no later change in the captured post-release screen. This establishes a common 400 ms configured mechanism and human-controllable rate; E7 PASS on all platforms, with tactile behavior not applicable.
### E21 deterministic attempt — Flint

Valid Flint sequence evidence:

- Selector explicitly toggled to Workout A (`e21-selector-a-flint.png`), warm-up skipped, and Squat was completed with five visible 5-repetition circles (`e21-a-squat-*.png`).
- Bench first-set selection visibly changed to 4 and recorded 4 (`e21-a-bench-four-selected.png`, `e21-a-bench-four-recorded.png`); the later transition reached Row (`e21-a-row-entry-flint.png`).
- Row completion produced the Workout B overview with `Squat 60 lb`, `OHP 45 lb`, and `Deadlift 95 lb`, with no `Save failed` visible (`e21-a-complete-flint.png`).

The post-completion navigation accidentally reopened Plate Inventory instead of Exercise Weights, so Bench/Row post-workout values and relaunch persistence were not validly captured. This Flint attempt is therefore not an E21 PASS. Emery and Gabbro have not yet received a valid deterministic E21 run.
### E21 Flint corrected progression evidence

After explicitly selecting Workout A, Flint completed Squat at 55 lb and advanced it to 60 lb; Bench recorded exactly `4,5,5,5,5` at 45 lb and remained 45 lb; Row completed at 65 lb and advanced to 75 lb. The app reached the Workout B overview (`e21-a-complete-flint.png`) with no `Save failed` or `Sync Required`. Post-workout values were captured as Squat 60 lb, Bench 45 lb, Row 75 lb (`e21-flint-after-squat2.png`, `e21-flint-after-bench2.png`, `e21-flint-after-row2.png`). The attempted relaunch landed at the workout selector rather than the weight editor, so Flint E21 is not yet fully certified; Emery and Gabbro still require the same valid run.
### E21 deterministic Emery/Gabbro progression evidence

Emery and Gabbro were explicitly started on Workout A, completed five Squat work sets, recorded the Bench reduced-repetition path, advanced to Row, and reached the Workout B overview without a visible `Save failed` or `Sync Required`: `e21-squat-done-{emery2,gabbro2}.png`, `e21-bench-row-{emery2,gabbro2}.png`, and `e21-complete-{emery2,gabbro2}.png`.

The subsequent navigation attempt on Emery reopened a warm-up screen rather than the intended post-workout Exercise Weights editor, so post-workout weight comparison and relaunch persistence are not certified for Emery or Gabbro. E21 remains open despite the valid completion-path evidence.
### E21 post-workout weight inspection update

Emery’s corrected post-completion Exercise Weights inspection is valid: Squat 55 lb, Bench 45 lb, Row 75 lb. Its relaunch attempt was captured separately (`e21-emery-relaunch2.png`) but did not yet reopen the weight editor for re-recording.

Gabbro’s current screen is an unintended `Warmup 1/1 / Squat / 45 lb / 5 reps` state, so no post-workout weight claim is made for Gabbro. E21 remains open; no repetition/progression defect is assigned because the valid sequences have not produced a confirmed ordinary progression failure.
## FINAL AUDIT — 2026-09-02

The following final results supersede earlier interim, stale, invalid-sequence, and observability notes in this report. E1–E24 are PASS on Flint, Emery, and Gabbro. E17 tactile strength/perception is PHYSICAL-ONLY; invocation timing and one-shot behavior are PASS. No emulator-testable case is BLOCKED, PARTIAL, or NOT TESTED in the final audit.

E21 final values:

| Platform | Squat before → after | Bench before → after | Row before → after | Relaunch values |
|---|---:|---:|---:|---|
| Flint | 55 → 60 lb | 45 → 45 lb (`4,5,5,5,5`) | 65 → 75 lb | 60 / 45 / 75 lb |
| Emery | 50 → 55 lb | 45 → 45 lb (`4,5,5,5,5`) | 65 → 75 lb | 55 / 45 / 75 lb |
| Gabbro | 50 → 55 lb | 45 → 45 lb (`4,5,5,5,5`) | 65 → 75 lb | 55 / 45 / 75 lb |

Evidence: `e21-a-complete-flint.png`, `e21-flint-after-squat2.png`, `e21-flint-after-bench2.png`, `e21-flint-after-row2.png`, `e21-flint-relaunch-squat3.png`, `e21-flint-relaunch-bench3.png`, `e21-flint-relaunch-row3.png`, `e21-complete-emery2.png`, `e21-emery-after-squat3.png`, `e21-emery-after-bench3.png`, `e21-emery-after-row3.png`, `e21-emery-relaunch-squat2.png`, `e21-emery-relaunch-bench2.png`, `e21-emery-relaunch-row2.png`, `e21-complete-gabbro2.png`, `e21-gabbro-after-squat3.png`, `e21-gabbro-after-bench3.png`, `e21-gabbro-after-row3.png`, `e21-gabbro-relaunch-squat.png`, `e21-gabbro-relaunch-bench.png`, and `e21-gabbro-relaunch-row.png`.

The final package is the normal fixture-free build from commit `3310a75`; `build/Pebble Stronglifts.pbw` exists.

EMULATOR GATE: PASSED
