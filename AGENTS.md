# Agent Instructions: WSL and Pebble SDK

These instructions are mandatory for every worker modifying or verifying this repository.

## Never run Pebble commands directly from Windows

The Pebble SDK is Linux-only. Do not run `pebble build`, `pebble install`, or any other Pebble SDK command directly in PowerShell or Windows Command Prompt.

Every Pebble command must run inside Ubuntu WSL 2 using this command shape:

```powershell
wsl -d Ubuntu -- bash -lc 'export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"; cd "/mnt/c/Users/<WINDOWS_USER>/Documents/repos/Pebble Stronglifts"; pebble build'
```

Keep the outer command in single quotes. This is required so `$HOME` and `$PATH` expand inside Ubuntu rather than in PowerShell.

## Required WSL preflight

Before running a build, run all three commands below:

```powershell
wsl --status
wsl --list --verbose
wsl -d Ubuntu -- echo WSL_OK
```

Do not proceed unless:

- WSL reports default version `2`.
- Ubuntu appears in the distro list and is available/running.
- The third command prints exactly `WSL_OK`.

## Mandatory response to `E_ACCESSDENIED`

If any WSL command returns either of these errors:

```text
WSL/Service/E_ACCESSDENIED
WSL/EnumerateDistros/Service/E_ACCESSDENIED
```

the command was blocked by the Windows host permission boundary. It is not a Pebble source error, SDK error, or reason to change the build command.

Stop the current attempt. Request host-level permission/elevation for the WSL command through the execution tool, using a clear justification such as:

> May I run host-level WSL diagnostics/build commands to resolve the access-denied blocker?

Then rerun the three preflight commands. Only after they succeed may the worker run `pebble build`.

Do not:

- Retry the same WSL command repeatedly inside the restricted sandbox.
- Run the build directly with Windows tools.
- Claim that the build is broken before testing WSL with host-level permission.
- Delete, reinstall, or reset the Ubuntu distribution without explicit user approval.
- Modify the Pebble SDK, project files, or README as a substitute for resolving host access.

If elevated WSL diagnostics still fail, report the exact error and stop. Ask the user to repair or enable WSL 2 and Virtual Machine Platform from an administrator PowerShell session. Do not perform destructive WSL repair actions autonomously.

## Build and test sequence

After the preflight succeeds:

1. Run `pebble build` inside Ubuntu WSL.
2. Run dependency-free host tests inside WSL when a Linux compiler is required.
3. Install only with the documented WSL commands:

   ```sh
   pebble install --emulator flint
   pebble install --emulator emery
   pebble install --emulator gabbro
   ```

4. Report the result separately for Flint, Emery, and Gabbro.
5. Do not commit `build/`, `.lock-waf_linux_build`, SDK files, emulator state, or generated test binaries.

The Windows checkout remains the shared source tree. Edit files there, but execute Linux-dependent commands through Ubuntu WSL.

## Install on the user's physical Pebble

Use the local Android Pebble Developer Connection, not CloudPebble. On the phone, enable Developer Mode and Developer Connection and leave that screen enabled during installation. The currently assigned phone/server address is `192.168.8.129`.

After the required WSL preflight and a successful build, first try the direct installation from PowerShell:

```powershell
wsl -d Ubuntu -- bash -lc 'export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"; cd "/mnt/c/Users/<WINDOWS_USER>/Documents/repos/Pebble Stronglifts"; pebble install --phone 192.168.8.129'
```

Success must include `Installing app...` followed by `App install succeeded.` Do not claim installation from a successful build alone.

If the direct command reports `No route to host`, Windows may have two interfaces on `192.168.8.0/24` and WSL may select the wrong one. Verify the phone's port from Windows using a `TcpClient` explicitly bound to the active Wi-Fi IPv4 address. If it is open, use a temporary byte-for-byte TCP bridge:

1. Determine the current Wi-Fi IPv4 address with `ipconfig` and the current WSL Windows-gateway address with `ip route` inside Ubuntu. Do not assume the example addresses remain unchanged.
2. Start a temporary Node TCP server on port `9000` at the WSL gateway. Its outbound `net.createConnection` must set `localAddress` to the Windows Wi-Fi IPv4 address and connect to the phone on port `9000`.
3. Run the normal WSL command above, but pass the WSL gateway to `--phone`.
4. Stop the bridge after installation. It must not remain as a background process.

The bridge proven on 2026-08-31 used Windows Wi-Fi `192.168.8.182`, WSL gateway `172.30.32.1`, and phone `192.168.8.129`. The install command was:

```powershell
node -e "const net=require('net'); const server=net.createServer(a=>{const b=net.createConnection({host:'192.168.8.129',port:9000,localAddress:'192.168.8.182'},()=>{console.log('RELAY_CONNECTED');a.pipe(b);b.pipe(a)}); const stop=()=>server.close(()=>process.exit()); a.on('close',stop); b.on('close',stop); b.on('error',e=>{console.error(e.message);stop()})}); server.listen(9000,'172.30.32.1',()=>console.log('RELAY_READY'))"
```

After that separate PowerShell process prints `RELAY_READY`, run:

```powershell
wsl -d Ubuntu -- bash -lc 'export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"; cd "/mnt/c/Users/<WINDOWS_USER>/Documents/repos/Pebble Stronglifts"; pebble install --phone 172.30.32.1'
```

Do not misdiagnose this verified routing case as a closed phone port, require Ethernet disconnection, or switch to GitHub/CloudPebble without first testing the phone port through the active Wi-Fi source address.

## File-placement and repository hygiene

Keep the repository root limited to project-level files: `README.md`, `AGENTS.md`, `package.json`, `wscript`, `.gitignore`, and the top-level source, test, documentation, and evidence directories. Do not leave screenshots, logs, build output, SDK files, emulator state, or one-off scripts in the root.

Use these locations:

- Application code belongs in `src/`; host tests and test fixtures belong in `tests/`.
- Product and developer documentation belongs in `docs/` or `README.md`.
- Curated acceptance reports, checklists, and walkthroughs belong in `testing-findings/`.
- Curated screenshots that are deliberately committed belong beside their report under `testing-findings/` or in `screenshots/`. Prefer descriptive, stable names and link them from the report.
- Raw or exploratory screenshots belong in the ignored `testing-findings/root-captures/` folder. Do not commit them unless a report explicitly requires that exact capture as durable evidence.
- Raw diagnostic logs belong in the ignored `testing-findings/root-logs/` folder. If a log is required to reproduce or audit a result, link it from a committed report and deliberately force-add only that log.
- Pebble build output, `.lock-waf_linux_build`, emulator state, generated binaries, and temporary SDK artifacts belong in the ignored `.local-artifacts/` folder or the SDK-managed locations. Never commit them.
- Temporary exports, local workout data, and personal configuration belong in the already-ignored `exports/`, `local-data/`, or `.env` files.

Before finishing a task, inspect `git status --short`, move any newly generated root files into the appropriate ignored folder, and update report links if a tracked artifact was relocated. Preserve and commit only authoritative source, documentation, tests, and intentionally curated evidence. Do not use broad cleanup commands or delete ambiguous files merely to make the status look clean.
