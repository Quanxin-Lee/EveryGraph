# P3-011: Implement scene layout clipboard and print export

## Metadata

- Task ID: P3-011
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-EXPORT-002;DG-EXPORT-003;DG-EXPORT-004;DG-EXPORT-005;DG-EXPORT-006;DG-EXPORT-008
- Dependencies: P2-012;P3-003;P3-004;P3-005;P3-006;P3-007
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement vector, raster, clipboard and print export for scenes and layouts with explicit physical dimensions, font policy, alpha handling and full-data resolution policy.

Allowed files and interfaces: `modules/render` SVG, PDF, EPS, PNG, TIFF and JPG export adapters, `apps/desktop` clipboard and print actions, `tests/render` tests and this record. The scene and layout public interfaces are consumed unchanged.

Out of scope: data export as text or XLSX, foreign formats, report/animation export, P6 integration adapters, release packaging and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | SVG, PDF and EPS preserve paths, text, clipping and physical dimensions; fonts are embedded or outlined with an explicit warning; EPS transparency is flattened or rejected explicitly. | Not run |
| AC-2 | PNG, TIFF and JPG export at chosen resolution with explicit pixel dimensions, DPI, alpha and color policy; JPEG loss and background behavior are disclosed. | Not run |
| AC-3 | Clipboard publishes PDF and bitmap flavors plus whole-figure and single-element bitmaps; layout export and OS print use correct page geometry. | Not run |
| AC-4 | Exports use full data or a declared resolution policy; a screen LOD buffer is never silently reused. | Not run |
| AC-5 | Independent readers verify PDF, SVG and PNG structure; golden export tests pass in the three macOS presets; no P6 compatibility claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-011 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-015 verifies the export corpus. Record the full-data and resolution policy, then request independent review.

Files or commands needed to resume: [rendering architecture section 6](../architecture/rendering-architecture.md), [P3-006 record](p3-006-layout-publication-sizing.md), [P3-007 record](p3-007-legends-annotations-reference-lines.md), [P2-012 record](p2-012-p2-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
