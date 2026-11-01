#!/usr/bin/env bash
# Build a self-contained macOS .app bundle for the P1 workbench shell from the
# pinned dynamic Qt graph, then sign it locally ad hoc with hardened runtime.
#
#   packaging/macos/build_bundle.sh [--preset macos-release-desktop]
#                                  [--out out/bundle/<preset>]
#                                  [--entitlements packaging/macos/entitlements.plist]
#                                  [--sign-mode hardened|adhoc|none]
#
# The input is the CMake-produced bundle in out/build/<preset>; the output is a
# copy in out/bundle/<preset> (ignored generated output, never a source tree).
# Nothing here edits a build tree, and no Qt tool other than the repository's
# own dependency graph is used: the pinned install carries no macdeployqt for
# the target triplet, so the closure walk below is this project's deployer.
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
# shellcheck source=macho_helpers.sh
. "$script_dir/macho_helpers.sh"

preset="macos-release-desktop"
out_dir=""
entitlements="$script_dir/entitlements.plist"
sign_mode="hardened"
vcpkg_installed="$repo_root/out/vcpkg-installed"

while [ $# -gt 0 ]; do
    case "$1" in
        --preset) preset=$2; shift 2 ;;
        --out) out_dir=$2; shift 2 ;;
        --entitlements) entitlements=$2; shift 2 ;;
        --sign-mode) sign_mode=$2; shift 2 ;;
        --vcpkg-installed) vcpkg_installed=$2; shift 2 ;;
        *) eg_die "unknown argument: $1" ;;
    esac
done
EG_VCPKG_INSTALLED=$(CDPATH= cd -- "$vcpkg_installed" 2>/dev/null && pwd -P) ||
    eg_die "no vcpkg install root at $vcpkg_installed; configure --preset $preset first"
export EG_VCPKG_INSTALLED
# The install root holds both triplets; only the target one may be distributed.
EG_VCPKG_TARGET_TRIPLET=arm64-osx-dynamic
export EG_VCPKG_TARGET_TRIPLET

build_dir="$repo_root/out/build/$preset"
src_app="$build_dir/everygraph_desktop.app"
[ -d "$src_app" ] || eg_die "no shell bundle at $src_app; build --preset $preset first"
# Dependency discovery always reads the pristine install-tree files, whose
# @executable_path rpaths point at the build-tree bundle.
EG_EXE_DIR="$src_app/Contents/MacOS"
export EG_EXE_DIR
if [ -z "$out_dir" ]; then
    out_dir="$repo_root/out/bundle/$preset"
fi
case "$out_dir" in
    /*) ;;
    *) out_dir="$repo_root/$out_dir" ;;
esac
case "$entitlements" in
    /*) ;;
    *) entitlements="$repo_root/$entitlements" ;;
esac
[ -f "$entitlements" ] || eg_die "missing entitlements: $entitlements"
eg_require_disposable_dir "$out_dir" "$repo_root"

# The closure walk, the rpath rewrite and the signature each depend on one of
# these; a missing tool has to stop the build instead of emptying a list.
eg_require_tools od otool install_name_tool codesign shasum realpath plutil /usr/libexec/PlistBuddy

app="$out_dir/everygraph_desktop.app"
exe="$app/Contents/MacOS/everygraph_desktop"
frameworks="$app/Contents/Frameworks"
plugins="$app/Contents/PlugIns"
resources="$app/Contents/Resources"
work=$(mktemp -d "${TMPDIR:-/tmp}/everygraph-bundle-XXXXXX")
trap 'rm -rf "$work"' EXIT

seen="$work/seen"
queue="$work/queue"
: >"$seen"
: >"$queue"

eg_copy_library() {
    # Copy one resolved dependency into Contents/Frameworks once and record the
    # source -> install-name pair the inventory step later maps to a package.
    local src=$1 base=$2 recorded
    recorded=$(eg_seen_source "$seen" "$base")
    if [ -n "$recorded" ]; then
        [ "$recorded" = "$src" ] || eg_die "two dependencies share the name $base ($recorded and $src)"
        return 0
    fi
    printf '%s\t%s\n' "$base" "$src" >>"$seen"
    cp "$src" "$frameworks/$base"
    chmod 644 "$frameworks/$base"
    eg_strip_signature "$frameworks/$base"
    printf '%s\t%s\n' "$src" "$frameworks/$base" >>"$queue"
}

eg_deploy_from() {
    # Rewrite every non-system dependency of one bundle Mach-O to @rpath and pull
    # its file into the bundle. Discovery reads the pristine dependency-tree file,
    # whose rpaths are the only way to resolve its @rpath install names; rewriting
    # is applied to the bundle copy. Soname symlinks (libQt6Gui.6.dylib) are
    # followed to their real file and rewritten to it, so the bundle holds one
    # physical copy per library: two copies of QtCore would give two instances.
    local src=$1 target=$2 dep resolved real base
    while IFS= read -r dep; do
        [ -n "$dep" ] || continue
        if eg_is_system_install_name "$dep"; then
            continue
        fi
        if ! resolved=$(eg_resolve "$src" "$dep"); then
            eg_die "$target references $dep, which resolves through no rpath of $src"
        fi
        real=$(eg_realpath "$resolved")
        eg_is_pinned_dependency "$real" ||
            eg_die "$target needs $real, outside $EG_VCPKG_INSTALLED (a Homebrew or system-absolute leak)"
        [ -f "$real" ] || eg_die "$target needs $real, which does not exist"
        base=${real##*/}
        eg_copy_library "$real" "$base"
        eg_install_name_tool -change "$dep" "@rpath/$base" "$target"
    done <<EOF
$(eg_deps "$src")
EOF
}

# --- stage 1: copy the CMake-produced bundle --------------------------------
eg_note "staging $src_app -> $app"
rm -rf "$out_dir"
mkdir -p "$out_dir"
cp -R "$src_app" "$app"
eg_strip_signature "$exe"
mkdir -p "$frameworks" "$plugins" "$resources"

# --- stage 2: the executable's library closure ------------------------------
# The executable's own rpaths are the build tree's, so resolve from the source
# file and rewrite the staged copy.
eg_deploy_from "$src_app/Contents/MacOS/everygraph_desktop" "$exe"

# --- stage 3: the queued dylibs, until the closure is empty -----------------
# Each pass drains the queue and enqueues whatever it discovers, so the walk
# terminates when a pass adds no new library name.
while [ -s "$queue" ]; do
    mv "$queue" "$work/current"
    while IFS=$(printf '\t') read -r src target; do
        [ -n "${src:-}" ] || continue
        eg_deploy_from "$src" "$target"
    done <"$work/current"
done

# --- stage 4: Qt plugins ----------------------------------------------------
# The pinned Qt keeps its plugin set under <prefix>/Qt6/plugins (debug builds
# under <prefix>/debug/Qt6/plugins). macdeployqt-equivalent coverage is not the
# goal: this bundle carries the platform and style plugins the shell actually
# loads. Image-format plugins join when a task adds image import.
# The substitution carries `|| true`: under pipefail a command substitution that
# fails would end the script before the diagnostic below can name the binary
# whose rpath is missing.
build_rpath=$(eg_rpaths "$src_app/Contents/MacOS/everygraph_desktop" | grep '^/' | sed -n '1p' || true)
[ -n "$build_rpath" ] || eg_die "the shell binary carries no absolute dependency rpath to locate the Qt prefix"
# A debug build installs under <prefix>/debug/lib and keeps its plugins in
# <prefix>/debug/Qt6/plugins with a _debug suffix, so the prefix and the plugin
# directory are read off the rpath itself rather than guessed from the preset.
case "$build_rpath" in
    */debug/lib) plugin_prefix="${build_rpath%/debug/lib}/debug/Qt6/plugins" ;;
    */lib) plugin_prefix="${build_rpath%/lib}/Qt6/plugins" ;;
    *) eg_die "unexpected Qt library rpath: $build_rpath" ;;
esac
[ -d "$plugin_prefix" ] || eg_die "no Qt plugin directory at $plugin_prefix"

plugin_selected=""
for spec in "platforms/libqcocoa" "platforms/libqoffscreen" "styles/libqmacstyle"; do
    for candidate in "$plugin_prefix/${spec}_debug.dylib" "$plugin_prefix/${spec}.dylib"; do
        [ -f "$candidate" ] || continue
        dir=${spec%/*}
        base=${candidate##*/}
        mkdir -p "$plugins/$dir"
        cp "$candidate" "$plugins/$dir/$base"
        chmod 644 "$plugins/$dir/$base"
        eg_strip_signature "$plugins/$dir/$base"
        eg_deploy_from "$candidate" "$plugins/$dir/$base"
        plugin_selected="$plugin_selected $dir/$base"
        break
    done
done
[ -n "$plugin_selected" ] || eg_die "no Qt platform plugin was found under $plugin_prefix"


# --- stage 5: rpath hygiene -------------------------------------------------
# The install tree's relative rpaths (@loader_path, @loader_path/../../../lib)
# describe the vcpkg layout, not the bundle's, and its absolute ones are
# build-machine paths. Each image ends with exactly one rpath that reaches
# Contents/Frameworks from where that image sits in the bundle.
eg_fix_rpaths() {
    local image=$1 loader=$2 rpath
    while IFS= read -r rpath; do
        [ -n "$rpath" ] || continue
        eg_install_name_tool -delete_rpath "$rpath" "$image"
    done <<EOF
$(eg_rpaths "$image")
EOF
    # Newline-delimited exact membership rather than a grep pattern: an rpath
    # carries glob characters, and the loop above may well have just deleted this
    # very value, so the test has to read what the image reports now.
    case $'\n'"$(eg_rpaths "$image")"$'\n' in
        *$'\n'"$loader"$'\n'*) ;;
        *) eg_install_name_tool -add_rpath "$loader" "$image" ;;
    esac
}

eg_fix_rpaths "$exe" "@executable_path/../Frameworks"

# eg_bundle_loader_rpath <image>: the one rpath that reaches
# Contents/Frameworks from wherever this image sits inside Contents. Deriving it
# from the image's depth means an image in any other bundle directory — a helper
# under MacOS, a resource plugin, a deeper PlugIns path — still gets a usable
# search path instead of keeping the build tree's absolute rpath, and an image
# outside Contents is a deployer bug rather than something to skip silently.
eg_bundle_loader_rpath() {
    local image=$1 rel dir ascent=""
    case "$image" in
        "$app/Contents/"*) ;;
        *) return 1 ;;
    esac
    rel=${image#"$app/Contents/"}
    dir=${rel%/*}
    [ "$dir" != "$rel" ] || dir=""
    # The deployer copies every library flat into Contents/Frameworks, so an
    # image in that directory resolves its @rpath dependencies from its own
    # directory and needs no ascent at all.
    [ "$dir" != Frameworks ] || { printf '@loader_path\n'; return 0; }
    # One "../" per directory level between Contents and the image's own folder.
    # Counted by stripping, so no IFS change and no glob expansion of a path
    # component is possible.
    while [ "$dir" != "${dir#*/}" ]; do
        ascent="$ascent../"
        dir=${dir#*/}
    done
    if [ -n "$dir" ]; then ascent="$ascent../"; fi
    printf '@loader_path/%sFrameworks\n' "$ascent"
}

while IFS= read -r image; do
    case "$image" in
        "$exe") continue ;;
    esac
    loader=$(eg_bundle_loader_rpath "$image") ||
        eg_die "$image is a Mach-O outside $app/Contents; the deployer has no rpath rule for it"
    eg_fix_rpaths "$image" "$loader"
done <<EOF
$(eg_list_macho "$app")
EOF

# --- stage 6: install names -------------------------------------------------
# Rewriting each dylib's own id to @rpath/<name> is what makes a recipient's
# rebuilt library drop in without relinking the application.
while IFS=$(printf '\t') read -r base src; do
    [ -n "${base:-}" ] || continue
    eg_install_name_tool -id "@rpath/$base" "$frameworks/$base"
done <"$seen"

# --- stage 7: license notices ----------------------------------------------
eg_note "collecting notices and the bundle inventory"
python3 "$script_dir/bundle_inventory.py" \
    --app "$app" \
    --seen "$seen" \
    --vcpkg-installed "$EG_VCPKG_INSTALLED" \
    --sbom "$repo_root/docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json" \
    --repo-root "$repo_root" \
    --output "$out_dir/bundle-inventory.json" \
    --notices

# --- stage 8: sign ----------------------------------------------------------
case "$sign_mode" in
    hardened) "$script_dir/sign_bundle.sh" --app "$app" --entitlements "$entitlements" --hardened ;;
    adhoc) "$script_dir/sign_bundle.sh" --app "$app" --entitlements "$entitlements" --adhoc ;;
    none) eg_note "signing skipped (--sign-mode none)" ;;
    *) eg_die "unknown --sign-mode: $sign_mode" ;;
esac

eg_note "bundle: $app"
eg_note "plugins:$plugin_selected"
