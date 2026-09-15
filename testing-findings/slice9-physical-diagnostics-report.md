# Slice 9 physical diagnostics

Build: debug `STRONGLIFTS_DEBUG=1`, normal UUID, installed over the existing
application at `192.168.8.120` without uninstalling or wiping state.

The complete chronological captures are in
[`root-logs/physical-diagnostics-capture-2.log`](root-logs/physical-diagnostics-capture-2.log) and
[`root-logs/physical-diagnostics-final.log`](root-logs/physical-diagnostics-final.log) in the
repository's raw-log folder. Both were written directly by
`pebble logs --phone 192.168.8.120 | tee ...` and are committed alongside this
report.

## Boundaries and result

Weights in this report are display pounds. Persistence stores quarter-pound
units via `WEIGHT_LB(lb)`, so raw logger values `180` and `200` correspond to
45 lb and 50 lb. The corrected diagnostic logger reports display pounds.

BOOT 1 loaded generation `789`, slot `0`:

`W=45,45,65,45,95 P=2,0,1,0,1,1,1`

The physical edit changed Squat from `45` to `50`. The immediate save
committed generation `790` to slot `1`, using core key `12` (148 bytes), sync
key `13` (176 bytes), and metadata key `14` (8 bytes); all writes returned
their requested byte counts.

The first normal exit committed generation `791`. BOOT 2 loaded generation
`791` and reported:

`W=50,45,65,45,95 P=2,0,1,0,1,1,1`

The second normal exit committed generation `793` after the unchanged-value
re-entry procedure. BOOT 3 loaded generation `793` and reported the same
five weights and plate counts.

The corrected final capture then recorded a further clean relaunch sequence:
BOOT 3 loaded generation `805` with
`W=60,45,65,45,95 P=2,0,1,0,1,1,1`. A normal exit committed generation `806`,
and the following relaunch loaded generation `806` with the identical five
weights and plate counts. All metadata, core, and sync reads returned their
requested byte counts.

No stored/loaded divergence or reset was observed. There was a later physical
edit from `200` to `220` after BOOT 3, followed by generation `795` and exit
generation `796`; the capture ended before a fourth reload, so that later
value is intentionally not claimed as reload-verified.

## Relevant event sequence

Each boot boundary contains `DBG_KEYS`, `DBG_EVENT 1`, metadata/core/sync
reads, `persist loaded generation=...`, and `DBG_STATE_LOAD`. Each save
contains `DBG_STATE_SAVE`, metadata read, core/sync/metadata writes, and
`SYNC_RECORD_COMMITTED`. No persistence read or write returned an error.

The tested procedure was: Setup weight editor, normal Back navigation to Home,
normal app exit, relaunch, and repeat once. This does not prove the exact
physical procedure that originally caused the reset if that procedure used a
different exit gesture, OS task termination, or watch sleep/power transition.
