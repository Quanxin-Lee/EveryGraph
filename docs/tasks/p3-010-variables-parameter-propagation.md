# P3-010: Implement variables and parameter propagation

## Metadata

- Task ID: P3-010
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-VAR-001;DG-VAR-002;DG-VAR-003;DG-VAR-004;DG-VAR-006;DG-VAR-007;DG-VAR-008;DG-VAR-009;DG-VAR-010;DG-VAR-011;DG-VAR-012;DG-STYLE-014;ORG-RECALC-003;EG-REL-023
- Dependencies: P2-005;P2-010;P2-012;P3-003;P3-006;P3-009
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement the variable system and its propagation through masks, labels, plots and analyses, including sliders, text, expressions, color and marker schemes, current time and analysis-lock visibility.

Allowed files and interfaces: `modules/data` variable and expression sources, `modules/application` variable commands, `modules/plot` propagation interfaces only where the public contract requires them, `tests/data`, `tests/application` and this record.

Out of scope: new plot types, UI dialogs beyond the declared controls, persistence, scientific analysis engines and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Slider and integer-step, value, text, number-from-column, expression, font, color-scheme, marker-scheme, current-time and text-menu or text-set variables work with organizing folders. | Not run |
| AC-2 | Variable changes propagate deterministically through masks, labels, plots and analyses; automatic and manual recalculation honors the P2 dependency graph. | Not run |
| AC-3 | Cycles, ambiguous names and invalid types produce diagnostics; undo restores the previous variable state and derived scenes. | Not run |
| AC-4 | Mask popup slider, axis selector and analysis-lock visibility metadata work; hiding and showing analysis locks is explicit. | Not run |
| AC-5 | Golden propagation tests pass in the three macOS presets and public data headers remain Qt-free. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-010 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

This record is an allocation plan, not completed work.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The task is blocked by the phase gate; no scene, plot, export or UI implementation for this task exists.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source and dependency inspection | Read the current board, roadmap, functional scope, rendering architecture and dependency records; no scene, plot, export or UI implementation for this task exists and the dependencies are not accepted. | Read-only check; not a test of the task |

Checks not run and their reasons: scene, plot, export, UI and preset tests. They require the P2 exit gate, accepted dependencies and authorization.

## Review and disposition

Reviewer and review type: independent review is required for this task. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not authorized.

## Handoff and remaining work

Last verified state: No implementation exists for this task; only the P0-000 allocation record and the board row exist.

Blockers and unblock actions: P2-012 and the listed P3 dependencies must be accepted (or explicitly gated by the user), and the user must authorize the module directories, build registration and a single writer per shared file.

Next task / concrete next action: P3-015 and P3-017 consume the propagation evidence. Verify variable undo and cycle diagnostics with the P4 analysis interfaces when they exist, then request review.

Files or commands needed to resume: [P2-005 record](p2-005-formula-expression-dag.md), [P2-010 record](p2-010-masks-mirrors-filtered-views.md), [P3-009 record](p3-009-interactive-exploration-picking.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
