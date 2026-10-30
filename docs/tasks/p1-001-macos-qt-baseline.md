# P1-001: Qualify Qt 6.12 and a Minimal Workbench on macOS arm64

## Metadata

- Task ID: P1-001
- Status: accepted
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P0-001 (accepted)
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / base revision: `feat/p1-001-qt-baseline`, based on `main` at `747809e`; implementation committed as `d226fa3`. The earlier record statement that no Git repository existed is obsolete; the repository and an `origin` remote now exist. No push was performed.
- Authorization: implementation authorized 2026-10-30 for the Homebrew Qt route, build-file changes, documentation updates and a local commit on the feature branch. No push, remote publication, signing identity, notarization, Windows execution or business feature implementation was authorized or performed.

## Scope and acceptance

Objective: prove that the selected Qt 6.12 baseline can build and start a minimal Qt Widgets workbench on macOS arm64 under project-controlled CMake presets, and record the exact version, deployment and LGPL replacement facts. This is a qualification task, not a product workbench. It supports, but does not complete, EG-ADD-016 or DG-PERF-004; the cross-platform gate requires the Windows evidence from P1-002.

Allowed files and interfaces: CMakeLists.txt, CMakePresets.json, a new `tools/qt_probe/` target, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md` and this record. No Qt types in future domain/application public headers; no product modules or business logic.

Out of scope: the full P1 workbench, menus/docks/tabs as a product, command gateway, data model, analysis, project persistence, Windows, CI changes, signed or notarized packaging, release artifacts, and any claim that P1 is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Exact Qt patch, install source, selected modules, license basis, CMake, Ninja, compiler, SDK, macOS and architecture recorded | Met: Qt 6.12.0 (`6.12.0_2`) Homebrew `qtbase` bottle for `arm64_golden_gate`, SHA-256 `178ff42e7d82f5ed7c6164f6c7c4e8b735e03a4b61bd96eb970341f7329214fb`; modules Core/Gui/Widgets plus other qtbase frameworks, no Charts/Graphs/DataVisualization/WebEngine; LGPLv3 route; CMake 3.31.6; Ninja 1.11.1.4; AppleClang 21.0.0.21000334; macOS SDK 27.0; macOS 27.0.1 build 26A434; arm64. Recorded in [development guide](../engineering/development.md) and [licensing policy](../engineering/licensing-and-dependencies.md) |
| AC-2 | A minimal QMainWindow-based Qt Widgets probe configures and builds in Debug and Release under a project preset on macOS arm64 | Met: `macos-debug-qt` and `macos-release-qt` configured and built from fresh directories with no compiler warnings; probe is `tools/qt_probe/main.cpp` (`QMainWindow` + central `QLabel` + status bar) and produces an arm64 `.app` |
| AC-3 | A deterministic offscreen smoke test runs in CTest without a logged-in GUI session; existing planning/toolchain tests still pass | Met: `qt_probe_smoke` sets `QT_QPA_PLATFORM=offscreen`, checks visibility, central widget and platform name, and exits through a zero-delay timer; full CTest 4/4 in both Qt presets; Qt-free `macos-debug`, `macos-release` and `macos-sanitize` each 3/3 |
| AC-4 | Dynamic Qt linkage is shown (for example with `otool -L`); deployment and a replaceable-LGPL-library experiment are recorded, including any limitation | Met: build-tree `otool -L` shows dynamic Core/Gui/Widgets; `macdeployqt` produced an ad-hoc signed bundle with `@executable_path/../Frameworks` paths and no Homebrew dependency edges (some bundled libraries retain Homebrew install names in `LC_ID_DYLIB`); clean-environment run passed; a modified, re-signed QtCore in the bundle loaded and passed. Limitations recorded (bundle seal invalidated by the added plugin and modified framework; no hardened runtime, Developer ID or notarization; macdeployqt did not copy the offscreen plugin) |
| AC-5 | The build remains separate from domain/application contracts, contains no scientific business logic, and makes no Windows, packaging, signing or CI claim | Met: probe has no domain/application headers and no scientific logic; Qt is opt-in and absent from default presets; documentation states no Windows, packaging, signing, notarization or CI claim. The local ad-hoc signed probe bundle is explicitly not a release artifact |
| AC-6 | The development guide and licensing document contain the actual commands, versions, results and unresolved obligations; all existing checks remain green | Met: both documents updated with commands, versions, linkage/deployment/replacement evidence and open obligations; planning validator passes; six validator unit tests pass; all presets green |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created the task and recorded the current toolchain gap; no implementation started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Implementation / implementer | Installed Homebrew `qtbase` 6.12.0_2 and pinned CMake/Ninja in an ignored `.venv`; added the Qt probe, opt-in CMake option and Qt presets; ran builds, tests, deployment and replacement experiments; updated engineering documents | This record, [CMakeLists.txt](../../CMakeLists.txt), [CMakePresets.json](../../CMakePresets.json), [probe](../../tools/qt_probe/main.cpp), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md) |

Independent review completed on 2026-10-30; findings were resolved and rechecked. No human approval or merge authorization is implied.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 build 26A434, arm64; AppleClang 21.0.0 (`clang-2100.3.34.2`, CMake `21.0.0.21000334`); CommandLineTools 27.0.0.0; macOS SDK 27.0; deployment target 14.4; CMake 3.31.6; Ninja 1.11.1.4 (`1.11.1.git.kitware.jobserver-1`); Python 3.13.9 in `.venv`, CMake-discovered Homebrew Python 3.14.2; Qt 6.12.0 Homebrew `qtbase` `6.12.0_2` at `/opt/homebrew/opt/qtbase`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `brew install qtbase` (`HOMEBREW_NO_AUTO_UPDATE=1`) | Passed: poured `qtbase--6.12.0_2.arm64_golden_gate.bottle.tar.gz`, 5,965 files, 62.3 MB | Bottle SHA-256 from `brew info --json=v2 qtbase`; `sbom.spdx.json` SPDX-2.3 with 36 packages |
| 2026-10-30 | `python3 -m venv .venv` and `.venv/bin/python -m pip install cmake==3.31.6 ninja==1.11.1.4` | Passed | Repository-local ignored environment, not a dependency lock |
| 2026-10-30 | `QT_ROOT="$(brew --prefix qtbase)" cmake --preset macos-debug-qt` then build and `ctest --preset macos-debug-qt` | Configured, built with no warnings, CTest 4/4 passed | Fresh build directory; smoke test 0.85 s on rerun |
| 2026-10-30 | `QT_ROOT="$(brew --prefix qtbase)" cmake --preset macos-release-qt` then build and `ctest --preset macos-release-qt` | Configured, built with no warnings, CTest 4/4 passed | Release probe bundle used for deployment |
| 2026-10-30 | `ctest --preset macos-debug`, `macos-release`, `macos-sanitize` | Each 3/3 passed | Existing Qt-free suites unaffected; ASan+UBSan still green without Qt |
| 2026-10-30 | `python3 tools/validate_planning.py` and validator unittests | PASS: 339 original IDs, 428 traced requirements, 35 documents; 6/6 unit tests | Local links, fences and English policy included |
| 2026-10-30 | `otool -L out/build/macos-release-qt/everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe` | Dynamic links to `/opt/homebrew/opt/qtbase/lib/QtWidgets.framework`, `QtGui.framework` and `QtCore.framework` under `Versions/A` | Build-tree linkage only |
| 2026-10-30 | `/opt/homebrew/opt/qtbase/bin/macdeployqt out/qt-deploy/everygraph_qt_probe.app`, then copy `libqoffscreen.dylib` into `Contents/PlugIns/platforms/` | Passed; frameworks, 22 dylibs and `libqcocoa.dylib` copied, paths rewritten, ad-hoc signed | Absolute tool path used because Anaconda shadows `macdeployqt`; adding the plugin invalidated the bundle seal; exact commands in the development guide |
| 2026-10-30 | Fresh deploy into `out/qt-deploy-check/` and hash QtCore framework | Deployed hash `c5a7b187ac06360d7e6c7fbc737e89cbfc8fe760528f2d5bdef7d28ab0382dd5` reproduced; Homebrew original `5ef20b68...` | Confirms the recorded deployed-copy hash is reproducible |
| 2026-10-30 | Clean-environment headless run with `DYLD_PRINT_LIBRARIES=1` | Exit 0; Qt libraries loaded from the bundle; no `/opt/homebrew` references remained | Proves local deployment, not packaging |
| 2026-10-30 | Replace deployed QtCore: `install_name_tool -add_rpath`, `codesign --force --sign -`, rerun smoke | Exit 0; modified framework (hash `c5ac0a6e...`) verified and loaded from the bundle | Bundle seal remains invalid; unsigned modified-framework loading untested; no hardened runtime/Developer ID/notarization test |
| 2026-10-30 | `xcrun vtool -show-build` on QtCore and probe | Both report `minos 14.4`, `sdk 27.0` | No execution on macOS 14.4 hardware |
| 2026-10-30 | Inspect installed frameworks and formula metadata | 12 qtbase frameworks; no Charts/Graphs/DataVisualization/WebEngine; Homebrew license expression recorded | Transitive closure not yet an approved release SBOM |

Checks not run and their reasons: execution on macOS 14.4, Intel Macs or Windows; ASan/UBSan combined with the Qt probe; hardened runtime, library validation, Developer ID signing, notarization, clean-machine deployment and CI execution. These are outside this task or remain release gates.

## Review and disposition

Reviewer and review type: independent EveryGraph Review, performed 2026-10-30 by a separate read-only review agent over the working-tree diff from `747809e`, the task record, the board, both engineering documents, and rerun configure/build/test, validator, `vtool`, `otool`, clean-environment execution and signature checks. Not a human review.

Reviewed files / revision: CMakeLists.txt, CMakePresets.json, tools/qt_probe/main.cpp, docs/engineering/development.md, docs/engineering/licensing-and-dependencies.md, docs/planning/task-board.csv and this record at the uncommitted P1-001 working-tree state based on `747809e`.

Findings, resolutions and recheck evidence:

| Finding | Resolution | Recheck |
|---|---|---|
| The guide claimed the Qt presets fail when `QT_ROOT` is unset; CMake can still discover Homebrew Qt through its default prefix | Corrected the guide; the version pin is enforced by `find_package`, not by the environment variable | Configured with `QT_ROOT` and `CMAKE_PREFIX_PATH` unset in a scratch build directory; configuration succeeded |
| "No Homebrew references remained" was too strong: bundled QtDBus and libbrotlicommon retain Homebrew `LC_ID_DYLIB` install names | Reworded to "no Homebrew dependency edges" in the guide and record; install-name limitation stated | `otool -L` and clean-environment `DYLD_PRINT_LIBRARIES` show all dependencies loading from the bundle |
| The recorded `macdeployqt` command was not reproducible because Anaconda shadows the tool, and the manual plugin step was not recorded | Recorded the absolute Homebrew tool path and the copy/deploy/plugin commands in the guide and record | Commands re-read against the executed sequence; fresh deploy reproduced the recorded QtCore hash |
| The bundle's code-signature seal is invalid after adding the plugin and modifying QtCore; this limitation was not stated | Stated in both engineering documents and the AC-4 evidence; the framework itself still verifies | `codesign --verify --deep --strict` reports the added file and modified framework; framework-level `codesign --verify` passes |
| `find_package(Qt6 6.12 ...)` accepted later minors | Changed to `find_package(Qt6 6.12.0 EXACT REQUIRED ...)` and documented the exact gate | Reconfigured and rebuilt both Qt presets; 4/4 tests passed in each |
| Raw `new` for the parent-owned `QLabel` | Retained: `QMainWindow::setCentralWidget` takes ownership, so a stack object would be destroyed twice; this is the Qt ownership contract, not an owning raw pointer leak | Code inspection; tests pass |
| Unrelated uncommitted P0-002/P0-003 record changes and a research file are present in the working tree | Excluded from the P1-001 commit; only P1-001 files are staged | Staged file list reviewed before commit |

Acceptance decision, accepting role and date: accepted by EveryGraph Implementation (main agent) on 2026-10-30 after the independent review and the rechecks above. Acceptance covers the macOS arm64 qualification only; it does not accept Windows, packaging, signing, notarization, CI or the full P1 workbench.

## Handoff and remaining work

Last verified state: Qt 6.12.0 `qtbase` is installed via Homebrew; the opt-in Qt probe configures, builds and passes Debug/Release offscreen tests under an exact `6.12.0` gate; the Qt-free Debug/Release/sanitize suites remain green; deployment and LGPL replacement experiments are recorded with their limitations; the planning validator and its unit tests pass.

Blockers and unblock actions:

- P1-003 must still pin the durable native dependency graph; the Homebrew route is provisional and must not be treated as the package-manager decision.
- P1-002 remains blocked on a Windows host or authorized CI; no Windows evidence exists.
- Release qualification must cover hardened runtime, Developer ID signing, notarization, license notices, corresponding-source access, complete SBOM and clean-machine testing.

Next task / concrete next action: P1-003 may start (P1-001 is accepted); P1-002 stays blocked pending a Windows host or authorized CI.

Files or commands needed to resume: [CMakeLists.txt](../../CMakeLists.txt), [CMakePresets.json](../../CMakePresets.json), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), `python3 tools/validate_planning.py`, and the `macos-debug-qt` / `macos-release-qt` presets with `QT_ROOT` set.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | Qt was absent and installation/network/build changes were not authorized; no execution started |
| 2026-10-30 | blocked -> in_progress | User authorized the Homebrew Qt route, build changes and a local commit |
| 2026-10-30 | in_progress -> in_review | Implementation, tests, deployment/replacement experiments and document updates completed; independent review requested |
| 2026-10-30 | in_review -> accepted | Independent review completed; findings corrected and all affected checks re-run green |
