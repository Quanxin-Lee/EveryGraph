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


def read_status_blocks(status_file: pathlib.Path) -> list[dict[str, str]]:
    blocks: list[dict[str, str]] = []
    block: dict[str, str] = {}
    for line in status_file.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            if block:
                blocks.append(block)
                block = {}
            continue
        key, separator, value = line.partition(": ")
        if separator:
            block[key] = value
    if block:
        blocks.append(block)
    return blocks


def parse_status(status_file: pathlib.Path) -> list[dict[str, str]]:
    return [
        package
        for package in read_status_blocks(status_file)
        if package.get("Status") == "install ok installed"
        and "Feature" not in package
        and "Version" in package
    ]


def parse_feature_records(
    status_file: pathlib.Path,
) -> tuple[dict[tuple[str, str], list[str]], dict[tuple[str, str], list[str]]]:
    features: dict[tuple[str, str], list[str]] = {}
    dependencies: dict[tuple[str, str], list[str]] = {}
    for record in read_status_blocks(status_file):
        if record.get("Status") != "install ok installed" or "Feature" not in record:
            continue
        key = (record["Package"], record.get("Architecture", ""))
        features.setdefault(key, []).append(record["Feature"])
        for token in record.get("Depends", "").split(","):
            token = token.strip()
            if token:
                dependencies.setdefault(key, []).append(token)
    return features, dependencies


def package_version(package: dict[str, str]) -> str:
    version = package["Version"]
    port_version = package.get("Port-Version")
    if port_version and port_version != "0":
        version += f"#{port_version}"
    return version


def license_declared(
    metadata: dict,
    enabled_features: list[str],
    override: dict[str, str],
) -> tuple[str, str]:
    base = override.get("license") or metadata.get("license")
    if not isinstance(base, str) or not base:
        base = "NOASSERTION"
    feature_licenses: set[str] = set()
    feature_metadata = metadata.get("features", {})
    if isinstance(feature_metadata, dict):
        for feature in enabled_features:
            entry = feature_metadata.get(feature, {})
            if isinstance(entry, dict):
                value = entry.get("license")
                if isinstance(value, str) and value:
                    feature_licenses.add(value)
    extra = sorted(value for value in feature_licenses if value != base)
    if extra:
        parts = [base, *extra]
        base = " AND ".join(
            f"({expression})" if " OR " in expression else expression
            for expression in parts
        )
    return base, override.get("evidence", "")


def package_purl(package: dict[str, str], purl_name: str) -> str:
    purl = f"pkg:generic/{purl_name}@{package['Version']}"
    qualifiers: list[str] = []
    architecture = package.get("Architecture")
    if architecture:
        qualifiers.append(f"triplet={architecture}")
    port_version = package.get("Port-Version")
    if port_version and port_version != "0":
        qualifiers.append(f"port_version={port_version}")
    if qualifiers:
        purl += "?" + "&".join(qualifiers)
    return purl


def load_port_metadata(roots: list[pathlib.Path], name: str) -> dict:
    for root in roots:
        manifest = root / name / "vcpkg.json"
        if manifest.is_file():
            return json.loads(manifest.read_text(encoding="utf-8"))
    return {}


def package_lists(
    info_dir: pathlib.Path, triplet: str
) -> tuple[dict[str, set[str]], dict[tuple[str, str], str]]:
    lists: dict[str, set[str]] = {}
    versions: dict[tuple[str, str], str] = {}
    pattern = re.compile(r"^(?P<name>.+)_(?P<version>[^_]+)_" + re.escape(triplet) + r"\.list$")
    for path in sorted(info_dir.glob(f"*_{triplet}.list")):
        match = pattern.match(path.name)
        if not match:
            continue
        name = match.group("name")
        versions[(name, triplet)] = match.group("version")
        entries = {
            line.strip()
            for line in path.read_text(encoding="utf-8").splitlines()
            if line.strip()
        }
        lists[name] = entries
    return lists, versions


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
            base_path = pathlib.Path(base)
            if not base_path.is_absolute():
                base_path = binary.parent / base_path
            candidate = base_path / relative
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


def find_owner(
    relative: str,
    lists_by_arch: dict[str, dict[str, set[str]]],
    architecture_order: list[str],
) -> tuple[str, str] | None:
    for architecture in architecture_order:
        for name, entries in lists_by_arch.get(architecture, {}).items():
            if relative in entries:
                return name, architecture
    return None


def dependency_edges(
    packages: list[dict[str, str]],
    package_ids: dict[tuple[str, str], str],
    default_architecture: str,
    feature_dependencies: dict[tuple[str, str], list[str]] | None = None,
) -> list[dict[str, str]]:
    feature_dependencies = feature_dependencies or {}
    relationships: list[dict[str, str]] = []
    seen_edges: set[tuple[str, str]] = set()
    for package in packages:
        name = package["Package"]
        architecture = package.get("Architecture", default_architecture)
        package_key = (name, architecture)
        tokens = package.get("Depends", "").split(",")
        tokens.extend(feature_dependencies.get(package_key, []))
        for token in tokens:
            token = token.strip()
            if not token:
                continue
            dependency_part, separator, dependency_arch = token.partition(":")
            dependency = dependency_part.split("[", 1)[0].strip()
            if not dependency:
                continue
            explicit_triplet = bool(separator and dependency_arch)
            if not explicit_triplet:
                dependency_arch = architecture
            dependency_key = (dependency, dependency_arch)
            if dependency_key not in package_ids:
                if explicit_triplet:
                    raise SystemExit(
                        f"unresolved dependency {token!r} declared by {name}:{architecture}"
                    )
                candidates = [key for key in package_ids if key[0] == dependency]
                if len(candidates) != 1:
                    raise SystemExit(
                        f"unresolved dependency {token!r} declared by {name}:{architecture}"
                    )
                dependency_key = candidates[0]
            edge = (package_ids[package_key], package_ids[dependency_key])
            if edge in seen_edges:
                continue
            seen_edges.add(edge)
            relationships.append(
                {
                    "spdxElementId": package_ids[package_key],
                    "relationshipType": "DEPENDS_ON",
                    "relatedSpdxElement": package_ids[dependency_key],
                }
            )
    return relationships


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
    parser.add_argument("--host-triplet", default="arm64-osx")
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
    for required in (
        args.vcpkg_root / "ports",
        args.overlay_ports,
        args.license_overrides,
    ):
        if not required.exists():
            raise SystemExit(f"missing required metadata input: {required}")

    packages = parse_status(status_file)
    if not packages:
        raise SystemExit(f"no installed packages found in {status_file}")
    features, feature_dependencies = parse_feature_records(status_file)

    metadata_roots = [args.overlay_ports.resolve(), args.vcpkg_root.resolve() / "ports"]
    license_overrides: dict[str, dict[str, str]] = {}
    component_notes: dict[str, str] = {}
    if args.license_overrides.is_file():
        overrides_document = json.loads(
            args.license_overrides.read_text(encoding="utf-8")
        )
        license_overrides = overrides_document.get("packages", {})
        component_notes = overrides_document.get("component_notes", {})
    target_lists, list_versions = package_lists(info_dir, args.triplet)
    host_lists, host_list_versions = package_lists(info_dir, args.host_triplet)
    list_versions.update(host_list_versions)
    lists_by_arch = {args.triplet: target_lists, args.host_triplet: host_lists}
    linked_names = otool_libraries(probe)
    rpaths = otool_rpaths(probe)

    created = args.created or datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    document_name = f"everygraph-macos-arm64-qt-{args.triplet}"
    namespace = f"https://spdx.org/spdxdocs/{document_name}-{created.replace(':', '')}"

    sbom_packages: list[dict] = []
    sbom_files: list[dict] = []
    relationships: list[dict] = []
    package_ids: dict[tuple[str, str], str] = {}
    file_ids: dict[str, str] = {}
    linked_owned: dict[tuple[str, str], list[str]] = {}

    for package in packages:
        name = package["Package"]
        version = package_version(package)
        architecture = package.get("Architecture", args.triplet)
        package_key = (name, architecture)
        metadata = load_port_metadata(metadata_roots, name)
        override = license_overrides.get(name, {})
        license_value, license_evidence = license_declared(
            metadata, features.get(package_key, []), override
        )
        package_kind = override.get("kind", "")
        package_id = spdx_id("Package", f"{name}-{architecture}")
        if package_key in package_ids:
            raise SystemExit(f"duplicate package entry for {name}:{architecture}")
        package_ids[package_key] = package_id
        dependencies = package.get("Depends", "")
        purl_name = metadata.get("name", name)
        purl = package_purl(package, purl_name)
        sbom_packages.append(
            {
                "SPDXID": package_id,
                "name": name,
                "versionInfo": version,
                "supplier": "NOASSERTION",
                "originator": "NOASSERTION",
                "downloadLocation": "NOASSERTION",
                "filesAnalyzed": False,
                "licenseConcluded": "NOASSERTION",
                "licenseDeclared": license_value,
                "copyrightText": "NOASSERTION",
                "externalRefs": [
                    {
                        "referenceCategory": "PACKAGE-MANAGER",
                        "referenceType": "purl",
                        "referenceLocator": purl,
                    }
                ],
                "comment": "; ".join(
                    part
                    for part in (
                        f"vcpkg ABI {package.get('Abi', 'unknown')}",
                        f"architecture: {package.get('Architecture', 'unknown')}",
                        (
                            "features: " + ", ".join(sorted(features.get(package_key, [])))
                            if features.get(package_key)
                            else ""
                        ),
                        f"dependencies: {dependencies or 'none'}",
                        f"homepage: {metadata['homepage']}" if metadata.get("homepage") else "",
                        f"class: {package_kind}" if package_kind else "",
                        f"license evidence: {license_evidence}" if license_evidence else "",
                        f"component note: {component_notes[name]}" if name in component_notes else "",
                    )
                    if part
                ),
            }
        )

    relationships = dependency_edges(
        packages, package_ids, args.triplet, feature_dependencies
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
            if install_name.startswith("@") or str(install_root) in install_name:
                raise SystemExit(f"cannot resolve linked library: {install_name}")
            continue
        relative = relative_to(resolved, install_root)
        if relative is None:
            if str(install_root) in str(resolved):
                raise SystemExit(f"linked library is outside the install root: {install_name}")
            continue
        owner_info = find_owner(
            relative, lists_by_arch, [args.triplet, args.host_triplet]
        )
        if owner_info is None:
            unresolved_relative = relative_to(pathlib.Path(install_name), install_root)
            if unresolved_relative is not None:
                owner_info = find_owner(
                    unresolved_relative, lists_by_arch, [args.triplet, args.host_triplet]
                )
        if owner_info is None:
            raise SystemExit(f"linked library has no vcpkg owner: {install_name}")
        owner, owner_arch = owner_info
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
                    "checksums": [
                        {"algorithm": "SHA1", "checksumValue": hash_file(resolved, "sha1")},
                        {"algorithm": "SHA256", "checksumValue": sha256_file(resolved)},
                    ],
                    "licenseConcluded": "NOASSERTION",
                    "copyrightText": "NOASSERTION",
                    "comment": (
                        f"Linked by the built probe; owning vcpkg package: {owner}; "
                        "path is relative to the vcpkg install root."
                    ),
                }
            )
        linked_owned.setdefault((owner, owner_arch), []).append(file_ids[key])

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
    for (owner, owner_arch) in sorted(linked_owned):
        relationships.append(
            {
                "spdxElementId": probe_id,
                "relationshipType": "DEPENDS_ON",
                "relatedSpdxElement": package_ids[(owner, owner_arch)],
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

    for package in packages:
        name = package["Package"]
        architecture = package.get("Architecture", args.triplet)
        key = (name, architecture)
        list_version = list_versions.get(key)
        if list_version is not None and list_version != package["Version"]:
            raise SystemExit(
                f"file-list version mismatch for {name}:{architecture}: "
                f"{list_version} != {package['Version']}"
            )
    if not linked_owned:
        raise SystemExit("probe does not link any vcpkg-owned library")

    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    print(f"wrote {output}")
    print(f"packages: {len(sbom_packages) - 1} vcpkg + 1 probe; files: {len(sbom_files)}")
    linked_summary = ", ".join(f"{name}:{arch}" for name, arch in sorted(linked_owned))
    print(f"linked vcpkg packages: {linked_summary}")
    print(f"input status: {relative_to(status_file, repository) or status_file}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
