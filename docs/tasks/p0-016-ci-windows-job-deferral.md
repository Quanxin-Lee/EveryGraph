# P0-016: Stop the CI Windows job from producing Windows activity under the deferral

## Metadata

- Task ID: P0-016
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: none — governance and build-configuration work; the roadmap Phase-0 exit clause and [ADR-0005](../decisions/ADR-0005-platform-sequencing.md) supply the rationale
- Dependencies: P0-008; P0-015
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not applicable until an executor starts; the allocation scaffold is committed by P0-000 on `main`
- Authorization: this record allocates the task only. The executor edits the single manifest path below, which P0-000 designates for this revision under AGENTS.md section 0 rule 8. No push, no CI run, no workflow dispatch and no Windows claim is authorized here

## Scope and acceptance

Objective: `.github/workflows/phase0.yml` currently declares a `windows` job (`runs-on: windows-2022`, presets `windows-debug` and `windows-release`) on the same `push` and `pull_request` triggers as the `macos` job. [ADR-0005](../decisions/ADR-0005-platform-sequencing.md) defers every Windows implementation and evidence claim until the Windows gate resumes on a Windows x64 host or authorized CI, so a push that runs that job either produces Windows activity nobody authorized or becomes a red run that is not a product defect. [P0-015](p0-015-p0-exit-re-verification.md) recorded this as an `unassigned` residual and as the one CI finding in its gate table. Make the Windows job impossible to trigger from `push` or `pull_request` while keeping its definition recoverable for the deferred gate.

Allowed files and interfaces: the single manifest path below. The `macos` job, its matrix, its steps and the workflow's top-level name are unchanged.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `.github/workflows/phase0.yml` | modify | P0-000 designates P0-016 as the single writer of this shared normative path for this revision (rule 8); the deferral it enforces is ADR-0005 |
| `docs/tasks/p0-016-ci-windows-job-deferral.md` | modify | Own task record |

Out of scope: enabling, disabling or renaming any preset; editing `CMakePresets.json`, `CMakeLists.txt` or the development guide; deleting the Windows job's definition or its presets; adding a Windows badge, status, evidence path or support statement; running the workflow, dispatching it, or pushing.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The `windows` job cannot run on `push` or `pull_request`: its trigger condition is a manual dispatch input, or the job is defined in a dispatch-only workflow, and the record states which mechanism was chosen and why it cannot fire on a push | Not run |
| AC-2 | The definition is recoverable for the deferred gate: the job's `runs-on`, matrix presets and steps remain in the repository, or the record states exactly where the recoverable copy lives | Not run |
| AC-3 | The `macos` job is unchanged: `git diff` over the workflow shows no change inside its block, and the record quotes the unchanged trigger and matrix | Not run |
| AC-4 | The file still parses as YAML and the record names the parser and version used; if a linter such as `actionlint` is unavailable, the record says so and states the resulting verification limit | Not run |
| AC-5 | No claim is added. The record states that a configured workflow is not an executed one, that no Windows evidence exists, and that this task creates no platform, support or parity claim | Not run |
| AC-6 | Only the two manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; rule 12 self-check (or `tools/validate_task_manifest.py` when it covers the task) is recorded in the Validation table | Not run |
| AC-7 | `python3 tools/validate_planning.py` passes with this record present, the record follows the English policy and the independent reviewer is not the implementer | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its two-path change manifest and the writer designation for `.github/workflows/phase0.yml`; no workflow change performed | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-015 record](p0-015-p0-exit-re-verification.md) residual table |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| — | Not run | Not run | The task is `planned`; no executor has started |

## Review and disposition

Reviewer and review type (independent or self-review): required and not yet performed. A reviewer that did not implement the change must inspect the committed diff, verify that no `push` or `pull_request` event can reach the Windows job, and record its label and the reviewed revision.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): none yet; the task is `planned`. When the executor accepts, it records the row, the state and the proving commit here for P0-000 to sync.

Last verified state: scaffold only. Working directory `/Users/leo/Documents/General/EveryGraph`, branch `main`.

Blockers and unblock actions: none for execution. The task needs an executor; the change itself is one YAML file. A CI run remains request-only and is not part of this task's evidence.

Next task / concrete next action: on release, read the current `.github/workflows/phase0.yml`, choose the minimal mechanism that provably cannot fire on a push, apply AC-1 to AC-7 and accept after an independent review.
