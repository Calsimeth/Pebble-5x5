# Physical release readiness — 2026-09-14

## Release identity

- Source commit: `f006cd6ce92bdace8634fdd52078b61398ebe319`
- Fixture-free bundle SHA-256: `c9894dd2bc5a7379b5b59b68a8e4962565c3b546b88a98a86b949c7e0025a3c1`
- WSL2 preflight: PASS (`Default Version: 2`, Ubuntu running, exact `WSL_OK`)
- Build: PASS

## Developer Connection discovery and install

The stale address `192.168.8.129` timed out from the active Ethernet source `192.168.8.107`. A bounded port-9000 check found listeners at `192.168.8.149`, `.183`, and `.188`. Read-only `pebble ping --phone` results were:

- `192.168.8.149`: `Connection to remote host was lost.`
- `192.168.8.183`: `Pong!` — identified as the live Pebble Developer Connection
- `192.168.8.188`: `Connection to remote host was lost.`

The current bundle was installed with:

`pebble install --phone 192.168.8.183`

The command reported `Installing app...` followed by `App install succeeded.` No wipe, uninstall, reset, or physical-watch data mutation beyond this app install was performed.

## Focused physical verification still required

The completed 09:50 physical checklist predates f006cd6 and is not retroactively reclassified. The installed build is ready for a brief watch-side check of only:

1. New Workout first Squat set defaults to five reps.
2. Squat fifth-set completion displays the explicit Next Exercise pause; Select begins the next exercise.
3. A completed workout reaches the expected synchronized state without a Save failed/Sync Required result.
4. History resolves to the current completed date.
5. Progress resolves to a rendered graph (or an explicit valid empty state), not indefinite Loading.

No physical-watch screen, button, or transport result was observed by this task, so none of these physical outcomes is claimed PASS or FAIL here. The prior physical findings remain historical until a watch-side observation is made.
