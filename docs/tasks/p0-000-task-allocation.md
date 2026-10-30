# P0-000: Allocate Task IDs and Maintain the Authoritative Task Board

## Metadata

- Task ID: P0-000
- Status: in_progress
- Owner: EveryGraph Coordination
- Record writer: P0-000 current executor (agent label per session)
- Related requirements: none; project coordination and task-allocation governance
- Dependencies: none
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not tied to one branch. The current repository is on `feat/p1-001-qt-baseline` at `41b6cf8`, with concurrent P1-004 work on `feat/p1-004-command-kernel`. This record was committed as `8f403e4`; its board row remains uncommitted with the shared board because the board also contains concurrent owner changes.
- Authorization: the user established P0-000 on 2026-10-30 as the standing task-allocation authority. It may allocate new task IDs and board rows, maintain its own record, and verify board consistency. It must not edit another owner's task record, execute other tasks or implement product work. No installation, commit, push or publication is authorized.

## Scope and acceptance

Objective: provide one authoritative allocation point for new task IDs and task-board rows, independent of which agent executes P0-000. The task board plus this record are the real-time source of truth; chat messages, agent memory and branch names are not.

P0-000 is a standing coordination task. It may be executed by different agents over time. Each executor must read the current board, this record and the target task record before allocating anything, and must record its own agent/session label in the Contributions table.

Allowed files and interfaces: this record and the task-board rows P0-000 allocates or maintains. P0-000 may add a board row for a new task and update its own P0-000 row. Each new task's record is written and maintained by its assigned owner, not by P0-000.

Out of scope: product implementation, executing another task, accepting another owner's task, editing another owner's task record, dependency installation, Windows/CI execution, signing, commit, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Every new task ID and board row is allocated by P0-000 with owner, dependencies, testable acceptance and an existing record path; no duplicate or reused ID | Met for the initial establishment: P0-000 created; the P0-006 eight-row batch is recognized as the first allocated batch |
| AC-2 | P0-000 can change executors; each executor records its agent/session label and reads the current board and this record before acting | Met: the rule is defined here; the current executor is the extra-agent session that created this record |
| AC-3 | The board and this record are the authoritative real-time task state; allocations are followed by CSV/metadata and planning-validator checks | Met: 17 board rows pass the schema/record check; the planning validator passes with 46 documents |
| AC-4 | P0-000 never edits another owner's task record; it only allocates rows and maintains its own record | Met: no other owner's record was edited by this task; the earlier P1-001/P0-003 process correction remains recorded in P0-006 |
| AC-5 | P0-000 does not implement product work or claim another task's acceptance | Met: allocation and documentation only |
| AC-6 | New allocations state file/interface boundaries and dependencies and do not create a speculative task tree | Met: the policy is defined; future allocations must follow it |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | User / project owner | Established P0-000 as the standing task-allocation authority; future task IDs are allocated through it | Direct user instruction |
| 2026-10-30 | EveryGraph Verification / current P0-000 executor | Created this record and the P0-000 board row; added the allocation-authority rule to AGENTS.md; recognized the P0-006 eight-row batch as the initial allocation | [Board](../planning/task-board.csv), [p0-006](p0-006-extra-agent-verification.md), [AGENTS.md](../../AGENTS.md) |
| 2026-10-30 | EveryGraph Verification / committer | Committed this record and P0-006 as `8f403e4`, after AGENTS.md `54a8a7b`; did not commit the shared board or other owners' files | Git log |

The P0-006 batch (P0-004, P0-005, P1-005 to P1-010) was allocated before P0-000 was established. It is recognized as the initial batch. Future allocations, including new P1/P2/P3 task IDs, must go through P0-000.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. Environment: macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Homebrew Qt 6.12.0_2 for the P1-001 probe. Concurrent P1-004 work is active.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `python3 tools/validate_planning.py` | Passed: 339 original IDs; 428 traced requirements; 46 documents | Includes this record; checks links, fences and English policy, not task semantics |
| 2026-10-30 | Board schema/record check, 17 rows | Passed: schema, unique IDs, dependencies, requirement references, record paths and metadata agree | P0-000 row added; no duplicate IDs |
| 2026-10-30 | Existing test suites | Last verified in P0-006: three Qt-free presets 3/3 and two Qt presets 4/4 | No source or build change in this task; not rerun for a documentation-only allocation |
| 2026-10-30 | `git status --short` | P1-004 record and `modules/` untracked; concurrent P0-002/P0-003/board changes present | This record and P0-006 were committed as `8f403e4`; the shared board remains uncommitted until concurrent owner records are committed |

Checks not run and their reasons: product builds, Windows execution, CI, dependency resolution and independent review of other tasks. P0-000 is a coordination record, not a product capability.

## Allocation rules for future executors

1. Read the current board, this record and the target task record before allocating.
2. Check that the task is not already allocated, that its dependencies are identified and that its file/interface boundary is explicit.
3. Allocate the next unused phase-prefixed ID; never reuse or renumber an existing ID.
4. Add one board row with owner, dependencies, testable acceptance and an existing record path.
5. Leave the new task record to its assigned owner unless the user explicitly assigns the record to the current executor.
6. Do not edit another owner's task record, and do not change another task's status without its owner.
7. Run the CSV/record consistency check and `python3 tools/validate_planning.py` after allocation.
8. Record the executor's agent/session label and the allocation in the Contributions table.

## Review and disposition

Reviewer and review type: the user established P0-000; this record is a self-review by the current executor. It is a standing coordination task, not a product acceptance.

Reviewed files / revision: the task board, P0-006, AGENTS.md and the current task records. Concurrent task records were read only.

Findings, resolutions and recheck evidence: the board had 16 rows before P0-000; it now has 17. P0-006's eight allocated rows remain valid but future allocations must use P0-000.

Acceptance decision, accepting role and date: P0-000 is `in_progress` as a standing task. It is not closed or accepted; the user may reassign its executor at any time.

## Handoff and remaining work

Last verified state: P0-000 is established with a board row and this record. The board is authoritative for current task state. P0-001/P0-002/P0-003/P1-001 are accepted; P1-004 is in progress; P1-002/P1-003 are blocked; P0-004/P1-005 are planned; P0-005/P1-006 to P1-010 are blocked; P0-006 is in review.

Blockers and unblock actions: P0-000 itself has no blocker. Its future executors must not treat chat history as state; they must read the board and this record. P1-003 still needs its owner to update the record/status and perform the authorized vcpkg work.

Next task / concrete next action: the next agent taking P0-000 reads the board and this record, then allocates only the task the user/coordinator requests. P1-004 continues under its owner; P1-003 awaits its owner; P0-006 awaits user/coordinator acceptance.

Files or commands needed to resume: [task board](../planning/task-board.csv), [recording guide](README.md), [P0-006](p0-006-extra-agent-verification.md), [AGENTS.md](../../AGENTS.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> in_progress | User established P0-000 as the standing task-allocation authority; it may be executed by different agents |
| 2026-10-30 | Recognized P0-006's eight-row batch as the initial allocation | P0-006 was completed before P0-000 existed; future allocations use P0-000 |
| 2026-10-30 | Committed this record | Committed as `8f403e4`; the shared board and other owners' records remain uncommitted |
