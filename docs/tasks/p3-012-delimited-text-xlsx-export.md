# P3-012: Implement delimited text and XLSX data export

## Metadata

- Task ID: P3-012
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-EXPORT-001;DG-EXPORT-002
- Dependencies: P2-003;P2-006;P2-008;P2-012
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: implement delimited text and XLSX data export from typed worksheet data with explicit locale, precision, missing-value and metadata rules.

Allowed files and interfaces: `modules/data` export sources, `tests/data` tests and this record. Import parsers, the project container and the pinned XLSX dependency are consumed through their public interfaces.

Out of scope: graph or layout export, foreign-format claims, macro or formula execution, UI, Windows execution and any P6 compatibility claim.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | All or selected rows and columns export as tab or comma text with typed values, precision, headers and metadata. | Not run |
| AC-2 | XLSX export preserves typed values, dates, missing values and precision and opens in an independent reader. | Not run |
| AC-3 | Locale, encoding, delimiter and quoting rules are explicit; loss and unsupported constructs are reported. | Not run |
| AC-4 | No macro or external link is executed; formulas are exported as values unless a documented subset is explicitly requested. | Not run |
| AC-5 | Golden round-trip tests pass in the three macOS presets; no foreign-format compatibility claim is made. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-012 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

This record is an allocation plan, not completed work.

## Validation

Working directory: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`; Python 3.13.9. The task is blocked by the phase gate; no scene, plot, export or UI implementation for this task exists.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source and dependency inspection | Read the current board, roadmap, functional scope, rendering architecture and dependency records; no scene, plot, export or UI implementation for this task exists and the dependencies are not accepted. | Read-only check; not a test of the task |

Checks not run and their reasons: scene, plot, export, UI and preset tests. They require the P2 exit gate, accepted dependencies and authorization.

## Review and disposition

Reviewer and review type: independent review is required for this task. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is not authorized.

## Handoff and remaining work

Last verified state: No implementation exists for this task; only the P0-000 allocation record and the board row exist.

Blockers and unblock actions: P2-012 and the listed P3 dependencies must be accepted (or explicitly gated by the user), and the user must authorize the module directories, build registration and a single writer per shared file.

Next task / concrete next action: P3-015 may add the export corpus cases. Verify the independent-reader result and request review.

Files or commands needed to resume: [P2-003 record](p2-003-delimited-text-import-clipboard.md), [P2-006 record](p2-006-project-container-atomic-save-recovery.md), [P2-008 record](p2-008-excel-import-formula-diagnostics.md), [functional scope](../planning/functional-scope.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
