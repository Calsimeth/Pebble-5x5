#!/bin/sh
set -eu
out="${TMPDIR:-/tmp}/workout-completion-test.$$"
trap 'rm -f "$out"' EXIT HUP INT TERM
gcc -std=c11 -Wall -Wextra -Werror -Isrc/c \
  tests/test_workout_completion_integration.c src/c/workout_completion.c \
  src/c/sync_completion.c src/c/sync.c src/c/progression.c src/c/deload.c \
  src/c/plates.c src/c/warmups.c -o "$out"
"$out"
