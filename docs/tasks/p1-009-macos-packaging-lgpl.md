# P1-009: Canceled — macOS Signed Packaging Qualification Merged into P1-012

## Metadata

- Task ID: P1-009
- Status: canceled
- Owner: EveryGraph Platform/Release
- Record writer: P0-000 at allocation; no executor was ever assigned
- Related requirements: EG-ADD-016; DG-PERF-004, both carried on the P1-012 row after the merge
- Dependencies: superseded by the merge; P1-012 absorbs the work and depends on P1-003;P1-006;P1-007;P1-008;P1-011
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: none; no branch or worktree was created for this task
- Authorization: none required and none granted. The task is canceled; the historical plan below must not be executed under this ID.

## Scope and acceptance

Canceled on 2026-10-31 by P0-000 at the user's direct instruction. The project is open source and runs only on this machine, so local ad-hoc signing is the only signature policy: no Apple developer identity, no notarization, no stapling and no Gatekeeper clean-machine qualification. This section preserves the superseded plan as history; it is not work to be done.

Historical objective, superseded: qualify a macOS test bundle that is signed, hardened, notarized and usable on a clean machine, and prove that the LGPL Qt replacement right survives the hardened-runtime and library-validation configuration.

Where the obligations went: [P1-012](p1-012-macos-bundle-hardened-runtime-lgpl.md) produces the reproducible bundle from the pinned dynamic Qt, applies hardened runtime with a local ad-hoc signature, proves replacement under library validation, records notices and SBOM correspondence, and records the open-source self-build and corresponding-source route as its AC-8, added when this task was canceled. No obligation and no requirement ID was dropped: `EG-ADD-016` and `DG-PERF-004` stay on the P1-012 board row, and P1-013 no longer depends on this ID.

| Criterion | Required evidence (historical plan) | Result |
|---|---|---|
| AC-1 | Developer ID signed test bundle with hardened runtime and the documented entitlements; `codesign --verify --deep --strict` passes | Not run, canceled. A Developer ID identity is outside the decided policy |
| AC-2 | The bundle is notarized and stapled and Gatekeeper accepts it on a clean machine | Not run, canceled. Notarization and the clean machine are outside the decided policy |
| AC-3 | Bundled Qt frameworks load from the bundle, and a modified LGPL Qt library can replace the bundled copy under hardened runtime and library validation, or the exact blocking interaction is documented | Moved to P1-012 AC-3, with the local ad-hoc signature |
| AC-4 | Notices, corresponding-source and build-material obligations and the Qt version and source are recorded from the actual bundle | Moved to P1-012 AC-4 and AC-5 |
| AC-5 | A clean-machine smoke test launches the shell without Homebrew or local build paths | Moved to P1-012 AC-1 as a launch from a copied location outside the build tree, and to P1-012 AC-8 as the open-source rebuild route |
| AC-6 | The development and licensing documents record exact commands, evidence without secrets, results and limits | Moved to P1-012 AC-6, which keeps the no-Developer-ID, no-notarization and no-clean-machine limit |
| AC-7 | If credentials or a clean machine are unavailable the task stays blocked and claims nothing signed, notarized or clean-machine tested | Withdrawn with the task. The condition it guarded was decided rather than left pending |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit and P1-001 packaging limits; no signing or packaging work started | [P1-001 record](p1-001-macos-qt-baseline.md), [licensing policy](../engineering/licensing-and-dependencies.md) and [board](../planning/task-board.csv) |
| 2026-10-31 | P0-000 / allocation executor, on direct user instruction | Canceled the task and merged it into [P1-012](p1-012-macos-bundle-hardened-runtime-lgpl.md): rewrote this record to the canceled state, removed this ID from the P1-013 dependency list on the board, added P1-012 AC-8, and set the board row to `canceled`. No executor existed, so the revision is allocation under section 0 rule 2 rather than a cross-task edit | [P1-012 record](p1-012-macos-bundle-hardened-runtime-lgpl.md), [roadmap](../planning/development-roadmap.md), board rows P1-009, P1-012 and P1-013 |

No bundle, signature, notarization submission or notice artifact was produced under this ID, and none will be.

## Validation

Working directory and environment at the cancellation: `/Users/leo/Documents/General/EveryGraph`, macOS 27.0.1 arm64. No packaging, signing or notarization command was ever run for this task.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | P1-001 deployment evidence review | Ad-hoc signed local bundle and LGPL replacement passed; hardened runtime, library validation, Developer ID and notarization were not tested | Local deployment only, not packaging qualification |
| 2026-10-30 | Credential and clean-machine availability | Not supplied | The task could not have started signing or notarization |
| 2026-10-30 | `python3 tools/validate_planning.py` | Passed: 35 documents | Documentation baseline only; no packaging test |
| 2026-10-31 | Cancellation consistency: read-only CSV parse of the board, `python3 tools/validate_planning.py` and `python3 -B tools/validate_task_manifest.py` after the merge | The board row and this record both read `canceled`, no remaining dependency names this ID, and neither validator gained a finding for this task | No packaging command was run because the task is canceled |

Checks not run and their reasons: Developer ID signing, hardened runtime under a Developer ID, library validation, notarization, stapling, Gatekeeper clean-machine testing and notice generation under this ID. They were decided out of scope on 2026-10-31, and the packaging checks that remain belong to P1-012.

## Review and disposition

Reviewer and review type: none, and none is required. Cancellation is an allocation act by P0-000 under the user's direct 2026-10-31 instruction, not an acceptance: no acceptance criterion was met, tested or claimed, and rule 11 does not apply to a canceled task because there is no deliverable to review.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: the superseded plan needed external credentials and a clean machine that were never available. The user resolved that condition by deciding the project is open source and local-only, which is why the task is canceled rather than still blocked.

Acceptance decision, accepting role and date: canceled by P0-000 on 2026-10-31 at the user's instruction. Not accepted, no deliverable exists, and the only phase-level effect is that P1-013 no longer lists this ID as a dependency.

## Handoff and remaining work

Last verified state: canceled. Nothing is in progress under this ID, no branch or worktree exists, and no artifact was produced.

Blockers and unblock actions: none. The previous blockers were the Apple developer identity, the notarization credentials and a clean machine; the 2026-10-31 decision removed the requirement for all three instead of satisfying them.

Next task / concrete next action: none under P1-009. The successor is [P1-012](p1-012-macos-bundle-hardened-runtime-lgpl.md), which produces the locally ad-hoc signed bundle and records the open-source rebuild and replacement route. Do not recreate this task unless the distribution policy returns to signing identities or notarization, which would need a new user decision and a new allocation.

Files or commands needed to resume: [P1-012 record](p1-012-macos-bundle-hardened-runtime-lgpl.md), [licensing policy](../engineering/licensing-and-dependencies.md), [roadmap](../planning/development-roadmap.md), [board](../planning/task-board.csv).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-003 and P1-006 were not accepted, and no signing or notarization credentials or clean machine were available |
| 2026-10-31 | Rescoped on the board from Developer ID and notarization qualification to local ad-hoc open-source qualification; status stayed blocked | Direct user instruction that the project is open source, does no signing, runs only on this machine and uses local ad-hoc signing throughout |
| 2026-10-31 | blocked -> canceled and merged into P1-012 | Same user decision plus the user's merge instruction. The residual verification duties fit P1-012's own packaging evidence, so a separate task and an extra serial step added no coverage, and the user explicitly authorized this record revision |
