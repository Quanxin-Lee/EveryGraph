# P0-003: Decompose Named Capability Catalogs into Evidence-Backed Child Entries

## Metadata

- Task ID: P0-003
- Status: accepted
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of the board and this record)
- Related requirements: ORG-IO-001; ORG-GRAPH-001; ORG-FIT-004; ORG-PEAK-005; ORG-EXPORT-001; EG-ADD-014; EG-ADD-015
- Dependencies: none
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: the worktree started on `main` at `747809e`; parallel P1-001 work created commits `d226fa3` and `41b6cf8`, and later parallel work switched the worktree to `feat/p1-004-command-kernel` at HEAD `41b6cf8`. P0-003 changes are uncommitted. These branches and commits belong to other writers; P0-003 did not create, amend, switch or commit them.
- Authorization: the user requested execution of P0-003 on 2026-10-30. Authorized scope is research and documentation only: a new evidence document under `docs/research/` plus this record and the P0-003 board row. No product implementation, no changes to the 428-row register or validator, no dependency installation, no Git commit, push or publication.

## Scope and acceptance

Objective: turn the named catalog umbrellas into evidence-backed enumerations and a proposed stable child-ID schema so later implementation tasks can be scoped without inventing missing capabilities.

Allowed files and interfaces: the new evidence document `docs/research/capability-catalog-decomposition.md`, this task record and the P0-003 row of `docs/planning/task-board.csv`. No changes to `origin-feature-inventory.md`, `datagraph-feature-inventory.md`, `docs/planning/feature-traceability.csv`, `docs/planning/functional-scope.md`, `tools/validate_planning.py`, product code or build configuration.

Out of scope: integrating child rows into the authoritative register, changing validator counts or status semantics, creating implementation tasks for individual features, implementing any feature, claiming catalog completeness or parity, acquiring proprietary manuals or SDKs without separate authorization.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Each listed requirement family has a section with source/manual/version citations and explicit unknowns | Pass. Seven family sections; S1-S11 source register; U-01..U-27 explicit unknowns |
| AC-2 | Named examples and discovered variants receive proposed stable child IDs plus parent, parameters, prerequisites, rights status, delivery class, fixture plan, expected output and uncertainty | Pass after review correction. 151 unique child entries: 36 import, 74 graph (73 named plus 1 claim), 2 fitting (1 named plus 1 claim), 1 peak claim, 17 export (16 named plus 1 claim), 14 compatibility, 7 gates. Per-catalog defaults make every field explicit |
| AC-3 | No invented catalog names or parameters; unavailable or rights-blocked catalogs are marked as such rather than guessed | Pass. Named entries trace to S1-S8; fitting and peak catalogs remain explicitly unnamed; rights statuses distinguish blocked and vendor-unverified; the unsupported S6 100+ citation was removed |
| AC-4 | A validator integration proposal defines the child-ID namespace, count expectations, source-anchor rules and test changes without modifying the current register or validator | Pass. Section 11 defines `EG-CAT-<AREA>-<NNN>`, a 23-column CSV schema, count constants, anchor hashing, register immutability, 13 tests and a migration gate. No register or validator change was made |
| AC-5 | Proposed evidence document passes the existing planning validator, local links and English-only policy | Pass. `python3 tools/validate_planning.py` reports PASS; no CJK; local links and fences pass; all 16 tables have consistent columns |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Research / main agent | Retrieved official Origin pages S3-S8 on 2026-10-30; read the preserved inventories, traceability rows and functional scope; wrote `docs/research/capability-catalog-decomposition.md`; proposed the child-ID schema and validator contract; ran the planning validator and a structural audit | Document sections 1-14; validator output recorded below |
| 2026-10-30 | Independent review subagent / read-only reviewer | Inspected the actual files, ran the planning validator, retrieved S3-S8, checked AC-1..AC-5 and scope boundaries; reported 2 blocking, 4 should-fix and 4 note findings | Review round 1 recorded below |
| 2026-10-30 | Independent review subagent / read-only re-reviewer | Rechecked all ten round-1 findings, recounted child IDs, re-ran the validator, verified scope isolation and AC-1..AC-5; found no blocking document issue and two record-accuracy corrections | Review round 2 recorded below |
| 2026-10-30 | EveryGraph Research / main agent | Resolved review findings: added DTG compatibility child; corrected S2 anchors and S6 overclaim; split S10/S11; added the seventh roll-up gate; tightened the schema with `primary_anchor`, `source_excerpt`, `support_claim` and translation rules; recorded Maps, generic Binary and EXPORT-017 exceptions | Document sections 2-12; recheck pending |

No product code, build configuration, dependency lock or platform claim was produced. The evidence document is new and uncommitted.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`. OS: macOS Darwin arm64. Python: 3.13.9. Shell: zsh. No Qt, Origin, DataGraph or compiler build was run because the task is documentation-only.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 45 documents; input hashes; coverage; source anchors; classifications; local links; fences; English policy; matrix totals | Document count includes concurrent task records added by another writer; P0-003 added one document |
| 2026-10-30 | Inline Python audit of child IDs and Markdown tables | 151 unique child IDs with contiguous sequences: IO 36, GRAPH 74, FIT 2, PEAK 1, EXPORT 17, COMPAT 14, GATE 7; 16 tables, 0 inconsistent; no CJK | Custom audit, not a repository test; it checks structure and counts, not scientific semantics |
| 2026-10-30 | `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py` | Inventory hashes match the validator constants: `fd2213...d0ff8` and `f5e687...9ad62fb`. Register, scope and validator are unchanged by `git diff --name-only` | Confirms protected files, not their content correctness |
| 2026-10-30 | `git status --short` and `git diff --name-only` | Final check: modified `AGENTS.md`, `docs/planning/task-board.csv`, the P0-002 and P0-003 records; untracked evidence document and P0-004/P0-005/P0-006/P1-004..P1-010 records. P0-003 changed only the evidence document, its own record and its own board row; `AGENTS.md` and the P0-002 record were changed by other writers. Earlier P1-001/CMake changes were committed or removed by their writer. Protected files remain unchanged | Proves scope isolation, not the quality of unrelated changes |
| 2026-10-30 | `git branch --show-current` and `git rev-parse --short HEAD` | `feat/p1-004-command-kernel` at `41b6cf8`; the branch was switched by parallel work during P0-003. P0-003 changes remain uncommitted and are not branch-specific | Records the actual handoff state, not a stable P0-003 branch |
| 2026-10-30 | HTTPS retrieval of S3-S8 | S3: 17-format assertion with 16 distinct rows; S4: third-party table with `AB?`; S5: over 100+ graph types, more than 30 data formats, close to 200 fits, over 25 peak functions, over 15 export formats; S6: 68 filter categories and no 100+ string; S7: more than 170; S8: over 20 surface functions | Live pages, no immutable version; retrieval is not runtime verification |

Checks not run and their reasons: no Origin/OriginPro or DataGraph interaction because neither application is available for authorized runtime inspection; no format fixtures because no lawful corpus was acquired; no implementation build because P0-003 is documentation-only; no external validation of scientific semantics. The validator does not check remote URL availability or Mermaid rendering; no Mermaid diagram was used.

## Review and disposition

Reviewer and review type: independent read-only review subagent (general-purpose agent, not the writer, not a human reviewer). Round 1 and round 2 were completed on 2026-10-30. Round 2 rechecked every round-1 finding, recounted the child entries, re-ran the validator and returned PASS for AC-1..AC-5 with no blocking document issue. Its two record-accuracy corrections (column count and review-severity count) and two wording notes were applied after round 2; those edits changed no catalog evidence or count.

Reviewed files / revision: `docs/research/capability-catalog-decomposition.md` (uncommitted), `docs/tasks/p0-003-catalog-decomposition.md` (uncommitted), `docs/planning/task-board.csv` (P0-003 row), plus the seven parent rows, inventories, functional scope and validator.

Findings, resolutions and recheck evidence:

| Finding | Severity | Resolution | Recheck |
|---|---|---|---|
| DTG was missing from the compatibility matrix although `EG-ADD-014` and functional scope name it | Blocking | Added `EG-CAT-COMPAT-014` with S2:264, `R-BLOCKED`, `D-GATE`, `F-NONE` -> `O-NONE`, `U4`; updated counts 13 to 14, gate 006 and `COMPAT_NAMED`; added `U-27` | Round 2 |
| Task record and board were stale at review time | Blocking | Updated this record to `in_review` with contributions, validation, review findings and AC results; updated the P0-003 board row | Round 2 |
| S2 anchors cited line 265 for `.dgraph` and DTG | Should-fix | `.dgraph` uses S2:100; DTG uses S2:264; source register corrected | Round 2 |
| S6 was cited for the 100+ claim, but its HTML does not state it | Should-fix | Removed S6 from the claim; S5 and S1:147 only; S6 supports 68 specific categories plus one aggregate label | Round 2 |
| Schema contradictions: parent-owned named rows, grouped names, missing primary-anchor and excerpt fields, CSV English exemption, `R-BLOCKED` versus named, undefined expectations file | Should-fix | Added `register_family` exception, one-row-per-stable-entry rule, `primary_anchor`, `source_excerpt`, `support_claim`, CSV-specific English exemption and `tools/catalog_expectations.json` proposal; revised tests 9-10 and added test 13 | Round 2 |
| EXPORT-017 inherited `D-ADAPTER` from the export default | Should-fix | Declared it the `D-GATE` exception in the defaults table, section 8.2 and the row | Round 2 |
| Maps inherited `R-OWN` while basemap data rights are unresolved | Note | Recorded that the chart is `R-OWN`, basemap data rights are separate and no map data is bundled | Round 2 |
| Generic `Binary` was named but only Binary 2D Array had a row | Note | Clarified that `Binary` is a category label, not a separate family; recorded under `U-08` | Round 2 |
| Six gates did not cover all of `EG-ADD-015` (method/options and App lists) | Note | Added `EG-CAT-GATE-007`; gate count is now 7 | Round 2 |
| S10 conflated the 428-row register with the board; S1:259 attribution was imprecise | Note | Split S10 (register) and S11 (board); reattributed the peak test requirement to S10's acceptance | Round 2 |

Acceptance decision, accepting role and date: accepted by EveryGraph Research (main agent) on 2026-10-30, after independent review rounds 1-2 and the final validator/audit. All five acceptance criteria pass. Acceptance is limited to the documented research and proposal scope: it does not assert catalog completeness, approve the 428-row register integration, imply human approval or authorize merge or publication.

## Handoff and remaining work

Last verified state: the accepted document exists at `docs/research/capability-catalog-decomposition.md`; it contains 151 unique child entries and the schema proposal; the planning validator passes with 428 traced requirements (45 documents at the final check); the protected register, validator, functional scope and inventories are unchanged; all P0-003 changes are uncommitted on the current worktree branch.

Blockers and unblock actions: the fit and peak function catalogs have no named membership; the graph claim is at least 27 named entries above the evidence floor; the seventeenth export format is unaccounted; all compatibility artifacts lack lawful specifications or rights decisions; no vendor runtime was available. These are explicit gaps, not failures hidden by the document. Unblock actions: obtain lawful versioned manuals or catalogs, rights-checked fixtures and authorized runtime access in a separately scoped task.

Next task / concrete next action: a separate authorized integration task must approve the schema, create `docs/planning/catalog-children.csv` and extend the validator. A second, separately authorized evidence task must obtain lawful fitting/peak catalogs, identify the seventeenth export format, close the graph gap and resolve compatibility rights. The catalog gaps remain explicit; no parity claim is available.

Files or commands needed to resume: [evidence document](../research/capability-catalog-decomposition.md), [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), the two root inventories, [recording guide](README.md), and `python3 tools/validate_planning.py` from the project root.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> planned | User requested only the next batch of tasks; catalog research was defined but not authorized to execute |
| 2026-10-30 | planned -> in_progress | User requested execution of P0-003 on 2026-10-30; scope limited to research, documentation and the P0-003 record/board row |
| 2026-10-30 | in_progress -> in_review | Candidate document, validator/audit evidence and independent review round 1 are recorded; acceptance pending round 2 |
| 2026-10-30 | in_review -> accepted | Round 2 closed every round-1 finding and passed AC-1..AC-5; post-review record corrections were applied and rechecked; catalog gaps remain explicit |
