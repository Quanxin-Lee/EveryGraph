# P0-008: Record the macOS-first platform sequencing decision

## Metadata

- Task ID: P0-008
- Status: accepted
- Owner: EveryGraph Lead
- Record writer: EveryGraph Lead / P0-008 executor (Qoder main-agent session, 2026-10-31)
- Related requirements: none; sequencing decision that affects EG-ADD-016 and DG-PERF-004 without changing the 428-row register
- Dependencies: none
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: repository `main` at `187071f` with uncommitted P1-003 and P2-001 files owned by other executors; no branch, worktree or stash was created and all P0-008 work stays in the existing working tree
- Authorization: the user decided on 2026-10-31 that Windows adaptation is paused for now, only platform interfaces remain in the lower layers, and macOS Apple Silicon is adapted first. This task records that decision in governance and architecture documents only. It does not authorize code, build changes, edits to another task's record, Windows implementation, signing or publication. On 2026-10-31 the user instructed the assigned executor to run P0-008 on the existing branch, use delegated independent review, and update this record and the P0-008 board row after completion; the board remains P0-000-owned and only that row is synced. On 2026-10-31 the user authorized a local Git commit of this task's documents, ADR, record and board row; push and publication remain unauthorized.

## Scope and acceptance

Objective: make the macOS-first sequencing authoritative in the standards and architecture documents, preserve Windows as a deferred target whose interfaces stay in the platform layer, and prevent any Windows support or cross-platform qualification claim.

Allowed files and interfaces: `AGENTS.md` (Section 1 targets and the Section 2 platform line), `docs/planning/functional-scope.md` (the UI acceptance gate and platform sequencing text), `docs/architecture/module-design.md` (platform ports and the dual-platform test statement), `docs/architecture/architecture-overview.md` only if it repeats the platform matrix, a new `docs/decisions/ADR-0005-platform-sequencing.md`, and this record.

Out of scope: code and CMake changes, `docs/engineering/development.md` while P1-003 has uncommitted changes, the task board (P0-000 owns it; only the P0-008 row is synced under the user-authorized scope amendment below), `docs/planning/feature-traceability.csv`, other task records, Windows implementation or evidence, acceptance of any task, signing, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | AGENTS.md states that macOS Apple Silicon is the only active qualification target for the current phase, Windows x64 is deferred but retained as a later target, and Windows support is not claimed | Met: AGENTS.md section 1 names macOS as the only active target and Windows as deferred with platform interfaces retained; section 2 asserts no Windows evidence inheritance; independent review passed |
| AC-2 | The functional-scope UI gate and platform text record macOS-only current acceptance and a deferred Windows gate; the 428-row traceability register is unchanged | Met: functional-scope P1 row, sequencing sentence and UI gate state macOS-only current acceptance and a deferred Windows gate; feature-traceability.csv is unmodified (header plus 428 rows) and the validator confirms every source anchor |
| AC-3 | The module design keeps the existing platform ports (FileAccess, SecretStore, ProcessHost, ResourceLimiter, SystemIntegration, DisplayCapabilities), states that only macOS implementations are delivered now, prohibits platform-specific types in domain and application public interfaces, and removes the both-platform P1 test claim | Met: module-design section 6 keeps all six ports, limits delivered implementations to macOS, prohibits platform types in domain/application interfaces and replaces the both-platform P1 test statement |
| AC-4 | ADR-0005 records the decision, the alternatives (continue both platforms; drop Windows permanently), the consequences (deferred evidence and portability debt) and the revisit trigger (a Windows host or authorized CI, and before any Windows claim or cross-platform release) | Met: ADR-0005 Decision, Alternatives, Consequences and Revisit triggers cover every required item |
| AC-5 | `python3 tools/validate_planning.py` passes with local links and the English policy intact | Met: final validator pass with 339 original IDs, 428 traced requirements, 66 documents, valid links, fences and English policy |
| AC-6 | Only the allowed documents and this record are edited; the board and other task records remain untouched, and no requirement is deleted or rewritten | Met under the recorded user-authorized board-row sync: P0-008 edits are AGENTS.md, functional-scope.md, module-design.md, ADR-0005 and this record, plus only the P0-008 board row; P0-008 did not edit any other board row, the P0-000 record or any other task record; a concurrent P0-007 executor changed only its own row and record; no requirement was deleted or rewritten |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | User / project owner | Decided that Windows adaptation is paused, platform interfaces stay in the lower layers, and macOS is adapted first | Direct user instruction |
| 2026-10-31 | EveryGraph Coordination / P0-000 executor | Allocated this task and its initial record scaffold; updated the board rows for P1-002, P1-010 and P1-013 to defer Windows and narrow the P1 exit to macOS; did not edit any task record | [Board](../planning/task-board.csv); [P0-000](p0-000-task-allocation.md) |
| 2026-10-31 | EveryGraph Lead / P0-008 executor (Qoder main-agent session) | Read the live board, P0-000 record, this record, AGENTS, functional scope, module design, architecture overview, validator and ADR format; confirmed the architecture overview does not repeat the platform matrix and that README, roadmap and other task files are outside the allowed set; started the task | This record; `git status --short`; `python3 tools/validate_planning.py` baseline with 65 documents |
| 2026-10-31 | EveryGraph Lead / P0-008 executor | Updated AGENTS section 1 targets and section 2 platform line, functional-scope P1 row, UI gate and platform sequencing sentence, module-design platform ports and test statement, and created ADR-0005; no code or other task record was edited at this stage; the board row sync is recorded separately | [AGENTS.md](../../AGENTS.md); [functional scope](../planning/functional-scope.md); [module design](../architecture/module-design.md); [ADR-0005](../decisions/ADR-0005-platform-sequencing.md) |
| 2026-10-31 | Independent reviewer / delegated read-only Qoder subagent | Reviewed AC-1 to AC-6 against the working tree; ran the planning validator and read-only Git checks; reported all six criteria PASS, no Windows support claim, no cross-document contradiction, required record-hygiene fixes, and residual pre-decision wording only in out-of-scope README, roadmap and validation report; made no edits or commits | Independent review report; `git status --short`; `python3 tools/validate_planning.py` |
| 2026-10-31 | EveryGraph Lead / P0-008 executor (main agent as shared-board writer) | Fixed the review's record-hygiene findings, filled the AC results, recorded acceptance and synced only the P0-008 board row to `accepted` under the user's explicit instruction; no other board row, the P0-000 record or any other task record was edited | This record; [board](../planning/task-board.csv) |
| 2026-10-31 | User / project owner | Authorized the local Git commit of P0-008's documents, ADR, record and board row; push and publication were not authorized | Direct user instruction |

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. macOS 27.0.1 arm64; Python 3.13.9.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `python3 tools/validate_planning.py` before edits | Passed: 339 original IDs; 428 traced requirements; 65 documents | Baseline; no P0-008 edit existed yet |
| 2026-10-31 | `python3 tools/validate_planning.py` after adding a standalone platform paragraph | Failed: 18 `Addition anchor mismatch` errors for EG-ADD-001 to EG-ADD-018 | The extra lines shifted the functional-scope EG-ADD anchors; the 428-row register must remain unchanged |
| 2026-10-31 | Removed the standalone paragraph and kept the sequencing text on existing lines; `git diff --numstat` | Resolved: functional-scope has 3 insertions and 3 deletions, so EG-ADD source lines are unchanged | AGENTS 2/2 lines; module-design 2/2 lines; ADR-0005 is a new file |
| 2026-10-31 | `python3 tools/validate_planning.py` after the fix | Passed: 339 original IDs; 428 traced requirements; 66 documents; input hashes, coverage, source anchors, classifications, local links, fences and English policy | Includes ADR-0005; matrix weights unchanged; remote URLs, Mermaid rendering and scientific semantics are not checked |
| 2026-10-31 | `git status --short`; `git diff --name-only`; `git ls-files --others --exclude-standard` | P0-008 edited only AGENTS.md, functional-scope.md, module-design.md, the new ADR-0005 and this record; `feature-traceability.csv`, `task-board.csv` and `architecture-overview.md` were unchanged at that point | Other modified/untracked files predate this task and belong to P1-003/P2-001; the P0-008 board row sync was deferred to the final step |
| 2026-10-31 | Independent read-only review by a delegated Qoder subagent; read-only `git status`/`git diff`; `python3 tools/validate_planning.py` | AC-1 to AC-6 PASS; diffs minimal and on-anchor; English-only; local links valid; no Windows support claim; no cross-document contradiction; record-hygiene findings reported for correction | Reviewer made no edits or commits; residual README/roadmap/validation-report wording reported as out of scope |
| 2026-10-31 | `git diff -- docs/planning/task-board.csv` after the P0-008 row sync | The P0-008 row changed from `planned` to `accepted`; a concurrent P0-007 executor separately changed only the P0-007 row to `in_progress` and updated its own record | P0-008 edited no other board row, the P0-000 record or any other task record |
| 2026-10-31 | Read-only Python check over `docs/planning/task-board.csv` | Passed: 33 rows; 33 unique IDs; all record paths exist; all statuses valid; all dependencies resolve; P0-008 is `accepted` | Custom read-only check; dependency cycles are not checked by this script |
| 2026-10-31 | `python3 tools/validate_planning.py` and `git diff --check` after all record and board updates | Passed: 339 original IDs; 428 traced requirements; 66 documents; valid links, fences and English policy; no whitespace errors | Final state; remote URLs, Mermaid rendering and scientific semantics remain unchecked |

Checks not run and their reasons: the task is documentation-only, so no CMake build, application test, macOS runtime test or Windows test applies. README, the development roadmap and the validation report still contain pre-decision platform wording; they are outside this task's allowed files and are reported as residual follow-ups rather than edited here.

## Review and disposition

Reviewer and review type: independent read-only review by a delegated Qoder general-purpose subagent with no edit or commit authority, plus an implementation self-check by the P0-008 executor. The independent reviewer is not the record writer and made no repository changes.

Reviewed files / revision: `main` at `187071f` plus the P0-008 working-tree edits; AGENTS.md, `docs/planning/functional-scope.md`, `docs/architecture/module-design.md`, `docs/architecture/architecture-overview.md` (read-only check), ADR-0005, this record, `docs/planning/feature-traceability.csv`, the task board, `python3 tools/validate_planning.py` output and read-only Git status/diff.

Findings, resolutions and recheck evidence:
- Independent review verdict: AC-1 through AC-6 PASS; diffs are minimal and anchored; English-only; local links valid; no Windows implementation, support or cross-platform qualification claim; no cross-document contradiction among the edited documents.
- Record hygiene finding: the handoff still said the task was planned and unstarted, the review section was the allocation scaffold and the AC result cells were empty. Resolved in this update; all AC results are filled and the handoff reflects the accepted state.
- Board authority finding: the task record originally excluded the board, but the user explicitly instructed the P0-008 row update. The scope change is recorded below; only the P0-008 row was synced by the main agent as shared-board writer, and the P0-000 record and other rows were not edited by P0-008. A concurrent P0-007 executor independently set its own row to `in_progress` and updated its own record; P0-008 did not touch either.
- Residual out-of-scope wording: `README.md` lines 3 and 16, `docs/planning/development-roadmap.md` lines 35, 36, 40 and 61, and `docs/engineering/validation-report.md` lines 39 and 49 retain pre-decision platform wording. They are outside this task's allowed files; they are carried forward as follow-up items and do not fail any P0-008 criterion.

Acceptance decision, accepting role and date: accepted by EveryGraph Lead / P0-008 executor (main agent) on 2026-10-31 after the independent review and a green validator; all six acceptance criteria are met, with the out-of-scope residuals retained above.

## Handoff and remaining work

Last verified state: P0-008 is accepted on 2026-10-31. AGENTS.md, functional-scope.md, module-design.md and ADR-0005 record macOS Apple Silicon as the only active qualification target and Windows x64 as a deferred later target with retained platform interfaces. The validator passes with 339 original IDs, 428 traced requirements and 66 documents, and the P0-008 board row is synced to `accepted`. The 428-row register is unchanged.

Blockers and unblock actions: none for P0-008. Do not edit `docs/engineering/development.md` until the P1-003 executor commits its uncommitted changes. A follow-up is needed for the pre-decision wording in `README.md`, `docs/planning/development-roadmap.md` and `docs/engineering/validation-report.md`; allocate it through P0-000 or obtain a direct user instruction for those files.

Next task / concrete next action: P0-000 may allocate a documentation-consistency follow-up for the residual files; P1-003 remains the active implementation task; Windows tasks P1-002 and P1-010 stay deferred and must not be started or claimed. A local commit was authorized and created on 2026-10-31; no push or publication was performed.

Files or commands needed to resume: [board](../planning/task-board.csv), [P0-000](p0-000-task-allocation.md), [AGENTS.md](../../AGENTS.md), [functional scope](../planning/functional-scope.md), [module design](../architecture/module-design.md), [ADR-0005](../decisions/ADR-0005-platform-sequencing.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned and allocated | Direct user decision to pause Windows adaptation and adapt macOS first; P0-000 allocated the task and created this scaffold |
| 2026-10-31 | planned -> in_progress | User instructed execution of P0-008 on 2026-10-31; the executor verified dependencies, allowed files and the 65-document validator baseline before editing |
| 2026-10-31 | Scope amendment: the P0-008 board row may be synced | The user explicitly instructed updating the P0-008 board row after completion; the main agent is the shared-board writer under AGENTS section 10. Only the P0-008 row is synced; P0-008 did not edit the P0-000 record or any other row or task record. A concurrent P0-007 executor changed only its own row and record. |
| 2026-10-31 | in_progress -> accepted | Independent read-only review passed AC-1 to AC-6, the validator is green, the record-hygiene findings were fixed and the residual out-of-scope wording is recorded; no criterion was weakened |
