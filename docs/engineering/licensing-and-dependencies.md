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

P1-001 used Homebrew `qtbase` 6.12.0_2 as an explicitly temporary qualification route for the macOS arm64 probe: bottle `arm64_golden_gate`, SHA-256 `178ff42e7d82f5ed7c6164f6c7c4e8b735e03a4b61bd96eb970341f7329214fb`, Homebrew formula license expression `(LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only) AND (GPL-3.0-only WITH Qt-GPL-exception-1.0) AND BSD-3-Clause AND GFDL-1.3-no-invariants-only`, with an SPDX-2.3 SBOM at `/opt/homebrew/Cellar/qtbase/6.12.0_2/sbom.spdx.json`. This route does not select Homebrew as the project dependency authority; P1-003 pins the durable graph below. The Homebrew bottle's transitive closure (ICU 79, OpenSSL 4, glib, dbus, freetype, harfbuzz, pcre2, zstd, libpng, jpeg-turbo, brotli, double-conversion, md4c, libb2) was deployed into the probe bundle and must be inventoried in any release SBOM.

### P1-003 pinned graph (macOS arm64)

P1-003 pins a vcpkg manifest at registry baseline `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091` with vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`. The target triplet is `arm64-osx-dynamic` (shared); host tools use `arm64-osx`. Qt is `qtbase` 6.12.0 with `freetype`, `gui` and `widgets` enabled and default features disabled; `freetype` is required because Qt builds its `offscreen` platform plugin only when the freetype feature is on, and the headless probe test uses that platform. The curated registry contains 6.11.2, so a 34-file copy of the upstream `qtbase` recipe at the baseline supplies 6.12.0; four files are modified (port version, `cmake/qt_port_details.cmake`, `port.data.cmake` source URL/SHA-512/filename, and `portfile.cmake`, which also installs Qt 6.12.0's exported harmony tools) and the obsolete `use_inotify_on_freebsd.patch` is deleted because Qt 6.12.0 already contains the change. The overlay is a temporary, removable version pin, not a fork or a second package manager. Remove it and re-pin when the registry ships Qt 6.12.

A second overlay, `cmake/vcpkg-overlay-ports/pcre2/`, is a copy of the registry pcre2 10.49#2 port whose GitHub API gitlink lookup and unused head-version branch are replaced by the pinned sljit commit. It does not change PCRE2's version, features or license. Sljit is vendored into pcre2 at build time: the pinned sljit source archive's `LICENSE` is BSD-2-Clause, while the installed pcre2 copyright only states that `deps/sljit` has its own LICENSE and does not install that text. The target pcre2 row therefore records the PCRE2 expression plus the vendored sljit component, and adding the sljit notice to the release notice bundle remains an open obligation. The pcre2, libb2, md4c and sljit archives were seeded from a verified GitHub mirror because direct GitHub access was unavailable; the SHA-512 values match the vcpkg pins, so no dependency or license terms changed. Remove the overlay when direct GitHub access is available.

Resolved versions, license expressions and evidence for the actual install are generated into `docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json` from the vcpkg status and the built probe's linkage by `docs/engineering/sbom/generate_sbom.py`. Packages whose vcpkg manifest declares no license use `docs/engineering/sbom/license-overrides.json`, with the installed copyright evidence. Target packages are `arm64-osx-dynamic`; `arm64-osx` host build tools are build-time only and are not distributed.

P0-018 cross-checked the committed SPDX package records against `vcpkg.json` and the tables below on 2026-10-31. The SBOM has 43 package records: 13 target packages, 29 host build-tool packages, and one `everygraph-qt-probe` record. Each package row below gives its exact SPDX package identifier, which can be looked up in the committed SBOM; the SBOM JSON is the row-level source for name, version, triplet and declared license expression. `vcpkg.json` declares one direct dependency, `qtbase` with `version>= 6.12.0`, default features disabled, and `freetype`, `gui` and `widgets`; that direct dependency maps to `SPDXRef-Package-qtbase-arm64-osx-dynamic`. The separate `qtbase-arm64-osx` record is a host build-tool instance. The probe record is EveryGraph's qualification executable, not a third-party dependency. `OR` expressions remain the package-declared alternatives, and `AND` expressions retain all listed license components; P0-018 does not select among alternatives.

Direct manifest dependency cross-check (source: committed `vcpkg.json`):

| Manifest package | Constraint and features | Matching target SBOM component |
| --- | --- | --- |
| qtbase | `version>= 6.12.0`; default features disabled; `freetype`, `gui`, `widgets` | `SPDXRef-Package-qtbase-arm64-osx-dynamic` |

Target runtime graph (`arm64-osx-dynamic`):

| Package | Version | License (port/evidence) | SPDX component ID |
| --- | --- | --- | --- |
| brotli | 1.2.1 | MIT | `SPDXRef-Package-brotli-arm64-osx-dynamic` |
| bzip2 | 1.0.8#6 | bzip2-1.0.6 | `SPDXRef-Package-bzip2-arm64-osx-dynamic` |
| double-conversion | 3.4.0 | BSD-3-Clause | `SPDXRef-Package-double-conversion-arm64-osx-dynamic` |
| egl-registry | 2025-05-27 | Apache-2.0 | `SPDXRef-Package-egl-registry-arm64-osx-dynamic` |
| freetype | 2.14.3 | FTL OR GPL-2.0-or-later | `SPDXRef-Package-freetype-arm64-osx-dynamic` |
| libb2 | 0.98.1#7 | CC0-1.0 | `SPDXRef-Package-libb2-arm64-osx-dynamic` |
| libpng | 1.6.59 | libpng-2.0 | `SPDXRef-Package-libpng-arm64-osx-dynamic` |
| md4c | 0.6.0 | MIT | `SPDXRef-Package-md4c-arm64-osx-dynamic` |
| opengl | 2022-12-04#3 | NOASSERTION (macOS framework wrapper) | `SPDXRef-Package-opengl-arm64-osx-dynamic` |
| opengl-registry | 2026-08-03 | NOASSERTION (per-file licenses) | `SPDXRef-Package-opengl-registry-arm64-osx-dynamic` |
| pcre2 | 10.49#2 | BSD-3-Clause WITH PCRE2-exception AND BSD-2-Clause (jit/sljit) | `SPDXRef-Package-pcre2-arm64-osx-dynamic` |
| qtbase | 6.12.0 | LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only | `SPDXRef-Package-qtbase-arm64-osx-dynamic` |
| zlib | 1.3.2#2 | Zlib | `SPDXRef-Package-zlib-arm64-osx-dynamic` |

Host build tools (`arm64-osx`, not distributed):

| Package | Version | License (port/evidence) | SPDX component ID |
| --- | --- | --- | --- |
| brotli | 1.2.1 | MIT | `SPDXRef-Package-brotli-arm64-osx` |
| bzip2 | 1.0.8#6 | bzip2-1.0.6 | `SPDXRef-Package-bzip2-arm64-osx` |
| dbus | 1.16.2#5 | AFL-2.1 OR GPL-2.0-or-later | `SPDXRef-Package-dbus-arm64-osx` |
| double-conversion | 3.4.0 | BSD-3-Clause | `SPDXRef-Package-double-conversion-arm64-osx` |
| egl-registry | 2025-05-27 | Apache-2.0 | `SPDXRef-Package-egl-registry-arm64-osx` |
| expat | 2.9.0 | MIT AND CC0-1.0 | `SPDXRef-Package-expat-arm64-osx` |
| freetype | 2.14.3 | FTL OR GPL-2.0-or-later | `SPDXRef-Package-freetype-arm64-osx` |
| harfbuzz | 14.6.0 | MIT-Modern-Variant AND MIT | `SPDXRef-Package-harfbuzz-arm64-osx` |
| icu | 79.1 | Unicode-3.0 | `SPDXRef-Package-icu-arm64-osx` |
| libb2 | 0.98.1#7 | CC0-1.0 | `SPDXRef-Package-libb2-arm64-osx` |
| libjpeg-turbo | 3.2.0#1 | BSD-3-Clause AND IJG | `SPDXRef-Package-libjpeg-turbo-arm64-osx` |
| libpng | 1.6.59 | libpng-2.0 | `SPDXRef-Package-libpng-arm64-osx` |
| libpq | 18.4#2 | PostgreSQL | `SPDXRef-Package-libpq-arm64-osx` |
| libspng | 0.7.4#1 | BSD-2-Clause AND libpng-2.0 | `SPDXRef-Package-libspng-arm64-osx` |
| lz4 | 1.10.0 | BSD-2-Clause | `SPDXRef-Package-lz4-arm64-osx` |
| md4c | 0.6.0 | MIT | `SPDXRef-Package-md4c-arm64-osx` |
| opengl | 2022-12-04#3 | NOASSERTION (macOS framework wrapper) | `SPDXRef-Package-opengl-arm64-osx` |
| opengl-registry | 2026-08-03 | NOASSERTION (per-file licenses) | `SPDXRef-Package-opengl-registry-arm64-osx` |
| openssl | 3.6.5#2 | Apache-2.0 | `SPDXRef-Package-openssl-arm64-osx` |
| pcre2 | 10.49#2 | BSD-3-Clause WITH PCRE2-exception AND BSD-2-Clause (jit/sljit) | `SPDXRef-Package-pcre2-arm64-osx` |
| qtbase | 6.12.0 | LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only | `SPDXRef-Package-qtbase-arm64-osx` |
| sqlite3 | 3.54.1 | blessing | `SPDXRef-Package-sqlite3-arm64-osx` |
| vcpkg-cmake | 2025-08-07 | MIT | `SPDXRef-Package-vcpkg-cmake-arm64-osx` |
| vcpkg-cmake-config | 2026-07-21 | MIT | `SPDXRef-Package-vcpkg-cmake-config-arm64-osx` |
| vcpkg-cmake-get-vars | 2025-05-29 | MIT | `SPDXRef-Package-vcpkg-cmake-get-vars-arm64-osx` |
| vcpkg-make | 2026-07-09#2 | MIT AND GPL-2.0-or-later | `SPDXRef-Package-vcpkg-make-arm64-osx` |
| vcpkg-tool-meson | 1.12.1 | Apache-2.0 AND MIT | `SPDXRef-Package-vcpkg-tool-meson-arm64-osx` |
| zlib | 1.3.2#2 | Zlib | `SPDXRef-Package-zlib-arm64-osx` |
| zstd | 1.5.7 | BSD-3-Clause OR GPL-2.0-only | `SPDXRef-Package-zstd-arm64-osx` |

The SBOM's additional non-dependency package record is `everygraph-qt-probe` version `0.0.0`, license `NOASSERTION`, SPDX ID `SPDXRef-Package-everygraph-qt-probe`. It identifies the qualification executable, not an external component.

The versions come from the resolved vcpkg status; license values are the port-declared expression or the installed-copyright evidence in `license-overrides.json`. `NOASSERTION` marks a wrapper/per-file set without one expression; it is not an omission of the obligation. The Qt source's bundled third-party files and the pcre2/sljit notice require a release notice bundle; the SBOM is qualification-scoped.

### License classes and text obligations

The expressions above contain these license identifiers and one exception. The summaries below describe text conditions to carry into release engineering; they are not a legal classification or clearance of the assembled product. Where an expression uses `OR`, P0-018 records the declared options without selecting one. Where it uses `AND`, retain obligations from each named license. Retrieval and inspection date for the linked license texts and official project pages: 2026-10-31.

| License identifier or exception | Components in this SBOM | Text-based obligation summary |
| --- | --- | --- |
| [`AFL-2.1`](https://spdx.org/licenses/AFL-2.1.html) | dbus (`OR` option; host build tool) | Section 3 is the Licensor's commitment to provide machine-readable source with copies that the Licensor distributes, or through an accessible repository while it distributes the work. A downstream recipient that distributes a derivative must retain the original notices and attribution text in source and add a prominent notice that it modified the work; section 9 requires a reasonable effort to obtain recipients' express assent. The text also bars endorsement using licensor/contributor names without written permission. |
| [`Apache-2.0`](https://www.apache.org/licenses/LICENSE-2.0) | egl-registry, openssl, vcpkg-tool-meson | Preserve license and copyright notices in source; include the license with object distributions; mark modified files; reproduce any upstream NOTICE attributions that apply to the redistributed portions. |
| [`BSD-2-Clause`](https://spdx.org/licenses/BSD-2-Clause.html) | lz4, libspng, pcre2's vendored sljit | Retain copyright, conditions and disclaimer in source and reproduce them in binary documentation/materials. |
| [`BSD-3-Clause`](https://spdx.org/licenses/BSD-3-Clause.html) | double-conversion, libjpeg-turbo, pcre2, zstd | Apply the BSD-2 notice/disclaimer retention and include the non-endorsement condition. |
| [`bzip2-1.0.6`](https://spdx.org/licenses/bzip2-1.0.6.html) | bzip2 | Retain the copyright, conditions and disclaimer in source; plainly mark altered source and do not misrepresent its origin. Product acknowledgment is described by the text as appreciated, not required. |
| [`CC0-1.0`](https://spdx.org/licenses/CC0-1.0.html) | libb2, expat | The text is a public-domain dedication with a fallback license and other-rights provisions. Keep provenance evidence and do not treat the SPDX expression alone as proof that every right in every jurisdiction was waived. |
| [`FTL`](https://spdx.org/licenses/FTL.html) | freetype | The text permits commercial inclusion and requires acknowledgment of FreeType use in program documentation; it supplies preferred credit wording. This is an `OR` option alongside GPL-2.0-or-later. |
| [`GPL-2.0-only`](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html) | qtbase, zstd (`OR` options) | If this option is selected for a conveyed covered work, comply with GPLv2 distribution terms, including preserving notices and providing the corresponding source required for object-code distribution. The package expression alone does not select this option. |
| [`GPL-2.0-or-later`](https://spdx.org/licenses/GPL-2.0-or-later.html) | dbus, freetype, vcpkg-make (`OR` options) | The port-declared expression permits GPL version 2 or a later version for covered material; if selected for a conveyed covered work, preserve notices and meet that version's source requirements. The build-tool instances are currently host-only. |
| [`GPL-3.0-only`](https://www.gnu.org/licenses/gpl-3.0.html) | qtbase (`OR` option) | If selected for a conveyed covered work, provide the GPLv3 license and corresponding source under its terms; provide Installation Information only where GPLv3 section 6 requires it. No such option is selected by this task. |
| [`IJG`](https://spdx.org/licenses/IJG.html) | libjpeg-turbo | If source is distributed, include the unaltered IJG README and document source changes; if only executable code is distributed, accompanying documentation must state that it is based in part on IJG work. The text restricts promotional use of IJG names. |
| [`LGPL-3.0-only`](https://www.gnu.org/licenses/lgpl-3.0.html) | qtbase (policy's proposed route; SBOM declares alternatives) | For a conveyed Combined Work, LGPLv3 section 4 requires prominent notices and copies of GPLv3/LGPLv3; it also requires either Minimal Corresponding Source plus Corresponding Application Code permitting recombination/relinking with a modified library, or a suitable dynamic/shared-library mechanism that uses a replaceable interface-compatible library and permits reverse engineering for debugging modifications. Installation Information applies only where GPLv3 section 6 would require it. Qt's official [6.12 licensing page](https://doc.qt.io/qt-6.12/licensing.html) and [LGPL text](https://doc.qt.io/qt-6.12/lgpl.html) are the version-specific references. |
| [`libpng-2.0`](https://spdx.org/licenses/libpng-2.0.html) | libpng, libspng | Do not misrepresent origin; mark altered source; do not remove or alter the copyright notice from source or altered-source distributions. A product documentation acknowledgment is stated as appreciated, not required. |
| [`MIT`](https://spdx.org/licenses/MIT.html) | brotli, md4c, expat, harfbuzz, vcpkg-cmake, vcpkg-cmake-config, vcpkg-cmake-get-vars, vcpkg-make, vcpkg-tool-meson | Include the copyright and permission notice in copies or substantial portions of the software. |
| [`MIT-Modern-Variant`](https://spdx.org/licenses/MIT-Modern-Variant.html) | harfbuzz | Include the copyright notice and the license's two disclaimer paragraphs in all copies. This appears with MIT under `AND`, so both expressions are retained. |
| [`PCRE2-exception`](https://spdx.org/licenses/PCRE2-exception.html) | pcre2 | Read together with BSD-3-Clause: the exception narrows the binary redistribution clause for a chain of binary-like packages. It does not remove the pcre2 notice obligations from the pcre2 package; the SPDX row also records vendored sljit under BSD-2-Clause. |
| [`PostgreSQL`](https://spdx.org/licenses/PostgreSQL.html) | libpq | Keep the copyright notice, permission paragraph and following disclaimer paragraphs in all copies. |
| [`Unicode-3.0`](https://spdx.org/licenses/Unicode-3.0.html) | icu | Include the copyright and permission notice with all copies of data/software or in associated documentation; do not use a holder's name to promote dealings without prior written authorization. |
| [`Zlib`](https://spdx.org/licenses/Zlib.html) | zlib | The notice may not be removed or altered from source distributions; altered source must be plainly marked and its origin not misrepresented. Product-documentation acknowledgment is described as appreciated, not required. |
| [`blessing`](https://spdx.org/licenses/blessing.html) | sqlite3 | The port-declared SPDX text is SQLite's blessing; keep its source/evidence with the inventory. This wording is separate from the broader SQLite public-domain statement and is not a legal conclusion about title or other SQLite files. |
| `NOASSERTION` | opengl wrapper, opengl-registry, everygraph-qt-probe | This is not a license and establishes no absence of obligations. The OpenGL registry is per-file; audit exact redistributed files and the Apple framework boundary before release. The probe is not a shipped third-party component. |

The SPDX identifiers and expression operators are standardized by the [SPDX License List](https://spdx.org/licenses/) and [license-expression specification](https://spdx.github.io/spdx-spec/v2.3/annexes/spdx-license-expressions/). Port-level expressions remain evidence inputs, not a determination of the exact files or terms that a future product will distribute.

Open obligations: Windows triplet resolution; Qt bundled third-party licenses and per-file notices; corresponding-source access; a release-scope SBOM covering distributed files and build flags; and legal review. The Homebrew P1-001 route remains qualification evidence, not the dependency authority.

Phase 0 had no runtime dependency graph or fabricated lockfile: only compiler/planning tools were installed in an isolated environment. P1-003 adds the first pinned graph (above); it is a qualification pin, not a release lock. P0-018 reconciles the committed qualification SBOM and manifest and records license-text obligations, but does not create a release SBOM or notice bundle. P1-001's Homebrew installation remains qualification evidence. Before P1/P2, qualify exact release versions and archive package hashes/source references. Floating 'latest' releases or development HEADs are forbidden in release builds. Research pins can guide inspection but are not automatically production choices.

If Rust is introduced, commit Cargo.lock for applications and pin the toolchain; audit crates/native dependencies and C ABI bindings. Python environments use exact package versions/hashes per supported platform, include native wheels in the SBOM and avoid in-place uncontrolled pip updates. A system Python installed by a user is not the distributed application's runtime contract.

## 5. Release evidence

Every release/optional pack needs an SBOM with direct/transitive versions, source origin, hashes, license expression, modifications and feature/build flags; license/NOTICE texts; corresponding-source access; model/font/data notices; vulnerability review; and reproducible build/packaging records. Record distributed dependencies, not merely what a source manifest lists. No automatic plugin/model download on project open.

Legal review remains open for the complete shipped closure, proprietary formats/SDKs, protected files, native package distribution, Qt replacement under signing/store policies and per-model terms. The selected policy is clear; final compliance is a release gate, not something this research can guarantee.
