# P1-014: Make the Native Focus-Order Qualification Deterministic

## Metadata

- Task ID: P1-014
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-009;DG-UI-009
- Dependencies: P1-008
- Created: 2026-11-01
- Updated: 2026-11-01
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-11-01. P1-008 and P1-013 are accepted.
- Authorization: not granted. Required: modify the native focus-order test in `apps/desktop/tests/accessibility_tests.cpp`, run repeated desktop sweeps and record the results in this record. No CMake, product-source, documentation, board, dependency-installation, push or publication action is authorized by this task.

## Scope and acceptance

Objective: close gap G2, the only P1-exit gap a local code change can close. P1-013 recorded that `accessibility_focus_native` fails non-deterministically — 1 failure in 6 full `macos-release-desktop` sweeps at the same code tree — with `focus did not return to the start: projectTree -> propertiesPanel -> functionLibraryList -> documentTabBar -> <focus-lost>` at `apps/desktop/tests/accessibility_tests.cpp:509-511` and `the walk never reached the appearance combo, active: no, focused: <null>` at `:586-588`. The `active: no` half of that second message is the cause P1-013 names: the window server had not made the window key when the first Tab was sent. The flake keeps the P1 exit at **not passed**, and it aborted P1-012's recipient-rebuild route twice, so it is the blocker for the release preset's first-time-green property.

Allowed files and interfaces: `apps/desktop/tests/accessibility_tests.cpp` and this record. Everything else, including `apps/desktop/CMakeLists.txt`, `CMakePresets.json`, the engineering guides and P1-008's record, is read-only.

Out of scope: physical multi-monitor and display-mode changes (P1-008 gap G1), a screen-reader or full WCAG audit, IME matrix work, any Windows run, and any change to the `accessibility_1x`, `accessibility_2x` or IME entries' assertions.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The failing run is reproduced or the cause is identified from the recorded assertion output, and the exact mechanism — window not key at the first Tab — is stated with the file lines involved | Not run |
| AC-2 | The native walk runs only after an explicit activation precondition that fails the test rather than waiting on a bounded timeout; no assertion is weakened, no case skipped and no `QVERIFY` replaced by a retry loop that can hide a real focus loss | Not run |
| AC-3 | Repeated full `macos-release-desktop` and `macos-debug-desktop` sweeps run with the exact commands, the per-sweep pass or fail results and the total consecutive-green count recorded; the release preset is green first time on every sweep run | Not run |
| AC-4 | The IME, `accessibility_1x` and `accessibility_2x` entries stay unchanged and pass in the same sweeps, and no other CTest registration changes result | Not run |
| AC-5 | The limits P1-008 keeps are restated, not claimed closed: simulated `QScreen` geometry and a Qt scale factor on one Retina panel, no physical display change, no screen reader | Not run |
| AC-6 | An independent reviewer that is not the author records the reviewed revision, its findings and the recheck; this record then moves to `accepted` and files a `Board sync` request naming row P1-014 and the proving commit | Not run |
| AC-7 | The five macOS presets, the desktop presets, `python3 tools/validate_planning.py` and `python3 tools/validate_task_manifest.py` stay green for this task's paths; only the two manifest paths are committed with exactly this task ID in the subject | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-11-01 | P0-000 / allocation executor | Allocated P1-014 to close gap G2 recorded by P1-013 and created this initial record scaffold; no test code was touched | [Board](../planning/task-board.csv); [P1-013 record](p1-013-p1-exit-verification.md); [P0-000 record](p0-000-task-allocation.md) |

No code change and no sweep evidence exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. The failing entry is `accessibility_focus_native` inside `everygraph_accessibility_tests`, registered by the desktop presets.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-11-01 | Allocation-time read of P1-013's gap table | G2 names P1-008's recorded timing sensitivity as the same residual; 2 failures in 4 runs pre-fix, then 45 consecutive green, then 1 failure in 6 release sweeps at `c20c152` | Confirms the task is not started and that P1-008 already recorded, but did not deny, the flake |

Checks not run and their reasons: the reproduction sweep, the activation-precondition change and the repeated full sweeps. They need an executor and this task's manifest.

## Review and disposition

Reviewer and review type: an independent reviewer that is not the author is required before acceptance. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not started.

## Handoff and remaining work

Last verified state: `accessibility_focus_native` passes most runs and fails non-deterministically; P1-013 accepted the exit check with that flake visible and graded E1 and E2a as pass-only-with-the-recorded-flake.

Blockers and unblock actions: none outside this task. Its dependency P1-008 is accepted. The user must authorize the test edit and the sweep runs; no dependency installation, CMake change or push is part of this task.

Next task / concrete next action: claim P1-014, correct this scaffold's `planned` status as the first write together with the Change manifest, reproduce the failure, implement the activation precondition, run the repeated sweeps, and file the `Board sync` request. The P1 exit stays not-passed until this gap and the others close or the user explicitly accepts the recorded trade-offs.

Files or commands needed to resume: [P1-008 record](p1-008-accessibility-display-input.md), [P1-013 record](p1-013-p1-exit-verification.md), `apps/desktop/tests/accessibility_tests.cpp`, `ctest --preset macos-release-desktop`, `ctest --preset macos-debug-desktop`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-11-01 | New -> planned | Allocated by P0-000 from gap G2 of the accepted P1-013 exit verification; the only P1-exit gap closable by a local code change on this host |
