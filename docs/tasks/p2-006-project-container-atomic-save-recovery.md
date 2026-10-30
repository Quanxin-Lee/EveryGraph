# P2-006: Implement the Project Container with Atomic Save and Recovery

## Metadata

- Task ID: P2-006
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-003; EG-ADD-010; EG-ADD-013
- Dependencies: P2-001; P2-002; P2-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The data model, edit commands and conformance plan are not accepted.
- Authorization: not granted. Required: create a storage module and tests under `modules/storage` and `tests/storage`, module/test CMake registration, the root `add_subdirectory(modules/storage)` line and this record. No UI, encryption, foreign project formats, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement the native project container: a versioned SQLite metadata store with bounded Arrow IPC data chunks, atomic save, reopen, migration hooks and interrupted-save recovery. The task proves only the native P2 format; OPJ/OPJU, `.dgraph`, protection and release packaging remain separate gates.

Allowed files and interfaces: new `modules/storage/include/everygraph/storage/` and `modules/storage/src/` sources, new `modules/storage/CMakeLists.txt`, new `tests/storage/` tests and fixtures generated at test time, the single root registration line required by the P1-011 topology, and this record. The data model and command interfaces are consumed; no other module is changed.

Out of scope: GUI save/open dialogs, encryption/protection, foreign project formats, cloud storage, auto-update, crash-reporting services, Windows execution and any claim that EG-ADD-013 or release-grade persistence is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A versioned project container stores schema metadata, documents, column bindings, dataset versions and provenance with a declared format version | Not run |
| AC-2 | Save is atomic: a successful reopen yields exactly the committed state; a failed save leaves the prior file intact | Not run |
| AC-3 | Interrupted-save injection recovers either the intact prior checkpoint or the committed journal without partial objects; the unsaved recovery window is documented | Not run |
| AC-4 | Reopen preserves typed values, units, null/NaN distinctions, stable IDs, row mappings and formula/DAG references | Not run |
| AC-5 | Schema migration hooks accept a declared older test schema or reject it with a stable diagnostic; corrupted files are detected without executing embedded content | Not run |
| AC-6 | Chunked Arrow IPC data remains bounded; the task records actual file sizes and does not copy full datasets into SQLite blobs | Not run |
| AC-7 | Golden round-trip, corruption, interrupted-save and migration tests pass in the three Qt-free presets; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-006 and created this initial record scaffold; no storage code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No project container, migration or recovery test exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. SQLite and Arrow versions come from P1-003; no ad hoc installation is authorized.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No storage module, SQLite schema or project container exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | The data model, edit commands and conformance plan are not accepted | The task is blocked before implementation |

Checks not run and their reasons: round-trip, atomic-save, interrupted-save, corruption, migration and chunk-bound tests; storage benchmarks. They require the data model, edit commands, P2-001 and authorization.

## Review and disposition

Reviewer and review type: independent storage/recovery review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no native project container exists. The storage contract is in [project file format](../architecture/project-file-format.md) and [data model](../architecture/data-model.md) section 2.

Blockers and unblock actions: P2-002 and P2-004 must provide the data and edit paths; P2-001 must define the persistence failure fixtures. The user must authorize the `modules/storage` module and ensure the build-file lock. SQLite/Arrow versions must come from P1-003.

Next task / concrete next action: after the dependencies, implement the container, atomic save, reopen, migration and recovery, add failure-injection tests, run the three Qt-free presets, and request review. P2-011 measures save/reopen performance.

Files or commands needed to resume: [project file format](../architecture/project-file-format.md), [data model](../architecture/data-model.md), [P2-001 record](p2-001-data-conformance-test-plan.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md), [P2-004 record](p2-004-worksheet-edit-commands-undo.md), [P1-003 record](p1-003-dependency-pins-sbom.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-002/P2-004 and the conformance plan are not accepted |
