# P0-003: Decompose Named Capability Catalogs into Evidence-Backed Child Entries

## Metadata

- Task ID: P0-003
- Status: planned
- Owner: EveryGraph Research
- Record writer: EveryGraph Research
- Related requirements: ORG-IO-001; ORG-GRAPH-001; ORG-FIT-004; ORG-PEAK-005; ORG-EXPORT-001; EG-ADD-014; EG-ADD-015
- Dependencies: none
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not applicable; no Git repository exists and the task has not started
- Authorization: not yet granted for execution. When authorized, research and documentation only; no product implementation, no changes to the 428-row register or validator, no dependency installation, Git initialization, commit, push or publication.

## Scope and acceptance

Objective: turn the named catalog umbrellas into evidence-backed enumerations and a proposed stable child-ID schema so later implementation tasks can be scoped without inventing missing capabilities.

Allowed files and interfaces when authorized: a new evidence document under `docs/research/` and this task record. No changes to `origin-feature-inventory.md`, `datagraph-feature-inventory.md`, `docs/planning/feature-traceability.csv`, `docs/planning/functional-scope.md`, `tools/validate_planning.py`, product code or build configuration.

Out of scope: integrating child rows into the authoritative register, changing validator counts or status semantics, creating development tasks for individual features, implementing any feature, claiming catalog completeness or parity, acquiring proprietary manuals or SDKs without separate authorization.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Each listed requirement family has a section with source/manual/version citations and explicit unknowns | Not run |
| AC-2 | Named examples and discovered variants receive proposed stable child IDs plus parent, parameters, prerequisites, rights status, delivery class, fixture plan, expected output and uncertainty | Not run |
| AC-3 | No invented catalog names or parameters; unavailable or rights-blocked catalogs are marked as such rather than guessed | Not run |
| AC-4 | A validator integration proposal defines the child-ID namespace, count expectations, source-anchor rules and test changes without modifying the current register or validator | Not run |
| AC-5 | Proposed evidence document passes the existing planning validator, local links and English-only policy | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Identified the catalog gap and created this scoped task; no research execution started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |

No research, catalog entries or code changes have been produced. This record defines a planned task, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: not applicable yet; the task has not started.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | None | Not run | This is a planned task; no catalog evidence or validator change exists |

Checks not run and their reasons: all research and documentation checks. The existing Phase-0 validator was last run by P0-002 before this task was created; its current green state does not validate this planned scope.

## Review and disposition

Reviewer and review type: independent EveryGraph Review will be required when a candidate evidence document exists. No reviewer has been assigned and no review has occurred.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; this task is planned and not authorized to start.

## Handoff and remaining work

Last verified state: no catalog decomposition document exists; the feature register still has 428 rows with 41 `unresolved` entries; the catalog-expansion gate remains open.

Blockers and unblock actions: execution requires user authorization to start the research and write the new evidence document. Access to vendor manuals or proprietary specifications requires separate evidence and rights review; unavailable sources remain explicit gaps.

Next task / concrete next action: after authorization, Research reads the inventories, preserved source evidence and obtainable public manuals, then produces the evidence document and proposed child-ID schema. A separate integration task must be authorized before changing the traceability register or validator.

Files or commands needed to resume: [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), the two root inventories, [recording guide](README.md), and `python3 tools/validate_planning.py` from the project root.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> planned | User requested only the next batch of tasks; catalog research is defined but not authorized to execute |
