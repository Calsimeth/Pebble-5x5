#!/bin/sh
set -eu
tmp="${TMPDIR:-/tmp}/stronglifts-tests.$$"
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir "$tmp"
sources="src/c/persistence.c src/c/sync.c src/c/sync_state.c src/c/sync_adapter.c src/c/sync_completion.c src/c/workout_completion.c src/c/workout_view.c src/c/workout_transition.c src/c/rest_state.c src/c/migration.c src/c/progression.c src/c/deload.c src/c/plates.c src/c/warmups.c src/c/setup_input.c src/c/history_progress.c src/c/query_controller.c"
for test in tests/test_persistence_transaction.c tests/test_persistence_schema10.c tests/test_persistence_workouts.c tests/test_history_progress.c tests/test_query_controller.c tests/test_sync.c tests/test_sync_allocator.c tests/test_sync_state.c tests/test_sync_adapter.c tests/test_sync_completion.c tests/test_pending.c tests/test_migration.c tests/test_progression.c tests/test_deload.c tests/test_plates.c tests/test_warmups.c tests/test_setup_input.c tests/test_workout_completion_integration.c tests/test_workout_view.c tests/test_workout_transition.c tests/test_rest_timer.c tests/test_migration_v10.c tests/test_persistence_sizes.c; do
  name=$(basename "$test" .c)
  gcc -std=c11 -Wall -Wextra -Werror -Isrc/c "$test" $sources -o "$tmp/$name"
  "$tmp/$name"
done
gcc -std=c11 -Wall -Wextra -Werror tests/test_debug_diagnostics.c -o "$tmp/test_debug_diagnostics"
"$tmp/test_debug_diagnostics"
node tests/test_sync.js
sh tests/verify_fixtures.sh
sh tests/test_screen_transitions.sh
sh tests/test_back_navigation.sh
sh tests/test_exercise_handoff.sh
sh tests/test_setup_registration.sh
sh tests/test_rep_default.sh
