# Pebble StrongLifts

Pebble StrongLifts is a proposed workout tracker for the current Pebble watch lineup. It is intentionally limited to the standard StrongLifts 5x5 Workout A and Workout B flow and is designed for fast, low-distraction use while lifting.

The second watch-only vertical slice is implemented. It provides persistent Workout A/B state, set completion, interruption-safe resume, completion alternation, and confirmed abandonment.

## Documentation

- [Product requirements](docs/requirements.md)
- [Architecture and data strategy](docs/architecture.md)
- [Development discussion guide](docs/development-guide.md)
- [Research sources](docs/research-sources.md)

## Product direction

The intended product is one installable Pebble watchapp with a bundled PebbleKit JS component that runs inside the official Pebble Android app. Users must not need to install or sideload a separate Android companion application.

The watch must remain useful without a phone connection. The bundled phone-side component may retain longer-term history and synchronize with the watch when available. Import and export are desirable, but are not guaranteed acceptance criteria because the PebbleKit JS sandbox may prevent a clean Android file workflow.

## Current status

The first two watch-only vertical slices are implemented. The watchapp is written in C, works without phone data, and includes only a minimal PebbleKit JS message acknowledgment foundation. The second slice displays the next fixed Workout A or B, records prescribed sets, resumes an active workout after interruption, saves completion, and alternates the next workout. During an active workout, hold Back for one second to request abandonment, then press Select to confirm; the same workout remains next.

The project uses the current Pebble SDK project layout:

- `package.json` contains the app metadata, message keys, and `flint`, `emery`, and `gabbro` targets.
- `wscript` contains the standard SDK build rules.
- C sources are under `src/c/`; phone-side PebbleKit JS is under `src/pkjs/`.

## Local SDK development

The Pebble SDK does not run natively on Windows. Use WSL 2 with Ubuntu. The SDK can be installed with:

```sh
uv tool install pebble-tool --python 3.13
pebble sdk install 4.33.1
```

The project builds all three targets with:

```sh
pebble build
```

SDK 4.33.1 also requires a Linux Node.js runtime and the SDK webpack launcher for the bundled JS step. The C builds and JS bundle complete in the configured WSL environment. Emulator launches require the WSL SDL2 runtime (`libSDL2-2.0.so.0`), which must be installed with administrator access when missing.

With the SDL2 runtime installed, the first slice was built and installed successfully in the Flint, Emery, and Gabbro emulators. Emulator validation for the second slice is recorded only when performed.

The remaining product work is described in [the development discussion guide](docs/development-guide.md). Weights, plates, warm-ups, progression, timers, deloading, history synchronization, and CSV import/export are intentionally not part of this second slice.
