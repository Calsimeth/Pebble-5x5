# Product requirements

## 1. Scope

The app must strictly support the standard StrongLifts 5x5 Workout A and Workout B structure.

| Workout A | Workout B |
| --- | --- |
| Squat: 5 sets x 5 reps | Squat: 5 sets x 5 reps |
| Bench press: 5 sets x 5 reps | Overhead press: 5 sets x 5 reps |
| Barbell row: 5 sets x 5 reps | Deadlift: 1 set x 5 reps |

The app must:

- Alternate Workout A and Workout B automatically.
- Preserve the exercise order shown above.
- Start every workout with squats.
- Default deadlifts to one work set of five repetitions.
- Track warm-up sets separately from work sets.
- Avoid becoming a general-purpose workout-program editor.

## 2. Supported hardware and phone environment

The target is the current Core Devices Pebble lineup, not historical Pebble watches:

- Pebble 2 Duo (`flint`)
- Pebble Time 2 (`emery`)
- Pebble Round 2 (`gabbro`)

The phone platform is Android.

The delivered product must not require a separately installed or sideloaded Android companion app. A bundled PebbleKit JS component running inside the official Pebble Android app is acceptable.

## 3. Workout flow

The watch must provide a linear workout experience:

1. Show the next Workout A or Workout B.
2. Show the current exercise and target working weight.
3. Offer the calculated warm-up sequence.
4. Record each warm-up or work set.
5. Start the rest timer after a logged set.
6. Move to the next exercise only after the current exercise is completed or explicitly skipped.
7. Save the completed workout and calculate future targets.

The user must be able to:

- Log a completed set with minimal interaction.
- Record fewer than the target repetitions.
- Skip a set or exercise deliberately.
- Undo an accidental entry.
- Override the working weight.
- End an incomplete workout without fabricating completed sets.
- Resume an interrupted workout after the app closes or the watch restarts.

## 4. Rest timer and feedback

The default work-set rest alert threshold is three minutes. Rest is an elapsed timer that starts at 0:00 after a qualifying work set and continues upward indefinitely; it does not automatically end at three minutes.

When vibration is available:

- At 1 minute 30 seconds, issue one short, subtle vibration.
- At 3 minutes, issue one unmistakable custom pattern of three long pulses separated by short pauses.
- Make the final notification more noticeable than the halfway notification.

The user must be able to start the next set while the elapsed timer is running. Up/Down must remain available to select 0–5 repetitions before Select records the set. Vibration must be configurable and must fail gracefully on hardware without vibration support.

Warm-up rest behavior is separate from work-set rest behavior. Its exact default remains a design decision.

## 5. Units, bar, and plate inventory

The default unit is pounds. Kilograms may optionally be supported.

The default Olympic bar weighs 45 lb. Bar weight should be configurable if kilogram support or alternate bars are added.

Plate inventory is specified per side. Loading must always be symmetrical; asymmetric loading is not allowed.

Default per-side plate sizes are:

- 45 lb
- 25 lb
- 10 lb
- 5 lb
- 2.5 lb

Optional per-side plate sizes are:

- 35 lb
- 15 lb

The calculator must not use optional plates unless the user explicitly enables them. Inventory quantities must be configurable, with no more than two of a given size per side assumed by default.

## 6. Plate calculation

For a requested total weight `T` and bar weight `B`, the required weight per side is:

```text
(T - B) / 2
```

The plate calculator must:

- Use only enabled plate sizes and available quantities.
- Produce symmetrical loading.
- Prefer an exact result.
- If exact loading is impossible, round down to the closest achievable total.
- Never silently exceed the requested weight.
- Minimize the total number of plates per side.
- Prefer heavier plates when configurations otherwise tie.
- Prefer fewer distinct plate sizes as a secondary tie-breaker.
- Show both total weight and plates per side.
- Explain when the requested total has been rounded down.

Examples using optional plates:

- 135 lb total: one 45 lb plate per side.
- 145 lb total: one 45 lb and one 5 lb plate per side, rather than two 25 lb plates.
- 210 lb total: one 45 lb, one 35 lb, and one 2.5 lb plate per side, only when the optional 35 lb plate is enabled.

## 7. Warm-up recommendations

Warm-up calculations begin from the 45 lb empty bar for squat, bench press, and overhead press. Rows and deadlifts may need a floor-appropriate starting load.

The default recommendation should be influenced by the number of 45 lb plates per side in the work-set configuration:

- No 45 lb plate per side: empty bar warm-up, then work weight.
- One 45 lb plate per side: empty bar warm-up, then work weight by default; an intermediate set may be suggested when the remaining load is substantial.
- Two or more 45 lb plates per side: empty bar plus progressive intermediate warm-up sets.

Intermediate targets must:

- Divide the distance between the starting load and work weight into sensible steps.
- Round down to an achievable load using the configured inventory.
- Never duplicate the work weight as a warm-up set.
- Increase monotonically.
- Remain editable or removable by the user.

The exact number of intermediate sets, target percentages, and warm-up repetition scheme remain open design decisions. The initial implementation should use a deterministic, documented rule rather than claim to reproduce a proprietary StrongLifts calculator.

## 8. Progression

Progression is calculated independently for each exercise.

- Completing every prescribed work-set repetition automatically increases that exercise's next target.
- A partial or failed work set repeats the same exercise weight next time.
- Skipping an exercise does not increase its weight.
- Success or failure on one exercise must not alter another exercise's progression.
- The user may override any automatically calculated weight.
- Manual overrides must be explicit and preserved in history.

Default increment sizes remain an open decision. Increment size should be configurable per exercise and constrained to weights achievable with the configured plate inventory.

## 9. Deload and plateau guidance

Deloads are recommendations and must never be forced or applied silently.

The app must suggest a deload when either condition applies:

- More than seven days have elapsed since the relevant prior workout date used by the final rule.
- The user has failed the same exercise repeatedly according to the configured failure threshold.

The StrongLifts default guidance should inform the initial repeated-failure behavior:

- Repeat a failed exercise weight.
- After three consecutive failed sessions for that exercise, suggest a deload of approximately 10%.
- Allow the user to accept, edit, postpone, or decline the suggestion.
- If the user repeatedly reaches and fails at the same weight after deloading, present a plateau advisory rather than endlessly suggesting identical deloads.

The plateau advisory may recommend reviewing:

- Sleep and recovery
- Nutrition
- Technique and range of motion
- Rest duration
- Smaller increments
- Less frequent progression
- StrongLifts top/back-off or intermediate guidance

The app must remain an A/B tracker and must not automatically switch to another program.

## 10. History and persistence

The watch must preserve enough local state to:

- Resume an in-progress workout.
- Know whether Workout A or Workout B is next.
- Calculate the next target for each exercise.
- Operate while disconnected from the phone.
- Retain a bounded amount of recent history where space permits.

The bundled PebbleKit JS component should aspire to maintain longer-term history in phone-side `localStorage` inside the official Pebble Android app.

Each historical exercise record should retain, where practical:

- Workout identifier and date/time
- Workout A or Workout B
- Exercise
- Target and actual working weight
- Target and actual repetitions per set
- Warm-up sets
- Completed, partial, failed, skipped, or abandoned status
- Rest timing where useful
- Manual overrides
- Deload recommendations and decisions
- Notes if notes are included in the eventual UI
- Data-schema and app version

Phone synchronization must tolerate disconnection, retries, duplicate messages, and app interruption. The watch should visibly indicate when completed data has not yet synchronized.

## 11. Import and export

The official StrongLifts CSV export is the only intended historical import format. Arbitrary third-party formats are out of scope; users with other data must transform it to match the StrongLifts format.

If import becomes feasible, it should:

- Detect columns by header rather than fixed position.
- Validate required fields.
- Preserve dates, exercises, weights, repetitions, and notes where available.
- Preview the import before committing.
- Avoid duplicate records.
- Require unit confirmation when units are missing or ambiguous.
- Keep real StrongLifts export fixtures for compatibility tests.

Because there is no separate Android companion app, import/export feasibility found no documented file picker, URI callback, file writer, or share-sheet API. Import and export are unsupported and are not release acceptance criteria. No compatibility is claimed without documented platform support and a representative legally usable fixture. See [the feasibility report](import-export-feasibility.md).

## 12. Watch interface and visual language

All watch-facing text must be no more than 20 characters. This is a hard content constraint, including spaces and punctuation.

Watch text must:

- Be clear without truncation.
- Use concise labels and familiar abbreviations.
- Be tested on rectangular and round screens.
- Move explanations longer than 20 characters out of the immediate watch flow where possible.

Example labels include:

- `Start Workout`
- `Next: Squat`
- `Set 2 of 5`
- `Rest 1:30`
- `Plates / Side`
- `Repeat Weight`
- `Deload?`
- `Not Synced`

Every watch screen uses a black primary background and white primary text. Red is reserved for accent icons, selection markers, progress circles, important actions, and timer-completion emphasis; it must never fill the screen background. On Flint, red accents translate into clear white filled, outlined, or inverted monochrome states.

The visual design should embrace Pebble's pixelated character:

- Pixel-oriented typography
- Blocky icons
- High contrast
- Minimal animation
- Simple visual hierarchy
- Deliberate use of vibration and symbols to reduce text

On color displays, the default theme should use a black background with white text and restrained red accent icons and circles. Red is an accent for important actions and status, never a full-screen background.

On monochrome hardware, the design must translate red emphasis into strong dark/light contrast without losing status distinctions.

## 13. Reliability and data safety

The app must:

- Persist a logged set promptly.
- Recover cleanly after interruption.
- Avoid silently losing unsynchronized workouts.
- Version stored data and provide migration paths between app releases.
- Make destructive actions such as clearing history deliberate and confirmable.
- Treat the workout log as authoritative and derive recommendations from it.
