# P0-004: Integrate Catalog Child Entries into a Separate Register and Validator

## Metadata

- Task ID: P0-004
- Status: planned
- Owner: EveryGraph Research
- Record writer: EveryGraph Research
- Related requirements: ORG-IO-001; ORG-GRAPH-001; ORG-FIT-004; ORG-PEAK-005; ORG-EXPORT-001; EG-ADD-014; EG-ADD-015
- Dependencies: P0-003
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`; P0-003 is uncommitted. A dedicated branch or worktree must be created before this task starts.
- Authorization: not granted. Required: create a separate catalog-child register and validator checks, update the planning documents and tests, and update this record and the board. No changes to the 428-row traceability register, no product code and no vendor claim are authorized.

## Scope and acceptance

Objective: turn the reviewed P0-003 proposal into a durable, separate planning register for catalog children, with executable checks for identifiers, anchors, counts and rights status. The register remains an evidence index, not a feature-delivery or parity claim.

Allowed files and interfaces: a new `docs/planning/catalog-children.csv`, a new or extended validator under `tools/`, validator tests, the P0-003 evidence document, this record and the board. The 428-row `docs/planning/feature-traceability.csv`, `docs/planning/functional-scope.md`, the preserved inventories and product/build files are read-only for this task.

Out of scope: inserting child IDs into the feature register, changing the 428-row count or its status contract, implementing any catalog capability, acquiring vendor manuals or licenses, and treating vendor counts (30+, 100+, close to 200, 25+, 17) as acceptance thresholds.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A separate CSV implements the P0-003 columns and contains reviewed rows for all 149 proposed child IDs (36 import, 74 graph, 2 fit, 1 peak, 17 export, 13 compatibility, 6 gate) | Not run |
| AC-2 | Every child ID is unique and matches `EG-CAT-<AREA>-<NNN>`; every parent exists in the register and remains `unresolved` | Not run |
| AC-3 | Every named row has a verifiable local or external source anchor and excerpt hash; claim-only and blocked rows are explicit and are not counted as named deliveries | Not run |
| AC-4 | Validator tests enforce ID grammar, parent integrity, required fields, count floors, claim-only counts, no rights elevation and register immutability | Not run |
| AC-5 | The 428-row register and its validator remain unchanged; the planning validator, new checks and existing tests pass; external URLs are not fetched in the default run | Not run |
| AC-6 | Documentation states that child rows are evidence entries, not implementation, and that the seven parents and `EG-ADD-015` remain unresolved until their closure conditions are met | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the second takeover audit; no register, validator or test was changed | [Takeover audit](p0-002-takeover-state-verification.md), [P0-003 evidence](p0-003-catalog-decomposition.md) and [board](../planning/task-board.csv) |

No child CSV, validator change or test exists for this task. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Python 3.13.9 (Anaconda) and Python 3.14.2 available; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. This task requires only Python and documentation tools.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `python3 tools/validate_planning.py` on the pre-task state | Passed: 339 original IDs, 428 traced requirements, 35 documents | Confirms the baseline before any register change; does not validate the future child CSV |
| 2026-10-30 | Review of the P0-003 proposal and child-ID count | 149 unique `EG-CAT-*` IDs identified | Counts are evidence floors from the candidate document, not a completeness claim |
| 2026-10-30 | No child CSV or catalog validator | Not found | The integration task has not started |

Checks not run and their reasons: child-register parsing, anchor verification, count assertions, validator tests and register immutability checks. They require the authorized implementation.

## Review and disposition

Reviewer and review type: independent review is required for the new register and validator. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is planned and not authorized to start.

## Handoff and remaining work

Last verified state: the P0-003 evidence document contains 620 lines and 149 proposed child IDs, but no separate register, validator extension or test exists. The 428-row register remains unchanged.

Blockers and unblock actions: P0-003 must pass independent review and be accepted before integration. The user must authorize changes to planning files and the validator. If the schema or count floors need revision, that decision belongs in this task and must not be made by silently altering the evidence document.

Next task / concrete next action: after P0-003 acceptance and authorization, create the CSV from reviewed rows, implement the checks, run the existing and new tests, and request independent review. P0-005 remains the evidence-closure task for the catalogs whose member lists are unavailable.

Files or commands needed to resume: [P0-003 evidence](../research/capability-catalog-decomposition.md), [feature traceability](../planning/feature-traceability.csv), [validator](../../tools/validate_planning.py), [validator tests](../../tools/test_validate_planning.py), [recording guide](README.md), and `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> planned | Second takeover decomposition; P0-003 candidate exists but integration is not authorized |
