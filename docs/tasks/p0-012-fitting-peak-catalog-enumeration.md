# P0-012: Enumerate fitting and peak function catalogs and record remaining gate status

## Metadata

- Task ID: P0-012
- Status: accepted
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of this record)
- Related requirements: ORG-GRAPH-001;ORG-FIT-004;ORG-PEAK-005;EG-ADD-015
- Dependencies: P0-003;P0-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main`; no new branch or worktree was created. HEAD advanced by other owners during the task and was `6b59fa5` when the final validator ran; HEAD was `4c4747e` when the authorized commit was prepared. The two P0-012 files were first included by another owner's snapshot commit `0277df4`, which also contains unrelated work. The user authorized a commit on 2026-10-31; it contains only the two P0-012 files and was not pushed. P0-012 made no earlier commit.
- Authorization: the user requested execution of P0-012 on 2026-10-31. Authorized scope is research and documentation only: the new `docs/research/fitting-and-peak-catalog-enumeration.md` plus this record. No product code, dependency change, register edit, other task record edit, board edit (P0-000-owned), gate closure, Windows execution, commit, push or publication.

## Scope and acceptance

Objective: turn the P0-005 evidence floors into a versioned, reviewable per-function enumeration for fitting and peak functions and record the current status of GATE-002 through GATE-007 and EG-ADD-015 without closing any gate that lacks per-function rows.

Allowed files and interfaces: a new `docs/research/fitting-and-peak-catalog-enumeration.md` and this record. The register, catalog children, inventories, functional scope, research evidence documents and every other task record are read-only.

Out of scope: register or catalog-child edits, gate closure, fit or peak implementation, vendor manual acquisition without lawful access, product code, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The document enumerates fitting functions from the P0-005 evidence with stable proposed child IDs, source/version, parameters, prerequisites, rights status, delivery class and uncertainty. | Pass. 239 fitting rows `EG-CAT-FIT-003`..`EG-CAT-FIT-241` in 32.1 sidebar order; per-row page links, parameters, published bounds, page-category tokens and rights/delivery/uncertainty codes; shared source version and per-catalog prerequisites in sections 2-4. Independent evidence verifier compared all 239 rows and found no invented fact |
| AC-2 | The document enumerates one-dimensional and surface peak functions with the same fields and marks Pro-only, unknown and vendor-claim-only entries explicitly. | Pass. 28 one-dimensional rows with 7 Pro-only markers and 21 shared fitting references; 22 surface rows, including reused `EG-CAT-FIT-002` and 21 new `EG-CAT-PEAK-030`..`050`; U1/U2 codes and explicit anomalies in sections 5-6. Independent evidence verifier compared all 28 and 22 rows and confirmed the 7 Pro-only markers |
| AC-3 | The document records the current status of GATE-002 through GATE-007 and EG-ADD-015 with evidence and names the integration owner; it does not edit or close a gate. | Pass. Section 7 records all seven gates as Open, cites P0-003 section 10 and P0-005 section 9 for status, assigns the integration owner under the accepted P0-004 schema and P0-000 board, and states no gate closure. No gate row was edited. Independent reviewer confirmed all gates remain Open and no gate evidence is claimed for the unexamined gates |
| AC-4 | No name, parameter or vendor fact is invented; unknown lists remain explicit; every claim cites the P0-003 or P0-005 evidence by section and source. | Pass after review fixes. Names, parameters and bounds come from the linked pages; section 8 records U-01/U-02/U-03/U-29/U-30 deltas and new U-34..U-40; sections 1-8 cite P0-003/P0-005 sections and per-row pages; no equation, description or FDF content was copied. The independent verifier found no invented fact and five precision omissions; all five were corrected and rechecked |
| AC-5 | The planning validator passes and only the new research document and this record are edited. | Pass. Final `python3 tools/validate_planning.py` reports PASS: 339 original IDs; 428 traced requirements; 91 documents; input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix totals. Only the two P0-012 files were authored by P0-012; the other files, including the board and register, were not edited. The two files were first included by another owner's snapshot commit `0277df4`; the authorized P0-012 commit contains only these two files |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-012 and created the initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | P0-000 / board owner | Synchronized the P0-012 board row to `accepted` in commit `6bee412`; P0-012 did not edit the board | [Board](../planning/task-board.csv) |
| 2026-10-31 | User / project owner | Authorized the commit of the two P0-012 files after acceptance; no push was requested | User instruction to commit on 2026-10-31 |
| 2026-10-31 | EveryGraph Research / main agent | Fetched and cached 272 public Help responses (268 unique function pages plus four index pages), parsed the 32.1 sidebar and 22 category tables, the 32.4 table and bold markers, and the 32.3 table; wrote [the enumeration](../research/fitting-and-peak-catalog-enumeration.md); assigned proposed IDs; recorded gate status and unknown deltas; ran the planning validator and structural audits | 239 fitting, 28 one-dimensional peak and 22 surface rows in the new document; final validator PASS; audit results below |
| 2026-10-31 | Independent read-only research verifier (subagent) | Recomputed every published count, compared all 239 fitting, 28 peak and 22 surface rows against the cached pages, re-derived every anomaly and checked the gate and unknown sections. No file edits | Verdict: no blocking findings; counts, row transcription, anomaly set, gate statuses and unknown registers evidence-sound; ten documentation/precision findings, five should-fix |
| 2026-10-31 | Independent read-only acceptance reviewer (subagent) | Checked AC-1..AC-5, scope isolation, ID allocation, protected files, board ownership and record truthfulness; ran the validator. No file edits | Verdict: not ready at first review because the record and document carried stale Git and board state; AC-1..AC-4 passed on content and AC-5 passed on criterion but its evidence was false |
| 2026-10-31 | EveryGraph Research / main agent | Resolved all review findings: corrected U-29 to Open, expanded U-36 with the Poisson, Power2 and Nonessencial2 bound-name mismatches, removed the U-30 edition attribution, rewrote the 32.3 bound evidence, changed surface blank bounds to `none published`, corrected retrieval times, F9 provenance, parameter slot/name totals, gate closure wording and owner sourcing, and disclosed the `0277df4` commit and the P0-000 board state; regenerated the document and reran the validator and audit | Findings table below; final validator PASS; final structural audit |
| 2026-10-31 | P1-003 owner (external task) | Corrected a Chinese quotation in the P1-003 record that briefly caused the global validator to fail during P0-012 review, in commit `6b59fa5`. P0-012 did not edit the P1-003 record or the validator | Final validator PASS; no cross-task edit by P0-012 |

The new research document and this record are P0-012's only file changes. The board is P0-000-owned and was not edited. No product code, build configuration or dependency was changed.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9 and 3.14.2; zsh. No product build was run because this is a documentation task.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | HTTPS retrieval of 272 public Help responses: 268 unique function pages plus four index pages | 272/272 HTTP 200; every response carried `Last-Modified` 2026-10-26 09:00:17 or 09:00:18 UTC; response dates 2026-10-30 19:11:17-19:11:41 UTC | Cached under `/tmp/p012_pages/` and `/tmp/p012_fetch_meta.json`, outside the repository; no vendor runtime, manual, SDK or FDF file was used |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 91 documents; input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix totals | Final run. An intermediate run failed on a Chinese quotation in the P1-003 record; the P1-003 owner corrected its own record concurrently and the final run passed. The validator does not check remote URL availability, Mermaid rendering, table semantics or scientific correctness |
| 2026-10-31 | Inline Python structural audit of the final document | 239 fitting rows, 28 one-dimensional peak rows, 22 surface rows; 289 unique child IDs (288 new proposed plus reused `EG-CAT-FIT-002`); FIT-003..FIT-241 and PEAK-002..PEAK-050 contiguous; 0 CJK; 0 table-column mismatches; 4/4 local links exist; 268 unique function URLs present; 16 surface rows `none published` and 6 with a published constraint | Structural and transcription audit, not scientific validation |
| 2026-10-31 | `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py docs/planning/catalog-children.csv` | Inventory `fd221384...d0ff8` and `f5e6876...ad62fb`; register `f2db6f3...82a53`; scope `eb9a910...ea69`; validator `cd3473d...b7a7`; catalog children `a2c95d4...ba997` | Confirms the current protected-file content. P0-012 commands wrote only the new research document and this record; no protected file was targeted |
| 2026-10-31 | `git rev-parse --short HEAD`, `git log --oneline`, `git status --short`, `git branch --show-current` | `main`; HEAD `6b59fa5` when the final validator ran and `4c4747e` when the authorized commit was prepared; the authorized commit contains only the two P0-012 files; no new branch; no push; the board records `accepted` after commit `6bee412` | Proves scope isolation for P0-012 and discloses that `0277df4` first included the two P0-012 files in another owner's snapshot. Later commits before the P0-012 commit are other owners' work and are outside P0-012. The board is P0-000-owned |
| 2026-10-31 | Parser recount of 32.1, 32.4 and 32.3 | 239 pages, 22 headings, 307 cells, 231 unique strings, 10 sidebar-only, 2 table-only; 28 peak names, 7 Pro-only, 21 shared; 22 surface names; 922 numbered fitting parameter slots and 923 listed names (`SineSqr`); 149 surface slots and 148 names (`Plane`); one duplicate-name surface row | Parser-based structural count; no equations or numerical fits were checked |

Checks not run and their reasons:

- No fit, peak or surface numerical execution: no implementation exists and no vendor runtime was authorized.
- No equation or parameter-meaning reproduction: the task records short factual fields only; no vendor expression, description or FDF content is copied.
- No vendor manual, SDK or FDF inspection: not acquired and not authorized.
- No legal review: the rights classification is engineering evidence, not legal advice.
- No versioned 2026b snapshot: the Help site is live and unversioned; the 2026b binding is inferred.
- No Windows execution: documentation-only task and platform out of scope.
- No build or test suite: the task adds no product code or build configuration.

## Review and disposition

Reviewer and review type: two independent read-only subagents reviewed the task on 2026-10-31. One verified the research counts and transcription; the other reviewed AC-1..AC-5, scope isolation and the gate/board boundaries. Neither was the record writer and neither edited files. No human reviewer participated.

Reviewed files / revision: the research document and this record at the review snapshot, plus read-only checks of the board, catalog children, feature register, functional scope, validator and the P0-003/P0-005 evidence. The final resolution recheck was performed by the record writer against the actual files; it was not a second independent review.

| ID | Source | Severity | Finding | Resolution | Recheck |
|---|---|---|---|---|---|
| R-A1 | Research verifier | should-fix | The result paragraph claimed page-category tokens for every function, but 21 of 22 surface pages and all 7 Pro-only peak pages publish none | Changed to "page-category tokens where present" and "bound expressions where published" | Final section 1; 0 unsupported tokens in the audit |
| R-A2 | Research verifier | should-fix | Section 6.3 described lower and upper bound cells in the 32.3 table; the index lists names only and has no bound columns | Rewrote the anomaly row to say the index has no bound columns and only six function pages publish a constraint | Final section 6.3 |
| R-A3 | Both reviewers | should-fix | U-29 was marked partially resolved although P0-005 section 11 keeps it Open and P0-012 added no structural resolution | Restored U-29 to Open and stated that the ten sidebar-only pages, typo and cross-appendix name were already recorded by P0-005 | Final section 8 |
| R-A4 | Both reviewers | should-fix | U-36 omitted page-level bound-name mismatches: `Poisson` bounds `r0` for parameter `r`; `Power2` bounds `p1` for `pl`; `Nonessencial2` bounds `S` for `A` | Retitled and expanded U-36 to cover malformed, unassociated and mismatched bounds | Final section 8 |
| R-A5 | Research verifier | should-fix | Section 6.1 attributed the per-function edition split to U-30, which covers surface count and naming only | Removed the attribution and stated that no per-function surface edition column is published | Final section 6.1 |
| R-A6 | Research verifier | note | "922 declared fitting parameters" was the sum of `Number` fields; 923 names are listed because of `SineSqr` | Reworded to 922 numbered slots and 923 names, and added the 149/148 surface count for `Plane` | Final section 11 |
| R-A7 | Research verifier | note | Retrieval was stated as 19:09-19:11 UTC although all responses were 19:11:17-19:11:41; the F9 row said retrieved although F9 was not re-fetched | Corrected the times and marked F9 as not retrieved by P0-012 and transcribed from P0-005 section 5.2 | Final sections 2.1, 10 and 11 |
| R-A8 | Research verifier | note | Integration-owner wording was not sourced to P0-003/P0-005, and the GATE-003/GATE-004 closure cells omitted the edition split and 2026b pin named in their evidence cells | Added P0-004/P0-000 sourcing for ownership and added the edition split and 2026b pin to the closure conditions | Final section 7 |
| R-A9 | Research verifier | note | Surface `none` read as a published value where the page publishes no constraint | Changed the 16 unconstrained surface rows to `none published` and explained the token | Final section 6.2; audit found 16 and 6 |
| R-B1 | Acceptance reviewer | blocking | The record and document claimed the files were uncommitted and HEAD was `b7f2cb3`; the files were included in another owner's commit `0277df4` and HEAD had advanced | Disclosed `0277df4` as an external snapshot commit, recorded HEAD `6b59fa5` when the final validator ran, and preserved the fact that P0-012 made no commit before the authorized one | Metadata, section 9 and validation table |
| R-B2 | Acceptance reviewer | should-fix | The record and document said the board row was `planned`; P0-000 had already set it to `in_review` | Recorded `in_review` at review time; P0-000 later synchronized the row to `accepted` in `6bee412` | Section 9 and handoff |
| R-B3 | Acceptance reviewer | should-fix | "289 proposed rows" overcounted because `EG-CAT-FIT-002` is reused | Corrected to 288 new proposed children plus one reused existing row | Final sections 1 and 3.1; AC-2 |
| R-B4 | Acceptance reviewer | note | New surface rows default to `peak-function` while the existing `Gauss2D` is `fit-function` | Added a note that the register owner decides the surface member type | Final section 3.2 |

Acceptance decision, accepting role and date: accepted on 2026-10-31 by EveryGraph Research, the main agent and sole record writer, after both independent reviews and the resolution recheck. No human reviewer participated. Acceptance covers this research deliverable and record only; it does not admit register rows, close a gate, approve implementation, authorize a push, or change the board. The user separately authorized the commit of the two P0-012 files.

## Handoff and remaining work

Last verified state: [the enumeration](../research/fitting-and-peak-catalog-enumeration.md) exists with 289 enumerated rows (288 new proposed children plus reused `EG-CAT-FIT-002`; 239 fitting, 28 one-dimensional peak, 22 surface), stable proposed IDs, explicit gate status and unknown deltas. The final `python3 tools/validate_planning.py` run passes. The catalog register, feature register, functional scope, inventories and validator are unchanged by P0-012. HEAD was `6b59fa5` when the final validator ran and `4c4747e` when the authorized commit was prepared; the two P0-012 files were first included in another owner's snapshot commit `0277df4`. The authorized commit contains only the two P0-012 files and was not pushed.

Blockers and unblock actions: none for P0-012. The P0-000 board already records `accepted` in commit `6bee412`. Admitting the proposed rows needs a separately authorized register-integration task; P0-012 cannot edit `catalog-children.csv` or close a gate. The vendor count, edition and surface conflicts still need lawful versioned catalogs or vendor clarification, not inference.

Next task / concrete next action: the P0-004 integration owner or a later authorized allocation receives a separate register-integration task. No gate is closed by this task. The authorized commit is local; any push requires separate authorization.

Files or commands needed to resume: [the enumeration](../research/fitting-and-peak-catalog-enumeration.md), [P0-003 record](p0-003-catalog-decomposition.md), [P0-005 record](p0-005-catalog-evidence-gaps.md), [P0-005 evidence](../research/catalog-evidence-gaps-2026b.md), [catalog register](../planning/catalog-children.csv), [functional scope](../planning/functional-scope.md), and `python3 tools/validate_planning.py` from the repository root.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user requested execution of P0-012 on 2026-10-31; scope limited to the new research document and this record. |
| 2026-10-31 | in_progress -> in_review | Candidate enumeration, validator PASS, structural audit and gate/unknown records were ready; two independent read-only reviews were assigned. |
| 2026-10-31 | in_review -> accepted | Both independent reviews completed; all blocking and should-fix findings were resolved; the final validator and structural audit pass. Acceptance is scoped to the research deliverable and record. |
