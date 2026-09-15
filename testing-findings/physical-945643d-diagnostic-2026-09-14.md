# Physical sync diagnostic — 945643d — 2026-09-14

- Source commit: `945643df769bcff6c1847979d7a7d15003a14eb1`
- App UUID: `8e2d6c6f-54ee-4f4a-ae6c-7e6cf6b4c8c1`
- Fixture-free bundle SHA-256: `bbecd4e9937064f2fadb3edefd31890770c2a471cc4bd385499e6a05f657f5bb`
- WSL2 preflight/tests/build: PASS
- Separate emulator installs: Flint, Emery, Gabbro PASS
- Physical endpoint: `192.168.8.183`, `pebble ping` → `Pong!`
- Physical install: PASS (`Installing app...`, `App install succeeded.`)

## Observed physical chain

Raw capture: `physical-945643d-logs.txt`.

On boot the watch reported `SYNC_BOOT_STATE q=3 pending=1`. The queued record `id=1` was sent and received by phone JS. The persistence/synchronization fix then emitted `HISTORY_ID_REMAP original=1 stored=3390373344`, `HISTORY_RECORD_VALID id=1 stored=3390373344`, `HISTORY_VERIFY_VALID id=3390373344`, and `HISTORY_ACK_SENT id=1`. The watch logged `SYNC_ACK_RECEIVED id=1`, followed by `SYNC_ACK_COMMITTED`.

Automatic retries for queued IDs 2, 3, and 4 also produced `HISTORY_RECORD_VALID`, `HISTORY_VERIFY_VALID`, `HISTORY_ACK_SENT`, matching `SYNC_ACK_RECEIVED`, and `SYNC_ACK_COMMITTED`. The queue progressed to `q=0` at `SYNC_ACK_COMMITTED id=4 ... q=0`.

This confirms the physical phone-side ID remap, record validation, ACK send, watch ACK receipt, and queue drain for the observed records. No physical History or Progress navigation was performed, so those UI outcomes remain unclaimed. The prior physical sync failure is resolved for this observed queued-record path; no additional root cause is asserted beyond the logged ID conflict/remap behavior.
