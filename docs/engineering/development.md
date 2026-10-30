# Phase 0 development and toolchain standards

## Scope and evidence boundary

This scaffold builds `everygraph_toolchain_probe`, a dependency-free C++20 standard-library executable. An opt-in `everygraph_qt_probe` target adds a minimal Qt Widgets workbench qualification probe; it is not EveryGraph, a GUI prototype, a scientific implementation, or a benchmark. No dependency-manager bootstrap, package lockfile, or business module is part of this work.

The toolchain probe checks `std::span`, an owning vector snapshot surviving mutation and destruction of its source, move-owned data in `std::jthread`, stop-token observation, and joining before reading worker results. A latch makes stop observation deterministic without timing assumptions. Explicit return-code checks remain active in Release; assertions are not used as tests.

## Verified local environment

P1-001 re-observed the implementation host on 2026-10-30; the values below are current and supersede the earlier Phase-0 observation. They describe this host only, not a supported-configuration table:

| Item | Observed value |
| --- | --- |
| OS | macOS 27.0.1, build 26A434 |
| Host and probe binaries | arm64; Mach-O 64-bit arm64 executables |
| Selected developer directory | `/Library/Developer/CommandLineTools` (CLT 27.0.0.0, not Xcode.app) |
| Compiler | AppleClang 21.0.0 (`clang-2100.3.34.2`); CMake identifies `21.0.0.21000334` |
| Compiler used by presets | `/usr/bin/clang++` |
| SDK | macOS 27.0 |
| Deployment target | 14.4; confirmed in the probe binaries and the Qt 6.12.0 frameworks |
| CMake | 3.31.6; project minimum 3.28 |
| Ninja pip distribution | 1.11.1.4 |
| `ninja --version` | `1.11.1.git.kitware.jobserver-1` |
| Tool environment | repository-local `.venv` (ignored), Python 3.13.9, pinned `cmake==3.31.6` and `ninja==1.11.1.4` |
| Local Python | `python3` on PATH is Anaconda 3.13.9; planning validation requires an interpreter >= 3.10 |
| CMake-discovered Python | `/opt/homebrew/Frameworks/Python.framework/Versions/3.14/bin/python3.14` (Homebrew Python 3.14.2) |
| Qt used by the probe presets | Homebrew `qtbase` 6.12.0_2 at `/opt/homebrew/opt/qtbase` |
| clang-format / clang-tidy | Not found on PATH; configuration supplied, checks not executed |

The tool environment is created with Python's `venv` and pip distributions `cmake==3.31.6` and `ninja==1.11.1.4`:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install cmake==3.31.6 ninja==1.11.1.4
```

The venv is ignored by Git and is not an application dependency lock. The pip distribution version and the Ninja executable's reported version are deliberately recorded separately. Local Python 3.13.9 is separate from the unexecuted CI choice of Python 3.13 and from the CMake-discovered Homebrew 3.14.2 interpreter.

Version self-checks used locally:

```sh
export PATH="$PWD/.venv/bin:$PATH"
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
export PATH="$PWD/.venv/bin:$PATH"
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

A successful run on macOS 27.0.1 with a deployment target of 14.4 is not evidence of execution on macOS 14.4, other macOS releases, Intel Macs, or Windows. These are toolchain checks, not application, numerical-accuracy, throughput, or memory-scaling results.

## Full planning and toolchain validation

CMake requires `Python3` >= 3.10 with the `Interpreter` component. When `BUILD_TESTING` is on, CTest registers:

- `toolchain_probe`: the compiled C++ probe, with a 30-second timeout.
- `planning_validation`: the Python interpreter running `tools/validate_planning.py` from the source root, with a 60-second timeout.
- `planning_validator_tests`: six standard-library unittest cases for validator behavior, with a 60-second timeout.

The initial compiler-only runs used the anchored selection above. After completing the documents and adding the actual validator, the final review reran configure/build and the following unfiltered suites; each passed 3/3 tests:

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

No workflow was dispatched. This is CI configuration, not CI success evidence. The local repository has an `origin` remote and a baseline commit, but no CI run has been observed.

## Future dependencies and release work: selected direction, partially qualified

The planned application baseline is **C++20 with Qt 6.12.0, dynamically linked LGPL-eligible Qt Core/Gui/Widgets**. P1-001 installed Homebrew `qtbase` 6.12.0_2 and proved a minimal Qt Widgets probe on macOS arm64, including local deployment and an LGPL replacement experiment. That qualifies the probe route only: it is not the final dependency lock, not a Windows result, and not a blanket license grant for every Qt module. Before packaging, audit selected modules and transitive features, preserve notices and source/offer obligations as applicable, and verify replacement/relinking rights and signing interactions. See the separate [GUI and license evidence](../research/gui-and-license-evidence.md) and [technology evaluation](../planning/technology-evaluation.md).

Planned library directions include Arrow C++ arrays/IPC and Parquet interchange, Eigen, SQLite, approved Ceres least-squares functionality, pocketfft, and selected Boost.Math. Python scientific workers may use NumPy/SciPy and other qualified packages. None was installed or linked by this scaffold; compatible releases, enabled features, ABI/CRT combinations, numerical behavior, licenses, and transitive closure still require real selection and tests. Qt Charts/Graphs/DataVisualization, Qt WebEngine, KDE Frameworks, and FFTW are not baseline dependencies.

Prefer **vcpkg manifest mode with a tested pinned registry baseline**, explicit features, and validated arm64 macOS/x64 Windows triplets. A baseline pins registry resolution; compiler/SDK versions, feature closure, and binary provenance must also be recorded. **Conan 2 is an alternative**, using qualified recipes, profiles, and a tested lockfile if that route is deliberately selected; do not mix two managers for the same native dependency graph. The Homebrew `qtbase` bottle is a P1-001 qualification route, not a second dependency authority: P1-003 must pin the native graph before business modules depend on Qt. No vcpkg manifest/baseline or Conan lockfile should be fabricated before package selection and both-platform builds establish what actually works.

Future unit tests should use GoogleTest for native core code and Qt Test for Qt adapters when those targets exist. Scientific worker tests should use pytest with deterministic fixtures and numerical tolerances. Those frameworks and test suites do not exist in this probe-only build and have no pass count here.

macOS application bundling, dependency deployment, code signing, hardened runtime, notarization/stapling, and clean-machine verification are release plans only. P1-001 produced a local ad-hoc signed probe bundle solely for the deployment and replacement experiment. No Developer ID signing, hardened runtime, notarization, installer or release artifact was produced. Windows packaging, DLL deployment, Authenticode signing, and clean-machine verification are likewise plans only.
