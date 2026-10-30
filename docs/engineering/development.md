# Phase 0 development and toolchain standards

## Scope and evidence boundary

This scaffold builds only `everygraph_toolchain_probe`, a dependency-free C++20 standard-library executable. It is not EveryGraph, a GUI prototype, a scientific implementation, or a benchmark. No Qt application target, dependency-manager bootstrap, library installation, package lockfile, Git initialization, or commit is part of this work.

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

The initial compiler-only runs used the anchored selection above. After completing the documents and adding the actual validator, the final review reran configure/build and the following unfiltered suites; each passed 3/3 tests:

```sh
ctest --preset macos-debug
ctest --preset macos-release
ctest --preset macos-sanitize
```

The six validator unit cases also passed when invoked directly with `python3 -m unittest discover -s tools -p test_validate_planning.py -v`. The validator checks preserved source hashes, coverage/anchors/classifications, local document links/fences/language and matrix arithmetic. It does not check remote URL availability, render Mermaid or test application/scientific behavior. The workflow runs full CTest so missing or failing planning checks fail CI.

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

Future unit tests should use GoogleTest for native core code and Qt Test for Qt adapters when those targets exist. Scientific worker tests should use pytest with deterministic fixtures and numerical tolerances. Those frameworks and test suites do not exist in this probe-only build and have no pass count here.

macOS application bundling, dependency deployment, code signing, hardened runtime, notarization/stapling, and clean-machine verification are release plans only. Windows packaging, DLL deployment, Authenticode signing, and clean-machine verification are likewise plans only. No app bundle, installer, signing identity, notarization submission, or signed artifact was produced.
