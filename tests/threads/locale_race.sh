#!/usr/bin/env bash
# Builds tests/threads/locale_race.c against LibXL and runs it unpatched (the
# race must reproduce) and patched (no locale change may reach another
# thread). It first checks that the install is refused for a library that
# imports a locale-dependent function without a wrapper. Linux with glibc
# on x86_64 or aarch64 only. Set LIBXL_LIBDIR when
# the LibXL library directory is not the one for this architecture.
#
# Usage: tests/threads/locale_race.sh /path/to/libxl [runs]
set -euo pipefail

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
	echo "usage: $0 /path/to/libxl [runs]" >&2
	exit 2
fi
libxl=$1
runs=${2:-3}
case $(uname -m) in
	aarch64) libdir=${LIBXL_LIBDIR:-$libxl/lib-aarch64} ;;
	*) libdir=${LIBXL_LIBDIR:-$libxl/lib64} ;;
esac
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

"${CC:-cc}" -O2 -D_GNU_SOURCE -Wall -Wextra -Werror \
	-I"$root" -I"$libxl/include_c" \
	"$here/locale_race.c" -o "$work/locale_race" \
	-L"$libdir" -lxl -lpthread

cflags=(-O0 -D_GNU_SOURCE -Wall -Wextra -Werror -fPIC)
"${CC:-cc}" "${cflags[@]}" -shared "$here/locale_fake_libxl.c" -o "$work/libfake_ok.so"
"${CC:-cc}" "${cflags[@]}" -DFAKE_UNWRAPPED -shared "$here/locale_fake_libxl.c" -o "$work/libfake_unwrapped.so"
"${CC:-cc}" "${cflags[@]}" -I"$root" "$here/locale_unwrapped.c" -o "$work/locale_unwrapped" -ldl -lpthread

status=0
"$work/locale_unwrapped" "$work/libfake_ok.so" "enabled" || status=1
"$work/locale_unwrapped" "$work/libfake_unwrapped.so" "disabled (LibXL imports strftime" || status=1

for i in $(seq 1 "$runs"); do
	for mode in unpatched patched; do
		rc=0
		env -u LC_ALL -u LC_CTYPE -u LC_NUMERIC LANG=C.UTF-8 TMPDIR="$work" \
			LD_LIBRARY_PATH="$libdir" "$work/locale_race" "$mode" || rc=$?
		if [ "$rc" -eq 77 ]; then
			exit 77
		fi
		if [ "$rc" -ne 0 ]; then
			echo "run $i $mode: exit $rc" >&2
			status=1
		fi
	done
done
exit "$status"
