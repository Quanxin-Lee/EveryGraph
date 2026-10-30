# P0-005: Close Catalog Evidence Gaps for Fitting, Peaks, Graphs and Export

## Metadata

- Task ID: P0-005
- Status: in_progress
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of the board and this record)
- Related requirements: ORG-FIT-004; ORG-PEAK-005; ORG-GRAPH-001; ORG-EXPORT-001; EG-ADD-015
- Dependencies: P0-003 (accepted)
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: P0-005 started while the worktree was on `feat/p1-001-qt-baseline`; concurrent work later moved the worktree to `main` at `747809e` and left a failed stash-pop conflict. The P0-005 record was restored from commit `8f403e4` on `feat/p1-001-qt-baseline`. No P0-005 branch or worktree was created. All P0-005 artifacts are uncommitted.
- Authorization: the user requested execution of P0-005 on 2026-10-30 and a checkpoint with a commit request on 2026-10-31. Authorized scope: lawful public vendor documentation retrieval, citation-only rights assessment, a new evidence appendix, a dated follow-up in the P0-003 evidence document, this record and the P0-005 board row. Not authorized: acquisition or redistribution of proprietary SDKs, manual corpora or sample files; system installation; product code; register or validator changes; push or publication. A local commit was requested, but it could not be performed because the current worktree contains unmerged paths from concurrent work. P0-005 made no commit and no push.

## Scope and acceptance

Objective: resolve or explicitly retain the four material evidence gaps left by P0-003 so later parity claims are not built on vendor marketing counts. The task seeks lawful, versioned membership evidence, not feature implementation.

Allowed files and interfaces: the P0-003 evidence document, a new evidence appendix if needed, this record and the board. Read-only use of the preserved inventories and the traceability register.

Out of scope: inventing function names, equations, parameters or format lists; acquiring or redistributing proprietary manuals, SDKs or sample files without rights review; changing the 428-row register or validator; implementing fitting, peak analysis, graph types or export formats; and claiming that a vendor count is an achieved catalog.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Fitting: obtain a lawful, versioned 2026b function catalog or record the gap as unresolved; reconcile "close to 200" versus "more than 170" | Partial. A lawful public catalog is located and counted: 239 numbered `32.1` pages, 22 categories, 231 unique category-table names, with per-function equations and parameters. The count conflict is narrowed to a current-versus-stale counting-basis conflict, but the 2026b release pin, the count basis, the ten uncategorized pages and the per-function edition split remain open. See [appendix section 4](../research/catalog-evidence-gaps-2026b.md#4-fitting-function-catalog-org-fit-004). |
| AC-2 | Peaks: obtain a lawful, versioned 1D and surface peak-function catalog or record the gap as unresolved; reconcile "over 25" versus "over 20 surface functions" | Partial. The 1D Peak Analyzer table lists 28 names, seven Pro-only and 21 shared; the surface table lists 22 names. The 25-versus-20 conflict is resolved by scope: 1D versus surface. The 21-versus-22 surface count and the 2026b pin remain open. See [appendix section 5](../research/catalog-evidence-gaps-2026b.md#5-peak-function-catalog-org-peak-005). |
| AC-3 | Graphs: identify the missing named types against the 100+ claim or correct the claim with evidence; distinguish built-in types from App-dependent types | Partial. The 73 P0-003 rows are exactly the 68 Graph Gallery filters plus five examples; the prior 27-entry-gap inference is withdrawn. The product page has 111 labels, and 21 additional named types are identified. Two entries are explicit App tools; two 2026 entries are co-listed as graph types and Apps. The 100-plus claim is corrected to a built-in-plus-extended and Plot-menu aggregate, not a certified distinct-built-in count. Most App classifications remain unpublished. See [appendix section 6](../research/catalog-evidence-gaps-2026b.md#6-graph-type-catalog-org-graph-001). |
| AC-4 | Export: identify the seventeenth asserted format or correct the claim; record the source conflict among over 15, 17 and 16 named formats | Pass for this criterion. No seventeenth format is documented. The export help table has 16 distinct formats; the current guide lists 15 because Adobe Illustrator is hidden; the X-Function text says 14 while its option list has 16 values. The source conflict is recorded. Runtime behavior and encoder rights remain open but are not part of this criterion. The register still says 17 and is intentionally unchanged. See [appendix section 7](../research/catalog-evidence-gaps-2026b.md#7-graph-export-catalog-org-export-001). |
| AC-5 | Every new or revised entry has a source, version, retrieval date, rights status and uncertainty; no invented names or parameters; the catalog roll-up gate remains open until all closure conditions pass | Partial. The revised control entries in appendix section 9 carry source, live-build or release identity, retrieval date, rights status and uncertainty. Every named entry is vendor-sourced; no equation, parameter or format was invented. The roll-up gate remains open; the residual unknowns are explicit. |
| AC-6 | The evidence document and record pass the planning validator and English/link checks; no register, validator or product change is made | Not passed at this checkpoint. The focused P0-005 checks pass: 63 local links and 17 tables in the appendix, 24 local links and 17 tables in the P0-003 document, no CJK and no broken local link. The repository-wide validator fails on two unrelated links in `docs/tasks/p1-003-dependency-pins-sbom.md` to the missing `p0-006-extra-agent-verification.md`; no P0-005 document error was reported. The 428-row register, validator, functional scope and inventories are unchanged. The shared board is in an unresolved concurrent conflict and was not validated as a whole. |

No criterion is accepted at this checkpoint. AC-1, AC-2, AC-3 and AC-5 are partial; AC-4 passes its criterion; AC-6 is blocked by an unrelated validator failure and the board conflict. Independent review is still pending, so the task is not accepted.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P0-003 unknown register; no new vendor source was retrieved | [P0-003 evidence](../research/capability-catalog-decomposition.md), [takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Research / main agent | Started execution after the user's request; re-verified P0-003 acceptance, the four gaps, the protected-file state and the current board; allocated four read-only research assignments (fitting, peaks, graphs and export) | This record; research reports summarized here |
| 2026-10-30 | Research subagents / read-only fitting, peak, graph and export researchers | Retrieved and summarized lawful public Origin documentation; reported counts, excerpts, URLs, version limits and candidate reconciliations; edited no repository file | Research reports; sources in appendix section 3 |
| 2026-10-30 to 2026-10-31 | EveryGraph Research / main agent | Independently fetched and parsed the vendor pages; verified 239 numbered fitting pages, 22 categories, 231 category-table names, 28 peak names, 7 Pro-only marks, 22 surface names, 68 gallery filters, 111 product labels, 65 extended templates and the 16-row export table; wrote the appendix and the P0-003 follow-up; restored the P0-005 record from the clean branch commit after the concurrent worktree change | [catalog evidence gaps 2026b](../research/catalog-evidence-gaps-2026b.md), [P0-003 follow-up](../research/capability-catalog-decomposition.md#15-p0-005-follow-up-2026-10-30-utc-2026-10-31-local) and this record |

No product code, build configuration, dependency lock, register row, validator rule or platform claim was produced. The appendix and the P0-003 follow-up are new uncommitted files relative to the current `main` worktree.

## Validation

Working directory, OS, architecture and tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9 and 3.14.2; zsh. No vendor application, proprietary manual, SDK or sample corpus was used.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Public HTTPS retrieval of 27 vendor pages with a normal browser user agent; response headers recorded | HTTP 200 for all fetched sources; documentation `Last-Modified: Mon, 26 Oct 2026 09:00:17 GMT`; response `Date: Fri, 30 Oct 2026 16:03:20 GMT` for the catalog pages | Live pages, not a frozen 2026b artifact; a later check must record a new date and hash |
| 2026-10-30 | HTML parse of `32.1 Curve Fitting Functions` | 239 numbered `32.1.1` to `32.1.239` pages; 22 categories; 307 category-table cells; 231 unique strings; 10 numbered pages absent from category tables; 2 table-only strings (one typo and one `32.4` name) | Parser count; the vendor publishes no total; page count is not a unique-model count |
| 2026-10-30 | HTML parse of `32.4 Peak Analyzer Functions` and `32.3 Non-linear Surface Fitting` | 28 peak entries with 7 bold Pro-only marks; 22 surface entries | Table-cell count; no surface-peak-only subset is published |
| 2026-10-30 | HTML parse of the Graph Gallery select and the product page graph list | 68 specific gallery filters; 111 product graph labels; 65 extended-template entries (64 unique) | Rotating gallery and marketing list; no unique counting rule; duplicate pairs recorded |
| 2026-10-30 | HTML parse of the export help, publishing guide and expGraph reference | 16 distinct format rows; 15 current-guide formats; X-Function text says 14 while the option list has 16 values; AI hidden since 2017; CGM and DXF documented as unsupported in 64-bit | No runtime test; no seventeenth name found |
| 2026-10-31 | Targeted local-link, CJK, fence and table check on `docs/research/catalog-evidence-gaps-2026b.md` and `docs/research/capability-catalog-decomposition.md` | Appendix: 63 local links, 17 tables, no error. P0-003 document: 24 local links, 17 tables, no error. No CJK, no broken local link, no unbalanced fence | Custom check, not a repository test; it verifies structure, not vendor semantics |
| 2026-10-31 | `python3 tools/validate_planning.py` | Failed with two broken local links in `docs/tasks/p1-003-dependency-pins-sbom.md` to the missing `p0-006-extra-agent-verification.md`. No P0-005 document error was reported. | The failure is unrelated to P0-005 but blocks the repository-wide AC-6 check; it must be fixed or confirmed by its owner |
| 2026-10-31 | `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py` | Inventory hashes match the validator constants: `fd2213...d0ff8` and `f5e687...9ad62fb`. Register, scope and validator have no working-tree diff. | Confirms protected files, not the correctness of their contents |
| 2026-10-31 | `git status --short -- docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py origin-feature-inventory.md datagraph-feature-inventory.md` | No output; protected files unchanged | The broader worktree has concurrent changes and unmerged paths unrelated to P0-005 |
| 2026-10-31 | Board schema and record check | Failed on the current worktree: 23 parsed rows and 20 unique IDs due to conflict markers; duplicate `P0-002`, `P0-003` and `P1-001`. The P0-005 row exists in the stashed side of the conflict. The board was not edited after the conflict was discovered. | Shared-board integrity is a blocker; the P0-000 owner must resolve the stash-pop conflict |

Checks not run and their reasons:

- No independent review: the user requested a pause before the review assignment was dispatched.
- No Origin or OriginPro runtime: the application is proprietary and not installed; no runtime access was authorized.
- No frozen 2026b Help snapshot or manual: the Help site is live and unversioned; no versioned manual or SDK was acquired.
- No exhaustive equation or parameter extraction across all 239 fitting, 28 peak and 22 surface entries: representative pages were sampled to prove structure, not to reproduce the vendor library.
- No full surface-peak subset: no official page publishes one.
- No legal review: the rights assessment is an engineering classification, not legal advice.
- No external page hashing or post-retrieval drift check: the appendix records retrieval dates and short excerpts, not immutable pages.
- No Git commit or push: the worktree has unmerged paths from concurrent work, so a safe commit is not possible at this checkpoint.

## Checkpoint: files, background work and unverified content

### Files changed by P0-005

| File | Change | Current state |
|---|---|---|
| `docs/research/catalog-evidence-gaps-2026b.md` | New 439-line evidence appendix with source register, four gap analyses, rights review, revised control entries, expectation updates and unknown-register delta | Present as an untracked file in the current worktree; preserved |
| `docs/research/capability-catalog-decomposition.md` | Added the header pointer and section 15, the dated P0-005 follow-up; original P0-003 text retained | Present as an untracked file in the current worktree; preserved |
| `docs/tasks/p0-005-catalog-evidence-gaps.md` | Restored from `feat/p1-001-qt-baseline` commit `8f403e4` and updated with this checkpoint | Present in the current worktree; uncommitted |
| `docs/planning/task-board.csv` | The P0-005 row was set to `in_progress` before the concurrent conflict; `updated_on` remains 2026-10-30 in the stashed side | Unmerged; not edited after the conflict was discovered. The intended row is in the stashed side; its owner must resolve it |

No register, validator, functional-scope, inventory, product-code or build change was made.

### Background work

- Four read-only research subagents completed; all returned source tables and counts. No subagent edited a repository file.
- No background process is running.
- The independent review agent was not dispatched because the user requested a pause.
- No commit and no push occurred.

### Unverified content

- Independent review of the appendix, the P0-003 follow-up and this record.
- The exact 2026b release binding of the live Help pages.
- The vendor counting basis behind 170, close to 200, 200-plus and 239.
- The ten numbered fitting pages absent from category tables and the two table-only strings.
- The per-function Origin versus OriginPro split.
- The surface 21-versus-22 count and the surface-peak-only subset.
- The 2026b attribution of Individual Value Chart and Overlaid Contour.
- The built-in versus App classification of most graph labels.
- The current 64-bit visible export set and all runtime export behavior.
- Encoder and implementation rights for the documented export formats.
- Legal review of the citation-only source use.

## Review and disposition

Reviewer and review type: self-review by the record writer only. No independent reviewer has been assigned; the planned independent review was not dispatched because of the pause. This is not an independent review.

Reviewed files / revision: the two P0-005 research documents and this record, all uncommitted at the checkpoint.

Findings, resolutions and recheck evidence: the self-check found no broken local link, table inconsistency, CJK or unbalanced fence in the P0-005 documents. It found two blockers outside P0-005: the repository-wide validator failure from the P1-003 missing P0-006 link, and the unmerged shared board. Neither was modified by P0-005.

Acceptance decision, accepting role and date: pending. AC-1, AC-2, AC-3 and AC-5 are partial; AC-4 passes its criterion; AC-6 is not passed. The task is not accepted, not in review, and not committed.

## Handoff and remaining work

Last verified state: the P0-005 evidence appendix and the P0-003 follow-up exist and pass focused link/table/English checks. The 428-row register, validator, functional scope and inventories are unchanged. The current worktree is `main` at `747809e` with a concurrent failed stash-pop conflict on `CMakeLists.txt`, `CMakePresets.json`, `docs/engineering/development.md` and `docs/planning/task-board.csv`. The P0-005 record was restored from `feat/p1-001-qt-baseline` commit `8f403e4`. The shared board has conflict markers and duplicate rows; it was not edited after discovery.

Blockers and unblock actions:

- **Shared worktree conflict.** The stash-pop conflict involves four files and the authoritative board. Do not reset, clean, drop the stash or resolve another owner's conflicting changes without coordination. The P0-000 owner or the agent that initiated the stash operation must resolve the conflict and restore a single valid board.
- **Repository-wide validator failure.** `docs/tasks/p1-003-dependency-pins-sbom.md` links to the missing `p0-006-extra-agent-verification.md`. The P1-003 or P0-006 owner must restore the record or correct the link; then rerun the validator.
- **Independent review pending.** No reviewer has inspected the appendix or the revised control entries.
- **Open evidence gaps.** The version pin, count basis, fitting appendix structure, surface 21-versus-22 count, 2026b graph attribution, graph App classification and 64-bit export runtime remain open. The roll-up gate is open.
- **Commit blocked.** A local commit cannot be created safely while the index has unmerged paths. No commit or push was performed.

Next task / concrete next action:

1. After the worktree conflict is resolved by its owner, re-read the live board and this record against the files; do not rely on this checkpoint as the current board state.
2. Restore the P0-005 board row to `in_progress` with `updated_on` 2026-10-31 if the resolved board does not already contain it; run the board schema/record check.
3. Fix or wait for the P1-003 broken link, then rerun `python3 tools/validate_planning.py` and record the exact result.
4. Dispatch one independent read-only reviewer for the appendix, the P0-003 follow-up and this record. Require a check of AC-1 to AC-6, the source excerpts, the rights statements, the counts and the scope isolation.
5. Resolve review findings and recheck. Do not mark any partial criterion as passed without evidence.
6. Only after all criteria pass and the worktree is clean, commit the P0-005 artifacts on a focused `feat/` branch. Do not push without separate authorization.

Files or commands needed to resume: [P0-005 evidence appendix](../research/catalog-evidence-gaps-2026b.md), [P0-003 follow-up](../research/capability-catalog-decomposition.md#15-p0-005-follow-up-2026-10-30-utc-2026-10-31-local), [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), [recording guide](README.md), and `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | Lawful vendor catalogs and rights review were unavailable; no acquisition was authorized |
| 2026-10-30 | blocked -> in_progress | The user requested execution of P0-005 on 2026-10-30, authorizing lawful public-documentation retrieval and the scoped evidence update |
| 2026-10-31 | In progress -> paused checkpoint | The user requested a pause at a safe checkpoint, a record update and a commit; the commit is blocked by the concurrent unmerged worktree. The task remains in_progress because independent review and acceptance are incomplete. |
