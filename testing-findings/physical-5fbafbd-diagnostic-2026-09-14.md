# Physical diagnostic — 5fbafbd — 2026-09-14

This is a read-only diagnostic follow-up. Existing watch/phone data was preserved; no wipe, reset, uninstall, navigation, message replay, or repeated workout was performed.

## Identity and install

- Source commit: `5fbafbd195fe1c6bf82b944e62eac075ecf8c7b4`
- App UUID: `8e2d6c6f-54ee-4f4a-ae6c-7e6cf6b4c8c1`
- Fixture-free bundle SHA-256: `68f08c0ff4c143d87ebc8123c0033bb5fd5a1eb396fa6424a000ab4536de170a`
- WSL2 preflight: PASS; `npm test`: PASS; `pebble build`: PASS
- Separate emulator installs: Flint, Emery, and Gabbro PASS
- Physical Developer Connection: `pebble ping --phone 192.168.8.183` → `Pong!`
- Physical install: `pebble install --phone 192.168.8.183` → `Installing app...` and `App install succeeded.`

## Read-only physical log capture

The bounded capture is preserved as `physical-5fbafbd-logs.txt`. It contains:

- During app teardown/install: `SYNC_RECORD_COMMITTED generation=1124 result=0 ... q=3`, `SYNC_SEND begin=0 bytes=114`, and phone-side `HISTORY_RECORD_RX id=1 workout=A`.
- On app boot: `APP_MESSAGE_OPEN ... result=0` and `SYNC_BOOT_STATE q=3 pending=1 blocked=0 adapter=0 ready=1`.
- The watch attempted `SYNC_SEND` twice, but no `SYNC_ACK` or ACK error line was emitted.
- Phone-side JS initialized and received the record, then emitted `HISTORY_RECORD_REJECTED id=1 reason=validation_or_storage`.
- No physical `CALENDAR_REQUEST`, `CALENDAR_RESPONSE`, `PROGRESS_REQUEST`, `PROGRESS_RESPONSE`, or query-timeout lines were available.

## Interpretation

The physical `Sync Required` result remains unresolved and is consistent with a non-empty watch queue (`q=3`) and missing observed ACK, but the logs do not establish why the phone rejected the record. History/Progress cannot be diagnosed as query failures from this capture because no physical query requests were logged; the phone-side record rejection and absent ACK occur earlier in the observable chain.

The emulator PASS at `f006cd6`/Slice 12 does not prove physical behavior: emulator runs use QEMU and a simulated phone/PKJS transport, whereas this trace is from the Android Developer Connection and the actual watch-side queue/storage path. The physical gate therefore remains **CLOSED / UNVERIFIED**, with the observed completion/sync outcome still **FAIL** and no root cause assigned.
