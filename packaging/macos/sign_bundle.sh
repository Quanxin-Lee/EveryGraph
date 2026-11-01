#!/usr/bin/env bash
# Sign a macOS bundle locally: ad hoc, with or without the hardened runtime.
#
#   packaging/macos/sign_bundle.sh --app out/bundle/<preset>/everygraph_desktop.app
#                                  [--entitlements packaging/macos/entitlements.plist]
#                                  [--hardened | --adhoc]
#
# There is no Developer ID identity and no timestamp request here: the signing
# is local and ad hoc, so `--timestamp=none` keeps the whole route offline.
# Nested code is signed inside out, one binary at a time, because `codesign
# --deep` is not a substitute for that and Apple deprecated it for signing.
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
# shellcheck source=macho_helpers.sh
. "$script_dir/macho_helpers.sh"

app=""
entitlements="$script_dir/entitlements.plist"
hardened=0
sign_identity="-"

while [ $# -gt 0 ]; do
    case "$1" in
        --app) app=$2; shift 2 ;;
        --entitlements) entitlements=$2; shift 2 ;;
        --hardened) hardened=1; shift ;;
        --adhoc) hardened=0; shift ;;
        *) eg_die "unknown argument: $1" ;;
    esac
done

[ -n "$app" ] || eg_die "--app is required"
case "$app" in
    /*) ;;
    *) app="$repo_root/$app" ;;
esac
[ -d "$app" ] || eg_die "no bundle at $app"
case "$entitlements" in
    /*) ;;
    *) entitlements="$repo_root/$entitlements" ;;
esac
[ -f "$entitlements" ] || eg_die "missing entitlements: $entitlements"
eg_require_tools codesign plutil od otool /usr/libexec/PlistBuddy

exe_dir="$app/Contents/MacOS"
main_exe="$exe_dir/$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$app/Contents/Info.plist")"
[ -x "$main_exe" ] || eg_die "no executable at $main_exe"
# Signing is only evidence about a bundle whose executable really is code. A
# Mach-O detection that read nothing would otherwise leave a non-binary
# executable sealed and counted as a signed bundle.
eg_is_macho "$main_exe" || eg_die "the bundle executable $main_exe is not a Mach-O image"
EG_EXE_DIR="$exe_dir"
export EG_EXE_DIR
runtime=""
if [ "$hardened" -eq 1 ]; then
    runtime="--options runtime"
fi

# Contents/Resources holds no code, so the nested binaries are the executable,
# the frameworks and the plug-ins.
nested=$(eg_list_macho "$app")
count=0
while IFS= read -r image; do
    [ -n "$image" ] || continue
    case "$image" in
        "$exe_dir"/*) continue ;;
    esac
    codesign --remove-signature "$image" >/dev/null 2>&1 || true
    codesign --force --sign "$sign_identity" --timestamp=none $runtime "$image"
    count=$((count + 1))
done <<EOF
$nested
EOF

# A code directory that holds files but yields no Mach-O to sign is a detection
# failure, not an unsigned-code-free bundle.
for code_dir in "$app/Contents/Frameworks" "$app/Contents/PlugIns"; do
    if [ "$count" -eq 0 ] && [ -n "$(find "$code_dir" -type f -print -quit 2>/dev/null)" ]; then
        eg_die "$code_dir holds files but no Mach-O was detected; refusing to seal an unread bundle"
    fi
done

codesign --remove-signature "$main_exe" >/dev/null 2>&1 || true
codesign --force --sign "$sign_identity" --timestamp=none $runtime \
    --entitlements "$entitlements" "$main_exe"
codesign --force --sign "$sign_identity" --timestamp=none $runtime \
    --entitlements "$entitlements" "$app"

eg_note "signed $count nested Mach-O file(s) plus the executable and bundle"
eg_note "mode: $([ "$hardened" -eq 1 ] && printf 'ad hoc + hardened runtime' || printf 'ad hoc only')"
eg_note "entitlements: $(plutil -convert xml1 -o - "$entitlements" | grep -c '<key>' ) key(s) declared"
