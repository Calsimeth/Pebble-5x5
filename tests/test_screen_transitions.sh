#!/bin/sh
set -eu
main=src/c/main.c
# Guard the production click-handler transitions against the confirmed
# fall-through: selecting a workout is terminal for that click event.
grep -q 's_screen == SCREEN_WORKOUT_SELECT).*update_display(); return;' "$main"
grep -q 's_screen == SCREEN_PROGRESS_PICKER).*query_send("progress_request"); update_display(); return;' "$main"
grep -q 's_screen == SCREEN_PROGRESS_GRAPH).*query_cancel(); s_screen=SCREEN_PROGRESS_PICKER; update_display(); return;' "$main"
grep -q 's_screen == SCREEN_PROGRESS_PICKER || s_screen == SCREEN_HISTORY).*query_cancel(); show_home(); return;' "$main"
grep -q 's_screen == SCREEN_HISTORY).*query_send("calendar_request");' "$main"
echo 'screen transition guards passed'
