# P1-011: Refactor the Build Topology into Per-Module Targets

## Metadata

- Task ID: P1-011
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: none; build and test governance, not a new product capability
- Dependencies: P1-003; P1-004
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. P1-003 is `in_progress` and currently owns `CMakeLists.txt`, `CMakePresets.json`, `docs/engineering/development.md` and the SBOM files.
- Authorization: not granted. Required: edit the root and module CMake files, create per-module CMake files, update the development guide and this record, and run the five macOS presets. No target behavior change, new module, dependency change, Qt version change, Windows execution, push or publication.

## Scope and acceptance

Objective: remove the single-file build bottleneck that makes otherwise independent tasks edit one root `CMakeLists.txt`. The root file keeps shared options, the no-Qt linkage policy, sanitizer policy and `add_subdirectory` registration; each module and test group owns its target definitions in a local CMake file. This is a prerequisite for safe parallel module work under the exclusive-ownership rule.

Allowed files and interfaces: `CMakeLists.txt`, `CMakePresets.json` only if a preset change is unavoidable, new `modules/foundation/CMakeLists.txt`, new `modules/application/CMakeLists.txt`, new `tests/CMakeLists.txt` and test-group CMake files, a new `tools/CMakeLists.txt` only if it preserves the existing probe target, [development guide](../engineering/development.md) and this record.

Out of scope: changing target names, tests, compiler flags, sanitizer policy, dependency versions, Qt version, source behavior or module public interfaces; creating empty production modules; Windows execution; packaging; push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Foundation, application, probe and test target definitions move into per-module/test CMake files; the root file contains only shared policy and subdirectory registration | Not run |
| AC-2 | Target names, linked libraries, compile features, warnings and sanitizer behavior are unchanged; the command kernel and toolchain probe binaries are identical in role | Not run |
| AC-3 | The missing `EVERYGRAPH_ENABLE_QT_PROBE` option declaration is restored with a default of `OFF`; the Qt presets still enable it explicitly | Not run |
| AC-4 | `macos-debug`, `macos-release`, `macos-sanitize`, `macos-debug-qt` and `macos-release-qt` configure, build and pass CTest with no new warnings | Not run |
| AC-5 | The development guide documents the per-module CMake convention and the root-file ownership rule; no other module or task file is changed | Not run |
| AC-6 | The planning validator and module boundary check pass; no Qt type enters the foundation/application public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P1-011 and created this initial record scaffold; no build change started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No CMake refactor, target move or preset change exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Qt 6.12.0 from the P1-003 pinned graph when that route is available.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | The root `CMakeLists.txt` still defines all targets directly and uses `EVERYGRAPH_ENABLE_QT_PROBE` without an `option()` declaration | Confirms the task is not started; P1-003 currently owns the file |
| 2026-10-31 | Allocation-time `python3 tools/validate_planning.py` | Passed before the allocation records were added; post-allocation state is in the [P0-000 record](p0-000-task-allocation.md) | Documentation baseline only |

Checks not run and their reasons: the CMake refactor, all five preset builds, the boundary check and review. They require P1-003 acceptance and an assigned executor.

## Review and disposition

Reviewer and review type: independent build/architecture review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked on P1-003 and is not authorized.

## Handoff and remaining work

Last verified state: the build topology is monolithically defined in the root [CMakeLists.txt](../../CMakeLists.txt). P1-004 added kernel targets there; P1-003 is currently editing the same file. The `EVERYGRAPH_ENABLE_QT_PROBE` option is used but not declared.

Blockers and unblock actions: P1-003 must be accepted and its uncommitted build-file changes committed by its owner. The user must authorize the refactor and assign one executor. Do not start while P1-003 owns the shared build files.

Next task / concrete next action: after P1-003 acceptance, move target definitions into per-module/test CMake files, restore the option, run the five presets, and request independent review. P1-005 and P1-006 must start after this task because they add targets.

Files or commands needed to resume: [CMakeLists.txt](../../CMakeLists.txt), [CMake presets](../../CMakePresets.json), [module design](../architecture/module-design.md), [development guide](../engineering/development.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-004 record](p1-004-command-transaction-kernel.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 to remove the shared build-file bottleneck; P1-003 is in progress and owns the file |
