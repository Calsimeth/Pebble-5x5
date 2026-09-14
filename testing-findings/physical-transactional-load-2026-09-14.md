# Physical transactional-load validation — 2026-09-14

## Verdict

PASS for the exercised physical Pebble Time 2 path. The second debug relaunch retained all five weights, and the normal release edit/relaunch retained the edited first weight and the other four unchanged. No divergence was observed.

## Build identity

- Source HEAD: `941a4e586d27851270d71705b1fce43dd10ee1d9` (`941a4e5`).
- Relevant working-tree source was dirty in `AGENTS.md`, `README.md`, `package.json`, `src/pkjs/index.js`, and `wscript`; these were preserved and not changed by this validation.
- Release build command: WSL Ubuntu 2, `STRONGLIFTS_DEBUG` unset, `pebble build`.
- Bundle SHA-256: `f8f9b92aa5de83f26765b8d1f4e09683e39ba279ef04b033ebbbbcce9e6f4ac3`.
- Flint ELF SHA-256: `3942dd8fbc3007fbd8e6c59c7e9bb0a13f2dec533fde8e3b0d17855e2e997903`.
- Emery ELF SHA-256: `a18f46500d26f78ae14ce57f273dfd73bb575abd066137f917891c95007ab213`.
- Gabbro ELF SHA-256: `8f716c7ed423805572913c190fc5dd5bf989cc72e6ddbd6f5e18389bb3c175d2`.
- `strings` scan of all three release ELFs found none of `DBG_`, `LOAD_CHOICE`, `UI_WEIGHTS`, `Diagnostics`, or `Schema`.
- Install evidence: `Installing app...` followed by `App install succeeded.` over phone `192.168.8.183`; no wipe/uninstall.

## Physical evidence

Exercise order is Squat, Bench, Row, Overhead Press, Deadlift.

1. Existing baseline trace: generation 884, transaction load, slot 1, loaded/displayed `115,100,100,100,100`.
2. Second debug relaunch: commit/load generation 887. Physical screen readback: `115,100,100,100,100`. Source was the transactional load; no edit was made.
3. Release edit/relaunch: one small edit changed Squat `115 -> 110`; mutation commit generation 889; normal exit commit generation 890; relaunch loaded generation 890. Physical screen readback: `110,100,100,100,100`.

Exact arrays:

```text
debug before/reloaded/displayed:  [115,100,100,100,100]
release before:                   [115,100,100,100,100]
release edited/saved/reloaded:    [110,100,100,100,100]
```

First divergence: none.

## Raw chronological logs

- [physical-precedence-20260914.log](../physical-precedence-20260914.log) — baseline transaction load through generation 884.
- [physical-debug-second-20260914.log](../physical-debug-second-20260914.log) — second debug relaunch through generation 887.
- [physical-release-20260914.log](../physical-release-20260914.log) — release edit, normal exit, and relaunch through generation 890.

The debug capture did not emit a `UI_WEIGHTS` line for the final screen redraw; the displayed debug array above is Caleb's direct physical readback, while the transactional source/generation is independently present in the preceding captured load sequence.
