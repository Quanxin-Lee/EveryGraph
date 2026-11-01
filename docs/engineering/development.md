# Phase 0 development and toolchain standards

## Scope and evidence boundary

This scaffold builds `everygraph_toolchain_probe`, a dependency-free C++20 standard-library executable. P1-004 adds the dependency-free `everygraph_foundation` and `everygraph_application` static libraries and the `everygraph_command_kernel_tests` executable; they implement a command, revision, transaction, undo/redo and event contract over a fake in-memory document, not a product domain model, persistence layer, GUI or wire protocol. An opt-in `everygraph_qt_probe` target adds a minimal Qt Widgets workbench qualification probe; it is not EveryGraph, a GUI prototype, a scientific implementation, or a benchmark. P1-003 adds a vcpkg manifest with an immutable baseline, a removable Qt 6.12.0 overlay, an `arm64-osx-dynamic` preset route and an initial SBOM generator; its section records the pinned graph and verification limits. P1-011 moves those targets' definitions into per-module and per-test-group CMake files without changing a target name, flag, test or output path; its section records the convention and the equivalence evidence. No product business module exists.

The probe checks `std::span`, an owning vector snapshot surviving mutation and destruction of its source, move-owned data in `std::jthread`, stop-token observation, and joining before reading worker results. A latch makes stop observation deterministic without timing assumptions. Explicit return-code checks remain active in Release; assertions are not used as tests.

## Verified local environment

Observed on the implementation host, not inferred from a compatibility table:

| Item | Observed value |
| --- | --- |
| OS | macOS 27.2, build 26B123 |
| Host and all three output binaries | arm64; Mach-O 64-bit arm64 executables |
| Selected developer directory | `/Applications/Xcode.app/Contents/Developer` |
| Compiler | AppleClang 21.0.0 (`clang-2100.3.34.2`); CMake identifies `21.0.0.21000334` |
| Compiler used by presets | `/usr/bin/clang++`, using the selected Xcode toolchain |
| SDK | macOS 27.0 |
| Deployment target | 14.4; also confirmed in the Debug binary's `LC_BUILD_VERSION` |
| CMake | 3.31.6; project minimum 3.28 |
| Ninja pip distribution | 1.11.1.4 |
| `ninja --version` | `1.11.1.git.kitware.jobserver-1` |
| Tool environment | `/tmp/everygraph-phase0-tools`, isolated Python virtual environment |
| Local Python | 3.14.6; planning validation requires an interpreter >= 3.10 |
| CMake-discovered Python | `/opt/homebrew/Frameworks/Python.framework/Versions/3.14/bin/python3.14` |
| clang-format / clang-tidy | Not found on PATH; configuration supplied, checks not executed |

The existing environment was provisioned with Python's `venv` and pip distributions `cmake==3.31.6` and `ninja==1.11.1.4`. The scaffold reused it by prefixing PATH; it installed no additional tools or libraries. The pip distribution version and the Ninja executable's reported version are deliberately recorded separately. Reproducing the provisioning method on a fresh host would use the following; these installation commands were not rerun during scaffold verification:

```sh
python3 -m venv /tmp/everygraph-phase0-tools
/tmp/everygraph-phase0-tools/bin/python -m pip install cmake==3.31.6 ninja==1.11.1.4
```

This temporary environment is not an application dependency lock and may disappear after host cleanup. Local Python 3.14.6 verification is separate from the unexecuted CI choice of Python 3.13.

Version self-checks used locally:

```sh
export PATH="/tmp/everygraph-phase0-tools/bin:$PATH"
sw_vers
uname -m
xcode-select -p
xcrun --find clang++
xcrun clang++ --version
xcrun --show-sdk-version
cmake --version
ninja --version
python --version
python -m pip show cmake ninja
```

## Actual macOS configure, build, and test commands

Run from the repository root (`/Users/leo/Documents/General/EveryGraph` on the verified host). Presets use Ninja, AppleClang, `arm64`, a 14.4 deployment target, and separate `out/build/<preset>` directories. Configure presets are Darwin-only. Build and test preset names match configure presets. Preset schema version is 6.

The following commands were executed successfully:

```sh
export PATH="/tmp/everygraph-phase0-tools/bin:$PATH"
cmake --list-presets=all

cmake --preset macos-debug
cmake --build --preset macos-debug --verbose
ctest --preset macos-debug -R '^toolchain_probe$'

cmake --preset macos-release
cmake --build --preset macos-release --verbose
ctest --preset macos-release -R '^toolchain_probe$'

cmake --preset macos-sanitize
cmake --build --preset macos-sanitize --verbose
ctest --preset macos-sanitize -R '^toolchain_probe$'

ctest --preset macos-debug -N
```

| Preset | Configure/build | Executed CTest result |
| --- | --- | --- |
| macos-debug | Passed; no compiler warnings emitted | Full suite: 3/3 passed |
| macos-release | Passed; no compiler warnings emitted | Full suite: 3/3 passed |
| macos-sanitize | Passed with ASan + UBSan compile/link flags | Full suite: 3/3 passed; no sanitizer diagnostics |

`file` confirmed all three output binaries are arm64. `xcrun vtool -show-build out/build/macos-debug/everygraph_toolchain_probe` confirmed minimum OS 14.4 and SDK 27.0. `otool -L out/build/macos-sanitize/everygraph_toolchain_probe` confirmed the ASan runtime linkage. The verbose sanitizer compile and link commands both contained `-fsanitize=address,undefined`.

The conflict guard was also exercised with `cmake --preset macos-sanitize -DEVERYGRAPH_ENABLE_TSAN=ON`: configuration failed as intended with the message that ASan/UBSan and TSan cannot be combined. The initial diagnostic-check helper failed because CMake wrapped that message across lines; normalizing whitespace made the helper pass. The normal `macos-sanitize` configure/build/test commands were rerun, restoring `EVERYGRAPH_ENABLE_TSAN=OFF` and passing the probe again. No actual probe configure/build/test failure occurred. TSan itself was not built or run.

A successful run on macOS 27.2 with a deployment target of 14.4 is not evidence of execution on macOS 14.4, other macOS releases, Intel Macs, or Windows. These are toolchain checks, not application, numerical-accuracy, throughput, or memory-scaling results.

## Full planning and toolchain validation

CMake requires `Python3` >= 3.10 with the `Interpreter` component. When `BUILD_TESTING` is on, CTest registers:

- `toolchain_probe`: the compiled C++ probe, with a 30-second timeout.
- `catalog_validation`: the Python interpreter running `tools/validate_catalog_children.py` from the source root, with a 60-second timeout.
- `catalog_validator_tests`: the 37 standard-library unittest cases for the catalog-child validator's behavior, with a 60-second timeout.
- `planning_validation`: the Python interpreter running `tools/validate_planning.py` from the source root, with a 60-second timeout.
- `planning_validator_tests`: six standard-library unittest cases for validator behavior, with a 60-second timeout.
- `command_kernel_tests`: the dependency-free command/transaction prototype executable, with a 60-second timeout.
- `module_boundary_check`: a CMake script that rejects Qt includes and Qt type tokens in `modules/foundation` and `modules/application`, with a 30-second timeout.
- `qt_probe_smoke`: only in Qt presets; the offscreen Qt Widgets probe with a 30-second timeout.

The initial compiler-only runs used the anchored selection above. Before P1-004, the final review reran configure/build and the unfiltered suites; each passed 3/3. After P1-004, the Qt-free suites report 5/5 and the two Qt presets report 6/6; details and exact commands are in the kernel section below.

```sh
ctest --preset macos-debug
ctest --preset macos-release
ctest --preset macos-sanitize
```

The six validator unit cases also passed when invoked directly with `python3 -m unittest discover -s tools -p test_validate_planning.py -v`. The validator checks preserved source hashes, coverage/anchors/classifications, local document links/fences/language and matrix arithmetic. It does not check remote URL availability, render Mermaid or test application/scientific behavior. The workflow runs full CTest so missing or failing planning checks fail CI.

### Catalog-child register checks (P0-011)

`tools/validate_catalog_children.py` and `tools/test_catalog_children.py` existed from P0-004 onward but were reachable only by direct invocation, so a regression in the catalog contract could land without failing any build or CI gate. P0-011 registers them as the two named CTest tests above, in `tools/CMakeLists.txt` — the tree that owns those scripts — guarded by `BUILD_TESTING` like every other registration.

**What they protect.** The 172-row `docs/planning/catalog-children.csv` evidence register against its pinned contract `tools/catalog_expectations.json`: child-ID grammar, sequence gaps without a recorded retirement, the seven unresolved catalog parents, required-field and enum values, verbatim local anchors with excerpt digests and registered retrieval dates, count floors and exact claim-only counts, the no-support-claim and no-vendor-count discipline, the ASCII policy outside the two exempt anchor fields, byte-level immutability of the feature register, the child register and the preserved root inventories, and agreement of the declared constants with the P0-003 and P0-005 evidence documents. `catalog_validator_tests` is the behavioral half: each case breaks one invariant and asserts the matching check reports it, so a green register proves a working check rather than a no-op.

**Prerequisite.** CMake must locate a `Python3` interpreter of version 3.10 or newer with the `Interpreter` component, which the root file requires unconditionally, and `BUILD_TESTING` must be on. Both entries run against the *source* tree, not the build tree, so the checkout must be complete: the checks read `docs/`, `tools/` and the preserved root inventory files. `WORKING_DIRECTORY` is pinned to `${CMAKE_SOURCE_DIR}` because the discovery start directory `tools` is a relative path, and the registered command is `${Python3_EXECUTABLE}`, the same interpreter handle `planning_validation` uses — Homebrew's framework 3.14 on this host, the repository `.venv` 3.13.9 when `Python3_EXECUTABLE` is pinned to it, and the CI `setup-python` interpreter in the workflow. Neither entry touches the network: the validator imports no network-capable module and both entries pass under a `sys.addaudithook` guard that rejects any `socket.*`, `os.system` or `subprocess.Popen` event.

**Failure semantics — no silent skip.** Neither registration sets `SKIP_RETURN_CODE`, so no catalog result can be reported as skipped. Measured on a throwaway copy of the repository outside the working tree: a single renamed named entry in the register fails both tests with CTest exit 8; deleting `docs/planning/catalog-children.csv` or `tools/catalog_expectations.json` fails both with a traceback rather than skipping; an interpreter that disappears after configure is reported `***Not Run` and counts as a failure; a missing interpreter at configure time fails configuration outright (`Could NOT find Python3 (missing: Interpreter)`); and renaming the test module makes `unittest discover` exit 5 with `NO TESTS RAN`, which CTest records as a failure. On a fresh `-DBUILD_TESTING=OFF` configure the four build targets still compile and zero tests register, which is the P1-011 rule that only `add_test` is guarded. Reconfiguring an *existing* build directory from `ON` to `OFF` instead leaves all registrations listed, because CMake 3.31.6 keeps the generated `CTestTestfile.cmake` files — pre-existing behavior that hits `planning_validation` and the rest exactly as it hits these two, and a route no preset takes, since every preset keeps `BUILD_TESTING` on and has its own `out/build/<preset>` directory.

Result on 2026-10-31, macOS 27.0.1 arm64, AppleClang 21.0.0.21000334, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`, Homebrew `qtbase` 6.12.0 for the Qt preset:

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug | Passed, no compiler warnings | 7/7 passed |
| macos-release | Passed, no compiler warnings | 7/7 passed |
| macos-sanitize | Passed with ASan + UBSan, no sanitizer diagnostics | 7/7 passed |
| macos-debug-qt | Passed, no compiler warnings | 8/8 passed |

The three Qt-free presets were configured, built and tested from wiped scratch build directories as well as through the recorded `out/build/<preset>` trees, so the result is a clean-configure proof rather than an incremental reconfigure. Because `.github/workflows/phase0.yml` runs the unfiltered `ctest --preset` for `macos-debug`, `macos-release` and `macos-sanitize`, these two entries are now part of the configured CI gate; the workflow itself is unchanged and has not been dispatched.

## Qt 6.12.0 Widgets probe on macOS arm64 (P1-001)

This is a qualification probe, not a product workbench. `tools/qt_probe/main.cpp` creates a `QMainWindow` with a centered `QLabel` and a status-bar version line; it contains no scientific or application business logic. Qt is opt-in: the default `macos-debug`, `macos-release` and `macos-sanitize` presets still build only the dependency-free probes, so CI and hosts without Qt are unaffected. Qt is required only by `macos-debug-qt` and `macos-release-qt`.

### Installed Qt and tools

The approved route for P1-001 was Homebrew; only the `qtbase` formula was installed, not the `qt` umbrella formula, to avoid pulling Qt Charts, Qt Graphs, Qt Data Visualization and Qt WebEngine (GPLv3-only or very large scopes) into the qualification environment.

| Item | Observed value |
| --- | --- |
| Qt version | 6.12.0, Homebrew revision `6.12.0_2` |
| Bottle | `qtbase--6.12.0_2.arm64_golden_gate.bottle.tar.gz`, SHA-256 `178ff42e7d82f5ed7c6164f6c7c4e8b735e03a4b61bd96eb970341f7329214fb` |
| Install prefix | `/opt/homebrew/opt/qtbase` (Homebrew reports `QT_INSTALL_PREFIX=/opt/homebrew`) |
| Cellar | `/opt/homebrew/Cellar/qtbase/6.12.0_2`, 5,965 files, 62.3 MB |
| Installed frameworks | QtConcurrent, QtCore, QtDBus, QtGui, QtNetwork, QtOpenGL, QtOpenGLWidgets, QtPrintSupport, QtSql, QtTest, QtWidgets, QtXml |
| License expression | `(LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only) AND (GPL-3.0-only WITH Qt-GPL-exception-1.0) AND BSD-3-Clause AND GFDL-1.3-no-invariants-only` |
| Linked by the probe | QtCore, QtGui, QtWidgets only, dynamically |
| Qt binary target | `LC_BUILD_VERSION` minimum OS 14.4, SDK 27.0 |
| Transitive runtime libraries | ICU 79.1, OpenSSL 4.1.0, glib 2.90.1, dbus 1.16.2, freetype 2.15.0, harfbuzz 15.0.0, pcre2 10.49, zstd 1.5.7, libpng, jpeg-turbo, brotli, double-conversion, md4c, libb2 |
| Homebrew SBOM | `/opt/homebrew/Cellar/qtbase/6.12.0_2/sbom.spdx.json`, SPDX-2.3, 36 packages |

The Homebrew bottle is a temporary qualification route, not the durable dependency authority. P1-003 must still pin the native graph (vcpkg manifest with an immutable baseline, or a recorded alternative) before business modules depend on Qt.

### Configure, build and test commands

Run from the repository root:

```sh
export PATH="$PWD/.venv/bin:$PATH"
export QT_ROOT="$(brew --prefix qtbase)"

cmake --preset macos-debug-qt
cmake --build --preset macos-debug-qt
ctest --preset macos-debug-qt

cmake --preset macos-release-qt
cmake --build --preset macos-release-qt
ctest --preset macos-release-qt

# Existing Qt-free suites
cmake --preset macos-debug && cmake --build --preset macos-debug && ctest --preset macos-debug
cmake --preset macos-release && cmake --build --preset macos-release && ctest --preset macos-release
cmake --preset macos-sanitize && cmake --build --preset macos-sanitize && ctest --preset macos-sanitize
```

The Qt presets set `EVERYGRAPH_ENABLE_QT_PROBE=ON` and `CMAKE_PREFIX_PATH=$env{QT_ROOT}`. `find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Widgets)` rejects other Qt versions, but it does not require `QT_ROOT`: if the variable is unset, CMake may still discover Qt through its default system prefix (on this host, `/opt/homebrew`). Set `QT_ROOT` to make the chosen Qt installation explicit. `qt_standard_project_setup()` enables automatic moc/uic; the current probe declares no `Q_OBJECT` types, so the generated moc compilation unit is empty.

| Preset | Result |
| --- | --- |
| macos-debug-qt | Configured and built with no compiler warnings; CTest 4/4 passed |
| macos-release-qt | Configured and built with no compiler warnings; CTest 4/4 passed |
| macos-debug | CTest 3/3 passed |
| macos-release | CTest 3/3 passed |
| macos-sanitize | CTest 3/3 passed with ASan + UBSan |

These are the historical P1-001 counts. After P1-004 added the kernel CTest entries and the P0 research artifact was committed, the current `main` tree passes 5/5 in the three Qt-free presets and 6/6 in the two Qt presets; the shared planning validator also passes. The kernel section below records the exact rerun.

`qt_probe_smoke` runs `everygraph_qt_probe --smoke-test` with `QT_QPA_PLATFORM=offscreen`. The probe verifies that a window and central widget exist, that the window is visible, and that the platform plugin is `offscreen`, then quits through a zero-delay timer. It does not require a logged-in GUI session and does not assert on wall-clock timing. Typical observed duration was 0.8 s after the first run; the first run took about 5 s while font caches were populated. The probe prints `qt_probe: Qt 6.12.0, platform offscreen, central widget present` and exits 0. The offscreen plugin also prints `This plugin does not support propagateSizeHints()` and a font-alias timing note; neither is a test failure.

### Dynamic linkage, deployment and LGPL replacement

Build-tree linkage is dynamic and points at the Homebrew prefix:

```text
/opt/homebrew/opt/qtbase/lib/QtWidgets.framework/Versions/A/QtWidgets
/opt/homebrew/opt/qtbase/lib/QtGui.framework/Versions/A/QtGui
/opt/homebrew/opt/qtbase/lib/QtCore.framework/Versions/A/QtCore
```

`macdeployqt` from `qtbase` deployed a copy of the Release bundle:

```sh
cp -R out/build/macos-release-qt/everygraph_qt_probe.app out/qt-deploy/
/opt/homebrew/opt/qtbase/bin/macdeployqt out/qt-deploy/everygraph_qt_probe.app
cp /opt/homebrew/share/qt/plugins/platforms/libqoffscreen.dylib \
  out/qt-deploy/everygraph_qt_probe.app/Contents/PlugIns/platforms/
```

The absolute `macdeployqt` path matters on this host: Anaconda earlier on `PATH` shadows the Homebrew tool. The tool copied the Qt frameworks and 22 transitive dylibs into `Contents/Frameworks`, rewrote the executable to `@executable_path/../Frameworks/...`, added the matching `LC_RPATH`, and ad-hoc signed the bundle (`flags=0x2(adhoc)`). No Homebrew dependency edge remained: every resolved library loaded from the bundle. Some bundled libraries still carry a Homebrew path in their own `LC_ID_DYLIB` (for example QtDBus and libbrotlicommon), which is an install name rather than a dependency and does not redirect loading. The tool copied only the `cocoa` platform plugin by default; the `offscreen` plugin was added manually for the headless check. Copying that plugin and later modifying QtCore invalidated the bundle's code-signature seal: `codesign --verify --deep --strict` reports an added file and a modified framework, and `spctl` rejects the bundle. This is expected for the local experiment and is not a distributable or notarized artifact.

The deployed bundle then ran with a clean environment and no Homebrew paths:

```sh
env -i HOME="$HOME" PATH=/usr/bin:/bin QT_QPA_PLATFORM=offscreen \
  out/qt-deploy/everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe --smoke-test
```

It exited 0, and `DYLD_PRINT_LIBRARIES=1` showed QtCore, QtGui, QtWidgets and `libqoffscreen.dylib` loading from the bundle. This is local deployment evidence, not a packaging, signing or notarization result.

The LGPL replacement experiment modified the deployed QtCore framework itself:

1. The deployed copy hashed `c5a7b187ac06360d7e6c7fbc737e89cbfc8fe760528f2d5bdef7d28ab0382dd5`; the Homebrew original hashed `5ef20b686ee05ec97105b0b7ec09e9c2e5a796e1243dbb5c6fdeb3d99033a4c8`, because `macdeployqt` rewrites install names in the deployed copy.
2. `install_name_tool -add_rpath /tmp/everygraph-lgpl-replacement` and `codesign --force --sign -` produced a modified framework hashing `c5ac0a6ea1d7fd980cc2b56f021484db2256230cac895b19c50c9648b4c16faf` and passing `codesign --verify`.
3. The bundle ran the modified QtCore from its own `Contents/Frameworks` and still passed the offscreen smoke test.

This demonstrates local replacement of an LGPL Qt library without relinking the application. The modified framework was re-signed ad-hoc before the run and `codesign --verify` passes for the framework itself; loading an unsigned modified framework was not tested. The bundle's overall seal remains invalid after the experiment, and hardened runtime, library validation, Developer ID signing, notarization and App Store rules are not tested; those remain release gates.

### Limits of this qualification

- Only macOS arm64 on macOS 27.0.1 was executed. The 14.4 deployment target and the Qt frameworks' 14.4 `LC_BUILD_VERSION` are recorded, but nothing ran on macOS 14.4 hardware or an Intel Mac.
- No Windows build, packaging, signing, notarization, CI change or release artifact is claimed. The Qt presets are local-only; CI does not install Qt.
- Sanitizers were not combined with the Qt probe; only the existing Qt-free sanitize suite was run. Qt and ASan/UBSan interaction is unqualified.
- The Homebrew Qt binary is not a project-built Qt and its transitive closure is not yet an approved release SBOM. P1-003 and release qualification still own the dependency graph, notices, corresponding source and clean-machine checks.
- `macdeployqt` did not deploy the offscreen platform plugin automatically; a real test/CI packaging flow must deploy the plugins it needs explicitly.

## Command and transaction kernel prototype (P1-004)

`everygraph_foundation` and `everygraph_application` are C++20 static libraries with no GUI toolkit, network, persistence or third-party dependency. The application prototype uses a fake in-memory document and is not a product Project/Workbook/Graph model, a SQLite/Arrow store, a numerical engine, a wire protocol, a permission system or a user interface. It exists so the Qt shell, Python, CLI and future AI clients can consume one command/transaction contract instead of inventing their own mutation path. It does not complete EG-ADD-001 or P1.

### Targets and contract

| Target / test | Purpose |
| --- | --- |
| `everygraph_foundation` | Stable `ObjectId`, revision/transaction/event IDs and structured `ErrorCode` values; standard library only |
| `everygraph_application` | Command envelope, versioned registry, fake document transaction, revision checks, gateway, undo/redo, sequenced events, repeat/reopen metadata and cancellation; links foundation only |
| `everygraph_command_kernel_tests` | Dependency-free C++ test executable containing 26 cases; no GoogleTest |
| `module_boundary_check` | CTest architecture check for Qt includes and type tokens in both module trees |

Public headers live under `modules/foundation/include/everygraph/foundation/` and `modules/application/include/everygraph/application/`; implementations live in the matching `src/` directories. The gateway is a single-threaded application-executor prototype. A command receives a private `CommandTransaction` candidate; only the gateway commits it, and the live document is swapped once. The prototype enforces:

- Command IDs use dotted lowercase `namespace.verb` names. Unknown commands, duplicate registrations, unsupported schema versions, malformed arguments, invalid envelopes and external-effect registrations return stable structured error codes.
- Declared read and write revisions must exactly match current revisions before execution; a stale read or write returns `revision_conflict` without staging or publishing anything. The gateway revalidates the expected revisions again immediately before commit.
- A handler may change only objects in its declared write set; undeclared writes are rejected before commit. A read-only command that attempts a write is rejected.
- A successful changed command creates one undo entry, increments every changed object's revision, updates the monotonic event sequence once and publishes one event containing the changed IDs, revisions and change kinds. A no-change command publishes no event, revision or undo entry.
- Property no-op detection uses canonical value equality: all NaN representations compare equal, positive and negative zero are distinct, and other values compare by type and value. This keeps no-op events and revision bumps consistent with the canonical parameter representation.
- Undo and redo apply the stored before/after content through the same executor, assign fresh revisions rather than reusing old numbers, publish their own sequenced events and do not add undo entries. A new edit clears the redo branch. Retired object IDs keep a tombstone revision so recreation does not reuse a revision number.
- Event listeners are invoked after the state change; listener exceptions are contained, and a listener that calls back into the gateway receives `reentrant_mutation` instead of mutating the document reentrantly. A handler is also guarded against reentrant gateway calls, so an inner attempt cannot commit a candidate that the outer command then overwrites.
- Every successful changed command records immutable parameter metadata. `recorded_command()` returns a request with the recorded command, schema version and copied arguments, plus current expected revisions for the original read/write IDs. `repeat_last()` re-executes only commands marked repeatable; repeated idempotent edits produce no additional result, revision, event or undo entry.
- `std::stop_token` is checked before dispatch and before commit. A canceled handler candidate is discarded with `canceled`; it publishes no event or revision.

### Commands and results

Run from the repository root:

```sh
export PATH="$PWD/.venv/bin:$PATH"

cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug

cmake --preset macos-release
cmake --build --preset macos-release
ctest --preset macos-release

cmake --preset macos-sanitize
cmake --build --preset macos-sanitize
ctest --preset macos-sanitize
```

| Preset | Configure/build | CTest result |
| --- | --- | --- |
| macos-debug | Passed; no compiler warnings | 5/5 passed |
| macos-release | Passed; no compiler warnings | 5/5 passed |
| macos-sanitize | Passed with ASan + UBSan | 5/5 passed; no sanitizer diagnostics |

These results were rerun on `main` at `02648bb` after the missing P0 research artifact was committed. The two Qt presets were rerun as well and each passed 6/6, including `qt_probe_smoke`. `everygraph_command_kernel_tests` does not link Qt in any preset. `otool -L` on the Debug test executable lists only `/usr/lib/libc++.1.dylib` and `/usr/lib/libSystem.B.dylib`; `file` reports a Mach-O arm64 executable.

Resolved 2026-10-31: the P0-004, P0-005 and P0-006 records now link to a committed `docs/research/capability-catalog-decomposition.md`; the planning validator passes with 48 documents in the final working tree (47 committed plus one concurrent untracked SBOM README) and all five presets pass. The artifact remains owned by the P0 documentation tasks.

The 26 cases cover registry rejection (unknown, duplicate, invalid ID, unsupported version, malformed arguments, external effect), stale read/write revisions, a multi-object create/update transaction, single undo entry and event sequencing, undo/redo state and fresh revisions, redo-branch clearing, empty history, pre-dispatch and mid-handler cancellation, stop-after-handler-before-commit, handler exceptions, rejected candidates, undeclared writes, handler reentrancy, NaN and signed-zero no-op behavior, read-only effect enforcement, listener exception containment, listener reentrancy, repeat with no duplicate result, non-repeatable rejection, immutable reopen metadata and envelope validation.

### Architecture boundary check

Two independent checks keep the GUI-independent modules free of Qt:

1. CMake configure time calls `everygraph_assert_no_qt_linkage()` for both targets and fails if a direct or exported link dependency matches `Qt<digits>::`.
2. The `module_boundary_check` CTest scans every `.hpp`, `.h`, `.cpp` and `.cc` file in the two module trees for Qt includes and `Q`-prefixed type tokens.

The negative path was exercised with a scratch tree containing `#include <QString>`; the check failed and reported the offending file and line. The check does not prove the absence of every transitive non-Qt dependency and does not replace compiler-enforced layering; it is a scoped guard for this prototype.

### Prototype limits

- The executor is synchronous and single-threaded. Stop tokens are checked at command boundaries; no worker, task handle, progress stream or serialized command queue exists (P1-005 owns that contract).
- The document is a fake `std::map`; every command and history operation copies it. There is no Project/Workbook domain, immutable chunk store, SQLite/Arrow persistence, query service, pagination or snapshot leasing.
- Command effects are limited to reversible in-memory document edits. External writes, network transfer, capability grants, confirmation receipts, idempotency/retry caches, result modes, per-effect receipts and partial-completion reporting are not implemented.
- Cancellation after commit is not tested because the prototype has no long-running work or external effects. Undo history is capped at 128 entries by default; discarded entries and their memory budget are not surfaced. Tombstone revisions are retained for the gateway lifetime.
- Undo and redo do not rewrite the last-action record; it continues to describe the most recent changed command. `repeat_last()` is a contract-level reproduction, not the final Origin-style recalculation semantics with new selections or parameters.
- Listener exceptions are contained without diagnostics. Allocation failures during request validation or metadata access can still propagate before a commit is staged; commit-time allocations are staged before the live document and tombstone maps are swapped, so a failed commit leaves the committed state unchanged.
- The test harness is a small custom executable with explicit return-code checks and one CTest entry, not GoogleTest with discovery, fixtures or death tests. GoogleTest remains a future pinned dependency under P1-003.
- No Windows build, CI run, sanitizer-plus-Qt combination or thread-sanitizer run is claimed. `macos-debug-qt` and `macos-release-qt` are local qualification runs, not release packaging.

## Worker handshake and cancellation prototype (P1-005)

P1-005 adds a versioned worker protocol, an in-process transport and a separate test-worker process to the application prototype. Fake bounded jobs exercise the contract; no Python, native plugin, AI provider, numerical kernel or real scientific worker is involved. The prototype is macOS/POSIX crash containment, **not an OS sandbox**, and it makes no untrusted-code containment claim. No code path changed after the artifact commit `0e0579b`; this section records the accepted build facts and supersedes the P1-004 limit sentence that said no worker, task handle, progress stream or serialized command queue existed.

### Targets and registration

| Target / test | Purpose |
| --- | --- |
| `everygraph_application` | Gains `worker_protocol.cpp`, `job_scheduler.cpp` and the private `child_process.cpp` helper, and links `Threads::Threads` in addition to `everygraph_foundation`. It is not a new target; the P1-004 description "links foundation only" is superseded for it. |
| `everygraph_test_worker` | New second executable, defined in `tests/worker/CMakeLists.txt` and linked to `everygraph_application`; one cooperative mode plus 11 injected faults. |
| `everygraph_worker_protocol_tests` | New dependency-free 50-case test executable; its compile definition `EVERYGRAPH_TEST_WORKER_PROGRAM` names the `$<TARGET_FILE:everygraph_test_worker>` path. |
| `worker_protocol_tests` | The single CTest entry for that executable, `WORKING_DIRECTORY` pinned to the binary directory and a 180-second timeout. It takes the three Qt-free presets from 7/7 to 8/8. |

### Protocol and test-worker mechanism

The wire format is schema version 1: one percent-encoded `key=value;` frame per line, bounded at 4096 bytes, with eight message kinds (submit, accepted, progress, cancel, result, failure, superseded and shutdown) and eleven failure reasons. The codec rejects unknown, duplicate or malformed fields, unsupported schema versions, oversized or multiline frames and raw control characters.

In-process jobs observe a `std::stop_token` at bounded checkpoints. Child jobs are spawned with `posix_spawn`; the host sends cancel and superseded control frames over a pipe, waits a bounded grace period and then escalates to `SIGKILL`. The scheduler blocks `SIGPIPE` on the threads that own the pipes, resets the child's signal disposition and mask, and redirects the worker's stderr so diagnostics cannot corrupt a frame. The test worker can be told to overrun its deadline, crash, exit early, emit malformed or unsupported frames, use a foreign identity, echo a host frame, report progress beyond its total, send a late result or ignore cancellation; those modes are how the failure-injection cases reach the host's timeout, crash, protocol and supersession paths.

Progress updates are bounded per stage and coalesced by `ProgressAggregator`; a progress count above its total is refused as a protocol violation. Admission checks run at submit time against a byte budget and a bounded pending queue, and a rejected job takes no reservation and starts no worker. `commit_worker_result` is the only path into the P1-004 command gateway: it accepts a succeeded candidate carrying a value and refuses canceled, superseded, failed and stale candidates, so a failed or canceled worker publishes no revision or event.

### Recorded result

On 2026-10-31, macOS 27.0.1 arm64, AppleClang 21.0.0.21000334, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`:

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug | Passed, no compiler warnings | 8/8 passed; direct binary 50/50 |
| macos-release | Passed, no compiler warnings | 8/8 passed; direct binary 50/50 |
| macos-sanitize | Passed with ASan + UBSan, no sanitizer diagnostics | 8/8 passed; direct binary 50/50 |

The thread-race check was an out-of-tree `-DEVERYGRAPH_ENABLE_TSAN=ON` configuration, not a fourth preset and not CI: it built with no warnings, passed 8/8 under CTest and 50/50 directly, with no ThreadSanitizer diagnostic. The worker group is selected with `ctest --preset macos-debug -R '^worker_protocol_tests$'` or run directly as `./out/build/macos-debug/everygraph_worker_protocol_tests`.

### Limits

- **No sandbox.** Process separation is crash containment only. No OS sandbox, capability grant, operating-system-enforced resource limit or untrusted-code test exists; a real worker transport needs its own containment and rights review.
- macOS/POSIX only. The child transport is untested on Linux, Windows and any non-arm64 host.
- Records and superseded outcomes are never pruned, escalation is `SIGKILL`-only after a fixed grace period, and the worker's stderr is discarded with no diagnostic channel.
- The jobs are fake bounded loops over an in-memory test document; no scientific semantics, throughput, memory-scaling or production-scheduler claim is made.
- CI execution, Windows execution and a Qt combination are not claimed. The test harness is the same dependency-free CTest executable pattern as P1-004, not GoogleTest.

## Pinned native dependency graph and initial SBOM (P1-003)

P1-003 replaces the temporary Homebrew-only dependency route with a vcpkg manifest, an immutable registry baseline and a local Qt 6.12.0 overlay for macOS arm64. This is a qualification graph for the Qt probe, not a product dependency set, a packaging result or a Windows qualification. The Homebrew section above remains historical P1-001 evidence.

### Pin, overlay and triplet

| Item | Value |
| --- | --- |
| vcpkg tool | `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` |
| Registry baseline | `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091` (immutable commit) |
| Target triplet | `arm64-osx-dynamic` (shared libraries) |
| Host triplet | `arm64-osx` (build tools only) |
| Qt | 6.12.0, local overlay `cmake/vcpkg-overlay-ports/qtbase/` |
| Qt features | `freetype`, `gui`, `widgets`; default features disabled |
| Qt source | `qtbase-everywhere-src-6.12.0.tar.xz`, SHA-512 `87b3bd96244e03ea6145e00d088cb1e0cfaa64a575bfe32c4ec5f0227bc7872a3777ccf29c8e54e567a6ea809a8ee894e88e659d14d96655b0730455d9869d7a` |
| Manifest files | `vcpkg.json`, `vcpkg-configuration.json` |

The curated registry contains `qtbase` 6.11.2, not the P1-001-qualified 6.12.0. The overlay is a copy of the registry port at the pinned baseline; four files are modified: the port version, `cmake/qt_port_details.cmake`, `port.data.cmake` (source URL/SHA-512/filename) and `portfile.cmake`. The obsolete `use_inotify_on_freebsd.patch` is deleted because Qt 6.12.0 already contains that change, and the portfile adds Qt 6.12.0's harmony tools to `TOOL_NAMES` because the generated host tools config exports them; without that, a native target Qt configure fails on missing tools. Remove the overlay and re-pin when the curated registry ships Qt 6.12.

A second small overlay, `cmake/vcpkg-overlay-ports/pcre2/`, is a copy of the registry pcre2 10.49#2 port. The GitHub API gitlink lookup and its unused head-version branch are removed and replaced by the pinned sljit commit `de0259c7aaf36aa40cba8014f3fad3edde9307f9`; the pinned-version path, PCRE2 JIT, the version, features, patches and the sljit SHA-512 are unchanged. It exists because this host cannot reach `api.github.com`. The commit-to-PCRE2-tag link was not independently re-checked because GitHub is unavailable; it is bound by the registry sljit SHA-512. The pcre2, libb2, md4c and sljit source archives were seeded from a GitHub mirror (`gh-proxy.com`) after verifying the vcpkg SHA-512 pins; they are build inputs under the ignored `out/` tree, not committed. Remove both workarounds when direct GitHub access is available and the registry pcre2 port can be used directly.

`arm64-osx` is vcpkg's static triplet. The project policy selects shared Qt for the LGPL replacement route, so the target uses the community `arm64-osx-dynamic` triplet; host tools remain `arm64-osx`.

The `freetype` feature is enabled because Qt builds its `offscreen` QPA platform plugin only when `QT_FEATURE_freetype` is on, and the existing probe smoke test requires the offscreen platform for headless runs. The feature adds `freetype`, `brotli`, `bzip2` and `libpng` to the target graph.

The vcpkg presets set `VCPKG_APPLOCAL_DEPS=OFF`: applocal copies Qt libraries into the probe bundle but not the platform plugins, so the build-tree probe keeps referencing the pinned install root instead. `qt_probe_smoke` sets `QT_PLUGIN_PATH` to the pinned Qt plugin directory (and `QT_QPA_PLATFORM=offscreen`). Self-contained bundle deployment, plugin copying and signing are delivered by [P1-012](#macos-bundle-ad-hoc-hardened-runtime-and-lgpl-substitution-p1-012), not by this qualification route.

### Prerequisites and bootstrap

vcpkg needs a C++ toolchain plus `pkg-config` and GNU autotools for some ports. On this host those were absent, so `pkgconf` 3.0.3, GNU m4 1.4.19, autoconf 2.72, automake 1.17, GNU libtool 2.5.4 and autoconf-archive 2024.10.16 were built in the ignored `out/` tree and added to `PATH`/`ACLOCAL_PATH`. A normal developer should install the equivalent system packages (for example `brew install pkg-config autoconf automake libtool autoconf-archive`) instead. vcpkg downloaded its own CMake 4.4.4 for port builds; the project still configures with CMake 3.31.6. Direct GitHub and the GitHub API were unavailable on this host; the workaround and source seeding are described above rather than silently bypassed.

```sh
git clone https://github.com/microsoft/vcpkg.git out/vcpkg
git -C out/vcpkg checkout d186940b2f0c4e8e5b4ff804af57d2a26fbfa091
out/vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT="$PWD/out/vcpkg"
```

`out/vcpkg` is ignored by Git. The baseline commit, not the mutable registry HEAD, defines the dependency graph.

### Resolve, build and test

Run from the repository root:

```sh
export PATH="$PWD/.venv/bin:$PATH"
export VCPKG_ROOT="$PWD/out/vcpkg"

cmake --preset macos-debug-qt-vcpkg
cmake --build --preset macos-debug-qt-vcpkg
ctest --preset macos-debug-qt-vcpkg

cmake --preset macos-release-qt-vcpkg
cmake --build --preset macos-release-qt-vcpkg
ctest --preset macos-release-qt-vcpkg
```

The presets select the vcpkg toolchain, set `VCPKG_MANIFEST_DIR` to the source root and use `arm64-osx-dynamic`/`arm64-osx` as target/host triplets. `vcpkg-configuration.json` supplies the overlay port. The toolchain resolves the manifest at configure time and installs into `out/build/<preset>/vcpkg_installed`.

### Resolved graph, licenses and SBOM

The resolved package versions and license evidence are generated into `docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json` by the command below; `docs/engineering/sbom/license-overrides.json` records the evidence for packages whose vcpkg manifest declares no license. Generate the SBOM after the Release build:

```sh
python3 docs/engineering/sbom/generate_sbom.py \
  --install-root out/build/macos-release-qt-vcpkg/vcpkg_installed \
  --probe out/build/macos-release-qt-vcpkg/everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe \
  --vcpkg-root "$PWD/out/vcpkg" \
  --triplet arm64-osx-dynamic \
  --baseline d186940b2f0c4e8e5b4ff804af57d2a26fbfa091 \
  --vcpkg-tool-version 2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b
```

The generator reads the actual `vcpkg/status` (base and feature records), the per-package file lists and the probe's Mach-O linkage. Feature-record `Depends` entries are attributed to their package and deduplicated; enabled features that declare a license (for example pcre2's `jit`) are merged into `licenseDeclared`. It fails if a declared dependency cannot be resolved, if a linked vcpkg-owned library has no package owner, or if a status version differs from the version embedded in the package's vcpkg file-list filename where present. It includes target `arm64-osx-dynamic` packages and host `arm64-osx` build tools, marks host tools in comments, and excludes macOS system libraries. vcpkg packages are package-level; linked artifacts are standalone file entries with the owning package named in the comment. The output is a qualification SBOM, not a release notice bundle or legal approval; see `docs/engineering/sbom/README.md`.

The generator unit tests run with `python3 -m unittest discover -s docs/engineering/sbom -p 'test_generate_sbom.py' -v`.

### P1-003 result on 2026-10-31

Environment: macOS 27.0.1 build 26A434, arm64; AppleClang 21.0.0.21000334; macOS SDK 27.0; deployment target 14.4; CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`; vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` at baseline `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`; locally built pkgconf/autotools in the ignored `out/` tree as described above; direct GitHub and the GitHub API unavailable, with the seeded SHA-512-verified sources and the pcre2 overlay.

| Check | Exact command | Result |
| --- | --- | --- |
| Repository manifest resolution | `out/vcpkg/vcpkg install --dry-run --triplet=arm64-osx-dynamic --x-install-root=out/vcpkg-612-test/repo-dryrun2` | Exit 0; 42 packages planned; `qtbase` 6.12.0 and `pcre2` 10.49#2 resolve from the two overlay ports; `version>= 6.12.0` satisfied |
| Target graph build | `out/vcpkg/vcpkg install --x-manifest-root=out/vcpkg-612-test/minimal --triplet=arm64-osx-dynamic --overlay-ports=$PWD/cmake/vcpkg-overlay-ports --x-install-root=out/vcpkg-612-test/dyn-installed` | Exit 0; 42 installed (29 host `arm64-osx` + 13 target `arm64-osx-dynamic`); target Qt 6.12.0 shared; `libqoffscreen.dylib` present; 6.7 min for the `freetype` rebuild |
| Release preset | `cmake --preset macos-release-qt-vcpkg`; `cmake --build --preset macos-release-qt-vcpkg`; `ctest --preset macos-release-qt-vcpkg` | Configure and build passed; CTest 6/6, including `qt_probe_smoke` on the offscreen platform |
| Debug preset | `cmake --preset macos-debug-qt-vcpkg`; `cmake --build --preset macos-debug-qt-vcpkg`; `ctest --preset macos-debug-qt-vcpkg` | Configure and build passed; CTest 6/6, including the debug-plugin offscreen smoke test |
| Qt-free regression | Fresh `cmake --preset macos-debug`, `macos-release`, `macos-sanitize` with build and `ctest` | 5/5 in each preset |
| SBOM | `python3 docs/engineering/sbom/generate_sbom.py ...` (command above) | Wrote the SPDX 2.3 document: 43 packages (29 host + 13 target + probe), 4 files (probe plus the three directly linked Qt dylibs), 111 relationships including feature-record dependencies; no home-directory paths; file-list versions cross-checked |

Limits: Windows is not resolved or executed; the SBOM is qualification-scoped and not a release notice bundle or legal approval. `VCPKG_APPLOCAL_DEPS=OFF` means the build-tree probe references the pinned install root rather than a self-contained bundle; bundle deployment and local ad-hoc signing are delivered by P1-012, whose route is documented above and which explicitly excludes notarization, stapling and clean-machine Gatekeeper qualification. Host `arm64-osx` tools are build-time only and are not distributed. Direct GitHub access was unavailable, so the pcre2 overlay and mirror-seeded sources remain removable workarounds.

The P1-003 task record holds the detailed evidence, review history and acceptance state.

## Qt workbench shell and document host skeleton (P1-006)

P1-006 adds the first product-shaped Qt Widgets layer: a `QMainWindow` workbench with menus, one command toolbar, a status bar, a read-only project-tree dock, a read-only properties dock and a central tabbed document host with detach and re-dock. Actions dispatch fake `project.*` commands through the P1-004 gateway from the composition root; widgets contain no scientific logic and never touch a document directly. Widgets read snapshots through the read-only `CommandQuery` port and submit writes only through `ShellCommandDispatcher`-backed bindings; no widget source holds the command gateway. The project objects are in-memory fakes, and this is a shell skeleton, not a data editor, plot application or release bundle.

### Targets and registration

| Target / test | Purpose |
| --- | --- |
| `everygraph_presentation_qt` | New static presentation module with seven sources: `ShellCommandDispatcher` and the read-only `CommandQuery` port, `DocumentHost`/`DetachedDocumentWindow`, `WindowPlacement`, programmatic `WorkbenchIcon` painting, read-only `ProjectTreeModel`, read-only `PropertiesPanel` and `WorkbenchWindow`. Links `everygraph_application`, Qt Core/Gui/Widgets; AUTOMOC is enabled for the five `QObject` classes in four presentation headers. |
| `everygraph_desktop_fakes` | New Qt-free static library in `apps/desktop` holding the fake seed document, three registered `project.create_object`, `project.move_object` and `project.find_object` commands and `FakeProjectController`. Links the application kernel only. |
| `everygraph_desktop` | New `MACOSX_BUNDLE` executable in `apps/desktop`; the composition root wires the fake controller into the window bindings. `--smoke-test` shows the window and quits immediately. |
| `everygraph_workbench_tests` | New Qt Test executable with nine slots covering startup composition, toolbar accessibility and high-DPI icons, the nested project tree, create/move/find dispatch through the gateway, monotonic object IDs across undo, undo/redo, the read-only properties panel, tabs/detach/dock/off-screen restore and window placement. |
| `workbench_tests`, `workbench_smoke`, `shell_boundary_check` | Three CTest entries. The first two run on the offscreen platform with `QT_PLUGIN_PATH` pinned to the selected vcpkg plugin directory; the third checks that kernel public headers stay Qt-free, kernel targets do not link the shell, presentation and desktop widget sources never include or hold `CommandGateway` or call kernel write methods, the root option defaults to `OFF`, and no vendor image assets or `fromTheme` calls exist. |

Both desktop presets set the new `EVERYGRAPH_ENABLE_DESKTOP_SHELL=ON`; the Qt-free presets leave it `OFF`, so they neither find Qt nor build the shell. The presentation module is not linked by `everygraph_foundation`, `everygraph_application` or the Qt-free tests, and a configure-time guard rejects such a link.

### Dependency delta and exact source

AC-6 requires Qt Test. `vcpkg.json` therefore gains one opt-in root feature, `desktop-shell`, whose dependency is the already-pinned `qtbase` with `default-features: false` and the additional `testlib` feature. The registry baseline, the overlay port, the Qt version, the source archive and every existing dependency are unchanged:

| Item | Value |
| --- | --- |
| Registry baseline | `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091` |
| Qt | 6.12.0 from local overlay `cmake/vcpkg-overlay-ports/qtbase/` |
| Qt source | `qtbase-everywhere-src-6.12.0.tar.xz`, SHA-512 `87b3bd96244e03ea6145e00d088cb1e0cfaa64a575bfe32c4ec5f0227bc7872a3777ccf29c8e54e567a6ea809a8ee894e88e659d14d96655b0730455d9869d7a` |
| Build features | Base graph remains `freetype`, `gui`, `widgets`; the `desktop-shell` feature adds `testlib` |
| Triplets and install root | `arm64-osx-dynamic` / `arm64-osx`; `out/vcpkg-installed`, shared by the debug and release desktop presets so Qt is built once |

The resolved port set is unchanged: the probe install root and `out/vcpkg-installed` both contain the same 42 ports (29 host + 13 target) at identical versions, and `docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json` still describes 43 SPDX packages. `testlib` adds `Qt6Test` headers and `libQt6Test.dylib` to the existing `qtbase` package; it does not add a port. The probe presets do not enable `desktop-shell`, so the P1-003 qualification graph and SBOM remain valid.

### Commands and recorded result

On 2026-10-31, macOS 27.0.1 arm64, AppleClang 21.0.0.21000334, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`, vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` at the pinned baseline, and the locally built pkgconf/autotools prefixes described above on `PATH`:

```sh
export PATH="$PWD/.venv/bin:$PWD/out/pkgconf-install/bin:$PWD/out/autotools-install/bin:$PATH"
export VCPKG_ROOT="$PWD/out/vcpkg"

cmake --preset macos-debug-desktop
cmake --build --preset macos-debug-desktop
ctest --preset macos-debug-desktop

cmake --preset macos-release-desktop
cmake --build --preset macos-release-desktop
ctest --preset macos-release-desktop

cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug

cmake --preset macos-release
cmake --build --preset macos-release
ctest --preset macos-release
```

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug-desktop | Passed from a clean tree, no compiler warnings; links `libQt6Test_debug` | 11/11 passed, including `workbench_tests`, `workbench_smoke` and `shell_boundary_check` |
| macos-release-desktop | Passed from a clean tree, no compiler warnings | 11/11 passed; Qt Test reports library 6.12.0 on the offscreen platform |
| macos-debug | Passed; Qt-free regression | 8/8 passed |
| macos-release | Passed; Qt-free regression | 8/8 passed |

`python3 tools/validate_planning.py` passes, including the English-policy and link checks. The task-record checker reports the pre-existing accepted-record findings, the two expected duplicate-claim reports for this task's designated `CMakeLists.txt` and `docs/engineering/development.md` rows, and the known P1-006 record-versus-board divergence notice; it does not resolve P0-000 designations, and P0-000 syncs the board row after acceptance. The P1-006 record holds the detailed evidence, independent review history and acceptance state.

### Limits

- The shell uses fake in-memory objects with stable integer IDs. There is no Project/Workbook/Graph domain, file import or persistence, SQLite/Arrow store, virtual table, plot canvas, numerical analysis, AI or plugin panel.
- The project tree and properties panel are read-only views rebuilt from the `CommandQuery` snapshot; there is no inline rename, drag-and-drop, multi-selection or property editing. Move is a single-object command to a fake Scratch folder.
- Window placement is tested against synthetic screen rectangles, not physical display reconfiguration. The workbench re-runs placement on `QGuiApplication::screenAdded` and `screenRemoved`, but a real monitor change, a native platform window, a multi-monitor DPI transition and session restore are not exercised or claimed.
- Icons are painted programmatically at 16/32/64 physical pixels. No vendor icon set, Qt resource bundle or platform theme icon is used, and no visual-design review is claimed.
- The offscreen platform exercises widget composition and signals, not a human-visible rendering or accessibility-tree audit. Toolbar actions expose accessible names; full keyboard-navigation, screen-reader and high-contrast verification remain future work.
- macOS arm64 only. No Windows build, CI execution, sanitizer-plus-Qt run, signing, notarization, packaging or deployment claim is made; `VCPKG_APPLOCAL_DEPS=OFF` still means the build-tree application references the pinned install root.

## Preferences, appearance and focus mode (P1-007)

P1-007 makes the P1-006 workbench configurable without mixing user preferences with project or scientific state. Interface, focus-mode, function-library and default-file-format settings are held in a typed `Preferences` value and a versioned JSON file outside the project; the file never contains dataset values, scene colors or export settings. Appearance recolors shell chrome only, and focus modes are panel-visibility presets. The function library remains a read-only placeholder list; no function executes, no project is saved and the default file format is a preference only.

### Targets and registration

| Target / test | Purpose |
| --- | --- |
| `everygraph_presentation_qt` | Adds `Preferences` and its enum names, `PreferencesStore`, `PreferencesDialog`, `ShellAppearance` and the read-only `FunctionLibraryPanel`; `WorkbenchWindow` gains a function-library dock, a default-format status label, an Edit > Preferences action, three exclusive focus-mode actions, layout save/restore and preference persistence. All shell-only behavior stays in the presentation module. |
| `everygraph_preferences_tests` | New Qt Test executable with twenty-three test methods plus init/cleanup: missing-file defaults, environment path override, full round trip, two-process application restart (including the three interface visibility booleans), invalid JSON/root/schema/type/value/window-state fallbacks, non-object groups and non-numeric schema, an unrestorable saved layout, read and write failures with previous-file preservation, project-state isolation, dialog editing and restore-defaults, shell-only appearance isolation for menu bar, status bar, toolbar, menu, dock and dock content, focus-mode panel toggles and keyboard shortcuts on a 1024x600 window, normal-mode visibility preferences, the default-format label, dock resize/rearrange persistence across a simulated restart, and the read-only function library. |
| `preferences_tests` | New CTest entry on the offscreen platform with the same pinned `QT_PLUGIN_PATH` and a test-only `EVERYGRAPH_PREFERENCES_PATH` as `workbench_tests` and `workbench_smoke`. |

### Settings location and schema

`PreferencesStore::default_file_path()` resolves to `EVERYGRAPH_PREFERENCES_PATH` when that variable is non-empty, otherwise to `QStandardPaths::AppConfigLocation/preferences.json`; the application sets organization `EveryGraph` and application name `EveryGraph Workbench`, and falls back to `~/.everygraph/preferences.json` only if the platform returns no application configuration directory. The file is written with `QSaveFile`, so a failed write leaves the previous file intact and surfaces `settings.write_failed` in the status bar.

Schema version 1 has five root keys:

| Key | Values |
| --- | --- |
| `schema_version` | `1`; any other value rejects the file and uses defaults |
| `interface.appearance` | `system`, `light` or `dark` |
| `interface.show_project_panel`, `interface.show_properties_panel`, `interface.show_toolbar` | booleans |
| `interface.window_state` | base64 of the raw `QMainWindow::saveState(1)` layout bytes; empty means the default layout |
| `focus_mode` | `normal`, `data_only` or `commands_only` |
| `function_library.visible` | boolean |
| `defaults.file_format` | `csv`, `parquet` or `everygraph_project` |

A missing file silently yields defaults. An unreadable, invalid-JSON, non-object or unsupported-schema file keeps the documented defaults and emits one diagnostic; an unknown key, wrong JSON type, unknown enum value, non-object group or invalid base64 layout keeps that field's documented default and emits a diagnostic. Diagnostic codes are `settings.invalid_json`, `settings.invalid_root`, `settings.unsupported_schema`, `settings.unknown_key`, `settings.invalid_type`, `settings.invalid_value`, `settings.invalid_window_state`, `settings.read_failed` and `settings.write_failed`; messages go to `qWarning` and, for user-triggered saves, the status bar. A validly encoded layout that `QMainWindow::restoreState` cannot apply is discarded with `settings.invalid_window_state` and the default layout is kept. A failed save leaves the previous file intact, verified by a write to a read-only directory. Loading and applying preferences never calls the command gateway; the test suite runs a real fake-project command, applies dark/commands-only preferences and a focus-mode change, persists, and compares the full object map and undo depth before and after.

### Appearance and focus behavior

`ShellAppearance` applies a palette to the menu bar, status bar, toolbars, docks and menus only. The test checks the dark and light palette on each of those chrome widget classes, including a menu and the project tree inside a themed dock, while the document host, a fake scene widget with scene background `#f7f1e3` and a fake export background `#ffffff` stay unchanged; `system` restores each chrome widget's captured base palette. The shell records the appearance name in the `everygraphAppearance` window property for tests. `Restore Defaults` in the dialog resets every editable field and keeps the current saved layout, because the layout is owned by the window rather than the dialog.

The View menu exposes `Normal Layout` (`Ctrl+Alt+0`), `Data-Only Focus Mode` (`Ctrl+Alt+D`) and `Commands-Only Focus Mode` (`Ctrl+Alt+C`) as an exclusive group, plus dock toggles for the project (`Ctrl+1`), properties (`Ctrl+2`) and function library (`Ctrl+3`). Normal mode honors the three visibility preferences; data-only shows only the project dock and document host; commands-only shows only menus, toolbar and status bar. A focus-mode change is persisted immediately, and dock visibility changes are persisted only in normal mode. `Edit > Preferences...` edits the typed value in a modal dialog; accepting applies appearance, visibility and the default-format label, then persists the whole preference value including the current layout. The composition root also persists preferences on `QCoreApplication::aboutToQuit`, so quitting through the Quit action or the `--smoke-test` timer saves the final layout.

Dock layout uses `QMainWindow::saveState(1)` and `QMainWindow::restoreState(state, 1)`; a state that cannot be restored is discarded with `settings.invalid_window_state` and the default layout is kept. The P1-006 off-screen detached-window restore is unchanged.

### Commands and recorded result

On 2026-10-31, macOS 27.0.1 arm64, AppleClang 21.0.0.21000334, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`, vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` at the pinned baseline and the locally built pkgconf/autotools prefixes on `PATH`:

```sh
export PATH="$PWD/.venv/bin:$PWD/out/pkgconf-install/bin:$PWD/out/autotools-install/bin:$PATH"
export VCPKG_ROOT="$PWD/out/vcpkg"

cmake --preset macos-debug-desktop
cmake --build --preset macos-debug-desktop
ctest --preset macos-debug-desktop

cmake --preset macos-release-desktop
cmake --build --preset macos-release-desktop
ctest --preset macos-release-desktop

cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug

cmake --preset macos-release
cmake --build --preset macos-release
ctest --preset macos-release
```

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug-desktop | Passed; no compiler warnings | 12/12 passed, including `preferences_tests` |
| macos-release-desktop | Passed; no compiler warnings | 12/12 passed |
| macos-debug | Passed; Qt-free regression | 8/8 passed |
| macos-release | Passed; Qt-free regression | 8/8 passed |

Run directly on the offscreen platform, `everygraph_preferences_tests` reports 25 passed and `everygraph_workbench_tests` reports 11 passed, 0 failed in both debug and release. `shell_boundary_check` also covers the new sources: presentation widgets still hold only `CommandQuery` and bindings, and the Qt-free fake project remains free of Qt. `python3 tools/validate_planning.py` passes.

### Limits

- Preferences are shell settings, not project state. There is no project-file setting, dataset or unit default, scene-color editor or export-background setting; the default file format is displayed and stored, but no file is saved or exported.
- The function library is a fixed read-only list of five placeholder entries; no analysis function is implemented or executed.
- The JSON schema has one version; a future schema must define migration or rejection explicitly. There is no settings synchronization, user profile or cloud storage.
- Appearance is verified by palette comparison and fake scene/export values, not a human-visible rendering review, high-contrast audit or platform dark-mode integration. The offscreen platform logs `propagateSizeHints` warnings; they are harness noise, not test failures.
- Focus modes and shortcuts are exercised by Qt Test key events on the offscreen platform at 1024x600; a physical small screen, real window manager, screen-reader tree and full keyboard-navigation audit remain future work.
- Dock resizing and rearrangement are verified with `resizeDocks`/`addDockWidget` and a `saveState`/`restoreState` round trip in one process. The settings file and application restart are verified separately by launching `everygraph_desktop --smoke-test` in two real processes: the first run loads values written by the test and saves its layout on quit, and the second run reloads the changed values.
- macOS arm64 only. No Windows build, CI execution, sanitizer-plus-Qt run, signing, notarization, packaging or release claim is made.

## Accessibility, display scaling and input qualification (P1-008)

P1-008 records what the accepted P1-006/P1-007 shell actually exposes to assistive technology, to a keyboard-only user and to an input-method user. It adds one Qt Test executable that inspects the live `QAccessible` tree, walks the Tab chain, measures both shell palettes with the WCAG relative-luminance formula, drives text entry through a synthetic `QInputMethodEvent` and through the real macOS Pinyin method, checks the layout at 1x and 2x, and restores detached windows over a simulated display topology. Shell sources gain accessible names, descriptions, focus policies and an explicit preferences tab order, and the dark theme's selection colors changed to clear the measured contrast floor. No application or domain contract changed, no dependency was installed, and no conformance, screen-reader or Windows claim is made.

### Targets and registration

| Target / test | Purpose |
| --- | --- |
| `everygraph_accessibility_tests` | New Qt Test executable with eight test methods plus init/cleanup: accessible names/roles/descriptions over the window, menu bar, status bar, toolbar, docks, project tree, properties panel, function library, document host, tab bar, detached window, preferences dialog and its controls; a Tab-chain walk of the shell and of the preferences dialog; light and dark contrast with non-color state cues; synthetic input-method preedit/commit/cancel; 1x/2x layout, icon and clipping checks; simulated single/dual/left-of-primary display topologies; and a macOS-only real Pinyin composition. On Apple platforms the target also compiles `tests/native_pinyin_ime.mm` (`OBJCXX`, C++20) and links `-framework Cocoa -framework Carbon`; elsewhere it compiles only the C++ translation unit. |
| `accessibility_1x`, `accessibility_2x` | Offscreen CTest entries over the same environment as `workbench_tests`, plus `QT_SCALE_FACTOR` and `EVERYGRAPH_EXPECTED_SCALE` set to 1 and 2 and `EVERYGRAPH_SCREENSHOT_DIR` set to `<build>/accessibility-evidence/1x` and `/2x`. Without `EVERYGRAPH_EXPECTED_SCALE` the scaling method skips, so the two entries are the only way that check runs. |
| `accessibility_focus_native`, `accessibility_ime_native` | Apple-only CTest entries on the `cocoa` platform. The first runs `focus_order_cycles_without_trap` and `preferences_focus_order_is_explicit` against a real window server, and since P1-014 it asserts an explicit window-activation precondition before the first Tab and after every Tab instead of trusting a bounded wait. The second runs `native_pinyin_ime_composes_text`, whose helper since P1-015 requires its own text-input context to name the Pinyin source before it types, gates the remaining four letters on the first opening Qt's live marked range, withholds the committing key once the composition has closed, and repeats a composition whose commit the input method disturbed. Both redirect `EVERYGRAPH_PREFERENCES_PATH` to `<build>/accessibility-native-preferences.json`, so the qualification never reads or writes the developer's real settings file, and write captures to `<build>/accessibility-evidence/native`. |
| `modules/presentation_qt` sources | `workbench_window.cpp`, `document_host.cpp`, `properties_panel.cpp`, `function_library_panel.cpp`, `preferences_dialog.cpp` and `shell_appearance.cpp` gain the accessibility metadata, focus policies, declared tab order and AA-meeting palette values described below. `apps/desktop/CMakeLists.txt` registers the new target and the four CTest entries. |

### Accessible names, roles and descriptions

`QAccessible::queryAccessibleInterface` is read for every named widget, and the test asserts the exact `Name`, a non-empty `Description` and a role other than `NoRole`. The window reports `EveryGraph Workbench`, the menu bar `Main menu`, the status bar `Status bar`, the document host `Document area`, its tab widget `Open documents`, the project tree `Project tree` with role `Tree`, the properties panel `Properties panel`, the function library `Function library` with its list `Placeholder analysis functions` at role `List`, and the preferences dialog `EveryGraph preferences` at role `Dialog`. Every toolbar button's name equals its action text and carries the action's tooltip as its description; nine or more buttons are covered. The detached document window is named `Welcome (detached document)`.

Two of Qt's own interfaces do not read the metadata a widget sets, and the first pass of this qualification recorded that before correcting the assertion instead of the framework: `QAccessibleDockWidget` builds the pane name from the visible dock title and overrides `text()` so no description is exposed, the widget factory builds a toolbar as `QAccessibleWidgetV2(widget, ToolBar, widget->windowTitle())` so the toolbar's name comes from its window title rather than its `accessibleName` (its description still comes from `accessibleDescription`), and `QAccessibleTabBar` takes the name of the whole tab list from the current tab's `accessibleTabName` and exposes no description. Setting `accessibleName` on a dock, a toolbar or a tab bar therefore does not change what an assistive technology reads, and the final diff puts the labels where Qt actually looks: the toolbar is created as `addToolBar("Workbench toolbar")`, the detached toolbar as `addToolBar("Detached document toolbar")`, each document tab carries `setAccessibleTabName(index, "<title>, <kind> document")` alongside its tooltip, and the descriptive text for the three panels lives on the panel widgets inside their docks. The base revision set no accessible name on any dock, toolbar or tab bar, so nothing ineffective is left behind or removed. The tab bar interface is asserted at role `PageTabList` with the name `Welcome, note document`, which is Qt's current-tab behavior, not a regression.

### Keyboard focus order

`walk_focus` sends real `Qt::Key_Tab` key clicks and records each focused widget by object name, stopping on a `<focus-lost>`, `<outside-window>` or `<stuck>` marker, so a trap or a leak to another top-level fails rather than looping. It first raises and activates the top level, sets focus on the starting widget once, and then asserts the window-server state through `is_key_window` (visible, `isActiveWindow()`, `QApplication::activeWindow()` equal to the top level and `QApplication::focusWidget()` equal to the start) before sending the first Tab, and re-asserts key status after every Tab; a walk whose window is not key is refused and fails with an `activation_error` that names the widget, the step, the state and the `WindowActivate`/`WindowDeactivate` counts, so the traversal assertions never run against a window that cannot move focus. The earlier bounded wait — 25 ms steps up to one second for the top level to become active, replacing a fixed single 30 ms assumption that made the native entry fail intermittently — is what P1-014 replaced, because macOS grants frontmost status to a newly launched process and does not hand it back to a background one, so waiting on it cannot be a precondition. A walk that still never leaves its starting widget fails outright on every platform and prints whether the window was active and which widget held focus; offscreen does activate these windows, so an earlier "the offscreen platform does not activate windows" skip was removed instead of hiding a regression. The shell cycle is `projectTree -> propertiesPanel -> functionLibraryList -> documentTabBar -> projectTree`, and the preferences dialog cycle is `appearanceCombo -> focusModeCombo -> showFunctionLibraryCheck -> showProjectPanelCheck -> showPropertiesPanelCheck -> showToolbarCheck -> defaultFileFormatCombo` followed by three buttons and back to the appearance combo. Both orders are identical on `offscreen` and on `cocoa`. `PreferencesDialog::build_ui` now declares that order with `setTabOrder` calls instead of relying on construction order, and the document tab bar, project tree, properties panel and function list gained `Qt::StrongFocus` because a `QTabBar` inside a `QTabWidget` and the read-only labels are otherwise unreachable by keyboard.

On macOS Qt gives a non-editable combo box and a dialog button a `Qt::TabFocus` policy, and the platform leaves those widgets out of the chain until the user enables System Settings > Keyboard > Keyboard navigation. That host setting is off, so the dialog walk promotes `TabFocus`-only children to `StrongFocus` for the duration of the test, which emulates the enabled state; the promotion is documented in the test and is not a production change. Nothing in this run verifies a screen reader's announcement order, only the Qt focus chain.

### Contrast and non-color state cues

Both shell palettes are measured with the WCAG relative-luminance formula against a 4.5:1 floor for six foreground/background role pairs. The values the suite now prints are:

| Theme | Window text | Base text | Alternate text | Button text | Selected text | Link text |
| --- | --- | --- | --- | --- | --- | --- |
| light | 14.55:1 | 16.29:1 | 15.21:1 | 12.81:1 | 5.27:1 | 4.71:1 |
| dark | 11.34:1 | 13.36:1 | 12.13:1 | 9.11:1 | 5.27:1 | 6.81:1 |

The previous dark `Highlight` of `#3d7ebd` measured 4.27:1 against white selected text, below the floor, so `ShellAppearance` now uses `#2f6fab` (5.27:1) for selection in both themes and separates `Link` from `Highlight`: dark links are `#8ab8e8` and light links `#2f6fab`. `themed_palette` takes a distinct link color for that reason. Non-color cues are asserted alongside the numbers: a non-empty status bar message, a tab tooltip and tab accessible name that carry the document identity, a checkable focus-mode action with a status tip, and text property values (`object id 4`, type `dataset`) that identify the selection without relying on color.

### Input method composition

`ime_composition_commits_only_after_confirmation` installs an event filter on a `QLineEdit` and sends a `QInputMethodEvent` with a `ni hao` preedit: the edit text must stay empty while the preedit is observed, then a commit event carrying `U+4F60 U+597D` must appear exactly once in the field. A preedit followed by an empty commit must leave the field empty, which is the cancel path. This proves the shell's Qt input-method surface only; no shell widget consumes or transforms composition text.

`native_pinyin_ime_composes_text` is the real path and runs only on `cocoa`. `apps/desktop/tests/native_pinyin_ime.mm` reads the current keyboard input source with Carbon TIS, becomes the key window, activates the widget's `NSTextInputContext`, and posts `CGEventCreateKeyboardEvent` keystrokes as `NSKeyDown` through `[NSTextInputContext handleEvent:]`. It never selects an input source: `TISSelectInputSource` changes the system-wide selection, which this task is not authorized to touch, so when Pinyin - Simplified (`com.apple.inputmethod.SCIM.ITABC`) is not already current the helper reports the source and the check skips with that reason. With the input source already set to Pinyin the run observed the preedit sequence `n`, `ni`, `ni h`, `ni ha`, `ni hao` and committed `U+4F60 U+597D`, which the test compares against the line edit's contents, then captured `native-pinyin-window.bmp`. Which of those two branches a run takes is decided by a host setting no task here changes, and it moved during this qualification: with Pinyin - Simplified current (`AppleCurrentKeyboardLayoutInputSourceID` reporting `com.apple.keylayout.PinyinKeyboard`), every one of the 20 release and 10 debug desktop sweeps run between 20:28 and 20:33 on 2026-11-01 composed and passed, while in an independent re-verification window from 20:46 to 20:47 the same host reported `com.apple.keylayout.ABC` and all ten of those sweeps plus ten isolated runs of the entry skipped — CTest `***Skipped`, preset exit still 0. P1-015 made that path deterministic: the helper waits until its own `NSTextInputContext` names that source before it types anything, requires the first syllable letter to open Qt's live marked range before the remaining four follow, sends the committing key only while a preedit is live and withholds it once the composition has closed, and treats an attempt whose commit the input method disturbed — the committing key's own character reaching the line edit — as failed, clearing the field and repeating the whole composition up to three times. Each run prints one `[native-ime]` diagnostic line carrying the attempt count, the precondition wait, the preedit count, the undelivered-key and artifact-attempt counters, whether the commit key was sent and the commit strings and field text as code points. `accessibility_ime_native` sets `SKIP_REGULAR_EXPRESSION` so CTest reports that entry `Skipped` rather than `Passed` when the composition never ran — Qt Test exits 0 on a skip, which would otherwise look like evidence. The test fails, rather than skips, when Pinyin is current and the composition does not produce the expected text.

### Scaling, clipping and display topology

The 1x and 2x entries assert that the screen reports the expected device pixel ratio, that the window keeps its requested 1200x760 size, that every toolbar button is at least its style size hint and every command still carries a non-null icon, that the first document tab is wider than its text and at least one font line high, that the status-bar default-format label fits its hint, and that all three docks and the document host keep a positive size. `clipped_text_widgets` walks every visible label and button with text and fails if the text needs more room than the widget has: a word-wrapped label is measured with `QFontMetrics::boundingRect` at the widget's own width, a single-line label with its advance and font height, and a button with its style `sizeHint()`. (`QWidget::heightForWidth` was the first attempt and is not the measurement: the base implementation returns -1 unless the widget owns a height-for-width layout (`qwidget.cpp:10419-10424`), which no `QCheckBox` does, so comparing that against the height could never flag a button, and `QLabel` answers it only for its own preferred text layout (`qlabel.cpp:628-634`), not for the room the widget ended up with.) Re-running the preferences check with the two `reserve_wrapped_height` calls compiled out shows what the measurement now catches: on `cocoa` both word-wrapped notes sit in a 25-pixel-high box while their wrapped text needs 32 (`<QLabel> (370x25 needs 370x32)`) and the four interface checkboxes are 16 high against a 20-pixel hint, and on `offscreen` the notes do get their wrapped height while the four checkboxes are squeezed to 12 and 13 against the same 20-pixel hint. `QFormLayout` therefore left the second line of each note clipped on the platform users actually run on, and `reserve_wrapped_height` now gives each note the height its wrapped text needs plus the minimum width that keeps the measured width and the assumed width consistent. Captures land in `accessibility-evidence/1x` and `/2x`, five files in each: `workbench-1x.bmp` and `workbench-2x.bmp` respectively, plus `workbench-dark.bmp`, `workbench-light.bmp`, `focus-on-document-tabs-offscreen.bmp` and `focus-on-default-format-offscreen.bmp` under the same name in both. Each 2x file carries exactly four times the pixel data of its 1x counterpart because the same window is captured at twice the linear size at the same 24-bit depth: the 54-byte BMP header stays constant, so the totals are `workbench-1x.bmp` 2 736 054 = 2 736 000 + 54 and `workbench-2x.bmp` 10 944 054 = 4 x 2 736 000 + 54, which is four times the 1x total minus the 162 bytes of header a naive multiplication would count four times. That exact 4x relationship of the pixel bytes is the recorded evidence that the device ratio changed. Icon *fidelity* at 2x is not verified: these are style-provided icons and the suite only checks that a non-null icon and a non-clipped button exist.

`display_topology_restores_detached_windows` uses the P1-006 `DocumentHost::restore_detached_windows` and `WindowPlacement` helpers over three simulated topologies: a single primary display at `0,0 1920x1080`, a dual setup with a second display to the right, and a second display to the left of the primary. A detached window inside the second display keeps its geometry when that display still exists, moves back onto the primary when the topology shrinks to one display while keeping its size, and stays put on a left-of-primary display at negative coordinates. A window placed at `5000,4000` — off every screen — is recovered onto the primary. The offscreen QPA adds a frame extent of its own, so the containment checks compare `frameGeometry()` rather than an exact position. This host has one physical display, so no real second panel is involved.

### Commands and recorded result

On 2026-11-01, macOS 27.0.1 arm64 (Apple M5), AppleClang 21.0.0, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`, `VCPKG_ROOT=$PWD/out/vcpkg`, the pinned Qt 6.12.0 graph at `out/vcpkg-installed/arm64-osx-dynamic`:

```sh
export PATH="$PWD/.venv/bin:$PWD/out/pkgconf-install/bin:$PWD/out/autotools-install/bin:$PATH"
export VCPKG_ROOT="$PWD/out/vcpkg"

cmake --preset macos-debug-desktop
cmake --build --preset macos-debug-desktop
ctest --preset macos-debug-desktop

cmake --preset macos-release-desktop
cmake --build --preset macos-release-desktop
ctest --preset macos-release-desktop

cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug

cmake --preset macos-sanitize
cmake --build --preset macos-sanitize
ctest --preset macos-sanitize

python3 tools/validate_planning.py
python3 -B tools/validate_task_manifest.py
```

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug-desktop | Passed; no compiler warnings | 16 passed, 0 failed, 0 skipped of 16 in 9 of the 10 sequential debug sweeps run between 20:28 and 20:33 on 2026-11-01 with Pinyin - Simplified as the host input source, and 10 of 10 isolated `accessibility_focus_native` runs on this preset; the fifth sweep failed only on entry 8 `worker_protocol_tests` (SIGPIPE), which is P1-005's accepted entry, is outside P1-014's manifest, and is reported to its owning lane in the [P1-014 record](../tasks/p1-014-native-focus-order-determinism.md) rather than fixed here, with all four accessibility entries green in that same sweep. A separate independent re-verification window, 20:46 to 20:47 on the same day with the host source on `com.apple.keylayout.ABC`, ran 4 further sweeps each reporting 15 passed, 1 skipped of 16 where the skip is `accessibility_ime_native` and `accessibility_focus_native` passed all four. Earlier in this section's history the entry reported 15 passed and 1 skipped of 16 for the same reason |
| macos-release-desktop | Passed; no compiler warnings | 16 passed, 0 failed, 0 skipped of 16, green first time on 20 of 20 sequential sweeps run between 20:28 and 20:33 on 2026-11-01 with Pinyin current and no sweep re-run; in P1-014's earlier round 24 of its 25 release sweeps were green first time and the exception was this very entry, in the sweep that failed before P1-015's helper fix. In this round `accessibility_focus_native` passed all 20 release sweeps and 40 of 40 isolated runs, and `accessibility_ime_native` passed all 20 release sweeps and 25 of 25 isolated runs, each isolated run logging `attempts: 1` and `artifact-attempts: 0`. The 20:46 to 20:47 re-verification window ran 6 further release sweeps with the host source on `com.apple.keylayout.ABC`, each 15 passed and 1 skipped of 16 — the skip being that entry — with `accessibility_focus_native` passing all six and 15 more isolated focus runs green |
| macos-debug | Passed; Qt-free regression | 8/8 passed |
| macos-sanitize | Passed; Qt-free regression | 8/8 passed |

`python3 tools/validate_planning.py` reports `PASS: 339 original IDs; 428 traced requirements; 96 documents` with exit 0, including the English-policy check. Run directly on `offscreen` with `QT_SCALE_FACTOR` and `EVERYGRAPH_EXPECTED_SCALE` set to 1, then to 2, `everygraph_accessibility_tests` reports 9 passed, 0 failed, 1 skipped in both runs; the skip is always `native_pinyin_ime_composes_text`, which requires `cocoa`. On `cocoa`, the focus pair reports 4 passed, 0 failed. The Pinyin method reports a skip with the reason "Pinyin - Simplified is enabled but is not the current input source" only on a host whose selection is not Pinyin. Both states occurred on this host on 2026-11-01, so that skip is a live branch rather than history: the composition evidence in this section is from runs with Pinyin selected, and the runs with `com.apple.keylayout.ABC` selected skipped. `python3 -B tools/validate_task_manifest.py` and its delta against this task's baseline are recorded in the P1-008 task record's Validation table. No finding reports a missing or malformed P1-008 manifest or a commit outside the declared set. `shell_boundary_check` still passes: the new metadata is Qt-side only and the Qt-free fake project is unchanged.

### Limits

- This is a qualification of the prototype shell, not a conformance claim. No screen reader (VoiceOver or other) was run against the tree, no WCAG audit of a complete product was performed, no platform accessibility API bridge was verified end to end, and the contrast figures are computed from `ShellAppearance` palette roles rather than from every rendered widget's effective colors.
- Focus order is the Qt focus chain exercised by synthetic Tab key events. The macOS run emulates the platform's keyboard-navigation setting by promoting `TabFocus`-only dialog controls for the duration of the test; the host setting itself was not changed, so the native chain with that setting off is not the chain recorded here. The native entry depends on the real window server granting activation, and P1-014 replaced the bounded wait with an asserted precondition: a window that is not key is never walked, and the run fails outright naming the widget, the step, the visible/key/active/focused state and the `WindowActivate`/`WindowDeactivate` counts. That makes every occurrence attributable instead of producing a truncated order, and no walk is retried, because a retry could not be told apart from hiding a real focus loss. It does not eliminate the sensitivity: one release sweep of P1-015's ten lost key status during the first Tab and failed exactly that way while the same process's preferences walk passed, so a transient host activation loss during the walk stays a disclosed limit rather than a claim closed; in P1-014's 30 sweeps and 50 isolated runs on 2026-11-01 the entry passed every time.
- The real IME check exercises only Pinyin - Simplified, and only in a plain `QLineEdit` fixture, and only when that source is already the user's current selection: the helper reads the source and never selects one, so on a host whose current source is `com.apple.keylayout.ABC` the native entry reports `Skipped`. P1-015's containment repeats a composition whose commit the input method disturbed, and no assertion reads that attempt count, so a run that recovers on a later attempt passes and only the `[native-ime]` diagnostic line shows it took more than one. No other input method, no right-to-left script, no handwriting or voice input, no composition inside a dock widget, table cell or the document host, and no commit through a physical keyboard is covered.
- Scaling is verified through `QT_SCALE_FACTOR` on the offscreen platform at 1x and 2x through the two dedicated CTest entries; the cocoa entries do not set `EVERYGRAPH_EXPECTED_SCALE`, so the scaling method skips there even though its window server, Tab chain and Retina captures are real. There is no physical 1x display, no fractional scale, no per-monitor DPI change and no runtime display reconfiguration. The evidence images are BMP because the qualified Qt build ships no PNG image-format plugin, and the suite only checks that each command still has a non-null icon, not that its bitmap carries 2x detail.
- The display topology is simulated by passing `QRect` sets to `restore_detached_windows` and `WindowPlacement`. This host has one physical display, so no real hot-plug, no display arrangement change and no window-manager behavior was observed; the left-of-primary case compares frame geometry because the offscreen platform invents its own frame extents.
- The project tree is still read-only and the properties panel still shows plain labels, so item-level `QAccessible` children, cell navigation, live region announcements and table semantics are out of scope until those widgets carry real data.
- macOS arm64 only. No Windows UIA or Magnifier check, no CI execution, no signing, packaging or release claim is made.

## Windows configuration only: not executed

The Windows-only configure presets select Ninja and `cl`; `architecture: { value: x64, strategy: external }` requires an already initialized x64 compiler environment. It does not cause Ninja to select an architecture. Use Visual Studio 2022 with the C++ desktop toolchain and Windows SDK, and its x64 Native Tools developer command prompt. Ensure CMake 3.31.6, the Ninja 1.11.1.4 pip distribution, and Python >= 3.10 are available there.

The following are configured instructions, not locally verified Windows commands or results. Run them from the Windows checkout root:

```bat
where cl
cmake --version
ninja --version
python --version
python -m pip show cmake ninja
cmake --preset windows-debug
cmake --build --preset windows-debug --verbose
ctest --preset windows-debug
cmake --preset windows-release
cmake --build --preset windows-release --verbose
ctest --preset windows-release
```

The workflow uses `vswhere` with the VS 2022 version range `[17.0,18.0)` and the x86/x64 C++ tools component to locate the installation, then calls `VsDevCmd.bat -arch=x64 -host_arch=x64` inside the same `cmd` step as configure/build/test. It checks the target architecture and availability of `cl`, preserving the initialized environment and propagating command failures. There is no Windows ASan/UBSan preset or Windows validation claim from this macOS host.

## Build topology and per-module targets (P1-011)

Before P1-011 every target, flag loop and `add_test` call lived in the root `CMakeLists.txt`. That made the root file a shared write target for otherwise independent tasks, which conflicts with the exclusive-ownership rule in AGENTS.md section 0. The topology is now split so that a task adds targets only inside the tree it owns.

| File | Owns |
| --- | --- |
| `CMakeLists.txt` (root) | `cmake_minimum_required`, `project`, the three shared `option()` declarations, the sanitizer/TSan conflict guard, `find_package(Threads)`, `find_package(Python3)`, `include(CTest)`, the top-level output directories, the two shared policy functions, the warning and sanitizer policy loops, and one `add_subdirectory` line per top-level tree |
| `modules/foundation/CMakeLists.txt` | `everygraph_foundation` |
| `modules/application/CMakeLists.txt` | `everygraph_application` |
| `tools/CMakeLists.txt` | `everygraph_toolchain_probe`, `everygraph_qt_probe` and their `toolchain_probe` / `qt_probe_smoke` tests, plus from P0-011 the `catalog_validation` / `catalog_validator_tests` registrations for the validator scripts that live in this tree, all behind `BUILD_TESTING` |
| `tests/CMakeLists.txt` | the `add_subdirectory` line per test group, then the repository-wide checks that compile nothing: `planning_validation`, `planning_validator_tests`, `module_boundary_check` |
| `tests/application/CMakeLists.txt` | `everygraph_command_kernel_tests` and its `command_kernel_tests` test |

Rules a task must follow when it adds build work:

- **Define a target in the directory that owns its sources.** Source paths in a target are relative to that directory, so `modules/foundation/CMakeLists.txt` lists `src/error.cpp` and resolves to the same file the root file previously named as `modules/foundation/src/error.cpp`.
- **Guard `add_test`, never a target definition.** The single-file topology created every test executable whether or not `BUILD_TESTING` was on and registered its test only when on. `tests/CMakeLists.txt` therefore adds its groups before its `BUILD_TESTING` early return, and `tests/application/CMakeLists.txt` and `tools/CMakeLists.txt` wrap only their `add_test` calls. A `-DBUILD_TESTING=OFF` configuration builds the same four targets before and after the refactor, and registers zero tests in both.
- **Register every build target that must carry the shared policy** by calling `everygraph_register_code_target(<target>)`. The warning set and the sanitizer flags are applied in the root file's two loops over the `EVERYGRAPH_CODE_TARGETS` global property, in that order per target. A global property is used because a subdirectory cannot append to a parent-scope variable. A target that is not registered silently escapes both policies.
- **Call `everygraph_assert_no_qt_linkage(<target>)` in every GUI-independent module** after that target's `target_link_libraries` call. The function is defined in the root file as shared policy; each module applies it to its own targets because only the module knows them. `module_boundary_check` remains the second, independent guard and scans the module trees for Qt includes and `Q`-prefixed type tokens.
- **Pin `WORKING_DIRECTORY` on any test whose registration moves.** A test's default working directory is the binary directory of the CMake file that registers it, so relocating an `add_test` call silently relocates the test's working directory even when its command is identical. `toolchain_probe`, `qt_probe_smoke`, `command_kernel_tests` and `module_boundary_check` now state `WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"`, which is the value each had while every target was defined in the root file. `planning_validation` and `planning_validator_tests` already pinned the source root explicitly and needed no change.
- **Keep flags, sanitizer behavior and warnings in the root loops.** A module file must not add `-W`, `-fsanitize` or `/W` options locally; per-target compile features, include directories and link libraries are the module's own business.
- **Adding a new top-level tree costs exactly one `add_subdirectory` line in the root file.** Everything else about it — targets, flags registration, tests — stays local to that tree. `modules/data`, `modules/plot` and the other planned directories are created only when their work starts, each with its own `CMakeLists.txt`.

The root file also pins `CMAKE_RUNTIME_OUTPUT_DIRECTORY`, `CMAKE_LIBRARY_OUTPUT_DIRECTORY` and `CMAKE_ARCHIVE_OUTPUT_DIRECTORY` to `${CMAKE_BINARY_DIR}`. Targets defined in a subdirectory would otherwise emit into `out/build/<preset>/tools/` and `out/build/<preset>/tests/application/`, which would break the recorded `macdeployqt`, `otool`, `vtool` and SBOM-generator command paths in this guide and the P1-012 bundle work that reads them. The assignment must precede `add_subdirectory(tools)`: Qt's `qt_standard_project_setup()` sets `CMAKE_RUNTIME_OUTPUT_DIRECTORY` to the defining directory on Windows when it is still unset, so pinning it first keeps the Windows route consistent with the macOS layout actually recorded here.

`EVERYGRAPH_ENABLE_QT_PROBE` is now declared as `option(... OFF)` in the root file. It was previously read while undeclared, so the Qt presets passed only because they set the cache variable; any other configuration left the option undefined rather than explicitly off. `macos-debug-qt`, `macos-release-qt` and the two `*-qt-vcpkg` presets still turn it on through their `cacheVariables`, and the three Qt-free presets continue to configure without locating Qt.

`CMakePresets.json` is unchanged by the refactor. Preset names, build and test presets and the Windows presets are the same set as before.

Because the target definitions moved, the order in which CMake registers tests changed with them, so `ctest -N` now lists `command_kernel_tests` and `module_boundary_check` in different positions. The test name set, each test's command, working directory, timeout and environment are unchanged — `ctest --show-only=json-v1` over the pre-refactor and refactored topologies reports the same five and six tests with zero differing properties — and the presets run the unfiltered suite, so no selection depends on the order.

Refactor result on 2026-10-31, macOS 27.0.1 arm64, AppleClang 21.0.0.21000334, CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`, Homebrew `qtbase` 6.12.0 for the two Qt presets and the retained `out/vcpkg` graph at baseline `d186940b` for the two vcpkg presets:

| Preset | Configure / build | CTest |
| --- | --- | --- |
| macos-debug | Passed, no compiler warnings | 5/5 passed |
| macos-release | Passed, no compiler warnings | 5/5 passed |
| macos-sanitize | Passed with ASan + UBSan | 5/5 passed, no sanitizer diagnostics |
| macos-debug-qt | Passed, no compiler warnings | 6/6 passed |
| macos-release-qt | Passed, no compiler warnings | 6/6 passed |
| macos-debug-qt-vcpkg | Passed, no compiler warnings | 6/6 passed |
| macos-release-qt-vcpkg | Passed, no compiler warnings | 6/6 passed |

The five AC-4 presets were also configured and built from wiped scratch directories, so their result is a clean-configure proof rather than an incremental reconfigure. The two vcpkg presets were re-configured only in place: a scratch build directory would force a full Qt 6.12.0 source rebuild instead of reusing the retained `out/vcpkg` install graph, which is outside this task. `Windows-debug` and `windows-release` remain unexecuted on this host. Against the pre-refactor captures, `compile_commands.json` matches entry for entry — same source files, same include directories, same defines and the same warning and sanitizer flags — after normalizing only the two build-tree locations that follow the defining directory (the per-target object directory, and Qt's `everygraph_qt_probe_autogen` directory, which is now under `tools/`). Release binaries, including `everygraph_qt_probe.app/Contents/MacOS/everygraph_qt_probe`, are byte-identical. Debug binaries differ only in `LC_UUID` and in the single `N_OSO` debug string that records the object-file path; the `__text` code section and the symbol count are identical.

## macOS bundle, ad-hoc hardened runtime and LGPL substitution (P1-012)

Executed on 2026-11-01 on the same Apple Silicon host and toolchain as the rows above; [P1-012](../tasks/p1-012-macos-bundle-hardened-runtime-lgpl.md) holds the per-criterion acceptance evidence.

### The packaging route

`packaging/macos/` is the whole macOS packaging route, and it is deliberately script-only: `CMakeLists.txt` and `CMakePresets.json` carry no install or bundle rule for this, because `macdeployqt` exists only as an `arm64-osx` host tool and therefore cannot deploy the `arm64-osx-dynamic` target graph this project ships.

| Script | What it does |
|---|---|
| `macho_helpers.sh` | Shared Mach-O reading: magic-byte file detection, `LC_LOAD_DYLIB` names, install names, rpath enumeration and expansion, and a bounded `env -i` launch with a throwaway `HOME`, `TMPDIR` and `EVERYGRAPH_PREFERENCES_PATH` |
| `build_bundle.sh --preset <preset> [--out <dir>] [--entitlements <plist>] [--sign-mode hardened\|adhoc\|none] [--vcpkg-installed <dir>]` | Restages the CMake `.app`, walks the Mach-O closure out of the pinned install root, rewrites every reference to `@rpath/<file>`, deploys `platforms/libqcocoa`, `platforms/libqoffscreen` and `styles/libqmacstyle`, normalizes rpaths and library ids, writes notices and the inventory, then signs |
| `bundle_inventory.py --app <app> --seen <closure> --vcpkg-installed <dir> --sbom <spdx.json> --repo-root <dir> --output <json> [--notices]` | Maps every bundled Mach-O to its pinned vcpkg package, version and license, checks each version against the accepted P1-003 SPDX document, scans for private-path leakage, and writes `bundle-inventory.json` plus `Contents/Resources/licenses/` |
| `sign_bundle.sh --app <app> --entitlements <plist> --hardened\|--adhoc` | Inside-out local ad-hoc signing of every nested Mach-O, then the bundle; the hardened runtime is applied only with `--hardened` |
| `verify_bundle.sh --app <app> [--vcpkg-installed <dir>] [--expect-hardened\|--expect-adhoc-only] [--no-content-scan] [--no-launch]` | Per-library dependency resolution, absolute-rpath and install-name checks, a load-command parse check, a generic private-path content scan, `codesign --verify --deep --strict`, signature description checks, a hermetic launch outside the build tree, and a post-signature inventory re-check. It refuses to run on a host missing a Mach-O tool and on a bundle whose executable is not code, because both would empty its iteration and turn "every image passed" into a vacuous statement |
| `lgpl_replace_test.sh --app <app>` | The replacement matrix: three signing states crossed with three library states, each launch bounded, plus the `DYLD_PRINT_LIBRARIES` load traces |
| `rebuild_from_source.sh [--revision <rev>] [--export-dir <dir>] [--preset <preset>] [--allow-missing-qt-archive]` | The recipient route: export the committed source, verify the Qt source archive against the recorded SHA-512 (a missing archive stops the run unless the caller asks to skip that check), configure, build, test, bundle, verify, assert byte-identity with the reference bundle, then substitute the LGPL library and relaunch |
| `entitlements.plist`, `strict-library-validation.entitlements.plist` | The shipped entitlement set (library validation disabled) and the empty comparison variant |

### Commands and recorded result

```sh
export PATH="$PWD/.venv/bin:$PWD/out/pkgconf-install/bin:$PWD/out/autotools-install/bin:$PATH"
export VCPKG_ROOT="$PWD/out/vcpkg"

cmake --preset macos-release-desktop
cmake --build --preset macos-release-desktop
packaging/macos/build_bundle.sh --preset macos-release-desktop
packaging/macos/verify_bundle.sh --app out/bundle/macos-release-desktop/everygraph_desktop.app \
    --vcpkg-installed out/vcpkg-installed
packaging/macos/lgpl_replace_test.sh --app out/bundle/macos-release-desktop/everygraph_desktop.app
packaging/macos/rebuild_from_source.sh --revision HEAD
```

`build_bundle.sh` produces `out/bundle/macos-release-desktop/everygraph_desktop.app`: 30 files under `Contents`, 17 of them Mach-O (the executable, 13 dylibs in `Contents/Frameworks`, 3 plugins in `Contents/PlugIns`), 26 MB. `verify_bundle.sh` reports **72 passed, 0 failed, 1 warning**; the warning is two `__FILE__` build-path strings inside the vendor `libQt6Widgets.6.12.0.dylib`, which are build material rather than linkage. `codesign --verify --deep --strict` exits 0 on a bundle reporting `flags=0x10002(adhoc,runtime)`, `Signature=adhoc`, `TeamIdentifier=not set`, `Runtime Version=27.0.0` and one entitlement key, with `LSMinimumSystemVersion => 14.4`. A second independent `build_bundle.sh --out out/bundle-repro` is byte-identical across all 30 files, and `rebuild_from_source.sh --revision HEAD` from a fresh `git archive` export configures, builds, passes 16/16 CTest entries, emits a bundle its own run asserts is byte-identical to the reference one, and then launches successfully with `libQt6Core.6.12.0.dylib` replaced in place — one added rpath, local re-sign, no recompile and no relink of the application.

Of 151 Mach-O linkage entries in the shipped bundle (load-command dependency names plus rpath values), none resolves outside the bundle or a system path, and none mentions Homebrew, `/usr/local`, `vcpkg` or `/out/build`. `DYLD_PRINT_LIBRARIES` on an ad-hoc copy traces 13 loads, every one from `Contents/Frameworks`; the hardened copy traces none because the hardened runtime strips that variable, which is why the hardened case is proven by a successful launch and the dependency-resolution checks instead.

### Hardened runtime, library validation and why the entitlement is there

The one platform finding worth reading before reusing these scripts: with `--options runtime` and **no** entitlements, dyld refuses to load this project's own Qt libraries even untouched — exit 134 with `code signature in <...> 'Contents/Frameworks/libQt6Widgets.6.12.0.dylib' not valid for use in process: mapping process and mapped file (non-platform) have different Team IDs`. Locally ad-hoc-signed nested libraries and an ad-hoc-signed main executable present as different (unset) team identities, so library validation rejects the mapping before any replacement is involved. `entitlements.plist` therefore declares `com.apple.security.cs.disable-library-validation`, and the empty `strict-library-validation.entitlements.plist` exists so that the comparison is measured rather than assumed.

With that entitlement the substitution behaves as LGPLv3 §4's shared-library route needs: baseline, replaced-and-unsealed and replaced-then-re-signed all launch (exit 0), with `codesign` reporting the seal as valid, broken and valid respectively — the expected consequence of re-signing locally. Nothing here is evidence about Developer ID, notarization, stapling, Gatekeeper or a clean machine, and none of those results is claimed: under same-team Developer ID signing the nested libraries would share a team identity and library validation might be kept enabled, a route this project does not use and did not test.

### Limits

The seal is local and ad hoc: it proves the bundle's internal consistency, not Apple trust, and distribution outside the building machine is untested. `packaging/macos/` first enters the repository with this task's acceptance commit, so `rebuild_from_source.sh` still prints that the exported revision lacks it and uses the working copy of the scripts; from the next revision onward the recipient route is self-contained. The recipient run reuses the pinned install root after verifying the Qt archive hash rather than compiling Qt from a cold cache. `bundle-inventory.json` and the `.app` are generated output under the ignored `/out/` tree — the durable correspondence evidence lives in the P1-012 record and here. The `verify_bundle.sh` private-path scan was itself corrected during this task: it originally matched only the repository root of the tree running it, which silently passed a bundle containing two `/Users/...` strings; it now matches generic patterns so the result does not depend on where the verifier is invoked from. Three further limits of the earlier scripts became part of the tool contract after the independent review: `verify_bundle.sh` and `sign_bundle.sh` now stop when a Mach-O tool is unavailable or when the bundle executable is not code, because an empty image list would otherwise make every per-image check pass without reading a binary; `rebuild_from_source.sh` treats a missing Qt source archive and a recipient bundle that is not byte-identical to the reference as failures rather than notes; and `verify_bundle.sh` canonicalises `--app`, so a bundle reached through a symlinked or temporary path reports the same result as the same bundle reached directly.

## Build and code standards

- CXX is the only enabled CMake language. Use target-level C++20 features, disable compiler extensions, and keep compile/link options target-local. `Threads::Threads` is a platform toolchain facility, not a downloaded dependency.
- MSVC uses `/W4 /permissive- /Zc:__cplusplus`; AppleClang/Clang/GCC use `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow`. GCC flag handling is not a claim of tested Linux support.
- Sanitizers are opt-in. `EVERYGRAPH_ENABLE_SANITIZERS=ON` selects ASan + UBSan. `EVERYGRAPH_ENABLE_TSAN=ON` is an alternative for a separate build directory and a qualified supporting toolchain. Enabling both is a configuration error. The MSVC toolchain is rejected for these configured sanitizer combinations.
- `.clang-format` defines LLVM-based C++20 formatting, four-space indentation, a 100-column limit, no tabs, and explicit pointer alignment. `.clang-tidy` selects analyzer, bug-prone, performance, and limited modernization checks. Neither tool has been installed, version-qualified, or run here; CI does not pretend to enforce these unprovisioned checks.
- When separately provisioned and qualified, use `clang-format --dry-run --Werror tools/toolchain_probe.cpp` and `clang-tidy -p out/build/macos-debug tools/toolchain_probe.cpp` from the source root. These commands are future checks, not recorded passes.
- Use English identifiers, comments, and docstrings. Prefer self-explanatory code; add comments only when they explain a non-obvious invariant. Keep borrowed views within an owner's lifetime and join workers before consuming their results.
- Generated output stays under `out/`; local build directories, user presets, Python caches, editor state, and OS metadata are ignored. No generated artifacts should be treated as source.

## CI configuration only: not executed

`.github/workflows/phase0.yml` has macOS Debug/Release/ASan+UBSan and Windows Debug/Release matrices. Each job installs exactly `cmake==3.31.6` and `ninja==1.11.1.4` with pip under Python 3.13, prints versions, and runs configure/build/full CTest. The Python patch version is selected by setup-python, not locked.

The macOS job requests **`macos-26-xlarge`**, the arm64 larger-runner label in the macOS 26 family, rather than assuming that a generic label guarantees Apple Silicon; the macOS 14 image family it requested until 2026-11-01, including `macos-14-xlarge`, entered deprecation on 2026-07-06 and GitHub's notice puts the macOS 14 images fully unsupported from 2026-11-02 ([actions/runner-images#13518](https://github.com/actions/runner-images/issues/13518)), while Microsoft's Azure DevOps schedule moves the `macos-14` hosted image to 2026-11-23. The chosen image's default Xcode 26.6 reports AppleClang 21.0.0, the same compiler version the recorded local runs in this guide use, but a runner is not the qualified host: its macOS 26.6.2 OS and 26.x SDK differ from the local macOS 27.0.1 / SDK 27.0 environment. This requires suitable larger-runner access, billing eligibility, and current label availability for the eventual repository. The job explicitly checks `uname -m` is `arm64`. The Windows job requests **`windows-2022`**, Python x64, and the VS 2022 x64 environment. Runner provisioning, images, compiler versions, architecture, and all CI results still need evidence from an actual run; hosted images are not frozen by the OS label.

Actions are pinned to the supplied verified commits: checkout v4 at `11d5960a326750d5838078e36cf38b85af677262` and setup-python v5 at `a26af69be951a213d495a4c3e4e4022e16d87065`. Permissions are restricted to `contents: read`; checkout does not persist credentials. The workflow contains no explicit secrets, write permissions, artifact uploads, publishing, signing, or app packaging. Matrix values interpolated into commands are fixed in the workflow, not event-controlled data.

No repository/remote was initialized and no workflow was dispatched. This is CI configuration, not CI success evidence.

## Future dependencies and release work: selected direction, not installed or locked

The planned application baseline is **C++20 with Qt 6.12.0, dynamically linked LGPL-eligible Qt Core/Gui/Widgets**. This is a future dependency selection, not a Qt installation, verified application build, or blanket license grant for every Qt module. Before packaging, audit selected modules and transitive features, preserve notices and source/offer obligations as applicable, and verify replacement/relinking rights and signing interactions. See the separate [GUI and license evidence](../research/gui-and-license-evidence.md) and [technology evaluation](../planning/technology-evaluation.md).

Planned library directions include Arrow C++ arrays/IPC and Parquet interchange, Eigen, SQLite, approved Ceres least-squares functionality, pocketfft, and selected Boost.Math. Python scientific workers may use NumPy/SciPy and other qualified packages. None was installed or linked by this scaffold; compatible releases, enabled features, ABI/CRT combinations, numerical behavior, licenses, and transitive closure still require real selection and tests. Qt Charts/Graphs/DataVisualization, Qt WebEngine, KDE Frameworks, and FFTW are not baseline dependencies.

Prefer **vcpkg manifest mode with a tested pinned registry baseline**, explicit features, and validated arm64 macOS/x64 Windows triplets. A baseline pins registry resolution; compiler/SDK versions, feature closure, and binary provenance must also be recorded. **Conan 2 is an alternative**, using qualified recipes, profiles, and a tested lockfile if that route is deliberately selected; do not mix two managers for the same native dependency graph. No vcpkg manifest/baseline or Conan lockfile should be fabricated before package selection and both-platform builds establish what actually works.

Future unit tests should use GoogleTest for native core code and Qt Test for Qt adapters when those targets exist. Scientific worker tests should use pytest with deterministic fixtures and numerical tolerances. GoogleTest and Qt Test are not present in this build. P1-004 deliberately uses a small dependency-free CTest executable with explicit return-code checks as a temporary harness and records the migration point; it is not a GoogleTest pass count.

macOS application bundling, dependency deployment, local ad-hoc code signing and the hardened runtime are no longer plans: P1-012 delivers them, documented above. Notarization, stapling and clean-machine Gatekeeper verification are not part of this project's route at all — the user decided on 2026-10-31 that the project is open source, runs on the machine that builds it, and signs only ad hoc, so P1-012's canceled predecessor P1-009 left no credential work behind. No Developer ID identity, notarization submission, installer or publicly trusted artifact exists or is claimed. Windows packaging, DLL deployment, Authenticode signing and clean-machine verification remain plans only, unexecuted and unauthorized on this host.
