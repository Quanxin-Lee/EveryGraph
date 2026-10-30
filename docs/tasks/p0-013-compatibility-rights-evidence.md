# P0-013: Establish the compatibility rights and specification evidence register

## Metadata

- Task ID: P0-013
- Status: accepted
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of this record)
- Related requirements: EG-ADD-014
- Dependencies: P0-003;P0-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main`. Current HEAD `6b59fa5`. The executor's in-progress record state was included by P0-000's snapshot commit `0277df4`; the executor did not create or amend that commit. The register is still uncommitted. No new branch or worktree was created.
- Authorization: the user requested execution of P0-013 on 2026-10-31 in the existing `main` worktree, without creating another branch, and authorized assigning research, implementation and independent review work. Authorized scope: create `docs/research/compatibility-rights-and-specifications.md` and edit this record only. Citation-only retrieval of lawful public vendor documentation is authorized; no SDK/manual/EULA acceptance, no account creation, no purchase. The task board is P0-000-owned and read-only to this task under AGENTS.md section 0; the register, catalog children, inventories, functional scope, architecture documents, other task records and product code are read-only. No dependency change, Windows execution, Git commit, push or publication is authorized. External legal review is an open dependency, not performed here.

## Scope and acceptance

Objective: create the versioned evidence register for native and foreign compatibility gates, separating functional, data, formula and ABI compatibility and recording rights, specification availability, lawful fixture plans and uncertainty for every family.

Allowed files and interfaces: a new `docs/research/compatibility-rights-and-specifications.md` and this record. The register, catalog children, inventories, functional scope, architecture documents and every other task record are read-only.

Out of scope: compatibility implementation, vendor SDK or manual acquisition without lawful access, legal advice, product code, dependency changes, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The register covers OPJ/OPJU, dgraph, DTG, legacy Plot, Prism, LabTalk, Origin C, XOP-like ABIs, vendor Apps and COM, MATLAB, Mathematica, LabVIEW, R/Rserve and the documented proprietary connectors. | Pass. CR-01 to CR-27 cover every named family: OPJ/OPJU/protected (CR-01 to CR-03), DataGraph package/single-file/DTG/legacy Plot (CR-04 to CR-07), Prism (CR-08), LabTalk, Origin C, XOP-like ABI, vendor Apps and COM/Orglab (CR-09 to CR-13), MATLAB MAT/Engine, Mathematica/Wolfram, LabVIEW, R/Rserve (CR-14 to CR-20), and the documented connectors Minitab, Ivium, RPC3, DMD, ADO/ODBC, SQL/cloud and Excel formulas (CR-21 to CR-27). Section 1.2 maps the coverage to the functional scope and catalog children. |
| AC-2 | Each family records functional, data, formula and ABI compatibility separately with source, version, rights status, specification availability, lawful fixture plan and uncertainty. | Pass. All 27 records have four axis rows with status `Open`, `Open-partial` or `N/A (U)`, plus source/version, rights, specification, fixture plan, uncertainty, owner and next evidence action. The structural check confirmed 27/27 blocks complete. |
| AC-3 | Every family without lawful evidence or rights is explicitly unresolved and no support or round-trip claim is made. | Pass. Every record states `support claim none; gate unresolved`; section 6 reports zero support claims and zero read/write/round-trip decisions; section 1.3 lists explicit non-claims; `R-BLOCKED` and `R-ADVERSE` records prohibit implementation or distribution claims. |
| AC-4 | External legal review is recorded as an open dependency; the task names the owner and the next evidence action for each family. | Pass. Section 9 records external legal review as an open dependency with eleven review questions and the accepting role; section 10 names an accountable role, a concrete next evidence action and exit evidence for every CR record; section 12 retains the per-family unknowns. |
| AC-5 | The planning validator passes and only the new research document and this record are edited. | Pass. `python3 tools/validate_planning.py` reports PASS for 339 original IDs, 428 traced requirements and 91 documents. Structural checks report 27 CR records, 72 sources, 85 links (18 local, none broken), no CJK and no unbalanced fence. `git status` shows the new register and this record as this task's only changes; `docs/tasks/p0-000-task-allocation.md` is another owner's concurrent modification and was not touched. |

All five criteria pass as of register version `2026-10-31.2`. The round-2 independent recheck passed all eight round-1 findings, found no blocking regression and recommended acceptance. P0-013 is **accepted by EveryGraph Research on 2026-10-31** with the limitations recorded below. The board row update to `accepted` is P0-000-owned and pending.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-013 and created the initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | EveryGraph Research / executor | Read the board, this record, P0-003 and P0-005, the catalog children, the inventories, the functional scope, the project-file-format contract and the licensing policy; started the task and assigned read-only research | This record; the sources in the register |
| 2026-10-31 | Research subagents / read-only researchers | Retrieved and summarized lawful public vendor documentation for Origin project files and languages, DataGraph and Prism containers, the licensed engines and the proprietary connectors; reported URLs, version identities, rights statements and specification access; edited no repository file | S-CR-01 to S-CR-72 in the register |
| 2026-10-31 | EveryGraph Research / executor | Wrote register version `2026-10-31.1`: 27 family records, four-axis separation, gate roll-up, vendor/OSS rights tables, fixture policy, legal-review dependency, ownership table, contradictions, unknown register and source register | [compatibility rights and specifications](../research/compatibility-rights-and-specifications.md) |
| 2026-10-31 | Independent review subagent / read-only reviewer | Reviewed AC-1 to AC-5 and spot-checked vendor sources; reported two blocking, two should-fix and four note findings; edited no file | Review round 1, below |
| 2026-10-31 | EveryGraph Research / executor | Applied the round-1 fixes: completed this record's evidence; corrected the Prism `.prism` open-format evidence and added S-CR-72; normalized `EG-CAT-COMPAT-013`; corrected the CR-25 roll-up; renamed the gate-level unknown; corrected the DataGraph spelling and added the missing legal question; corrected the MAT-file access description; issued register version `2026-10-31.2` | This record; register version `2026-10-31.2` |
| 2026-10-31 | Independent review subagent / read-only reviewer | Round 2 rechecked F-01 to F-08, re-ran the validator and structural checks, confirmed no scope breach or acceptance claim, and recommended acceptance; edited no file | Review round 2, below |
| 2026-10-31 | EveryGraph Research / executor | Refreshed the git snapshot, recorded round 2 and accepted P0-013 with retained limitations; did not edit the board | This record |

The register is new and this record is the only other file changed by P0-013. No product code, build configuration, dependency lock, register row, catalog child, board row or platform claim was produced by this task.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9; zsh. No vendor application, SDK, registered manual or sample corpus was installed or downloaded.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 91 documents; input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix weights all pass | The validator does not check remote URL availability, board schema or vendor semantics |
| 2026-10-31 | Structural script over the register: CR and source numbering, four-axis blocks, support-claim statement, local links, CJK and fences | 27/27 CR records present with four axis rows and `support claim none`; S-CR-01 to S-CR-72 contiguous; 85 links, 18 local, 0 broken; no CJK; 0 fences | Custom check, not a repository test; it verifies structure, not vendor correctness |
| 2026-10-31 | `git status --short`; `git log --oneline -3`; `git branch --show-current` | Branch `main`, HEAD `6b59fa5`; this task's changes are the untracked register and this modified record. The other dirty files in the worktree belong to P0-000 and P0-012 and were not touched by P0-013. | Confirms scope isolation; the board row remains P0-000-owned and P0-013 did not edit it |
| 2026-10-31 | Independent reviewer spot checks of public vendor pages: Origin file types and EULA, Orglab EULA, Wolfram Engine terms/FAQ, R licenses, CRAN Rserve, DataGraph Saving Files, Prism PZF/PZFX and version-compatibility guides, Prism schema FAQ, LabVIEW macOS compatibility, Library of Congress MAT description | Confirmed the OPJ read-only behavior, the reverse-engineering clauses, the Wolfram production limits, R GPL-2/GPL-3, Rserve 1.8-19.1 GPL-2, the DataGraph `.dgraphs` extension and DTG, the Prism open `.prism` statement and version incompatibility, and the LabVIEW Community-Edition-only macOS constraint | The MAT-File Format PDF and MathWorks pages returned HTTP 403 in the review environment; that gap is recorded as `U`, not as a verified fact |

Checks not run and their reasons:

- No vendor runtime: no Origin, DataGraph, Prism, MATLAB, Mathematica, LabVIEW, Minitab, Ivium, DEWETRON or R installation was available or authorized; no read/write/round-trip behavior was tested.
- No SDK, registered manual or sample corpus: acquisition requires rights review and, in some cases, purchase or account creation; none was performed.
- No external legal review: the rights statuses are engineering triage, not legal advice.
- No frozen vendor snapshot or page hashing: live Help sites are unversioned; the register records retrieval dates and excerpts only.
- No exhaustive format reverse engineering or schema extraction across all families: only published evidence was read.

## Review and disposition

Reviewer and review type: independent read-only subagent review, round 1, on 2026-10-31. This is not a self-review. Round 2 is a focused recheck of the fixes and is recorded below when complete.

Reviewed files / revision: the register at version `2026-10-31.1` and this record at its allocation-plus-start state.

Findings and resolutions:

| Finding | Severity | Resolution |
|---|---|---|
| F-01: the record was still the allocation scaffold, with all ACs "Not run" and no validation evidence, although the deliverable existed | Blocking | This record now records the contributions, AC results, exact validation commands and review round 1, and states the true pending acceptance. |
| F-02: the register claimed the record was updated and that the record was the only other changed file, while the record was unmodified | Blocking | The record is now updated; the AC-5 row and section 13 point to this record's validation, and the scope statement is accurate as of register version `2026-10-31.2`. |
| F-03: the Prism record understated public evidence on the current `.prism` container and cited the Prism 11 incompatibility sentence to the wrong page | Should-fix | CR-08 now quotes the open-file-format statement, records that no current `.prism` schema was located, cites the compatibility page and adds S-CR-72; the Data and ABI axis wording was revised without changing the `R-ADVERSE`/`SPEC-PARTIAL` status. |
| F-04: six records cited `COMPAT-013` instead of the canonical catalog ID | Should-fix | All six now cite `EG-CAT-COMPAT-013`. |
| F-05: the CR-25 roll-up dropped the `R-OPEN` ODBC-specification component | Note | The roll-up now shows `R-VENDOR`/`R-OPEN`. |
| F-06: `U-CR-28` had no corresponding CR-28 record | Note | Renamed to `U-CR-GATE-01`, a gate-level unknown. |
| F-07: "Datagraph" spelling; the legal-question table omitted CR-07/CR-22/CR-23/CR-24 | Note | Spelling corrected and a question covering those records added. |
| F-08: the MAT-File Format PDF was described as login-gated when the observed behavior was HTTP 403 | Note | The record and S-CR-35 now state HTTP 403 with access conditions not established. |

Round 2: the independent read-only reviewer rechecked F-01 to F-08, re-ran `python3 tools/validate_planning.py` (PASS, 339/428/91) and the structural checks (27 CR records, 72 sources, 85 links with 18 local and 0 broken, no CJK, balanced fences), confirmed the record does not claim `accepted` and that P0-013 did not edit the board, and found no blocking regression. Its only note was that the record's git snapshot was stale; this record was refreshed to HEAD `6b59fa5`. The reviewer recommended acceptance.

Acceptance decision, accepting role and date: **accepted by EveryGraph Research (main agent) on 2026-10-31** with the retained limitations: no family is legally cleared, no vendor runtime was tested, no SDK/manual/fixture was acquired, and every compatibility gate remains unresolved. The board row update is P0-000-owned and pending.

## Handoff and remaining work

Last verified state: register version `2026-10-31.2` exists with 27 unresolved family records and 72 sources; the validator and structural checks pass; only this task's two files are changed by this task. The register is uncommitted. The board row `P0-013` is `in_progress` and was last updated by P0-000 on 2026-10-31; P0-013 did not edit it. The task is accepted in this record.

Blockers and unblock actions: no blocker for the register itself. Every compatibility family remains blocked on two owner actions: (1) an external legal-review position on rights, and (2) lawful specification or vendor documentation for the `SPEC-NONE` and `SPEC-PARTIAL` families. The P0-000 executor must update the board row to `accepted`; P0-013 cannot do so under AGENTS.md section 0. Committing or pushing this task's files also requires explicit user authorization.

Next task / concrete next action: P0-000 sets the board row to `accepted`. The product owner then decides, family by family, whether a compatibility pack is authorized; no read, write or round-trip decision is made by this register.

Files or commands needed to resume: [register](../research/compatibility-rights-and-specifications.md), [functional scope section 6](../planning/functional-scope.md), [catalog children](../planning/catalog-children.csv), [P0-003 record](p0-003-catalog-decomposition.md), [P0-005 record](p0-005-catalog-evidence-gaps.md), [task board](../planning/task-board.csv); `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user requested execution of P0-013 on 2026-10-31 in the existing `main` worktree, without a new branch, and authorized the research, implementation and independent review work within the allowed files. |
| 2026-10-31 | Register `2026-10-31.1` -> `2026-10-31.2` | Round-1 review corrections; no rights status, version scope, fixture plan or axis status changed, but the evidence text and source register were corrected. |
| 2026-10-31 | in_progress -> accepted | All five acceptance criteria pass, round-2 independent review recommended acceptance, and the retained limitations are recorded. The board row update remains P0-000-owned. |
