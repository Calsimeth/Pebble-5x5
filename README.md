# Pebble StrongLifts

Pebble StrongLifts is a proposed workout tracker for the current Pebble watch lineup. It is intentionally limited to the standard StrongLifts 5x5 Workout A and Workout B flow and is designed for fast, low-distraction use while lifting.

Slice nine adds the bounded, offline-first history synchronization contract. Completed records use a three-entry watch outbox; the phone stores deduplicated records in versioned localStorage chunks and acknowledges only after storage succeeds. CSV import/export remains out of scope.

## Documentation

- [Product requirements](docs/requirements.md)
- [Architecture and data strategy](docs/architecture.md)
- [Development discussion guide](docs/development-guide.md)
- [Research sources](docs/research-sources.md)

## Product direction

The intended product is one installable Pebble watchapp with a bundled PebbleKit JS component that runs inside the official Pebble Android app. Users must not need to install or sideload a separate Android companion application.

The watch must remain useful without a phone connection. The bundled phone-side component may retain longer-term history and synchronize with the watch when available. Import and export are desirable, but are not guaranteed acceptance criteria because the PebbleKit JS sandbox may prevent a clean Android file workflow.

## Current status

The first six watch-only vertical slices are implemented. Working weights are stored as quarter-pound scaled integers (45 lb = 180 units), with defaults of Squat 45, Bench 45, Row 65, OHP 45, and Deadlift 95 lb. From the inactive overview, press Down to open setup; Select advances fields and Up/Down changes the selected weight or per-side plate count. Back leaves setup. Inventory defaults to 45:2, 35:0, 25:2, 15:0, 10:2, 5:2, and 2.5:2 plates per side. Inventory changes normalize affected weights downward. During an active workout, hold Back for one second to request abandonment, then press Select to confirm; the same workout remains next.

Warm-ups start with the configured empty bar and five repetitions. The calculator inspects the 45 lb plates actually used by the work-set configuration: zero produces only the bar, one may add one evenly-spaced intermediate for work weights at or above 135 lb, and two may add two for work weights at or above 225 lb. Each target is rounded down through the plate calculator, and duplicates or unachievable targets are omitted. Plans are capped at three sets.

During a warm-up, Select marks it done and Down skips it; only completing the final warm-up starts the normal three-minute work-set rest. Up opens plates for the current warm-up weight, and Select returns without changing set state. A 45 lb exercise begins work sets immediately. Warm-ups are not counted toward 5x5 or deadlift 1x5.

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

### WSL `E_ACCESSDENIED` workaround

In a restricted worker or sandbox, WSL can fail before Ubuntu starts with `WSL/Service/E_ACCESSDENIED`. This is a host-permission issue, not a Pebble SDK or project failure. Run the WSL command with host-level permission (for example, approve the elevated WSL command when prompted), then verify WSL independently before retrying the build:

```powershell
wsl --status
wsl --list --verbose
wsl -d Ubuntu -- echo WSL_OK
```

The expected result is Ubuntu version 2, a `Running` distribution, and `WSL_OK`. If these commands still return `E_ACCESSDENIED`, restart or enable the Windows WSL/Virtual Machine Platform components using an administrator PowerShell session, then retry. Do not change the Pebble command or install a separate Android companion app to work around this error.

Build all configured targets and install the result in each emulator with:

```sh
pebble build
pebble install --emulator flint
pebble install --emulator emery
pebble install --emulator gabbro
```

Workers must report the build and emulator result for each target. They should not commit `/build/`, `.lock-waf_linux_build`, SDK files, emulator state, or other generated output. If an emulator command hangs, stop only that command and retry the affected platform; a transient WebSocket disconnect does not imply a source or build failure.

Slice seven adds five-repetition work-set entry (including valid zero-rep failures), persisted active-workout weight snapshots and per-set results, independent exercise progression, and consecutive-failure streaks. Successful exercises advance to the first achievable target at least 5 lb heavier; failed exercises repeat their weight. Warm-ups remain separate from progression. Schema version 6 migrates active version-5 workouts, preserving completed work sets and repairing expired rests.

Slice eight adds optional deload guidance. Three consecutive failures or a gap of more than seven days can suggest an independently calculated, achievable reduction of about 10%; the user can accept, adjust, decline, or back out. Recommendations never silently change weights, and gap reviews are recorded. Accepted deload counts support a dismissible plateau advisory. Schema version 7 preserves slice-seven state while adding completion timestamps and per-exercise advisory state.

The current working tree was host-tested in Ubuntu WSL and built for Flint, Emery, and Gabbro. Emulator installation checks are recorded only when performed; no emulator validation has been run for slice eight yet.

The remaining product work is described in [the development discussion guide](docs/development-guide.md). CSV import/export remains future work.
