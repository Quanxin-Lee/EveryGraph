# P3-013: Implement polar ternary function and specialty 2D charts

## Metadata

- Task ID: P3-013
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-GRAPH-012;ORG-GRAPH-014;DG-PLOT-014;DG-FIT-006;DG-STYLE-004;EG-REL-003;EG-REL-021
- Dependencies: P2-005;P2-012;P3-002;P3-003;P3-004;P3-006
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement polar, vector and ternary plots, function plots, cause-and-effect charts and Graph Maker, and enumerate the specialty plot variants without claiming unenumerated support.

Allowed files and interfaces: `modules/plot` polar, ternary, function and specialty sources, `apps/desktop` Graph Maker surfaces, `tests/plot` and `tests/ui` tests, and this record.

Out of scope: P7 domain and geographic graphs, GIS and image engines, advanced statistical charts, foreign-format compatibility and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Polar angle conventions, vector magnitude and direction, and ternary composition axes render and invert correctly. | Not run |
| AC-2 | Function plots handle discontinuities without misleading bridges and use a declared sampling policy. | Not run |
| AC-3 | Graph Maker creates grouped and faceted graphs from column roles; append modes work. | Not run |
| AC-4 | Cause-and-effect charts render; DG-PLOT-014 specialty variants are enumerated with source, rights and uncertainty, and no support is claimed for unenumerated variants. | Not run |
| AC-5 | Golden scene tests pass in the three macOS presets; no P7 domain or geographic claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-013 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-015 adds the golden cases. Hand the DG-PLOT-014 enumeration to P0-000 for later allocation and request review.

Files or commands needed to resume: [P3-002 record](p3-002-axis-scale-transform-engine.md), [P3-004 record](p3-004-categorical-charts.md), [P2-005 record](p2-005-formula-expression-dag.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
