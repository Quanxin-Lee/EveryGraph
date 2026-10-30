# P3-006: Implement layout multi-axis panels and publication sizing

## Metadata

- Task ID: P3-006
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-GRAPH-005;ORG-PLOT-008;ORG-PLOT-009;ORG-PLOT-011;DG-STYLE-006;DG-STYLE-007;DG-STYLE-008;EG-REL-024;EG-REL-027
- Dependencies: P2-012;P3-001;P3-002;P3-003;P3-004;P3-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement the layout engine for multi-axis and multi-panel graphs, insets, graph groups, page sizing and publication rescaling so on-screen and export geometry share one model.

Allowed files and interfaces: `modules/plot` layout, panel, inset and page sources, `tests/plot` tests and this record. The earlier P3 scene, axis and chart types are consumed through public interfaces.

Out of scope: UI controls and dialogs, export writers, persistence, new chart types and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Dual-Y and multiple-X/Y panels, linked and independent ranges, insets with independent zoom, layer ordering and graph groups are implemented. | Not run |
| AC-2 | Page size, margins, multiple graphs per page and publication resize with explicit font and line scaling are verified against a journal-width fixture. | Not run |
| AC-3 | Current-layer indication, floating graph rescaling and drawing-group behavior are covered. | Not run |
| AC-4 | On-screen and export geometry use one layout engine, verified with P3-011. | Not run |
| AC-5 | Golden geometry tests pass in the three macOS presets and no UI thread is blocked. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-006 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-007, P3-011 and P3-013 consume the layout model. Verify the shared layout engine with the export corpus before requesting review.

Files or commands needed to resume: [rendering architecture](../architecture/rendering-architecture.md), [P3-001 record](p3-001-scene-snapshot-cpu-renderer.md), [P3-002 record](p3-002-axis-scale-transform-engine.md), [P3-003 record](p3-003-xy-curves-lod.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
