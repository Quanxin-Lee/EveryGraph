# P3-001: Implement the scientific scene snapshot and CPU reference renderer

## Metadata

- Task ID: P3-001
- Status: blocked
- Owner: EveryGraph Visualization
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-PLOT-001;DG-PLOT-002;DG-STYLE-002
- Dependencies: P2-012;P2-002;P2-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement the immutable scientific scene snapshot, page and layer model and CPU reference renderer that every later P3 task consumes, keeping scientific coordinates in float64 and the scene backend-neutral.

Allowed files and interfaces: new `modules/plot/include/everygraph/plot/` and `modules/plot/src/` scene sources, new `modules/render/include/everygraph/render/` and `modules/render/src/` CPU reference sources, the module CMake files, new `tests/plot/` and `tests/render/`, the root subdirectory registration lines and this record. Other modules, build options and task records are read-only.

Out of scope: chart types, axes, UI widgets, export formats, persistence, GPU backends, Windows execution and any claim that a P3 capability is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A SceneSnapshot carries a version, page dimensions and device scale, color policy, ordered draw groups, clips, transforms, styles, glyph runs, paths, images and stable pick IDs; public scene headers contain no Qt or GPU types. | Not run |
| AC-2 | The renderer consumes an immutable scene and target description, returns diagnostics and cannot mutate a project or dataset; unsupported features produce explicit fallback diagnostics rather than missing objects. | Not run |
| AC-3 | Layers, coordinate spaces, z-order and clipping are explicit and deterministic; scientific coordinates remain float64 with local-origin handling for very large values. | Not run |
| AC-4 | Graph and document creation and binding to immutable dataset versions go through the P1-004 kernel; no direct mutation path exists outside the application executor. | Not run |
| AC-5 | Headless golden scenes render deterministically and the module boundary check proves no Qt type in scene public headers. | Not run |
| AC-6 | Tests pass in `macos-debug`, `macos-release` and `macos-sanitize`; no Windows claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-001 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-002 and P3-003 consume the scene contract. Run the three macOS presets, the module boundary check and the headless golden scenes, then request independent review.

Files or commands needed to resume: [rendering architecture](../architecture/rendering-architecture.md), [module design](../architecture/module-design.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [P2-012 record](p2-012-p2-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
