# Slice 9 physical weight persistence trace

This is one continuous `pebble logs --phone 192.168.8.120` capture from the
debug build, installed over the existing app with the unchanged UUID and no
wipe. The complete chronological output is preserved in
[`root-logs/slice9-weight-failure-capture.log`](root-logs/slice9-weight-failure-capture.log).

Weights below are displayed pounds; the app stores quarter-pound units.

| checkpoint | displayed weights | evidence |
|---|---|---|
| Before | 45,45,65,45,95 | `22:40:41 UI_WEIGHTS` |
| Edited | 50,45,65,45,95 | `22:40:48 SETUP_CHANGE i=0 old=45 new=50` |
| Committed | 50,45,65,45,95 | generation 808, slot 1; keys 12/13/14; 148/176/8 bytes, all full writes |
| Loaded | 60,45,65,45,95 | generation 813, slot 0; transactional reads all full |
| Displayed after | 45,45,65,45,95 | `22:41:25 UI_WEIGHTS` |

The user interaction generated additional setup edits after the requested
`+15 lb` edit, ending at 60 lb before the first captured exit. The capture
then recorded a second edit sequence ending at 60 lb and a second relaunch.

## First divergence

The first demonstrated divergence is at `22:41:25`, `UI_WEIGHTS
W=45,45,65,45,95`, after `22:41:21`, `LOAD_RESULT result=0 generation=813
slot=0 W=60,45,65,45,95`. The transactional save and load checkpoints before
that line are successful. Therefore this trace classifies as a UI-state/editor
divergence, not a persistence-write defect. No fix is being claimed or applied.

The same capture also shows a later transactional load at generation 817 with
`W=60,45,65,45,95`, followed by the same editor entry at 45 lb. This confirms
the discrepancy is repeatable in the captured editor-entry path, but the exact
original user exit/re-entry gesture is not independently distinguished from the
captured normal Back/Home/app-exit procedure.

## Canonical event sequence

The debug build emits `APP_BOOT`, `APP_DEINIT`, `SETUP_CHANGE`, `SAVE_BEGIN`,
per-key `DBG_PERSIST_*` results, `SAVE_COMMIT`, `LOAD_CHOICE`, `LOAD_RESULT`,
and `UI_WEIGHTS`. Every read/write in the relevant transactions returned its
requested byte count. `SYNC_SETUP_MUTATION` still prints raw quarter-pound
units for compatibility; the canonical diagnostic events use pounds.
