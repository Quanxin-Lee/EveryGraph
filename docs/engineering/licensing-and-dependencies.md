# Licensing and Dependency Governance

Decision date: 2026-10-30. **Select Apache-2.0 for EveryGraph-owned source and developer documentation.** This is the project licensing policy, not a legal certification of a future installer. Before public distribution, the copyright holders must publish the authoritative LICENSE/NOTICE material and confirm rights to contributed material. Preserved third-party research/source excerpts and dependencies retain their applicable rights; do not blanket-relicense them.

## 1. Project-license comparison

| License | Commercial use and private modification | Distribution obligations / fit |
|---|---|---|
| [MIT](https://opensource.org/license/mit) | Permitted | Preserve copyright/license notice; minimal conditions, no express patent grant comparable to Apache-2.0 |
| [BSD-3-Clause](https://opensource.org/license/bsd-3-clause) | Permitted | Source/binary notices and non-endorsement; similarly permissive |
| [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) | Permitted | License, applicable NOTICE and change notices; explicit patent license/termination; no general trademark grant. Preferred for broad commercial/research integration |
| [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) | Permitted; not a ban on commercial software | Conveyed covered combined/derivative works require GPL terms and corresponding source; Installation Information for relevant User Products. Strong reciprocity, but constrains proprietary linked plugins/integration |
| [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.html) | Permitted | Library-focused copyleft with combined-work, replacement/relinking and source obligations; appropriate dependency route, not the simplest whole-app policy |
| [MPL-2.0](https://www.mozilla.org/en-US/MPL/2.0/) | Permitted | Covered files remain under MPL with source availability on distribution; larger works may use other terms under its conditions. Useful compromise, but less permissive than the chosen policy |

Apache-2.0 prioritizes reuse, explicit patent treatment and commercial scientific extensions. The tradeoff is that downstream proprietary improvements need not all be returned. GPLv3 would better enforce broad reciprocity but conflicts with the intended permissive plugin/reuse direction. License compatibility is directional and artifact-specific: Apache code can participate in a compatible GPLv3 combined work, but that does not permit relabeling GPL code as Apache-only. GPLv2-only is a different compatibility question.

## 2. Baseline runtime dependency policy

The [evidence ledger](../research/gui-and-license-evidence.md) supplies exact inspected source pins and license texts; those pins are **research snapshots**, not a validated production lockfile.

| Component | Policy / primary license | Release obligation or gate |
|---|---|---|
| Qt Core/Gui/Widgets/Concurrent/Network/Svg/PrintSupport | Selected LGPLv3 shared-library route. P1-001 installed only Homebrew `qtbase` 6.12.0_2 (Core/Gui/Widgets and the other qtbase modules; no Charts/Graphs/DataVisualization/WebEngine) and linked a probe against Core/Gui/Widgets | Exact Qt source/build/patch bundle, license notices, replacement/debugging rights and packaging test |
| Qt Qml/Quick | LGPLv3 route available, not initial UI baseline | Add only with a feature/maintenance case; audit actual imported modules |
| Qt Charts/Graphs/DataVisualization | GPLv3/commercial, no LGPL route in inspected versions | Not baseline; requires explicit licensing/architecture change or suitable commercial rights |
| Qt WebEngine | Qt LGPL/GPL/commercial plus Chromium dependency closure | Not baseline; not falsely classified as GPL-only, but very substantial packaging/security/license scope |
| Eigen | MPL-2.0 primary | Preserve notices and provide exact covered source including modifications; keep covered implementation out of Apache-only files |
| Arrow and C++ Parquet | Apache-2.0 primary | NOTICE, bundled third-party/compression/allocator/build-feature inventory |
| SQLite core | Public-domain dedication | Separate extension/build-tool licensing; jurisdiction/title concerns reviewed where relevant |
| Ceres | BSD-3-Clause primary | Inspect selected linear algebra/accelerator dependencies and file exceptions |
| pocketfft | BSD-3-Clause | Candidate default FFT backend; exact source pin and numerical qualification |
| Boost.Math | BSL-1.0 | Select only required libraries, inspect dependency closure |
| CPython | PSF plus historical/incorporated licenses | Ship complete notices; reproducible interpreter and package inventory |
| pybind11 | BSD-3-Clause | Does not relicense Python or native wrapped libraries |
| Polars | MIT | Optional, measured query-engine need only |
| VTK | BSD-3-Clause primary | Optional P7 modules; ThirdParty/render/readers/codecs audit |
| wgpu | MIT OR Apache-2.0 | Optional future renderer; backend SDK/tooling closure |
| llama.cpp | MIT | Optional inference; model weights/tokenizers and GPU SDKs separately licensed |
| FFTW | GPL-2.0-or-later or separately negotiated commercial terms | Excluded from default permissive distribution; not LGPL |

LabPlot, Blender, QGIS and GIMP are architectural references, not baseline source dependencies. Do not copy GPL implementation into EveryGraph without an explicit project-license decision. GEGL's library/operation license scope also needs selected-file review. GPUI's Apache license is distinct from Zed application GPL licensing.

## 3. Dynamic/static linking and distribution

Dynamic linking alone is not a compliance certificate. For Qt's LGPLv3 route, provide the required prominent notices and GPL/LGPL texts, corresponding Qt source and modifications/build material, and a working ability to replace interface-compatible modified libraries. Allow reverse engineering for debugging those modifications. Evaluate Installation Information obligations where applicable. macOS hardened runtime/library validation, signing and updater behavior, Windows packaging and store agreements must not silently negate required rights.

P1-001 exercised local replacement on macOS arm64: `macdeployqt` produced an ad-hoc signed probe bundle containing the Qt frameworks and transitive dylibs, and a modified copy of the bundle's QtCore framework (new LC_RPATH, re-signed ad-hoc) loaded successfully in place of the original. Adding the manually copied offscreen plugin and modifying QtCore invalidated the bundle's code-signature seal; only the framework itself was re-signed and verified. That is evidence that a recipient can substitute a modified LGPL library without relinking, but not evidence of compliance under hardened runtime, library validation, Developer ID signing or notarization; those interactions still require release qualification. Dependency-deployment and license-notice deliverables for the bundle remain open.

Static linking is not categorically prohibited, but adds relinkable-object/source and installation/replacement obligations; it is not the default packaging path. Open application source helps but does not automatically prove a recipient can rebuild/replace the library. Keep an actual replacement/rebuild test in release qualification. Obtain counsel review for concrete distribution channels, especially stores.

Native plugins linking into an application can create combined-work questions. A worker/IPC boundary is useful engineering separation, not an automatic legal exception. Review purpose, integration and actual licenses. Third-party vendors' SDKs, Office/COM integrations, MATLAB/Mathematica/LabVIEW, fonts, codecs, models and format documentation require independent permission. Do not equate free-of-charge with open-source rights.

## 4. Dependency management

Use one C++ dependency authority: vcpkg manifest mode with an immutable baseline, explicit versions/features and macOS arm64 / Windows x64 triplet policy. Prefer shared Qt and consistently selected CRT; don't mix arbitrary Homebrew libraries, downloaded Qt binaries and incompatible vcpkg builds. Conan is a credible alternative for package/profile control, but adopting both as equal sources increases graph ambiguity. Choose a manager switch through an ADR.

P1-001 used Homebrew `qtbase` 6.12.0_2 as an explicitly temporary qualification route for the macOS arm64 probe: bottle `arm64_golden_gate`, SHA-256 `178ff42e7d82f5ed7c6164f6c7c4e8b735e03a4b61bd96eb970341f7329214fb`, Homebrew formula license expression `(LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only) AND (GPL-3.0-only WITH Qt-GPL-exception-1.0) AND BSD-3-Clause AND GFDL-1.3-no-invariants-only`, with an SPDX-2.3 SBOM at `/opt/homebrew/Cellar/qtbase/6.12.0_2/sbom.spdx.json`. This route does not select Homebrew as the project dependency authority and does not satisfy the vcpkg/Conan decision; P1-003 must pin the durable graph before business modules depend on Qt. The Homebrew bottle's transitive closure (ICU 79, OpenSSL 4, glib, dbus, freetype, harfbuzz, pcre2, zstd, libpng, jpeg-turbo, brotli, double-conversion, md4c, libb2) was deployed into the probe bundle and must be inventoried in any release SBOM.

Phase 0 intentionally has no runtime dependency graph or fabricated lockfile: only compiler/planning tools are installed in an isolated environment. P1-001 added the first runtime dependency, Qt `qtbase` 6.12.0_2, under a provisional Homebrew route without a committed lockfile; that installation is qualification evidence, not a release dependency pin. Before P1/P2, qualify exact release versions and archive package hashes/source references. Floating 'latest' releases or development HEADs are forbidden in release builds. Research pins can guide inspection but are not automatically production choices.

If Rust is introduced, commit Cargo.lock for applications and pin the toolchain; audit crates/native dependencies and C ABI bindings. Python environments use exact package versions/hashes per supported platform, include native wheels in the SBOM and avoid in-place uncontrolled pip updates. A system Python installed by a user is not the distributed application's runtime contract.

## 5. Release evidence

Every release/optional pack needs an SBOM with direct/transitive versions, source origin, hashes, license expression, modifications and feature/build flags; license/NOTICE texts; corresponding-source access; model/font/data notices; vulnerability review; and reproducible build/packaging records. Record distributed dependencies, not merely what a source manifest lists. No automatic plugin/model download on project open.

Legal review remains open for the complete shipped closure, proprietary formats/SDKs, protected files, native package distribution, Qt replacement under signing/store policies and per-model terms. The selected policy is clear; final compliance is a release gate, not something this research can guarantee.
