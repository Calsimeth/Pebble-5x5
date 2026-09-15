# Physical-Watch Acceptance Report

Build: commit `3310a75` (normal fixture-free package)
Watch model: Not recorded
Phone connected: Not recorded
Overall result: FAIL

## Results

| Test | Result | Recorded observation |
|---|---|---|
| P1 Launches to Home | PASS | Home launched with `New Workout`. |
| P2 Separate Setup editors | PASS | Exercise Weights and Plate Inventory opened separately. |
| P3 Setup Back traversal | PASS | Setup Back traversal passed. |
| P4 Plate defaults | PASS | Plate defaults passed. |
| P5 Hold-to-repeat | FAIL | Up and Down repeated, but weight jumped from 80 to 95 instead of using 5 lb increments. |
| P6 Workout selection | FAIL | Back/enter behavior was perceived as reversed: backing out proceeded with the exercise, while entering canceled the workout. |
| P7 Set circles/transition | FAIL | After the fifth set, the next exercise began automatically; expected an additional Enter action. |
| P8 Failed set repetitions | PASS | Fewer-than-five repetitions recorded correctly. |
| P9 Back confirmation/navigation | FAIL | Expected exercise-by-exercise Back traversal and context-sensitive Enter behavior were not available. |
| P10 Rest timer | PASS | Rest counted beyond three minutes. |
| P11 Rest vibrations | PASS | Halfway and three-minute vibrations were reported distinguishable. |
| P12 Active workout relaunch | FAIL | No way to exit the app was found on the watch. |
| P13 Synchronization | FAIL | Exercise completed, but instructions were unclear and no synchronization result was observed. |
| P14 History date | FAIL | History selection did nothing; selecting again started a workout. |
| P15 Progress recorded weight | FAIL | Progress remained on `Loading`; selecting again started a workout. |
| P16 Weight progression | PASS | Successful/failed progression behavior passed during the workout. |
| P16 Relaunch persistence | FAIL | After exiting and re-entering, weight values reset to their original values. |

## Failures by subject

### Workout/UI

- P5: weight adjustment jumped from 80 to 95 rather than using 5 lb increments.
- P6: workout selection/back/enter behavior was observed as reversed.
- P7: the next exercise began immediately after the fifth set instead of waiting for Enter.
- P9: expected exercise-by-exercise Back traversal and reverse Enter behavior were not available.

### Rest/vibration

- No failures recorded. Tactile perception was reported as distinguishable on the physical watch.

### Setup

- No failures recorded for P2–P4.

### Persistence/synchronization

- P12: no app-exit path was found, preventing the relaunch check.
- P13: no synchronization result was observed.
- P16 relaunch: weight values reset after re-entry.

### History/Progress

- P14: History did not open reliably and could start a workout.
- P15: Progress remained on `Loading` and could start a workout on another selection.

No watch model, phone-connection state, screenshots, or exact displayed synchronization text were recorded in the walkthrough; those fields remain intentionally unfilled.

## Physical retest preparation — 2026-09-03

The exact current fixture-free package from commit `12a5c65b744567dc47565675fd64b7f16e17689e` was installed using the WSL procedure in `AGENTS.md`. WSL preflight passed (`Default Version: 2`, Ubuntu version 2, `WSL_OK`). Direct installation to `192.168.8.129` returned `[Errno 113] No route to host`; the documented temporary bridge used Wi-Fi `192.168.8.182` and WSL gateway `172.30.32.1`, with phone port 9000 confirmed open. Bridged installation output was:

```
Installing app...
App install succeeded.
```

The bridge reported `RELAY_READY` and `RELAY_CONNECTED`, then exited; no listener remained on port 9000. Physical retesting is now pending one-action-at-a-time observations. Watch model, firmware, phone model, Pebble app version, connection status, and test date/time remain to be recorded from the physical setup.

## Final-build installation — 2026-09-03

The final authoritative emulator-passed build, commit `12a5c65b744567dc47565675fd64b7f16e17689e`, was installed via WSL to the Android Pebble Developer Connection at `192.168.8.129`. WSL preflight passed (`Default Version: 2`, Ubuntu version 2, `WSL_OK`). Installation output:

```
Installing app...
App install succeeded.
```

Physical watch model, firmware, phone model, Pebble app version, phone connection state, and exact walkthrough time are still awaiting observation from the physical setup.

## P5 physical-install attempt — 2026-09-09

Build: fixture-free commit `201e7d88a70c1d3a06c40eccfeb248f5e0fd045e`.

WSL2 preflight and build passed. Direct WSL installation to `192.168.8.129` returned:

```text
[Errno 113] No route to host
```

Following `AGENTS.md`, Windows Wi-Fi `192.168.8.182` and WSL gateway `172.30.32.1` were determined. A Windows `TcpClient` explicitly bound to Wi-Fi `192.168.8.182` failed to connect to `192.168.8.129:9000` (connection timeout). Because the phone port was not reachable, the documented bridge was not started. No physical installation succeeded, no watch metadata was observed, and focused P5 physical acceptance has not begun.

## 2026-09-09 installation revalidation

For commit `12a5c65b744567dc47565675fd64b7f16e17689e`, the required WSL2 preflight passed, the complete WSL test suite passed, and the normal fixture-free build completed successfully. A new direct install attempt returned `[Errno 113] No route to host`. The Wi-Fi-bound check from `192.168.8.182` to `192.168.8.129:9000` timed out, so the bridge was not started and no new installation is claimed from this attempt. The earlier successful installation remains recorded above; physical testing requires a reachable Developer Connection before proceeding.

## 2026-09-09 address update

Caleb reported the active Developer Connection at `192.168.8.120`. Using the exact fixture-free commit `12a5c65b744567dc47565675fd64b7f16e17689e`, WSL preflight passed and direct installation to `192.168.8.120` succeeded:

```
Installing app...
App install succeeded.
```
