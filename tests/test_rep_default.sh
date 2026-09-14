#!/bin/sh
set -eu
main=src/c/main.c
view=src/c/workout_view.c

# The production new-workout branch must reset both selector copies before
# entering warmup. Active-session loads intentionally remain untouched.
branch=$(sed -n '/static void select_click/,/static void up_click/p' "$main")
printf '%s\n' "$branch" | grep -F 's_state.selected_reps = 5;'
printf '%s\n' "$branch" | grep -F 's_selected_reps = 5;'
printf '%s\n' "$branch" | grep -F 's_state.active = 1;'

# Zero through five remain deliberate valid selections in the production view.
grep -F 'bool workout_view_rep_selected_valid(uint8_t reps) { return reps <= 5; }' "$view"
echo "production new-workout five-rep default and zero-rep selection passed"
