# P0-009: Re-pin the catalog contract after the accepted functional-scope change

## Metadata

- Task ID: P0-009
- Status: accepted
- Owner: EveryGraph Research
- Record writer: P0-009 executor (Qoder main-agent session, 2026-10-31); the initial scaffold was written by P0-000 at allocation
- Related requirements: none; phase-gate governance and evidence verification
- Dependencies: P0-004;P0-008
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main` at `1646259` plus P0-000's uncommitted board and record updates and other owners' uncommitted files on 2026-10-31. No new branch, worktree or stash was created. Committed on `main` as `675b92f` on 2026-10-31; not pushed.
- Authorization: the user instructed execution of P0-009 on 2026-10-31 on the existing branch and delegated the necessary implementation and independent review inside the task scope; the same instruction authorized updating this record and the task board after completion. On 2026-10-31 the user authorized a local Git commit of this task's work. Authorized edits: `tools/catalog_expectations.json`, this record, and only the P0-009 board row. The commit stages only the P0-009 board row; P0-000's uncommitted allocation rows stay in the working tree for P0-000. Not authorized: product code, dependency changes, catalog re-enumeration, register or inventory edits, edits to other task records including P0-004, a new branch or worktree, Windows execution, Git push or publication.

## Scope and acceptance

Objective: restore the committed catalog evidence check after the accepted P0-008 functional-scope edit by re-pinning the protected functional-scope digest in the catalog contract, without changing any catalog row, requirement or gate.

Allowed files and interfaces: `tools/catalog_expectations.json` and this record. The accepted `docs/planning/functional-scope.md` change is reviewed before the edit. The catalog children, the 428-row register, both inventories, the P0-004 record and every other task record are read-only.

Out of scope: catalog re-enumeration, gate closure, register changes, the P0-004 record correction, the human semantic review, product code, dependency changes, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | `protected_inputs["docs/planning/functional-scope.md"]` matches the current accepted file and every other contract field is unchanged; the register and catalog children are byte-identical. | Met: the pin is now `eb9a91041b0bd71fc1cce39a5fb966a44d3a6644d298ba63176f99f09b6aea69`, the SHA-256 of the current accepted file. `git diff --numstat -- tools/catalog_expectations.json` reports 1 insertion and 1 deletion and the diff shows only that value changed. The register (`f2db6f3c…12782a53`), catalog children (`a2c95d4f…d9ba997`), both inventories (`fd2213…d0ff8`, `f5e687…9ad62fb`) and the 172-row child count are unchanged from the pre-edit baseline. |
| AC-2 | `python3 tools/validate_catalog_children.py` exits 0 with 172 child rows and the 428-row register unchanged. | Met: exit 0; output begins `PASS: 172 child rows; 161 named, 4 claim-only` and every check line passes. The validator independently re-hashes the 428-row register and both inventories; the register digest and row count match the contract. |
| AC-3 | `python3 -m unittest discover -s tools -p "test_*.py"` passes the full suite and `python3 tools/validate_planning.py` passes. | Met: the suite exits 0 with `Ran 43 tests` and `OK`; the pre-edit baseline was `FAILED (failures=4)`. The planning validator exits 0 with 339 original IDs, 428 traced requirements and 89 documents. |
| AC-4 | The record states the old and new digests, the review basis and the exact commands; the P0-004 record correction and the human semantic review remain with the P0-004 owner and are not edited here. | Met: the old pin was `93918a9b0d0cf170c8906e0a3d433574d6551fa0aa508d112d97a3d2c6991fd7`; the new pin is `eb9a91041b0bd71fc1cce39a5fb966a44d3a6644d298ba63176f99f09b6aea69`. The review basis is the accepted P0-008 edit committed as `2b1169d` (3 insertions and 3 deletions in the scope file: the P1 row, the ADR-0005 sequencing sentence and the UI gate bullet), and the working-tree file is unmodified since that commit. The exact commands and results are in the Validation section. The P0-004 record and the human semantic review were not edited and remain with the P0-004 owner. |
| AC-5 | Only the contract file and this record are edited. | Met under the recorded user-authorized board-row sync: this task's repository edits are the single contract field and this record, plus only the P0-009 board row under the direct user instruction recorded in Status and scope changes. The row sync is performed at acceptance, after the independent review. No other row, task record, register, inventory or catalog child was edited. |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-009 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | P0-009 executor (Qoder main-agent session) | Read the live board, the P0-000 allocation record, the P0-004, P0-007 and P0-008 records, the contract, validator and tests; reviewed the accepted P0-008 scope diff (`747809e..2b1169d`) and confirmed the working tree is unmodified since `2b1169d`; reproduced the failing baseline (validator exit 1, 43 tests with 4 failures); applied the one-field re-pin; reran the catalog validator, the full tool suite and the planning validator; made no other repository edit | `tools/catalog_expectations.json`; this record |
| 2026-10-31 | Independent reviewer / delegated read-only Qoder subagent | Read the contract, record, accepted scope change, register, catalog children, inventories and validator code; re-hashed every protected input and the register and children; performed a leaf-level contract comparison against HEAD; reran the catalog validator, the 43-test suite and the planning validator; reported all five criteria PASS with two record-hygiene findings and no blocking finding; made no edits or commits | Review section; `python3 tools/validate_catalog_children.py`; `python3 -m unittest discover -s tools -p "test_*.py"`; `python3 tools/validate_planning.py` |
| 2026-10-31 | P0-009 executor (Qoder main-agent session) | Committed the contract change, this record and only the P0-009 board row as `675b92f` on `main`; P0-000's 22 uncommitted allocation rows were left unstaged in the working tree; no push | `git show --stat 675b92f`; `git status --short` |

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The task is a documentation or contract correction and does not build product code.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Pre-edit `shasum -a 256 docs/planning/functional-scope.md origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/catalog-children.csv` | Scope `eb9a9104…aea69` versus the pinned `93918a9b…991fd7`; inventories, register and catalog children matched their pins exactly | Confirms the scope pin is the only stale protected input |
| 2026-10-31 | `git log --oneline -2 -- docs/planning/functional-scope.md`; `git diff 747809e 2b1169d -- docs/planning/functional-scope.md`; `git diff HEAD -- docs/planning/functional-scope.md` | The accepted P0-008 commit `2b1169d` made 3 insertions and 3 deletions (P1 row, ADR-0005 sequencing sentence, UI gate bullet); the working-tree file is unchanged since that commit | Review basis for the new digest; the P0-008 record and its independent review cover the edit |
| 2026-10-31 | Pre-edit `python3 tools/validate_catalog_children.py`; `python3 -m unittest discover -s tools -p "test_*.py"` | Validator exit 1 with `Preserved inventory changed: docs/planning/functional-scope.md`; suite ran 43 tests with 4 failures, all this check | Reproduces the P0-007 gap before the edit |
| 2026-10-31 | Edit of `protected_inputs["docs/planning/functional-scope.md"]` only; `python3 -c "import json; json.load(open('tools/catalog_expectations.json'))"`; `git diff --numstat -- tools/catalog_expectations.json` | JSON parses with the new pin; diff is 1 insertion and 1 deletion | Single-field edit; no other contract field touched |
| 2026-10-31 | Post-edit `shasum -a 256` over the scope file, both inventories, the register and the catalog children | Scope `eb9a9104…aea69` equals the new pin; inventories `fd2213…d0ff8`, `f5e687…9ad62fb`; register `f2db6f3c…12782a53`; catalog children `a2c95d4f…d9ba997` | Register, children and inventories are byte-identical to the pre-edit baseline |
| 2026-10-31 | `python3 tools/validate_catalog_children.py` | Exit 0; `PASS: 172 child rows; 161 named, 4 claim-only`; all seven PASS lines including register and inventory immutability | Remote URL availability, semantic register-family mapping and scientific semantics remain unchecked by design |
| 2026-10-31 | `python3 -m unittest discover -s tools -p "test_*.py"` | Exit 0; `Ran 43 tests in 0.096s`; `OK` | Full tools suite in the working tree |
| 2026-10-31 | `python3 tools/validate_planning.py` | Exit 0; `PASS: 339 original IDs; 428 traced requirements; 89 documents`; input hashes, coverage, anchors, classifications, links, fences, English policy and matrix weights pass | Task records and the board are included in the document count; remote URLs and semantics remain unchecked |
| 2026-10-31 | `git status --short` restricted to this task's files; `git diff --stat HEAD -- tools/catalog_expectations.json docs/planning/task-board.csv` | Only `tools/catalog_expectations.json` is newly modified by this task; the pre-existing P0-000 board insertions remain untouched; this record is the task's only other edited file | Other modified and untracked files are not P0-009 edits: they belong to the P0-000 allocation, P1-003 and P2-001, and to concurrent P0-010 and P0-013 work that touched `README.md`, `docs/planning/development-roadmap.md`, `docs/engineering/validation-report.md` and its own records after the re-pin |
| 2026-10-31 | Final `python3 tools/validate_catalog_children.py`; `python3 -m unittest discover -s tools -p "test_*.py"`; `python3 tools/validate_planning.py`; read-only CSV check over `docs/planning/task-board.csv` | Catalog validator exit 0 with 172 child rows; suite exit 0 with `Ran 43 tests` and `OK`; planning validator exit 0 with 339 original IDs, 428 traced requirements and 89 documents; board has 56 unique rows, no unresolved dependency or missing record path, and P0-009 is `accepted` | Final state after the record corrections and the single-row board sync |
| 2026-10-31 | `git diff --cached --name-status`; `git diff --cached --stat`; `git diff --cached --check`; `git show --stat --oneline HEAD`; `git diff -- docs/planning/task-board.csv \| grep -c "^+P[0-9]"` | Staged exactly the contract, this record and one added board line; commit `675b92f` contains 3 files with 95 insertions and 1 deletion; the remaining working-tree board diff is 22 P0-000 allocation rows | The shared board was partially staged so no other owner's allocation was committed |

Checks not run and their reasons: no CMake build, CTest run, application test or Windows execution applies, because the task edits one JSON contract value and one task record; the planning validator covers the documentation links and English policy. Catalog-validator registration in CTest remains P0-011, and live vendor-page re-retrieval, the human semantic review of the register mappings and the P0-004 record correction remain outside this task.

## Review and disposition

Reviewer and review type: independent read-only review by a delegated Qoder general-purpose subagent, completed on 2026-10-31. The review is required for this task and was assigned before acceptance. Implementation self-checks by the P0-009 executor are recorded in the Validation section and are not presented as independent review.

Reviewed files / revision: the working tree at `main` plus uncommitted changes based on `1646259`; `tools/catalog_expectations.json`, this record, the accepted `docs/planning/functional-scope.md`, the register, the catalog children, the inventories and the validator/test outputs.

Findings, resolutions and recheck evidence:
- Independent review verdict: AC-1 through AC-5 PASS, no blocking finding, no acceptance criterion weakened. The reviewer independently confirmed the new pin equals the live scope-file hash, the contract diff is exactly one leaf, the register and children are unmodified, the validator exits 0 with 172 rows, the suite is 43/43 OK, the planning validator exits 0, the old pin equals the pre-P0-008 blob, and commit `2b1169d` was a 3/3-line scope diff with P0-008 accepted.
- Should-fix: the record attributed every other dirty file to P0-000, P1-003 and P2-001, but concurrent P0-010 and P0-013 work modified `README.md`, the roadmap, the validation report and its own records after the re-pin. Resolved: the validation note now names those concurrent tasks and states that they are not P0-009 edits.
- Should-fix: the AC-5 and scope-amendment wording implied the board row sync was already done. Resolved: both now state that the sync is user-authorized and performed at acceptance, after the review.
- Reviewer limitation: the reviewer did not revert the working tree to re-create the red baseline, because that would modify the tree; the pre-edit failure is covered by the executor's recorded reproduction and corroborated by the P0-007 record and the old pin matching the pre-P0-008 blob.
- Recheck after the fixes: the record text was corrected only; the contract value, tests and validators were unchanged, and the final planning validator run recorded in Validation covers the edited record.

Acceptance decision, accepting role and date: accepted by the P0-009 executor (Qoder main-agent session) on 2026-10-31 after the independent read-only review passed all five criteria and its two record-hygiene findings were resolved; no blocking finding remains.

## Handoff and remaining work

Last verified state: P0-009 is accepted on 2026-10-31. The functional-scope digest is re-pinned to the accepted file and the catalog-child evidence check is green again. `python3 tools/validate_catalog_children.py` exits 0 with 172 child rows, all 43 tool tests pass and the planning validator passes with 339 original IDs, 428 traced requirements and 89 documents. The register, catalog children and both inventories are byte-identical to their pre-edit state. Only the P0-009 board row is synced to `accepted`; the board's other rows remain P0-000's. The task is committed on `main` as `675b92f` and not pushed; P0-000's 22 allocation rows remain uncommitted in the working tree.

Blockers and unblock actions: none. The P0-004 record still cites the old digest and the human semantic review of the register mappings is still carried by the P0-004 owner; neither is a P0-009 blocker, and neither was edited here.

Next task / concrete next action: P0-007 can re-verify the P0 gate at a revision containing this re-pin; P0-011 additionally waits for P1-011. The P0-004 owner should correct its own record's stale digest reference and complete the carried-forward human semantic review.

Files or commands needed to resume: [catalog contract](../../tools/catalog_expectations.json), [P0-004 record](p0-004-catalog-register-integration.md), [P0-008 record](p0-008-macos-first-platform-sequencing.md), [P0-007 record](p0-007-p0-exit-verification.md), [board](../planning/task-board.csv), `python3 tools/validate_catalog_children.py`, `python3 -m unittest discover -s tools -p "test_*.py"`, `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user instructed execution of P0-009 on the existing branch on 2026-10-31; the executor verified the dependencies, allowed files and the failing baseline before editing. No new branch or worktree was created. |
| 2026-10-31 | Scope amendment: the P0-009 board row may be synced | The user explicitly instructed updating the task board after completion. Only the P0-009 row is synced, at acceptance; the board remains P0-000's artifact and no other row or task record is edited. |
| 2026-10-31 | in_progress -> in_review | The one-field re-pin is applied and the catalog validator, all 43 tool tests and the planning validator pass; the independent review is assigned and pending. |
| 2026-10-31 | in_review -> accepted | The independent read-only review passed AC-1 to AC-5 with no blocking finding, its two record-hygiene findings were resolved, and the recheck validators remain green; only the P0-009 board row is synced. |
| 2026-10-31 | Commit authorized | The user instructed a Git commit on 2026-10-31. Only the contract change, this record and the P0-009 board row are staged; P0-000's uncommitted allocation rows stay in the working tree for P0-000, and no push or publication is authorized. |
| 2026-10-31 | Committed | The contract change, this record and only the P0-009 board row were committed as `675b92f` on `main`; P0-000's allocation rows remain uncommitted; no push. |
