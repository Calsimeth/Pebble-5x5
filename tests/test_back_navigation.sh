#!/bin/sh
set -eu
main=src/c/main.c
grep -q 'window_single_click_subscribe(BUTTON_ID_BACK, back_click)' "$main"
grep -q 'BACK_SINGLE' "$main"
grep -q 'if (s_confirm_abandon)' "$main"
grep -q 's_confirm_abandon = false;[[:space:]]*save_state();[[:space:]]*show_home();' "$main"
grep -q 's_state.active = 0' "$main"
grep -q '"End Workout"' "$main"
if grep -q 'window_multi_click_subscribe(BUTTON_ID_BACK\|window_long_click_subscribe(BUTTON_ID_BACK\|window_raw_click_subscribe(BUTTON_ID_BACK' "$main"; then exit 1; fi
if grep -q 'BACK_DOUBLE\|BACK_LONG' "$main"; then exit 1; fi
echo "production Back single-click and abandonment routing passed"
