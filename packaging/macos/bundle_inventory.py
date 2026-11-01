#!/usr/bin/env python3
"""Inventory a built macOS bundle and write its license notices.

The bundle is described from what is actually inside it: every Mach-O file is
hashed and attributed to the vcpkg package that installed it, using the package
file lists under ``<install>/vcpkg/info``. Versions and license expressions are
cross-checked against the accepted SPDX document, so this inventory cannot
silently disagree with the pinned graph.

Paths written into the output are bundle-relative or repository-relative: the
build machine's directory, its home directory and its temporary directory must
not appear in the evidence.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys

TRIPLET = "arm64-osx-dynamic"
SYSTEM_PREFIXES = ("/usr/lib/", "/System/Library/", "/usr/sbin/", "/System/iOSSupport/")
FORBIDDEN_FRAGMENTS = ("/Users/", "/home/", "/opt/homebrew", "/usr/local/", "vcpkg-installed/")


def run(arguments: list[str]) -> str:
    result = subprocess.run(arguments, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        raise SystemExit(
            f"command failed ({result.returncode}): {' '.join(arguments)}: {result.stderr.strip()}"
        )
    return result.stdout


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


MACHO_MAGICS = {
    b"\xcf\xfa\xed\xfe",
    b"\xce\xfa\xed\xfe",
    b"\xfe\xed\xfa\xcf",
    b"\xfe\xed\xfa\xce",
    b"\xbe\xba\xfe\xca",
    b"\xbf\xba\xfe\xca",
    b"\xca\xfe\xba\xbe",
    b"\xca\xfe\xba\xbf",
}


def is_macho(path: pathlib.Path) -> bool:
    """True for a Mach-O or fat Mach-O file, tested on its magic bytes.

    ``otool -h`` is not used because this toolchain's otool reports
    ``is not an object file`` for a plist and still exits 0.
    """
    if not path.is_file():
        return False
    with path.open("rb") as stream:
        return stream.read(4) in MACHO_MAGICS


def install_name(path: pathlib.Path) -> str:
    """The image's own LC_ID_DYLIB, empty for an executable."""
    result = subprocess.run(
        ["otool", "-DX", str(path)], capture_output=True, text=True, check=False
    )
    lines = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    return lines[0] if lines else ""


def load_commands_text(path: pathlib.Path) -> str:
    return run(["otool", "-l", str(path)])


def dependencies(path: pathlib.Path) -> list[str]:
    """Every LC_LOAD_DYLIB install name, in file order.

    ``otool -L`` is not used because it also prints the image's own
    ``LC_ID_DYLIB`` as its first entry.
    """
    names: list[str] = []
    want = False
    for line in load_commands_text(path).splitlines():
        if "cmd LC_LOAD_DYLIB" in line:
            want = True
            continue
        if not want:
            continue
        stripped = line.strip()
        if stripped.startswith("name "):
            value = stripped[len("name "):]
            names.append(re.sub(r" \(offset.*$", "", value))
            want = False
    return names


def rpaths(path: pathlib.Path) -> list[str]:
    values: list[str] = []
    want = False
    for line in load_commands_text(path).splitlines():
        if "cmd LC_RPATH" in line:
            want = True
            continue
        if want and line.strip().startswith("path "):
            values.append(line.strip().split()[1])
            want = False
    return values


def parse_list_names(list_file: pathlib.Path) -> tuple[str, str]:
    stem = list_file.name[: -len(".list")]
    port_version, _, triplet = stem.rpartition("_")
    if triplet != TRIPLET:
        raise SystemExit(f"unexpected triplet in package list name {list_file.name}")
    port, _, version = port_version.rpartition("_")
    if not port or not version:
        raise SystemExit(f"cannot parse package list name {list_file.name}")
    return port, version


def build_port_index(info_dir: pathlib.Path, installed: pathlib.Path) -> dict[str, list[tuple[str, str]]]:
    """Map bundle file basename to the packages that install a file of that name."""
    if not info_dir.is_dir():
        raise SystemExit(f"no vcpkg package list directory at {info_dir}")
    index: dict[str, list[tuple[str, str]]] = {}
    for list_file in sorted(info_dir.glob(f"*_{TRIPLET}.list")):
        port, version = parse_list_names(list_file)
        for line in list_file.read_text(encoding="utf-8").splitlines():
            relative = line.strip()
            if not relative or relative.endswith("/"):
                continue
            if not relative.startswith(f"{TRIPLET}/"):
                continue
            if not (installed / relative).is_file():
                continue
            parts = relative.split("/")
            if parts[-1] in ("vcpkg.json", "copyright") or "share" in parts[1:2]:
                continue
            index.setdefault(parts[-1], []).append((port, version, relative))
    return index


def version_without_port_revision(version: str) -> str:
    """vcpkg writes ``1.0.8#6`` in the status file and ``1.0.8`` in the file list.

    The ``#N`` suffix is the port revision, not part of the upstream version, so
    the correspondence check compares the upstream version and reports both.
    """
    return version.split("#", 1)[0]


def load_sbom(path: pathlib.Path) -> dict[str, dict[str, str]]:
    if not path.is_file():
        raise SystemExit(f"no SPDX document at {path}")
    document = json.loads(path.read_text(encoding="utf-8"))
    packages: dict[str, dict[str, str]] = {}
    for package in document.get("packages", []):
        triplet = ""
        for reference in package.get("externalRefs", []):
            locator = reference.get("referenceLocator", "")
            match = re.search(r"[?&]triplet=([A-Za-z0-9_-]+)", locator)
            if match:
                triplet = match.group(1)
        if triplet != TRIPLET:
            continue
        name = package.get("name", "")
        if not name:
            raise SystemExit(f"SPDX package without a name in {path}")
        packages[name] = {
            "version": package.get("versionInfo", ""),
            "license": package.get("licenseDeclared", ""),
            "spdx_id": package.get("SPDXID", ""),
        }
    if not packages:
        raise SystemExit(f"{path} declares no {TRIPLET} package")
    return packages


def attribute(
    name: str,
    index: dict[str, list[tuple[str, str]]],
    expected_port: str | None,
    kind: str,
) -> tuple[str, str, str]:
    candidates = index.get(name, [])
    if expected_port is not None:
        candidates = [item for item in candidates if item[0] == expected_port]
    plugins_only = kind == "plugin"
    if plugins_only:
        candidates = [item for item in candidates if "/plugins/" in item[2]]
    if not candidates:
        raise SystemExit(f"no {TRIPLET} package installs {name}")
    ports = sorted({item[0] for item in candidates})
    if len(ports) > 1:
        raise SystemExit(f"{name} is installed by more than one package: {ports}")
    return candidates[0]


def qt_source_pin(repo_root: pathlib.Path) -> dict[str, str]:
    data = repo_root / "cmake/vcpkg-overlay-ports/qtbase/port.data.cmake"
    if not data.is_file():
        raise SystemExit(f"no Qt port source pin at {data}")
    text = data.read_text(encoding="utf-8")

    def value(key: str) -> str:
        match = re.search(rf'set\(qtbase_{key} "([^"]*)"\)', text)
        if not match:
            raise SystemExit(f"qtbase_{key} is missing from {data}")
        return match.group(1)

    urls = value("URL").split(";")
    return {
        "filename": value("FILENAME"),
        "sha512": value("HASH"),
        "primary_url": urls[0],
        "mirror_url": urls[1] if len(urls) > 1 else "",
        "overlay_port": "cmake/vcpkg-overlay-ports/qtbase",
    }


def manifest_pins(repo_root: pathlib.Path) -> dict[str, str]:
    manifest = json.loads((repo_root / "vcpkg.json").read_text(encoding="utf-8"))
    return {
        "builtin_baseline": manifest.get("builtin-baseline", ""),
        "direct_dependency": "qtbase 6.12.0 (features freetype, gui, widgets; desktop-shell adds testlib)",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--app", type=pathlib.Path, required=True)
    parser.add_argument("--seen", type=pathlib.Path)
    parser.add_argument("--vcpkg-installed", type=pathlib.Path, required=True)
    parser.add_argument("--sbom", type=pathlib.Path, required=True)
    parser.add_argument("--repo-root", type=pathlib.Path, required=True)
    parser.add_argument("--output", type=pathlib.Path)
    parser.add_argument("--notices", action="store_true")
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args()

    app = args.app.resolve()
    installed = args.vcpkg_installed.resolve()
    repo_root = args.repo_root.resolve()
    if not app.is_dir():
        raise SystemExit(f"no bundle at {app}")

    seen: dict[str, str] = {}
    if args.seen and args.seen.is_file():
        for line in args.seen.read_text(encoding="utf-8").splitlines():
            base, _, source = line.partition("\t")
            if base and source:
                seen[base] = source

    index = build_port_index(installed / "vcpkg" / "info", installed)
    packages = load_sbom(args.sbom)
    copyright_root = installed / TRIPLET / "share"

    records: list[dict[str, object]] = []
    attributed: dict[str, dict[str, object]] = {}
    system_names: set[str] = set()
    leaks: list[str] = []

    for path in sorted(p for p in app.rglob("*") if is_macho(p)):
        relative = str(path.relative_to(app))
        if "/Contents/MacOS/" in f"/{relative}":
            role = "executable"
        elif "/Contents/PlugIns/" in f"/{relative}":
            role = "plugin"
        elif "/Contents/Frameworks/" in f"/{relative}":
            role = "framework"
        else:
            role = "other"
        entry: dict[str, object] = {
            "bundle_path": relative,
            "role": role,
            "bytes": path.stat().st_size,
            "sha256": sha256_file(path),
            "install_name": install_name(path),
            "rpaths": rpaths(path),
            "dependencies": dependencies(path),
        }
        if role == "executable":
            entry["package"] = {
                "name": "everygraph-desktop",
                "version": "0.0.0",
                "license": "NOASSERTION (EveryGraph-owned code; the project license is not yet formally adopted)",
                "spdx_id": "",
                "origin": "built from this repository's tracked source",
            }
        else:
            expected_port = None
            source = seen.get(path.name)
            relative_source = ""
            if source:
                relative_source = str(pathlib.Path(source).resolve().relative_to(installed))
                expected_port = next(
                    (port for port, _v, rel in index.get(path.name, []) if rel == relative_source),
                    None,
                )
                if expected_port is None:
                    raise SystemExit(f"{source} was copied into the bundle but no package list owns it")
            port, version, rel = attribute(path.name, index, expected_port, role)
            record = packages.get(port)
            if record is None:
                raise SystemExit(f"bundled {path.name} comes from {port}, which the SPDX document omits")
            if version_without_port_revision(record["version"]) != version:
                raise SystemExit(
                    f"{port}: package list says {version}, SPDX says {record['version']}"
                )
            entry["package"] = {
                "name": port,
                "version": version,
                "spdx_version": record["version"],
                "license": record["license"],
                "spdx_id": record["spdx_id"],
                "installed_from": rel,
            }
            attributed[port] = {
                "name": port,
                "version": version,
                "spdx_version": record["version"],
                "license": record["license"],
                "spdx_id": record["spdx_id"],
                "files": attributed.get(port, {}).get("files", []) + [relative],
            }
        for value in [entry["install_name"], *entry["dependencies"], *entry["rpaths"]]:
            if value.startswith(SYSTEM_PREFIXES):
                system_names.add(value)
                continue
            for fragment in FORBIDDEN_FRAGMENTS:
                if fragment in value:
                    leaks.append(f"{relative}: {value}")
        if role != "executable" and entry["install_name"] and not str(
            entry["install_name"]
        ).startswith("@rpath/"):
            leaks.append(f"{relative}: install name {entry['install_name']} is not @rpath based")
        records.append(entry)

    if leaks:
        for leak in sorted(set(leaks)):
            print(f"path leak: {leak}", file=sys.stderr)
        raise SystemExit(f"{len(set(leaks))} private or build-tree path(s) remain in the bundle")

    unused = sorted(set(packages) - set(attributed))
    inventory = {
        "bundle": app.name,
        "triplet": TRIPLET,
        "macho_files": records,
        "packages": [attributed[name] for name in sorted(attributed)],
        "spdx_target_packages_not_in_bundle": [
            {"name": name, "version": packages[name]["version"], "license": packages[name]["license"]}
            for name in unused
        ],
        "system_install_names": sorted(system_names),
        "checks": {
            "every_bundled_library_attributed_to_a_package": True,
            "versions_match_the_spdx_document": True,
            "no_private_or_build_tree_path": True,
        },
    }
    qt_pin = qt_source_pin(repo_root)
    pins = manifest_pins(repo_root)
    inventory["qt_source"] = qt_pin
    inventory["vcpkg_pins"] = pins

    output = args.output
    if output is None:
        output = app.parent / "bundle-inventory.json"
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    serialised = json.dumps(inventory, indent=2, sort_keys=True) + "\n"
    for fragment in FORBIDDEN_FRAGMENTS:
        if fragment in serialised:
            raise SystemExit(f"the inventory itself contains the private fragment {fragment}")
    output.write_text(serialised, encoding="utf-8")

    notice_files: list[dict[str, str]] = []
    if args.notices:
        licenses = app / "Contents/Resources/licenses"
        licenses.mkdir(parents=True, exist_ok=True)
        for name in sorted(attributed):
            source = copyright_root / name / "copyright"
            if not source.is_file():
                raise SystemExit(f"no installed copyright notice for {name}")
            target = licenses / f"{name}.txt"
            target.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
            notice_files.append({"file": f"Contents/Resources/licenses/{target.name}",
                                 "package": name,
                                 "sha256": sha256_file(target)})
        index_path = licenses / "NOTICE.md"
        index_path.write_text(
            render_notice(attributed, qt_pin, pins, notice_files), encoding="utf-8"
        )
        inventory["notices"] = notice_files + [
            {"file": "Contents/Resources/licenses/NOTICE.md", "package": "index",
             "sha256": sha256_file(index_path)}
        ]
        output.write_text(json.dumps(inventory, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if not args.quiet:
        print(f"bundle: {app}")
        print(f"inventory: {output}")
        print(f"macho files: {len(records)}; packages: {len(attributed)}; system install names: {len(system_names)}")
        for name in sorted(attributed):
            package = attributed[name]
            print(f"| {name} | {package['version']} | {package['license']} | {len(package['files'])} |")
    return 0


def render_notice(
    attributed: dict[str, dict[str, object]],
    qt_pin: dict[str, str],
    pins: dict[str, str],
    notice_files: list[dict[str, str]],
) -> str:
    lines = [
        "# Third-party notices for this bundle",
        "",
        "Generated by `packaging/macos/build_bundle.sh` from the files actually present in",
        "`Contents/Frameworks` and `Contents/PlugIns`; do not edit by hand.",
        "",
        "## EveryGraph",
        "",
        "The executable `everygraph_desktop` is EveryGraph's own code. EveryGraph is an open",
        "source project intended for Apache-2.0; the formal release license, copyright and",
        "notice files are an unresolved distribution gate, so this bundle makes no license",
        "claim for the application itself. The application source is the repository revision",
        "the bundle was built from.",
        "",
        "## Qt",
        "",
        f"- Qt version: qtbase 6.12.0 (vcpkg package, triplet `{TRIPLET}`, shared libraries).",
        f"- Declared license options: {attributed['qtbase']['license']}.",
        "  This bundle uses the LGPL-3.0-only shared-library option proposed by",
        "`docs/engineering/licensing-and-dependencies.md`; the other declared options are",
        "recorded, not selected.",
        f"- Exact Qt source: `{qt_pin['filename']}`, SHA-512 `{qt_pin['sha512']}`,",
        f"  from `{qt_pin['primary_url']}` (mirror `{qt_pin['mirror_url']}`).",
        f"- Build material: vcpkg manifest baseline `{pins['builtin_baseline']}`, direct",
        f"  dependency {pins['direct_dependency']}, and the version-overlay port",
        f"  `{qt_pin['overlay_port']}` in this repository. The overlay's four modified files and",
        "  the deleted FreeBSD patch are listed in the P1-003 record.",
        "- Linking and replacement: Qt is loaded as dynamic libraries from",
        "  `Contents/Frameworks`, each with the install name `@rpath/<file>`. A recipient can",
        "  replace one of these libraries with a modified interface-compatible build without",
        "  relinking the application. `packaging/macos/lgpl_replace_test.sh` exercises that",
        "  route on this bundle, and the signature uses no hardened-runtime option that",
        "  removes it.",
        "",
        "## Notices carried for the linked packages",
        "",
        "Each file below is the copyright and license text that vcpkg installed for that",
        "package in the same triplet this bundle draws from.",
        "",
        "| File | Package | SHA-256 |",
        "| --- | --- | --- |",
    ]
    for entry in notice_files:
        lines.append(f"| `{entry['file']}` | {entry['package']} | `{entry['sha256']}` |")
    lines += [
        "",
        "The Qt source archive additionally bundles third-party licenses under",
        "`LICENSES/` and `REUSE.toml`; the OpenGL wrapper and registry components carry",
        "per-file or framework terms recorded as `NOASSERTION` in the SPDX document and",
        "require file-level review before any distribution. This index is engineering",
        "documentation, not a legal opinion.",
        "",
    ]
    return "\n".join(lines)


if __name__ == "__main__":
    sys.exit(main())
