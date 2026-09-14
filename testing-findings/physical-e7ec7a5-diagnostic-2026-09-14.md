# Physical diagnostic — e7ec7a5 — 2026-09-14

## Identity and readiness

- Source commit: `e7ec7a5d5ea2853c135d657dee32ad0c71aa9f6f`
- App UUID: `8e2d6c6f-54ee-4f4a-ae6c-7e6cf6b4c8c1`
- Fixture-free bundle SHA-256: `ee74c56ceb4c5837c76b38c141e20e2de4ef3d2181a9f7d5a40b1fa8736729f4`
- WSL2 preflight: PASS; `npm test`: PASS; `pebble build`: PASS
- Separate Flint, Emery, and Gabbro emulator installs: PASS
- Current physical Developer Connection: `192.168.8.183` (`pebble ping --phone` → `Pong!`)
- Physical install: PASS (`Installing app...`, `App install succeeded.`)

## Physical log evidence

Raw bounded capture: `physical-e7ec7a5-logs.txt`.

On app teardown/restart and boot, the watch reported `SYNC_RECORD_COMMITTED generation=1125 result=0 ... q=3`, then `SYNC_BOOT_STATE q=3 pending=1 blocked=0 adapter=0 ready=1` and attempted `SYNC_SEND`. The phone-side JS received `HISTORY_RECORD_RX id=1 workout=A`. The new Slice 9 diagnostics then reported:

`HISTORY_STORE_REJECTED id=1 reason=id_conflict`

followed by:

`HISTORY_RECORD_REJECTED id=1 reason=store_rejected`

No `HISTORY_RECORD_VALID`, `HISTORY_ACK_SENT`, or `SYNC_ACK_COMMITTED` line appeared, and the queue remained at `q=3` in the observed boot state. This is the first precise physical rejection reason captured. It establishes an ID conflict in the phone-side stored history path, but does not establish whether the existing record or incoming record is authoritative; no root cause is assigned.

## Query and deferred-query status

No physical `CALENDAR_REQUEST`, `CALENDAR_RESPONSE`, `PROGRESS_REQUEST`, `PROGRESS_RESPONSE`, `QUERY_DEFERRED`, `QUERY_TIMEOUT`, or `DEFERRED_RESUME` lines were available. The current emulator evidence set likewise has no fresh deferred-query trace tied to this e7ec7a5 build; this was not manufactured with fixtures or by resetting data.

The emulator query PASS results do not prove physical behavior because the emulator uses QEMU and a simulated PKJS transport, while this trace is from the Android Developer Connection and actual phone storage. Physical sync remains **FAIL / unresolved**; History and Progress remain **unverified as transport diagnoses** because no physical query request was observed. The physical gate remains **CLOSED**. Existing watch/phone data was preserved; no wipe, reset, uninstall, navigation, message replay, or new workout was performed.
