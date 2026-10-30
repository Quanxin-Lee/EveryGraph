# P1-003: Pin the Native Dependency Graph and Produce an Initial SBOM

## Metadata

- Task ID: P1-003
- Status: in_progress
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: none; build, dependency and licensing governance, not a new product capability
- Dependencies: P1-001 (accepted)
- Created: 2026-10-30
- Updated: 2026-10-31
- Branch / worktree / base revision: checkpoint branch `feat/p1-003-vcpkg-graph` from `fca707a`, committed in the isolated worktree `out/p1-003-checkpoint`. The shared worktree is on `main` at `747809e` with an unresolved stash apply and was left untouched. The checkpoint commit hash is recorded in the following record commit; no push occurred.
- Authorization: user granted the vcpkg/network scope on 2026-10-30; the decision is recorded in [P0-006](p0-006-extra-agent-verification.md) (metadata, contributions and "Recommended first task" sections) and the user instructed this session to execute P1-003. Covers vcpkg bootstrap and package-source/binary downloads, `vcpkg.json`, optional `vcpkg-configuration.json`, CMake preset/CMakeLists dependency wiring, engineering-document updates, an `docs/engineering/sbom/` artifact, and a clean-state configure/build/test. The user authorized a checkpoint commit on 2026-10-31. Push, publication and signing remain unauthorized; Windows resolution and legal approval remain out of scope.

## Scope and acceptance

Objective: replace the temporary tool environment and unpinned dependency direction with a reproducible native dependency graph for macOS arm64, including an immutable registry baseline, explicit features/triplets, actual resolved versions, license evidence and an SBOM generated from real build artifacts.

Allowed files and interfaces: `vcpkg.json`, `vcpkg-configuration.json` if needed, `CMakePresets.json`, `CMakeLists.txt` for dependency wiring, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md`, a new `docs/engineering/sbom/` artifact directory, a new `cmake/vcpkg-overlay-ports/qtbase/` recipe copy for the Qt 6.12.0 version pin, and this record.

Out of scope: inventing package versions or a registry baseline before actual resolution; Windows triplet qualification; application business logic; publishing the SBOM or releases; claiming legal approval for Qt, Arrow, Eigen, SQLite or any transitive dependency.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | `vcpkg.json` uses an immutable `builtin-baseline` commit and explicit features; no branch, HEAD or fabricated lock | Partial: `builtin-baseline` is `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`; features are explicit (`gui`, `widgets`, default features disabled); a `version>= 6.12.0` guard and the overlay pin Qt 6.12.0. The target is `arm64-osx-dynamic` (shared), not the literal static `arm64-osx`; repo-manifest/config resolution has not yet been exercised. |
| AC-2 | A clean-state manifest install resolves on macOS arm64 with the declared triplet and records actual package versions | Partial: the static `arm64-osx` graph resolved and built Qt 6.12.0 from the overlay (13 packages, 5.8 min); the required dynamic `arm64-osx-dynamic` graph stopped at an `lz4` source-download HTTP2 error before Qt was built. No clean-state repo-manifest resolution exists yet. |
| AC-3 | The project and minimal Qt probe configure, build and pass existing tests using the pinned graph | Not run: the probe has not been configured, built or tested against the vcpkg graph. |
| AC-4 | License/notice inventory is updated from actual resolved packages, with transitive and platform-specific gaps stated | Partial: `license-overrides.json` records evidence for packages whose vcpkg manifests declare no license; the inventory is not complete or reviewed and the licensing document is not updated. |
| AC-5 | An SBOM is generated from actual build artifacts and checked for package/version correspondence; no secrets or private data are included | Not run: the generator exists and compiles, but no SBOM has been generated or checked. |
| AC-6 | The development guide records exact commands, environment, results and limits; Windows resolution and release compliance remain open | Not run: the development guide and licensing document are not updated with this route yet. |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created the task and recorded the absent package manager and temporary tooling; no dependency change started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |
| 2026-10-30 | EveryGraph Implementation / implementer | Started the authorized vcpkg route; bootstrapped vcpkg from `microsoft/vcpkg` at `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`, selected the immutable baseline, and began clean-state resolution of the Qt 6.12.0 graph. | This record and the working-tree evidence below |
| 2026-10-31 | EveryGraph Implementation / implementer | Added `vcpkg.json`, `vcpkg-configuration.json`, the Qt 6.12.0 overlay under `cmake/vcpkg-overlay-ports/qtbase/`, the `arm64-osx-dynamic` CMake presets, the SPDX generator and license overrides; built static Qt 6.12.0 successfully; the dynamic build stopped at an `lz4` download error | Manifest, overlay, presets, SBOM files and the dynamic-build log listed below |
| 2026-10-31 | EveryGraph Implementation / checkpoint writer | Committed the P1-003 checkpoint on `feat/p1-003-vcpkg-graph` from `fca707a` without touching the conflicted `main` worktree, the index or `stash@{0}` | Checkpoint commit referenced in the Handoff section |

The task is in progress and paused at a checkpoint. The registry's latest `qtbase` is 6.11.2 while P1-001 qualified Qt 6.12.0; the overlay preserves 6.12.0. The static graph proves the overlay recipe builds Qt; the required dynamic graph and all probe/SBOM/doc evidence remain unverified.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 build 26A434, arm64; AppleClang 21.0.0.21000334; macOS SDK 27.0; deployment target 14.4; CMake 3.31.6 and Ninja 1.11.1.4 from `.venv`; vcpkg tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b` from `microsoft/vcpkg` commit `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`; vcpkg's internal port-build CMake 4.4.4; locally built host prerequisites pkgconf 3.0.3, GNU m4 1.4.19, autoconf 2.72, automake 1.17, GNU libtool 2.5.4 and autoconf-archive 2024.10.16 under the ignored `out/` tree (not committed). The 2026-10-30 rows below are the pre-start baseline.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | `which vcpkg` and package-manager inspection | Not found | Historical pre-start state; superseded by the 2026-10-31 bootstrap |
| 2026-10-30 | Existing build/test state | Passed 3/3 in debug, release and sanitize | Historical dependency-free probe state |
| 2026-10-30 | Dependency manifest and SBOM inspection | None found | Historical pre-start state |
| 2026-10-31 | `git clone https://github.com/microsoft/vcpkg.git out/vcpkg` then `out/vcpkg/bootstrap-vcpkg.sh -disableMetrics` | Passed; tool `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`; registry commit `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091` | Repo-local ignored checkout; the tool downloaded its own CMake 4.4.4 for port builds |
| 2026-10-31 | `diff -ru out/vcpkg/ports/qtbase cmake/vcpkg-overlay-ports/qtbase` | 4 files differ, 7 changed lines: version, source URL, SHA-512, filename, and removal of the obsolete `use_inotify_on_freebsd.patch` | The overlay is a version-pinned copy of the upstream recipe, not a new recipe; the removed patch is already present upstream in 6.12.0 |
| 2026-10-31 | `vcpkg install --x-manifest-root=out/vcpkg-612-test/minimal --triplet=arm64-osx --overlay-ports=$PWD/out/qtbase-overlay --x-install-root=out/vcpkg-612-test/min-installed` | Passed: 13 packages, "All requested installations completed successfully in: 5.8 min"; `qtbase` 6.12.0 | Static `arm64-osx` only; `libQt6Widgets.a` proves static linkage, so this is recipe/build evidence, not the selected deployment route |
| 2026-10-31 | Same manifest install with `--triplet=arm64-osx-dynamic` | Failed before Qt: `lz4` source download failed with curl error 16 (HTTP2 framing); 13/38 packages installed | Not a Qt compile failure and not a successful dynamic graph; log at `out/vcpkg-612-test/dyn-install.log` |
| 2026-10-31 | `python3 -m json.tool CMakePresets.json`, `vcpkg.json`, `vcpkg-configuration.json`; `python3 -m py_compile docs/engineering/sbom/generate_sbom.py` | Passed | Syntax/JSON validity only; no manifest resolution or SBOM generation |
| 2026-10-31 | `python3 tools/validate_planning.py` in `out/p1-003-checkpoint` | Failed on 7 pre-existing links in other owners' records (`p0-004`, `p0-005`, `p0-006`, `p1-006`) to files not present at `fca707a`; the P1-003 record has no broken link or CJK finding | The shared worktree is a partial stash apply and additionally reports a missing P0-006; this is not a P1-003 artifact or an acceptance check |
| 2026-10-31 | Configure/build/test the project with the vcpkg presets | Not run | Blocked by the incomplete dynamic graph and the paused session |
| 2026-10-31 | Generate/check the SBOM, update the engineering documents, independent review | Not run | Tooling exists; evidence and review are pending |

Checks not run and their reasons: repo-manifest resolution through `vcpkg-configuration.json`, a clean-state dynamic graph, Qt probe configure/build/test, the existing CTest presets against the vcpkg route, SBOM generation/validation, engineering-document updates, Windows resolution and independent review. The dynamic build stopped at an `lz4` source download; the session was paused at a checkpoint.

## Review and disposition

Reviewer and review type: no independent review has occurred. The checkpoint needs an independent EveryGraph Review plus a dedicated license/dependency review, especially for the Qt 6.12.0 overlay provenance and the `arm64-osx-dynamic` triplet decision. This is a self-checkpoint, not acceptance.

Reviewed files / revision: the P1-003 checkpoint on `feat/p1-003-vcpkg-graph` from `fca707a`; files listed in the Handoff section. No reviewer has inspected them independently.

Findings, resolutions and recheck evidence: none yet. The obsolete `use_inotify_on_freebsd.patch` was removed from the overlay because Qt 6.12.0 already contains the change; that removal is recorded and must be reviewed.

Acceptance decision, accepting role and date: pending. P1-003 is `in_progress`, not accepted. AC-2 through AC-6 remain open.

## Handoff and remaining work

Last verified state: vcpkg is bootstrapped at registry commit `d186940b2f0c4e8e5b4ff804af57d2a26fbfa091`; the manifest/configuration, Qt 6.12.0 overlay, `arm64-osx-dynamic` presets, SBOM generator and license overrides exist in the checkpoint; static `arm64-osx` Qt 6.12.0 built successfully from the overlay; the dynamic graph did not finish because the `lz4` source download failed. No probe build/test, SBOM, engineering-document update or independent review exists.

Modified files in the checkpoint:

- `vcpkg.json`, `vcpkg-configuration.json`.
- `CMakePresets.json` (only the vcpkg presets; `CMakeLists.txt` was intentionally not changed).
- `cmake/vcpkg-overlay-ports/qtbase/` (35-file recipe copy; 4 files differ from the registry port).
- `docs/engineering/sbom/generate_sbom.py` and `docs/engineering/sbom/license-overrides.json`.
- This record and the P1-003 board row. The board in the checkpoint is the `fca707a` base board plus the P1-003 row; the P0-000/P1-004 batch rows present in the shared working tree are other agents' uncommitted allocations and are not included.

Completed verification:

- vcpkg bootstrap and exact tool/registry revisions.
- Overlay diff and source SHA-512.
- Static Qt 6.12.0 build from the overlay (13 packages, 5.8 min); static linkage proven by `libQt6Widgets.a`, hence not the selected route.
- JSON syntax and Python syntax checks for the new artifacts.

Not yet verified:

- Repo-manifest resolution through `vcpkg-configuration.json` and the `version>= 6.12.0` guard.
- Complete `arm64-osx-dynamic` graph, Qt dynamic linkage, probe configure/build, `qt_probe_smoke`, the existing CTest presets and the Qt-free regression suites.
- SBOM generation, package/version correspondence, no-secret/path review and license inventory completeness.
- Development-guide and licensing-document updates; independent review.

Background task status: the dynamic build task `bca6z4ka5` ended with exit 1 at the `lz4` source download (`curl` error 16, HTTP2 framing). It installed 13 of 38 packages (including host `openssl` and `sqlite3`) but did not build Qt. Log: `out/vcpkg-612-test/dyn-install.log`; install root: `out/vcpkg-612-test/dyn-installed`. No vcpkg process remains. The static build completed successfully earlier; its log is `out/vcpkg-612-test/min-install5.log`.

Git and shared-worktree state:

- The shared worktree is on `main` at `747809e` with an unresolved stash apply (`stash@{0}`, commit `f6008b7`, a Qoder branch-switch autosave). `CMakeLists.txt`, `CMakePresets.json`, `docs/engineering/development.md` and `docs/planning/task-board.csv` contain conflict markers. This task did not resolve, reset, restore or commit those paths; the stash and index are preserved.
- The P1-003 checkpoint was committed from a clean detached worktree at `fca707a` on branch `feat/p1-003-vcpkg-graph`; the checkpoint commit is recorded in the following record commit. No push occurred.
- The checkpoint commit contains only the files listed above; it does not contain the concurrent P1-004 modules/tests, the P0-002/P0-003 record edits or the other agents' board allocations.

Blockers and unblock actions:

1. The `lz4` (and possibly later) GitHub archive download needs a seeded tarball or a retry; only then can the dynamic graph and probe tests proceed.
2. The repo-manifest `version>=`/overlay resolution is untested; if vcpkg rejects it, use the CMake `find_package(Qt6 6.12.0 EXACT)` gate as the enforcement and adjust the manifest with review.
3. The shared `main` stash-apply conflicts must be resolved by the coordinator or the owning agents; P1-003 must not absorb P1-004's work.
4. Windows resolution, signing, publication and release compliance remain outside this task.

Next task / concrete next action: seed or retry the dynamic source downloads, complete `arm64-osx-dynamic`, configure/build/test `macos-debug-qt-vcpkg` and `macos-release-qt-vcpkg` plus the Qt-free presets, generate and validate the SBOM, update the development/licensing documents, and request independent review. Do not mark P1-003 accepted until AC-2 through AC-6 are evidenced.

Files or commands needed to resume: [CMake presets](../../CMakePresets.json), [vcpkg manifest](../../vcpkg.json), [vcpkg configuration](../../vcpkg-configuration.json), [SBOM generator](../engineering/sbom/generate_sbom.py), [development guide](../engineering/development.md), [licensing policy](../engineering/licensing-and-dependencies.md), and the P1-001 record.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-001 dependency not started; vcpkg and network/install authorization absent |
| 2026-10-30 | blocked -> in_progress | P1-001 accepted; user granted the vcpkg/network scope on 2026-10-30, recorded in [P0-006](p0-006-extra-agent-verification.md), and instructed this session to execute P1-003 |
| 2026-10-30 | Scope clarification: evaluate a local `qtbase` overlay for Qt 6.12.0 | The curated vcpkg registry contains Qt 6.11.2, not the P1-001-qualified 6.12.0. The overlay would reuse the upstream recipe with only the version/source pin changed; it is recorded as a temporary, removable deviation, not a new package manager or a product change. |
| 2026-10-31 | Technical route: target triplet `arm64-osx-dynamic` | The accepted licensing policy requires shared Qt for the LGPL replacement route. vcpkg's `arm64-osx` triplet sets `VCPKG_LIBRARY_LINKAGE static`; the standard community `arm64-osx-dynamic` variant is therefore used for the target. Host tools remain on `arm64-osx`. The board wording "arm64-osx triplet" is read as the macOS arm64 triplet family, not the static variant; this decision and its evidence must be reviewed. |
| 2026-10-31 | Checkpoint commit; task remains `in_progress` | User requested a safe pause and checkpoint commit. Only P1-003 files are committed on `feat/p1-003-vcpkg-graph` from `fca707a`; the shared `main` conflicts and `stash@{0}` are untouched; no acceptance is claimed. |
