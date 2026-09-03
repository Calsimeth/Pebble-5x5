#!/bin/sh
set -eu
main=src/c/main.c
grep -q 'window_multi_click_subscribe(BUTTON_ID_BACK, 1, 2, 350, true' "$main"
grep -q 'click_number_of_clicks_counted(recognizer)' "$main"
grep -q 'BACK_SINGLE' "$main"
grep -q 'BACK_DOUBLE' "$main"
grep -q 'if (s_confirm_abandon)' "$main"
if grep -q 'window_long_click_subscribe(BUTTON_ID_BACK\|window_raw_click_subscribe(BUTTON_ID_BACK' "$main"; then exit 1; fi
echo "production Back single/double routing passed"
