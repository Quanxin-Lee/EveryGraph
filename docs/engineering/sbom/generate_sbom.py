#!/usr/bin/env python3
"""Generate an SPDX 2.3 SBOM from a resolved vcpkg graph and a built Qt probe.

The generator reads the actual vcpkg install status and the built probe's
Mach-O linkage. It does not invent package versions, licenses or dependency
edges. The output is a scoped qualification SBOM, not a release or legal
approval.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys
from datetime import datetime, timezone

SPDX_VERSION = "SPDX-2.3"
DATA_LICENSE = "CC0-1.0"
DOCUMENT_ID = "SPDXRef-DOCUMENT"


def hash_file(path: pathlib.Path, algorithm: str) -> str:
    digest = hashlib.new(algorithm)
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def sha256_file(path: pathlib.Path) -> str:
    return hash_file(path, "sha256")


def package_verification_code(file_sha1_values: list[str]) -> str:
    joined = "".join(sorted(value.lower() for value in file_sha1_values))
    return hashlib.sha1(joined.encode("ascii")).hexdigest()


def spdx_id(prefix: str, name: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9.-]", "-", name)
    return f"SPDXRef-{prefix}-{safe}"


def parse_status(status_file: pathlib.Path) -> list[dict[str, str]]:
    packages: list[dict[str, str]] = []
    block: dict[str, str] = {}
    for line in status_file.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            if block:
                packages.append(block)
                block = {}
            continue
        key, separator, value = line.partition(": ")
        if separator:
            block[key] = value
    if block:
        packages.append(block)
    return [p for p in packages if p.get("Status") == "install ok installed"]


def package_version(package: dict[str, str]) -> str:
    version = package["Version"]
    port_version = package.get("Port-Version")
    if port_version and port_version != "0":
        version += f"#{port_version}"
    return version


def load_port_metadata(roots: list[pathlib.Path], name: str) -> dict:
    for root in roots:
        manifest = root / name / "vcpkg.json"
        if manifest.is_file():
            return json.loads(manifest.read_text(encoding="utf-8"))
    return {}


def package_lists(info_dir: pathlib.Path, triplet: str) -> dict[str, set[str]]:
    lists: dict[str, set[str]] = {}
    pattern = re.compile(r"^(?P<name>.+)_(?P<version>[^_]+)_" + re.escape(triplet) + r"\.list$")
    for path in sorted(info_dir.glob(f"*_{triplet}.list")):
        match = pattern.match(path.name)
        if not match:
            continue
        name = match.group("name")
        entries = {
            line.strip()
            for line in path.read_text(encoding="utf-8").splitlines()
            if line.strip()
        }
        lists[name] = entries
    return lists


def run_otool(arguments: list[str]) -> str:
    result = subprocess.run(
        ["otool", *arguments],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"otool {' '.join(arguments)} failed: {result.stderr.strip()}")
    return result.stdout


def otool_libraries(binary: pathlib.Path) -> list[str]:
    lines = run_otool(["-L", str(binary)]).splitlines()
    libraries: list[str] = []
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue
        libraries.append(line.split(" (compatibility version")[0].strip())
    return libraries


def otool_rpaths(binary: pathlib.Path) -> list[str]:
    lines = run_otool(["-l", str(binary)]).splitlines()
    rpaths: list[str] = []
    for index, line in enumerate(lines):
        if "cmd LC_RPATH" not in line:
            continue
        for candidate in lines[index : index + 5]:
            match = re.search(r"path (.+?) \(offset", candidate)
            if match:
                rpaths.append(match.group(1))
                break
    return rpaths


def resolve_install_name(name: str, binary: pathlib.Path, rpaths: list[str]) -> pathlib.Path | None:
    if name.startswith("@rpath/"):
        relative = name[len("@rpath/") :]
        for rpath in rpaths:
            base = rpath.replace("@loader_path", str(binary.parent))
            base = base.replace("@executable_path", str(binary.parent))
            candidate = pathlib.Path(base) / relative
            if candidate.exists():
                return candidate.resolve()
        return None
    if name.startswith("@loader_path/"):
        return (binary.parent / name[len("@loader_path/") :]).resolve()
    candidate = pathlib.Path(name)
    return candidate.resolve() if candidate.exists() else None


def relative_to(path: pathlib.Path, root: pathlib.Path) -> str | None:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return None


def find_owner(relative: str, lists: dict[str, set[str]]) -> str | None:
    for name, entries in lists.items():
        if relative in entries:
            return name
    return None


def main() -> int:
    repository = pathlib.Path(__file__).resolve().parents[3]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=pathlib.Path, default=repository)
    parser.add_argument(
        "--install-root",
        type=pathlib.Path,
        default=repository / "out/build/macos-release-qt-vcpkg/vcpkg_installed",
    )
    parser.add_argument(
        "--probe",
        type=pathlib.Path,
        default=repository
        / "out/build/macos-release-qt-vcpkg/everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe",
    )
    parser.add_argument("--vcpkg-root", type=pathlib.Path, default=repository / "out/vcpkg")
    parser.add_argument(
        "--overlay-ports", type=pathlib.Path, default=repository / "cmake/vcpkg-overlay-ports"
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=repository / "docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json",
    )
    parser.add_argument(
        "--license-overrides",
        type=pathlib.Path,
        default=repository / "docs/engineering/sbom/license-overrides.json",
    )
    parser.add_argument("--triplet", default="arm64-osx-dynamic")
    parser.add_argument("--baseline", default="")
    parser.add_argument("--vcpkg-tool-version", default="")
    parser.add_argument("--created", default="")
    args = parser.parse_args()

    repository = args.repo_root.resolve()
    install_root = args.install_root.resolve()
    probe = args.probe.resolve()
    status_file = install_root / "vcpkg" / "status"
    info_dir = install_root / "vcpkg" / "info"
    for required in (status_file, info_dir, probe):
        if not required.exists():
            raise SystemExit(f"missing required input: {required}")

    packages = parse_status(status_file)
    if not packages:
        raise SystemExit(f"no installed packages found in {status_file}")

    metadata_roots = [args.overlay_ports.resolve(), args.vcpkg_root.resolve() / "ports"]
    license_overrides: dict[str, dict[str, str]] = {}
    if args.license_overrides.is_file():
        license_overrides = json.loads(
            args.license_overrides.read_text(encoding="utf-8")
        ).get("packages", {})
    lists = package_lists(info_dir, args.triplet)
    linked_names = otool_libraries(probe)
    rpaths = otool_rpaths(probe)

    created = args.created or datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    document_name = f"everygraph-macos-arm64-qt-{args.triplet}"
    namespace = f"https://spdx.org/spdxdocs/{document_name}-{created.replace(':', '')}"

    sbom_packages: list[dict] = []
    sbom_files: list[dict] = []
    relationships: list[dict] = []
    package_ids: dict[str, str] = {}
    file_ids: dict[str, str] = {}
    linked_owned: dict[str, list[str]] = {}

    for package in packages:
        name = package["Package"]
        version = package_version(package)
        metadata = load_port_metadata(metadata_roots, name)
        override = license_overrides.get(name, {})
        license_value = override.get("license") or metadata.get("license") or "NOASSERTION"
        if not isinstance(license_value, str):
            license_value = "NOASSERTION"
        license_evidence = override.get("evidence", "")
        package_kind = override.get("kind", "")
        package_id = spdx_id("Package", name)
        if package_id in package_ids.values():
            raise SystemExit(f"duplicate SPDX package id for {name}")
        package_ids[name] = package_id
        dependencies = package.get("Depends", "")
        purl_name = metadata.get("name", name)
        sbom_packages.append(
            {
                "SPDXID": package_id,
                "name": name,
                "versionInfo": version,
                "supplier": "NOASSERTION",
                "originator": "NOASSERTION",
                "downloadLocation": metadata.get("homepage") or "NOASSERTION",
                "filesAnalyzed": False,
                "licenseConcluded": "NOASSERTION",
                "licenseDeclared": license_value,
                "copyrightText": "NOASSERTION",
                "externalRefs": [
                    {
                        "referenceCategory": "PACKAGE-MANAGER",
                        "referenceType": "vcpkg",
                        "referenceLocator": f"{purl_name}@{version}?triplet={args.triplet}",
                    }
                ],
                "comment": "; ".join(
                    part
                    for part in (
                        f"vcpkg ABI {package.get('Abi', 'unknown')}",
                        f"dependencies: {dependencies or 'none'}",
                        f"class: {package_kind}" if package_kind else "",
                        f"license evidence: {license_evidence}" if license_evidence else "",
                    )
                    if part
                ),
            }
        )

    for package in packages:
        name = package["Package"]
        for dependency in package.get("Depends", "").split(","):
            dependency = dependency.strip().split("[", 1)[0].strip()
            if not dependency or dependency not in package_ids:
                continue
            relationships.append(
                {
                    "spdxElementId": package_ids[name],
                    "relationshipType": "DEPENDS_ON",
                    "relatedSpdxElement": package_ids[dependency],
                }
            )

    probe_id = "SPDXRef-Package-everygraph-qt-probe"
    probe_relative = relative_to(probe, repository)
    if probe_relative is None:
        raise SystemExit("probe binary is outside the repository root")
    probe_file_id = "SPDXRef-File-everygraph-qt-probe"
    probe_sha256 = sha256_file(probe)
    probe_sha1 = hash_file(probe, "sha1")
    probe_file = {
        "SPDXID": probe_file_id,
        "fileName": f"./{probe_relative}",
        "checksums": [
            {"algorithm": "SHA1", "checksumValue": probe_sha1},
            {"algorithm": "SHA256", "checksumValue": probe_sha256},
        ],
        "licenseConcluded": "NOASSERTION",
        "copyrightText": "NOASSERTION",
        "comment": "Built qualification probe; not a release artifact.",
    }
    sbom_files.append(probe_file)
    file_ids[str(probe)] = probe_file_id

    for install_name in linked_names:
        resolved = resolve_install_name(install_name, probe, rpaths)
        if resolved is None:
            continue
        relative = relative_to(resolved, install_root)
        if relative is None:
            continue
        owner = find_owner(relative, lists)
        if owner is None:
            unresolved_relative = relative_to(pathlib.Path(install_name), install_root)
            if unresolved_relative is not None:
                owner = find_owner(unresolved_relative, lists)
        if owner is None:
            raise SystemExit(f"linked library has no vcpkg owner: {install_name}")
        key = str(resolved)
        if key not in file_ids:
            file_id = spdx_id("File", owner + "-" + relative)
            if file_id in file_ids.values():
                file_id = file_id + "-" + str(len(file_ids))
            file_ids[key] = file_id
            sbom_files.append(
                {
                    "SPDXID": file_id,
                    "fileName": relative,
                    "checksums": [{"algorithm": "SHA256", "checksumValue": sha256_file(resolved)}],
                    "licenseConcluded": "NOASSERTION",
                    "copyrightText": "NOASSERTION",
                    "comment": "Linked by the built probe; path is relative to the vcpkg install root.",
                }
            )
        linked_owned.setdefault(owner, []).append(file_ids[key])

    sbom_packages.append(
        {
            "SPDXID": probe_id,
            "name": "everygraph-qt-probe",
            "versionInfo": "0.0.0",
            "supplier": "NOASSERTION",
            "originator": "NOASSERTION",
            "downloadLocation": "NOASSERTION",
            "filesAnalyzed": True,
            "packageVerificationCode": {
                "packageVerificationCodeValue": package_verification_code([probe_sha1]),
            },
            "hasFiles": [probe_file_id],
            "licenseConcluded": "NOASSERTION",
            "licenseDeclared": "NOASSERTION",
            "copyrightText": "NOASSERTION",
            "comment": (
                "Local qualification probe built from tools/qt_probe/main.cpp; "
                "EveryGraph-owned source is intended for Apache-2.0 but the release "
                "license and notices are not yet published."
            ),
        }
    )
    relationships.append(
        {
            "spdxElementId": DOCUMENT_ID,
            "relationshipType": "DESCRIBES",
            "relatedSpdxElement": probe_id,
        }
    )
    for owner in sorted(linked_owned):
        relationships.append(
            {
                "spdxElementId": probe_id,
                "relationshipType": "DEPENDS_ON",
                "relatedSpdxElement": package_ids[owner],
            }
        )
        for file_id in sorted(set(linked_owned[owner])):
            relationships.append(
                {
                    "spdxElementId": package_ids[owner],
                    "relationshipType": "CONTAINS",
                    "relatedSpdxElement": file_id,
                }
            )
    relationships.append(
        {
            "spdxElementId": probe_id,
            "relationshipType": "CONTAINS",
            "relatedSpdxElement": probe_file_id,
        }
    )

    creation_comment = (
        "Generated from the resolved vcpkg status file and the built probe's Mach-O "
        "linkage. vcpkg baseline "
        f"{args.baseline or 'unknown'}; vcpkg tool {args.vcpkg_tool_version or 'unknown'}; "
        f"triplet {args.triplet}. Not a release SBOM and not a legal approval."
    )
    document = {
        "spdxVersion": SPDX_VERSION,
        "dataLicense": DATA_LICENSE,
        "SPDXID": DOCUMENT_ID,
        "name": document_name,
        "documentNamespace": namespace,
        "creationInfo": {
            "created": created,
            "creators": ["Tool: docs/engineering/sbom/generate_sbom.py"],
            "comment": creation_comment,
        },
        "packages": sbom_packages,
        "files": sbom_files,
        "relationships": relationships,
    }

    all_ids = [DOCUMENT_ID]
    all_ids.extend(package_ids.values())
    all_ids.append(probe_id)
    all_ids.extend(file_ids.values())
    if len(all_ids) != len(set(all_ids)):
        raise SystemExit("duplicate SPDX identifiers")
    for relationship in relationships:
        for key in ("spdxElementId", "relatedSpdxElement"):
            if relationship[key] not in all_ids:
                raise SystemExit(f"relationship references unknown id: {relationship[key]}")

    by_name = {package["name"]: package["versionInfo"] for package in sbom_packages}
    for package in packages:
        name = package["Package"]
        expected = package_version(package)
        if by_name.get(name) != expected:
            raise SystemExit(f"version mismatch for {name}: {by_name.get(name)} != {expected}")
    if not linked_owned:
        raise SystemExit("probe does not link any vcpkg-owned library")

    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    print(f"wrote {output}")
    print(f"packages: {len(sbom_packages) - 1} vcpkg + 1 probe; files: {len(sbom_files)}")
    print(f"linked vcpkg packages: {', '.join(sorted(linked_owned))}")
    print(f"input status: {relative_to(status_file, repository) or status_file}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
