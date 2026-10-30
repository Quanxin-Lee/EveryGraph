# P2-002: Implement the Typed Data Model and Immutable Dataset Versions

## Metadata

- Task ID: P2-002
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-DATA-001; ORG-DATA-011; ORG-DATA-012; DG-DATA-005; DG-DATA-006; EG-ADD-002; EG-ADD-008
- Dependencies: P1-013; P2-001
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. P1-003 is in progress; the P1 exit gate and the P2 conformance plan are not accepted.
- Authorization: not granted. Required: create `modules/data` public and private sources, `tests/data` tests, module-local CMake registration and this record. No UI, persistence, formulas, Excel import, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement the in-memory scientific data core that later import, formula, view and persistence tasks consume. The core owns stable object identities, typed columns, immutable dataset versions, row-identity domains, units and provenance edges. It is GUI-independent and does not expose Qt, Arrow object graphs or storage handles in public interfaces.

Allowed files and interfaces: new `modules/data/include/everygraph/data/` and `modules/data/src/` sources, new `modules/data/CMakeLists.txt`, new `tests/data/` tests, the single root `add_subdirectory(modules/data)` registration line required by the P1-011 topology, and this record. The command kernel from P1-004 is consumed through its public interfaces. Other modules and documents are read-only.

Out of scope: import parsers, worksheet edit commands, formulas, masks, SQLite/Arrow persistence, virtual-table UI, plotting, Windows execution and any claim that ORG-DATA-001 or the P2 phase is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Project, Workbook, Worksheet, ColumnBinding, DatasetVersion and Chunk types use stable opaque IDs; no array index or translated name is an identity | Not run |
| AC-2 | Numeric, text, date and IEEE binary columns are typed; missing, NaN, infinite, empty text, zero and signed zero remain distinct and round-trip through the in-memory API | Not run |
| AC-3 | Units and dimensions convert compatible values and reject incompatible arithmetic; offset units are covered; no silent unit drop occurs | Not run |
| AC-4 | A new edit creates an immutable dataset version with provenance edges; prior versions remain readable and unchanged; row identity survives column operations | Not run |
| AC-5 | Foreign-domain column binding or mirroring requires an explicit keyed/positional mapping and reports unmatched rows; unrelated columns are never zipped by array position | Not run |
| AC-6 | Command-driven create/append operations publish revisions through the P1-004 kernel; no direct mutation path exists outside the executor | Not run |
| AC-7 | Golden tests pass in `macos-debug`, `macos-release` and `macos-sanitize`; the module boundary check passes; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-002 and created this initial record scaffold; no data-model code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No module, header, test or CMake target exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. The data core must remain Qt-free and must use the P1-003 dependency graph if it needs Arrow or another pinned library.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | Only the P1-004 foundation and application prototypes exist; no data module, dataset version or typed-column implementation exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | The P1 exit gate and the P2 conformance plan are not accepted | The task is blocked before implementation |

Checks not run and their reasons: data-model tests, immutable-version tests, units tests, row-identity tests, boundary checks and preset runs. They require the P1 exit gate, P2-001 and authorization.

## Review and disposition

Reviewer and review type: independent data-architecture review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no data module exists. The design contracts are [data model](../architecture/data-model.md) and [module design](../architecture/module-design.md); P2-001 defines the conformance plan.

Blockers and unblock actions: P1-013 must be accepted (or explicitly gated by the user) and P2-001 must define the fixtures. The user must authorize `modules/data` and a single build-file writer; do not run concurrently with another task that edits the root `CMakeLists.txt`. Arrow/SQLite versions come from P1-003, not from ad hoc installs.

Next task / concrete next action: after the gate and plan, create the typed data core with golden tests, run the three Qt-free presets and the boundary check, and request independent review. P2-003, P2-004, P2-006 and P2-007 consume this core.

Files or commands needed to resume: [data model](../architecture/data-model.md), [module design](../architecture/module-design.md), [project file format](../architecture/project-file-format.md), [functional scope](../planning/functional-scope.md), [P1-004 record](p1-004-command-transaction-kernel.md), [P1-011 record](p1-011-build-topology-per-module.md), [P1-013 record](p1-013-p1-exit-verification.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; the P1 exit gate and P2 conformance plan are not accepted |
