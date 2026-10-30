# P0-013: Establish the compatibility rights and specification evidence register

## Metadata

- Task ID: P0-013
- Status: in_progress
- Owner: EveryGraph Research
- Record writer: EveryGraph Research (main agent; sole writer of this record)
- Related requirements: EG-ADD-014
- Dependencies: P0-003;P0-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main` at `1646259` plus P0-000's uncommitted board and record update on 2026-10-31. Uncommitted files owned by P1-003, P2-001 and P0-009 through P0-013 exist in the same worktree and are excluded from this task's edits. No new branch or worktree was created.
- Authorization: the user requested execution of P0-013 on 2026-10-31 in the existing `main` worktree, without creating another branch, and authorized assigning research, implementation and independent review work. Authorized scope: create `docs/research/compatibility-rights-and-specifications.md` and edit this record only. Citation-only retrieval of lawful public vendor documentation is authorized; no SDK/manual/EULA acceptance, no account creation, no purchase. The task board is P0-000-owned and read-only to this task under AGENTS.md section 0; the register, catalog children, inventories, functional scope, architecture documents, other task records and product code are read-only. No dependency change, Windows execution, Git commit, push or publication is authorized. External legal review is an open dependency, not performed here.

## Scope and acceptance

Objective: create the versioned evidence register for native and foreign compatibility gates, separating functional, data, formula and ABI compatibility and recording rights, specification availability, lawful fixture plans and uncertainty for every family.

Allowed files and interfaces: a new `docs/research/compatibility-rights-and-specifications.md` and this record. The register, catalog children, inventories, functional scope, architecture documents and every other task record are read-only.

Out of scope: compatibility implementation, vendor SDK or manual acquisition without lawful access, legal advice, product code, dependency changes, Windows execution, push or publication.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The register covers OPJ/OPJU, dgraph, DTG, legacy Plot, Prism, LabTalk, Origin C, XOP-like ABIs, vendor Apps and COM, MATLAB, Mathematica, LabVIEW, R/Rserve and the documented proprietary connectors. | Not run |
| AC-2 | Each family records functional, data, formula and ABI compatibility separately with source, version, rights status, specification availability, lawful fixture plan and uncertainty. | Not run |
| AC-3 | Every family without lawful evidence or rights is explicitly unresolved and no support or round-trip claim is made. | Not run |
| AC-4 | External legal review is recorded as an open dependency; the task names the owner and the next evidence action for each family. | Not run |
| AC-5 | The planning validator passes and only the new research document and this record are edited. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P0-013 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | EveryGraph Research / executor | Read the board, this record, P0-003 and P0-005, the catalog children, the inventories, the functional scope, the project-file-format contract and the licensing policy; started the task and assigned read-only research | This record; the sources listed under files needed to resume |

This record is an allocation plan plus the executor's running work log, not completed work.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The task is a documentation or contract correction and does not build product code.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source and dependency inspection | Confirmed that functional scope section 6 records OPJ/OPJU, dgraph, DTG, legacy Plot, Prism, LabTalk/Origin C, XOP-like ABI, vendor Apps/COM, licensed engines and R/Rserve as unresolved; no task-level rights register exists. | Read-only check; not a test of the task |

Checks not run and their reasons: the corrective edit, post-edit validator and test runs. They require an executor and authorization.

## Review and disposition

Reviewer and review type: independent review is required for this task. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not authorized.

## Handoff and remaining work

Last verified state: The target files are unchanged and the residual described in the objective is present.

Blockers and unblock actions: None; the dependencies are accepted. The user must assign an executor and authorize the allowed edits.

Next task / concrete next action: Publish the rights and specification register, then request a product-scope decision for any family whose rights or specifications remain unavailable. No compatibility pack may start before that decision.

Files or commands needed to resume: [functional scope section 6](../planning/functional-scope.md), [catalog children](../planning/catalog-children.csv), [P0-003 record](p0-003-catalog-decomposition.md), [P0-005 record](p0-005-catalog-evidence-gaps.md), [roadmap section 7](../planning/development-roadmap.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 at the user's request; the task is startable and its dependencies are accepted. |
| 2026-10-31 | planned -> in_progress | The user requested execution of P0-013 on 2026-10-31 in the existing `main` worktree, without a new branch, and authorized the research, implementation and independent review work within the allowed files. |
