# P3-016: Establish the million-point curve and scatter rendering baseline

## Metadata

- Task ID: P3-016
- Status: blocked
- Owner: EveryGraph Performance
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-PERF-002
- Dependencies: P2-012;P3-003;P3-005;P3-009
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: measure 1M and 10M curve and dense-scatter rendering with LOD build, first frame, interaction latency, peak memory, copy bytes and exact picking accuracy on declared hardware and build.

Allowed files and interfaces: new `benchmarks/` fixtures and harness sources, new `tests/performance/` checks and this record. Product code is read-only; deficits are reported to the owning task.

Out of scope: optimizing product code, changing LOD policy, closing the P7 performance row, vendor comparisons, Windows execution and release claims.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Deterministic 1M and 10M curve and dense-scatter fixtures generate without committing large data files. | Not run |
| AC-2 | The harness measures LOD build, first meaningful frame, interaction latency, peak memory, upload or copy bytes and exact picking accuracy with exact commands. | Not run |
| AC-3 | Full-data analysis and export paths remain separate from LOD and are verified. | Not run |
| AC-4 | Results are recorded with hardware, build and cache limits; no vendor speedup or P7 row closure is claimed. | Not run |
| AC-5 | Measurements do not block the UI thread and failures or cancellations are reported, not hidden. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-016 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: P3-017 uses the baseline as exit evidence. Report any budget gap to P3-003 and P3-009 rather than changing their contracts.

Files or commands needed to resume: [rendering architecture section 4](../architecture/rendering-architecture.md), [P3-003 record](p3-003-xy-curves-lod.md), [P3-009 record](p3-009-interactive-exploration-picking.md), [performance protocol](../engineering/performance-validation.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
