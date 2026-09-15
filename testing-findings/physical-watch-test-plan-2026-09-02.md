# StrongLifts Physical-Watch Acceptance Walkthrough

Build under test: commit `3310a75` (normal fixture-free build)
Watch: ____________________
Phone connected: Yes / No
Date/time: ____________________

Record the exact displayed text and result for every case. Do not infer a result from emulator behavior. Perform one numbered test at a time. If a test changes workout state, use the specified cleanup before continuing.

## Home and navigation

### P1 — Launches to Home

Action: From the Pebble launcher, open StrongLifts.

Expected: Home appears with `New Workout` selected.

Result: PASS

### P2 — Setup opens separate editors

Action: Home → `Setup` → Select. Open `Exercise Weights`; press Back to Setup. Open `Plate Inventory`.

Expected: The two editors open separately and show their own items.

Result: PASS

### P3 — Setup Back traversal

Action: In Exercise Weights, advance at least three items. Press Back once per item until the first item, then press Back to Setup and Back to Home. Repeat for Plate Inventory.

Expected: Each Back moves to the preceding item; the editor exits only from its first item; Setup returns to Home.

Result: PASS

### P4 — Plate defaults

Action: In Plate Inventory, inspect every item without changing values.

Expected per side: `45: 2`, `35: 0`, `25: 1`, `15: 0`, `10: 1`, `5: 1`, `2.5: 1`.

Result: PASS

### P5 — Weight hold-to-repeat

Action: In Exercise Weights, note the initial value. Hold Up for exactly 2 seconds, release, and observe for at least one further repeat interval. Return to the initial value if needed. Repeat with Down for exactly 2 seconds.

Expected: Both directions repeat at the same human-controllable rate; release stops changes; no extra short-click adjustment occurs.

Observed behavior: Holding up and down continues adjusting the weight; however, weight adjusts directly from 80 to 95 which violates the 5 lb increments.

Result: FAIL

## Workout behavior

### P6 — Workout selection

Action: Home → `New Workout` → choose Workout A. Back out if necessary, then repeat and choose Workout B.

Expected: Each selected workout opens with the correct exercise list.

Observed text/behavior: Backing out proceeds with the exercise and entering cancels the workout. The intuative behavior would be the opposite. So, backing out should cancel the workout and entering would be the opposite behavior.

Result: FAIL

### P7 — Active workout summary and circles

Action: Start a workout. Complete one set with five repetitions.

Expected: Exercise summary and set circles are visible; the completed circle turns red.

Observed text/behavior: After completing the fifth set, the next exercise begins automatically. Desired behavior is that the next exercise should begin after hitting enter again.

Result: FAIL

### P8 — Failed set with fewer than five reps

Action: On a set, select 3 or 4 repetitions and submit it.

Expected: The set records the selected value below five and displays as failed.

Result: PASS

### P9 — Back confirmation, Keep, and End

Action: During an active workout, press Back once.

Expected: `End Workout?` with `Back: Keep` and `Select: End`.

Action: Press Back. Verify the identical workout state returns. Press Back again, then Select.

Expected: The workout is deliberately abandoned and Home shows `New Workout`.

Observed text/behavior: Hitting back during a workout should return to the previous exercise until the first exercise where doing so again should prompt for ending abandoning the workout. If the user presses enter and has paged back in the exercises then pressing enter should reverse the behavior and proceed to the next exercise (which may be a complete exercise) this behavior should continue until the user returns to the active exercise in the workout where the button behavior should resume controlling set outcome (enter confirms next set is complete and up down controls successful reps.)

Result: FAIL

### P10 — Rest timer

Action: Complete a set and remain on the rest screen for more than three minutes.

Expected: Rest counts upward beyond `3:00`.

Result: PASS

### P11 — Rest vibrations

Action: During rest, note the vibration around 1:30 and the vibration at 3:00. Continue beyond 3:00 and begin a later rest.

Expected: Halfway and three-minute notifications are distinguishable; the three-minute notification does not repeat; a later rest can notify again.

Result: PASS

### P12 — Leave and reopen active workout

Action: Start a workout and record at least one set. Exit the app without ending the workout. Reopen StrongLifts.

Expected: Home shows `Continue`; selecting it restores the workout, recorded sets, and repetitions.

Observed text/behavior: The pebble device does not present a way to exit the app.

Result: FAIL

## History and Progress

### P13 — Complete a workout and synchronize

Action: Complete a workout. Wait for phone synchronization while the phone is connected.

Observed text/behavior: Exercise was completed. Instructions were unclear. Nothing was observed.

Result: FAIL

### P14 — History date

Action: Open History and navigate to the month containing the completed workout.

Expected: The workout date is visibly marked.

Observed month/date/text: Selecting history does not do anything. Selecting again starts a workout. These are two severe bugs.

Result: FAIL

### P15 — Progress recorded weight

Action: Open Progress, select the exercise represented in the completed workout, and wait for the response.

Expected: A graph or progress result shows the exercise and its recorded weight/date information; it does not remain on `Loading`, `Phone Needed`, or `No Progress` when a record exists.

Observed text/graph/weight: Progress gets stuck on "Loading". Selecting again starts a workout when it should not.

Result: FAIL

## Optional progression check

### P16 — Successful and failed weight progression

Action: Record Squat, Bench, and Row weights. In Workout A, complete Squat successfully, record Bench with one set below five reps and the remaining sets at five, and complete Row successfully. Do not start the proposed Workout B. Open Setup → Exercise Weights.

Expected: Squat and Row advance to their next achievable weights; Bench remains unchanged; the three results are independent.

Result: PASS

Action: Exit and reopen the app without reinstalling. Recheck all three weights.

Expected: All post-workout values persist unchanged.

Observed values: After having completed a workout and exited the app, when I re-enter the app and start a new workout, my weight values have reset to their original values.

Result: FAIL

## Cleanup and summary

Overall physical acceptance: FAIL
