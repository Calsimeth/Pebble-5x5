#!/bin/sh
set -eu
main=src/c/main.c
grep -q 'window_single_click_subscribe(BUTTON_ID_UP, up_short_click)' "$main"
grep -q 'window_single_click_subscribe(BUTTON_ID_DOWN, down_short_click)' "$main"
grep -q 'window_long_click_subscribe(BUTTON_ID_UP, SETUP_INITIAL_HOLD_DELAY_MS, up_long_start, up_long_release)' "$main"
grep -q 'window_long_click_subscribe(BUTTON_ID_DOWN, SETUP_INITIAL_HOLD_DELAY_MS, down_long_start, down_long_release)' "$main"
grep -q 'adjustment_long_start(BUTTON_ID_UP)' "$main"
grep -q 'adjustment_long_start(BUTTON_ID_DOWN)' "$main"
if grep -q 'window_raw_click_subscribe(BUTTON_ID_UP\|window_raw_click_subscribe(BUTTON_ID_DOWN' "$main"; then
  echo 'raw Up/Down registration remains' >&2
  exit 1
fi
echo 'production setup recognizer registration passed'
