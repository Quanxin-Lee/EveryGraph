# P0-017: Admit the reviewed fitting and peak children into the catalog register

## Metadata

- Task ID: P0-017
- Status: accepted
- Owner: EveryGraph Research
- Record writer: EveryGraph Research
- Related requirements: ORG-FIT-004; ORG-PEAK-005
- Dependencies: P0-004; P0-012
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main`; no new branch/worktree; progress-record commit `0cfbe3e` is on top of P0-000 authorization `bcaddd0`; final artifact commit is prepared from that revision
- Authorization: the user authorized a limited scope expansion on 2026-10-31: admit 239 fitting rows and 28 one-dimensional Peak Analyzer rows; register the P0-012 research document as a digest-pinned local evidence source; keep the 21 generic surface rows unadmitted and both gates open. The executor edits only the manifest paths below. No support claim is admitted, no catalog gate is closed, and no other task's record or shared feature register is edited.

## Scope and acceptance

Objective: admit the reviewed 239 fitting entries (`EG-CAT-FIT-003`..`241`) and 28 one-dimensional Peak Analyzer entries (`EG-CAT-PEAK-002`..`029`) from [P0-012](p0-012-fitting-peak-catalog-enumeration.md) into [the child register](../planning/catalog-children.csv) under the accepted P0-004 schema. Register and pin the P0-012 document as a local source; each new row anchors to its literal research table line at commit `c4394c72a26537a9c8d39184755a26340f781dc6`. Keep `support_claim=none`, leave the 21 generic surface-fitting entries (`PEAK-030`..`050`) unadmitted, and re-derive named floors of 240 fitting entries and 28 one-dimensional peak entries while leaving `GATE-003` and `GATE-004` open.

Allowed files and interfaces: the five exact manifest paths below. P0-000's commit `bcaddd0` designates this task the sole writer of the previously unclaimed catalog validator and test paths for this revision. The user-authorized source registration and digest pin fit the existing `tools/catalog_expectations.json` contract; the register's 23 columns, their order, family names and gate IDs remain fixed.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `docs/planning/catalog-children.csv` | modify | P0-017 is the single claiming task for the child register this revision (rule 8); the schema is P0-004's accepted contract |
| `tools/catalog_expectations.json` | modify | Re-pin the child-register digest and row-count floors; register S11 as the P0-012 local anchor and pin its reviewed document digest |
| `tools/validate_catalog_children.py` | modify | P0-000 commit `bcaddd0` grants P0-017 sole write ownership of this previously unclaimed path for the strict P0-012 count-contract extension |
| `tools/test_catalog_children.py` | modify | P0-000 commit `bcaddd0` grants P0-017 sole write ownership of this previously unclaimed path; tests must cover the strict count contract and surface exclusion |
| `docs/tasks/p0-017-catalog-fitting-peak-register-integration.md` | modify | Own task record |

Out of scope: `docs/planning/feature-traceability.csv` and the requirement register (shared normative); `docs/planning/functional-scope.md`; closing or changing the seven `EG-CAT-GATE-*` states; any `support_claim` other than `none`; editing the P0-012 research document; admitting the 21 general surface-fitting rows `EG-CAT-PEAK-030`..`050`; or reusing any existing ID. Existing `EG-CAT-FIT-002` remains unchanged.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Each of the 267 new rows carries all 23 columns, anchors the exact P0-012 research table line under S11 at the named commit, resolves `source_file`/`source_line`, has a matching excerpt SHA-256, and uses `support_claim=none`; the source file digest is pinned in expectations | Pass; independently verified |
| AC-2 | `membership_status` is `named` only where the source literally names the entry; otherwise the P0-004 value for a proposed or claim-only child is used, and the record states the rule applied and the count per value | Pass; all 267 additions are literally named (`named=267`, `claim_only=0`, `open=0`, `blocked=0`); full register status totals are 428/4/7/0 respectively |
| AC-3 | No `child_id` collides with an existing row and no retired ID is reused; the record states the ID scheme and the new ID range | Pass; 239 `FIT-003`..`FIT-241`, 28 `PEAK-002`..`PEAK-029`; reviewer confirmed unique, contiguous IDs |
| AC-4 | `python3 tools/validate_catalog_children.py` passes with both document digests and updated floors; record states 439 total rows, 240 named FIT rows and 28 named PEAK rows | Pass; validator reports 439 rows, 240 FIT, 28 PEAK and reconciles all declared evidence counts including the P0-012 source |
| AC-5 | `GATE-003` and `GATE-004` floors read 240 and 28, derived from named register rows; both gate rows remain `open`, and no gate is closed or support claim added | Pass; independent review confirmed all seven gates remain `open` and all rows use `support_claim=none` |
| AC-6 | `python3 tools/validate_planning.py` still passes, and `python3 -B -m unittest discover -s tools -p 'test_*.py'` (or the CTest registration from P0-011) is run and its result recorded | Pass; planning validator passes and the full tool suite passes all 161 tests |
| AC-7 | Only the five manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; the rule 12 self-check or `tools/validate_task_manifest.py` output is recorded | Pass; final five-path P0-017 acceptance commit is `docs(p0-017): accept fitting and peak catalog register integration`; cached path check and post-commit `git show --name-only` will verify exact scope |
| AC-8 | An independent reviewer that did not build the rows re-checks a sample of admissions against the cited sources and confirms the no-claim and no-gate-closure statements | Pass; independent review sampled admissions and separately reviewed the contract code and tests against all five P0-000 constraints |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its three-path change manifest, following [P0-015](p0-015-p0-exit-re-verification.md)'s residual that a register-integration task must be allocated because a research document alone does not move a gate; no register row admitted | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-012 record](p0-012-fitting-peak-catalog-enumeration.md); [P0-004 record](p0-004-catalog-register-integration.md) |
| 2026-10-31 | EveryGraph Research / executor | Read dependencies and current workspace, then delegated read-only admission research and independent scope review. Found a contradiction: the objective asks to admit the reviewed fitting and peak enumeration, but the same scope explicitly excludes all 288 proposed P0-012 children as failing the admission test; P0-004 also has no registered local source for that enumeration. No child, expectation, or board file was changed | Research report from `/root/p017_research`; independent review from `/root/p017_scope_review`; P0-012 enumeration and P0-017 scope |
| 2026-10-31 | User / project owner | Authorized a limited scope amendment: admit 239 fitting entries and 28 one-dimensional peak entries using a digest-pinned local P0-012 source; exclude 21 general surface rows and keep both gates open | User's scope decision received 2026-10-31 |
| 2026-10-31 | EveryGraph Research / executor | Added 267 rows anchored to S11, re-pinned the register, registered the P0-012 source digest, updated FIT/PEAK count floors and gate text. `validate_planning.py` passes; catalog validation and the tool suite expose a P0-004-owned historical-count comparison that blocks acceptance | `docs/planning/catalog-children.csv`; `tools/catalog_expectations.json`; validation evidence below |
| 2026-10-31 | Independent reviewer `/root/p017_scope_review` | Reviewed the updated candidate and confirmed row counts, S11 anchors, source pin, named membership, unique IDs, claim discipline, exclusions and open gate states. Confirmed the remaining validator failure is in the P0-004-owned document-consistency check | Independent review evidence and exact finding below |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| 2026-10-31 | Read board, task record, dependency records and current Git status | Complete | P0-004 and P0-012 are `accepted`; P0-017's three manifest paths had no pre-existing changes. Unrelated working-tree changes belong to other tasks and were left untouched |
| 2026-10-31 | Independent field/anchor audit via `tools.validate_catalog_children` functions | PASS for schema, IDs, parent integrity, fields, anchors, floors, claims, duplicate names, immutability, file form and source-contract completeness | 439 rows total; 239 new FIT and 28 new PEAK rows; 428 named, 7 open, 4 claim-only overall; 267 new rows are all named. FIT named=240, PEAK named=28. S11 source commit `c4394c72a26537a9c8d39184755a26340f781dc6`; source digest `10797d90df5afed6f944f629802918477abec8d7a8693f47338a2841dd3f3c4f`; child-register digest pin matches |
| 2026-10-31 | `python3 tools/validate_catalog_children.py` | FAIL | Only errors: `Count table drift for FIT: documents give 1, register 240` and `Count table drift for PEAK: documents give 0, register 28`. The P0-004 validator compares new admissions to historical P0-003 section 3 counts as exact current totals |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS | 339 original IDs; 428 traced requirements; 96 documents; input hashes, coverage, anchors, classifications, links, fences and English policy pass |
| 2026-10-31 | `python3 -B -m unittest discover -s tools -p 'test_*.py'` | FAIL | 158 tests ran; 3 failures (`test_committed_register_passes_every_check`, `test_default_run_never_opens_a_network_socket`, `test_main_reports_success_on_the_committed_register`), all cascading from the same P0-004 historical-count mismatch. No other test failed |
| 2026-10-31 | `python3 -B tools/validate_task_manifest.py` | Repository-wide FAIL; P0-017 has a notice | 35 errors and 56 notices across 61 task records. P0-017's reported item is `RECORD_BOARD_DIVERGENCE`: record is `blocked`, board is `planned`, and P0-000 owns the sync. Historical findings in other task records and commits are outside P0-017 |
| 2026-10-31 | `python3 tools/validate_catalog_children.py` after the P0-000 authorization | PASS | 439 rows: 428 named, 4 claim-only; FIT=240, PEAK=28; P0-003, P0-005 and P0-012 counts reconcile; the P0-012 surface list remains unadmitted |
| 2026-10-31 | `python3 -B -m unittest discover -s tools -p 'test_catalog_children.py' -v` | PASS | 40 catalog tests; includes expectation-driven additions, 22-row surface exclusion, undeclared-child rejection and wrong-count rejection |
| 2026-10-31 | `python3 -B -m unittest discover -s tools -p 'test_*.py'` | PASS | 161 tests; no failures |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS | 339 original IDs; 428 requirements; 96 documents; planning, link, hash, fence and English checks pass |
| 2026-10-31 | Baseline preservation audit comparing current expectations with `HEAD:tools/catalog_expectations.json` | PASS | P0-003 baseline and every P0-005 `declared_floors` value are byte-equivalent as parsed JSON; FIT/PEAK increments map to `FIT_NUMBERED_PAGES` and `PEAK_1D_NAMED` |
| 2026-10-31 | Independent contract review and follow-up recheck by `/root/p017_scope_review` | PASS | Confirmed the final delta values are read directly from configured expectation floor keys after P0-012 table count checks; no hard-coded increments or findings |
| 2026-10-31 | `python3 -B tools/validate_task_manifest.py` after P0-000 path grant | Repository-wide FAIL; P0-017 has a notice | 35 historical repository errors and 55 notices. P0-017's notice is record/board sync pending (`in_progress` in its record, `planned` on the P0-000-owned board); manifest path claims themselves have no P0-017-specific finding |

## Review and disposition

Reviewer and review type: independent review completed by `/root/p017_scope_review` (not the implementer). Round 1 sampled FIT-003, FIT-023 (sidebar-only), FIT-241, PEAK-002 and PEAK-007 and confirmed row anchors, IDs, membership, exclusions, claims and open gates. Round 2 reviewed the contract changes in `tools/validate_catalog_children.py`, `tools/test_catalog_children.py`, `tools/catalog_expectations.json`, and the candidate rows. The reviewer confirmed the P0-003 and P0-005 declared values remain unchanged; FIT/PEAK increments come from configured expectation floor keys after the P0-012 tables match those declarations; exact P0-012 table IDs are required; all surface rows remain excluded from S11 and the 21 non-retained surface IDs remain absent; negative tests reject undeclared children and wrong counts. No findings. Latest recheck: `python3 tools/validate_catalog_children.py` passes and `python3 -B -m unittest discover -s tools -p 'test_catalog_children.py'` passes all 40 catalog tests. The earlier validator failure was resolved by this P0-017 contract extension under P0-000 authorization `bcaddd0`.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): P0-017, `accepted`, proving commit `docs(p0-017): accept fitting and peak catalog register integration` (five-path manifest-only commit). P0-000 owns the board; executor did not edit it.

Last verified state: all scoped artifacts and evidence passed validation and independent contract review on `/Users/leo/Documents/General/EveryGraph`, branch `main`. Progress record commit `0cfbe3e` contains only the task record; the five-path acceptance commit named above contains the catalog rows, expectations, validator, tests and accepted record. The board remains P0-000-owned.

Blockers and unblock actions: none after P0-000's explicit path grant in `bcaddd0`. The validator and its tests are now in this task's manifest as the sole P0-017 writer. Hard constraints: derive P0-012 row increments from declared data in `tools/catalog_expectations.json`; do not hard-code counts or relax the exact reconciliation; leave all P0-003/P0-005 baselines and declared floor values unchanged; keep the 22 surface entries unadmitted and assert that in tests; retain negative tests for undeclared rows and wrong counts; and have the independent reviewer inspect the contract change itself. Both gates stay open and all rows retain `support_claim=none`.

Next task / concrete next action: P0-000 syncs the P0-017 board row from this accepted record and proving commit. No push or publication is authorized; the 21 general surface rows remain excluded.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | planned -> in_progress | User requested P0-017 execution; dependencies and task paths were checked |
| 2026-10-31 | in_progress -> blocked | Scope conflict and missing P0-012 source registration discovered; blocker recorded and independently reviewed |
| 2026-10-31 | blocked -> in_progress | User explicitly authorized the limited amendment to admit 239 fitting and 28 one-dimensional peak entries, register the P0-012 source, exclude 21 generic surface rows and keep both gates open |
| 2026-10-31 | in_progress -> blocked | Implementation checks exposed the P0-004 validator's exact comparison against P0-003 historical counts; exact rule 9 request recorded |
| 2026-10-31 | blocked -> in_progress | P0-000 commit `bcaddd0` designated P0-017 as sole writer for the two previously unclaimed validator paths under the five hard constraints recorded above; the task record, not the board, was returned to `in_progress` |
| 2026-10-31 | in_progress -> accepted | P0-017 met AC-1 through AC-8 after independent review of the contract itself, passing validators and tests, the five-path acceptance commit, and the manifest-only path checks |
