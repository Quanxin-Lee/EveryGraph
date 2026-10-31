# P0-017: Admit the reviewed fitting and peak children into the catalog register

## Metadata

- Task ID: P0-017
- Status: planned
- Owner: EveryGraph Research
- Record writer: EveryGraph Research
- Related requirements: ORG-FIT-004; ORG-PEAK-005
- Dependencies: P0-004; P0-012
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not applicable until an executor starts; the allocation scaffold is committed by P0-000 on `main`
- Authorization: this record allocates the task only. The executor edits the two manifest paths below. It does not admit a support claim, does not close a catalog gate, does not edit `docs/planning/feature-traceability.csv` (a shared normative path) and does not touch another task's record

## Scope and acceptance

Objective: [P0-012](p0-012-fitting-peak-catalog-enumeration.md) produced a reviewed enumeration of fitting functions and peak types, and [P0-004](p0-004-catalog-register-integration.md) accepted the schema by which children reach the register, but none of those children are in [the child register](../planning/catalog-children.csv): it holds 172 rows and `GATE-003`/`GATE-004` still record a named floor of 1 and 0. A research document alone does not move a gate. Bring the reviewed enumerated children in as register rows under the P0-004 schema, keep every one of them at `support_claim=none`, and re-derive the named floors from the register instead of from the research prose.

Allowed files and interfaces: the two manifest paths below. The register's 23 columns, their order and the validator's contract are fixed by `tools/catalog_expectations.json`; changing a column, a family name or a gate ID is out of scope.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `docs/planning/catalog-children.csv` | modify | P0-017 is the single claiming task for the child register this revision (rule 8); the schema is P0-004's accepted contract |
| `tools/catalog_expectations.json` | modify | The child-document sha256 and the count floors are pinned there, so admitting rows requires re-pinning them in the same commit |
| `docs/tasks/p0-017-catalog-fitting-peak-register-integration.md` | modify | Own task record |

Out of scope: `docs/planning/feature-traceability.csv` and the requirement register (shared normative — request through rule 9 if a row is genuinely needed); `docs/planning/functional-scope.md`; the seven `EG-CAT-GATE-*` rows' open state; any `support_claim` other than `none`; the P0-012 research document; the 288 proposed children that fail the P0-004 admission test, which stay enumerated in research and unadmitted in the register.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Every admitted row carries all 23 columns, a `source_file` and `source_line` that resolve to the cited research document at a named revision, a `primary_anchor`, a `source_excerpt` whose sha256 matches its own `source_excerpt_sha256`, and `support_claim=none` | Not run |
| AC-2 | `membership_status` is `named` only where the source literally names the entry; otherwise the P0-004 value for a proposed or claim-only child is used, and the record states the rule applied and the count per value | Not run |
| AC-3 | No `child_id` collides with an existing row and no retired ID is reused; the record states the ID scheme and the new ID range | Not run |
| AC-4 | `python3 tools/validate_catalog_children.py` passes with the re-pinned child-document digest and updated count floors, and the record quotes the new row count and family counts | Not run |
| AC-5 | `GATE-003` and `GATE-004` named floors are re-derived from the register and stated with the counts; both gates stay `open` and the record explicitly states that no gate is closed and no support claim exists | Not run |
| AC-6 | `python3 tools/validate_planning.py` still passes, and `python3 -B -m unittest discover -s tools -p 'test_*.py'` (or the CTest registration from P0-011) is run and its result recorded | Not run |
| AC-7 | Only the three manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; the rule 12 self-check or `tools/validate_task_manifest.py` output is recorded | Not run |
| AC-8 | An independent reviewer that did not build the rows re-checks a sample of admissions against the cited sources and confirms the no-claim and no-gate-closure statements | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its three-path change manifest, following [P0-015](p0-015-p0-exit-re-verification.md)'s residual that a register-integration task must be allocated because a research document alone does not move a gate; no register row admitted | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-012 record](p0-012-fitting-peak-catalog-enumeration.md); [P0-004 record](p0-004-catalog-register-integration.md) |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| — | Not run | Not run | The task is `planned`; no executor has started |

## Review and disposition

Reviewer and review type (independent or self-review): required and not yet performed. A reviewer that did not implement the rows must sample the admissions against their cited sources and confirm that no gate was closed and no support claim was added.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): none yet; the task is `planned`. When the executor accepts, it records the row, the state and the proving commit here.

Last verified state: scaffold only. Working directory `/Users/leo/Documents/General/EveryGraph`, branch `main`.

Blockers and unblock actions: none for execution — P0-004 and P0-012 are both accepted and the enumeration exists in the repository. The rights and compatibility questions that keep the gate rows open are **not** this task's to resolve and stay open.

Next task / concrete next action: on release, read `tools/validate_catalog_children.py` and its expectations file first, then the P0-004 schema and the P0-012 enumeration, and build the admission test before writing any row.
