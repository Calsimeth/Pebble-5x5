# Development discussion guide

This document lists the decisions that should be resolved before implementation and suggests an order for the development conversation.

## Confirmed decisions

- Strict StrongLifts Workout A/B support only
- Deadlift defaults to 1x5
- Pounds by default; kilograms optional
- Symmetrical loading only
- Plate inventory specified per side
- Default plates: 45, 25, 10, 5, and 2.5 lb
- Optional plates: 35 and 15 lb
- Automatic progression with manual override
- Failed exercises repeat their weight
- Deloads are suggested, never forced
- Deload recommendation after a qualifying gap of more than seven days
- Repeated-failure guidance based on the StrongLifts program
- Three-minute work-set rest timer
- Subtle halfway vibration and stronger final vibration
- Android phone environment
- No separate Android companion app
- Bundled PebbleKit JS phone-side storage is acceptable
- Import/export is aspirational rather than required
- Current Pebble hardware only
- Watch text limited to 20 characters
- Pixelated visual theme
- Red primary color with white contrast

## Decisions still needed

### 1. Current hardware definition

Confirm that "all current Pebble watches" means Pebble 2 Duo (`flint`), Pebble Time 2 (`emery`), and Pebble Round 2 (`gabbro`). This affects the choice of C versus Alloy and the layout matrix.

Recommended default: target all three.

### 2. Progression increments

Choose default increments per exercise.

Candidate default:

| Exercise | Default increase |
| --- | ---: |
| Squat | 5 lb |
| Bench press | 5 lb |
| Barbell row | 5 lb |
| Overhead press | 5 lb |
| Deadlift | 5 lb |

Smaller increments may be preferable for upper-body lifts, but a 2.5 lb total increase requires 1.25 lb plates per side, which are not in the current default inventory. The progression rule must reconcile desired increments with achievable plate configurations.

### 3. Seven-day deload clock

Decide whether the gap is measured from:

- The most recent completed workout overall, or
- The most recent completed session for the specific exercise.

Recommended default: use the most recent completed workout overall for a return-after-break advisory, then calculate proposed reductions per exercise.

### 4. Deload calculation

Confirm the default percentage and rounding behavior.

Recommended default: suggest 10%, then round down to the closest achievable symmetrical load.

### 5. Warm-up algorithm

Define:

- How many warm-up sets correspond to each work-weight range.
- Whether the rule is based on 45 lb plate count, percentage of work weight, absolute weight gap, or a combination.
- Repetitions for intermediate sets.
- Floor-appropriate minimums for rows and deadlifts.
- Default warm-up rest duration.

Recommended starting point: use 45 lb plate count to choose the number of sets, then divide the load gap evenly and round each target down through the plate calculator.

### 6. Manual editing

Decide how much history can be edited from the watch. Full historical editing may be impractical with the screen and input constraints.

Recommended default: allow undo during the active workout and correction of the most recent completed workout; defer arbitrary old-history editing.

### 7. Phone-side history limits

PebbleKit JS `localStorage` is larger than watch storage but is not an explicit database service. Establish:

- A chunk size
- A retention policy if storage fails
- A visible storage-health state
- Whether old history can be summarized while retaining set-level detail elsewhere

Recommended default: retain full history until a write fails, warn before deleting anything, and never prune silently.

### 8. Import/export experiment

Before promising StrongLifts CSV import or export, create a small technical proof of concept that tests what the current Pebble Android app and PebbleKit JS sandbox permit. Obtain a real StrongLifts CSV sample for parser fixtures if import proves feasible.

## Recommended development sequence

1. Validate the current SDK and hardware targets.
2. Prototype persistence limits and PebbleKit JS storage behavior.
3. Specify the compact data model and synchronization protocol.
4. Specify the plate-calculation algorithm and tests.
5. Finalize warm-up and progression rules.
6. Produce low-fidelity layouts for all target displays.
7. Build a watch-only vertical slice for one Workout A session.
8. Add Workout B, progression, failure, and deload behavior.
9. Add phone-side synchronization and history.
10. Evaluate import/export as a separate feasibility feature.

## Definition-of-done framing

The first usable release should be considered successful when it reliably tracks A/B workouts, survives interruption, calculates achievable loading, handles progression and deload suggestions, and stores longer-term history through bundled PebbleKit JS without a separate Android installation.

StrongLifts CSV import/export should not block the first usable release unless a technical experiment demonstrates a clean user experience inside the existing Pebble app.
