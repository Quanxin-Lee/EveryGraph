# P1-012: Package the macOS Shell and Prove Hardened-Runtime LGPL Replacement

## Metadata

- Task ID: P1-012
- Status: blocked
- Owner: EveryGraph Platform/Release
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P1-003; P1-006; P1-007; P1-008; P1-011
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. P1-003 is `in_progress`; the shell tasks are not accepted.
- Authorization: not granted. Required: create packaging scripts and install/bundle rules, build a local test bundle, apply a local hardened-runtime signature, run the LGPL replacement experiment, update the licensing and development documents and this record. No Developer ID identity, notarization credentials, clean-machine Gatekeeper claim, release publication, App Store submission or secret storage in the repository.

## Scope and acceptance

Objective: close the packaging gap that does not require external credentials. P1-001 proved ad-hoc deployment only. This task produces a reproducible bundle with the pinned dynamic Qt, applies hardened runtime with a local signature, and tests whether a replaced LGPL Qt library still loads under library validation. P1-009 remains the separate Developer ID, notarization and clean-machine qualification.

Allowed files and interfaces: new `packaging/` scripts and resources, `CMakeLists.txt` and `CMakePresets.json` only for install/bundle hooks, [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), `docs/engineering/sbom/` only for bundle-correspondence evidence, and this record. Application and module source code are read-only.

Out of scope: Developer ID signing, notarization, stapling, Gatekeeper clean-machine acceptance, release publication, auto-update, crash reporting, Windows packaging, a legal opinion, and any claim that the LGPL replacement route is release-ready.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A documented script produces a reproducible `.app` bundle from the pinned dynamic Qt graph without Homebrew or local build paths leaking into the bundle | Not run |
| AC-2 | `codesign --verify --deep --strict` passes on a locally signed, hardened-runtime bundle with the declared entitlements | Not run |
| AC-3 | The bundled Qt libraries are loaded from inside the bundle, and a replaced LGPL Qt library either loads under hardened runtime/library validation or the exact blocking interaction is documented | Not run |
| AC-4 | License notices, corresponding-source/build-material obligations and the exact Qt version/source are recorded from the actual bundle | Not run |
| AC-5 | The SBOM or a bundle inventory corresponds to the packages and versions actually present; no secret or private path is recorded | Not run |
| AC-6 | The development and licensing documents record exact commands, results and limits; no Developer ID, notarization, clean-machine or release claim is made | Not run |
| AC-7 | The five macOS presets and the planning validator remain green; no application source or another task's record is changed | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P1-012 and created this initial record scaffold; no packaging work started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No packaging script, bundle, signature or replacement result exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. Qt must come from the P1-003 pinned dynamic graph, not Homebrew.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time packaging inspection | No `packaging/` directory or install/bundle rule exists; P1-001 produced only an ad-hoc local probe bundle | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | P1-003's dynamic graph and SBOM are not accepted; P1-006/P1-007/P1-008 are blocked | The task cannot start before the shell and pinned Qt are available |

Checks not run and their reasons: bundle creation, hardened-runtime signing, library validation, replacement, notice generation, SBOM correspondence and clean-environment launch. They require the shell, the pinned Qt graph and authorization.

## Review and disposition

Reviewer and review type: independent release-engineering and license/compliance review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked on the shell and pinned Qt graph.

## Handoff and remaining work

Last verified state: no packaging scripts or hardened-runtime evidence exists. P1-001 tested an ad-hoc probe bundle and a local replacement experiment only; P1-009 holds the credential-dependent half.

Blockers and unblock actions: P1-003 must pin the dynamic Qt graph, and P1-006/P1-007/P1-008 must produce and qualify the shell. The user must authorize packaging work. Do not claim Gatekeeper, notarization or release readiness from this task; P1-009 requires the Developer ID identity and a clean machine.

Next task / concrete next action: after the dependencies are accepted, build the bundle from the pinned graph, sign it locally with hardened runtime, test library validation and LGPL replacement, record notices and SBOM correspondence, and request review. P1-009 then performs Developer ID signing, notarization and clean-machine qualification.

Files or commands needed to resume: [P1-001 record](p1-001-macos-qt-baseline.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [licensing policy](../engineering/licensing-and-dependencies.md), [development guide](../engineering/development.md), [P1-009 record](p1-009-macos-packaging-lgpl.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 to separate credential-free packaging from P1-009; the shell and pinned Qt graph are not accepted |
