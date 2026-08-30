# Pebble StrongLifts

Pebble StrongLifts is a proposed workout tracker for the current Pebble watch lineup. It is intentionally limited to the standard StrongLifts 5x5 Workout A and Workout B flow and is designed for fast, low-distraction use while lifting.

The third watch-only vertical slice is implemented. It adds a persistent three-minute rest timer between work sets, halfway and completion vibration feedback, interruption-safe timer resume, and safe expiration/skip handling. The visual foundation uses black primary backgrounds, white primary text, and red accent icons/circles/actions; Flint uses clear white monochrome equivalents for those accents.

## Documentation

- [Product requirements](docs/requirements.md)
- [Architecture and data strategy](docs/architecture.md)
- [Development discussion guide](docs/development-guide.md)
- [Research sources](docs/research-sources.md)

## Product direction

The intended product is one installable Pebble watchapp with a bundled PebbleKit JS component that runs inside the official Pebble Android app. Users must not need to install or sideload a separate Android companion application.

The watch must remain useful without a phone connection. The bundled phone-side component may retain longer-term history and synchronize with the watch when available. Import and export are desirable, but are not guaranteed acceptance criteria because the PebbleKit JS sandbox may prevent a clean Android file workflow.

## Current status

The first three watch-only vertical slices are implemented. The watchapp is written in C, works without phone data, and includes only a minimal PebbleKit JS message acknowledgment foundation. The third slice starts a 180-second rest after non-final work sets, shows the upcoming set, vibrates once halfway and twice at completion, persists timestamps, and resumes or safely clears rest after interruption. The display keeps black backgrounds and white content, reserving red for accent status/action elements with white monochrome equivalents on Flint. During an active workout, hold Back for one second to request abandonment, then press Select to confirm; the same workout remains next.

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
