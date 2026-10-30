# P2-005: Implement the Formula and Expression DAG

## Metadata

- Task ID: P2-005
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-004; ORG-MATH-001; ORG-RECALC-002; DG-DATA-007; DG-DATA-009; DG-FIT-008; DG-FIT-011
- Dependencies: P2-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. Worksheet edit commands are not accepted.
- Authorization: not granted. Required: create formula/expression sources under `modules/dataflow`, tests under `tests/dataflow`, module/test CMake registration, the root `add_subdirectory(modules/dataflow)` line and this record. No Excel formula import, UI, persistence, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement the typed expression and column/cell formula engine with a dependency DAG, cycle rejection, deterministic invalidation and recalculation over immutable data versions. The engine is Qt-free and uses the P2 data model and P1-004 transaction kernel.

Allowed files and interfaces: new `modules/dataflow/include/everygraph/dataflow/` and `modules/dataflow/src/` formula/expression sources, new `modules/dataflow/CMakeLists.txt`, new `tests/dataflow/` tests, the single root `add_subdirectory(modules/dataflow)` registration line required by the P1-011 topology, and this record. The data and command interfaces from P2-002/P2-004 are consumed.

Out of scope: Excel formula import, Flash Fill/Smart Formula, fitting functions, analysis algorithms, masks, UI formula editors, persistence, Windows execution and any claim that ORG-DATA-004 or all DG function-library semantics are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A typed expression parser and evaluator support column references, cell references, functions, operators, constants and row numbers with deterministic precedence and typed errors | Not run |
| AC-2 | Column and cell formulas recompute dependents on upstream edits; only the affected subgraph is invalidated; results are cached with input revision identity | Not run |
| AC-3 | Cycles and ambiguous names are rejected with stable diagnostics before commit; no partial result is published | Not run |
| AC-4 | Auto and manual recalculation modes are explicit; a reopened manual-mode formula does not silently recompute without the declared trigger | Not run |
| AC-5 | A versioned function library supports deterministic name resolution and rejects unsafe or unsupported syntax; no host code execution is possible | Not run |
| AC-6 | Text expressions preserve Unicode and declared missing-value/format policies; numeric errors remain distinct from missing values | Not run |
| AC-7 | Golden numeric/text/DAG/cycle tests pass in the three Qt-free presets; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-005 and created this initial record scaffold; no formula code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No parser, DAG or formula test exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No expression parser, formula engine or dependency DAG exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | P2-004 is not implemented | The task is blocked before implementation |

Checks not run and their reasons: parser/evaluator tests, DAG invalidation, cycle rejection, recalculation modes, function-library tests and preset runs. They require P2-004 and authorization.

## Review and disposition

Reviewer and review type: independent formula/data review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no formula engine exists. The dataflow contract is in [data model](../architecture/data-model.md) and [module design](../architecture/module-design.md); the edit contract is in [P2-004](p2-004-worksheet-edit-commands-undo.md).

Blockers and unblock actions: P2-004 must provide the edit path and data model. The user must authorize the new `modules/dataflow` module and ensure the build-file lock. Excel formula import remains a separate adapter task.

Next task / concrete next action: after P2-004, implement the parser, evaluator, DAG and function registry with golden tests, run the three Qt-free presets, and request review. P2-008, P2-009 and P2-010 consume formula semantics.

Files or commands needed to resume: [data model](../architecture/data-model.md), [module design](../architecture/module-design.md), [P1-004 record](p1-004-command-transaction-kernel.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [P2-001 record](p2-001-data-conformance-test-plan.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-004 is not accepted |
