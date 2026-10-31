# P0-016: Stop the CI Windows job from producing Windows activity under the deferral

## Metadata

- Task ID: P0-016
- Status: in_progress
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: none — governance and build-configuration work; the roadmap Phase-0 exit clause and [ADR-0005](../decisions/ADR-0005-platform-sequencing.md) supply the rationale
- Dependencies: P0-008; P0-015
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main`, shared checkout, base `4d43122`; no branch/worktree created because task is a one-line workflow change and commits use pathspecs
- Authorization: execution of the two-path manifest is authorized by this task allocation and P0-000's writer designation. No push, CI run, workflow dispatch or Windows claim is authorized

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
| AC-1 | The `windows` job cannot run on `push` or `pull_request`: its trigger condition is a manual dispatch input, or the job is defined in a dispatch-only workflow, and the record states which mechanism was chosen and why it cannot fire on a push | Pass — job condition requires `workflow_dispatch` event |
| AC-2 | The definition is recoverable for the deferred gate: the job's `runs-on`, matrix presets and steps remain in the repository, or the record states exactly where the recoverable copy lives | Pass — existing Windows job definition retained |
| AC-3 | The `macos` job is unchanged: `git diff` over the workflow shows no change inside its block, and the record quotes the unchanged trigger and matrix | Pass — workflow triggers remain `push`, `pull_request`, `workflow_dispatch`; macOS matrix remains `[macos-debug, macos-release, macos-sanitize]` |
| AC-4 | The file still parses as YAML and the record names the parser and version used; if a linter such as `actionlint` is unavailable, the record says so and states the resulting verification limit | Pass — Ruby Psych 3.1.0 parsed the workflow; `actionlint` is unavailable, so GitHub Actions semantic validation was not run |
| AC-5 | No claim is added. The record states that a configured workflow is not an executed one, that no Windows evidence exists, and that this task creates no platform, support or parity claim | Pass — this configured workflow was not executed; no Windows evidence exists; this task makes no platform, support or parity claim |
| AC-6 | Only the two manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; rule 12 self-check (or `tools/validate_task_manifest.py` when it covers the task) is recorded in the Validation table | Pass — this acceptance record and workflow are committed together with the required P0-016 pathspec commit; post-commit path verification is recorded in the handoff |
| AC-7 | `python3 tools/validate_planning.py` passes with this record present, the record follows the English policy and the independent reviewer is not the implementer | Pass — validator passed; independent review passed |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its two-path change manifest and the writer designation for `.github/workflows/phase0.yml`; no workflow change performed | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-015 record](p0-015-p0-exit-re-verification.md) residual table |
| 2026-10-31 | EveryGraph Implementation / P0-016 executor | Added a job-level event guard requiring manual `workflow_dispatch`; retained Windows job body and left macOS block untouched | `.github/workflows/phase0.yml` diff |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| 2026-10-31 | `python3 -c 'import yaml; print(yaml.__version__)'` | Unavailable | PyYAML is not installed; no dependency was installed |
| 2026-10-31 | `ruby -e 'require "yaml"; p Psych::VERSION; YAML.parse_file(".github/workflows/phase0.yml"); puts "YAML parse passed"'` | Passed | Ruby 2.6.10; Psych 3.1.0; validates YAML syntax only, not Actions semantics |
| 2026-10-31 | `command -v actionlint` | Unavailable | No actionlint binary on PATH; no installation performed |
| 2026-10-31 | `python3 tools/validate_planning.py` | Passed | 339 original IDs; 428 traced requirements; 96 documents; English policy and links passed; validator notes remote URL, Mermaid, scientific semantics and application behavior are not checked |
| 2026-10-31 | `git diff --check -- .github/workflows/phase0.yml docs/tasks/p0-016-ci-windows-job-deferral.md` | Passed | No whitespace errors |
| 2026-10-31 | AGENTS.md section 0 rule 12 self-check | Passed | Manifest contains exactly the workflow and own record; unrelated worktree changes were excluded. User explicitly authorized the local commit on 2026-10-31. Post-commit path verification is recorded below |

## Review and disposition

Reviewer and review type (independent or self-review): P0-016 Review / independent reviewer (not implementer); passed.
Reviewed files / revision: `.github/workflows/phase0.yml` and this record; base HEAD `4d431228e8f749931991a8bd810c3f3205fccdda` plus working diff.
Findings, resolutions and recheck evidence: No substantive findings or fixes required. Reviewer confirmed the job-level `workflow_dispatch` condition skips `push` and `pull_request`, Windows definition/matrix/steps remain recoverable, macOS block is unchanged, and Ruby Psych 3.1.0 parses the file. `actionlint` is unavailable on PATH. Rechecked with `python3 tools/validate_planning.py` and the Ruby YAML parse command above.
Acceptance decision, accepting role and date (or pending): accepted by EveryGraph Implementation / P0-016 executor on 2026-10-31 under AGENTS.md section 0 rule 11, after the independent review above and scoped checks. This accepts only P0-016's workflow change, not a phase exit, Windows gate, CI execution, platform qualification, or human approval. The authorized local acceptance commit is verified in the handoff.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): sync row P0-016 to `accepted`, proven by this record's acceptance commit (hash verified in Git history after commit); P0-016 does not edit the board.

Last verified state: accepted and committed in the shared working tree. Working directory `/Users/leo/Documents/General/EveryGraph`, branch `main`; acceptance commit subject and exact changed paths were verified after commit.

Blockers and unblock actions: P0-000 must apply the requested board sync; a CI run remains unauthorized and is not part of this task's evidence.

Next task / concrete next action: P0-000 syncs row P0-016 from the accepted record and proving commit. No further P0-016 implementation work remains.
