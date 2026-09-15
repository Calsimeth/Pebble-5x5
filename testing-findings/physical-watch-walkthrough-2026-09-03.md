# StrongLifts Physical-Watch Walkthrough — 2026-09-03

Build: `12a5c65b744567dc47565675fd64b7f16e17689e` (normal fixture-free)
Developer Connection: `192.168.8.120` (reported live)
Watch model: ____________________  Firmware: ____________________
Android phone model: ____________________  Pebble app version: ____________________
Phone connected during test: Yes / No  Date/time: ____________________

Installation evidence: commit `12a5c65b744567dc47565675fd64b7f16e17689e`

```text
Installing app...
App install succeeded.
```

Walkthrough rule: complete every action below in order, record the exact visible result immediately, and use the cleanup instruction before continuing. Do not substitute emulator behavior for a watch observation.

Use the exact sequence below. Complete one test, record the observation, then continue. Record exact visible text, behavior, connection state, and PASS/FAIL. Take a watch photograph where practical. Do not infer from emulator results.

## Launch and setup

### 1. Launch

Action: From the Pebble launcher, select StrongLifts and press Select once.

Expected: StrongLifts Home appears with `New Workout` selected.

Result: PASS

### 2. Separate setup editors

Action: From Home, move to `Setup`, press Select, open `Exercise Weights`, press Back to Setup, then open `Plate Inventory`.

Expected: Each editor opens separately with its own title and items.

Result: PASS

### 3. Setup Back traversal

Action: In Exercise Weights, advance at least three items; press Back once per item back to the first item, then Back to Setup and Back to Home. Repeat in Plate Inventory.

Expected: Back traverses preceding items; the editor exits only from its first item; Setup returns to Home.

Result: PASS

### 4. Plate defaults

Action: Inspect every Plate Inventory item without changing values.

Expected per side: `45: 2`, `35: 0`, `25: 1`, `15: 0`, `10: 1`, `5: 1`, `2.5: 1`.

Result: PASS

### 5. Weight adjustment

Action: Note the current weight. Short-press Up once. Then hold Up for about two seconds and release. Repeat with Down for about two seconds and release.

Expected: Short Up makes one increment; both held directions repeat at a controllable comparable rate; release stops repetition; no extra short-click adjustment occurs.

Result: PASS

## Workout and persistence

### 6. Workout selection

Action: Home → `New Workout` → select Workout A and inspect its overview. Return to Home, repeat, and select Workout B.

Expected: The selected workout shows the correct exercise list.

Result: PASS

### 7. Active workout summary and circles

Action: Start either workout and complete one set with five repetitions.

Expected: Exercise summary and set circles are visible; the completed circle turns red.

Result: PASS

### 8. Failed set

Action: On a work set, choose fewer than five repetitions and submit it.

Expected: The selected count is recorded and the failed circle is visibly distinct/red where supported.

Result: PASS

### 9. Rest timer and vibrations

Action: Complete a set and remain on Rest beyond three minutes; note the notification around 1:30 and at 3:00, then trigger a later rest.

Expected: Rest counts above `3:00`; halfway and three-minute notifications are distinguishable; the three-minute notification does not repeat; a later rest can notify again.

Result: PASS

### 10. Active workout exit and Continue

Action: Start a workout, record at least one set, and press Back once.

Expected: In-app Home appears with `Continue` (not the watch clock).

Result: PASS

### 11. Continue restoration

Action: Select `Continue`.

Expected: The identical workout returns with the same exercise, set circles, repetitions, and working weight.

Result: PASS

### 12. Deliberate abandonment

Action: From the active workout, select `End Workout`, then select `End` on `End Workout?`.

Expected: Inactive Home appears with `New Workout`; `Continue` and `End Workout` are absent; no Workout A/B overview appears between confirmation and Home.

Observed text/behavior: "End Workout" is not an available option from within an active workout. 

Result: FAIL

Note: If you meant that I should select "End Workout" from the home page, then the app behaves as expected and the exercise is abandoned.

### 13. Keep behavior

Action: Start another workout, record progress, open `End Workout`, then press Back for Keep.

Expected: The unchanged active workout returns directly.

Result: PASS

### 14. App exit and setup persistence

Action: From Home, press Back to exit to the watch clock. Reopen StrongLifts, change one exercise weight and one plate count, exit again, then reopen Setup.

Expected: Home remains reachable; both custom values persist.

Observed text/behavior/values: setup weight and plates do not persist after exiting the app  

Result: FAIL

### 15. Active-workout relaunch

Action: Start a workout, record a set, exit to the watch clock without ending it, and reopen StrongLifts.

Expected: Home shows `Continue`; Continue restores the active workout and recorded set.

Observed text/behavior: Active workout is gone.  Result: FAIL

## Completion, History, and Progress

### 16. Completion and synchronization

Action: Complete a workout while the phone is connected and wait for synchronization.

Expected: No `Save failed` or `Sync Required`; record completion and matching ACK are observable in the phone/Pebble environment if available.

Observed text/logs/record ID/ACK ID: ____________________  Phone connected: ____  Result: PASS / FAIL

### 17. History date

Action: From inactive Home, open `History` without starting a workout; wait for loading and navigate to the completed workout’s month.

Expected: The completed workout date is visibly marked.

Observed month/date/text: ____________________  Phone connected: ____  Result: PASS / FAIL

### 18. Progress recorded weight

Action: From inactive Home, open `Progress`, select the exercise represented by the completed workout, and wait.

Expected: Loading resolves to a graph/result showing the exercise, recorded weight, date information, unit, and page indicator; it does not show `Phone Needed` or `No Progress` when a record exists.

Observed text/graph/weight: ____________________  Phone connected: ____  Result: PASS / FAIL

### 19. Mixed progression

Action: Record starting Squat, Bench, and Row weights. In Workout A, complete Squat and Row successfully; record one Bench set below five repetitions and complete the remaining Bench sets. Inspect Setup after completion, then exit/reopen and inspect again.

Expected: Squat and Row advance independently; Bench repeats its starting weight; all resulting values persist after relaunch.

Starting values: ____________________  After completion: ____________________  After relaunch: ____________________

Observed text/behavior: ____________________  Phone connected: ____  Result: PASS / FAIL

## Final summary

Failures by subject:

- Workout/UI: ____________________
- Rest/vibration: ____________________
- Setup: ____________________
- Persistence/synchronization: ____________________
- History/Progress: ____________________

Overall physical acceptance: PASS / FAIL
