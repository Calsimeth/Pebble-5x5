# Pebble Time 2: self-guided acceptance checklist

Use this whenever convenient. No live conversation or log session is required. This is for the release app already installed after the Slice 9 persistence fix. The weight edit → exit → relaunch check passed on September 14 and need not be repeated unless it fails again.

Date/time: __________  App name shown: __________  Phone connected to Pebble: Yes / No / Unsure

For each check, mark **Pass / Fail / Not tested** and write what the watch actually showed. A screenshot or brief note is enough; do not try to reconstruct an exact button sequence afterward. Stop if continuing could create an unwanted workout record.

## Safe checks (no workout completion required)

1. **Home and entry.** Open the app from the launcher. Note whether Home shows `New Workout` or `Continue`. Open Setup and return Home with Back. Result: ______  Text/state: ____________________
2. **Weight adjustment feel.** In Setup → Exercise Weights, record the starting Squat value: ____ lb. Tap Up once (expected +5 lb): ____ lb. Hold Up briefly and release; note whether changes repeat at a controllable speed and stop on release: ____________________. If desired, use Down to restore the original value. Result: ______. This changes saved setup weights; restore the starting value if you do not want the edit retained.
3. **Workout choice.** Only if Home says `New Workout`: open it, switch between A and B, and confirm the intended three exercises before starting. Back out without starting if you do not want a session. Result: ______  Exercises shown: ____________________
4. **Continue and Back.** Only if a workout is already active: enter it via `Continue`, note the exercise and completed circles, press Back once, and confirm Home still offers `Continue`. Re-enter and check the same state remains. Result: ______  Before/after: ____________________. Do **not** choose `End Workout` unless you want to abandon the session.

## Workout checks (use during a real workout)

These actions change workout history and progression. Prefer your next actual workout; do not start a throwaway session. Record whether it is A or B: ____  Date: ____  Starting exercise/weight: __________.

5. **Set circles.** Complete a normal working set. Expected: its circle becomes red and shows the recorded reps. Result: ______  Observed: ____________________
6. **Failed set.** If a set genuinely falls short, select the actual reps (for example 4) and record it. Expected: that circle shows the lower number, and the failed exercise does not automatically increase its next target weight. Skip this check rather than deliberately failing a set. Result: ______  Reps/text: ____________________
7. **Rest.** After a set, leave the rest timer running. Note the roughly halfway buzz at ____ and the three-minute buzz at ____. Expected: the three-minute alert is more noticeable, and the timer continues upward past `3:00` until the next set is recorded. Result: ______  Timer/vibration notes: ____________________
8. **Final set and transition.** On the last set of an exercise, watch the final circle immediately after recording. Expected: it fills before the app advances; the next exercise is reached without losing the completed set. Result: ______  Observed: ____________________
9. **Workout completion.** After the final exercise (Deadlift is `1×5` in Workout B), note the completion/next-workout screen. Expected: no `Save failed` or `Sync Required`; the completed workout is no longer offered as `Continue`. Result: ______  Exact text: ____________________

## Phone-backed history (after a real completed workout)

Keep the Android Pebble connection active. Allow a moment for sync; no separate phone app or manual export is needed. If the phone is disconnected, mark these checks **Not tested**, not Fail.

10. **History.** From Home, open History. Expected: a calendar opens without starting a workout, and the completed workout date is marked. Result: ______  Month/date/text: ____________________
11. **Progress.** From Home, open Progress and choose an exercise from that completed workout. Expected: its graph appears rather than remaining on `Loading`, `Phone Needed`, or `No Progress`. Result: ______  Exercise/weight/graph/text: ____________________

## If something fails

Record the first failed check number: ____  Watch screen before: ____________________  Button/action: ____________________  Screen after/exact text: ____________________  Phone connected: Yes / No / Unsure. Stop dependent checks; keep the workout/data intact. Share this completed file or a photo of the relevant section when you are ready—there is no need to provide live screen-by-screen updates.
