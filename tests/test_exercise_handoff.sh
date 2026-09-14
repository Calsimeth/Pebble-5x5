#!/bin/sh
set -eu
main=src/c/main.c
grep -q 's_state.warmup_active = 2' "$main"
grep -q 'static void mark_exercise_handoff' "$main"
grep -q 'if (s_state.warmup_active == 2)' "$main"
grep -q '"Next Exercise"' "$main"
grep -q '"Select to begin' "$main"
grep -q 'else mark_exercise_handoff();' "$main"
if grep -q 'if (s_state.exercise_index >= 3).*mark_exercise_handoff' "$main"; then exit 1; fi
echo "production exercise handoff acknowledgement passed"
