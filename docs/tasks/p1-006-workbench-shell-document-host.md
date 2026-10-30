# P1-006: Build the Qt Workbench Shell and Document Host Skeleton

## Metadata

- Task ID: P1-006
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: ORG-UI-001; ORG-UI-004; DG-UI-004; EG-REL-009; EG-ADD-016
- Dependencies: P1-001; P1-003; P1-004
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated branch or worktree must be created after the dependencies are accepted.
- Authorization: not granted. Required: create `apps/desktop` and `modules/presentation_qt`, Qt targets/tests, documentation and this record. Qt must come from the P1-003 pinned graph, not the provisional Homebrew installation. No business data, real project persistence, Windows execution, code signing, commit, push or publication is authorized.

## Scope and acceptance

Objective: build a minimal but real Qt Widgets workbench skeleton that hosts documents, docks and command-driven actions without putting scientific logic in widgets. The shell consumes the P1-004 command kernel and uses fake in-memory project objects only.

This task is the first product-shaped UI layer, not a data editor or plot application. It must demonstrate the workspace composition described in the architecture: main window, menus/toolbars, dockable panels, central tabbed documents, detachable/restorable windows and a project tree with stable fake object IDs.

Allowed files and interfaces: new `apps/desktop` sources, new `modules/presentation_qt` public/private sources, Qt Test targets, `CMakeLists.txt` and `CMakePresets.json` for the shell targets, `docs/engineering/development.md` and this record. The application/foundation public interfaces from P1-004 are consumed, not duplicated.

Out of scope: real Project/Workbook/Graph domain objects, file import or persistence, virtual tables, plot canvases, numerical analysis, AI/plugin panels, Windows execution, release packaging/signing and any claim that P1 or EG-ADD-016 is complete.

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

No shell source, Qt target or UI test exists. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Homebrew Qt 6.12.0_2 is present only as provisional P1-001 evidence.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | P1-003 state inspection | No vcpkg manifest, immutable baseline or SBOM exists; `vcpkg` is absent | The shell must not depend on the provisional Homebrew Qt route |
| 2026-10-30 | Existing Qt probe tests | `macos-debug-qt` and `macos-release-qt` each 4/4 passed | Probe evidence only; no shell or document host exists |
| 2026-10-30 | Source inspection | No `apps/` or `modules/presentation_qt` directory exists | Confirms the task has not started |

Checks not run and their reasons: shell build, Qt Test startup/navigation/detach tests, off-screen restore, accessibility names, high-DPI icons and architecture checks. They require P1-003 and P1-004.

## Review and disposition

Reviewer and review type: independent UI and architecture review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: the only Qt artifact is the P1-001 probe; no workbench shell, project tree, document host or UI test exists.

Blockers and unblock actions:

- P1-001 must pass independent review and be accepted as the Qt baseline evidence.
- P1-003 must pin the native dependency graph and produce an SBOM; the provisional Homebrew Qt bottle is not the durable dependency authority.
- P1-004 must establish the command/transaction contract consumed by the shell.
- The user must authorize creation of `apps/desktop` and `modules/presentation_qt`, plus Qt Test targets and build changes.

Next task / concrete next action: after P1-001, P1-003 and P1-004 are accepted, build the shell with fake objects, run offscreen Qt Test, and request independent review. Do not start with the provisional Homebrew library as if it were the final dependency route.

Files or commands needed to resume: [architecture overview](../architecture/architecture-overview.md), [module design](../architecture/module-design.md), [development guide](../engineering/development.md), [P1-001 record](p1-001-macos-qt-baseline.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-004 record](p1-004-command-transaction-kernel.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-001 is in review, P1-003 is blocked and P1-004 is not started; implementation is not authorized |
