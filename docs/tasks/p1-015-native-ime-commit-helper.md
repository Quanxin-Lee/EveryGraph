# P1-015: Make the Native Pinyin IME Helper Commit Deterministic

## Metadata

- Task ID: P1-015
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-009;DG-UI-009
- Dependencies: P1-008
- Created: 2026-11-01
- Updated: 2026-11-01
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-11-01, above P1-014's `6f1a49b`. P1-008 and P1-013 are accepted; P1-014 is `blocked` on this task.
- Authorization: not granted. Required: modify the native Pinyin helper `apps/desktop/tests/native_pinyin_ime.mm` and `apps/desktop/tests/native_pinyin_ime.hpp`, run the repeated IME loops and desktop sweeps and record them here. No edit to `apps/desktop/tests/accessibility_tests.cpp`, no CMake, documentation, board, dependency-installation, push or publication action is authorized by this task.

## Scope and acceptance

Objective: close the residual P1-exit flake that P1-014 named as its own blocker and could not fix from its manifest. P1-014 made `accessibility_focus_native` deterministic — 24 of 24 post-edit release sweeps, 8 of 8 debug sweeps and 40 of 40 isolated reruns green — and in doing so left `accessibility_ime_native` as the remaining non-deterministic entry: 2 failures of the 106 post-edit observations (about 2%).

P1-014's rule 9 request states the mechanism from its logs: `send_key(context, 49); // space commits the first candidate` at `apps/desktop/tests/native_pinyin_ime.mm:161` is followed by `result.committed_text = target.text();` at `:167`, so the committing space can also land in the line edit and `QCOMPARE(result.committed_text, greeting())` at `apps/desktop/tests/accessibility_tests.cpp:999` fails on `"\u4F60\u597D "` versus `"\u4F60\u597D"`; and one isolated run recorded `preedits:  commit: nihao`, bypassing composition and failing `QVERIFY2(!result.preedit_steps.isEmpty(), ...)` at `:997-998`.

Allowed files and interfaces: `apps/desktop/tests/native_pinyin_ime.mm`, `apps/desktop/tests/native_pinyin_ime.hpp` and this record. Everything else — `apps/desktop/tests/accessibility_tests.cpp`, `apps/desktop/CMakeLists.txt`, `CMakePresets.json`, the engineering guides and the P1-008 and P1-014 records — is read-only.

Out of scope: any change to the IME assertions in `accessibility_tests.cpp:997-1000`, which are the contract this helper must satisfy; the native focus walk, which P1-014 owns; an IME matrix beyond the single `com.apple.inputmethod.SCIM.ITABC` input method and single `QLineEdit` P1-008 qualified; physical display or screen-reader work; any Windows run.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The two mechanisms P1-014 recorded are confirmed or corrected from this task's own failing runs, with the exact assertion output, log path and file lines | Not run |
| AC-2 | The helper commits the candidate without letting the committing key reach the line edit, and keeps the input method in composition mode so the preedit sequence is never bypassed; `apps/desktop/tests/accessibility_tests.cpp` is byte-for-byte unchanged, no assertion is weakened, no case is skipped and no retry loop can hide a failure | Not run |
| AC-3 | Repeated isolated `-R '^accessibility_ime_native$'` runs on both the release and debug desktop presets with per-run results recorded and zero failures, plus full `macos-release-desktop` and `macos-debug-desktop` sweeps with the per-sweep result of each | Not run |
| AC-4 | The host state for the runs is recorded — active input source, scale factor, and any disturbance such as another app taking focus — and an anomalous run is kept with its log evidence and explained rather than discarded | Not run |
| AC-5 | `accessibility_focus_native`, `accessibility_1x` and `accessibility_2x` stay green in the same sweeps, and no other CTest registration changes result | Not run |
| AC-6 | This task's acceptance releases P1-014's blocker by naming the commit that lands the helper change; this task does not accept P1-014, whose executor re-runs AC-3 and AC-4 of its own record and self-accepts there | Not run |
| AC-7 | P1-008's retained limits are restated, not claimed closed: one input method on one line edit, simulated `QScreen` geometry, a Qt scale factor on one Retina panel, no screen reader and no Windows run | Not run |
| AC-8 | An independent reviewer that is not the author records the reviewed revision, its findings and the recheck; this record moves to `accepted` and files a `Board sync` request naming row P1-015 and the proving commit | Not run |
| AC-9 | The five macOS presets, the desktop presets, `python3 tools/validate_planning.py`, `python3 tools/validate_catalog_children.py` and `python3 tools/validate_task_manifest.py` stay green for this task's paths; only the three manifest paths are committed with exactly this task ID in the subject | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-11-01 | P0-000 / allocation executor | Allocated P1-015 from the rule 9 request P1-014 filed, and created this initial record scaffold; no helper code was touched | [Board](../planning/task-board.csv); [P1-014 record](p1-014-native-focus-order-determinism.md); [P0-000 record](p0-000-task-allocation.md) |

No code change and no sweep evidence exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. The entry is `accessibility_ime_native` inside `everygraph_accessibility_tests`, registered by the desktop presets; the native IME source observed by P1-013 and P1-014 is `com.apple.inputmethod.SCIM.ITABC`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-11-01 | Allocation-time read of P1-014's rule 9 request, gap I1, and its IME loop rows | Two mechanisms named with file lines; 2 failures of 106 post-edit IME observations | Confirms the task is not started and that the fix belongs to the helper, not to the assertions |

Checks not run and their reasons: the reproduction loop, the helper change and the repeated sweeps. They need an executor and this task's manifest.

## Review and disposition

Reviewer and review type: an independent reviewer that is not the author is required before acceptance. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not started.

## Handoff and remaining work

Last verified state: `accessibility_focus_native` is deterministic after P1-014's activation precondition (`6f1a49b`); `accessibility_ime_native` fails about 2% of post-edit observations for the two reasons this record names, which is why P1-014 recorded itself `blocked` instead of self-accepting.

Blockers and unblock actions: none outside this task. Its dependency P1-008 is accepted, and the two target paths are P1-008's own deliverable, claimed before rule 7 existed; P0-000 designates this task as the writer for the next revision of those two files. The user must authorize the helper edit and the runs.

Next task / concrete next action: claim P1-015, correct this scaffold's `planned` status as the first write together with the Change manifest, reproduce the two failures, fix the helper without touching the assertions, run the repeated loops and sweeps, then record the acceptance commit so P1-014's executor can re-run its own AC-3 and AC-4 and self-accept. The P1 exit stays not-passed until G1 to G7 and this residual close or the user explicitly accepts the recorded trade-offs.

Files or commands needed to resume: [P1-014 record](p1-014-native-focus-order-determinism.md), [P1-008 record](p1-008-accessibility-display-input.md), `apps/desktop/tests/native_pinyin_ime.mm`, `apps/desktop/tests/native_pinyin_ime.hpp`, `ctest --preset macos-release-desktop -R '^accessibility_ime_native$'`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-11-01 | New -> planned | Allocated by P0-000 from the rule 9 request P1-014 filed against `native_pinyin_ime.mm/.hpp`; the residual IME flake is the remaining non-deterministic P1-exit entry on this host |
