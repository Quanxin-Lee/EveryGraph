#!/usr/bin/env bash
# LGPL replacement experiment for a locally built bundle (P1-012 AC-3).
#
#   packaging/macos/lgpl_replace_test.sh [--app out/bundle/<preset>/everygraph_desktop.app]
#                                        [--library libQt6Core.6.12.0.dylib]
#                                        [--keep]
#
# The right an LGPL-3.0 shared-library route has to keep working is that a
# recipient may rebuild the library and put their build in place of the one in
# Contents/Frameworks without relinking the application. This script tests that
# on three signing states of the same bundle, each in its own scratch copy:
#
#   hardened-shipped    hardened runtime, packaging/macos/entitlements.plist (the
#                       configuration this project ships)
#   hardened-strict     hardened runtime, library validation left enabled
#   adhoc               local ad hoc signature only, no hardened runtime
#
# and, for each, three substitution states: the untouched bundle, the replaced
# library with the bundle left unsealed, and the replaced library re-signed ad
# hoc with the bundle re-sealed. It also records whether the Qt libraries really
# load from inside the bundle. Nothing here touches the bundle under test.
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
# shellcheck source=macho_helpers.sh
. "$script_dir/macho_helpers.sh"

app=""
library=""
keep=0

while [ $# -gt 0 ]; do
    case "$1" in
        --app) app=$2; shift 2 ;;
        --library) library=$2; shift 2 ;;
        --keep) keep=1; shift ;;
        *) eg_die "unknown argument: $1" ;;
    esac
done

[ -n "$app" ] || eg_die "--app is required"
case "$app" in
    /*) ;;
    *) app="$repo_root/$app" ;;
esac
[ -d "$app" ] || eg_die "no bundle at $app"
eg_require_tools codesign install_name_tool otool od strings /usr/libexec/PlistBuddy
app=$(CDPATH= cd -- "$app" && pwd -P) || eg_die "cannot resolve the bundle path of $app"
[ -n "$library" ] || library=libQt6Core.6.12.0.dylib

work=$(mktemp -d "${TMPDIR:-/tmp}/everygraph-lgpl-XXXXXX")
if [ "$keep" -eq 0 ]; then
    trap 'rm -rf "$work"' EXIT
fi
report="$work/report.txt"
: >"$report"

eg_say() {
    printf '%s\n' "$*" | tee -a "$report"
}

eg_prepare() {
    local name=$1 hardened=$2 entitlements=$3
    local copy="$work/$name/everygraph_desktop.app"
    mkdir -p "$work/$name"
    cp -R "$app" "$copy"
    if [ "$hardened" = "1" ]; then
        "$script_dir"/sign_bundle.sh --app "$copy" --entitlements "$entitlements" --hardened \
            >"$work/$name-sign.log" 2>&1
    else
        "$script_dir"/sign_bundle.sh --app "$copy" --entitlements "$entitlements" --adhoc \
            >"$work/$name-sign.log" 2>&1
    fi
    printf '%s\n' "$copy"
}

# Replace the bundled LGPL library with a modified but interface-compatible
# build. The modification is one added LC_RPATH carrying a unique marker: it
# changes the file's bytes and breaks its signature seal while leaving every
# exported Qt symbol untouched, which is the substitution the licence protects.
eg_replace() {
    local copy=$1 marker=$2
    local target="$copy/Contents/Frameworks/$library"
    [ -f "$target" ] || eg_die "no $library in $copy"
    chmod 644 "$target"
    install_name_tool -add_rpath "@loader_path/$marker" "$target"
    eg_rpaths "$target" | grep -q -- "$marker" || eg_die "the marker rpath is missing from $target"
}

eg_dyld_error() {
    grep -m 1 -E 'dyld|Code Signature|library validation|Killed|aborting|Mach signal' "$1" 2>/dev/null || true
}

eg_run_case() {
    local label=$1 state=$2 copy=$3
    local log="$work/$label-launch.log"
    local status=0 seal=0
    if eg_launch_bounded "$copy/Contents/MacOS/everygraph_desktop" "$log" 60; then
        status=0
    else
        status=$?
    fi
    # The seal is reported next to the launch result because they are different
    # questions: codesign asks whether the bundle still matches its own signature,
    # dyld asks whether it will load. A recipient's substituted library breaks the
    # first and, depending on the signing state, not the second.
    if codesign --verify --deep --strict "$copy" >/dev/null 2>&1; then
        seal=0
    else
        seal=1
    fi
    eg_say "$(printf '%-32s %-30s exit=%-4s seal=%-7s %s' "$label" "$state" "$status" \
        "$([ "$seal" = 0 ] && printf 'valid' || printf 'broken')" "$(eg_dyld_error "$log")")"
}

eg_case() {
    local name=$1 hardened=$2 entitlements=$3
    local copy
    copy=$(eg_prepare "$name" "$hardened" "$entitlements")
    eg_say "$(printf '%-32s %s' "$name" "$(codesign -dvvv "$copy" 2>&1 | grep -m 1 'flags=')")"
    eg_run_case "$name/baseline" "baseline, untouched library" "$copy"

    eg_replace "$copy" "lgpl-replacement-$name"
    eg_say "$(printf '%-32s replaced sha256 %s' "$name" "$(eg_sha256 "$copy/Contents/Frameworks/$library")")"
    eg_run_case "$name/replaced-unsealed" "replaced, bundle not re-sealed" "$copy"

    codesign --force --sign - --timestamp=none "$copy/Contents/Frameworks/$library" \
        >>"$work/$name-sign.log" 2>&1
    if [ "$hardened" = "1" ]; then
        codesign --force --sign - --timestamp=none --options runtime \
            --entitlements "$entitlements" "$copy" >>"$work/$name-sign.log" 2>&1
    else
        codesign --force --sign - --timestamp=none "$copy" >>"$work/$name-sign.log" 2>&1
    fi
    eg_run_case "$name/replaced-resigned" "replaced, re-signed ad hoc" "$copy"
    eg_say ""
}

eg_trace() {
    local copy=$1 log=$2 tag=$3
    local status=0
    env -i HOME="$work" TMPDIR="$work" PATH=/usr/bin:/bin \
        DYLD_PRINT_LIBRARIES=1 EVERYGRAPH_PREFERENCES_PATH="$work/$tag-preferences.json" \
        "$copy/Contents/MacOS/everygraph_desktop" --smoke-test >"$log" 2>&1 || status=$?
    printf '%s' "$status"
}

eg_say "bundle under test: $app"
eg_say "library under test: Contents/Frameworks/$library"
eg_say "original sha256: $(eg_sha256 "$app/Contents/Frameworks/$library")"
eg_say ""

eg_case "hardened-shipped" 1 "$script_dir/entitlements.plist"
eg_case "hardened-strict" 1 "$script_dir/strict-library-validation.entitlements.plist"
eg_case "adhoc" 0 "$script_dir/entitlements.plist"

# Does the Qt code really come from inside the bundle? A hardened process strips
# DYLD_* variables, so the trace runs on an ad hoc non-hardened copy and the same
# variable is then set on a hardened copy to show it produces nothing.
# verify_bundle.sh proves the property for the shipped hardened bundle from its
# linkage instead: every dependency resolves to a file under
# Contents/Frameworks and no image keeps an absolute rpath.
trace_copy=$(eg_prepare "load-trace-adhoc" 0 "$script_dir/entitlements.plist")
hardened_copy=$(eg_prepare "load-trace-hardened" 1 "$script_dir/entitlements.plist")
status=$(eg_trace "$trace_copy" "$work/load-trace-adhoc.log" adhoc)
eg_say "ad hoc copy, DYLD_PRINT_LIBRARIES=1 (exit $status): $(grep -c '/Contents/Frameworks/' "$work/load-trace-adhoc.log") load(s) from inside the bundle"
grep -- '/Contents/Frameworks/' "$work/load-trace-adhoc.log" |
    sed -e 's|.*Contents/Frameworks/|  Contents/Frameworks/|' | sort -u | tee -a "$report"
status=$(eg_trace "$hardened_copy" "$work/load-trace-hardened.log" hardened)
eg_say "hardened copy, DYLD_PRINT_LIBRARIES=1 (exit $status): $(grep -c '/Contents/Frameworks/' "$work/load-trace-hardened.log") traced load(s); the hardened runtime strips the variable"
eg_say ""
eg_say "scratch copies and logs: $work (removed unless --keep)"
