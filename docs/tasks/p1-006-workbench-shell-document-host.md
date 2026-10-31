# P1-006: Build the Qt Workbench Shell and Document Host Skeleton

## Metadata

- Task ID: P1-006
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation / P1-006 executor (Qoder main-agent session, 2026-10-31)
- Related requirements: ORG-UI-001; ORG-UI-004; DG-UI-004; EG-REL-009; EG-ADD-016
- Dependencies: P1-001; P1-003; P1-004; P1-005 (the board's set; all four are `accepted`)
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: `main` in the shared checkout at `45ff336` (`docs(p0-000): sync the P0-017 and P1-005 acceptances and open P1-006`); no branch or worktree is created. Section 0 rule 10's pathspec commit form is mandatory in this shared tree.
- Authorization: the user released P1-006 on 2026-10-31 by instructing this session to execute it from the board, complete implementation, testing, review and necessary fixes inside the authorized scope, and not return for ordinary technical choices. The same instruction requires explicit authorization before scope expansion, architecture change, system dependency installation, Git commit or push, paid resources or external publication. On 2026-10-31 the user explicitly authorized (1) enabling the pinned `qtbase` `testlib` feature and rebuilding it, because AC-6 requires Qt Test, and (2) **manifest-only local commits only** for P1-006 and for P0-000's designation and board sync; no push, publication, tag, branch creation or force operation is authorized. The user instruction of 2026-10-31 also directed that the task record and board be updated: the record is updated by this executor, while the board remains P0-000's artifact under section 0 rule 4 and is synced through the recorded `Board sync request`. Not authorized: product business data or persistence, real Project/Workbook/Graph domain objects, a Windows build or claim, signing/notarization/packaging, dependency installation beyond the authorized `testlib` feature, or any edit to another task's record or artifact.

## Scope and acceptance

Objective: build a minimal but real Qt Widgets workbench skeleton that hosts documents, docks and command-driven actions without putting scientific logic in widgets. The shell consumes the P1-004 command kernel and uses fake in-memory project objects only.

This task is the first product-shaped UI layer, not a data editor or plot application. It must demonstrate the workspace composition described in the architecture: main window, menus/toolbars, dockable panels, central tabbed documents, detachable/restorable windows and a project tree with stable fake object IDs.

Allowed files and interfaces: new `apps/desktop` sources, new `modules/presentation_qt` public/private sources, Qt Test targets, `CMakeLists.txt` and `CMakePresets.json` for the shell targets, `vcpkg.json` for the authorized `testlib` feature declaration, `docs/engineering/development.md` and this record. The application/foundation public interfaces from P1-004 are consumed, not duplicated.

Out of scope: real Project/Workbook/Graph domain objects, file import or persistence, virtual tables, plot canvases, numerical analysis, AI/plugin panels, Windows execution, release packaging/signing and any claim that P1 or EG-ADD-016 is complete.

Change manifest (AGENTS.md section 0 rule 7; this table is the exact writable and committable set. It gates the `planned`/`blocked` -> `in_progress` move. Shared normative rows wait for P0-000's designation in the rule 9 request below; the task does not write them before that grant):

| Path | Action | Basis |
|---|---|---|
| `docs/tasks/p1-006-workbench-shell-document-host.md` | modify | Own task record |
| `modules/presentation_qt/CMakeLists.txt` | create | Own module target definition; no live manifest claims it |
| `modules/presentation_qt/include/everygraph/presentation/command_dispatcher.hpp` | create | Own task deliverable; kernel-dispatch boundary |
| `modules/presentation_qt/include/everygraph/presentation/document_host.hpp` | create | Own task deliverable; tabbed host and detached window |
| `modules/presentation_qt/include/everygraph/presentation/project_tree_model.hpp` | create | Own task deliverable; read-only project tree model |
| `modules/presentation_qt/include/everygraph/presentation/properties_panel.hpp` | create | Own task deliverable; read-only properties dock panel |
| `modules/presentation_qt/include/everygraph/presentation/window_placement.hpp` | create | Own task deliverable; off-screen restore |
| `modules/presentation_qt/include/everygraph/presentation/workbench_icons.hpp` | create | Own task deliverable; programmatic high-DPI icons |
| `modules/presentation_qt/include/everygraph/presentation/workbench_window.hpp` | create | Own task deliverable; QMainWindow composition |
| `modules/presentation_qt/src/command_dispatcher.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/document_host.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/project_tree_model.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/properties_panel.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/window_placement.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/workbench_icons.cpp` | create | Own task deliverable |
| `modules/presentation_qt/src/workbench_window.cpp` | create | Own task deliverable |
| `apps/desktop/CMakeLists.txt` | create | Own composition root targets and test registration |
| `apps/desktop/src/fake_project.hpp` | create | Own task deliverable; fake in-memory project commands and seed |
| `apps/desktop/src/fake_project.cpp` | create | Own task deliverable; Qt-free fake command registry |
| `apps/desktop/src/main.cpp` | create | Own task deliverable; composition root executable |
| `apps/desktop/tests/workbench_tests.cpp` | create | Own task deliverable; Qt Test offscreen suite |
| `apps/desktop/tests/check_shell_boundaries.cmake` | create | Own task deliverable; architecture check |
| `vcpkg.json` | modify | P1-003 artifact; the user authorized the graph change on 2026-10-31 and P0-000 designates this task the writer for the `desktop-shell` feature only |

Shared normative paths are not in the manifest until P0-000 grants the request below: `CMakeLists.txt`, `CMakePresets.json` and `docs/engineering/development.md`.

### Rule 9 request to P0-000 (2026-10-31)

The task needs four changes outside its currently granted manifest. Under section 0 rule 9 it requests them here and does not write them until P0-000 designates the writer for this revision.

| Target path | Exact change wanted | Reason | Owning task |
|---|---|---|---|
| `CMakeLists.txt` | One `EVERYGRAPH_ENABLE_DESKTOP_SHELL` option and one conditional block that finds Qt 6.12.0 Core/Gui/Widgets/Test, runs `qt_standard_project_setup()`, and adds `modules/presentation_qt` and `apps/desktop`; every Qt-free preset keeps the option `OFF` | The new targets must be reachable from the authoritative root build without changing the probe or kernel targets | Shared normative; P0-000 names the designated writer for this revision |
| `CMakePresets.json` | One hidden desktop base preset (vcpkg toolchain, immutable manifest, `arm64-osx-dynamic`, shared `VCPKG_INSTALLED_DIR`, `VCPKG_MANIFEST_FEATURES=desktop-shell`, `EVERYGRAPH_ENABLE_DESKTOP_SHELL=ON`) plus `macos-debug-desktop`/`macos-release-desktop` configure, build and test presets; no change to existing preset values | AC-6 and AC-7 need reproducible pinned-graph commands, and Qt must not be built twice for debug and release | Shared normative; P0-000 names the designated writer for this revision |
| `docs/engineering/development.md` | One P1-006 section: targets, exact Qt source from P1-003, commands, results, limits, no Windows/signing claim | AC-7 requires the current build facts in their authoritative document | Shared normative; P0-000 names the designated writer for this revision |
| `vcpkg.json` | Add one root feature `desktop-shell` whose only dependency is `qtbase` with the `testlib` feature; baseline, existing dependencies and features unchanged; probe presets do not enable it, so their resolved graph and the P1-003 SBOM remain unchanged | AC-6 requires Qt Test and the pinned graph omits the non-default `testlib` feature; the user authorized this graph change on 2026-10-31 | P1-003 artifact (accepted, no live executor); P0-000 adjudicates the writer for this revision |

`docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json` is deliberately not requested: the feature is opt-in per build tree, so the probe graph that the SBOM describes keeps its 43 packages. The desktop build's graph delta is recorded in this record and the development guide, and a release SBOM remains P1-012's obligation.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A QMainWindow-based application starts with menus, a toolbar, a status bar, a project-tree dock, a properties dock and a central document host | Not run |
| AC-2 | The project tree displays nested fake folders/objects with stable IDs; create, move and find actions dispatch registered P1-004 commands rather than mutating widgets directly | Not run |
| AC-3 | The document host supports multiple tabs, detach/float and re-dock; an off-screen window is restored to a visible screen after a simulated display change | Not run |
| AC-4 | Toolbar actions have accessible names, shortcuts and high-DPI icons; no vendor icon assets are copied | Not run |
| AC-5 | Widgets contain no scientific business logic and no direct document mutation; the application kernel is the only writer | Not run |
| AC-6 | Qt Test/offscreen tests cover startup, project-tree actions, tab/detach/restore and command dispatch; the existing suites remain green | Not run |
| AC-7 | The development guide records the exact Qt source from P1-003, commands, results and limits; no Windows, signing or packaging claim is made | Not run |
| AC-8 | An architecture check confirms no Qt types appear in foundation/application public headers and no UI target is linked into the application kernel | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit criteria and architecture composition contract; no UI source or target started | [Takeover audit](p0-002-takeover-state-verification.md), [architecture overview](../architecture/architecture-overview.md) section 4 and [board](../planning/task-board.csv) |
| 2026-10-31 | EveryGraph Implementation / P1-006 executor | Verified the board, all four dependencies and the current tree; confirmed the pinned Qt graph lacks Qt Test, obtained the user's authorizations for `testlib` and local commits, and wrote this manifest plus the rule 9 request. No shared normative path or source file has been written | Board rows P1-001/P1-003/P1-004/P1-005 `accepted`; `vcpkg.json` feature list; `out/build/macos-debug-qt-vcpkg/vcpkg_installed/arm64-osx-dynamic` |

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` at `out/vcpkg`; pinned manifest baseline `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`; Homebrew Qt 6.12.0_2 remains provisional P1-001 evidence and is not used by this task.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `grep '^P1-00[1-6]' docs/planning/task-board.csv` and the four dependency records | P1-001, P1-003, P1-004 and P1-005 read `accepted`; P1-006 is `planned` on the board and was `blocked` in its record | Dependency-clear; the record/board divergence is P0-000's sync backlog |
| 2026-10-31 | `python3 -B tools/validate_task_manifest.py` | Baseline: 36 errors and 49 notices, all pre-existing in accepted records, plus the known P1-006 record/board divergence notice | The checker does not resolve designations; new shared-path claims by this task will add documented duplicate-claim reports |
| 2026-10-31 | `python3 tools/validate_planning.py` | Passed before any edit in this round | The task has not changed a planning document yet |
| 2026-10-31 | `ls out/build/macos-debug-qt-vcpkg/vcpkg_installed/arm64-osx-dynamic/include/Qt6 | grep -i test` and the lib listing | No QtTest headers or `libQt6Test`; `testlib` is a non-default `qtbase` feature absent from `vcpkg.json` | Confirms AC-6 needs the authorized graph change |
| 2026-10-31 | `git status --short` | Clean working tree at `45ff336` before this record edit | No other task has uncommitted work in the tree |

Checks not run and their reasons: shell build, Qt Test startup/navigation/detach tests, off-screen restore, accessibility names, high-DPI icons and architecture checks. They require the P0-000 designation and the rebuilt graph.

## Review and disposition

Reviewer and review type: independent UI and architecture review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked on P0-000's designation and has not started implementation.

## Handoff and remaining work

Last verified state: the only Qt artifact is the P1-001 probe; no workbench shell, project tree, document host or UI test exists. This record now carries the manifest and the rule 9 request; no shared normative path has been edited.

Blockers and unblock actions:

- P0-000 must designate P1-006 as the single writer for this revision of `CMakeLists.txt`, `CMakePresets.json`, `docs/engineering/development.md` and `vcpkg.json`, with the limits stated in the rule 9 request. Until then the task stays blocked.

Next task / concrete next action: after P0-000's designation commit, move the record to `in_progress`, add the four granted paths to the manifest with the designation commit, enable the `desktop-shell` feature, build the shell with fake objects, run offscreen Qt Test, and request independent review. Do not use the provisional Homebrew Qt as the shell dependency.

Files or commands needed to resume: [architecture overview](../architecture/architecture-overview.md), [module design](../architecture/module-design.md), [development guide](../engineering/development.md), [P1-001 record](p1-001-macos-qt-baseline.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-004 record](p1-004-command-transaction-kernel.md), [P1-005 record](p1-005-worker-handshake-cancellation.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-001 was in review, P1-003 blocked and P1-004 not started; implementation was not authorized |
| 2026-10-31 | Blocked confirmed; scope extended to one opt-in `vcpkg` feature and two build/preset paths | All four dependencies are accepted and the user released the task, but section 0 rule 9 requires P0-000's designation before the shared normative paths and the P1-003 `vcpkg.json` can be written; the user explicitly authorized the `testlib` graph change and local commits |
