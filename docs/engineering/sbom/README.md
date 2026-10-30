# Initial SBOM for the pinned macOS arm64 dependency graph

This directory holds a qualification-scoped SPDX 2.3 SBOM for the vcpkg graph and the Qt Widgets probe. It is not a release SBOM, a notice bundle or legal approval.

## Inputs

- The resolved vcpkg install status (`<install-root>/vcpkg/status`) and per-package file lists (`vcpkg/info/*.list`).
- The built probe's Mach-O linkage (`otool -L` and `otool -l` for `LC_RPATH`).
- vcpkg port manifests for declared licenses and homepages.
- `license-overrides.json` for packages whose port manifest declares no license, with the installed copyright evidence used, plus component notes for vendored code such as sljit inside pcre2.

The generator does not invent versions, licenses or edges. It reads both base package and feature records; feature-level `Depends` entries are attributed to the owning package. It fails if a declared dependency cannot be resolved, if a linked vcpkg-owned library has no package owner, or if a package's status version differs from its vcpkg file-list filename where present.

## Files

- `generate_sbom.py`: generator and consistency checks.
- `test_generate_sbom.py`: unit tests for feature-record parsing (including feature dependencies), file-list versions, per-package dependency edges, deduplication and purls.
- `license-overrides.json`: evidence-backed license values for packages whose vcpkg manifest is silent.
- `everygraph-macos-arm64-qt.spdx.json`: generated SBOM (after a successful Release build).

## Generate

Run from the repository root after configuring and building `macos-release-qt-vcpkg`:

```sh
python3 docs/engineering/sbom/generate_sbom.py \
  --install-root out/build/macos-release-qt-vcpkg/vcpkg_installed \
  --probe out/build/macos-release-qt-vcpkg/everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe \
  --vcpkg-root "$PWD/out/vcpkg" \
  --triplet arm64-osx-dynamic \
  --baseline d186940b2f0c4e8e5b4ff804af57d2a26fbfa091 \
  --vcpkg-tool-version 2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b
```

The probe path is the default; the other flags may be omitted when the default checkout layout is used.

Run the generator unit tests with:

```sh
python3 -m unittest discover -s docs/engineering/sbom -p 'test_generate_sbom.py' -v
```

## Checks

- Every installed package appears once; feature records are not treated as packages. Enabled features that declare their own license are merged into `licenseDeclared`. The status version is cross-checked against the version embedded in that package's vcpkg file-list filename where present.
- Every `Depends` token, including tokens declared on feature records, is resolved or the generator exits with an error; every resolved edge is attributed to the declaring package and deduplicated, including `name[feature]:triplet` tokens.
- Every vcpkg-owned library linked by the probe maps to a package and has SHA-1/SHA-256 file entries; the owning package is named in the file comment.
- Every SPDX relationship endpoint exists; identifiers are unique.
- The output uses repository-relative or install-root-relative paths and contains no home-directory path.
- The package verification code for the probe is derived from its SHA-1 file hash.
- vcpkg packages are package-level (`filesAnalyzed: false`); the linked artifacts are standalone file entries whose comment names the owning package and install-root path. They are traceable through that comment and the probe's `DEPENDS_ON` package edge; no package `CONTAINS` relationship is asserted because the packages are package-level. Only the probe package has a complete file list and verification code.

## Scope and limits

- Target packages are `arm64-osx-dynamic`; host build tools are `arm64-osx` and are not distributed with the application.
- macOS system libraries and frameworks are outside the SBOM; they are not distributed by EveryGraph.
- Qt's bundled third-party licenses are summarized in the licensing document; the complete release notice, corresponding-source and per-file obligations remain open.
- No Windows resolution, packaging, signing, notarization or legal approval is claimed.
