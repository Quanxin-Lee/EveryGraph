# P2-009: Implement Reshape, Query and Reduction Operations

## Metadata

- Task ID: P2-009
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-009; ORG-DATA-010; ORG-PREP-002; ORG-PREP-003; ORG-PREP-004; ORG-PREP-005; ORG-PREP-007
- Dependencies: P1-013; P2-004; P2-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The edit and formula engines are not accepted.
- Authorization: not granted. Required: create reshape/query commands and tests under `modules/data` and `tests/data`, module-local CMake registration and this record. No UI, persistence, plotting, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement the P2 table transformations that users need before plotting and analysis: merge, append, split, transpose, pivot, stack/unstack, worksheet query, conditional extraction and reduction. Every operation preserves typed metadata, row lineage and undo semantics, and reports duplicate-key or invalid-expression cases explicitly.

Allowed files and interfaces: new command sources under `modules/data/src/commands/`, public headers under `modules/data/include/everygraph/data/commands/`, tests and generated fixtures under `tests/data/commands/`, the module/test CMake entries required by P1-011, and this record. The data model and formula/edit interfaces are consumed.

Out of scope: plotting, statistical aggregation kernels, Pivot gadget UI, batch automation, import, persistence, Windows execution and any claim that all ORG-PREP or ORG-DATA-009 variants are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Merge, append, split and transpose preserve column types, metadata and row lineage; invalid shapes fail without partial output | Not run |
| AC-2 | Pivot and stack/unstack round-trip wide/long grouped data with category order, empty groups and declared duplicate-key policies | Not run |
| AC-3 | Worksheet query evaluates typed predicates and extracts expected stable row IDs; invalid fields or expressions fail atomically | Not run |
| AC-4 | Conditional extraction and duplicate reduction retain source-row lineage and use an explicit representative/aggregation policy | Not run |
| AC-5 | Every operation is a revision-checked command with undo/redo; dependent views receive the new revision and affected IDs | Not run |
| AC-6 | Golden fixtures cover each named operation, null/NaN categories, duplicates, empty inputs and rollback; no silent reordering occurs | Not run |
| AC-7 | Tests pass in the three Qt-free presets; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-009 and created this initial record scaffold; no reshape code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No reshape, query or reduction command exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No reshape, query or reduction command exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | The edit and formula engines are not accepted | The task is blocked before implementation |

Checks not run and their reasons: reshape/query/reduction tests, duplicate and null cases, undo and preset runs. They require P2-004, P2-005 and authorization.

## Review and disposition

Reviewer and review type: independent data review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no reshape or query command exists. The data/edit contracts are in [P2-002](p2-002-typed-data-model-immutable-versions.md) and [P2-004](p2-004-worksheet-edit-commands-undo.md); formula predicates use [P2-005](p2-005-formula-expression-dag.md).

Blockers and unblock actions: P2-004 and P2-005 must provide the edit and expression paths. The user must authorize the implementation and ensure the build-file lock. Statistical aggregation kernels remain separate P4 work.

Next task / concrete next action: after the dependencies, implement each operation as a command with golden fixtures, run the three Qt-free presets, and request review. P2-010 reuses grouping and selection semantics.

Files or commands needed to resume: [data model](../architecture/data-model.md), [functional scope](../planning/functional-scope.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [P2-005 record](p2-005-formula-expression-dag.md), [P2-001 record](p2-001-data-conformance-test-plan.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-004 and P2-005 are not accepted |
