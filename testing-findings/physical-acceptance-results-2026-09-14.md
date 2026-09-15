# Physical acceptance results — 2026-09-14

## Scope and environment

- Watch: Pebble Time 2
- App shown: `5x5`
- Phone connection: connected
- Test time: 2026-09-14 09:50 local
- Build identity: current installed release after the persistence/synchronization persistence fix; exact installed commit was not recorded in the completed checklist.

This report transcribes the direct physical-watch observations in [physical-watch-self-guided-checklist.md](physical-watch-self-guided-checklist.md). It does not infer success from emulator evidence.

## Classification

| Area | Result | Exact physical observation | Ownership |
| --- | --- | --- | --- |
| Home / Setup entry | PASS | Home and Setup navigation passed. | — |
| Weight adjustment | PASS with usability note | Squat `110 lb` → `115 lb` on one Up tap. Holding repeated controllably and stopped on release. The initial hold delay felt somewhat long. | No defect confirmed; product-feedback note only. |
| Workout choice | PASS | Workout choice check passed. | — |
| First work-set default | FAIL | “First squat set defaults to 0 reps.” | Repetition entry/default state. |
| Exercise-end transition | FAIL | After the fifth first-exercise set, the next exercise’s warmup appeared immediately. The direct physical observation expected a separate acknowledgement before beginning the next exercise so equipment/location can be changed. The direct physical observation reports later exercise transitions did not show the same behavior. | Workout/UI transition. |
| Failed-set entry | PASS | Fewer-than-five repetitions recorded as expected. | — |
| Rest timing / cues | PASS with request | Rest behavior passed. The direct physical observation requested that the halfway vibration also occur at `5:00`; this is a new product request, not a failed stated behavior. | No defect confirmed. |
| Final-circle visibility | PASS | Final circle was observed before advancement. The direct physical observation clarified the expected behavior should apply to every exercise, not only the last exercise. | — |
| Completion and synchronization | UNVERIFIED | Checklist says `Not known`; exact text recorded as `Not Synced`. There is no confirmed completed-record ID, ACK, or queue-cleared evidence. | Not assigned without a reproducible completed-record/sync trace. |
| History | FAIL | First History press did nothing. Second press showed `Loading`, then did not resolve. Phone was connected. | History query/rendering, subject to confirmation after a known completed and synchronized record exists. |
| Progress | FAIL | Selecting a completed exercise showed `Loading` and did not resolve. Phone was connected. | Progress query/rendering, subject to confirmation after a known completed and synchronized record exists. |

## Important interpretation

The phone-backed failures are direct physical UI failures: History and Progress did not resolve from `Loading` while the phone was connected. The workout-completion/synchronization state, however, was not established well enough to prove whether a completed record existed and was acknowledged. That underlying sync condition is therefore **UNVERIFIED**, not a confirmed persistence/synchronization defect.

## Failures by subject

- Workout/UI: first Squat set defaults to `0` reps; first exercise advances directly into the next exercise’s warmup after its fifth set. Confirmed owners: repetition/progression and Workout/UI interaction respectively.
- Rest/vibration: no failed accepted behavior. A `5:00` vibration is a requested enhancement.
- Setup: no confirmed failure. The hold-delay comment is usability feedback.
- Persistence/synchronization: UNVERIFIED; `Not Synced` was recorded, but no record/ACK evidence was captured.
- History/Progress: History and Progress remained on `Loading` despite a connected phone. Provisional owner: History/Progress, pending confirmation with a known completed/synchronized record.

## Overall physical acceptance

**FAIL** — confirmed physical workflow and phone-backed display failures remain. The result does not invalidate the already-verified September 14 Exercise Weights edit → exit → relaunch persistence path.
