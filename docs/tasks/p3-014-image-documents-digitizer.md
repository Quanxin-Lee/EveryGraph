# P3-014: Implement image documents and the digitizer

## Metadata

- Task ID: P3-014
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-IO-012;ORG-GADGET-004;ORG-REPORT-004;DG-IO-011;DG-ANNOT-011;EG-REL-047;EG-REL-067
- Dependencies: P2-012;P3-002;P3-007;P3-009
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement image documents, image drag-and-drop, anchored image annotations and the digitizer kernel and gadget so points can be calibrated and extracted from an image into typed worksheet data.

Allowed files and interfaces: `modules/plot` image-document and digitizer sources, `apps/desktop` image window and gadget sources, `tests/plot` and `tests/ui`, and this record.

Out of scope: image processing and morphology, GIS, 3D, foreign image-format adapters beyond the pinned Qt support, export writers and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Images import by drag-and-drop or file open, display as document objects with stable IDs, and malformed files report diagnostics. | Not run |
| AC-2 | The digitizer calibrates numeric, date and time axes from an image and extracts points with exact source-row provenance. | Not run |
| AC-3 | The digitizer gadget shares the kernel and supports manual and automatic extraction from an interactive image ROI. | Not run |
| AC-4 | Anchored and cropped image annotations bind to scene anchors and resize correctly; SVG import into worksheet cells produces a declared representation. | Not run |
| AC-5 | Golden extraction tests pass in the three macOS presets; no P7 image-processing claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-014 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-015 adds the extraction corpus. Verify date and time calibration against the P3-002 transforms and request review.

Files or commands needed to resume: [P3-007 record](p3-007-legends-annotations-reference-lines.md), [P3-009 record](p3-009-interactive-exploration-picking.md), [P3-002 record](p3-002-axis-scale-transform-engine.md), [P2-012 record](p2-012-p2-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
