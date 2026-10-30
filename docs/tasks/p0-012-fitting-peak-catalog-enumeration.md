# P0-012: Enumerate fitting and peak function catalogs and record remaining gate status

## Metadata

- Task ID: P0-012
- Status: in_review
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of this record)
- Related requirements: ORG-GRAPH-001;ORG-FIT-004;ORG-PEAK-005;EG-ADD-015
- Dependencies: P0-003;P0-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main`; HEAD advanced from `1646259` to `b7f2cb3` by P0-009 commits while this task ran. P0-000's board and record update and other owners' staged changes remain in the shared worktree. P0-012 files are uncommitted. No new branch or worktree was created.
- Authorization: the user requested execution of P0-012 on 2026-10-31. Authorized scope is research and documentation only: the new `docs/research/fitting-and-peak-catalog-enumeration.md` plus this record. No product code, dependency change, register edit, other task record edit, board edit (P0-000-owned), gate closure, Windows execution, commit, push or publication.

## Scope and acceptance

Objective: turn the P0-005 evidence floors into a versioned, reviewable per-function enumeration for fitting and peak functions and record the current status of GATE-002 through GATE-007 and EG-ADD-015 without closing any gate that lacks per-function rows.

Allowed files and interfaces: a new `docs/research/fitting-and-peak-catalog-enumeration.md` and this record. The register, catalog children, inventories, functional scope, research evidence documents and every other task record are read-only.

Out of scope: register or catalog-child edits, gate closure, fit or peak implementation, vendor manual acquisition without lawful access, product code, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The document enumerates fitting functions from the P0-005 evidence with stable proposed child IDs, source/version, parameters, prerequisites, rights status, delivery class and uncertainty. | Pass pending independent review. 239 fitting rows `EG-CAT-FIT-003`..`EG-CAT-FIT-241` in 32.1 sidebar order; per-row page links, parameters, bounds, category tokens and rights/delivery/uncertainty codes; shared source version and per-catalog prerequisites in sections 2-4 |
| AC-2 | The document enumerates one-dimensional and surface peak functions with the same fields and marks Pro-only, unknown and vendor-claim-only entries explicitly. | Pass pending independent review. 28 one-dimensional rows with 7 Pro-only markers and 21 shared fitting references; 22 surface rows, including existing `EG-CAT-FIT-002` and 21 new `EG-CAT-PEAK-030`..`050`; U1/U2 codes and explicit anomalies in sections 5-6 |
| AC-3 | The document records the current status of GATE-002 through GATE-007 and EG-ADD-015 with evidence and names the integration owner; it does not edit or close a gate. | Pass pending independent review. Section 7 records all seven gates as Open, cites P0-003 section 10 and P0-005 section 9, names EveryGraph Research as register integration owner plus P0-000/P0-009 boundaries, and states no gate closure. No gate row was edited |
| AC-4 | No name, parameter or vendor fact is invented; unknown lists remain explicit; every claim cites the P0-003 or P0-005 evidence by section and source. | Pass pending independent review. Names and parameters come from the linked pages; section 8 records U-01/U-02/U-03/U-29/U-30 deltas and new U-34..U-40; sections 1-8 cite P0-003/P0-005 sections and per-row pages; no equation, description or FDF content was copied |
| AC-5 | The planning validator passes and only the new research document and this record are edited. | Pass. `python3 tools/validate_planning.py` reports PASS; `git status --short` shows the new research document and this record as P0-012 changes; the pre-existing board modification belongs to P0-000 and was not touched |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-012 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | EveryGraph Research / main agent | Fetched and cached 272 public Help responses (268 unique function pages plus four index pages), parsed the 32.1 sidebar and 22 category tables, the 32.4 table and bold markers, and the 32.3 table; wrote [the enumeration](../research/fitting-and-peak-catalog-enumeration.md); assigned proposed IDs; recorded gate status and unknown deltas; ran the planning validator and structural audits | 239 fitting, 28 one-dimensional peak and 22 surface rows in the new document; validator PASS; audit results below |
| 2026-10-31 | Independent read-only research verifier (subagent) | Assigned to recount the 239/22/307/231/10/2 and 28/7/21/22 evidence, spot-check parameter and bound transcription, and verify that no name or parameter is invented; no file edits authorized | Result pending; findings will be recorded below |
| 2026-10-31 | Independent read-only reviewer (subagent) | Assigned to inspect actual files and scope, check AC-1..AC-5, the gate table and board ownership boundary, run the validator and structural checks, and try to falsify the ID mapping and claims; no file edits authorized | Result pending; findings will be recorded below |

The new research document and this record are P0-012's only file changes. The board remains P0-000-owned and was not edited. No product code, build configuration or dependency was changed.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9 and 3.14.2; zsh. No product build was run because this is a documentation task.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | HTTPS retrieval of 272 public Help responses: 268 unique function pages plus four index pages | 272/272 HTTP 200; every response carried `Last-Modified` 2026-10-26 09:00:17 or 09:00:18 UTC; response date 2026-10-30 | Cached under `/tmp/p012_pages/` and `/tmp/p012_fetch_meta.json`, outside the repository; no vendor runtime, manual, SDK or FDF file was used |
| 2026-10-31 | `python3 tools/validate_planning.py` | PASS: 339 original IDs; 428 traced requirements; 90 documents; input hashes, coverage, source anchors, classifications, local links, fences, English policy and matrix totals | The validator does not check remote URL availability, Mermaid rendering, table semantics or scientific correctness |
| 2026-10-31 | Inline Python structural audit of the new document | 239 fitting rows, 28 one-dimensional peak rows, 22 surface rows; 289 unique child IDs; FIT-002..FIT-241 and PEAK-002..PEAK-050 contiguous; 0 CJK lines; 0 table-column mismatches; 4/4 local links exist; 268 unique function URLs cached | Structural and transcription audit, not scientific validation |
| 2026-10-31 | `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py docs/planning/catalog-children.csv` | Inventory `fd221384...d0ff8` and `f5e6876...ad62fb`; register `f2db6f3...82a53`; scope `eb9a910...ea69`; validator `cd3473d...b7a7`; catalog children `a2c95d4...ba997` | Confirms the current protected-file content. P0-012 commands wrote only the new research document and this record; no protected file was targeted |
| 2026-10-31 | `git status --short` and `git log --oneline -5` | P0-012 changes are the new `docs/research/fitting-and-peak-catalog-enumeration.md` and this record. HEAD advanced to `b7f2cb3` through P0-009 commits during the task. Other owners' staged and unstaged changes, including the P0-000 board, remain untouched | Proves scope isolation for P0-012; it does not validate unrelated work. The board row is not synchronized by P0-012 because the board is P0-000-owned |
| 2026-10-31 | Parser recount of 32.1, 32.4 and 32.3 | 239 pages, 22 headings, 307 cells, 231 unique strings, 10 sidebar-only, 2 table-only; 28 peak names, 7 Pro-only, 21 shared; 22 surface names; 922 declared fitting parameters; one shared count mismatch; one surface count mismatch; one duplicate-name surface row | Parser-based structural count; no equations or numerical fits were checked |

Checks not run and their reasons:

- No fit, peak or surface numerical execution: no implementation exists and no vendor runtime was authorized.
- No equation or parameter-meaning reproduction: the task records short factual fields only; no vendor expression, description or FDF content is copied.
- No vendor manual, SDK or FDF inspection: not acquired and not authorized.
- No legal review: the rights classification is engineering evidence, not legal advice.
- No versioned 2026b snapshot: the Help site is live and unversioned; the 2026b binding is inferred.
- No Windows execution: documentation-only task and platform out of scope.
- No build or test suite: the task adds no product code or build configuration.

## Review and disposition

Reviewer and review type: two independent read-only subagents were assigned on 2026-10-31. One verifies the research counts and transcription; the other reviews AC-1..AC-5, scope isolation and the gate/board boundaries. Neither is the record writer and neither may edit files. No human reviewer participated.

Reviewed files / revision: `docs/research/fitting-and-peak-catalog-enumeration.md` and this record (both uncommitted), plus read-only checks of the board, catalog children, feature register, functional scope, validator and the P0-003/P0-005 evidence.

Findings, resolutions and recheck evidence: pending the subagent results. No finding is recorded as resolved until the reviewer evidence is inspected.

Acceptance decision, accepting role and date: pending. The task is in review, not accepted. Acceptance requires all five criteria to pass and the independent review findings to be resolved without weakening scope.

## Handoff and remaining work

Last verified state: [the enumeration](../research/fitting-and-peak-catalog-enumeration.md) exists with 289 proposed rows (239 fitting, 28 one-dimensional peak, 22 surface), stable proposed IDs, gate status and unknown deltas. `python3 tools/validate_planning.py` passes. The catalog register, feature register, functional scope, inventories and validator are unchanged. The task record and research document are uncommitted on `main`; HEAD is `b7f2cb3` after P0-009 commits.

Blockers and unblock actions: independent review is pending; this is a required review step, not a scope blocker. The board row is still `planned` because the board is P0-000-owned; P0-000 must synchronize it to `in_review`/the final state. Admitting the proposed rows needs a separately authorized register-integration task; P0-012 cannot edit `catalog-children.csv` or close a gate. A resolution of the vendor count, edition and surface conflicts needs lawful versioned catalogs or vendor clarification, not inference.

Next task / concrete next action: inspect the independent review results, resolve findings, rerun the validator and structural audit, then record acceptance or retain the unmet conditions. After that, request P0-000 to synchronize the board and request a separate register-integration task from the P0-004 integration owner. Do not edit the board or register from P0-012.

Files or commands needed to resume: [the enumeration](../research/fitting-and-peak-catalog-enumeration.md), [P0-003 record](p0-003-catalog-decomposition.md), [P0-005 record](p0-005-catalog-evidence-gaps.md), [P0-005 evidence](../research/catalog-evidence-gaps-2026b.md), [catalog register](../planning/catalog-children.csv), [functional scope](../planning/functional-scope.md), and `python3 tools/validate_planning.py` from the repository root.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user requested execution of P0-012 on 2026-10-31; scope limited to the new research document and this record. |
| 2026-10-31 | in_progress -> in_review | Candidate enumeration, validator PASS, structural audit and gate/unknown records are ready; two independent read-only reviews are assigned and pending. |
