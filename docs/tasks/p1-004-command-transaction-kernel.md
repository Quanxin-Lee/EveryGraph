# P1-004: Prototype the GUI-Independent Command and Transaction Kernel

## Metadata

- Task ID: P1-004
- Status: in_review
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-001; ORG-UI-008; ORG-RECALC-007
- Dependencies: P0-001 (accepted)
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: local `main` at `02648bb` (15 commits ahead of `origin/main`, not pushed), which contains the merged P1-004 checkpoint commits `969842a` (implementation) and `46eb6a4` (record link) via merge `7b6646b`. The former `feat/p1-004-command-kernel` branch and worktree were merged and removed; only `main` remains. This evidence update is committed on `main`; no new branch was created.
- Authorization: implementation was authorized by the user on 2026-10-30 within this task's scope. The 2026-10-31 user instruction authorizes rerunning the AC-7 presets on `main`, updating only this task's record and allowed files, and reporting the results. It also requires no new branches, no cross-task edits, no modification of other task records or board rows, and no push; all were followed.

## Scope and acceptance

Objective: establish the executable application contract that the Qt shell, Python clients, CLI and future AI clients will share. This is a contract prototype with fake in-memory fixtures, not a product domain, persistence layer or user interface.

The prototype must prove that the command envelope, registry, revision checks, transaction boundaries, undo/redo and event sequencing are independent of Qt and of any concrete document format. The shell task P1-006 consumes this contract; it must not invent a second mutation path.

Allowed files and interfaces: new public headers under `modules/foundation/include/everygraph/foundation/` and `modules/application/include/everygraph/application/`, private sources under the matching `src/` directories, a test executable under `tests/`, `CMakeLists.txt`, `CMakePresets.json` only for target/test registration, `docs/engineering/development.md` and this record. No existing business module or UI file is changed.

Out of scope: a real Project/Workbook/Graph domain model, SQLite or Arrow persistence, numerical analysis, Qt widgets, Python bindings, AI providers, plugin loading, a wire-level JSON schema beyond the in-memory prototype, GoogleTest or any downloaded test framework, and any claim that P1 or EG-ADD-001 is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | `everygraph_foundation` and `everygraph_application` targets build with C++20 and contain no Qt includes or Qt types in public headers or implementation | Met: both build as C++20 static libraries in all five presets with no compiler warnings; `module_boundary_check` passes; `otool -L` on `everygraph_command_kernel_tests` lists only `/usr/lib/libc++.1.dylib` and `/usr/lib/libSystem.B.dylib`; `file` reports a Mach-O arm64 executable |
| AC-2 | A versioned command registry rejects unknown commands, duplicate registration, unsupported schema versions and malformed arguments with stable structured error codes | Met: the 26-case suite rejects an unknown command (`unknown_command`), duplicate `demo.set_value` (`duplicate_command`), invalid ID (`invalid_argument`), schema version 2 against version 1 (`unsupported_schema_version`), missing arguments (`invalid_argument`) and external-effect registration (`unsupported_feature`); command IDs require dotted lowercase `namespace.verb` form and definitions declare supported schema versions |
| AC-3 | A command gateway validates expected read/write revisions and rejects stale writes with `revision_conflict` without mutating the fake document | Met: stale write (object 1 expected 0, current 1) and stale read (object 2 expected 0, current 1) return `revision_conflict`; document content, undo depth, event sequence and last-action metadata remain unchanged; expected revisions are revalidated immediately before commit |
| AC-4 | A transaction groups a multi-object fake edit, commits once, emits monotonic sequenced events and creates one undo entry; undo and redo restore the expected fake state | Met: `demo.edit_pair` creates object 3 and updates objects 1/2 in one transaction; one undo entry, one event with sequence 1 and three changes, revisions 2/2/1; the same gateway observes sequences 1, 2 and 3 across commit, undo and redo; undo restores content with revisions 3/3 and removes object 3; redo restores content with revisions 4/4/2 |
| AC-5 | Repeat-last-action and reopen-parameters metadata reproduce the recorded command without duplicating its result; the parameter snapshot is immutable after commit | Met: `demo.set_pair` records immutable canonical parameters; `repeat_last()` re-executes with current expected revisions and produces no new result, revision, event or undo entry because the result is unchanged; `demo.edit_pair` is rejected as `not_repeatable`; mutating the original request or a reopened request does not change the recorded snapshot; executing the reopened request with unchanged parameters is also a no-op |
| AC-6 | The application executor is the only writer; a rejected or canceled candidate publishes no event and no revision | Met: a pre-stopped token, mid-handler cancellation, a handler that stages a change and returns `numerical_failure`, an undeclared write, and a handler exception all leave the document, undo depth and sequence unchanged; a handler that calls back into the gateway receives `reentrant_mutation` and cannot commit an inner candidate; only the gateway swaps a staged candidate into the live document |
| AC-7 | CTest runs the prototype tests plus the existing suites; `macos-debug`, `macos-release` and `macos-sanitize` pass; no Qt is required by these targets | Met on `main` at `02648bb`: each Qt-free preset passed 5/5 after the P0 research artifact was committed; `command_kernel_tests` passed 26/26 directly in Debug, Release and ASan/UBSan; `planning_validation` PASS with 339 original IDs, 428 traced requirements and 48 documents in the final working tree (47 committed plus one concurrent untracked SBOM README); Qt presets also passed 6/6 each. No Qt is linked by the kernel targets |
| AC-8 | The development guide records the targets, exact commands, results and the deliberate dependency-free test-harness limitation; an architecture check demonstrates no Qt dependency in foundation/application | Met: the guide records targets, contract, commands, current 5/5 and 6/6 results, limits and the dependency-free harness; CMake fails configuration if either module links a `Qt<digits>::` target; `module_boundary_check` scans module sources; an injected `#include <QString>` in a scratch tree failed as expected |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit criteria and module contracts; no source, test or build change started | [Takeover audit](p0-002-takeover-state-verification.md), [module design](../architecture/module-design.md) and [board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Implementation / implementer (main agent) | Added foundation IDs/errors and application value, command, document, registry and gateway contracts; added the dependency-free test executable and module boundary check; registered CMake targets/tests; updated the development guide and this record | [CMakeLists.txt](../../CMakeLists.txt), [gateway header](../../modules/application/include/everygraph/application/command_gateway.hpp), [gateway source](../../modules/application/src/command_gateway.cpp), [value source](../../modules/application/src/value.cpp), [tests](../../tests/application/command_kernel_tests.cpp), [boundary check](../../tests/check_module_boundaries.cmake), [development guide](../engineering/development.md) |
| 2026-10-30 | EveryGraph Review / independent review agent | Read-only review of the candidate, CMake, tests, guide and record; found handler reentrancy/stale-commit risk, NaN/signed-zero no-op errors, unenforced read-only effects, exception-safety and coverage gaps, and record inaccuracies | Review findings recorded below |
| 2026-10-31 | EveryGraph Implementation / implementer | Fixed the review findings: execution guard and pre-commit revision revalidation; canonical value equality (all NaNs equal, signed zero distinct); read-only effect rejection; staged tombstone/outcome/listener-snapshot ordering; added handler/listener exception, stop-before-commit, invalid-ID, NaN/signed-zero and read-only tests; suite grew from 19 to 26 | [gateway source](../../modules/application/src/command_gateway.cpp), [value source](../../modules/application/src/value.cpp), [tests](../../tests/application/command_kernel_tests.cpp) |
| 2026-10-31 | EveryGraph Review / second independent review agent | Verified the three substantive findings resolved and the exception-safety mitigations present; manually compiled the current sources with warnings-as-errors (26/26) and ASan/UBSan (26/26), boundary check passed; full presets were blocked by the then-shared worktree's stash conflicts, not by P1-004 code | Verification report recorded below |
| 2026-10-31 | EveryGraph Implementation / implementer | After the P0 research artifact was merged and the workspace returned to `main`, reran the validator and all five presets, confirmed 5/5 and 6/6, and updated only the P1-004 record, its guide section and its board row to request acceptance | Commands and results below |

No background agent or job is running. Both independent review agents completed and returned their reports.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph` on `main` at `02648bb`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in the main repository `.venv`; Qt 6.12.0 is present only for the separate opt-in probe and is not linked by the kernel targets.

Current final-tree runs:

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `cmake --preset macos-debug && cmake --build --preset macos-debug && ctest --preset macos-debug --output-on-failure` | Configured and built with no warnings; CTest 5/5 passed | Final tree on `main` at `02648bb`; all entries passed |
| 2026-10-31 | `cmake --preset macos-release && cmake --build --preset macos-release && ctest --preset macos-release --output-on-failure` | Configured and built with no warnings; CTest 5/5 passed | Release; explicit checks, no assertions |
| 2026-10-31 | `cmake --preset macos-sanitize && cmake --build --preset macos-sanitize && ctest --preset macos-sanitize --output-on-failure` | Configured with ASan + UBSan; CTest 5/5 passed; no sanitizer diagnostics | ASan/UBSan only; no TSan |
| 2026-10-31 | `./out/build/macos-debug/everygraph_command_kernel_tests` | 26/26 cases passed | Direct dependency-free harness run |
| 2026-10-31 | `./out/build/macos-release/everygraph_command_kernel_tests` | 26/26 cases passed | Direct dependency-free harness run |
| 2026-10-31 | `./out/build/macos-sanitize/everygraph_command_kernel_tests` | 26/26 cases passed | Direct dependency-free harness run |
| 2026-10-31 | `QT_ROOT="$(brew --prefix qtbase)" cmake --preset macos-debug-qt && cmake --build --preset macos-debug-qt && ctest --preset macos-debug-qt --output-on-failure` | Configured and built; CTest 6/6 passed, including `qt_probe_smoke` | Local Qt probe only; not packaging |
| 2026-10-31 | `QT_ROOT="$(brew --prefix qtbase)" cmake --preset macos-release-qt && cmake --build --preset macos-release-qt && ctest --preset macos-release-qt --output-on-failure` | Configured and built; CTest 6/6 passed, including `qt_probe_smoke` | Local Qt probe only; no Qt link in kernel targets |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 48 documents in the final working tree (47 committed plus one concurrent untracked SBOM README); input hashes, coverage, source anchors, classifications, local links, fences and English policy; matrix weights total 100 | Remote URL availability, Mermaid rendering, scientific semantics and application behavior not checked |
| 2026-10-31 | `python3 -m unittest discover -s tools -p test_validate_planning.py` | 6/6 validator unit tests passed | Standard library only |
| 2026-10-31 | `otool -L out/build/macos-debug/everygraph_command_kernel_tests`; `file` on the same binary | Only `libc++.1.dylib` and `libSystem.B.dylib`; Mach-O 64-bit arm64 | No dynamic Qt dependency |
| 2026-10-30 | Scratch negative boundary test with `#include <QString>` | Failed as intended and reported the injected file and line | Scratch tree removed; proves the scan's negative path |

Historical blocked-state evidence, retained rather than erased:

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Debug/Release/ASan+UBSan presets in the isolated checkpoint worktree before the P0 artifact was merged | Each 4/5; only `planning_validation` failed on the absent P0 research artifact; kernel 26/26 | This is the blocker that the final-tree rerun resolved |
| 2026-10-30 | Shared worktree before branch isolation | Each 4/5; only `planning_validation` failed on the P1-003 record link to P0-006; Qt presets 5/6 with kernel and `qt_probe_smoke` passing | Branch mismatch and stash-pop artifacts; resolved to the stashed side and preserved the stash |
| 2026-10-31 | `git worktree add out/p1-004-checkpoint feat/p1-004-command-kernel`; `git merge --ff-only fca707a`; `git commit` | Checkpoint commit `969842a` created with 20 P1-004 files; no push | Merged into `main` via `7b6646b`; old branch and worktree removed |

Checks not run and their reasons: Windows execution, CI, TSan, ASan/UBSan combined with the Qt probe, thread/race tests (the executor is single-threaded), long-running cancellation after commit, persistence, permission/capability grants, idempotency/retry storage, external effects, GoogleTest fixtures/death tests and human review. All scoped ACs are met; acceptance is pending the designated acceptor.

## Review and disposition

Reviewer and review type: two independent read-only review agents. The first reviewed the original candidate; the second verified the fixes. Neither is a human review.

Reviewed files / revision: the P1-004 module headers and sources, tests, CMake targets/tests, development guide and record at the working-tree state based on `41b6cf8`, then the fixed state on `fca707a`, now merged into `main`.

Findings, resolutions and recheck evidence:

| Finding | Severity | Resolution | Recheck |
|---|---|---|---|
| Handler reentrancy could commit an inner edit that the outer command then overwrote; no pre-commit revision revalidation | Medium | Added an execution guard covering handler and commit, guarded `undo`/`redo`/`repeat_last`, and revalidated expected revisions immediately before commit | New reentrancy test passes; second review confirmed the finding resolved |
| IEEE `Value` equality treated NaN rewrites as changes and `+0.0`/`-0.0` as equal | Medium | Added canonical `values_equal`/`parameters_equal`: all NaNs equal, signed zero distinct; used by no-op detection and `ObjectState` equality | NaN no-op and signed-zero change tests pass; second review confirmed the finding resolved |
| `EffectClass::read_only` was not enforced | Low | Commit rejects any read-only command that changes the document | New read-only test passes; second review confirmed the finding resolved |
| Tombstones, sequence and event/outcome construction could be ahead of the live document; listener allocation errors could suppress an event | Low | Staged tombstone map, event, outcome and listener snapshot before the document swap; sequence assigned after the swap; listener exceptions contained per callback; commit exceptions converted to `internal_error` | Code inspection and second review; remaining allocation-before-stage limits documented in the guide |
| Coverage gaps: no handler/listener exceptions, stop-after-handler, invalid-ID, double or read-only tests | Low | Added the missing cases; suite grew from 19 to 26 | 26/26 pass in Debug, Release and ASan/UBSan |
| Record claimed a green suite or 19 tests while the tree state was different; branch/worktree state was unstable | Medium | Record rewritten with the actual blocked-state evidence, the final green rerun, 26 tests and the final `main` commit; no unsupported green claim remains | This record; all final-tree runs listed above |

Acceptance decision, accepting role and date: all AC-1 through AC-8 are met, independent review is complete and findings are resolved. The task is submitted as `in_review` for acceptance by the designated accepting role; this record does not self-accept. No human review is claimed.

## Handoff and remaining work

Last verified state: `main` at `02648bb` contains the merged P1-004 implementation and record. The 26-case kernel suite passes in Debug, Release and ASan/UBSan; all three Qt-free presets pass 5/5; both Qt presets pass 6/6; the planning validator passes with 48 documents in the final working tree (47 committed plus one concurrent untracked SBOM README). The old checkpoint branch/worktree no longer exists, and no new branch was created.

Blockers and unblock actions: no P1-004 blocker remains. Acceptance is pending the designated accepting role. P1-006 remains blocked on P1-003's pinned dependency graph; P1-005 is planned and depends on an accepted P1-004.

Next task / concrete next action: the accepting role reviews this record and the committed artifacts. If accepted, P1-004 becomes `accepted` and P1-005 may start. No push or merge is authorized.

Files or commands needed to resume: [CMakeLists.txt](../../CMakeLists.txt), [gateway header](../../modules/application/include/everygraph/application/command_gateway.hpp), [gateway source](../../modules/application/src/command_gateway.cpp), [tests](../../tests/application/command_kernel_tests.cpp), [boundary check](../../tests/check_module_boundaries.cmake), [development guide](../engineering/development.md), and the five presets from the validation table.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> planned | Second takeover decomposition; implementation not authorized |
| 2026-10-30 | planned -> in_progress | User authorized implementation, testing, review and necessary fixes within the task scope; branch created locally |
| 2026-10-30 | in_progress -> in_review | Foundation/application prototype, tests, boundary check and documentation completed; independent review requested |
| 2026-10-31 | in_review -> blocked | Independent review findings were resolved and rechecked, but AC-7 could not pass while committed P0 records linked to an absent research artifact |
| 2026-10-31 | blocked -> in_review | The P0 research artifact was committed on `main`; the validator and all five presets were rerun green; acceptance is requested without self-acceptance |
