#!/bin/sh
set -eu

tmp_dir="${TMPDIR:-/tmp}/stronglifts-fixtures.$$"
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
mkdir "$tmp_dir"
gcc -std=c11 -Wall -Wextra -Werror -Isrc/c \
  tests/gen_sync_fixture.c src/c/sync.c -o "$tmp_dir/gen_sync_fixture"
"$tmp_dir/gen_sync_fixture" > "$tmp_dir/generated"
cat tests/fixtures/sync_a.json tests/fixtures/sync_b.json > "$tmp_dir/committed"
cmp "$tmp_dir/generated" "$tmp_dir/committed"
