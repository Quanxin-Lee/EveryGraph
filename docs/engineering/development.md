# Phase 0 development and toolchain standards

## Scope and evidence boundary

This scaffold builds `everygraph_toolchain_probe`, a dependency-free C++20 standard-library executable. P1-004 adds the dependency-free `everygraph_foundation` and `everygraph_application` static libraries and the `everygraph_command_kernel_tests` executable; they implement a command, revision, transaction, undo/redo and event contract over a fake in-memory document, not a product domain model, persistence layer, GUI or wire protocol. An opt-in `everygraph_qt_probe` target adds a minimal Qt Widgets workbench qualification probe; it is not EveryGraph, a GUI prototype, a scientific implementation, or a benchmark. No dependency-manager bootstrap, package lockfile, or business module is part of this work.

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

These are the historical P1-001 counts. After P1-004 added the kernel CTest entries, the Qt-preset total is 6/6 and the Qt-free total is 5/5. The consolidated `main` tree was rerun green on 2026-10-31 after the research appendix and the P0-006 record were committed; see the kernel section below for the current results.

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
| macos-debug | Passed; no compiler warnings | 5/5 passed, including `planning_validation` |
| macos-release | Passed; no compiler warnings | 5/5 passed, including `planning_validation` |
| macos-sanitize | Passed with ASan + UBSan | 5/5 passed, including `planning_validation`; no sanitizer diagnostics |

The Qt presets were also rerun because P1-004 changed the shared CMake target list: `macos-debug-qt` and `macos-release-qt` each reported 6/6, with the kernel tests and `qt_probe_smoke` passing. `everygraph_command_kernel_tests` does not link Qt in any preset. `otool -L` on the Debug test executable lists only `/usr/lib/libc++.1.dylib` and `/usr/lib/libSystem.B.dylib`; `file` reports a Mach-O arm64 executable.

Consolidation rerun, 2026-10-31: the P1-004 checkpoint originally recorded 4/5 for the Qt-free presets and 5/6 for the Qt presets because `planning_validation` failed on a missing research appendix and P0-006 record. Those files are now committed on `main`, and the P1-001, P1-003 and P1-004 branches were integrated. The five presets were rerun with the exact commands above and the results in the table; the earlier failures were external documentation failures, not kernel failures. The vcpkg presets remain unverified because the `arm64-osx-dynamic` graph did not finish; see the P1-003 record.

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

The macOS job requests **`macos-14-xlarge`**, the ARM64 larger-runner label in the macOS 14 family, rather than assuming that the generic `macos-14` label guarantees Apple Silicon. This requires suitable larger-runner access, billing eligibility, and current label availability for the eventual repository. The job explicitly checks `uname -m` is `arm64`. The Windows job requests **`windows-2022`**, Python x64, and the VS 2022 x64 environment. Runner provisioning, images, compiler versions, architecture, and all CI results still need evidence from an actual run; hosted images are not frozen by the OS label.

Actions are pinned to the supplied verified commits: checkout v4 at `11d5960a326750d5838078e36cf38b85af677262` and setup-python v5 at `a26af69be951a213d495a4c3e4e4022e16d87065`. Permissions are restricted to `contents: read`; checkout does not persist credentials. The workflow contains no explicit secrets, write permissions, artifact uploads, publishing, signing, or app packaging. Matrix values interpolated into commands are fixed in the workflow, not event-controlled data.

No repository/remote was initialized and no workflow was dispatched. This is CI configuration, not CI success evidence.

## Future dependencies and release work: selected direction, not installed or locked

The planned application baseline is **C++20 with Qt 6.12.0, dynamically linked LGPL-eligible Qt Core/Gui/Widgets**. This is a future dependency selection, not a Qt installation, verified application build, or blanket license grant for every Qt module. Before packaging, audit selected modules and transitive features, preserve notices and source/offer obligations as applicable, and verify replacement/relinking rights and signing interactions. See the separate [GUI and license evidence](../research/gui-and-license-evidence.md) and [technology evaluation](../planning/technology-evaluation.md).

Planned library directions include Arrow C++ arrays/IPC and Parquet interchange, Eigen, SQLite, approved Ceres least-squares functionality, pocketfft, and selected Boost.Math. Python scientific workers may use NumPy/SciPy and other qualified packages. None was installed or linked by this scaffold; compatible releases, enabled features, ABI/CRT combinations, numerical behavior, licenses, and transitive closure still require real selection and tests. Qt Charts/Graphs/DataVisualization, Qt WebEngine, KDE Frameworks, and FFTW are not baseline dependencies.

Prefer **vcpkg manifest mode with a tested pinned registry baseline**, explicit features, and validated arm64 macOS/x64 Windows triplets. A baseline pins registry resolution; compiler/SDK versions, feature closure, and binary provenance must also be recorded. **Conan 2 is an alternative**, using qualified recipes, profiles, and a tested lockfile if that route is deliberately selected; do not mix two managers for the same native dependency graph. No vcpkg manifest/baseline or Conan lockfile should be fabricated before package selection and both-platform builds establish what actually works.

Future unit tests should use GoogleTest for native core code and Qt Test for Qt adapters when those targets exist. Scientific worker tests should use pytest with deterministic fixtures and numerical tolerances. GoogleTest and Qt Test are not present in this build. P1-004 deliberately uses a small dependency-free CTest executable with explicit return-code checks as a temporary harness and records the migration point; it is not a GoogleTest pass count.

macOS application bundling, dependency deployment, code signing, hardened runtime, notarization/stapling, and clean-machine verification are release plans only. Windows packaging, DLL deployment, Authenticode signing, and clean-machine verification are likewise plans only. No app bundle, installer, signing identity, notarization submission, or signed artifact was produced.
