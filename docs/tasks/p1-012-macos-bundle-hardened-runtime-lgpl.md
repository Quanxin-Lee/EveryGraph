# P1-012: Package the macOS Shell and Prove Local Ad-Hoc LGPL Replacement

## Metadata

- Task ID: P1-012
- Status: blocked
- Owner: EveryGraph Platform/Release
- Record writer: not yet assigned. The initial scaffold was written by P0-000 at allocation, which also revised it on 2026-10-31 to absorb the canceled P1-009 while no executor existed
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P1-003; P1-006; P1-007; P1-008; P1-011
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. P1-003 and the command kernel are accepted; the shell tasks are not.
- Authorization: not granted. Required: create packaging scripts and install/bundle rules, build a local test bundle, apply a local ad-hoc hardened-runtime signature, run the LGPL replacement experiment, record the open-source rebuild route, update the licensing and development documents and this record. No Developer ID identity, notarization credentials, stapling, Gatekeeper clean-machine claim, release publication, App Store submission or secret storage in the repository.

## Scope and acceptance

Objective: this is the only macOS packaging route for the shell. P1-001 proved ad-hoc deployment only. The task produces a reproducible bundle with the pinned dynamic Qt, applies hardened runtime with a local ad-hoc signature, and tests whether a replaced LGPL Qt library still loads under library validation. Because the project is open source and runs only on the machine that builds it, the task also records what a recipient needs to rebuild the same bundle from source and substitute that library; per the 2026-10-31 decision no Apple developer identity, notarization, stapling or Gatekeeper clean-machine qualification is in scope. The task absorbed the former P1-009 on that date: the credential-dependent half of that plan was decided out of scope rather than deferred, and its remaining checks are this record's criteria.

Allowed files and interfaces: new `packaging/` scripts and resources, `CMakeLists.txt` and `CMakePresets.json` only for install/bundle hooks, [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), `docs/engineering/sbom/` only for bundle-correspondence evidence, [roadmap](../planning/development-roadmap.md) only for the P1 exit wording that P0-000 designated to this task on 2026-10-31, and this record. Application and module source code are read-only.

Out of scope: Developer ID signing, notarization, stapling, Gatekeeper clean-machine acceptance, App Store rules, release publication, auto-update, crash reporting, Windows packaging, a legal opinion, and any claim that the LGPL replacement route is release-ready. The canceled [P1-009](p1-009-macos-packaging-lgpl.md) must not be revived inside this task.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A documented script produces a reproducible `.app` bundle from the pinned dynamic Qt graph without Homebrew or local build paths leaking into the bundle | Not run |
| AC-2 | `codesign --verify --deep --strict` passes on a locally ad-hoc signed, hardened-runtime bundle with the declared entitlements | Not run |
| AC-3 | The bundled Qt libraries are loaded from inside the bundle, and a replaced LGPL Qt library either loads under hardened runtime/library validation or the exact blocking interaction is documented | Not run |
| AC-4 | License notices, corresponding-source build-material obligations and the exact Qt version and source are recorded from the actual bundle | Not run |
| AC-5 | The SBOM or a bundle inventory corresponds to the packages and versions actually present; no secret or private path is recorded | Not run |
| AC-6 | The development and licensing documents record exact commands, results and limits; no Developer ID, notarization, clean-machine or release claim is made | Not run |
| AC-7 | The five macOS presets and the planning validator remain green; no application source or another task's record is changed | Not run |
| AC-8 | A recipient can rebuild the shell from the recorded source and the pinned graph and substitute the bundled LGPL Qt library without relinking; the open-source self-build route and corresponding-source pointers are recorded, and no Developer ID, notarization, stapling, Gatekeeper clean-machine or publication result is claimed | Not run |

AC-8 is the obligation absorbed from the canceled P1-009. It is not a signing or notarization check: it proves that the distribution route the project actually uses, open source built locally, remains reproducible and that the LGPL substitution right survives it.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P1-012 and created this initial record scaffold; no packaging work started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | P0-000 / allocation executor, on direct user instruction | Revised this scaffold to absorb the canceled [P1-009](p1-009-macos-packaging-lgpl.md) after the user decided the project is open source, local-only and signed only ad-hoc: added AC-8, restated the objective and authorization without any credential requirement, and added the roadmap to the allowed files as the P1-exit wording revision P0-000 designated. No executor existed, so this is allocation under section 0 rule 2 | [P1-009 record](p1-009-macos-packaging-lgpl.md); board rows P1-009, P1-012 and P1-013 |

No packaging script, bundle, signature or replacement result exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. Qt must come from the P1-003 pinned dynamic graph, not Homebrew.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time packaging inspection | No `packaging/` directory or install/bundle rule exists; P1-001 produced only an ad-hoc local probe bundle | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | P1-003 and the command kernel are accepted; P1-006, P1-007, P1-008 and P1-011 are not | The task cannot start before the shell and pinned Qt are available |
| 2026-10-31 | Allocation revision absorbing the canceled P1-009 | The user decided open source, local-only and local ad-hoc signing only, so the Developer ID, notarization and clean-machine criteria were canceled with P1-009 and their surviving checks are AC-1 to AC-8 here | [P1-009 record](p1-009-macos-packaging-lgpl.md); board rows P1-009, P1-012 and P1-013. No packaging command was run |

Checks not run and their reasons: bundle creation, hardened-runtime signing, library validation, replacement, notice generation, SBOM correspondence, the open-source rebuild route and a launch from outside the build tree. They require the shell, the pinned Qt graph and authorization.

## Review and disposition

Reviewer and review type: independent release-engineering and license/compliance review is required, and it must cover AC-8's open-source rebuild and replacement route rather than only the bundle. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked on the shell.

## Handoff and remaining work

Last verified state: no packaging scripts or ad-hoc hardened-runtime evidence exists. P1-001 tested an ad-hoc probe bundle and a local replacement experiment only. The former P1-009 is canceled and merged here, so no credential-gated packaging task remains.

Blockers and unblock actions: P1-006, P1-007, P1-008 and P1-011 must produce and qualify the shell, and the user must authorize packaging work. Do not claim Gatekeeper, notarization or release readiness from this task; no Apple credential is needed any longer, so nothing external blocks it once the shell exists.

Next task / concrete next action: after the dependencies are accepted and the task is authorized, build the bundle from the pinned graph, sign it locally ad-hoc with hardened runtime, test library validation and LGPL replacement, record notices, SBOM correspondence and the open-source self-build route (AC-8), correct the P1-exit wording in the roadmap and the two engineering documents where they still describe notarization as a release plan, and request independent review.

Files or commands needed to resume: [P1-001 record](p1-001-macos-qt-baseline.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [licensing policy](../engineering/licensing-and-dependencies.md), [development guide](../engineering/development.md), [roadmap](../planning/development-roadmap.md), [P1-009 record](p1-009-macos-packaging-lgpl.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 to separate credential-free packaging from P1-009; the shell and pinned Qt graph are not accepted |
| 2026-10-31 | Scope expanded to absorb the canceled P1-009; status stays blocked | Direct user instruction that the project is open source, local-only and signed only ad-hoc, plus the user's instruction to merge P1-009. The Developer ID, notarization and clean-machine criteria were dropped, and the surviving obligations became AC-1 to AC-8 |
