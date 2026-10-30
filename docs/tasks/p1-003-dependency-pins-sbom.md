# P1-003: Pin the Native Dependency Graph and Produce an Initial SBOM

## Metadata

- Task ID: P1-003
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: none; build, dependency and licensing governance, not a new product capability
- Dependencies: P1-001
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not applicable; no Git repository exists and the task has not started
- Authorization: not granted. Required: vcpkg bootstrap and network downloads; dependency manifest and build-file changes; documentation and SBOM artifact creation; no Windows resolution, publication, signing or license-approval claim beyond the recorded evidence.

## Scope and acceptance

Objective: replace the temporary tool environment and unpinned dependency direction with a reproducible native dependency graph for macOS arm64, including an immutable registry baseline, explicit features/triplets, actual resolved versions, license evidence and an SBOM generated from real build artifacts.

Allowed files and interfaces: `vcpkg.json`, `vcpkg-configuration.json` if needed, `CMakePresets.json`, `CMakeLists.txt` for dependency wiring, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md`, a new `docs/engineering/sbom/` artifact directory and this record.

Out of scope: inventing package versions or a registry baseline before actual resolution; Windows triplet qualification; application business logic; publishing the SBOM or releases; claiming legal approval for Qt, Arrow, Eigen, SQLite or any transitive dependency.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | `vcpkg.json` uses an immutable `builtin-baseline` commit and explicit features; no branch, HEAD or fabricated lock | Not run |
| AC-2 | A clean-state manifest install resolves on macOS arm64 with the declared triplet and records actual package versions | Not run |
| AC-3 | The project and minimal Qt probe configure, build and pass existing tests using the pinned graph | Not run |
| AC-4 | License/notice inventory is updated from actual resolved packages, with transitive and platform-specific gaps stated | Not run |
| AC-5 | An SBOM is generated from actual build artifacts and checked for package/version correspondence; no secrets or private data are included | Not run |
| AC-6 | The development guide records exact commands, environment, results and limits; Windows resolution and release compliance remain open | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created the task and recorded the absent package manager and temporary tooling; no dependency change started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |

No manifest, lock, SBOM or dependency installation exists. This record is a blocked plan.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.2 arm64; CMake 3.31.6 and Ninja 1.11.1 from the temporary `/tmp/everygraph-phase0-tools` environment; vcpkg absent.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `which vcpkg` and package-manager inspection | Not found | No vcpkg manifest, baseline or installed dependency graph exists |
| 2026-10-30 | Existing build/test state | Passed 3/3 in debug, release and sanitize | Dependency-free probe only; no Qt or third-party package is linked |
| 2026-10-30 | Dependency manifest and SBOM inspection | None found | No reproducible package graph or SBOM artifact exists |

Checks not run and their reasons: manifest resolution, clean-state restore, dependency builds/tests, license review and SBOM generation. The task depends on P1-001 and lacks installation authorization.

## Review and disposition

Reviewer and review type: independent EveryGraph Review plus a dedicated license/dependency review will be required. No reviewer has been assigned; no review has occurred.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: the project has no vcpkg manifest, no dependency lock and no SBOM; only the dependency-free compiler/planning scaffold is verified.

Blockers and unblock actions:

- P1-001 must establish the exact Qt patch and package route. Do not pin a Qt version that was not actually qualified.
- vcpkg is absent and network/install authorization is not granted. Authorize bootstrap and package downloads.
- No Git repository exists. Authorize local version control before manifest changes if a reviewable diff is required.
- Windows triplet resolution depends on P1-002 and must not be claimed here.
- License/compliance review may require human legal review; this task can record evidence and open obligations but cannot approve distribution.

Next task / concrete next action: after P1-001 is accepted and installation is authorized, Implementation creates the manifest with an immutable baseline, resolves from a clean state, builds and tests, updates the license inventory and generates an SBOM from actual artifacts for independent Review. P1-002 can then use the settled package route on Windows.

Files or commands needed to resume: [CMake presets](../../CMakePresets.json), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), and the P1-001 record.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-001 dependency not started; vcpkg and network/install authorization absent |
