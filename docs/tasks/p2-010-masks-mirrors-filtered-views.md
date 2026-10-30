# P2-010: Implement Grouping, Masks, Mirrors and Filtered Views

## Metadata

- Task ID: P2-010
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-007; ORG-DATA-008; ORG-DATA-012; ORG-DATA-013; ORG-PREP-001; ORG-PREP-006; ORG-PREP-009; DG-DATA-013
- Dependencies: P1-013; P2-002; P2-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The data model and edit commands are not accepted.
- Authorization: not granted. Required: create grouping/mask/mirror commands and tests under `modules/data` and `tests/data`, module-local CMake registration and this record. No UI beyond command interfaces, plotting, analysis kernels, persistence, Windows execution, push or publication.

## Scope and acceptance

Objective: implement stable sort/filter, grouping, mask columns, mirror bindings and cross-sheet column propagation as revision-checked commands. These features share row identity and selection semantics; plots and analyses must later receive exactly the same selected row IDs.

Allowed files and interfaces: new command sources under `modules/data/src/commands/`, public headers under `modules/data/include/everygraph/data/commands/`, tests and generated fixtures under `tests/data/commands/`, the module/test CMake entries required by P1-011, and this record. The data model and formula/edit interfaces are consumed.

Out of scope: plot/analysis kernels, visual mask editing, ROI extraction, P7 data filtering variants, UI widgets, persistence, Windows execution and any claim that ORG-DATA-007/008 or DG mask semantics are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Stable sort preserves ties deterministically; filtering mixed missing/NaN values is explicit; clearing the filter restores the original row order and values | Not run |
| AC-2 | Grouping by categorical columns and row groups preserves group order and metadata for downstream plot/analysis inputs | Not run |
| AC-3 | Mask columns evaluate own/other-column conditions and expose a selection with stable row IDs; a command-level fixture confirms plots and analyses would receive the identical selection | Not run |
| AC-4 | Mirror columns share immutable storage across workbooks; source edits create new versions and preserve prior bindings; deleting a source reports dangling dependents | Not run |
| AC-5 | Add-column propagation copies formulas with correct relative-reference adjustment across selected sheets and is undoable | Not run |
| AC-6 | Group reduction uses an explicit aggregation and preserves group counts, order and lineage | Not run |
| AC-7 | Golden tests cover ties, null/NaN, reorder/restore, mirror cycles, dangling references, undo and rollback; the three Qt-free presets pass; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-010 and created this initial record scaffold; no grouping, mask or mirror code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No grouping, mask, mirror or filtered-view command exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No grouping, mask, mirror or filtered-view command exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | The data model and edit commands are not accepted | The task is blocked before implementation |

Checks not run and their reasons: grouping, mask, mirror, propagation and filtered-view tests; plot/analysis selection parity; preset runs. They require P2-002, P2-004 and authorization.

## Review and disposition

Reviewer and review type: independent data/selection review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no grouping, mask, mirror or filtered-view command exists. Row-alignment and mirror semantics are defined in [data model](../architecture/data-model.md) section 1.

Blockers and unblock actions: P2-002 and P2-004 must provide the data model and command path. The user must authorize the implementation and ensure the build-file lock. Plot/analysis selection parity is verified when those consumers exist; this task only proves the command-level selection.

Next task / concrete next action: after the dependencies, implement the commands with golden fixtures, run the three Qt-free presets, and request review. P3 plot/analysis tasks must reuse the same row-ID selection contract.

Files or commands needed to resume: [data model](../architecture/data-model.md), [functional scope](../planning/functional-scope.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [P2-009 record](p2-009-reshape-query-grouping.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-002 and P2-004 are not accepted |
