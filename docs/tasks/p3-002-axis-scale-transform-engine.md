# P3-002: Implement the axis scale tick and transform engine

## Metadata

- Task ID: P3-002
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-PLOT-001;ORG-PLOT-002;ORG-PLOT-003;DG-STYLE-003;EG-REL-005;EG-ADD-008
- Dependencies: P3-001
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement scientific scales, ticks, grids, axis breaks and coordinate transforms with correct domain restrictions, inverse mappings and unit labels for every P3 plot.

Allowed files and interfaces: `modules/plot` axis, scale, tick and transform sources, `tests/plot` tests and this record. The scene model from P3-001 is consumed through its public interfaces.

Out of scope: plot types, UI dialogs, layout, export, persistence, Windows execution and P4 analysis.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Linear, log, probability, reciprocal, date and time scales implement forward and inverse mappings, domain restrictions and explicit gaps for invalid samples. | Not run |
| AC-2 | Ticks, minor ticks, grids, zero and reference lines, ranges, inversion and unit labels render deterministically; scientific notation and date/time labels are verified. | Not run |
| AC-3 | Axis breaks, independent dual-Y and multiple-X axes map and invert correctly; linked and independent ranges are distinguishable. | Not run |
| AC-4 | Inverse mapping round-trips within a declared tolerance; invalid log-domain samples form gaps rather than silent clipping. | Not run |
| AC-5 | Golden coordinate tests pass in `macos-debug`, `macos-release` and `macos-sanitize`; no Qt type appears in axis public headers. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-002 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-003, P3-004 and P3-005 consume the axis engine. Add the cross-layer reference-line cases to P3-007 through the public transform interfaces, then request review.

Files or commands needed to resume: [rendering architecture](../architecture/rendering-architecture.md), [P3-001 record](p3-001-scene-snapshot-cpu-renderer.md), [P2-012 record](p2-012-p2-exit-verification.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
