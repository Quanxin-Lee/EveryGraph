# P3-007: Implement legends annotations drawing objects and reference lines

## Metadata

- Task ID: P3-007
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-PLOT-004;ORG-PLOT-005;DG-ANNOT-001;DG-ANNOT-002;DG-ANNOT-003;DG-ANNOT-004;DG-ANNOT-005;DG-ANNOT-006;DG-ANNOT-007;DG-ANNOT-008;DG-ANNOT-009;DG-ANNOT-010;DG-PLOT-008;DG-STYLE-013;EG-REL-017;EG-REL-018;EG-REL-019;EG-REL-020;EG-REL-022;EG-REL-062
- Dependencies: P2-012;P3-001;P3-002;P3-003;P3-004;P3-005;P3-006
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement automatic and custom legends, color scales, extra axes, annotations, drawing objects, reference lines and snapping with transactional editing.

Allowed files and interfaces: `modules/plot` legend, annotation, drawing and reference-line sources, `tests/plot` tests and this record. The scene, axis and layout interfaces are consumed unchanged.

Out of scope: image-document annotations that require P3-014, UI property inspectors, export writers, persistence and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Automatic, custom, categorical and layer legends plus color ramps, color-scale legends and extra axes are implemented. | Not run |
| AC-2 | Text, label, bracket, region, range, graphic, magnify, slope and anchored-image annotations support anchors, offsets, alignment, snapping and clipping. | Not run |
| AC-3 | Reference lines from columns, label rows and across layers render correctly, including single-label line behavior. | Not run |
| AC-4 | A declared math and text subset renders with font fallback for Unicode and CJK; no system TeX dependency is introduced. | Not run |
| AC-5 | Annotation edits are transactional and undoable; golden scene tests pass in the three macOS presets. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-007 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-008 and P3-011 consume the annotation and legend model. Verify undo and snapping with the editor UI before requesting review.

Files or commands needed to resume: [rendering architecture section 3](../architecture/rendering-architecture.md), [P3-006 record](p3-006-layout-publication-sizing.md), [P3-001 record](p3-001-scene-snapshot-cpu-renderer.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
