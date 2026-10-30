# P1-013: Verify the P1 Exit Gate

## Metadata

- Task ID: P1-013
- Status: blocked
- Owner: EveryGraph Verification
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-001; EG-ADD-016; EG-ADD-018; DG-PERF-004
- Dependencies: P1-001; P1-002; P1-003; P1-004; P1-005; P1-006; P1-007; P1-008; P1-009; P1-010; P1-011; P1-012
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. Most P1 tasks are blocked or in progress.
- Authorization: not granted. Required: read-only verification of every P1 artifact, execution of the existing presets and validator, and edits to this record only. No product code, board edit, other task record, Windows/CI execution, signing, push or publication.

## Scope and acceptance

Objective: independently verify the P1 exit conditions in the [development roadmap](../planning/development-roadmap.md) section 3 and the [functional scope](../planning/functional-scope.md): native macOS arm64 and Windows x64 shell evidence, keyboard/IME/high-DPI/multi-monitor workflows, no Qt in domain/application interfaces, command-driven create/edit/undo fixtures, worker cancellation, and signed-test packaging with LGPL replacement. The gate records pass, partial or gap for each item and never weakens a criterion to make a platform failure disappear.

Allowed files and interfaces: this record only. All other files are read-only, including the [task board](../planning/task-board.csv), the [development guide](../engineering/development.md), the [architecture contracts](../architecture/architecture-overview.md) and every P1 record.

Out of scope: accepting another task, implementing product work, Windows or signing execution, changing acceptance criteria or platform minima, and any release/publication claim.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Every P1 exit item is classified pass, partial or gap with the exact artifact, command, platform and revision | Not run |
| AC-2 | The Windows x64 probe and shell items are reported from actual Windows evidence only; macOS results are not reused as Windows evidence | Not run |
| AC-3 | The public-header and linkage checks confirm no Qt types or GUI dependencies in the foundation/application interfaces and no UI target linked into the kernel | Not run |
| AC-4 | The command-driven create/edit/undo fixture and the worker cancellation/crash evidence are reproduced or explicitly reported as missing | Not run |
| AC-5 | Packaging and LGPL replacement results are reported separately from Developer ID, notarization and clean-machine qualification; no unperformed claim is accepted | Not run |
| AC-6 | The planning validator, board schema/dependency/record check and the existing preset suites pass at the reviewed revision, or failures are recorded as gaps | Not run |
| AC-7 | The record states that the verification was independent and that no other owner's file was edited | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P1-013 and created this initial record scaffold; no gate verification started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No P1 gate result exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. A Windows x64 host or authorized CI and an Apple Developer identity are external inputs for P1-002/P1-010 and P1-009.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time P1 state inspection | P1-001 and P1-004 are accepted; P1-003 is in progress; the remaining P1 tasks are planned or blocked | The gate cannot pass while required P1 tasks are open |
| 2026-10-31 | Allocation-time `python3 tools/validate_planning.py` | Passed before the allocation records were added; post-allocation state is in the [P0-000 record](p0-000-task-allocation.md) | Documentation baseline only; not a P1 gate result |

Checks not run and their reasons: the full P1 exit matrix, Windows execution, signed/notarized packaging, headless/UI smoke reproduction and independent review. They require accepted P1 tasks and an assigned executor.

## Review and disposition

Reviewer and review type: independent EveryGraph Verification review is the task itself. No reviewer or executor has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked on the P1 dependencies and is not authorized.

## Handoff and remaining work

Last verified state: the P1 gate has not been independently verified. The macOS Qt baseline and command kernel are accepted; the dependency pins, worker prototype, shell, preferences, accessibility, packaging and Windows replication remain open or blocked.

Blockers and unblock actions: every listed P1 task must be accepted or explicitly recorded as a gap. P1-002, P1-009 and P1-010 require external Windows/signing inputs; if those remain unavailable, the gate records a partial P1 exit and does not claim completion. The user must authorize the verification and assign one executor.

Next task / concrete next action: after the dependencies are resolved or explicitly gapped, read every P1 artifact, rerun the validator and the available presets, build the exit matrix, and record the decision. P0-000 updates the board; P1-013 does not edit it.

Files or commands needed to resume: [task board](../planning/task-board.csv), [roadmap](../planning/development-roadmap.md), [functional scope](../planning/functional-scope.md), the P1 records, [development guide](../engineering/development.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 as the P1 exit gate; P1-002, P1-003 and the shell tasks are not accepted |
