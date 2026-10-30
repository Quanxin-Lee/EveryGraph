# P0-010: Record macOS-first sequencing in the residual documentation

## Metadata

- Task ID: P0-010
- Status: accepted
- Owner: EveryGraph Lead
- Record writer: EveryGraph Lead / P0-010 executor (Qoder main-agent session, 2026-10-31)
- Related requirements: none; phase-gate governance and evidence verification
- Dependencies: P0-008
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: repository `main`; P0-010 began at `1646259` with P0-000's uncommitted board and record updates, and concurrent P0-009 and P1-003 commits advanced HEAD from `1646259` to `9b366ce` during execution. The P0-010 commit is created on top of `9b366ce`; a user-authorized coordination snapshot for the other pending task records and board rows follows it. Uncommitted P1-003 and P2-001 files remain owned by those tasks. P0-010 works in the existing working tree only; no branch, worktree or stash was created.
- Authorization: the user instructed execution of P0-010 on 2026-10-31, delegated necessary research, implementation and independent review, prohibited a new branch, and instructed the task record and P0-010 board row to be updated after completion. Authorized: edit `README.md`, `docs/planning/development-roadmap.md`, `docs/engineering/validation-report.md`, this record and the P0-010 board row; run the planning validator and read-only checks. Not authorized: product code, build changes, dependency changes, register or catalog-contract edits, other task records or board rows, Windows execution, Git commit or push, signing, publication, or any support claim.

## Scope and acceptance

Objective: remove the pre-decision platform wording that remains in README, the development roadmap and the validation report and make them consistent with AGENTS, the functional scope, the module design and ADR-0005.

Allowed files and interfaces: `README.md`, `docs/planning/development-roadmap.md`, `docs/engineering/validation-report.md` and this record. AGENTS.md, the functional scope, the module design, ADR-0005, every other board row and every other task record are read-only; only the P0-010 board row may be synced under the user's instruction.

Out of scope: changing product scope, requirements, phases or tasks; Windows implementation or evidence; product code; build changes; push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | README, the roadmap and the validation report state macOS Apple Silicon as the only active qualification target and Windows x64 as a deferred later gate with platform interfaces retained. | Met: README lines 3 and 16, the roadmap status line, P1 row, sequencing paragraph, P3/P6 rows and next-work item 1, and the validation report introduction, Windows-preset and runtime rows and section 4 gates state the macOS-only active target, the deferred Windows gate with retained interfaces and no Windows support claim. Round 1 and round 2 independent read-only reviews confirmed the statements in context, including the corrected evidence wording |
| AC-2 | The roadmap P1 and P3 exit rows, and its immediate-next-work list, no longer claim current dual-platform qualification and remain consistent with the board's deferred P1-002 and P1-010 and the macOS P1-013 exit. | Met: the P1 exit names P1-002/P1-010 as the deferred Windows gate and P1-013 as the macOS exit; the P3 exit and next-work item 1 are macOS-scoped with Windows replication deferred; the final board check shows P1-002 and P1-010 `blocked` as deferred and P1-013 `blocked` with macOS-only acceptance. Both reviews PASS |
| AC-3 | No requirement, phase or capability is removed; Windows remains a tracked later target and no Windows support is claimed. | Met: the diff replaces platform wording only; phases P0-P7, the 428-row register, Windows 10/11 later targets, the Windows risk and every capability list remain; no Windows build, evidence or support claim is introduced. Both reviews PASS |
| AC-4 | `python3 tools/validate_planning.py` passes with links, fences and English policy intact. | Met: the final `python3 tools/validate_planning.py` reports 339 original IDs, 428 traced requirements and 90 documents with input hashes, coverage, source anchors, classifications, local links, fences and English policy valid, and matrix weights total 100; the six planning-validator tests pass. The first run reported 89 documents; a concurrent P0-012 research document was added afterwards |
| AC-5 | Only the allowed documents and this record are edited. | Met: the final document diff covers only README (4/4), the validation report (7/6) and the roadmap (8/6); this record and only the P0-010 board row are the remaining P0-010 edits. Round 2 confirmed that other modified and untracked files pre-date P0-010 and belong to other tasks |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-010 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | User / project owner | Instructed execution of P0-010 on the existing branch, delegated necessary research, implementation and independent review, prohibited a new branch, and authorized the record and P0-010 board row updates | Direct user instruction |
| 2026-10-31 | EveryGraph Lead / P0-010 executor | Verified dependencies, read the allowed documents, the board rows P1-002/P1-010/P1-013, AGENTS, the functional scope, the module design and ADR-0005; surveyed residual dual-platform wording; edited only README, the development roadmap and the validation report; ran the validator, the planning-validator tests, a residual-wording scan and a board consistency check | [README](../../README.md); [roadmap](../planning/development-roadmap.md); [validation report](../engineering/validation-report.md); validation table below |
| 2026-10-31 | Independent reviewer (round 1, delegated read-only Qoder subagent) | Reviewed AC-1 to AC-5 against the working tree, the record, the board rows and the validator output; reported all five PASS with no blocking findings, four should-fix items and notes; made no edits or commits | [Review and disposition](#review-and-disposition) |
| 2026-10-31 | EveryGraph Lead / P0-010 executor | Resolved the four should-fix findings: added ADR-0005 to the validation report's deliverable inventory and corrected it to 38 items/27 Markdown; clarified the record's read-only scope; corrected the inspection and AC-1 evidence wording; added the stale P1-013 record to the residual list. Also clarified the README release-claim sentence and the roadmap risk evidence | Validation table below |
| 2026-10-31 | Independent reviewer (round 2, new delegated read-only Qoder subagent) | Rechecked AC-1 to AC-5 and the four should-fix resolutions; reported all five PASS and all four RESOLVED, no blocking findings, two low severity record-only notes (HEAD advanced to `b7f2cb3`; the document count is now 90) and one nit; made no edits or commits | [Review and disposition](#review-and-disposition) |
| 2026-10-31 | EveryGraph Lead / P0-010 executor (main agent as shared-board writer) | Applied the round-2 record corrections, synced only the P0-010 board row to `accepted`, and ran the final validator, board and tree checks; no other board row or task record was edited | This record; [board](../planning/task-board.csv) |
| 2026-10-31 | User / project owner | Authorized committing the other pending board rows and task records in addition to P0-010, removing the earlier exclusion constraint | Direct user instruction |
| 2026-10-31 | EveryGraph Lead / P0-010 executor | Prepared the authorized local commit on top of `9b366ce`: README, the roadmap, the validation report, this record and only the P0-010 board row; the follow-up coordination snapshot is prepared separately under the user's authorization | Commit scope note below; `git log` |

This is the accepted execution record; acceptance covers P0-010 only and does not accept the P0 exit gate.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The task is a documentation or contract correction and does not build product code.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source and dependency inspection | Confirmed that README, the development roadmap and the validation report still contain pre-decision platform wording; P0-008 is accepted and committed as `2b1169d`. | Read-only check; not a test of the task |
| 2026-10-31 | Read-only inspection of `README.md`, `docs/planning/development-roadmap.md`, `docs/engineering/validation-report.md` and board rows P1-002/P1-010/P1-013 | Confirmed dual-platform wording at README lines 3/16/47, roadmap lines 35/37/40/61 and validation-report section 3 rows 36/39 and section 4 rows 48/49; the report introduction had no platform statement before this task; confirmed the board defers P1-002/P1-010 and narrows P1-013 to macOS | The board has 56 rows with 56 unique IDs; no edit yet |
| 2026-10-31 | Scope-checked edits and `git diff --numstat -- README.md docs/planning/development-roadmap.md docs/engineering/validation-report.md` | README 4 insertions/4 deletions; validation report 5/5; roadmap 7/5. All replacements preserve phases, capabilities and requirement coverage and add explicit deferral and interface-retention statements | Only the three allowed documents were edited at this stage |
| 2026-10-31 | `python3 tools/validate_planning.py` after the edits | PASS: 339 original IDs; 428 traced requirements; 89 documents at that run; input hashes, coverage, source anchors, classifications, local links, fences and English policy; matrix weights total 100 | The extra documents are other tasks' records and plans in the current tree; a concurrent P0-012 research document later raised the count to 90. Remote URLs, Mermaid rendering and scientific semantics are not checked |
| 2026-10-31 | `python3 -m unittest discover -s tools -p "test_validate_planning.py" -v` | Ran 6 tests; OK; exit 0 | Planning-validator regression only; the catalog-child suite is outside this task and was not run |
| 2026-10-31 | `rg -n -i "both platforms\|both OSes\|on both OS\|macOS and Windows\|Windows and macOS\|x64 Windows\|Mac and x64\|both operating systems\|both-platform\|dual-platform" README.md docs/planning/development-roadmap.md docs/engineering/validation-report.md` | No matches (`NO_DUAL_PLATFORM_CLAIMS`) | Word-pattern scan, not a semantic review; the independent reviewer must confirm the statements in context |
| 2026-10-31 | Read-only Python check over `docs/planning/task-board.csv` for P1-002, P1-010 and P1-013 | P1-002 and P1-010 are `blocked` and explicitly deferred; P1-013 is `blocked` with a macOS-only exit and no Windows claim; 56 rows, 56 unique IDs | Does not execute the tasks or prove their future evidence |
| 2026-10-31 | `git status --short`; `git diff --numstat` for the edited documents | This task modified only README, the roadmap and the validation report; the other listed modifications and untracked files pre-date P0-010 and belong to other tasks | Record and board edits are pending; no commit or push was performed |
| 2026-10-31 | Independent round-1 read-only review of AC-1 to AC-5, the working tree, the record, the board and the validator output | AC-1 to AC-5 PASS; no blocking findings; four should-fix items (deliverable inventory omitted ADR-0005; record read-only scope contradicted the board-row authorization; record misstated the validation-report pre-edit lines and AC-1 evidence; residual list omitted the stale P1-013 record) and notes | Reviewer made no edits or commits; the findings were applied by the executor |
| 2026-10-31 | Re-ran `python3 tools/validate_planning.py` and `python3 -m unittest discover -s tools -p "test_validate_planning.py" -v` after the review fixes | PASS: 339 original IDs; 428 traced requirements; 90 documents in the current tree; 6 tests OK; matrix weights total 100 | The added ADR-0005 inventory line and link are included in the link/fence/English check; the 90th document is the concurrent P0-012 research file, not a P0-010 edit |
| 2026-10-31 | Read-only inventory recount over `docs/engineering/validation-report.md` section 5 | 38 listed deliverables; 27 Markdown; `../decisions/ADR-0005-platform-sequencing.md` present | Confirms the corrected 38/27 counts and ADR-0005 entry |
| 2026-10-31 | `git diff --check`; `git diff --numstat` for the three documents | No whitespace errors; README 4/4, validation report 7/6, roadmap 8/6 | Final document edit sizes |
| 2026-10-31 | Independent round-2 read-only recheck of AC-1 to AC-5, the four should-fix resolutions, the working tree and the validator output | AC-1 to AC-5 PASS; all four should-fix items RESOLVED; no blocking findings; two low record-only notes (HEAD advanced to `b7f2cb3`; document count 90) and one nit, all corrected in this record | Reviewer made no edits or commits |
| 2026-10-31 | Final `python3 tools/validate_planning.py`; read-only board check; `git diff -U0 -- docs/planning/task-board.csv`; `git status --short` for the five P0-010 files | Validator PASS: 339 original IDs, 428 traced requirements, 90 documents, all checks green; the working-tree board has 56 rows/56 unique IDs, every record path exists and every dependency resolves; P0-010 is `accepted` and only its row changed; P0-010's tree changes are README, the roadmap, the validation report, the board row and this record | Final working-tree state. The P0-010 commit adds only the P0-010 row to the 34 committed task rows (35 lines). Under the user's separate authorization, a follow-up coordination snapshot records the full 56-row working board and the other pending task documents; those are not P0-010's own changes. See the commit-scope note below. No build or Windows execution was performed |

Checks not run and their reasons: CMake/CTest builds and application tests (documentation-only task); Windows execution (deferred and unauthorized); the catalog-child validator suite (governed by P0-009 and outside this task's allowed files).

## Review and disposition

Reviewer and review type: independent read-only reviews by delegated Qoder general-purpose subagents with no edit or commit authority. Rounds 1 and 2 are complete. The reviewers are not the record writer and made no repository changes.

Reviewed files / revision: `main` at `b7f2cb3` plus the P0-010 working-tree edits for rounds 1 and 2 (P0-010 began at `1646259`; concurrent P0-009 commits advanced HEAD to `b7f2cb3`, and later P1-003 commits advanced it to `9b366ce` without touching P0-010 files); README, the development roadmap, the validation report, this record, the board rows P0-010/P1-002/P1-010/P1-013, AGENTS sections 1-2, the functional scope, the module design, ADR-0005, the validator output and read-only Git status/diff.

Findings, resolutions and recheck evidence (round 1):

| Finding | Severity | Resolution | Recheck |
|---|---|---|---|
| The validation report's delivered-file inventory said 37 items/26 Markdown and omitted ADR-0005 although the post-edit report links it and ADR-0005 is a Phase-0 decision deliverable | Should-fix | Added ADR-0005 to the decisions list and corrected the inventory to 38 items/27 Markdown, with a note that later task artifacts are tracked on the board | Read-only inventory recount: 38 items, 27 Markdown, ADR-0005 present |
| The record's allowed-files line called the board and every task record read-only, contradicting the authorized P0-010 board-row sync | Should-fix | Clarified that every other board row and every other task record are read-only and only the P0-010 row may be synced | This record |
| The record misstated the validation report's pre-edit platform lines as including the introduction and overstated what the section 3 rows state | Should-fix | Corrected the inspection row to section 3 rows 36/39 and section 4 rows 48/49, noted the introduction had no platform statement before this task, and corrected the AC-1 evidence wording | This record |
| The residual list omitted the stale P1-013 record, which still requires Windows x64 evidence and dependencies although the board row is macOS-only | Should-fix | Added `docs/tasks/p1-013-p1-exit-verification.md` to the out-of-scope residual list for P0-000/P1-013 | This record |
| README "These are not tested release claims" was ambiguous | Note | Reworded to "No qualification target is a tested release claim" | [README](../../README.md) line 16 |
| The roadmap risk table said "Cross-platform scene/export corpus" in a macOS-only phase | Note | Reworded to the macOS corpus now with the cross-platform corpus when the Windows gate resumes | [Roadmap](../planning/development-roadmap.md) risk table |
| Board P0-010 was `planned` while the record was `in_progress`; acceptance should wait for the row sync | Note | Expected pre-sync state; the row is synced only at acceptance | Final tree check |

Round 1 verdict: AC-1 to AC-5 PASS with no blocking findings.

Round 2 findings, resolutions and evidence (new independent read-only reviewer):

| Finding | Severity | Resolution | Recheck |
|---|---|---|---|
| The record's base/review revision said `1646259` while concurrent P0-009 commits had advanced HEAD to `b7f2cb3` | Low | Updated the base/revision text and the review section; P0-010 made no commit | `git log` and `git rev-parse HEAD` |
| The record and handoff said 89 documents while the validator now reports 90 because a concurrent P0-012 research document was added | Low | Updated the AC-4 result, validation rows and handoff to 90 with the 89-at-first-run note | Final validator output |
| The round-1 rationale said the pre-edit report linked ADR-0005; the pre-edit report had no such link | Nit | Corrected to "the post-edit report links it and ADR-0005 is a Phase-0 decision deliverable" | This record |

Round 2 verdict: AC-1 to AC-5 PASS; all four round-1 should-fix items RESOLVED; no blocking findings. Only record-accuracy corrections remained and are applied above.

Acceptance decision, accepting role and date: accepted by EveryGraph Lead / P0-010 executor (main agent) on 2026-10-31 after two independent read-only reviews, the final validator PASS, the board consistency check and the P0-010 board-row sync. All five acceptance criteria are met. Acceptance covers P0-010 only; the P0 exit gate remains subject to P0-007/P0-000 re-verification, and the out-of-scope residuals remain in the handoff.

## Handoff and remaining work

Last verified state: P0-010 is accepted on 2026-10-31. README, the development roadmap and the validation report state macOS Apple Silicon as the only active qualification target and Windows x64 as a deferred later gate with platform interfaces retained. The final planning validator passes with 339 original IDs, 428 traced requirements and 90 documents; the six planning-validator tests pass; board rows P1-002/P1-010 are deferred and P1-013 is the macOS exit; only the P0-010 board row was synced to `accepted`. Two independent read-only reviews passed all five criteria. The P0-010 local commit and the user-authorized coordination snapshot are described in the commit-scope note below.

Blockers and unblock actions: none for P0-010. Residual out-of-scope platform wording is reported for P0-000 allocation, not edited here: `docs/planning/technology-evaluation.md`, `docs/engineering/development.md`, `docs/engineering/performance-validation.md`, `docs/architecture/rendering-architecture.md` and the stale P1-013 record at `docs/tasks/p1-013-p1-exit-verification.md` (still describes Windows x64 evidence and dependencies although the board row is macOS-only).

Next task / concrete next action: P0-007/P0-000 may re-verify the P0 gate now that no pre-decision platform wording remains in the three documents; P0-000 owns allocation of the residual platform-wording cleanup and the P1-013 record reconciliation. No push or publication was performed by P0-010.

Commit scope: the P0-010 local commit contains README, the development roadmap, the validation report, this record and only the P0-010 row of `docs/planning/task-board.csv`. On 2026-10-31 the user separately authorized committing the other pending board rows and task records. A follow-up coordination snapshot therefore records the remaining working-tree allocation and plan documents (the full 56-row board, the P0-000 and P2-001 record updates and the newly allocated task records/plans); that snapshot is outside P0-010's own scope and does not change P0-010's acceptance.

Files or commands needed to resume: [functional scope](../planning/functional-scope.md), [ADR-0005](../decisions/ADR-0005-platform-sequencing.md), [P0-008 record](p0-008-macos-first-platform-sequencing.md), [P0-007 record](p0-007-p0-exit-verification.md), [roadmap](../planning/development-roadmap.md), [validation report](../engineering/validation-report.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user requested execution on the existing branch, delegated the necessary work and independent review, prohibited a new branch and authorized the record and P0-010 board row updates; the executor verified P0-008's accepted dependency and the current board before editing. |
| 2026-10-31 | in_progress -> in_review | The edits and validator checks were complete and round 1 independent read-only review was requested. |
| 2026-10-31 | in_review -> accepted | Round 2 independent recheck confirmed AC-1 to AC-5 PASS and all round-1 findings resolved; the final validator, planning tests, board consistency check and scope-limited diff passed, and only the P0-010 board row was synced to `accepted`. |
