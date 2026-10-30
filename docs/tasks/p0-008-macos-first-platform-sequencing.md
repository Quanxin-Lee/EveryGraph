# P0-008: Record the macOS-first platform sequencing decision

## Metadata

- Task ID: P0-008
- Status: planned
- Owner: EveryGraph Lead
- Record writer: unassigned until an executor starts
- Related requirements: none; sequencing decision that affects EG-ADD-016 and DG-PERF-004 without changing the 428-row register
- Dependencies: none
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: repository `main` at `d0adda8` with the uncommitted P0-000 board update that allocated this task; no branch, worktree or stash exists
- Authorization: the user decided on 2026-10-31 that Windows adaptation is paused for now, only platform interfaces remain in the lower layers, and macOS Apple Silicon is adapted first. This task records that decision in governance and architecture documents only. It does not authorize code, build changes, board edits, edits to another task's record, Windows implementation, signing or publication.

## Scope and acceptance

Objective: make the macOS-first sequencing authoritative in the standards and architecture documents, preserve Windows as a deferred target whose interfaces stay in the platform layer, and prevent any Windows support or cross-platform qualification claim.

Allowed files and interfaces: `AGENTS.md` (Section 1 targets and the Section 2 platform line), `docs/planning/functional-scope.md` (the UI acceptance gate and platform sequencing text), `docs/architecture/module-design.md` (platform ports and the dual-platform test statement), `docs/architecture/architecture-overview.md` only if it repeats the platform matrix, a new `docs/decisions/ADR-0005-platform-sequencing.md`, and this record.

Out of scope: code and CMake changes, `docs/engineering/development.md` while P1-003 has uncommitted changes, the task board (P0-000 owns it), `docs/planning/feature-traceability.csv`, other task records, Windows implementation or evidence, acceptance of any task, signing, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | AGENTS.md states that macOS Apple Silicon is the only active qualification target for the current phase, Windows x64 is deferred but retained as a later target, and Windows support is not claimed | Document diff |
| AC-2 | The functional-scope UI gate and platform text record macOS-only current acceptance and a deferred Windows gate; the 428-row traceability register is unchanged | Document diff and validator |
| AC-3 | The module design keeps the existing platform ports (FileAccess, SecretStore, ProcessHost, ResourceLimiter, SystemIntegration, DisplayCapabilities), states that only macOS implementations are delivered now, prohibits platform-specific types in domain and application public interfaces, and removes the both-platform P1 test claim | Document inspection |
| AC-4 | ADR-0005 records the decision, the alternatives (continue both platforms; drop Windows permanently), the consequences (deferred evidence and portability debt) and the revisit trigger (a Windows host or authorized CI, and before any Windows claim or cross-platform release) | ADR |
| AC-5 | `python3 tools/validate_planning.py` passes with local links and the English policy intact | Command output |
| AC-6 | Only the allowed documents and this record are edited; the board and other task records remain untouched, and no requirement is deleted or rewritten | `git status` and diff |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | User / project owner | Decided that Windows adaptation is paused, platform interfaces stay in the lower layers, and macOS is adapted first | Direct user instruction |
| 2026-10-31 | EveryGraph Coordination / P0-000 executor | Allocated this task and its initial record scaffold; updated the board rows for P1-002, P1-010 and P1-013 to defer Windows and narrow the P1 exit to macOS; did not edit any task record | [Board](../planning/task-board.csv); [P0-000](p0-000-task-allocation.md) |

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. macOS 27.0.1 arm64; Python 3.13.9.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Not run | Pending | This task was allocated but not started |

Checks not run and their reasons: the task is planned; no document was edited and no validator run was performed for this task.

## Review and disposition

Reviewer and review type: not assigned; no independent review has started.
Reviewed files / revision: none; this record is the allocation scaffold.
Findings, resolutions and recheck evidence: none.
Acceptance decision, accepting role and date: pending.

## Handoff and remaining work

Last verified state: the task is planned and unstarted. The board defers Windows and narrows P1-013 to a macOS exit; the P1-002, P1-010 and P1-013 records still describe the original scope because only their executors may edit them.

Blockers and unblock actions: none for this documentation task. Do not edit `docs/engineering/development.md` until the P1-003 executor commits its uncommitted changes.

Next task / concrete next action: assign an executor; it must read the board, the P0-000 record and this record, then update only the allowed documents.

Files or commands needed to resume: [board](../planning/task-board.csv), [P0-000](p0-000-task-allocation.md), [AGENTS.md](../../AGENTS.md), [functional scope](../planning/functional-scope.md), [module design](../architecture/module-design.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned and allocated | Direct user decision to pause Windows adaptation and adapt macOS first; P0-000 allocated the task and created this scaffold |
