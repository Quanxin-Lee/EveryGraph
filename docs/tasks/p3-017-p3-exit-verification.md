# P3-017: Verify the P3 exit gate

## Metadata

- Task ID: P3-017
- Status: blocked
- Owner: EveryGraph Verification
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: none; phase-gate governance and evidence verification
- Dependencies: P2-012;P3-001;P3-002;P3-003;P3-004;P3-005;P3-006;P3-007;P3-008;P3-009;P3-010;P3-011;P3-012;P3-013;P3-014;P3-015;P3-016
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: read-only verification of committed artifacts, the planning validator, the catalog checks, a board check and an isolated build if authorized; edits to this record and its board row only. No product code, no other task record edit, no acceptance of another task, no commit or push without separate authorization.

## Scope and acceptance

Objective: independently verify every P3 exit item against actual committed artifacts and commands and record a truthful pass, partial or gap verdict with a named owner for every residual item.

Allowed files and interfaces: this record and the P3-017 board row. All other files are read-only, including every P3 task record, the conformance corpus, benchmarks, source code and the board's other rows.

Out of scope: accepting another task, editing another owner's record, product implementation, new fixtures, dependency installation, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Every P3 exit item in the development roadmap and functional scope is classified pass, partial or gap with the exact artifact and command used as evidence. | Not run |
| AC-2 | P3-001 through P3-016 are accepted and their evidence is committed; any unaccepted task is a gap and the gate is not accepted. | Not run |
| AC-3 | Golden coordinate, scene and export tests, 1M/10M baselines, exact picking, accessible editing and font/DPI/clip/math policy are evidenced. | Not run |
| AC-4 | The planning validator, catalog checks, board schema/dependency/record-path check and macOS presets pass at the reviewed revision; failures are recorded rather than hidden. | Not run |
| AC-5 | No edit to another owner's files; the record states self-review versus independent review truthfully and makes no P4, parity, Windows or release claim. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-017 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: After all P3 tasks are accepted and committed, verify at that revision and record the gate verdict. P4 allocation may start only after an accepted P3 exit or an explicit user decision.

Files or commands needed to resume: [roadmap](../planning/development-roadmap.md), [functional scope](../planning/functional-scope.md), [P2-012 record](p2-012-p2-exit-verification.md), [board](../planning/task-board.csv), [development guide](../engineering/development.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
