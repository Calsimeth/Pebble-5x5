# Pebble StrongLifts

Pebble StrongLifts is a proposed workout tracker for the current Pebble watch lineup. It is intentionally limited to the standard StrongLifts 5x5 Workout A and Workout B flow and is designed for fast, low-distraction use while lifting.

The fourth watch-only vertical slice adds persistent quarter-pound working weights and a deterministic symmetrical per-side plate calculator. It retains the persistent three-minute rest timer, interruption-safe resume, and established black, white, and red visual system; Flint uses clear white monochrome equivalents for those accents.

## Documentation

- [Product requirements](docs/requirements.md)
- [Architecture and data strategy](docs/architecture.md)
- [Development discussion guide](docs/development-guide.md)
- [Research sources](docs/research-sources.md)

## Product direction

The intended product is one installable Pebble watchapp with a bundled PebbleKit JS component that runs inside the official Pebble Android app. Users must not need to install or sideload a separate Android companion application.

The watch must remain useful without a phone connection. The bundled phone-side component may retain longer-term history and synchronize with the watch when available. Import and export are desirable, but are not guaranteed acceptance criteria because the PebbleKit JS sandbox may prevent a clean Android file workflow.

## Current status

The first four watch-only vertical slices are implemented. Working weights are stored as quarter-pound scaled integers (45 lb = 180 units), with defaults of Squat 45, Bench 45, Row 65, OHP 45, and Deadlift 95 lb. Plate inventory configuration, progression, warm-ups, history synchronization, and CSV import/export remain future work. During an active workout, hold Back for one second to request abandonment, then press Select to confirm; the same workout remains next.

The project uses the current Pebble SDK project layout:

- `package.json` contains the app metadata, message keys, and `flint`, `emery`, and `gabbro` targets.
- `wscript` contains the standard SDK build rules.
- C sources are under `src/c/`; phone-side PebbleKit JS is under `src/pkjs/`.

## Local SDK development

The Pebble SDK does not run natively on Windows. Workers developing this repository must run SDK commands inside the Ubuntu WSL 2 distribution, while continuing to edit the files in the Windows checkout.

### One-time WSL setup

Install the Linux runtime dependencies from an Ubuntu WSL shell:

```sh
sudo apt-get update
sudo apt-get install -y curl nodejs npm libsdl2-2.0-0
```

Install `uv`, a Linux Node.js runtime, Pebble Tool, and the current SDK. The project has been verified with Pebble Tool 5.0.40 and SDK 4.33.1:

```sh
curl -LsSf https://astral.sh/uv/install.sh | sh
export PATH="$HOME/.local/bin:$PATH"
uv tool install pebble-tool --python 3.13
pebble sdk install 4.33.1
```

The bundled PebbleKit JS step also needs the SDK webpack launcher to be executable:

```sh
chmod +x "$HOME/.local/share/pebble-sdk/SDKs/4.33.1/node_modules/.bin/webpack"
```

### Worker command pattern

Each worker shell must expose the user-local Node.js runtime, SDK webpack launcher, and Pebble Tool before invoking the SDK:

```sh
export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"
cd "/mnt/c/Users/Caleb/Documents/repos/Pebble Stronglifts"
```

From PowerShell or an automated worker, wrap the Linux commands with `wsl -d Ubuntu -- bash -lc`. Keep the Linux command in single quotes so `$HOME` and `$PATH` expand inside WSL:

```powershell
wsl -d Ubuntu -- bash -lc 'export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"; cd "/mnt/c/Users/Caleb/Documents/repos/Pebble Stronglifts"; pebble build'
```

Build all configured targets and install the result in each emulator with:

```sh
pebble build
pebble install --emulator flint
pebble install --emulator emery
pebble install --emulator gabbro
```

Workers must report the build and emulator result for each target. They should not commit `/build/`, `.lock-waf_linux_build`, SDK files, emulator state, or other generated output. If an emulator command hangs, stop only that command and retry the affected platform; a transient WebSocket disconnect does not imply a source or build failure.

With the SDL2 runtime installed, the first slice was built and installed successfully in the Flint, Emery, and Gabbro emulators. Emulator validation for the second slice is recorded only when performed.

The remaining product work is described in [the development discussion guide](docs/development-guide.md). Weights, plates, warm-ups, progression, timers, deloading, history synchronization, and CSV import/export are intentionally not part of this second slice.
