# P1-009: Qualify macOS Signed Packaging, Hardened Runtime and LGPL Replacement

## Metadata

- Task ID: P1-009
- Status: blocked
- Owner: EveryGraph Platform/Release
- Record writer: EveryGraph Platform/Release
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P1-003; P1-006
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated release branch or worktree must be created after the dependencies are accepted.
- Authorization: not granted. Required: Apple Developer signing identity, notarization credentials, a clean test machine or VM, packaging scripts and documentation. No release publication, App Store submission, paid distribution or credential storage in the repository is authorized.

## Scope and acceptance

Objective: qualify a macOS test bundle that is signed, hardened, notarized and usable on a clean machine, and prove that the LGPL Qt replacement right survives the hardened-runtime and library-validation configuration. This is a release-engineering qualification, not a public release.

P1-001 proved local ad-hoc deployment and replacement. It did not test Developer ID signing, hardened runtime, library validation, notarization, stapling or a clean-machine installation. This task closes those gaps for the shell.

Allowed files and interfaces: packaging scripts under `packaging/`, `CMakeLists.txt`/presets only for packaging hooks, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md`, an SBOM/notice artifact directory, a test bundle under ignored output and this record. No application business logic is changed.

Out of scope: Windows packaging, Mac App Store submission, paid distribution, auto-update, crash-reporting services, a final legal opinion and any claim that the project is release-ready.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A Developer ID signed test bundle is produced with hardened runtime and the documented entitlements; `codesign --verify --deep --strict` passes | Not run |
| AC-2 | The bundle is notarized and stapled; Gatekeeper accepts it on a clean macOS test environment with no developer tools or quarantine bypass | Not run |
| AC-3 | The bundled Qt frameworks are dynamically linked from the bundle, and a modified LGPL Qt library can replace the bundled copy under hardened runtime/library validation, or the exact blocking interaction is documented | Not run |
| AC-4 | License notices, corresponding-source/build-material obligations and the Qt version/source are recorded from the actual bundle | Not run |
| AC-5 | A clean-machine smoke test launches the shell and exercises the documented command smoke path without Homebrew or local build paths | Not run |
| AC-6 | The development and licensing documents record exact commands, certificate/notarization evidence (without secrets), results and limits; no release publication occurs | Not run |
| AC-7 | If credentials or a clean machine are unavailable, the task remains blocked and no signing, notarization or clean-machine claim is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit and P1-001 packaging limits; no signing or packaging work started | [P1-001 record](p1-001-macos-qt-baseline.md), [licensing policy](../engineering/licensing-and-dependencies.md) and [board](../planning/task-board.csv) |

No signed bundle, notarization submission, clean-machine result or notice artifact exists for this task. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; Homebrew Qt 6.12.0_2 from P1-001 is provisional. No Developer ID identity, notarization credentials or clean test machine was supplied or inspected.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | P1-001 deployment evidence review | Ad-hoc signed local bundle and LGPL replacement passed; hardened runtime, library validation, Developer ID and notarization were not tested | Local deployment only, not packaging qualification |
| 2026-10-30 | Credential and clean-machine availability | Not supplied | The task cannot start signing or notarization |
| 2026-10-30 | `python3 tools/validate_planning.py` | Passed: 35 documents | Documentation baseline only; no packaging test |

Checks not run and their reasons: Developer ID signing, hardened runtime, library validation, notarization, stapling, Gatekeeper clean-machine testing, notice/source artifact generation and replacement under hardened runtime. They require credentials, a clean machine and the accepted P1-003/P1-006 dependencies.

## Review and disposition

Reviewer and review type: independent release-engineering review plus a license/compliance review are required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: P1-001 produced an ad-hoc signed probe bundle and a local LGPL replacement experiment. No Developer ID, hardened runtime, notarization, Gatekeeper or clean-machine evidence exists.

Blockers and unblock actions:

- Provide an Apple Developer signing identity and notarization credentials through the approved secret mechanism; never store them in the repository or task record.
- Provide a clean macOS test machine or VM with no developer tools and no quarantine bypass.
- P1-003 must pin the Qt graph and produce the actual dependency/SBOM artifacts; P1-006 must produce the shell bundle.
- Decide whether the distribution route requires App Store rules or direct Developer ID distribution; that decision affects entitlements and replacement testing.

Next task / concrete next action: after P1-003/P1-006 and credential authorization, build the signed test bundle, notarize/staple it, run the clean-machine smoke test, repeat the LGPL replacement experiment under hardened runtime, and record the outcome for review.

Files or commands needed to resume: [P1-001 record](p1-001-macos-qt-baseline.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-003/P1-006 are not accepted and no signing/notarization credentials or clean machine are available |
