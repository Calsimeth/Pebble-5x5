# Targeted emulator follow-up — 2026-09-14

## Scope and identity

This is a read-only follow-up to `emulator-current-audit-2026-09-14.md`. The checkout HEAD at inspection was `02cfccaacc6f7034c088a8423890a80596e528e1`; the prior audit's application checkout was `be5ef21830b5c3216d4525d1a235289cf7abd97`. Existing dirty and untracked files were preserved. No production source was modified.

Required WSL2 preflight passed: default version 2, Ubuntu running, and exact `WSL_OK` output.

The fixture-free host test suite passed. `pebble build` passed. The resulting bundle SHA-256 was:

`640319e8b7eeb6e39257f389feefd39c0079e380563c3d6d0fe3e4c40d3c6f61`

Separate installs of this build passed on Flint, Emery, and Gabbro. Each reported `Installing app...` followed by `App install succeeded.`

## Targeted runtime verdict

The Pebble CLI was callable inside Ubuntu WSL. `pebble emu-button --help` confirmed `click`, `push`, and `release` for `back`, `up`, `select`, and `down`; `pebble screenshot --help` confirmed emulator capture. Fresh screenshots were captured for all three installed emulators: `followup-flint-current.png`, `followup-emery-current.png`, and `followup-gabbro-current.png`. Flint showed inactive Home (`New Workout`, `Setup`, `History`, `Progress`); Emery and Gabbro showed retained active sessions with `Continue` and `End Workout`.

Flint was exercised from its actual inactive Home checkpoint: New Workout → Workout A → Squat. The final Squat set produced a fresh `Next Exercise / Select to begin / Bench` screen (`accept-flint-next.png`). Select then opened `Bench 5x5` (`accept-flint-bench.png`), proving the first boundary and Select-to-begin behavior. Earlier fresh captures show all five Squat circles completed (`followup-flint-handoff-final2.png`), Back returning Home with `Continue` (`followup-flint-back.png`), and Select restoring active Squat (`followup-flint-continue.png`). The log command ran but emitted no lines, so no transport correlation is claimed. The second exercise boundary, paused relaunch, and Workout B Deadlift were not reached in this follow-up. Those remaining items are **UNVERIFIED for this exact checkout/build run**.

The retained evidence files in the dirty workspace include prior current-build-style handoff and relaunch captures, plus Progress captures, but their capture provenance cannot be re-established against the freshly built bundle in this worker. They are retained evidence only and are not promoted to fresh-run proof.

History/Progress transport/render correlation is also **UNVERIFIED** for this run: no fresh request ID, phone-side response/send callback, watch accepted/rejected response, and rendered result chain could be observed. A fixture-only result was not used.

## Defect classification

No new defect can be assigned from this follow-up. The prior retained P5 timing finding is not a confirmed failure: later emulator evidence reclassified held-weight behavior as PASS because QEMU time is accelerated, and physical acceptance on 2026-09-14 recorded weight hold as PASS with only a delay/usability note. This follow-up does not reopen P5.

| Platform | Fresh build/install | Targeted handoff/relaunch | Fresh History/Progress transport/render | Overall |
| --- | --- | --- | --- | --- |
| Flint | PASS | UNVERIFIED | UNVERIFIED | UNVERIFIED |
| Emery | PASS | UNVERIFIED | UNVERIFIED | UNVERIFIED |
| Gabbro | PASS | UNVERIFIED | UNVERIFIED | UNVERIFIED |

**EMULATOR GATE: UNVERIFIED / NOT PASSED.**

## Continuation evidence — CLI-driven runtime pass

The previously stated “unavailable emulator controls” limitation is superseded. The WSL Pebble CLI exposed and successfully executed `emu-button`, `emu-set-time`, `screenshot`, `install`, and `logs`.

### Handoff, relaunch, and final exercise

- Flint: Squat’s first boundary displayed `Next Exercise / Select to begin / Bench` (`accept-flint-next.png`); Select opened Bench (`accept-flint-bench.png`). Bench’s final boundary displayed `Next Exercise / Select to begin / Row` (`accept-flint-second-handoff.png`); Back returned Home with `Continue` (`accept-flint-handoff-home.png`), reinstall/relaunch preserved `Continue` (`accept-flint-relaunch.png`), and Select restored the Row handoff (`accept-flint-relaunch-restored.png`). Row then completed to the Workout B overview (`accept-flint-final3.png`). Result: **PASS** for both handoffs, Select-to-begin, paused Back/Continue/relaunch restoration, and final-exercise no-handoff.
- Emery: retained Squat/Bench state was restored, warmup was skipped only where the UI explicitly said `Dn:skip`, and Bench’s final boundary displayed `Next Exercise / Select to begin / Row` (`accept-emery-next.png`). Back/Home, reinstall/relaunch, and Select restored the same Row handoff (`accept-emery-home.png`, `accept-emery-relaunch-home.png`, `accept-emery-relaunch-restored.png`). Row completed to Workout B (`accept-emery-final2.png`). Result: **PASS** for the observed handoff/relaunch/final-exercise paths.
- Gabbro: Bench’s final boundary displayed `Next Exercise / Select to begin / Row` (`accept-gabbro-next.png`). Back returned Home (`accept-gabbro-home.png`); after the app exit/relaunch sequence, `accept-gabbro-relaunch-home.png` showed Home with `Continue`, and Select restored `Next Exercise / Select to begin / Row` (`accept-gabbro-relaunch-home2.png`, corroborated by `accept-gabbro-relaunch-restored.png`). The retained Row path completed to Workout B (`accept-gabbro-final2.png`). Result: **PASS** for handoff, paused relaunch restoration, and final exercise.

### History/Progress transport and rendering

- Flint History: `accept-flint-history2.log` correlates `CALENDAR_REQUEST id=1`, phone-side request/records/response, watch `CALENDAR_ACCEPTED expected_id=1`, and `CALENDAR_RENDER populated`; `accept-flint-history-result2.png` is the corresponding rendered result.
- Flint Progress: `accept-flint-progress2.log` correlates request `id=2`, phone response, watch `PROGRESS_ACCEPTED id=2`, `PROGRESS_RENDER graph points=3`, and `PROGRESS_SEND_OK`; `accept-flint-progress-result2.png` renders the graph. Result: **PASS**.
- Emery Progress: `accept-emery-progress2.log` correlates request `id=1`, phone response, watch acceptance, graph render with five points, and phone send success; `accept-emery-progress-result2.png` renders the graph. Result: **PASS**.
- Gabbro Progress: `accept-gabbro-progress2.log` correlates request `id=1`, two phone response chunks, acceptance of chunk 0, then `PROGRESS_SEND_FAILED id=1 chunk=2`; `accept-gabbro-progress-result2.png` renders `No Progress` despite the response containing six points. Result: **FAIL**, first confirmed current-build defect is the multi-chunk Progress transport/render path on Gabbro.

### Revised platform verdict

| Platform | Handoffs / Select | Back/Continue + relaunch | Final exercise | History/Progress correlation | Verdict |
| --- | --- | --- | --- | --- | --- |
| Flint | PASS | PASS | PASS | PASS | PASS |
| Emery | PASS | PASS | PASS | PASS | PASS |
| Gabbro | PASS | PASS | PASS | FAIL | FAIL |

**EMULATOR GATE: FAIL.** The Gabbro multi-chunk Progress failure is reproducible in the current installed build and blocks a full gate pass. No physical watch was used and no production source was modified.

## Slice 12 post-remediation verification — commit f006cd6

Retest identity: `f006cd6ce92bdace8634fdd52078b61398ebe319`; fixture-free bundle SHA-256 `c9894dd2bc5a7379b5b59b68a8e4962565c3b546b88a98a86b949c7e0025a3c1`. WSL2 preflight and tests passed. Separate Flint, Emery, and Gabbro installs each reported `Installing app...` and `App install succeeded.`

- Flint: `s12-f006-flint.log` shows request `id=1`, phone response, `PROGRESS_ACCEPTED id=1`, `PROGRESS_RENDER graph points=3`, and `PROGRESS_SEND_OK`; `s12-f006-flint-result2.png` renders the graph. **PASS**.
- Emery: `s12-f006-emery.log` shows request `id=1`, response chunks `0/2` (5 points) and `1/2` (1 point), acceptance of both, `PROGRESS_RENDER graph points=6`, and send success for both; `s12-f006-emery-result2.png` renders the graph. **PASS**.
- Gabbro: `s12-f006-gabbro.log` shows request `id=1`, response chunks `0/2` (5 points) and `1/2` (2 points), acceptance of both, `PROGRESS_RENDER graph points=7`, and send success for both; `s12-f006-gabbro-result2.png` renders the graph. **PASS**.

These were normal emulator-data records, not fixtures. The earlier pre-remediation Gabbro send failure is superseded for f006cd6. **SLICE 12 PROGRESS GATE: PASS.** Combined with the corrected relaunch evidence, the current emulator acceptance gate is **PASS** for the tested scope. No physical install was performed; unrelated dirty files were preserved.

## Gabbro relaunch evidence audit correction

The earlier relaunch-failure wording was an evidence-labeling error. Direct inspection of the exact tracked PNGs shows `accept-gabbro-relaunch-home.png` is Home with `Continue`, while `accept-gabbro-relaunch-home2.png` and `accept-gabbro-relaunch-restored.png` both show `Next Exercise / Select to begin / Row`. The capture sequence was: exit from the handoff to Home, relaunch/install, capture Home with Continue, Select, then capture the restored handoff. No separate blank relaunch capture was found under these filenames, and no app root cause is asserted. The Gabbro relaunch classification is therefore corrected to **PASS**. The independently observed Gabbro Progress failure and closed emulator gate are unchanged.
