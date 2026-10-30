# P0-000: Allocate Task IDs and Maintain the Authoritative Task Board

## Metadata

- Task ID: P0-000
- Status: in_progress
- Owner: EveryGraph Coordination
- Record writer: P0-000 current executor (Qoder main-agent session, 2026-10-31)
- Related requirements: none; project coordination and task-allocation governance
- Dependencies: none
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: repository `main` at the consolidation tip `7e4bc8e`, based on `origin/main` at `747809e`. The task branches were merged as `fca707a` (fast-forward, P1-001), `c5ad88b` (P1-003) and `7b6646b` (P1-004), then deleted together with their checkpoint worktrees; no stash remains. The newest worktree records and research documents were integrated in `26f391e`. No push occurred.
- Authorization: the user instructed on 2026-10-31 to consolidate all branches and task state, use the newest files, dispose of the branches, and then execute P0-000. The same instruction prohibits modifying other owners' work files or records. The user also directly instructed that only the task executor may modify its own task files, that no other agent may do so even with user authorization, and that each agent is responsible for only one task; that rule is now section 0 of AGENTS.md. Local commits and branch/worktree/stash cleanup are authorized; push, remote publication and acceptance of another owner's task are not.

## Scope and acceptance

Objective: provide one authoritative allocation point for new task IDs and task-board rows, independent of which agent executes P0-000. The task board plus this record are the real-time source of truth; chat messages, agent memory and branch names are not.

P0-000 is a standing coordination task. It may be executed by different agents over time. Each executor must read the current board, this record and the target task record before allocating anything, and must record its own agent/session label in the Contributions table.

Allowed files and interfaces: this record and the task-board rows P0-000 allocates or maintains. P0-000 may add a board row for a new task and update its own P0-000 row. It may also update the top-level exclusive task-ownership text in AGENTS.md when the user directly requests that governance change; that is a shared-file edit, not a cross-task edit. Each new task's record is written and maintained by its assigned owner, not by P0-000.

Out of scope: product implementation, executing another task, accepting another owner's task, editing another owner's task record, dependency installation, Windows/CI execution, signing, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Every new task ID and board row is allocated by P0-000 with owner, dependencies, testable acceptance and an existing record path; no duplicate or reused ID | Met for the current board: 17 rows, 17 unique IDs, every record present, all owners/dependencies/acceptance populated. No new ID was requested in this execution, so none was allocated |
| AC-2 | P0-000 can change executors; each executor records its agent/session label and reads the current board and this record before acting | Met: the executor label and the pre-work reads are recorded in Contributions; the standing rule is unchanged |
| AC-3 | The board and this record are the authoritative real-time task state; allocations are followed by CSV/metadata and planning-validator checks | Met: the 17-row board passes schema, unique-ID, record-path, status and dependency checks; `python3 tools/validate_planning.py` passes with 47 documents; this record and its row were updated together |
| AC-4 | P0-000 never edits another owner's task record; it only allocates rows and maintains its own record | Met for this execution: P0-000 edited only this record and its own board row. The preceding main-agent consolidation briefly edited P1-003/P1-004 records; the user prohibited that, the edits were reverted, and the final files match the owner branch versions exactly |
| AC-5 | P0-000 does not implement product work or claim another task's acceptance | Met: this execution performed coordination, Git consolidation, verification and documentation only. It did not accept P1-003, P1-004 or P0-005 |
| AC-6 | New allocations state file/interface boundaries and dependencies and do not create a speculative task tree | Met: no new task was allocated; the existing 17-row tree is unchanged in scope |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | User / project owner | Established P0-000 as the standing task-allocation authority; future task IDs are allocated through it | Direct user instruction |
| 2026-10-30 | EveryGraph Verification / current P0-000 executor | Created this record and the P0-000 board row; added the allocation-authority rule to AGENTS.md; recognized the P0-006 eight-row batch as the initial allocation | [Board](../planning/task-board.csv), [P0-006](p0-006-extra-agent-verification.md), [AGENTS.md](../../AGENTS.md) |
| 2026-10-30 | EveryGraph Verification / committer | Committed this record and P0-006 as `8f403e4`, after AGENTS.md `54a8a7b`; did not commit the shared board or other owners' files | Git log |
| 2026-10-31 | EveryGraph Coordination / current P0-000 executor | Read the live board and all task records; inventoried three branches, two checkpoint worktrees and two stashes; merged P1-001, P1-003 and P1-004 into `main`; integrated the newest worktree records and research documents; deleted the merged branches, worktrees and stashes; reran the validator, board checks and five macOS presets; updated the P0-000 row and this record | Commits `fca707a`, `c5ad88b`, `7b6646b`, `26f391e`, `7e4bc8e`; [board](../planning/task-board.csv); this record |
| 2026-10-31 | EveryGraph Coordination / current P0-000 executor | Ownership correction: reverted unauthorized edits to the P1-003/P1-004 records and restored the P1-004 build and guide files to their owner versions before P0-000 execution | Revert `7e4bc8e`; file hashes match the branch tips |
| 2026-10-31 | EveryGraph Coordination / current P0-000 executor | Added the exclusive task-ownership rule as section 0 of AGENTS.md at the user's direct request; no other task record or artifact was edited | [AGENTS.md](../../AGENTS.md); this record |
| 2026-10-31 | User / project owner | Confirmed narrowing P0-005 AC-3 to vendor-evidenced built-in/App labels, with the unlabelled remainder recorded as an explicit unknown | Direct user instruction |
| 2026-10-31 | EveryGraph Research / P0-005 executor | Updated and committed the P0-005 acceptance record under the narrowed AC-3; validator passes with 48 documents and no CJK | `12f1adc`; [P0-005](p0-005-catalog-evidence-gaps.md) |
| 2026-10-31 | EveryGraph Coordination / current P0-000 executor | Verified the P0-005 acceptance evidence and updated the board row to `accepted`; did not edit the P0-005 record | [Board](../planning/task-board.csv) |

The P0-006 batch (P0-004, P0-005, P1-005 to P1-010) was allocated before P0-000 was established. It is recognized as the initial batch. Future allocations, including new P1/P2/P3 task IDs, must go through P0-000.

The consolidation committed the newest worktree versions of the P0-002, P0-003 and P0-005 records and both research documents. Their content was not rewritten by P0-000; it is the owners' own newest content. The P1-003 and P1-004 records are their branch versions.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. Environment: macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Homebrew Qt 6.12.0 for the opt-in probe; Python 3.13.9. The repository is on `main`; no branch, worktree or stash remains.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `python3 tools/validate_planning.py` | Passed: 339 original IDs; 428 traced requirements; 47 documents; input hashes, coverage, links, fences, English policy and matrix weights all pass | Checks documents, not task semantics or remote URLs |
| 2026-10-31 | CSV/record consistency script over `docs/planning/task-board.csv` | Passed: 17 rows; 17 unique IDs; all record paths exist; all statuses valid; all dependencies resolve; no cycles checked by inspection | Custom read-only check; not a repository test |
| 2026-10-31 | `cmake --preset macos-debug` / `macos-release` / `macos-sanitize`, with build and `ctest --output-on-failure` | Each 5/5 passed, including `planning_validation`; no sanitizer diagnostics | Final tree after ownership restoration; not a wiped build |
| 2026-10-31 | `QT_ROOT="$(brew --prefix qtbase)" cmake --preset macos-debug-qt` and `macos-release-qt`, with build and `ctest` | Each 6/6 passed, including `command_kernel_tests` and `qt_probe_smoke` | Final tree after ownership restoration |
| 2026-10-31 | `git branch -a`; `git worktree list`; `git stash list`; `git status --short` | Only `main`; no extra worktrees; no stashes; clean working tree | `main` is ahead of `origin/main` and was not pushed |
| 2026-10-31 | MD5 comparison of the P1-003/P1-004 records and P1-004 build/guide files against their branch tips | Exact match | Confirms no residual edits to other owners' files |
| 2026-10-31 | `python3 tools/validate_planning.py` after the P0-005 acceptance | Passed: 339 original IDs; 428 traced requirements; 48 documents; no CJK | P0-005 record/evidence; concurrent P1-003 changes remained uncommitted and outside P0-000 |

Checks not run and their reasons: the vcpkg `arm64-osx-dynamic` graph, Windows execution, CI, push/remote publication, and independent review of the consolidation. P0-000 is coordination, not a product capability or acceptance of another task.

## Review and disposition

Reviewer and review type: self-review by the current executor only. No independent reviewer was assigned. The user directed the consolidation and prohibited edits to other owners' files.

Reviewed files / revision: the task board, this record, the three branch tips, the two checkpoint worktrees, both stashes, the final `main` tree and the validation outputs. Other task records were read only.

Findings, resolutions and recheck evidence:

- P1-004 was accepted by its owner on 2026-10-31 after AC-7 reran green on the consolidated tree (`77c3693`). P0-000 did not edit its record. A residual build-file finding remains: the P1-004 `CMakeLists.txt` lacks the P1-001 `EVERYGRAPH_ENABLE_QT_PROBE` option declaration; the Qt presets pass because they set the cache variable.
- P1-003 remains `in_progress`; its shared-worktree/stash blocker is resolved, and its owner has uncommitted SBOM and development-guide changes. Its dynamic graph and SBOM evidence remain open.
- P0-005 was accepted by EveryGraph Research on 2026-10-31 under the user-authorized AC-3 narrowing (`12f1adc`); the unlabelled graph remainder and the other catalog gaps are explicit. P0-000 updated the board row to `accepted` and did not edit the P0-005 record.
- The P1-004 branch's `CMakeLists.txt` lacks the `EVERYGRAPH_ENABLE_QT_PROBE` option declaration present in P1-001. The Qt presets still pass because they set the cache variable, but the default option is undefined outside the presets. The P1-004 owner should restore it; P0-000 did not edit the file.
- The P1-004 version of `docs/engineering/development.md` still says `planning_validation` is red because P0-006 is absent. That caveat is stale on `main`; its owner should update it.
- The consolidation briefly edited the P1-003/P1-004 records and P1-004 build/guide files. The user prohibited editing other owners' files; the edits were reverted in `7e4bc8e`, and the final hashes match the branch versions.

Acceptance decision, accepting role and date: P0-000 remains `in_progress` as a standing coordination task; it is not closed or accepted. This execution produced no new task allocation and no acceptance of another task.

## Handoff and remaining work

Last verified state: `main` contains all three task branches and the newest worktree records; the worktree is clean, no extra branch or stash remains, and the validator, board checks and all five macOS presets pass. The pre-cleanup backup is outside the repository at `/tmp/everygraph-20261031-precleanup-tree.tar.gz` and `/tmp/everygraph-20261031-precleanup-refs.bundle`.

Current board states: P0-001, P0-002, P0-003, P0-005, P0-006, P1-001 and P1-004 are accepted; P0-000 and P1-003 are in progress; P0-004 and P1-005 are planned; P1-002, P1-006, P1-007, P1-008, P1-009 and P1-010 are blocked.

Owner actions before further progress:

1. P1-003 owner: commit its own SBOM and documentation changes; the dynamic graph and SBOM evidence remain open.
2. P0-005 owner: no further acceptance action; the P0-003 owner still decides whether to retain the pre-rule section 15 append in the P0-003 evidence document.
3. P1-005 owner: its P1-004 dependency is satisfied, but it must not run concurrently with P1-003 because both write `CMakeLists.txt` and `docs/engineering/development.md`; sequence them or assign a single build-file writer.
4. P0-004 owner: planned and startable, but before starting it must remove the board and the P0-003 evidence document from its writable scope under AGENTS.md section 0, or obtain an explicit reassignment from the P0-003 owner.

Blockers and unblock actions: P0-000 itself has no blocker. The standing task can be executed by another agent after re-reading the live board and this record. Remote push remains unauthorized.

Next task / concrete next action: the task owners update their own records from the verified consolidated state; P0-000 allocates only what the user or coordinator requests. No push or publication.

Files or commands needed to resume: [task board](../planning/task-board.csv), [recording guide](README.md), [P0-006](p0-006-extra-agent-verification.md), [AGENTS.md](../../AGENTS.md), [development guide](../engineering/development.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> in_progress | User established P0-000 as the standing task-allocation authority; it may be executed by different agents |
| 2026-10-30 | Recognized P0-006's eight-row batch as the initial allocation | P0-006 was completed before P0-000 existed; future allocations use P0-000 |
| 2026-10-30 | Committed this record | Committed as `8f403e4`; the shared board and other owners' records remained uncommitted |
| 2026-10-31 | Repository and branch consolidation executed; no status change | User directed consolidation of all branches and task state, use of the newest files, and P0-000 execution; push remains unauthorized |
| 2026-10-31 | Ownership constraint recorded | User prohibited modifying other owners' work files; the brief unauthorized edits were reverted and P0-000 edited only its own record and row |
| 2026-10-31 | Top-level ownership rule added to AGENTS.md; no task status change | Direct user instruction; the rule makes exclusive task ownership mandatory and prohibits cross-task edits even with user authorization |
| 2026-10-31 | P1-004 accepted by its owner; no P0-000 edit | AC-7 reran green on the consolidated tree and the owner recorded acceptance in `77c3693` |
| 2026-10-31 | P0-005 accepted under narrowed AC-3; board row synced | User narrowed AC-3; the P0-005 executor recorded acceptance in `12f1adc`; P0-000 updated the board row and did not edit the record |
