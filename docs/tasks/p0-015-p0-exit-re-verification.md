# P0-015: Re-verify the Phase-0 exit gate at the current committed revision

## Metadata

- Task ID: P0-015
- Status: planned
- Owner: EveryGraph Verification
- Record writer: unassigned
- Related requirements: none; this is a gate re-verification of already-traced P0 obligations, not a new capability
- Dependencies: P0-007; P0-009; P0-012; P0-013
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not applicable until an executor is assigned; the executor records the actual `main` revision it reviews at the start of this task. The verification is read-only against committed history, so a dirty shared working tree does not invalidate it as long as the reviewed revision is named and every command is re-run against a stated revision.
- Authorization: allocation by P0-000 on 2026-10-31 in response to the gap P0-007 itself recorded. Releasing this task to an executor authorizes, under AGENTS.md section 0 rule 11, that executor's own manifest-only **local** commits including its acceptance commit. Not authorized: push or any other external action, any edit to another task's record or artifact, any board edit, product or build-code work, dependency installation, Windows or CI execution, signing, publication, or closing a gate the evidence does not close.

## Scope and acceptance

Objective: restate the Phase-0 exit verdict at the revision the project actually stands on. [P0-007](p0-007-p0-exit-verification.md) classified every P0 exit item at `2b1169d` and recorded its principal blocking gap as the stale catalog contract pin, which the accepted P0-009 re-pin, the accepted P0-012 fitting and peak enumeration and the accepted P0-013 compatibility register have since changed. The P0 gate therefore has no verdict at the current committed revision, and P0-000 does not perform the verification itself.

Allowed files and interfaces: this record only. Every other repository path is read-only for this task.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `docs/tasks/p0-015-p0-exit-re-verification.md` | modify | Own task record |

Out of scope: editing the board, `docs/engineering/validation-report.md`, the development guide, `AGENTS.md`, the traceability register, `docs/planning/catalog-children.csv`, `tools/catalog_expectations.json` or any other task's record; allocating or performing the residual items this task names; any Windows, CI, packaging, licensing or release action; changing or softening any P0 exit criterion; and any parity, support or product claim.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The reviewed commit revision is named, and every Phase-0 exit clause of [development roadmap](../planning/development-roadmap.md) section 3 plus every functional-scope P0 obligation is classified pass, partial or gap with the exact artifact or command as evidence; no item is left unclassified | Not run |
| AC-2 | `python3 tools/validate_catalog_children.py` and `python3 -B -m unittest discover -s tools -p 'test_*.py'` are re-run at the named revision, and the record states whether P0-007's principal gap (the `93918a9b...` versus `eb9a9104...` digest mismatch) is closed by the accepted P0-009 re-pin, quoting the digest now pinned | Not run |
| AC-3 | The accepted P0-012 and P0-013 deliverables are checked as committed artifacts, and every catalog gate GATE-001 to GATE-007, the EG-ADD-015 roll-up and the EG-ADD-014 compatibility obligation are kept open where evidence or rights are missing | Not run |
| AC-4 | Every residual item names its owning task or role; items with no allocated task are marked `unassigned` with the allocation owner recorded as P0-000, and this task neither edits nor accepts them | Not run |
| AC-5 | The task writes only its own record: no board row, no other owner's file and no build or product path is modified, proven by the commit's own path list | Not run |
| AC-6 | An independent reviewer that did not perform this verification rechecks the classification and at least the three validation commands, and the record names that reviewer, the reviewed revision, the findings and the recheck before this task sets itself to `accepted` | Not run |
| AC-7 | `python3 tools/validate_planning.py` passes with this record present, and the record keeps the English policy, states no support or platform claim and reports the board-sync request instead of editing the board | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-015 and created this initial record scaffold with its change manifest; no verification performed | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: not run.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|

Checks not run and their reasons: Windows execution, CI runs, remote repository state, dependency qualification and any semantic architecture review. This task verifies documentation and validator evidence only; it does not re-run platform builds and produces no product, compatibility or release claim.

## Review and disposition

Reviewer and review type (independent or self-review): not run.
Reviewed files / revision: not run.
Findings, resolutions and recheck evidence: not run.
Acceptance decision, accepting role and date (or pending): pending; the executor self-accepts only after the independent review above.

## Handoff and remaining work

Commits (each with this task ID in the subject and only manifest paths; state that no commit exists and why when true): none yet; this scaffold is committed by P0-000 under its own allocation scope.
Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): row `P0-015`.
Last verified state: not verified by this record yet.
Blockers and unblock actions: none for this task's own execution; it is dependency-clear because P0-007, P0-009, P0-012 and P0-013 are all accepted.
Next task / concrete next action: an executor starts this task, re-derives the P0 exit matrix at the current revision, and hands its residual `unassigned` items to P0-000 for an allocation decision rather than fixing them itself.
Files or commands needed to resume: [task board](../planning/task-board.csv), [recording guide](README.md), [P0-007 record](p0-007-p0-exit-verification.md), [P0-009 record](p0-009-catalog-contract-repin.md), [P0-012 record](p0-012-fitting-peak-catalog-enumeration.md), [P0-013 record](p0-013-compatibility-rights-evidence.md), [roadmap](../planning/development-roadmap.md), [functional scope](../planning/functional-scope.md), `python3 tools/validate_planning.py`, `python3 tools/validate_catalog_children.py`, `python3 -B -m unittest discover -s tools -p 'test_*.py'`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | Created as `planned` with a single-path manifest | P0-000 allocation. P0-007's next-action line requires an independent P0 re-verification that P0-000 must not perform, and a record-only write scope keeps the task collision-free against P1-011 and P0-014 |
