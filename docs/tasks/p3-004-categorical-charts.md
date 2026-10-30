# P3-004: Implement categorical bar pie waterfall and grouped charts

## Metadata

- Task ID: P3-004
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-GRAPH-003;ORG-GRAPH-004;ORG-GRAPH-006;ORG-GRAPH-009;DG-PLOT-004;DG-PLOT-005;DG-PLOT-006;DG-PLOT-007;DG-PLOT-010;EG-REL-049;EG-REL-060
- Dependencies: P2-012;P2-002;P3-001;P3-002
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement the categorical and distribution chart family: single, grouped, stacked, horizontal and vertical bars and areas, pie and doughnut, waterfall, grouped and nested categories, stocks, pivot and individual-value charts.

Allowed files and interfaces: `modules/plot` categorical chart sources, `tests/plot` tests and this record. The P3-001 scene and P3-002 transforms are consumed through public interfaces.

Out of scope: statistical histogram variants from ORG-GRAPH-010, scalar-field charts, legends and annotations, UI, export, persistence and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Single, grouped, stacked, horizontal and vertical bars and areas with negative values and connecting bars render at numeric, date and category positions. | Not run |
| AC-2 | Pie and doughnut proportions and label anchoring; waterfall offset, stack ordering and displacement are verified. | Not run |
| AC-3 | Grouped and nested category reorder, stocks, pivot and individual-value chart variants render with explicit semantics; bar and pivot fill patterns are covered. | Not run |
| AC-4 | Stable category and row identity survive filtering and reorder; no value is mutated by rendering. | Not run |
| AC-5 | Golden scene tests pass in the three macOS presets; no P4 statistical claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-004 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-006 and P3-007 consume these charts. Record the explicit histogram deferral to P4 in the review notes and request review.

Files or commands needed to resume: [functional scope](../planning/functional-scope.md), [P3-001 record](p3-001-scene-snapshot-cpu-renderer.md), [P3-002 record](p3-002-axis-scale-transform-engine.md), [P2-012 record](p2-012-p2-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
