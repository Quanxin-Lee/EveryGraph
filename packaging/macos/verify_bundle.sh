#!/usr/bin/env bash
# Verify a locally built macOS bundle: linkage, path hygiene, signature and a
# hermetic launch.
#
#   packaging/macos/verify_bundle.sh --app out/bundle/<preset>/everygraph_desktop.app
#                                    [--expect-hardened | --expect-adhoc-only]
#                                    [--no-content-scan] [--no-launch]
#
# Every check reads the bundle itself, not the build tree, so a passing run is
# evidence about the artifact a recipient would hold. Findings that are a
# distribution-privacy matter rather than a linkage defect are reported as WARN
# lines and do not fail the run.
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
# shellcheck source=macho_helpers.sh
. "$script_dir/macho_helpers.sh"

app=""
expect_hardened=1
content_scan=1
launch_check=1
vcpkg_installed="$repo_root/out/vcpkg-installed"

while [ $# -gt 0 ]; do
    case "$1" in
        --app) app=$2; shift 2 ;;
        --expect-hardened) expect_hardened=1; shift ;;
        --expect-adhoc-only) expect_hardened=0; shift ;;
        --no-content-scan) content_scan=0; shift ;;
        --no-launch) launch_check=0; shift ;;
        --vcpkg-installed) vcpkg_installed=$2; shift 2 ;;
        *) eg_die "unknown argument: $1" ;;
    esac
done

[ -n "$app" ] || eg_die "--app is required"
case "$app" in
    /*) ;;
    *) app="$repo_root/$app" ;;
esac
[ -d "$app" ] || eg_die "no bundle at $app"
# Canonicalised once, here: the linkage checks compare a resolved dependency
# against $frameworks, and eg_realpath turns /var/folders into
# /private/var/folders. Left lexical, a bundle reached through a symlinked or
# temporary path would report its own in-bundle dependencies as out-of-bundle.
app=$(CDPATH= cd -- "$app" && pwd -P) || eg_die "cannot resolve the bundle path of $app"
out_dir=$(dirname "$app")

eg_require_tools od otool install_name_tool codesign shasum strings python3 realpath grep find sed sort awk /usr/libexec/PlistBuddy

EG_EXE_DIR="$app/Contents/MacOS"
export EG_EXE_DIR
exe="$EG_EXE_DIR/everygraph_desktop"
frameworks="$app/Contents/Frameworks"
failures=0
passed=0
warnings=0

eg_check() {
    if [ "$1" = "0" ]; then
        passed=$((passed + 1))
        printf 'ok   %s\n' "$2"
    else
        failures=$((failures + 1))
        printf 'FAIL %s\n' "$2" >&2
    fi
}

eg_warn() {
    warnings=$((warnings + 1))
    printf 'WARN %s\n' "$1" >&2
}

# eg_expect <description> <command ...>: run a command, report its exit status.
eg_expect() {
    local description=$1
    shift
    local status=0
    "$@" || status=$?
    eg_check "$status" "$description"
}

plist_has_executable() {
    local declared
    declared=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$1/Contents/Info.plist")
    [ "$declared" = everygraph_desktop ] && [ -x "$1/Contents/MacOS/everygraph_desktop" ]
}

plist_declares_minimum() {
    /usr/libexec/PlistBuddy -c 'Print :LSMinimumSystemVersion' "$1" >/dev/null 2>&1
}

description_has() {
    printf '%s\n' "$CODESIGN_DESCRIPTION" | grep -q -- "$1"
}

# --- structure ---------------------------------------------------------------
plist="$app/Contents/Info.plist"
[ -f "$plist" ] || eg_die "no Info.plist in $app"
minimum=$(/usr/libexec/PlistBuddy -c 'Print :LSMinimumSystemVersion' "$plist" 2>/dev/null || echo 'none')
eg_expect "the bundle executable is everygraph_desktop and is executable" plist_has_executable "$app"
eg_expect "Info.plist declares LSMinimumSystemVersion ($minimum)" plist_declares_minimum "$plist"

# --- linkage -----------------------------------------------------------------
eg_dependencies_resolve_inside() {
    local image=$1 dep resolved bad=""
    while IFS= read -r dep; do
        [ -n "$dep" ] || continue
        if eg_is_system_install_name "$dep"; then
            continue
        fi
        case "$dep" in
            @rpath/*) ;;
            *) bad="$bad [$dep]"; continue ;;
        esac
        if ! resolved=$(eg_resolve "$image" "$dep"); then
            bad="$bad [$dep unresolvable]"
            continue
        fi
        case "$resolved" in
            "$frameworks"/*) ;;
            *)
                # @executable_path/../Frameworks and @loader_path/../../Frameworks
                # reach the same directory through a lexical hop, so compare the
                # canonical path dyld would load.
                if [ "$(eg_realpath "$resolved")" != "$frameworks/${resolved##*/}" ]; then
                    bad="$bad [$dep -> $resolved]"
                fi
                ;;
        esac
    done <<EOF
$(eg_deps "$image")
EOF
    printf '%s\n' "${bad:-}" >"$eg_bad_file"
    [ -z "$bad" ]
}

eg_no_absolute_rpath() {
    local rpath
    while IFS= read -r rpath; do
        [ -n "$rpath" ] || continue
        case "$rpath" in
            @*) ;;
            *) return 1 ;;
        esac
    done <<EOF
$(eg_rpaths "$1")
EOF
    return 0
}

eg_install_name_is_rpath() {
    local id
    id=$(eg_install_name "$1")
    [ -n "$id" ] && [ "${id#@rpath/}" != "$id" ]
}

# eg_dependency_parse_is_complete <image>: the load-command parse must account
# for every entry `otool -L` prints (minus the image's own id, which otool lists
# first and which is not a dependency). Two independent readers of the same
# table: without this, an image whose dependencies the parser failed to recognise
# would satisfy "every dependency resolves inside Contents/Frameworks" vacuously.
eg_dependency_parse_is_complete() {
    local image=$1 entries id read_count
    entries=$(eg_otool_entry_count "$image") || return 1
    [ -n "$entries" ] || return 1
    id=$(eg_install_name "$image")
    if [ -n "$id" ]; then
        entries=$((entries - 1))
    fi
    read_count=$(eg_deps "$image" | grep -c . || true)
    [ "$read_count" = "$entries" ]
}

eg_tmp=$(mktemp -d "${TMPDIR:-/tmp}/everygraph-verify-XXXXXX")
eg_bad_file="$eg_tmp/bad-dependencies"
trap 'rm -rf "$eg_tmp"' EXIT
# The linkage section below iterates over exactly this list, so a list that came
# back empty — because a Mach-O detection could not run — would make every
# per-image check pass without having read a single binary. The executable is
# asserted separately because it is the one image that must always be code.
macho_images=$(eg_list_macho "$app")
image_count=$(printf '%s\n' "$macho_images" | grep -c . || true)
eg_is_macho "$exe" || eg_die "the bundle executable $exe is not a Mach-O image"
if [ "$image_count" -gt 1 ]; then
    eg_check 0 "the bundle exposes $image_count Mach-O image(s): the executable and at least one nested library"
else
    eg_check 1 "the bundle exposes only $image_count Mach-O image(s); the read is not trusted"
fi
while IFS= read -r image; do
    [ -n "$image" ] || continue
    if eg_dependencies_resolve_inside "$image"; then
        eg_check 0 "${image#$app/}: every dependency resolves inside Contents/Frameworks"
    else
        eg_check 1 "${image#$app/}: unresolvable or out-of-bundle dependency$(cat "$eg_bad_file")"
    fi
    eg_expect "${image##*/}: no absolute LC_RPATH" eg_no_absolute_rpath "$image"
    eg_expect "${image##*/}: the dependency parse covers every load command" eg_dependency_parse_is_complete "$image"
    case "$image" in
        "$frameworks"/*) eg_expect "${image##*/}: install name is @rpath based" eg_install_name_is_rpath "$image" ;;
    esac
done <<EOF
$macho_images
EOF

# --- private paths anywhere in the bundle (distribution privacy) -------------
# The patterns are generic, not "the tree running this check": a bundle built in
# one directory and verified in another must report the same thing, and vendor
# libraries embed the directory they were compiled in. Every file under Contents
# is scanned, not only Mach-O images, because a plist, a _CodeSignature resource
# rule or a copied license text can carry a build path too. Whole matching lines
# are reported so a reader can act on the finding.
if [ "$content_scan" -eq 1 ]; then
    while IFS= read -r file; do
        [ -n "$file" ] || continue
        # grep status kept, not swallowed: exit 1 means "no private path", which
        # is the pass, while 2+ means the file could not be read — and a scan
        # that cannot read must never contribute zero warnings.
        scan_status=0
        private=$(LC_ALL=C grep -a -o -F -e '/Users/' -e '/home/' -e '/opt/homebrew' \
            -e '/usr/local/' -e '/private/var/folders/' -e 'vcpkg-installed/' \
            -e 'out/vcpkg/buildtrees' -e "$repo_root" "$file" 2>/dev/null |
            sort -u) || scan_status=$?
        if [ "$scan_status" -gt 1 ]; then
            eg_check 1 "the private-path scan of ${file#$app/} could not be read (exit $scan_status)"
            continue
        fi
        [ -n "$private" ] || continue
        if eg_is_macho "$file"; then
            string_status=0
            strings_hits=$(LC_ALL=C strings -a "$file" 2>/dev/null |
                grep -a -F -e '/Users/' -e '/home/' -e '/opt/homebrew' -e '/usr/local/' \
                            -e '/private/var/folders/' -e 'vcpkg-installed/' \
                            -e 'out/vcpkg/buildtrees' -e "$repo_root" | sort -u) || string_status=$?
            if [ "$string_status" -gt 1 ]; then
                eg_check 1 "the string scan of ${file#$app/} could not be read (exit $string_status)"
                continue
            fi
            detail=''
            while IFS= read -r hit; do
                [ -n "$hit" ] || continue
                detail="$detail${hit//"$repo_root"/<repo-root>}|"
            done <<EOF
$(printf '%s\n' "${strings_hits:-$private}" | sed -n '1,5p')
EOF
            eg_warn "${file#$app/} embeds private path strings in its binary content (build material, not linkage): $detail"
        else
            eg_warn "${file#$app/} contains a private path in a non-Mach-O file: $(printf '%s\n' "$private" | tr '\n' '|')"
        fi
    done <<EOF
$(find "$app/Contents" -type f | sort)
EOF
fi

# --- signature ---------------------------------------------------------------
status=0
codesign --verify --deep --strict --verbose=2 "$app" >"$out_dir/codesign-verify.log" 2>&1 || status=$?
eg_check "$status" "codesign --verify --deep --strict passes ($out_dir/codesign-verify.log)"

CODESIGN_DESCRIPTION=$(codesign -dvvv "$app" 2>&1)
eg_expect "the bundle signature is ad hoc with no team identifier" description_has 'TeamIdentifier=not set'
if [ "$expect_hardened" -eq 1 ]; then
    eg_expect "the bundle declares the hardened runtime flag" description_has 'runtime)'
    eg_note "codesign flags: $(printf '%s\n' "$CODESIGN_DESCRIPTION" | grep -m 1 'flags=')"
else
    if description_has 'runtime)'; then
        eg_check 1 "the bundle declares no hardened runtime"
    else
        eg_check 0 "the bundle declares no hardened runtime"
    fi
fi

# --- launch ------------------------------------------------------------------
if [ "$launch_check" -eq 1 ]; then
    status=0
    eg_launch_bounded "$exe" "$out_dir/launch-smoke.log" 60 || status=$?
    eg_check "$status" "the bundle launches from outside the build tree in an empty environment (exit $status)"
    if [ "$status" -ne 0 ]; then
        sed -n '1,20p' "$out_dir/launch-smoke.log" >&2
    fi
    # A hardened process sanitises DYLD_* variables, so the load trace is taken
    # from an unsigned-for-this-purpose copy of the same bundle layout: see
    # packaging/macos/lgpl_replace_test.sh, which records both outcomes.
fi

# --- inventory correspondence ------------------------------------------------
status=0
python3 "$script_dir/bundle_inventory.py" \
    --app "$app" \
    --vcpkg-installed "$vcpkg_installed" \
    --sbom "$repo_root/docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json" \
    --repo-root "$repo_root" \
    --output "$out_dir/bundle-inventory-verify.json" \
    --quiet >"$out_dir/bundle-inventory-verify.log" 2>&1 || status=$?
eg_check "$status" "every bundled library attributes to a pinned package at its SPDX version"

printf 'summary: %s passed, %s failed, %s warning(s)\n' "$passed" "$failures" "$warnings"
[ "$failures" -eq 0 ] || eg_die "bundle verification failed"
