# P3-005: Implement heatmap contour and scalar-field plots

## Metadata

- Task ID: P3-005
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-GRAPH-007;DG-PLOT-011;EG-REL-048
- Dependencies: P2-012;P2-002;P3-001;P3-002
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement regular-grid and XYZ heatmap, contour and color-block plots with explicit interpolation nodes, missing-cell masks and overlaid contours across layers.

Allowed files and interfaces: `modules/plot` scalar-field sources, a minimal typed scalar-grid or interpolation node in `modules/dataflow` if the contract requires it, `tests/plot` and `tests/dataflow`, the module CMake files and this record.

Out of scope: statistical or matrix engines, 3D surfaces, image processing, UI, export, persistence and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Regular-grid and XYZ heatmap, contour and color-block scenes render with checked orientation, color mapping and missing-cell masks. | Not run |
| AC-2 | Interpolation and binning are explicit typed dataflow nodes with declared algorithms and options; the renderer performs no opaque analysis. | Not run |
| AC-3 | Overlaid contours across layers and color scales render correctly; missing cells form explicit gaps. | Not run |
| AC-4 | Golden numeric and scene tests show no silent smoothing or replacement of raw data. | Not run |
| AC-5 | Tests pass in the three macOS presets; no P7 scalar-field performance or domain claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-005 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-006 and P3-016 consume the scalar-field scenes. Record the interpolation-node contract for P4 reuse and request review.

Files or commands needed to resume: [rendering architecture](../architecture/rendering-architecture.md), [P3-001 record](p3-001-scene-snapshot-cpu-renderer.md), [P3-002 record](p3-002-axis-scale-transform-engine.md), [P2-012 record](p2-012-p2-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
