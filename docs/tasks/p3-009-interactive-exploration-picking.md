# P3-009: Implement interactive exploration and exact picking

## Metadata

- Task ID: P3-009
- Status: blocked
- Owner: EveryGraph UI
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-EXPLORE-001;ORG-EXPLORE-002;ORG-EXPLORE-003;ORG-EXPLORE-004;ORG-EXPLORE-005;ORG-EXPLORE-006;ORG-EXPLORE-007;DG-UI-006;DG-UI-013;EG-REL-069;ORG-GADGET-002
- Dependencies: P1-006;P2-004;P2-010;P2-012;P3-003;P3-004;P3-005;P3-006
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement zoom, pan, readers, loupe, ROI extraction, non-destructive point editing, browser graph, slicer, click-event dashboards and exact picking that resolves original data rows.

Allowed files and interfaces: `apps/desktop` plot interaction, reader and dashboard sources, `modules/plot` hit-testing and selection sources, `tests/plot` and `tests/ui`, and this record.

Out of scope: new chart types, export writers, persistence, scientific analysis and Windows execution.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Zoom rectangle, pan and reset preserve correct inverse transforms; linked groups update together and independent groups do not. | Not run |
| AC-2 | Exact picking resolves original row IDs and coordinates after filtering; the data reader and screen reader show exact values and units. | Not run |
| AC-3 | Loupe and hover tooltips do not change axes; ROI extraction returns exactly the included source rows. | Not run |
| AC-4 | Non-destructive mask and point edits are undoable; browser graph, slicer, click-event dashboards and the vertical cursor dispatch commands and preserve selections. | Not run |
| AC-5 | Keyboard and accessible paths are verified with Qt Test offscreen in the Qt presets. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-009 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-010, P3-014 and P3-016 consume the picking and selection interfaces. Verify exact picking against the dense fixtures in P3-016 and request review.

Files or commands needed to resume: [P1-006 record](p1-006-workbench-shell-document-host.md), [P2-010 record](p2-010-masks-mirrors-filtered-views.md), [P3-003 record](p3-003-xy-curves-lod.md), [P3-006 record](p3-006-layout-publication-sizing.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
