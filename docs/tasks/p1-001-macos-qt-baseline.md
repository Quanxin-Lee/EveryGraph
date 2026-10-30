# P1-001: Qualify Qt 6.12 and a Minimal Workbench on macOS arm64

## Metadata

- Task ID: P1-001
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P0-001
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not applicable; no Git repository exists. A local Git baseline is requested as part of authorization before build-file changes.
- Authorization: not granted. Required: Qt 6.12.x installation and network downloads; vcpkg or approved package-manager bootstrap; edits to CMakeLists.txt, CMakePresets.json and a new tools/qt_probe target; optional local Git initialization and baseline commit; documentation updates for actual commands, versions and licensing. No remote publication, signing, notarization, Windows execution or business feature implementation.

## Scope and acceptance

Objective: prove that the selected Qt 6.12 baseline can build and start a minimal Qt Widgets workbench on macOS arm64 under project-controlled CMake presets, and record the exact version, deployment and LGPL replacement facts. This is a qualification task, not a product workbench. It supports, but does not complete, EG-ADD-016 or DG-PERF-004; the cross-platform gate requires the Windows evidence from P1-002.

Allowed files and interfaces: CMakeLists.txt, CMakePresets.json, a new `tools/qt_probe/` target, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md` and this record. No Qt types in future domain/application public headers; no product modules or business logic.

Out of scope: the full P1 workbench, menus/docks/tabs as a product, command gateway, data model, analysis, project persistence, Windows, CI changes, signed or notarized packaging, release artifacts, and any claim that P1 is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Exact Qt patch, install source, selected modules, license basis, CMake, Ninja, compiler, SDK, macOS and architecture recorded | Not run |
| AC-2 | A minimal QMainWindow-based Qt Widgets probe configures and builds in Debug and Release under a project preset on macOS arm64 | Not run |
| AC-3 | A deterministic offscreen smoke test runs in CTest without a logged-in GUI session; existing planning/toolchain tests still pass | Not run |
| AC-4 | Dynamic Qt linkage is shown (for example with `otool -L`); deployment and a replaceable-LGPL-library experiment are recorded, including any limitation | Not run |
| AC-5 | The build remains separate from domain/application contracts, contains no scientific business logic, and makes no Windows, packaging, signing or CI claim | Not run |
| AC-6 | The development guide and licensing document contain the actual commands, versions, results and unresolved obligations; all existing checks remain green | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created the task and recorded the current toolchain gap; no implementation started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |

No Qt code, build change, installation or test result exists. This record is a blocked plan, not evidence of qualification.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.2 build 26B123, arm64; AppleClang 21.0.0; Python 3.14.6; CMake 3.31.6 and Ninja 1.11.1 only from the temporary `/tmp/everygraph-phase0-tools` environment. Qt and vcpkg are absent.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `cmake --version` and `ninja --version` with the temporary tools path | Available | CMake 3.31.6; Ninja 1.11.1.git.kitware.jobserver-1; not a durable dependency baseline |
| 2026-10-30 | Qt and vcpkg discovery (`qmake6`, package directories, `which vcpkg`) | Not found | Qt 6.12.x is not installed; no package route is established |
| 2026-10-30 | Existing CTest suites before this task | Passed 3/3 in debug, release and sanitize | Baseline only; no Qt target exists |

Checks not run and their reasons: Qt configure/build/startup, offscreen smoke test, deployment, LGPL replacement, ASan/UBSan with Qt, packaging and minimum-runtime tests. The task is blocked before these checks.

## Review and disposition

Reviewer and review type: independent EveryGraph Review will be required for the implementation and evidence. No reviewer has been assigned; no review has occurred.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: the repository contains only the Phase-0 planning/compiler scaffold; Qt is not installed, no Qt target exists and no Git baseline exists.

Blockers and unblock actions:

- Qt 6.12.x is not installed and network/package installation is not authorized. Confirm the exact available patch (do not assume 6.12.0) and authorize an official Qt installer or an approved package route.
- The license/module scope is not yet confirmed for the actual installation; record whether only LGPL-eligible Core/Gui/Widgets modules are used and preserve replacement/relinking evidence.
- vcpkg or the selected package manager is absent; P1-003 covers the durable dependency graph, but P1-001 needs a temporary approved route.
- No Git repository exists. Authorize local `git init` and a baseline commit before build-file edits if a reviewable diff is required; no remote or push is requested.
- The `/tmp` tool environment may disappear after host cleanup; do not record it as a dependency lock.

Next task / concrete next action: on authorization, Implementation installs the approved Qt patch, adds a minimal Qt Widgets probe, runs Debug/Release and the full CTest suite, performs the offscreen smoke and deployment/replacement checks, and returns actual logs and version evidence for independent Review. P1-003 follows; P1-002 remains blocked on a Windows environment.

Files or commands needed to resume: [CMake presets](../../CMakePresets.json), [CMake build definition](../../CMakeLists.txt), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), and the existing `python3 tools/validate_planning.py` check.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | Qt is absent and installation/network/build changes are not authorized; no execution started |
