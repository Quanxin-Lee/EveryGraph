# P1-016: Reject an Inconsistent Initial Document Snapshot in the Command Gateway

## Metadata

- Task ID: P1-016
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-001
- Dependencies: P1-004
- Created: 2026-11-01
- Updated: 2026-11-01
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-11-01. P1-004 is accepted.
- Authorization: not granted. Required: change the `everygraph::application` gateway construction surface and its direct callers, add the covering tests, and record the runs here. No persistence format change, no dependency installation, no push or publication, and no edit to another task's record is authorized by this task.

## Scope and acceptance

Objective: close a trust-boundary hole in the accepted command kernel. `CommandGateway::CommandGateway(CommandRegistry, DocumentSnapshot, GatewayOptions)` at `modules/application/src/command_gateway.cpp:47-51` moves `initial_state.objects` into `document_` without checking that each map key equals the stored `ObjectState::id`. A caller can therefore construct a gateway whose lookup key and the object's own identity disagree, which contradicts the AGENTS.md section 2 invariant that domain and data keep stable IDs and the section 5 rule that untrusted input is validated at system boundaries.

The hole is latent, not live: `CommandTransaction::create_object` sets `state.id = id` and emplaces under the same `id`, so every mutation path maintains the invariant, and all three current builders key by the object's own id — `tests/application/command_kernel_tests.cpp:350-351` (`emplace(first.id, first)`), `tests/worker/worker_protocol_tests.cpp:287-289` (`snapshot.objects[kTarget] = state`) and `apps/desktop/src/fake_project.cpp:226-238`. What is missing is the guarantee at the boundary that a future loader, import or undo-from-disk path will feed this constructor, which is exactly the moment P2 persistence needs it.

Allowed files and interfaces: `modules/application/include/everygraph/application/document.hpp`, `modules/application/include/everygraph/application/command_gateway.hpp`, `modules/application/src/command_gateway.cpp`, `modules/application/src/document.cpp`, `tests/application/command_kernel_tests.cpp`, `tests/worker/worker_protocol_tests.cpp`, `apps/desktop/src/fake_project.cpp`, `apps/desktop/src/fake_project.hpp` and this record. P0-000 designates this task as the writer for those paths for its next revision; the accepted P1-004, P1-005 and P1-006 claims on them are superseded history. `docs/engineering/development.md` is not in this manifest — request any guide sentence through rule 9.

Out of scope: the persistence format, dataset versioning, any P2 data-model behaviour, the Qt presentation layer, the CI workflow, and any change that alters the scientific semantics of a command.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The mismatch is reproduced as a failing test before the fix — a snapshot keyed under one `ObjectId` holding an `ObjectState` with a different `id` — with the exact assertion output, and the chosen contract (reject with a structured `foundation::Error`, or re-key) is stated with its reason | Not run |
| AC-2 | Construction of an inconsistent snapshot fails through the project's structured-error channel with a stable code and an English developer diagnostic; no exception crosses a C ABI, no silent re-keying that hides the caller's mistake, and no fallback that changes semantics | Not run |
| AC-3 | Every caller of the changed construction surface is updated and listed, proven by a full build of all five macOS presets plus the desktop presets, so AGENTS.md section 5's "check every caller" clause is evidenced rather than asserted | Not run |
| AC-4 | The existing kernel suite stays green with no assertion weakened, no case skipped and no expectation loosened; the 26 prior command-kernel cases still pass and the new cases are additive | Not run |
| AC-5 | `find()`, `snapshot()`, revision checks and event publishing are re-verified to agree on one identity for every object after the change, with the commands and outputs recorded | Not run |
| AC-6 | If the chosen contract changes a public application interface, the architectural consequence is filed as a rule 9 request to the ADR lane naming the target file; this task writes no ADR and no development-guide sentence | Not run |
| AC-7 | `python3 tools/validate_planning.py`, `python3 tools/validate_task_manifest.py` and `ctest --preset macos-debug` are green for this task's paths; an independent reviewer that is not the author records the reviewed revision, findings and recheck; the record moves to `accepted` with a `Board sync` request naming row P1-016 and the proving commit | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-11-01 | P0-000 / allocation executor | Allocated P1-016 from an external review finding the user supplied, verified the finding against `command_gateway.cpp:47-51`, `document.cpp:7-10` and the three current snapshot builders, and created this initial record scaffold; no source file was touched | [Board](../planning/task-board.csv); [P1-004 record](p1-004-command-transaction-kernel.md); [P0-000 record](p0-000-task-allocation.md) |

No code change and no test evidence exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-11-01 | Allocation-time read of the constructor, `DocumentSnapshot::find`, `CommandTransaction::create_object` and the three snapshot builders | Hole is latent, not live | `command_gateway.cpp:47-51` validates nothing; `document.cpp:7-10` looks up by key; `create_object` keeps key and id equal; all current callers key by id |
| 2026-11-01 | Allocation-time search for existing identity validation in the module | None found | Only `validate_envelope` and `validate_expected_revisions` exist at `command_gateway.cpp:53` and `:99`; neither inspects an initial snapshot |

Checks not run and their reasons: the failing-test reproduction, the fix, the preset builds and the event-identity re-verification. They need an executor and this task's manifest.

## Review and disposition

Reviewer and review type: an independent reviewer that is not the author is required before acceptance, and the reviewer must re-run rather than re-read the build evidence. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not started.

## Handoff and remaining work

Last verified state: the constructor accepts any map it is handed; the mutation paths keep the invariant themselves, so no shipped behaviour is wrong today.

Blockers and unblock actions: none outside this task. P1-004 is accepted. The user must authorize the source and test edits. If the executor decides the contract change is architectural, the ADR request goes to the ADR lane by rule 9 and this task stays inside its manifest.

Next task / concrete next action: claim P1-016, correct this scaffold's `planned` status and add the Change manifest as the first write, reproduce the mismatch as a failing test, implement the boundary check, update every caller, and record the independent review and `Board sync` request.

Files or commands needed to resume: [P1-004 record](p1-004-command-transaction-kernel.md), `modules/application/src/command_gateway.cpp`, `modules/application/include/everygraph/application/document.hpp`, `ctest --preset macos-debug`, `cmake --preset macos-release-qt`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-11-01 | New -> planned | Allocated by P0-000 against the external review finding the user supplied; verified as a real trust-boundary gap in the accepted P1-004 interface, and it must land before any P2 loader feeds a snapshot |
