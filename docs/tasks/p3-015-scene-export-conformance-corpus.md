# P3-015: Build the golden scene export conformance corpus

## Metadata

- Task ID: P3-015
- Status: blocked
- Owner: EveryGraph Verification
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: DG-UI-010;EG-ADD-009
- Dependencies: P2-012;P3-008;P3-011;P3-013;P3-014
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation on 2026-10-31. The dependencies are not accepted and no branch or worktree exists for this task.
- Authorization: not granted. Required: create or edit only the files listed in the allowed files, register the module or test target once the build topology is fixed, and record evidence. No UI work outside the scope, no persistence, engine or dependency beyond the contract, no Windows execution, no push or publication.

## Scope and acceptance

Objective: build the independent golden coordinate, scene and export corpus and verify font, DPI, clipping, math-policy, accessibility and example-file behavior for the whole P3 surface.

Allowed files and interfaces: new `tests/conformance/` fixtures, reference images and test sources, new `examples/` lawful deterministic example files, a new `docs/engineering/plot-conformance-plan.md`, and this record. Product code, other task records and the feature register are read-only.

Out of scope: fixing product defects found by the corpus, changing P3 acceptance criteria, new product capabilities, vendor fixtures without rights, Windows execution and release packaging.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The golden coordinate and scene corpus covers every P3 plot family with reference images and declared font and antialias tolerances. | Not run |
| AC-2 | The export corpus verifies PDF, SVG and EPS structure and PNG, TIFF and JPG pixels with independent readers. | Not run |
| AC-3 | Font, DPI, clipping, transparency and math-policy checks are explicit; missing objects or wrong coordinates fail regardless of pixel tolerance. | Not run |
| AC-4 | Accessibility checks cover keyboard, focus, labels and non-color encodings for table and plot editing; manual limits are recorded. | Not run |
| AC-5 | Example files are lawful and deterministic and open without external services; the corpus runs headless in the three macOS presets. | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P3-015 and created this initial record scaffold; no implementation started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

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

Next task / concrete next action: Report gaps to the owning P3 tasks instead of fixing their code; P3-017 uses this corpus as exit evidence.

Files or commands needed to resume: [rendering architecture section 6](../architecture/rendering-architecture.md), [P3-008 record](p3-008-graph-editor-ui-styles.md), [P3-011 record](p3-011-scene-layout-clipboard-export.md), [P3-013 record](p3-013-polar-ternary-specialty-charts.md), [P3-014 record](p3-014-image-documents-digitizer.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000 at the user's request; the P2 exit gate and the listed dependencies are not accepted. |
