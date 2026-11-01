#!/usr/bin/env bash
# Recipient rebuild route for the macOS shell (P1-012 AC-8).
#
#   packaging/macos/rebuild_from_source.sh [--revision HEAD]
#                                          [--export-dir out/p1012-recipient]
#                                          [--preset macos-release-desktop]
#                                          [--vcpkg-installed out/vcpkg-installed]
#                                          [--allow-missing-qt-archive]
#
# This is the route a recipient of the open-source project actually uses: take
# the committed source at a revision, resolve the pinned dependency graph, build
# the shell, bundle it, sign it locally and substitute a rebuilt LGPL Qt library
# without relinking. The exported tree holds only tracked files, so the run
# cannot depend on an uncommitted local edit.
#
# The dependency graph is reused from the install root the caller names: it is
# the same baseline, manifest, overlay and versions, and the script checks the
# Qt source archive's SHA-512 against the recorded pin before it builds, so
# "reused" is not "assumed". A cold-cache rebuild from the archive is the same
# commands with an empty install root and costs the Qt compile.
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
# shellcheck source=macho_helpers.sh
. "$script_dir/macho_helpers.sh"

revision=HEAD
export_dir=""
preset=macos-release-desktop
vcpkg_installed="$repo_root/out/vcpkg-installed"
allow_missing_archive=0

while [ $# -gt 0 ]; do
    case "$1" in
        --revision) revision=$2; shift 2 ;;
        --export-dir) export_dir=$2; shift 2 ;;
        --preset) preset=$2; shift 2 ;;
        --vcpkg-installed) vcpkg_installed=$2; shift 2 ;;
        --allow-missing-qt-archive) allow_missing_archive=1; shift ;;
        *) eg_die "unknown argument: $1" ;;
    esac
done

command -v cmake >/dev/null || eg_die "cmake is not on PATH"
command -v ninja >/dev/null || eg_die "ninja is not on PATH"
eg_require_tools git tar shasum od otool install_name_tool codesign /usr/libexec/PlistBuddy
[ -n "${VCPKG_ROOT:-}" ] || eg_die "VCPKG_ROOT must point at the vcpkg checkout"
[ -d "$VCPKG_ROOT" ] || eg_die "no vcpkg checkout at $VCPKG_ROOT"
if [ -z "$export_dir" ]; then
    export_dir="$repo_root/out/p1012-recipient"
fi
[ ! -f "$export_dir" ] || eg_die "--export-dir must be a directory"
case "$export_dir" in
    /*) ;;
    *) export_dir="$repo_root/$export_dir" ;;
esac
eg_require_disposable_dir "$export_dir" "$repo_root"
case "$vcpkg_installed" in
    /*) ;;
    *) vcpkg_installed="$repo_root/$vcpkg_installed" ;;
esac
[ -d "$vcpkg_installed" ] || eg_die "no dependency install root at $vcpkg_installed"

source_dir="$export_dir/source"
commit=$(git -C "$repo_root" rev-parse "$revision") || eg_die "unknown revision: $revision"
eg_note "recipient source revision: $commit"

# --- stage 1: export the committed source, nothing else ----------------------
rm -rf "$export_dir"
mkdir -p "$source_dir"
git -C "$repo_root" archive --format=tar "$commit" | tar -x -C "$source_dir"
if [ ! -d "$source_dir/apps/desktop" ]; then
    eg_die "the exported tree has no apps/desktop; wrong revision?"
fi
if [ ! -f "$source_dir/packaging/macos/build_bundle.sh" ]; then
    eg_note "the exported revision does not yet carry packaging/macos; using this task's working copy"
    mkdir -p "$source_dir/packaging/macos"
    cp "$script_dir"/*.sh "$source_dir/packaging/macos/"
    cp "$script_dir"/bundle_inventory.py "$source_dir/packaging/macos/"
    cp "$script_dir"/*.plist "$source_dir/packaging/macos/"
    chmod +x "$source_dir"/packaging/macos/*.sh
fi

# --- stage 2: the pinned source and graph are the recorded ones --------------
port_data=$(sed -n 's/^set(qtbase_HASH "\(.*\)")$/\1/p' "$source_dir/cmake/vcpkg-overlay-ports/qtbase/port.data.cmake" | head -1)
port_file=$(sed -n 's/^set(qtbase_FILENAME "\(.*\)")$/\1/p' "$source_dir/cmake/vcpkg-overlay-ports/qtbase/port.data.cmake" | head -1)
baseline=$(sed -n 's/.*"builtin-baseline": "\(.*\)".*/\1/p' "$source_dir/vcpkg.json" | head -1)
[ -n "$port_data" ] && [ -n "$port_file" ] && [ -n "$baseline" ] || eg_die "cannot read the Qt pin from the exported tree"
archive=$(find "$VCPKG_ROOT/downloads" -maxdepth 1 -name "$port_file" -print | head -1)
if [ -n "$archive" ]; then
    actual=$(shasum -a 512 "$archive" | cut -d' ' -f1)
    [ "$actual" = "$port_data" ] || eg_die "$archive has SHA-512 $actual, the pin says $port_data"
    eg_note "Qt source archive $port_file verified against the recorded SHA-512"
elif [ "$allow_missing_archive" -eq 1 ]; then
    eg_note "Qt source archive $port_file is absent; verification skipped on request (--allow-missing-qt-archive)"
else
    # The recorded route claims the pinned source is the one being built, and the
    # archive hash is the only reading of that claim available before a cold-cache
    # compile. Absent archive therefore stops the run rather than degrading the
    # claim to "the install root was reused as recorded".
    eg_die "the pinned Qt archive $port_file is not under $VCPKG_ROOT/downloads, so its SHA-512 cannot be checked; pass --allow-missing-qt-archive to reuse the install root without that check"
fi
eg_note "vcpkg baseline $baseline; install root $vcpkg_installed"
for triplet in arm64-osx-dynamic arm64-osx; do
    [ -d "$vcpkg_installed/$triplet" ] || eg_die "the install root has no $triplet tree"
done

# --- stage 3: configure, build, bundle and verify ----------------------------
cd "$source_dir"
cmake --preset "$preset" -DVCPKG_INSTALLED_DIR="$vcpkg_installed"
cmake --build --preset "$preset"
ctest --preset "$preset"

recipient_bundle="$source_dir/out/bundle/$preset/everygraph_desktop.app"
"$source_dir/packaging/macos/build_bundle.sh" --preset "$preset" \
    --vcpkg-installed "$vcpkg_installed"
[ -d "$recipient_bundle" ] || eg_die "no recipient bundle at $recipient_bundle"
"$source_dir/packaging/macos/verify_bundle.sh" --app "$recipient_bundle" \
    --vcpkg-installed "$vcpkg_installed"

# --- stage 4: compare with the reference bundle before any substitution ------
# The byte-identity of a recipient's bundle and the shipped one is AC-8's
# evidence, so it is asserted, not reported. Paths are hashed NUL-separated and
# the sorted output compared, because a library name is not a shell word; an
# empty file list would hash empty stdin on both sides and match trivially.
eg_bundle_hashes() {
    local root=$1 count
    count=$(find "$root/Contents" -type f -print0 | tr -cd '\0' | wc -c | tr -d ' ')
    [ "$count" -gt 0 ] || eg_die "no files under $root/Contents to compare"
    (cd "$root" && find Contents -type f -print0 | xargs -0 shasum -a 256) | LC_ALL=C sort
}

reference_bundle="$repo_root/out/bundle/$preset/everygraph_desktop.app"
if [ -d "$reference_bundle" ]; then
    reference=$(eg_bundle_hashes "$reference_bundle")
    rebuilt=$(eg_bundle_hashes "$recipient_bundle")
    if [ "$reference" = "$rebuilt" ]; then
        eg_note "the recipient bundle is byte-identical to the reference bundle"
    else
        printf 'files differing from the reference bundle:\n' >&2
        diff <(printf '%s\n' "$reference") <(printf '%s\n' "$rebuilt") | sed -e 's/^/  /' >&2 || true
        eg_die "the recipient bundle differs from $reference_bundle; rebuild the reference with the same revision before comparing"
    fi
else
    eg_note "no reference bundle at $reference_bundle; comparison skipped"
fi

# --- stage 5: substitution without relinking ---------------------------------
# Nothing in this step rebuilds or relinks the application: the bundled Qt
# library is replaced by a modified build of the same interface-compatible
# library and the recipient re-signs locally.
library=libQt6Core.6.12.0.dylib
target="$recipient_bundle/Contents/Frameworks/$library"
[ -f "$target" ] || eg_die "no $library in the recipient bundle"
chmod 644 "$target"
before=$(eg_sha256 "$target")
install_name_tool -add_rpath "@loader_path/lgpl-recipient-rebuild" "$target"
eg_rpaths "$target" | grep -q -- lgpl-recipient-rebuild || eg_die "the marker rpath is missing"
after=$(eg_sha256 "$target")
[ "$before" != "$after" ] || eg_die "the replaced library is byte-identical to the original"
codesign --force --sign - --timestamp=none "$target"
codesign --force --sign - --timestamp=none --options runtime \
    --entitlements "$source_dir/packaging/macos/entitlements.plist" "$recipient_bundle"
application_binary="$recipient_bundle/Contents/MacOS/everygraph_desktop"
status=0
if eg_launch_bounded "$application_binary" "$export_dir/recipient-launch.log" 60; then
    eg_note "the recipient bundle launches with the substituted Qt library (exit 0)"
else
    status=$?
    eg_die "the substituted recipient bundle failed to launch (exit $status); see $export_dir/recipient-launch.log"
fi
codesign --verify --deep --strict "$recipient_bundle"
eg_note "signature still verifies after the substitution (--deep --strict)"

eg_note "recipient bundle: $recipient_bundle"
