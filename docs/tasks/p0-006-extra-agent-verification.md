# P0-006: Independent Extra-Agent Verification and Next-Batch Proposal

## Metadata

- Task ID: P0-006
- Status: accepted
- Owner: EveryGraph Verification
- Record writer: EveryGraph Verification (extra agent)
- Related requirements: none; independent verification and coordination proposal, not a product capability
- Dependencies: none
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: working tree on branch `feat/p1-001-qt-baseline` at commit `41b6cf8`, based on `747809e`; the P1-004 owner has concurrently created `feat/p1-004-command-kernel` from `41b6cf8`. This record and the allocated candidate records were committed as `8f403e4`; AGENTS.md was committed as `54a8a7b`. The shared task board remains uncommitted because it contains concurrent P0-002/P0-003 status changes and the P1-004 row whose record is still untracked. P0-006 does not own the P1-004 branch or its implementation.
- Authorization: the user requested an additional parallel verification task and explicitly authorized allocating a new task ID, adding its record and board row, allocating the non-conflicting candidate batch, and the P1-003 vcpkg/network scope. No edits to other owners' task records, no product implementation by this task, no installs, Windows execution, CI dispatch, commit, push or publication are authorized.

## Scope and acceptance

Objective: independently verify the current project state from files and executed checks, separate delivered evidence from plans and blockers, and propose a scoped next batch for the coordinator. This is an extra-agent view; it does not accept, reopen or rewrite any other task.

Allowed files and interfaces: this record, the P0-006 board row, and the board allocation of the candidate rows named below. The candidate task records are drafts owned by their named owners; this task does not edit their content. All other task records, planning registers, engineering documents, source code and build files are read-only.

Out of scope: editing P0-002, P0-003, P1-001, P1-002, P1-003 or P1-004 records; accepting any other task; product implementation; Qt/vcpkg installation; Windows execution; CI dispatch; signing; commits, pushes or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Read AGENTS.md, board, recording guide, functional scope, traceability, roadmap, validation report and relevant task/architecture records | Met: all listed sources were read; concurrent changes and stale-record limits are stated in this record |
| AC-2 | Verify current artifacts, Git state and executed checks rather than relying on historical records | Met: current Git log, worktree status, validator, unit tests and five CTest presets were checked on 2026-10-30 |
| AC-3 | Classify completed with evidence, active but not accepted, planned but not started, blocked, and in scope but not decomposed | Met: classification below; no planning claim is treated as implementation |
| AC-4 | Propose only a scoped next batch with stable candidate IDs, dependencies, acceptance and order; no project-wide empty task tree | Met: the user approved board allocation on 2026-10-30. P1-004 was concurrently allocated and started by its owner, so this task added the remaining eight rows (P0-004, P0-005, P1-005 to P1-010) without duplicating P1-004 |
| AC-5 | Recommend one first task and name the user authorizations it needs | Met: P1-003 is recommended; the user granted its vcpkg/network authorization, which is recorded here for its owner to apply |
| AC-6 | Do not edit another owner's task record in the final state | Met with a disclosed process correction: this agent initially edited P1-001 and P0-003, then reverted those edits after user correction; final state has no edit by this task to those records |
| AC-7 | This record and its board row agree; the planning validator and existing tests remain green | Met: 16 board rows parse and agree with their records; validator passes with 45 documents; all existing suites pass as recorded below |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Verification / extra-agent verifier | Read standards, board, guide, scope, traceability, roadmap, validation report, architecture contracts and active P0/P1 records; ran read-only checks | Validation section below |
| 2026-10-30 | EveryGraph Verification / extra-agent planner | Drafted candidate task records `p0-004`, `p0-005` and `p1-005` to `p1-010`; `p1-004` was concurrently taken over and started by its owner | Candidate records in `docs/tasks/`; P1-004 owner record |
| 2026-10-30 | EveryGraph Verification / board allocator | After user approval, added eight non-conflicting rows (P0-004, P0-005, P1-005 to P1-010) to the board; did not duplicate or edit the active P1-004 row | [Board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Verification / record writer | Created this record and the P0-006 board row only; did not change other board rows | [Board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Verification / boundary correction | Initially edited the P1-001 and P0-003 records without ownership; reverted the P1-001 added rows and restored P0-003 to its pre-edit text after the user corrected the role | P1-001 now matches its owner's committed version; P0-003 retains its owner's accepted record |
| 2026-10-30 | EveryGraph Verification / documentation contributor | Added the user-requested parallel-agent isolation clause to AGENTS.md before the boundary correction | [AGENTS.md](../../AGENTS.md), task-record section |
| 2026-10-30 | User / project owner | Confirmed completion and accepted the P0-006 verification/proposal | Direct user confirmation |
| 2026-10-30 | EveryGraph Verification / committer | Committed AGENTS.md as `54a8a7b` and P0-000/P0-006 plus the candidate task records as `8f403e4`; did not commit other owners' files or the shared board | Git log |

User decision recorded on 2026-10-30: the candidate batch was approved for board allocation, and the P1-003 vcpkg/network scope was authorized. P1-004 was already allocated and started by its owner, so the extra agent did not add a duplicate row, did not edit its record and did not claim its work. The P1-003 authorization is recorded here, not in the P1-003 record; its owner must update that record and board row when starting.

The candidate records remain drafts. Their owners must reconcile them with current dependencies, authorization and scope before execution. The coordinator may keep, rewrite, reassign or delete them. Board allocation is not technical acceptance and does not grant execution permission.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. Observed environment: macOS 27.0.1 build 26A434, arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Homebrew Qt 6.12.0_2 installed for the P1-001 probe. Concurrent agents may change files after this snapshot.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `python3 tools/validate_planning.py` | Passed: 339 original IDs; 428 traced requirements; 45 documents | Includes this record and the candidate records; checks links, fences and English policy, not task semantics or remote URLs |
| 2026-10-30 | `python3 -B -m unittest discover -s tools -p test_validate_planning.py -v` | Passed: 6/6 | Validator regression tests only |
| 2026-10-30 | `ctest --preset macos-debug`, `macos-release`, `macos-sanitize` | Each 3/3 passed | Qt-free toolchain and planning suites |
| 2026-10-30 | `QT_ROOT="$(brew --prefix qtbase)" ctest --preset macos-debug-qt`, `macos-release-qt` | Each 4/4 passed | P1-001 Qt probe; not a product application |
| 2026-10-30 | `git log --oneline -5` | `41b6cf8`, `d226fa3`, `747809e` | P1-001 implementation and record-link commits exist locally |
| 2026-10-30 | `git status --short` | P0-002, P0-003 and board modified; P1-004 record and `modules/` untracked | This task committed its own records as `8f403e4` and AGENTS.md as `54a8a7b`; concurrent P1-004 work and the shared board remain uncommitted |
| 2026-10-30 | Board consistency check, 16 rows | Passed: schema, unique IDs, dependencies, requirement references, record paths and metadata agree | Includes the eight newly allocated rows; does not validate semantic acceptance or authorization |
| 2026-10-30 | `brew list --versions qtbase` | `qtbase 6.12.0_2` | Qualification installation only; not a pinned dependency graph |

Checks not run and their reasons: implementation builds beyond the existing probes, Windows execution, CI runs, vcpkg resolution, numerical/rendering correctness, signing/notarization and independent review of the candidate batch. Those belong to their owners and require separate authorization.

## Current-state classification

Snapshot: 2026-10-30, after P1-001 commits `d226fa3` and `41b6cf8` and the concurrent P1-004 start. Other agents may update their records after this snapshot.

### 1. Completed with evidence

- **P0-001** is accepted and documents the task-record governance.
- **P0-002** is accepted by its owner; its second-round record reconciles the takeover state. It is coordination evidence, not product implementation.
- **P0-003** is accepted by its owner. The candidate evidence document contains 620 lines and 149 proposed `EG-CAT-*` child IDs; the 428-row register is unchanged. Acceptance is for the research proposal, not catalog completeness.
- **P1-001** is accepted by its owner with commits `d226fa3` and `41b6cf8`. It qualifies a minimal Qt 6.12.0 Widgets probe on macOS arm64, offscreen tests, dynamic deployment and an LGPL replacement experiment. It is not the EveryGraph workbench, not a Windows result and not a release artifact.
- The Phase-0 planning/architecture/research corpus and the dependency-free compiler probe are present; the validator and toolchain suites pass.
- The 428-row feature register remains 387 `planned` and 41 `unresolved`; no product capability is implemented.

### 2. Active

- **P1-004** is `in_progress` under its owner. The owner created `feat/p1-004-command-kernel` and an untracked `modules/` tree. This extra agent did not implement, edit or accept that work.

### 3. Planned but not started

- **P0-004** catalog-register integration depends on P0-003 (accepted) and remains planned.
- **P1-005** worker handshake depends on P1-004 and remains planned.
- **P1-006 to P1-010** are allocated but blocked by dependencies, credentials or a Windows host.

### 4. Blocked by environment, permission or evidence

- **P1-003** dependency pins/SBOM: P1-001 is accepted and the user granted the vcpkg/network scope on 2026-10-30; the board row is still `blocked` until its owner starts and updates it. vcpkg is not yet installed.
- **P1-002** Windows Qt baseline: requires a Windows x64 host or authorized CI, plus P1-003.
- **P0-005** catalog evidence closure: lawful versioned fitting/peak catalogs, graph inventory and the seventeenth export format are unavailable.
- **P1-006** shell, **P1-007** preferences, **P1-008** accessibility/display, **P1-009** signed packaging and **P1-010** Windows shell are blocked by P1-003, credentials, a Windows host or their predecessor tasks.

### 5. In scope but not decomposed into development tasks

- P2-P7 implementation families, the 41 `unresolved` register rows, the remaining catalog/rights gaps, numerical method catalogs, proprietary compatibility gates, AI/plugin/reporting/animation work and large-scale performance qualification.
- This task does not create that full tree. It allocates only the next P0/P1 batch.

## Allocated next batch and order

The eight rows added by P0-006 are board-allocated drafts. P1-004 is not one of them; it was allocated and started by its owner. The coordinator and owners must reconcile each draft before execution.

| Order | Task | Board status | Depends on | Purpose |
|---|---|---|---|---|
| 1 | [P1-003](p1-003-dependency-pins-sbom.md) | blocked (authorization granted; owner must start) | P1-001 (accepted) | Pin the native dependency graph and produce an SBOM; critical path to Qt shell and Windows |
| 2 | [P1-004](p1-004-command-transaction-kernel.md) | in_progress (owner) | P0-001 | GUI-independent command/transaction kernel; already active |
| 3 | [P1-005](p1-005-worker-handshake-cancellation.md) | planned | P1-004 | Worker handshake, cancellation and resource governance |
| 4 | [P1-006](p1-006-workbench-shell-document-host.md) | blocked | P1-001; P1-003; P1-004 | Qt workbench shell, project tree and document host |
| 5 | [P1-007](p1-007-preferences-appearance-focus-mode.md) | blocked | P1-006 | Preferences, dark appearance and focus mode |
| 6 | [P1-008](p1-008-accessibility-display-input.md) | blocked | P1-006; P1-007 | Accessibility, IME, high-DPI and multi-monitor qualification |
| 7 | [P1-009](p1-009-macos-packaging-lgpl.md) | blocked | P1-003; P1-006 | Signed, hardened, notarized macOS test bundle and LGPL replacement |
| 8 | [P1-010](p1-010-windows-shell-smoke.md) | blocked | P1-002; P1-006; P1-007; P1-008 | Replicate the shell smoke suite on Windows x64 |
| 9 | [P0-004](p0-004-catalog-register-integration.md) | planned | P0-003 (accepted) | Separate catalog-child register and validator integration |
| 10 | [P0-005](p0-005-catalog-evidence-gaps.md) | blocked | P0-003 (accepted) | Close fitting/peak/graph/export evidence gaps or keep them explicitly unresolved |

Ordering rationale: P1-003 is the only remaining P1 dependency blocker after P1-001 acceptance. P1-004 is already active. The UI chain follows the pinned Qt graph. P0-004 follows P0-003 acceptance; P0-005 stays blocked until lawful sources exist.

## Recommended first task and authorizations

Recommended first task: **P1-003, Pin the native dependency graph and produce an initial SBOM**, assigned to its existing owner EveryGraph Implementation. The user granted the authorization on 2026-10-30; the owner can start after updating its own record and board row.

Required user authorizations (granted and recorded here; not a substitute for the owner's record update):

1. Bootstrap/install vcpkg (or an explicitly approved equivalent) and allow network downloads of package sources/binaries.
2. Authorize `vcpkg.json`, any `vcpkg-configuration.json`, `CMakePresets.json`/`CMakeLists.txt` dependency wiring, documentation updates and an `docs/engineering/sbom/` artifact.
3. Authorize a clean-state configure/build/test and recording of resolved versions, licenses and SBOM from actual artifacts.
4. Do not authorize Windows resolution, signing, publication, commit or push as part of this task; those remain separate.

P1-004 is already in progress under its owner, so it is no longer a parallel alternative for this extra agent. P1-005 remains the next Qt-free follow-up after P1-004.

## Review and disposition

Reviewer and review type: this is an extra-agent self-review of a read-only verification and proposal. It is not an independent review of P0-003, P1-001, P1-004 or any other task. The coordinator and the assigned owners must review the allocated draft batch before execution.

Reviewed files / revision: standards and planning sources, the task board, the accepted P0-003/P1-001 records and the active P1-004 record at the working-tree revision. Candidate task records are drafts.

Findings, resolutions and recheck evidence: P0-003 and P1-001 are accepted by their owners; P1-004 is active and must not be duplicated; P1-003 is authorized but still needs its owner's record/status update; the candidate records must be reconciled by their owners. The process violation (editing P1-001/P0-003) was corrected by reverting the edits; the final state is recorded above.

Acceptance decision, accepting role and date: accepted by the user (project owner) on 2026-10-30 after confirming the extra-agent work is complete. This accepts the P0-006 verification and proposal only; it does not accept P0-003, P1-001, P1-004 or any allocated task, and it does not claim independent review of those tasks.

## Handoff and remaining work

Last verified state: P0-003 and P1-001 are accepted; P1-004 is in progress under its owner; P1-002/P1-003 remain blocked on the board; P1-005 to P1-010 and P0-004/P0-005 are now board-allocated drafts. No product application exists. All existing validator and CTest suites pass.

Blockers and unblock actions:

- P1-003: the user granted vcpkg/network authorization; its owner must update the record/status and perform the clean-state resolution.
- P1-002: provide a Windows x64 host or authorized CI after P1-003.
- P0-005: lawful vendor catalogs or an explicit unresolved decision.
- P1-006 to P1-010: depend on the pinned Qt graph and/or credentials/Windows hosts.
- The allocated draft records need owner reconciliation before execution.

Next task / concrete next action: the P1-003 owner starts the authorized dependency pin and SBOM work; the P1-004 owner continues the active kernel prototype; the coordinator decides whether to accept or revise the allocated draft batch. The extra agent does not start any of these tasks.

P0-000 was established on 2026-10-30 as the standing task-allocation authority. This P0-006 eight-row batch is recognized as the initial allocation; future task IDs and board rows must be allocated through P0-000. P0-000 may be executed by different agents, so the current board and the P0-000 record are the authoritative real-time state.

Files or commands needed to resume: [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-004 record](p1-004-command-transaction-kernel.md), [P0-003 evidence](../research/capability-catalog-decomposition.md), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), and the existing presets.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> in_review | Extra-agent verification and proposal completed; user authorized a new task ID and its own record/board row |
| 2026-10-30 | User approved candidate batch allocation and P1-003 vcpkg/network scope | Direct user answer to the P0-006 proposal |
| 2026-10-30 | P1-004 concurrently allocated and started by its owner | Independent parallel work; P0-006 did not duplicate, edit or accept it |
| 2026-10-30 | Boundary correction | The agent initially edited P1-001/P0-003 without ownership; edits were reverted after user correction; no further edits to those records |
| 2026-10-30 | P0-000 established as the standing task-allocation authority | User instruction; the P0-006 eight-row batch is recognized as the initial allocation and future allocations must use P0-000 |
| 2026-10-30 | in_review -> accepted | User confirmed completion on 2026-10-30; acceptance covers the P0-006 verification/proposal only |
| 2026-10-30 | Committed own records and governance rule | AGENTS.md `54a8a7b`; P0-000/P0-006 and candidate records `8f403e4`; shared board deferred until concurrent owner records are committed |
