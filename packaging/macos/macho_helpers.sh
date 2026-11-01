# Shared Mach-O and bundle helpers for the macOS packaging scripts.
# Sourced, not executed. bash 3.2 compatible: no associative arrays, no mapfile.

eg_die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

eg_note() {
    printf '%s\n' "$*" >&2
}

# eg_require_tools <tool ...>: a missing tool must end the run, not shorten a
# file list. Every Mach-O, linkage and signature check below iterates over what
# these tools report, so on a host without `od` the list is simply empty and a
# verifier would report "every dependency resolves inside the bundle" for a
# bundle whose binaries it never read.
eg_require_tools() {
    local tool
    for tool in "$@"; do
        command -v "$tool" >/dev/null 2>&1 || eg_die "$tool is required but was not found on PATH"
    done
}

# eg_require_disposable_dir <path> <repo-root>: guard before a recursive delete.
# The builders wipe their output directory, so a mistyped --out or --export-dir
# would otherwise take a source tree or a home directory with it. Only generated
# output is disposable: the repository's out/ tree or a temporary directory.
eg_require_disposable_dir() {
    local path=${1%/} root=$2 parent
    [ -n "$path" ] && [ "$path" != "/" ] || eg_die "refusing to treat '$1' as disposable output"
    parent=${path%/*}
    [ -n "$parent" ] || eg_die "refusing to treat '$1' as disposable output"
    case "$path" in
        "$root"/out/* | /tmp/* | /private/tmp/* | /var/folders/* | /private/var/folders/*) return 0 ;;
        *) ;;
    esac
    eg_die "$path is neither under $root/out nor under a temporary directory; refusing to delete it"
}

# Mach-O and fat-binary magics, as the four bytes actually stored in the file.
# otool -h is not the test: on this toolchain it prints "is not an object file"
# for a non-Mach-O input and still exits 0.
eg_is_macho() {
    local magic
    [ -f "$1" ] || return 1
    magic=$(od -An -tx1 -N4 "$1" 2>/dev/null | tr -d ' \n')
    case "$magic" in
        cffaedfe | cefaedfe | feedfacf | feedface | bebafeca | bfbafeca | cafebabe | cafebabf)
            return 0 ;;
        *) return 1 ;;
    esac
}

# eg_list_macho <dir>: every Mach-O file below <dir>, sorted, NUL-free (paths must not contain newlines).
eg_list_macho() {
    local candidate
    find "$1" -type f | sort | while IFS= read -r candidate; do
        if eg_is_macho "$candidate"; then
            printf '%s\n' "$candidate"
        fi
    done
}

# eg_deps <macho>: the install names every dylib load command of one Mach-O
# refers to, one per line. otool -L would add the image's own LC_ID_DYLIB as its
# first entry, which is not a dependency, so the load commands are parsed. All
# four loading command types are read: an image that re-exports or weak-binds a
# dependency must still have that dependency inside the bundle.
eg_deps() {
    otool -l "$1" | awk '
        /cmd LC_(LOAD_DYLIB|LOAD_WEAK_DYLIB|REEXPORT_DYLIB|LOAD_UPWARD_DYLIB)/ { want = 1; next }
        want && /^[[:space:]]*name / {
            value = $0
            sub(/^[[:space:]]*name /, "", value)
            sub(/ \(offset.*/, "", value)
            print value
            want = 0
        }'
}

# eg_load_command_count <macho>: how many dylib load commands the image carries,
# so a caller can tell "no dependencies" from "the parser read nothing".
eg_load_command_count() {
    otool -l "$1" | grep -c 'cmd LC_\(LOAD_DYLIB\|LOAD_WEAK_DYLIB\|REEXPORT_DYLIB\|LOAD_UPWARD_DYLIB\)'
}

# eg_otool_entry_count <macho>: the entries `otool -L` prints, which include the
# image's own id. Comparing the two counts catches a parser that silently
# stopped matching a load-command label. A failed `otool -L` is reported as a
# failure rather than as zero entries, because zero entries and "read nothing"
# would otherwise satisfy the comparison the same way.
eg_otool_entry_count() {
    local output
    if ! output=$(otool -L "$1" 2>&1); then
        printf '%s\n' "$output" >&2
        return 1
    fi
    printf '%s\n' "$output" | tail -n +2 | grep -c '^[[:space:]]' || true
}

# eg_install_name <macho>: the LC_ID_DYLIB value, empty for an executable.
eg_install_name() {
    otool -DX "$1" 2>/dev/null | sed -e 's/^[[:space:]]*//' -e '/^$/d'
}

# eg_rpaths <macho>: every LC_RPATH value exactly as the image stores it.
eg_rpaths() {
    otool -l "$1" | awk '/cmd LC_RPATH/{want=1;next} want && /path /{print $2; want=0}'
}

# eg_rpath_dirs <macho>: the LC_RPATH values with their load tokens expanded.
# dyld expands @loader_path against the image's own directory and @executable_path
# against the main executable, which the caller publishes in EG_EXE_DIR. The
# pinned install tree uses relative rpaths (@loader_path, @loader_path/../../../lib),
# so expanding them is what lets the closure walk find a dependency's file.
eg_rpath_dirs() {
    local rpath
    while IFS= read -r rpath; do
        [ -n "$rpath" ] || continue
        case "$rpath" in
            @loader_path/*) printf '%s/%s\n' "$(dirname -- "$1")" "${rpath#@loader_path/}" ;;
            @loader_path) printf '%s\n' "$(dirname -- "$1")" ;;
            @executable_path/*)
                [ -n "${EG_EXE_DIR:-}" ] || continue
                printf '%s/%s\n' "$EG_EXE_DIR" "${rpath#@executable_path/}" ;;
            @*) ;;
            *) printf '%s\n' "$rpath" ;;
        esac
    done <<EOF
$(eg_rpaths "$1")
EOF
}

# eg_resolve <macho> <install-name>: absolute path of the file dyld would load.
# @rpath values resolve against the image's expanded rpaths, @loader_path against
# its directory; @executable_path and an unresolvable name return 1 so the caller
# fails loudly instead of guessing.
eg_resolve() {
    local macho="$1" dep="$2" dir candidate
    case "$dep" in
        @rpath/*)
            while IFS= read -r dir; do
                [ -n "$dir" ] || continue
                candidate="$dir/${dep#@rpath/}"
                if [ -f "$candidate" ]; then
                    printf '%s\n' "$candidate"
                    return 0
                fi
            done <<EOF
$(eg_rpath_dirs "$macho")
EOF
            return 1
            ;;
        @loader_path/*)
            candidate="$(dirname -- "$macho")/${dep#@loader_path/}"
            if [ -f "$candidate" ]; then
                printf '%s\n' "$candidate"
                return 0
            fi
            return 1
            ;;
        @*)
            return 1
            ;;
        *)
            printf '%s\n' "$dep"
            ;;
    esac
}

eg_is_system_install_name() {
    case "$1" in
        /usr/lib/*|/System/Library/*|/usr/sbin/*|/System/iOSSupport/*) return 0 ;;
        *) return 1 ;;
    esac
}

eg_realpath() {
    command -v realpath >/dev/null 2>&1 || eg_die "realpath is required to resolve the library closure"
    realpath "$1"
}

eg_is_pinned_dependency() {
    # True only for a file under the *target* triplet tree of the install root the
    # caller declared in EG_VCPKG_INSTALLED. The install root also holds the
    # arm64-osx host tree, its tools and the unpacked packages cache, and none of
    # those may enter a distributed bundle: a host-triplet dylib would be an
    # unreadable architecture for the shipped process, and a packages-cache copy
    # would let a stale build satisfy the check.
    [ -n "${EG_VCPKG_INSTALLED:-}" ] || eg_die "EG_VCPKG_INSTALLED is not set"
    [ -n "${EG_VCPKG_TARGET_TRIPLET:-}" ] || eg_die "EG_VCPKG_TARGET_TRIPLET is not set"
    case "$1" in
        "$EG_VCPKG_INSTALLED/$EG_VCPKG_TARGET_TRIPLET"/*) return 0 ;;
        *) return 1 ;;
    esac
}

# eg_seen_source <records-file> <basename>: the source recorded for that name,
# or empty. Exact field match: a basename contains regex metacharacters.
eg_seen_source() {
    awk -F'\t' -v b="$2" '$1==b{print $2; exit}' "$1"
}

eg_sha256() {
    shasum -a 256 "$1" | cut -d' ' -f1
}

# eg_launch_bounded <exe> <log> [seconds] [KEY=VALUE ...]: run the shell's smoke
# test with an empty environment so no developer settings, Home directory or
# PATH leaks into the result, and return the executable's exit status (124 when
# it had to be killed). EVERYGRAPH_PREFERENCES_PATH is pinned to a scratch file
# so a verification run never writes the developer's real preferences.
eg_launch_bounded() {
    local exe=$1 log=$2 limit=${3:-60} sandbox pid waited status
    [ $# -ge 3 ] && shift 3
    sandbox=$(mktemp -d "${TMPDIR:-/tmp}/everygraph-launch-XXXXXX")
    env -i HOME="$sandbox" TMPDIR="$sandbox" PATH=/usr/bin:/bin \
        EVERYGRAPH_PREFERENCES_PATH="$sandbox/preferences.json" "$@" \
        "$exe" --smoke-test >"$log" 2>&1 &
    pid=$!
    waited=0
    status=0
    while kill -0 "$pid" 2>/dev/null; do
        if [ "$waited" -ge "$limit" ]; then
            kill -9 "$pid" 2>/dev/null || true
            status=124
            break
        fi
        sleep 1
        waited=$((waited + 1))
    done
    if [ "$status" -ne 124 ]; then
        wait "$pid" || status=$?
    fi
    rm -rf "$sandbox"
    return "$status"
}

eg_install_name_tool() {
    local status=0 output=""
    output=$(install_name_tool "$@" 2>&1) || status=$?
    if [ "$status" -ne 0 ]; then
        eg_die "install_name_tool $* failed (exit $status): $output"
    fi
    if [ -n "$output" ]; then
        printf '%s\n' "$output" >&2
    fi
}

eg_strip_signature() {
    # A Mach-O copied out of the dependency tree keeps the signature it was built
    # with, and rewriting it invalidates that seal. Remove it here and let
    # sign_bundle.sh apply the bundle's own signature last.
    codesign --remove-signature "$1" >/dev/null 2>&1 || true
}
