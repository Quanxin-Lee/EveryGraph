# P3-008: Build the graph editor UI and style template system

## Metadata

- Task ID: P3-008
- Status: blocked
- Owner: EveryGraph UI
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-UI-002;ORG-PLOT-006;ORG-PLOT-007;ORG-PLOT-010;DG-UI-002;DG-UI-003;DG-STYLE-001;DG-STYLE-005;DG-STYLE-010;DG-STYLE-011;DG-STYLE-012;EG-REL-010;EG-REL-011;EG-REL-025;EG-REL-041;EG-REL-059;EG-REL-061
- Dependencies: P1-006;P2-012;P3-003;P3-004;P3-005;P3-006;P3-007
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: build the professional graph editor surface and the style, theme, template and graph-option system on top of the P3 scene model, with every edit dispatched through application commands.

Allowed files and interfaces: `apps/desktop` graph editor, object manager, inspector and option-dialog sources, `modules/plot` option and style model sources, `tests/plot` and `tests/ui`, and this record.

Out of scope: new chart or layout engines, scientific analysis, persistence beyond graph options, export writers and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Object manager, graph controls, graph navigation, mini-axis settings, gear menu, style context menu and column selection are keyboard accessible and dispatch application commands. | Not run |
| AC-2 | Style and theme save/apply, templates and defaults, graph-option persistence and reload, graph-table font size, and color and fill styles are implemented. | Not run |
| AC-3 | Axis-break mini buttons, shift-graphs-on-column-add, the connect command icon and current-layer behavior integrate with the editor. | Not run |
| AC-4 | Invalid settings produce diagnostics; persisted options reopen without changing stored data. | Not run |
| AC-5 | Qt Test offscreen accessibility and focus checks pass in the Qt presets; no direct scene mutation outside commands is possible. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-008 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-009 and P3-015 consume the editor surface. Record the accessible names and focus order as evidence and request independent review.

Files or commands needed to resume: [P1-006 record](p1-006-workbench-shell-document-host.md), [P3-006 record](p3-006-layout-publication-sizing.md), [P3-007 record](p3-007-legends-annotations-reference-lines.md), [module design](../architecture/module-design.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
