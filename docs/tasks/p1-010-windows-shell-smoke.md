# P1-010: Replicate the Workbench Shell Smoke Suite on Windows x64

## Metadata

- Task ID: P1-010
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-016; DG-PERF-004
- Dependencies: P1-002; P1-006; P1-007; P1-008
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated branch or worktree must be created after the dependencies are accepted.
- Authorization: not granted. Required: an actual Windows x64 host or authorized CI with a compatible Windows runner, Windows build/test changes, and documentation updates. No signing, installer publication, release claim or macOS evidence reuse is authorized.

## Scope and acceptance

Objective: prove that the P1 workbench shell and its command smoke path behave on Windows x64 with the same pinned Qt graph, and record the exact Windows/MSVC/SDK/Qt environment. The P1-002 task qualifies only the minimal probe; this task covers the shell.

Allowed files and interfaces: Windows-specific build presets/tests, `apps/desktop` and `modules/presentation_qt` only where platform adaptation is required, `docs/engineering/development.md` and this record. The application/domain contracts must remain platform-neutral.

Out of scope: a full Windows feature-parity claim, installer creation, Authenticode signing, store submission, macOS evidence reuse, Windows 10 22H2/11 runtime qualification beyond the actual host, and any business feature not present in the P1 shell.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | An actual Windows x64 host or authorized CI identity is recorded with Windows build, MSVC 2022, Windows SDK, CMake, Ninja and Qt versions | Not run |
| AC-2 | The shell configures and builds in Debug and Release with the pinned Qt graph; no ad hoc package route is introduced | Not run |
| AC-3 | The same command smoke path creates a fake project object, edits it, undoes the edit and verifies the committed state through the shared command contract | Not run |
| AC-4 | The project tree, document tabs/detach/restore, preferences and basic accessibility behaviors run without Windows-specific crashes or silent divergence | Not run |
| AC-5 | Dynamic Qt deployment is checked and recorded; any Windows-specific limitation is explicit | Not run |
| AC-6 | The development guide records exact commands, outputs, environment and verification limits; no macOS result is reused and no signing/release claim is made | Not run |
| AC-7 | If no Windows host or authorized runner is available, the task remains blocked and no Windows shell claim is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit and the P1-002 probe boundary; no Windows shell work started | [P1-002 record](p1-002-windows-qt-baseline.md), [takeover audit](p0-002-takeover-state-verification.md) and [board](../planning/task-board.csv) |

No Windows shell build, test or runner result exists. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64. No Windows x64 host, Visual Studio 2022 environment or Windows runner is available from this host.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Host inspection | macOS arm64 only; no Windows host or runner | No Windows execution is possible here |
| 2026-10-30 | CI workflow inspection | Windows jobs are configured but no run has been observed; the workflow does not build a Qt shell | Configuration only; not evidence |
| 2026-10-30 | P1-002 state | Blocked on a Windows host or authorized CI and on P1-001/P1-003 | The probe has not been replicated on Windows |
| 2026-10-30 | `python3 tools/validate_planning.py` | Passed: 35 documents | Documentation baseline only; no Windows claim |

Checks not run and their reasons: Windows shell configure/build, command smoke, UI behavior, deployment, accessibility basics, preferences and detach/restore. They require a Windows host or authorized CI and the accepted P1-002/P1-006/P1-007/P1-008 dependencies.

## Review and disposition

Reviewer and review type: independent Windows verification and UI review are required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: no Windows shell build or test exists. The P1-002 probe has not run on Windows; the CI workflow is configuration only.

Blockers and unblock actions:

- Provide an actual Windows x64 machine with Visual Studio 2022 C++ desktop tools, Windows SDK, CMake 3.31.6, Ninja 1.11.1.4 and the P1-003 pinned Qt graph, or authorize a Windows CI runner.
- P1-002, P1-006, P1-007 and P1-008 must be accepted first.
- The Windows build must not reuse macOS binaries or infer compatibility from the macOS shell.
- Signing and installer work remain separate release tasks and are not authorized here.

Next task / concrete next action: after the dependencies and a Windows environment are available, build and run the shell smoke suite, record exact environment and results, and request independent review. Do not mark EG-ADD-016 complete from the macOS shell alone.

Files or commands needed to resume: [P1-002 record](p1-002-windows-qt-baseline.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [P1-007 record](p1-007-preferences-appearance-focus-mode.md), [P1-008 record](p1-008-accessibility-display-input.md), [development guide](../engineering/development.md), `CMakePresets.json`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | No Windows host or authorized runner; P1-002/P1-006/P1-007/P1-008 are not accepted |
