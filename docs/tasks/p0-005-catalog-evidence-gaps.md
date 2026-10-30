# P0-005: Close Catalog Evidence Gaps for Fitting, Peaks, Graphs and Export

## Metadata

- Task ID: P0-005
- Status: in_progress
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of this record)
- Related requirements: ORG-FIT-004; ORG-PEAK-005; ORG-GRAPH-001; ORG-EXPORT-001; EG-ADD-015
- Dependencies: P0-003 (accepted)
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: current branch `main`; HEAD `77c3693`. The working tree contains the P0-005 appendix and this record, plus concurrent P1-003 changes (`docs/engineering/development.md`, `docs/engineering/sbom/generate_sbom.py`, `docs/tasks/p1-003-dependency-pins-sbom.md`, and untracked `docs/engineering/sbom/README.md`). P0-005 did not edit the board, another task's record or the P1-003 files, and did not create a branch or worktree.
- Authorization: the user requested execution on 2026-10-30 and continuation on the current branch on 2026-10-31, without creating another branch. Authorized scope: lawful public vendor documentation retrieval, citation-only rights assessment, the new evidence appendix and this record. The P0-003 evidence document contains a P0-005 follow-up section added before the exclusive-ownership rule; P0-005 has not modified that owner's file in this commit and reports the pre-rule append for the P0-003 owner's decision. The board is P0-000-owned and was not edited. No push or publication is authorized.

## Scope and acceptance

Objective: resolve or explicitly retain the four material evidence gaps left by P0-003 so later parity claims are not built on vendor marketing counts. The task seeks lawful, versioned membership evidence, not feature implementation.

Allowed files and interfaces: the new evidence appendix and this record. The P0-003 evidence document is read-only to P0-005 under AGENTS.md section 0; it still contains the pre-rule P0-005 section 15 in HEAD, and the P0-003 owner must decide whether to retain or remove it. The board is P0-000-owned and read-only to P0-005. Preserved inventories and the traceability register are read-only.

Out of scope: inventing function names, equations, parameters or format lists; acquiring or redistributing proprietary manuals, SDKs or sample files without rights review; changing the 428-row register or validator; implementing fitting, peak analysis, graph types or export formats; and claiming that a vendor count is an achieved catalog.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Fitting: obtain a lawful, versioned 2026b function catalog or record the gap as unresolved; reconcile "close to 200" versus "more than 170" | Pass with a recorded version-pin gap. A lawful public catalog is located: 239 numbered `32.1` pages, 22 categories, 231 unique category-table names, with per-function equations and parameters. A frozen 2026b release artifact was not available; the live Help build is 2026-10-26 and the 2026b binding is an inference, so that part is explicitly unresolved under the criterion's alternative. The count conflict is narrowed and explained by a counting-basis and version-drift hypothesis (current Help and tutorial at 200-plus, 239 numbered pages, the 170 page as the outlier, product copy at close to 200); vendor confirmation remains unknown. See [appendix section 4](../research/catalog-evidence-gaps-2026b.md#4-fitting-function-catalog-org-fit-004). |
| AC-2 | Peaks: obtain a lawful, versioned 1D and surface peak-function catalog or record the gap as unresolved; reconcile "over 25" versus "over 20 surface functions" | Pass with recorded gaps. The 1D Peak Analyzer table lists 28 names, seven Pro-only and 21 shared; the surface table lists 22 names. The 25-versus-20 conflict is resolved by scope: 1D Peak Analyzer versus surface fitting. The 2026b pin and the 21-versus-22 surface count remain explicit unknowns; completeness is not claimed. See [appendix section 5](../research/catalog-evidence-gaps-2026b.md#5-peak-function-catalog-org-peak-005). |
| AC-3 | Graphs: identify the missing named types against the 100+ claim or correct the claim with evidence; distinguish built-in types from App-dependent types | **Partial.** The 73 P0-003 rows are exactly the 68 Graph Gallery filters plus five examples; the prior 27-entry-gap inference is withdrawn. The product page has 111 labels, and 21 additional named types are identified with per-entry source, version, retrieval, rights and uncertainty. Two entries are explicit App tools and two 2026 entries are co-listed as graph types and Apps. The 100-plus claim is corrected to a built-in-plus-extended and Plot-menu aggregate. However, the vendor does not publish a built-in/App marker for most of the 111 labels; only four labels have a documented App distinction. AC-3 has no "record the gap as unresolved" alternative, and P0-005 will not amend the criterion unilaterally. The criterion therefore remains Partial. See [appendix section 6](../research/catalog-evidence-gaps-2026b.md#6-graph-type-catalog-org-graph-001). |
| AC-4 | Export: identify the seventeenth asserted format or correct the claim; record the source conflict among over 15, 17 and 16 named formats | Pass. No seventeenth format is documented. The export help table has 16 distinct formats; the current guide lists 15 because Adobe Illustrator is hidden; the X-Function text says 14 while its option list has 16 values. The source conflict is recorded. Runtime behavior and encoder rights remain open but are not part of this criterion. The register still says 17 and is intentionally unchanged. See [appendix section 7](../research/catalog-evidence-gaps-2026b.md#7-graph-export-catalog-org-export-001). |
| AC-5 | Every new or revised entry has a source, version, retrieval date, rights status and uncertainty; no invented names or parameters; the catalog roll-up gate remains open until all closure conditions pass | Pass. Every revised control entry in appendix section 9 and every one of the 21 named graph additions in appendix section 6.4 carries a source, a version or release identity, a retrieval date, a rights status and an uncertainty. No function, equation parameter, format or graph name was invented. The roll-up gate remains open and the residual unknowns are explicit. |
| AC-6 | The evidence document and record pass the planning validator and English/link checks; no register, validator or product change is made | Pass. `python3 tools/validate_planning.py` reports PASS for 339 original IDs, 428 traced requirements and 48 documents, including input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix weights. The focused check passes: appendix 63 total links (8 local), 17 tables; P0-003 document 24 total links (11 local), 17 tables; record 12 total links (12 local), 7 tables; no CJK, no broken local link, no unbalanced fence. The 428-row register, validator, functional scope, inventories, product code and build configuration are unchanged. |

AC-1, AC-2, AC-4, AC-5 and AC-6 pass; AC-3 remains Partial. Under the recording guide, acceptance requires all scoped criteria to pass. P0-005 is therefore **not accepted** and remains `in_progress`. The retained catalog gaps and the open roll-up gate are unchanged.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P0-003 unknown register; no new vendor source was retrieved | [P0-003 evidence](../research/capability-catalog-decomposition.md), [takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Research / main agent | Started execution; re-verified P0-003 acceptance, the four gaps, the protected-file state and the current board; allocated four read-only research assignments | This record |
| 2026-10-30 | Research subagents / read-only fitting, peak, graph and export researchers | Retrieved and summarized lawful public Origin documentation; reported counts, excerpts, URLs, version limits and candidate reconciliations; edited no repository file | Sources in appendix section 3 |
| 2026-10-30 to 2026-10-31 | EveryGraph Research / main agent | Independently fetched and parsed 27 source entries (26 unique URLs); verified 239 numbered fitting pages, 22 categories, 231 category-table names, 28 peak names, 7 Pro-only marks, 22 surface names, 68 gallery filters, 111 product labels, 65 extended templates and the 16-row export table; wrote the appendix | [catalog evidence gaps 2026b](../research/catalog-evidence-gaps-2026b.md) |
| 2026-10-31 | Independent review subagent / read-only reviewer | Round 1 reviewed the appendix, the P0-003 append and the record; independently fetched the key vendor pages and checked counts; reported 3 blocking, 3 should-fix and 4 note findings, plus the writer-discovered P0-003 cross-task issue; edited no file | Review round 1, below |
| 2026-10-31 | EveryGraph Research / main agent | Resolved all round-1 factual findings: corrected the 2026b source claim; completed AC-5 metadata for the 21 graph additions and the two control rows; removed the unregistered 2024 note; corrected link counts and source counts; updated current HEAD/validator state. The pre-rule P0-003 append was reported to its owner; P0-005 restored the file to HEAD and made no further edit to it. | This record; appendix |
| 2026-10-31 | Independent review subagent / read-only reviewer | Round 2 rechecked the fixes; confirmed most were resolved; found that AC-3 remains Partial and the record's link/table counts were wrong; identified AC-1 wording and P0-003 remediation wording as should-fix; edited no file | Review round 2, below |
| 2026-10-31 | EveryGraph Research / main agent | Applied round-2 factual fixes: record counts corrected to 12/12 links and 7 tables; AC-1 wording changed to narrowed/explained rather than reconciled; P0-003 pre-rule append documented as a cross-task item for its owner; P1-003 dirty set updated; appendix extended-label sentence corrected; exact Conclusion Charts name and Decision Tree uncertainty corrected. AC-3 retained Partial; no acceptance criterion was amended. | This record; appendix |

No product code, build configuration, dependency lock, register row, validator rule or platform claim was produced. P0-000 committed the earlier P0-005 artifacts in `26f391e`; P0-005 did not edit the board or P1-003 files.

## Validation

Working directory, OS, architecture and tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9 and 3.14.2; zsh. No vendor application, proprietary manual, SDK or sample corpus was used.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Public HTTPS retrieval of 27 source entries (26 unique URLs) with a normal browser user agent; response headers recorded | HTTP 200 for all fetched sources; documentation `Last-Modified: Mon, 26 Oct 2026` (08:55-09:00 UTC, varying by page); response `Date: Fri, 30 Oct 2026 16:03:20 GMT` for the catalog pages | Live pages, not a frozen 2026b artifact; a later check must record a new date and hash |
| 2026-10-30 | HTML parse of `32.1 Curve Fitting Functions` | 239 numbered `32.1.1` to `32.1.239` pages; 22 categories; 307 category-table cells; 231 unique strings; 10 numbered pages absent from category tables; 2 table-only strings (one typo and one `32.4` name) | Parser count; the vendor publishes no total; page count is not a unique-model count |
| 2026-10-30 | HTML parse of `32.4 Peak Analyzer Functions` and `32.3 Non-linear Surface Fitting` | 28 peak entries with 7 bold Pro-only marks; 22 surface entries | Table-cell count; no surface-peak-only subset is published |
| 2026-10-30 | HTML parse of the Graph Gallery select and the product page graph list | 68 specific gallery filters; 111 product graph labels; 65 extended-template entries (64 unique) | Rotating gallery and marketing list; no unique counting rule; duplicate pairs recorded |
| 2026-10-30 | HTML parse of the export help, publishing guide and expGraph reference | 16 distinct format rows; 15 current-guide formats; X-Function text says 14 while the option list has 16 values; AI hidden since 2017; CGM and DXF documented as unsupported in 64-bit | No runtime test; no seventeenth name found |
| 2026-10-31 | Targeted local-link, CJK, fence and table check on the appendix, the P0-003 document and this record | Appendix: 63 total links (8 local), 17 tables, no error. P0-003 document: 24 total links (11 local), 17 tables, no error. Record: 13 total links (13 local), 7 tables, no error. No CJK, no broken local link, no unbalanced fence | Custom check, not a repository test; it verifies structure, not vendor semantics |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 48 documents; input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix weights all pass | The validator does not check remote URL availability, Mermaid rendering, scientific semantics or application behavior |
| 2026-10-31 | `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py` | Inventory hashes match the validator constants: `fd2213...d0ff8` and `f5e687...9ad62fb`. Register, scope and validator have no working-tree diff. | Confirms protected files, not the correctness of their contents |
| 2026-10-31 | CSV/record consistency check over the 17-row board | 17 rows, 17 unique IDs; P0-005 row is present, `in_progress`, with a resolvable `P0-003` dependency, all listed requirement IDs present and the record path existing | Read-only check; the board is P0-000-owned and was not modified by P0-005 |
| 2026-10-31 | `git branch --show-current`; `git rev-parse --short HEAD`; `git status --short` | Branch `main`; HEAD `77c3693`; no unmerged paths. Dirty files: P0-005 appendix/record and concurrent P1-003 files (`docs/engineering/development.md`, `docs/engineering/sbom/generate_sbom.py`, `docs/tasks/p1-003-dependency-pins-sbom.md`, untracked `docs/engineering/sbom/README.md`). The P0-003 document is unchanged relative to HEAD. | The P1-003 files are another owner's work and were not touched by P0-005 |
| 2026-10-31 | `git show HEAD:docs/research/capability-catalog-decomposition.md` piped to `wc -l`, then inspection | 650 lines; section 15 present; the file is unchanged in the working tree relative to HEAD | The pre-rule P0-005 append remains in the P0-003 owner's file; its owner must decide whether to retain or remove it |

Checks not run and their reasons:

- No round-3 review: no acceptance is claimed; AC-3 remains Partial, so a further review cannot change the acceptance outcome without new evidence or an explicit scope decision.
- No Origin or OriginPro runtime: the application is proprietary and not installed; no runtime access was authorized.
- No frozen 2026b Help snapshot or manual: the Help site is live and unversioned; no versioned manual or SDK was acquired.
- No exhaustive equation or parameter extraction across all 239 fitting, 28 peak and 22 surface entries: representative pages were sampled to prove structure, not to reproduce the vendor library.
- No full surface-peak subset: no official page publishes one.
- No legal review: the rights assessment is an engineering classification, not legal advice.
- No external page hashing or post-retrieval drift check: the appendix records retrieval dates and short excerpts, not immutable pages.

## Checkpoint: files and background work

### Files changed by P0-005

| File | Change | Current state |
|---|---|---|
| `docs/research/catalog-evidence-gaps-2026b.md` | Evidence appendix with the corrected 2026b source claim, source counts, normalization caveats, 21-entry metadata and AC-5 fields | Modified relative to `26f391e`; no broken links or table errors; to be committed on `main` |
| `docs/tasks/p0-005-catalog-evidence-gaps.md` | This record, updated with both review rounds, current HEAD/validator state and the retained AC-3 Partial | Modified; to be committed on `main` |

The P0-003 document is unchanged relative to HEAD; the pre-rule section 15 append is reported to its owner. The board is P0-000-owned and unchanged. Concurrent P1-003 files are not P0-005 work and must be excluded from the P0-005 commit.

No register, validator, functional-scope, inventory, product-code or build change was made by P0-005.

### Background work

- Four read-only research subagents completed; all returned source tables and counts. No subagent edited a repository file.
- Two independent read-only review rounds completed; their findings and resolutions are recorded below.
- No background process is running.
- No push occurred.

### Unverified content

- A complete vendor-backed built-in versus App-dependent classification for the 111 product labels; only four labels have a documented App distinction.
- The P0-003 owner's decision on the pre-rule P0-005 section 15 append in that owner's file.
- The exact 2026b release binding of the live Help pages.
- The vendor counting basis behind 170, close to 200, 200-plus and 239.
- The ten numbered fitting pages absent from category tables and the two table-only strings.
- The per-function Origin versus OriginPro split.
- The surface 21-versus-22 count and the surface-peak-only subset.
- The 2026b attribution of Individual Value Chart and Overlaid Contour.
- The current 64-bit visible export set and all runtime export behavior.
- Encoder and implementation rights for the documented export formats.
- Legal review of the citation-only source use.

## Review and disposition

Reviewer and review type: two independent read-only subagent reviews, rounds 1 and 2, on 2026-10-31. This is not a self-review. No round 3 was run because no acceptance is claimed.

Reviewed files / revision: the appendix, the P0-003 append and the record, at the states before each review.

Findings, resolutions and recheck evidence:

### Round 1

| Finding | Severity | Resolution |
|---|---|---|
| F10/§4.4 claimed the 2026b page names Fit ODE, Fit Convolution and Fitting Function Library updates, but those strings are only in global navigation | Blocking | Removed from F10 and §4.4; the appendix now states only that the 2026b highlights contain no fitting-function count and no documented 32.1 change. Rechecked against the fetched 2026b main content. |
| AC-5 fields missing for `EG-CAT-FIT-002`, `EG-CAT-GRAPH-074` and the 21 named graph additions; unregistered 2024 release note | Blocking | Added source, version/release identity, retrieval date, rights and uncertainty to every one of the 21 additions; added explicit version/retrieval text to the two control rows; removed the unregistered 2024 note. |
| Record claimed `main` at `02648bb` and a clean tree; validator count was stale | Blocking | Updated to the actual HEAD and dirty P1-003 set; validator count is now 48 documents. P1-003 files are explicitly attributed to their owner. |
| AC results mixed pass-by-alternative with Partial labels | Should-fix | AC-1, AC-2, AC-4 and AC-5 pass; AC-3 is Partial; AC-6 passes. The retained gaps are listed. |
| Allowed files included the P0-005 board row while the board is P0-000-owned | Should-fix | Removed the board from P0-005's allowed files; the board is read-only and P0-000 updates it. |
| Link counts were labeled as local when they were total links | Should-fix | Corrected to the actual total/local counts: appendix 63/8, P0-003 24/11, record 12/12. |
| 68/68 graph match omitted the ampersand normalization | Note | Added the normalization caveat and the exact 67-of-68 result. |
| 107-label arithmetic was ambiguous | Note | Clarified that one label is removed from each duplicate pair plus two explicit App labels. |
| 27 vendor pages versus 26 unique URLs; Last-Modified times varied | Note | Changed to 27 source entries and 26 unique URLs; replaced the single-second timestamp with the observed 08:55-09:00 range. |
| "Membership gap closed" could be read as completeness | Note | Replaced with "page/name enumeration resolved" and "completeness not claimed" in the summary, §4.5 and §5.4. |
| The P0-003 append was a cross-task edit under AGENTS.md section 0 | Writer-discovered cross-task issue | Reported to the P0-003 owner. P0-005 restored the file to HEAD and made no further edit; the owner must decide whether to retain or remove section 15. |

### Round 2

| Finding | Severity | Resolution |
|---|---|---|
| AC-3 still Partial because built-in versus App-dependent is distinguished for only four labels; no AC-3 unresolved alternative | Blocking | Retained AC-3 as Partial. P0-005 did not amend the criterion. The task is not accepted; a vendor-backed classification or an explicit planning scope decision is required. |
| Record link/table counts said 15/15 and 5 tables, but round 2 observed 13/13 and 6 tables | Blocking | Corrected; after the final edits the record has 12 total links (12 local) and 7 tables, and the P0-003 document has 24 total links (11 local) and 17 tables. Rechecked with a parser. |
| AC-1 said "reconciled" while the appendix says "narrowed, not officially reconciled" | Should-fix | Changed AC-1 to "narrowed and explained; vendor confirmation remains unknown." |
| P0-003 restore is itself a cross-task edit; record said no further edit and excluded it from allowed files | Should-fix | Documented the pre-rule append as a cross-task item for the P0-003 owner; P0-005 restored the file to HEAD and makes no further edit. |
| Round-1 finding count was 3 + 3 + 4 but the table has 11 rows | Note | Counted the P0-003 append row as one writer-discovered cross-task issue; total 11. |
| "111 labels mix built-in, extended and App-dependent" was unsupported | Note | Changed to built-in-looking and explicitly App-labeled labels; extended templates are a separate list. |
| Appendix §12 said hashes were "to be recorded" | Note | Changed to recorded in this record; hashes match. |
| P1-003 dirty set expanded during review | Note | Updated the current dirty set; P0-005 files are unchanged. |
| Conclusion Charts shorthand and Decision Tree U3 | Note | Used the exact G4 name "Conclusion Charts in t-test Report Sheets"; Decision Tree uncertainty changed to U2 because the name is enumerated. |

Acceptance decision, accepting role and date: **not accepted**. AC-3 remains Partial, so the recording guide's requirement that all scoped criteria pass is not met. The task remains `in_progress`. P0-005 did not amend AC-3 or any other acceptance criterion. The open condition is a vendor-backed built-in/App classification for the unlabelled graph entries, or an explicit planning decision to narrow AC-3; either requires the user/planner, not a silent change by P0-005.

## Handoff and remaining work

Last verified state: `main` at `77c3693`; no unmerged paths; the 17-row board is valid and P0-005 remains `in_progress`; `python3 tools/validate_planning.py` passes with 48 documents; the appendix and record carry both review rounds' fixes; the P0-003 document is unchanged at 650 lines with the pre-rule section 15 append, pending its owner's decision. The concurrent P1-003 changes are another owner's work and are excluded from P0-005.

Blockers and unblock actions:

- **AC-3 Partial.** The vendor does not publish a built-in/App marker for most of the 111 graph labels. To pass AC-3, obtain a vendor-backed classification or have the planner explicitly narrow AC-3; P0-005 must not amend the criterion. Until then, no acceptance.
- **P0-003 owner decision.** The pre-rule P0-005 section 15 remains in the P0-003 evidence document at HEAD. P0-005 did not modify it in this commit; the P0-003 owner must decide whether to retain or remove it.
- **Board synchronization is P0-000-owned.** The board remains `in_progress`. If the AC-3 condition is resolved later, P0-000 must update the row to `accepted` and `updated_on` 2026-10-31; P0-005 must not edit the board.
- **Open evidence gaps.** The 2026b version pin, count basis, fitting appendix structure, surface 21-versus-22 count, 2026b graph attribution, unlabelled App classification and 64-bit export runtime remain open. The roll-up gate is open.
- **Commit scope.** A local commit on the current `main` branch may include only the P0-005 appendix and this record; it must exclude the concurrent P1-003 files. No push.

Next task / concrete next action:

1. The user/planner decides whether to obtain a vendor-backed built-in/App classification for the graph labels or explicitly narrow AC-3. Do not silently change the criterion.
2. If the condition is resolved, re-run the focused checks and validator, record a final acceptance decision, and ask P0-000 to update the board row.
3. Commit only the P0-005 appendix and this record on the current `main` branch; do not create another branch and do not push. The P0-003 owner handles its own pre-rule append.

Files or commands needed to resume: [P0-005 evidence appendix](../research/catalog-evidence-gaps-2026b.md), [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), [recording guide](README.md), and `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | Lawful vendor catalogs and rights review were unavailable; no acquisition was authorized |
| 2026-10-30 | blocked -> in_progress | The user requested execution of P0-005 on 2026-10-30, authorizing lawful public-documentation retrieval and the scoped evidence update |
| 2026-10-31 | In progress -> paused checkpoint | The user requested a pause at a safe checkpoint; a concurrent stash-pop conflict blocked the commit |
| 2026-10-31 | Paused checkpoint -> active review | The P0-000 consolidation resolved the conflict and validator failure; the user requested continuation on the current branch without creating another branch |
| 2026-10-31 | Review rounds 1 and 2 completed; fixes applied | Round 1 found 3 blocking, 3 should-fix and 4 note issues plus one writer-discovered cross-task issue; the factual findings were addressed. Round 2 confirmed the fixes but retained AC-3 as Partial and found the record count error, which was corrected. AC-3 remains Partial; the task is not accepted and stays in_progress. |
