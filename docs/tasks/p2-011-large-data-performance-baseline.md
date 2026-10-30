# P2-011: Establish the Large-Data Import and Save Performance Baseline

## Metadata

- Task ID: P2-011
- Status: blocked
- Owner: EveryGraph Performance
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-PERF-001; ORG-PERF-003; DG-PERF-002
- Dependencies: P1-013; P2-003; P2-006
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. Text import and the project container are not accepted.
- Authorization: not granted. Required: create benchmark scripts and generated fixtures under `benchmarks/`, a benchmark target/test registration, update the performance document and this record. No committed large datasets, vendor claims, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: collect the first truthful performance baseline for P2 import and save/reopen on declared macOS hardware and build settings. The baseline informs budgets; it does not close the P7 performance requirements.

Allowed files and interfaces: new `benchmarks/` scripts, fixture generators and a benchmark executable target, the benchmark CMake registration required by P1-011, [performance protocol](../engineering/performance-validation.md), [development guide](../engineering/development.md) only for measured build/command facts, and this record. The import and storage public interfaces are consumed; no product behavior changes.

Out of scope: optimization, SIMD/GPU/out-of-core work, Windows execution, vendor benchmark reproduction, committing 1GB fixtures, changing scientific semantics, release claims and closing ORG-PERF-001/ORG-PERF-003/DG-PERF-002.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Fixtures are generated deterministically at 100MB and 1GB with declared row/column types, locale and missing-value rates; large data is not committed | Not run |
| AC-2 | Import is measured against a serial baseline with input size, wall time, throughput, peak process-tree memory and cancellation behavior | Not run |
| AC-3 | Save/reopen of a mixed-series project is measured with latency, file size, peak memory and crash-safety checks on the same hardware/build | Not run |
| AC-4 | Every result records hardware, OS, compiler, CMake, Ninja, Qt/dependency versions, warm/cold cache and the exact command | Not run |
| AC-5 | Correctness checks confirm imported values and reopened state match expected fixtures; timing is not reported for a failed or partial run | Not run |
| AC-6 | The performance document records measurements and limits; no vendor speed claim or P7 row closure is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-011 and created this initial record scaffold; no benchmark or measurement started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No benchmark, fixture generator or measurement exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64. The baseline must record the actual build and hardware at execution time; no measurement is claimed here.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No benchmark harness, fixture generator or large-data measurement exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | Text import and the project container are not accepted | The task is blocked before measurement |

Checks not run and their reasons: 100MB/1GB import, save/reopen, memory, cancellation and correctness checks. They require P2-003, P2-006, declared hardware and authorization.

## Review and disposition

Reviewer and review type: independent performance review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no performance baseline exists. The measurement protocol is in [performance validation](../engineering/performance-validation.md); import and storage contracts are in [P2-003](p2-003-delimited-text-import-clipboard.md) and [P2-006](p2-006-project-container-atomic-save-recovery.md).

Blockers and unblock actions: P2-003 and P2-006 must be accepted. The user must approve the benchmark scope and provide or confirm the measurement hardware. Large fixtures remain generated and ignored; no vendor number is adopted as a budget.

Next task / concrete next action: after the dependencies, generate fixtures, run the import/save/reopen measurements, record exact commands and limits, and request review. P7 optimization follows only after this baseline and the P4 numerical workloads exist.

Files or commands needed to resume: [performance protocol](../engineering/performance-validation.md), [development guide](../engineering/development.md), [P2-003 record](p2-003-delimited-text-import-clipboard.md), [P2-006 record](p2-006-project-container-atomic-save-recovery.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-003 and P2-006 are not accepted |
