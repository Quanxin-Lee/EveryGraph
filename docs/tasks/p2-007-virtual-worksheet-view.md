# P2-007: Build the Virtual Worksheet View

## Metadata

- Task ID: P2-007
- Status: blocked
- Owner: EveryGraph UI
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-UI-001; DG-DATA-001; DG-DATA-002; ORG-DATA-001; ORG-DATA-003
- Dependencies: P1-006; P1-013; P2-002; P2-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The shell, data model and edit commands are not accepted.
- Authorization: not granted. Required: extend `apps/desktop` and `modules/presentation_qt` with a worksheet view and Qt Test coverage, register the target/test, and update this record. No changes to data/application public interfaces beyond consuming them, no scientific logic in widgets, no Windows execution, push or publication.

## Scope and acceptance

Objective: provide the first real data view on top of the P2 data model and P1-004 command kernel. The view renders typed worksheet data with bounded memory, dispatches edits as commands, and exposes stable column/row identities without copying source arrays into widgets.

Allowed files and interfaces: `apps/desktop` and `modules/presentation_qt` sources and tests created by P1-006, Qt Test target/test registration, the module/test CMake entries established by P1-011, and this record. The data model and edit commands are consumed through public interfaces; they are not modified.

Out of scope: formulas, masks/filters beyond status indicators, plots, Excel import, persistence, scientific calculations in widgets, Windows execution and any claim that the full virtual table or ORG-DATA-001 is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A bounded Qt model/view renders typed columns and rows from immutable dataset versions without copying the full dataset into widget-owned storage | Not run |
| AC-2 | Stable column and row IDs are preserved through scrolling, sorting display, editing and refresh; no translated header becomes an identity | Not run |
| AC-3 | Cell edits dispatch P2-004 commands and refresh only affected views; rejected edits show diagnostics and leave the prior revision visible | Not run |
| AC-4 | Missing, NaN, infinite, empty text, dates and units have documented display rules distinct from stored values; display precision never writes back | Not run |
| AC-5 | A 100MB fixture scrolls and edits without blocking the UI thread beyond the declared budget; no scientific logic or direct document mutation lives in widgets | Not run |
| AC-6 | Qt Test/offscreen tests cover startup, rendering, scrolling, editing, rejected edits, refresh and stable IDs; the existing suites remain green | Not run |
| AC-7 | The architecture check confirms no Qt types in foundation/application/data public headers and no UI target linked into the kernel | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-007 and created this initial record scaffold; no view code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No worksheet view, model or test exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Qt 6.12.0 from the P1-003 pinned graph. No Windows execution.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No `apps/desktop` worksheet view or table model exists; only the P1-001 Qt probe and P1-004 kernel prototypes exist | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | P1-006, P2-002 and P2-004 are not accepted | The task is blocked before implementation |

Checks not run and their reasons: view/model tests, 100MB scrolling, editing/refresh, offscreen Qt Test and boundary checks. They require the shell, data model, edit commands and authorization.

## Review and disposition

Reviewer and review type: independent UI and architecture review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no worksheet view exists. The shell contract is in [P1-006](p1-006-workbench-shell-document-host.md); the data and edit contracts are in [P2-002](p2-002-typed-data-model-immutable-versions.md) and [P2-004](p2-004-worksheet-edit-commands-undo.md).

Blockers and unblock actions: P1-006 must provide the shell, P2-002 the data model and P2-004 the command path. The user must authorize the UI work and ensure the build-file lock. Windows behavior remains in P1-010/P2-012 scope, not this task.

Next task / concrete next action: after the dependencies, implement the bounded model/view and command dispatch, add offscreen tests and the 100MB fixture, run the Qt presets and boundary check, and request review.

Files or commands needed to resume: [architecture overview](../architecture/architecture-overview.md), [module design](../architecture/module-design.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [development guide](../engineering/development.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; the shell, data model and edit commands are not accepted |
