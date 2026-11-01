# P0-020: Record the macOS P1 Entry-Gate Disposition as an ADR and Align the Roadmap Exit Column

## Metadata

- Task ID: P0-020
- Status: planned
- Owner: EveryGraph Lead
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: EG-ADD-009;EG-ADD-016;DG-UI-009
- Dependencies: P0-008; P1-013; P1-014; P1-015; P1-016
- Created: 2026-11-01
- Updated: 2026-11-01
- Branch / worktree / base revision: not started; `main` at the P0-000 sync-and-decision commit on 2026-11-01. P0-008, P1-013, P1-014, P1-015 and P1-016 are accepted.
- Authorization: not granted. Required: create one ADR, revise one sentence of the roadmap's P1 exit column and maintain this record. No product source change, no dependency installation, no traceability status change, no push or publication is authorized by this task.

## Scope and acceptance

Objective: put the coordinator's P1-exit disposition into the register AGENTS.md section 10 names for lasting decisions, so the gate basis is an ADR plus roadmap wording rather than a paragraph inside a coordination record. P1-013 graded the macOS P1 exit **not passed**; P1-015 and P1-014 then closed the non-deterministic `accessibility_ime_native` and `accessibility_focus_native` entries, and P1-016 closed the command-gateway snapshot boundary. What still cannot be evidenced on this host is not a defect: this Mac has one display — `system_profiler SPDisplaysDataType` returns only the built-in `3024 x 1964 Retina` panel with `Main Display: Yes` and no second display — so physical high-DPI and multi-monitor qualification is impossible here; no cold-cache recipient rebuild has run; the SBOM still carries `licenseConcluded: NOASSERTION` and no per-file copyright audit or written corresponding-source offer exists; Windows x64 stays deferred by [ADR-0005](../decisions/ADR-0005-platform-sequencing.md).

The disposition this task must record, as decided by P0-000 on the user's 2026-11-01 delegation: those four residuals become **release and distribution gates**, not gates on starting P2 engineering, and the macOS P1 exit is therefore treated as an **entry gate that is satisfied** while every one of the four stays explicitly open and unclaimed.

Allowed files and interfaces: a new `docs/decisions/ADR-0006-*.md`, the P1 exit column sentence in [development roadmap](../planning/development-roadmap.md) §3 and this record. P0-000 designates this task as the writer for those paths for its next revision.

Out of scope: any change to [feature traceability](../planning/feature-traceability.csv), the task board, AGENTS.md, the engineering guides, the minimum platform, the license policy, the packaging scripts or any product source; any claim that a physical display, a cold-cache recipient machine, a clean environment or a Windows host has been qualified; any weakening of an acceptance criterion, which this ADR must instead restate as a gate that stays open.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The ADR states the decision, its context, the alternatives rejected — declaring a flat pass, or keeping P2 closed indefinitely on hardware this host lacks — and its consequences, in the existing ADR shape, and supersedes nothing silently | Not run |
| AC-2 | The ADR names each residual gate with what would close it: two physical displays with a real display-mode change and a hot-plug; a cold-cache recipient rebuild without the retained `out/vcpkg-installed`; a per-file copyright audit plus a written corresponding-source offer; the Windows x64 host or authorized CI, and it records that a screen reader has never run against the tree | Not run |
| AC-3 | The roadmap's P1 exit column is rewritten to distinguish the entry gate it now describes from the release gates it defers, in one sentence, with the deferred items still listed by name rather than deleted, and no Developer ID, notarization, clean-machine or Windows claim is introduced | Not run |
| AC-4 | Nothing in the ADR or the roadmap sentence says or implies that P1-013's matrix rows E2c, E2d, E6c or E6d passed, that accessibility is fully qualified, or that the license position is cleared; the record quotes the sentences back to prove it | Not run |
| AC-5 | `python3 tools/validate_planning.py` and `python3 tools/validate_catalog_children.py` pass with the new ADR linked, every local link resolves, the English policy holds, and traceability row counts and statuses are byte-for-byte unchanged | Not run |
| AC-6 | The six ADR files' numbering, titles and cross-links stay consistent, and the new ADR is referenced from the roadmap sentence it supports | Not run |
| AC-7 | An independent reviewer that is not the author reads the ADR against P1-013's matrix and the four accepted records and states explicitly whether any criterion was weakened; the review, the reviewed revision and the recheck are recorded here, the record moves to `accepted`, and a `Board sync` request names row P0-020 and the proving commit | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-11-01 | P0-000 / allocation executor | Allocated P0-020 to carry the coordinator's P1-exit disposition into the ADR register, recorded the one-display finding from `system_profiler` as the reason physical display qualification cannot run here, and created this initial record scaffold; no decision document was written | [Board](../planning/task-board.csv); [P1-013 record](p1-013-p1-exit-verification.md); [P0-000 record](p0-000-task-allocation.md) |

No ADR exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-11-01 | `system_profiler SPDisplaysDataType` | Only the built-in `Apple M5` display, `Resolution: 3024 x 1964 Retina`, `Main Display: Yes` | Confirms P1-013's `screens: 1` and that gap G1 cannot be closed on this host; one display is not evidence about multi-monitor behaviour |
| 2026-11-01 | `ls docs/decisions/` | ADR-0001 through ADR-0005 exist | Fixes the new ADR number at ADR-0006 and requires the existing five to stay consistent |
| 2026-11-01 | Read of the roadmap §3 P1 exit column | It currently lists the native build, keyboard/IME/high-DPI/multimonitor workflows, the interface boundary, the fake-fixture command path and the bundle/rebuild evidence as one undivided exit | Confirms a sentence needs changing and that the deferred items must be named, not removed |

Checks not run and their reasons: the ADR itself, the roadmap rewrite and the validator pair with the new document present. They need an executor and this task's manifest.

## Review and disposition

Reviewer and review type: an independent reviewer that is not the author and that reads the ADR against P1-013's matrix, because the risk of this task is silent criterion-weakening rather than a wrong word. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not started.

## Handoff and remaining work

Last verified state: the three residual P1 work streams that needed code or a host change are closed — P1-015's helper, P1-014's activation precondition plus the guide sentences, P1-016's boundary rejection — and P0-000 holds the exit disposition only in this coordination record until this ADR exists.

Blockers and unblock actions: none inside this task. Its dependencies are accepted. **P2-002 is held open by this task**: the data-model lane starts only once the gate basis is in the ADR register, and P0-000 added this task to that row's dependencies for that reason.

Next task / concrete next action: claim P0-020, correct this scaffold's `planned` status and add the Change manifest as the first write, write ADR-0006 with the rejected alternatives and the named residual gates, revise the single roadmap sentence, run both validators, and file the `Board sync` request. When this task is accepted, P0-000 opens row P2-002.

Files or commands needed to resume: [P1-013 record](p1-013-p1-exit-verification.md), [P1-014 record](p1-014-native-focus-order-determinism.md), [P1-015 record](p1-015-native-ime-commit-helper.md), [P1-016 record](p1-016-command-gateway-snapshot-invariant.md), [ADR-0005](../decisions/ADR-0005-platform-sequencing.md), [roadmap](../planning/development-roadmap.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-11-01 | New -> planned | Allocated by P0-000 after the user delegated the pending decisions; the disposition is a lasting platform-qualification decision and therefore belongs in the ADR register, not only in a coordination record |
