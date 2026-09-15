# Slice 9 physical persistence observations

## Scope

This report describes one continuous debug-build `APP_LOG` session from the
physical Pebble Time 2 at `192.168.8.120`. The debug build used the normal UUID
and was installed over the existing app without uninstalling or wiping state.
The complete raw capture is
[`root-logs/slice9-weight-failure-capture.log`](root-logs/slice9-weight-failure-capture.log).

Weights in the canonical diagnostic lines are display pounds. The application
stores quarter-pound units internally (`WEIGHT_LB(lb) == lb * 4`).

## Confirmed observations

Initial exercise-editor state:

```text
UI_WEIGHTS W=45,45,65,45,95
```

The first Squat adjustment was:

```text
SETUP_CHANGE i=0 old=45 new=50
SAVE_BEGIN reason=state W=50,45,65,45,95
```

That save used the transactional store. The persistence operations reported:

```text
READ  key=14 requested=8 returned=8
WRITE key=12 requested=148 returned=148
WRITE key=13 requested=176 returned=176
WRITE key=14 requested=8 returned=8
SAVE_COMMIT result=0 generation=808 slot=1 W=50,45,65,45,95
```

No write failure or partial write was observed.

The session then included additional Squat adjustments. It did not remain at
exactly `+15 lb`; this is recorded as a limitation, not discarded evidence.

At a later exit and relaunch, the transaction loaded successfully:

```text
APP_DEINIT
SAVE_BEGIN reason=state W=60,45,65,45,95
WRITE key=10 requested=148 returned=148
WRITE key=11 requested=176 returned=176
WRITE key=14 requested=8 returned=8
APP_BOOT
READ key=14 requested=8 returned=8
READ key=10 requested=148 returned=148
READ key=11 requested=176 returned=176
LOAD_CHOICE source=transaction version=10
LOAD_RESULT result=0 generation=813 slot=0 W=60,45,65,45,95
```

## First observed divergence

The first direct disagreement between checkpoints was:

```text
22:41:21 LOAD_RESULT result=0 generation=813 slot=0 W=60,45,65,45,95
22:41:25 UI_WEIGHTS W=45,45,65,45,95
```

This is downstream of the transactional read and commit. The trace therefore
does not demonstrate a persistence-write defect. It demonstrates that the
state reported as loaded is not the state subsequently reported when entering
the exercise-weight editor.

## Classification

Based solely on this trace, the first divergence is classified as a
UI/editor-state divergence. The evidence is insufficient to identify which
intervening production operation changed the in-memory state, so no fix is
claimed and none was applied.

## Limitations

- The user performed additional adjustments, so this was not a strict single
  `+15 lb` reproduction.
- The exact original exit/re-entry gesture was not independently reproduced.
- The trace proves a loaded-versus-editor discrepancy, but does not yet prove
  the exact statement or callback responsible for changing the in-memory
  weights.
