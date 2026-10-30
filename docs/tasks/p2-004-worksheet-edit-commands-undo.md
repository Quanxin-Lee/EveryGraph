# P2-004: Implement Worksheet Edit Commands and Undo

## Metadata

- Task ID: P2-004
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-003; DG-DATA-002; DG-DATA-004
- Dependencies: P2-002
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The P2 data core is not accepted.
- Authorization: not granted. Required: create worksheet edit commands and tests under `modules/data` and `tests/data`, module-local CMake registration and this record. No UI, formulas, persistence, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement cell, row and column editing as revision-checked commands over immutable dataset versions, with typed validation, stable identities and undo/redo through the P1-004 kernel. This is the write path that the virtual table and formula engine consume.

Allowed files and interfaces: new command sources under `modules/data/src/commands/`, public headers under `modules/data/include/everygraph/data/commands/`, tests under `tests/data/commands/`, the module/test CMake entries required by the P1-011 topology, and this record. The P1-004 command kernel and P2-002 data model are consumed through their public interfaces.

Out of scope: formulas, masks, sorting/filtering, import, persistence, virtual-table widgets, multi-user editing, Windows execution and any claim that the data-editing requirements are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Insert, delete, move and resize row/column commands preserve column roles, units, formats, metadata and stable IDs; undo restores the prior version | Not run |
| AC-2 | Cell edits validate type, unit and missing-value rules; invalid input returns a structured error and leaves the previous version intact | Not run |
| AC-3 | Column type/format changes preview incompatible conversions or reject them with diagnostics rather than silently changing values | Not run |
| AC-4 | All edits use the P1-004 expected-revision check and commit path; reentrant or stale edits cannot overwrite a newer revision | Not run |
| AC-5 | Undo/redo and repeat-last-action operate on the same transaction contract; dependent-view events carry the new revision and affected IDs | Not run |
| AC-6 | Golden tests cover every command, rejected edits, undo/redo sequences and stale-revision races; no direct mutation path exists | Not run |
| AC-7 | Tests pass in `macos-debug`, `macos-release` and `macos-sanitize`; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-004 and created this initial record scaffold; no edit-command code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No edit command, undo path or test exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | The P1-004 kernel exists; no worksheet edit command or data model exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | P2-002 is not implemented | The task is blocked before implementation |

Checks not run and their reasons: edit-command tests, undo/redo sequences, stale-revision races, boundary checks and preset runs. They require P2-002 and authorization.

## Review and disposition

Reviewer and review type: independent data/transaction review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no worksheet edit command exists. The command/transaction contract is in [P1-004](p1-004-command-transaction-kernel.md); the data contract is in [data model](../architecture/data-model.md).

Blockers and unblock actions: P2-002 must provide the data model and row-identity domain. The user must authorize the implementation and ensure the build-file lock. Formulas and sorting/filtering remain separate tasks.

Next task / concrete next action: after P2-002, implement the edit commands and undo sequences, add stale-revision and rejected-edit tests, run the three Qt-free presets, and request review. P2-005, P2-007, P2-009 and P2-010 consume this command path.

Files or commands needed to resume: [data model](../architecture/data-model.md), [P1-004 record](p1-004-command-transaction-kernel.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md), [P2-001 record](p2-001-data-conformance-test-plan.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-002 is not accepted |
