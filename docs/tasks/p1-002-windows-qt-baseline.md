# P1-002: Replicate the Qt Baseline Qualification on Windows x64

## Metadata

- Task ID: P1-002
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P1-001; P1-003
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not applicable; no Git repository exists and the task has not started
- Authorization: not granted. Required: an actual Windows x64 host or authorized CI execution; local Git initialization and a repository remote if CI is used; no signing, publishing or Windows runtime support claim.

## Scope and acceptance

Objective: reproduce the P1-001 Qt baseline on actual Windows x64 with MSVC 2022, record the exact Windows/SDK/tool/Qt versions, and establish whether the same minimal probe configures, builds and tests in Debug and Release. This supports, but does not complete, EG-ADD-016 or DG-PERF-004; the cross-platform gate remains open until both platform results and published minima are reviewed.

Allowed files and interfaces: `CMakePresets.json`, `.github/workflows/phase0.yml` if CI is authorized, `tools/qt_probe/` for platform-specific build files, `docs/engineering/development.md` and this record.

Out of scope: macOS results, minimum Windows 10 22H2/11 runtime qualification beyond the actual host, full P1 workbench, business logic, packaging/signing/notarization, release publication, and any inference from the macOS probe.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Actual Windows host or authorized CI identity and Windows build, x64 architecture, MSVC 2022, Windows SDK, CMake, Ninja and Qt versions recorded | Not run |
| AC-2 | `windows-debug` and `windows-release` configure and build the same minimal Qt Widgets probe | Not run |
| AC-3 | Full CTest suites pass on Windows, including the offscreen smoke test and planning checks | Not run |
| AC-4 | Dynamic Qt deployment on Windows is checked and recorded; any platform-specific limitation is explicit | Not run |
| AC-5 | Development guide records actual commands, outputs and verification limits; no macOS evidence is reused; no signing or release claim is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created the task and recorded the absent Windows environment; no implementation started | [Takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |

No Windows execution, CI run or build evidence exists.

## Validation

Working directory, OS, architecture and relevant tool versions: not applicable; no Windows host or runner is available from the inspected macOS environment. The configured Windows presets are documented but unexecuted.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Windows preset inspection | Configuration only | `windows-debug` and `windows-release` exist; no `cl`, Windows SDK or Windows host is available here |
| 2026-10-30 | CI workflow inspection | Configuration only | The workflow defines Windows jobs but no repository, remote or run exists |
| 2026-10-30 | Windows execution | Not run | No Windows host or authorized CI |

Checks not run and their reasons: Windows Qt configure/build/test, offscreen smoke, deployment, Windows runtime tests, CI dispatch, signing and packaging. The task is blocked before these checks.

## Review and disposition

Reviewer and review type: independent EveryGraph Review will be required for the Windows evidence. No reviewer has been assigned; no review has occurred.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: Windows presets and CI configuration exist, but no Windows build, test result or runner has been observed. P1-001 has not started and is a dependency; P1-003 must settle the package route before Windows replication so the platforms share one dependency graph.

Blockers and unblock actions:

- No Windows x64 host or authorized runner is available. Provide a Windows 10 22H2/11 x64 machine or authorize a repository remote and CI run with a compatible Windows runner.
- No Git repository or remote exists. Authorize local initialization and remote publication only if CI is the chosen route; no push is implied by this record.
- The exact Qt patch and package route come from P1-001 and P1-003. Do not use a different Qt version or an ad hoc package graph without explicit review.
- The Windows host must have the Visual Studio 2022 C++ desktop toolchain and an initialized x64 developer environment; the preset uses `cl` and external x64 selection.

Next task / concrete next action: after P1-001 and P1-003 are accepted and a Windows environment is authorized, Implementation runs the two Windows presets with the settled dependency graph, records actual versions and logs, and returns them for independent Review. No Windows support claim is made until that evidence exists.

Files or commands needed to resume: [CMake presets](../../CMakePresets.json), [Phase-0 CI](../../.github/workflows/phase0.yml), [development guide](../engineering/development.md), and the P1-001 record.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | No Windows host or authorized CI; P1-001 and P1-003 dependencies not started |
