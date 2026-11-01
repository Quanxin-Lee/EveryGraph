# P0-019: Move the Phase-0 macOS Job Off the Retiring macos-14 Runner Labels

## Metadata

- Task ID: P0-019
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: none directly; it protects the Phase-0 toolchain and planning validation surface that AGENTS.md section 8 assigns to CI
- Dependencies: P0-016
- Created: 2026-11-01
- Updated: 2026-11-01
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-11-01. P0-016, which last touched this workflow, is accepted.
- Authorization: not granted. Required: one edit to `.github/workflows/phase0.yml` and this record. No push, no workflow dispatch, no dependency installation, no other normative path and no run-history claim is authorized by this task.

## Scope and acceptance

Objective: the macOS job requests a runner label that GitHub is removing. `.github/workflows/phase0.yml:14` says `runs-on: macos-14-xlarge`. GitHub announced that the macOS 14 Sonoma based runner images entered deprecation on 2026-07-06 and become fully unsupported for GitHub Actions and Azure DevOps on **2026-11-02** ([actions/runner-images#13518](https://github.com/actions/runner-images/issues/13518)), and the deprecation covers the `macos-14`, `macos-14-large` and `macos-14-xlarge` labels. The supported arm64 larger-runner labels in the [larger runners reference](https://docs.github.com/actions/reference/runners/larger-runners) are `macos-15-xlarge`, `macos-26-xlarge` and `macos-latest-xlarge`; the [runner-image lifecycle list](https://endoflife.date/github-actions-runner-images) puts macOS 14 arm64 support end at 2026-11-02.

P0-000's disposition: the fix is real but its exposure is conditional. The workflow triggers on `push`, `pull_request` and `workflow_dispatch` (`phase0.yml:3-7`), and this repository has no authorized push path in the current round, so no run fails until a push or dispatch happens. This task therefore changes one label and records what is not yet proven, rather than claiming a green CI run.

Allowed files and interfaces: `.github/workflows/phase0.yml` and this record. `docs/engineering/development.md:832`, which documents why the job requested `macos-14-xlarge`, is **not** in this manifest: P0-000 designated P1-014 as the single writer for the next revision of that guide, so this task files the sentence correction as a rule 9 request naming the target line, the exact replacement wanted and the reason, and changes nothing there. `CMakePresets.json`, the presets themselves and the workflow's `windows` job guard at `phase0.yml:54` stay unchanged.

Out of scope: running or dispatching the workflow, reading Actions run history, changing the pinned `cmake==3.31.6` and `ninja==1.11.1.4` installs, re-baselining the local AppleClang toolchain, adding a matrix over macOS images, and any claim that a macOS 15 or 26 runner result substitutes for the locally qualified macOS 14.4+ Apple Silicon evidence.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The label is replaced with a supported arm64 larger-runner label, and the choice is stated with the reason plus what else changes with the image — Xcode, AppleClang and SDK versions the job prints at `phase0.yml:28-34` | Not run |
| AC-2 | The edit is proven locally without a run: `python3 -c "import yaml..."` or an equivalent parse of the workflow file, plus a check that the architecture assertion `test "$(uname -m)" = arm64` and the pinned tool steps are unchanged, and `git diff` shows exactly the label line | Not run |
| AC-3 | The task states plainly that no GitHub Actions run has been performed or observed, names what would prove the label works, and records the retirement date as documented fact with its source rather than as inference | Not run |
| AC-4 | The `docs/engineering/development.md` sentence is requested through rule 9 with the target line, the exact change and the reason, and this task writes no word to that file | Not run |
| AC-5 | The Windows job's `if: github.event_name == 'workflow_dispatch'` guard and every other workflow path are byte-for-byte unchanged, and no other task's file is staged | Not run |
| AC-6 | An independent reviewer that is not the author records the reviewed revision, findings and recheck; this record moves to `accepted` with a `Board sync` request naming row P0-019 and the proving commit | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-11-01 | P0-000 / allocation executor | Allocated P0-019 from the CI finding the user supplied, verified `runs-on: macos-14-xlarge` at `.github/workflows/phase0.yml:14` and the retirement window against GitHub's own announcement, and created this initial record scaffold; no workflow file was touched | [Board](../planning/task-board.csv); [P0-016 record](p0-016-ci-windows-job-deferral.md); [P0-000 record](p0-000-task-allocation.md) |

No workflow edit exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. `gh` is not installed on this host, which P1-013 also recorded, so Actions run history cannot be read from here.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-11-01 | `grep -n "runs-on" .github/workflows/phase0.yml` | `14: runs-on: macos-14-xlarge` and `56: runs-on: windows-2022` | Confirms the finding; the Windows label is out of this task's scope |
| 2026-11-01 | Read of the workflow triggers and the macOS job steps at `phase0.yml:3-52` | `push`, `pull_request` and `workflow_dispatch` triggers; the job asserts arm64 and prints `sw_vers`, `xcode-select -p`, `xcrun clang++ --version` and `xcrun --show-sdk-version` | Establishes that the label change is observable in a future run and that a push is required for any run to happen |
| 2026-11-01 | GitHub announcement and reference pages checked by web search on 2026-11-01 | Deprecation began 2026-07-06; full unsupported date 2026-11-02 for the macOS 14 image family; `macos-15-xlarge` and `macos-26-xlarge` listed as arm64 xlarge labels | Documentation fact, not a run result. The brownout windows circulated in secondary coverage are recorded as **unverified**: no GitHub source was read for them, and the local clock is one day before the stated removal date |
| 2026-11-01 | `python3 tools/validate_planning.py` | PASS at the current tree | Documentation baseline only; no CI state |

Checks not run and their reasons: any workflow execution, dispatch or run-history read; they need a push or dispatch authorization and a `gh`-capable host.

## Review and disposition

Reviewer and review type: an independent reviewer that is not the author is required before acceptance. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not started.

## Handoff and remaining work

Last verified state: `phase0.yml:14` still requests the retiring label; the development guide's `macos-14-xlarge` rationale sentence at `docs/engineering/development.md:832` is designated to P1-014, so the two edits cannot be made by one executor.

Blockers and unblock actions: none inside this task's manifest. Proving the new label needs a push or an authorized workflow dispatch, which AGENTS.md section 7 and P0-000's disposition D7 keep request-only; the task must not claim a green run without one.

Next task / concrete next action: claim P0-019, correct this scaffold's `planned` status and add the Change manifest as the first write, change the single label, verify the diff shape and workflow parse locally, file the rule 9 request for the guide sentence, record the independent review and file the `Board sync` request. If the user wants CI proven rather than reasoned about, the concrete ask is one push or one authorized `workflow_dispatch`.

Files or commands needed to resume: [P0-016 record](p0-016-ci-windows-job-deferral.md), `.github/workflows/phase0.yml`, [GitHub larger runners reference](https://docs.github.com/actions/reference/runners/larger-runners), [actions/runner-images#13518](https://github.com/actions/runner-images/issues/13518).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-11-01 | New -> planned | Allocated by P0-000 against the CI finding the user supplied; verified against GitHub's own retirement notice, and scoped to one label because the guide sentence belongs to P1-014 |
