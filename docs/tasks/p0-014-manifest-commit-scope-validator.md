# P0-014: Validate change manifests and commit file scope

## Metadata

- Task ID: P0-014
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: none; governance work that enforces AGENTS.md section 0 rules 7 to 12
- Dependencies: none
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31
- Authorization: not granted. Required: create only the manifest paths below, run them with the repository Python, and record evidence. No CMake or CI change, no dependency installation, no Windows execution, no edit to another task's record or to the shared board, no push or publication.

## Scope and acceptance

Objective: turn AGENTS.md section 0 rules 7 to 12 from prose into a checked invariant, so a missing manifest, two tasks claiming one path, a manifest that claims a shared normative path, and a commit that lands outside its own manifest all fail visibly instead of being discovered later as a mixed commit.

Allowed files and interfaces: advisory only. The change manifest below is the writable set (AGENTS.md section 0 rule 7).

Change manifest:

| Path | Action | Basis |
|---|---|---|
| `tools/validate_task_manifest.py` | create | Own task deliverable |
| `tools/test_validate_task_manifest.py` | create | Own task deliverable |
| `docs/tasks/p0-014-manifest-commit-scope-validator.md` | modify | Own task record |

Out of scope: registering the checker in CMake or CTest (that path is contested by P1-011 and P0-011 and needs a request through rule 9), a GitHub Actions change, editing `docs/engineering/development.md`, rewriting any existing task record to add its manifest, changing the board schema, and any product code.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The checker reads `docs/planning/task-board.csv` and every `docs/tasks/p*.md` record with the repository Python and no network access, and exits non-zero with per-task diagnostics. | Not run |
| AC-2 | It fails when a task whose board status is `in_progress`, `in_review` or `accepted` has no change manifest, and passes for `planned`, `blocked` and `canceled` rows without one. | Not run |
| AC-3 | It fails when two non-`canceled` manifests claim the same path, and names both tasks. | Not run |
| AC-4 | It fails when a manifest claims a shared normative path (`AGENTS.md`, the task board, the feature traceability register, root `CMakeLists.txt`, `CMakePresets.json`, the development guide, the licensing guide, `.github/workflows/*`, `tools/validate_planning.py`) unless the basis column names the designated writer; the board path always fails for a non-P0-000 task. | Not run |
| AC-5 | A commit-scope check reports every committed path outside the committing task's manifest, plus every commit subject without exactly one task ID; run against the current `main` history it reproduces the recorded findings (post-rule-7 board writes and the unscoped commits) rather than silently passing. | Not run |
| AC-6 | Unit tests cover each failure mode and are picked up by `python3 -B -m unittest discover -s tools -p 'test_*.py'` without a new runner. | Not run |
| AC-7 | `python3 tools/validate_planning.py` still passes; no other task's record, the board or a build file is modified. | Not run |
| AC-8 | The checker flags a task whose record says `accepted` while its Review section names no reviewer other than the implementing executor, or while the task has no commit — the two ways section 0 rule 11 self-acceptance can be claimed without evidence. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-014 and created this initial record scaffold with its change manifest; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | P0-000 / allocation executor | Before any executor was assigned, added AC-8 and the matching board acceptance clause so the checker also catches a self-acceptance claim with no independent reviewer or no commit, per AGENTS.md section 0 rule 11 | [AGENTS.md](../../AGENTS.md) section 0 rules 11 and 12 |

This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: not run.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|

Checks not run and their reasons: no executor is assigned, so nothing above has been run.

## Review and disposition

Reviewer and review type (independent or self-review): none yet.
Reviewed files / revision: none.
Findings, resolutions and recheck evidence: none.
Acceptance decision, accepting role and date (or pending): pending.

## Handoff and remaining work

Commits: none; this scaffold is committed by P0-000 with the rule change.
Last verified state: AGENTS.md section 0 rules 7 to 12 and the [task guide](README.md) describe the manifest, the ownership tiers, the request channel and the pathspec commit form; no tool enforces them yet.
Blockers and unblock actions: none. The executor must add this record's manifest confirmation and move the scaffold `Status: planned` to `in_progress` at start.
Next task / concrete next action: implement the checker, then report which existing tasks lack a manifest so their own executors can add them.
Files or commands needed to resume: `docs/planning/task-board.csv`, `docs/tasks/*.md`, `git log --name-only`, `python3 -B -m unittest discover -s tools -p 'test_*.py'`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | Created as `planned` | P0-000 allocation after the user directed a manifest and commit-scope rule |
