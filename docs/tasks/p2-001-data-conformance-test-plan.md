# P2-001: Define the P2 Data and Persistence Conformance Test Plan

## Metadata

- Task ID: P2-001
- Status: planned
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-001; ORG-DATA-011; DG-DATA-005; DG-DATA-006; EG-ADD-002; EG-ADD-008; EG-ADD-010; EG-ADD-013
- Dependencies: none; this is a documentation plan that may start before P1 exits
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. P0-004 and P1-003 are being executed concurrently by their owners.
- Authorization: not granted. Required: create `docs/engineering/data-conformance-plan.md` and edit this record. All existing documents, the task board, the feature register, the architecture contracts, build files and product code are read-only. No implementation, dependency installation, Windows execution, push or publication.

## Scope and acceptance

Objective: turn the P2 data, import, formula and persistence contracts into an executable conformance plan before implementation. The plan defines golden fixtures, expected semantics, failure injection, tolerances and measurement procedures so later P2 tasks do not invent acceptance after the fact.

The plan must cover the shared DATA, IO and PERF gates in the [functional scope](../planning/functional-scope.md) section 3 and the storage and row-alignment contracts in the [data model](../architecture/data-model.md) and [project file format](../architecture/project-file-format.md). It is a planning document, not a test implementation or a coverage claim.

Allowed files and interfaces: a new `docs/engineering/data-conformance-plan.md` and this record. The [development roadmap](../planning/development-roadmap.md), [feature traceability register](../planning/feature-traceability.csv), [performance protocol](../engineering/performance-validation.md), [concurrency model](../architecture/concurrency-model.md) and every task record are read-only.

Out of scope: writing C++ or Python test code, changing the architecture contracts, changing the 428-row register or validator, acquiring proprietary fixtures, implementing import or storage, and claiming that any P2 capability is delivered.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The plan defines typed-value fixtures covering numeric, text, date, IEEE binary, missing, NaN, infinite, empty text, signed zero and display-versus-stored precision | Not run |
| AC-2 | The plan defines delimiter, quoting, encoding, header, skip/region, decimal-comma, malformed-row and unsupported-construct fixtures for CSV/TXT/clipboard import with expected diagnostics | Not run |
| AC-3 | The plan defines row-identity, column-identity, unit/dimension, mirror, mask and join/unmatched-row fixtures with invariant checks | Not run |
| AC-4 | The plan defines formula DAG, cycle, invalidation, recalculation and stable-error fixtures with deterministic expected results | Not run |
| AC-5 | The plan defines atomic-save, interrupted-save, recovery, migration and reopen fixtures, including the unsaved recovery window and corruption cases | Not run |
| AC-6 | The plan defines 100MB/1GB import and save/reopen measurement procedures, hardware/build disclosure, latency, throughput, peak memory and cancellation checks; no vendor number is used as a budget | Not run |
| AC-7 | The plan and this record pass the planning validator, contain no CJK and make no implementation or parity claim | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-001 and created this initial record scaffold; no plan content was written | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No conformance plan exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9. The task needs only documentation tools.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | The architecture contracts and P2 requirements exist; no consolidated conformance plan exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time `python3 tools/validate_planning.py` | Passed before the allocation records were added; post-allocation state is in the [P0-000 record](p0-000-task-allocation.md) | Documentation baseline only |

Checks not run and their reasons: the plan itself, fixture review and any implementation test. They require an assigned executor and authorization; implementation remains blocked on P1-013.

## Review and disposition

Reviewer and review type: independent data/QA review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is planned and not authorized.

## Handoff and remaining work

Last verified state: no P2 conformance plan exists. The data, import, formula, storage and performance contracts exist as architecture and requirement text; P2 implementation has not started.

Blockers and unblock actions: the user must authorize this documentation task and assign one executor. P2 implementation tasks remain blocked on the P1 exit gate (P1-013); this plan does not authorize code. Package versions from P1-003 are provisional until that task is accepted.

Next task / concrete next action: write the plan from the existing contracts, record every fixture family and failure case, run the planning validator, and request independent review. The plan informs P2-002 through P2-011.

Files or commands needed to resume: [data model](../architecture/data-model.md), [project file format](../architecture/project-file-format.md), [concurrency model](../architecture/concurrency-model.md), [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), [performance protocol](../engineering/performance-validation.md), [development guide](../engineering/development.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 as a startable documentation task; code implementation remains gated by P1-013 |
