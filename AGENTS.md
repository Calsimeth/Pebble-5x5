# Agent Instructions: WSL and Pebble SDK

These instructions are mandatory for every worker modifying or verifying this repository.

## Never run Pebble commands directly from Windows

The Pebble SDK is Linux-only. Do not run `pebble build`, `pebble install`, or any other Pebble SDK command directly in PowerShell or Windows Command Prompt.

Every Pebble command must run inside Ubuntu WSL 2 using this command shape:

```powershell
wsl -d Ubuntu -- bash -lc 'export PATH="$HOME/.local/node/bin:$HOME/.local/share/pebble-sdk/SDKs/current/node_modules/.bin:$HOME/.local/bin:$PATH"; cd "/mnt/c/Users/Caleb/Documents/repos/Pebble Stronglifts"; pebble build'
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
