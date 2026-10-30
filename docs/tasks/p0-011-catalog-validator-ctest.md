# P0-011: Wire the catalog-child validator into CTest

## Metadata

- Task ID: P0-011
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: none; phase-gate governance and evidence verification
- Dependencies: P1-011;P0-009
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: register the catalog-child validator and its tests as a first-class CTest test after the P1-011 build-topology refactor, so the catalog contract cannot regress unnoticed.

Allowed files and interfaces: the root `CMakeLists.txt` or the per-module build file created by P1-011, the new CTest registration file if needed, `docs/engineering/development.md` and this record. The catalog data, contract, register, other task records and product code are read-only.

Out of scope: changing catalog expectations or rows, fixing the re-pin (P0-009), a new CI service, dependency installation, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The catalog-child validator and the full tool test suite run as a named CTest test using the repository Python and no network access. | Not run |
| AC-2 | `ctest --preset macos-debug` includes the catalog test and passes; the same passes for `macos-release` and `macos-sanitize`. | Not run |
| AC-3 | A catalog failure fails CTest with a non-zero result; a missing Python or missing fixture is an explicit failure, never a silent skip. | Not run |
| AC-4 | `docs/engineering/development.md` documents the test name, what it protects and the prerequisite. | Not run |
| AC-5 | The planning validator and board check pass; no catalog data or other task record is edited. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-011 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

This record is an allocation plan, not completed work.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The catalog-child suite is currently reachable only by direct unittest invocation.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source and dependency inspection | Confirmed that P1-011 is blocked and the catalog-child suite is not registered in CMake or CTest; it is reachable only by direct unittest invocation. | Read-only check; not a test of the task |

Checks not run and their reasons: the corrective edit, post-edit validator and test runs. They require an executor and authorization.

## Review and disposition

Reviewer and review type: independent review is required for this task. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not authorized.

## Handoff and remaining work

Last verified state: The target files are unchanged and the residual described in the objective is present.

Blockers and unblock actions: P1-011 and P0-009 must be accepted, and the user must authorize the build-file edit and a single writer for the root CMake file.

Next task / concrete next action: After P1-011 and P0-009 are accepted, add the CTest registration, run the three presets, record the results and request review. The catalog suite then becomes part of the normal gate.

Files or commands needed to resume: [P1-011 record](p1-011-build-topology-per-module.md), [P0-009 record](p0-009-catalog-contract-repin.md), [development guide](../engineering/development.md), [board](../planning/task-board.csv).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; P1-011 and P0-009 are not accepted. |
