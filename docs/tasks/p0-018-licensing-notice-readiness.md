# P0-018: Prepare the licensing, notice and dependency-compliance evidence

## Metadata

- Task ID: P0-018
- Status: planned
- Owner: EveryGraph Lead
- Record writer: EveryGraph Lead
- Related requirements: none — distribution and legal-governance work; the rationale is the roadmap Phase-0 exit clause "explicit legal and licensing gap with owner", [AGENTS.md](../../AGENTS.md) section 3 and the [license policy](../engineering/licensing-and-dependencies.md)
- Dependencies: P1-003
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not applicable until an executor starts; the allocation scaffold is committed by P0-000 on `main`
- Authorization: this record allocates the task only. The executor edits the manifest paths below and produces a decision package. It does **not** change the project's license, publish a `LICENSE` or `NOTICE` file, install a dependency, contact a lawyer or make a legal conclusion; those are the user's acts under AGENTS.md section 0 rule 11 and the license policy

## Scope and acceptance

Objective: [P0-015](p0-015-p0-exit-re-verification.md) recorded the licensing gap as the one Phase-0 exit item with no owner: no `LICENSE` or `NOTICE` file is committed, and formal release license, distribution notices, corresponding-source access and complete dependency compliance remain distribution gates. [P1-003](p1-003-dependency-pins-sbom.md)'s accepted vcpkg graph and SBOM are input, not clearance. Turn that orphaned gap into an owned, reviewable evidence package: a complete dependency-license inventory derived from the accepted SBOM and the manifest, the concrete obligations each license class imposes, and a proposed notice text — held as a proposal for the user's decision rather than committed as a license.

Allowed files and interfaces: the two manifest paths below. The license policy's existing statements of intent and its Phase-0 status are corrected only where they are factually stale; no obligation is weakened or removed.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `docs/engineering/licensing-and-dependencies.md` | modify | P0-000 designates P0-018 as the single writer of this shared normative path for this revision (rule 8) |
| `docs/tasks/p0-018-licensing-notice-readiness.md` | modify | Own task record |

Out of scope: creating `LICENSE`, `NOTICE`, `COPYING` or any other license text at the repository root (a rule 9 request to the user, not an edit); changing any dependency, version, license, minimum platform or architecture; `vcpkg.json`, `vcpkg-configuration.json`, the SBOM and the `docs/engineering/sbom/` tree (P1-003's accepted artifacts, read-only here); `.github/workflows/*`; an external legal review, any paid resource and any vendor contact; a conclusion about whether a use is lawful.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A dependency-license inventory covers every component in the accepted SBOM and every manifest dependency, with each row's source being the committed artifact it was read from; the record states the component count and how it reconciles with P1-003's count | Not run |
| AC-2 | Obligations are stated per license class actually present (at minimum the Qt LGPLv3 obligations — dynamic linking, relinking and corresponding-source access — and the permissive notices), each traceable to a cited license text or official page, with the retrieval date recorded | Not run |
| AC-3 | EveryGraph-owned code's intended license is restated exactly as the policy states it, with an explicit statement that this task neither chose nor changed it | Not run |
| AC-4 | A proposed `NOTICE` text and a proposed `LICENSE` choice are written **inside this record** as a proposal, not committed as files, with the rule 9 request that names the target paths and asks the user to decide | Not run |
| AC-5 | The document's Phase-0 status is accurate after the edit: what is evidenced, what remains a distribution gate, and that no legal review has occurred; no support, release-readiness or clearance claim is added | Not run |
| AC-6 | Only the two manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; `tools/validate_task_manifest.py` or the rule 12 self-check is recorded | Not run |
| AC-7 | `python3 tools/validate_planning.py` passes (the licensing guide is an input to the planning validator, so a factual correction must keep its hashes and links consistent) and the record follows the English policy | Not run |
| AC-8 | An independent reviewer that did not write the inventory checks a sample of rows against the SBOM and confirms AC-3's no-license-change statement | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its two-path change manifest and the writer designation for the licensing guide; the user delegated this allocation decision to P0-000 and no licensing content was written | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-015 record](p0-015-p0-exit-re-verification.md) residual table |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| — | Not run | Not run | The task is `planned`; no executor has started |

## Review and disposition

Reviewer and review type (independent or self-review): required and not yet performed. A reviewer that did not write the inventory must verify it against the committed SBOM and confirm that no license was chosen, changed or published by this task.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): none yet; the task is `planned`. When the executor accepts, it records the row, the state and the proving commit here.

Last verified state: scaffold only. Working directory `/Users/leo/Documents/General/EveryGraph`, branch `main`.

Blockers and unblock actions: none for the evidence work. The **legal review** and the decision to publish a license are outside this task and outside this host: they need the user's instruction and, for the review, an external resource. This task records them as open rather than resolving them.

Next task / concrete next action: on release, read the licensing guide, the accepted P1-003 SBOM and `vcpkg.json` first, then build the inventory before proposing any text.
