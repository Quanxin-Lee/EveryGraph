# P1-008: Qualify Accessibility, Keyboard/IME, High-DPI and Multi-Monitor Behavior

## Metadata

- Task ID: P1-008
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-009; DG-UI-009; ORG-UI-004
- Dependencies: P1-006; P1-007
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated branch or worktree must be created after P1-006/P1-007.
- Authorization: not granted. Required: add or extend Qt Test checks, run a documented manual checklist on macOS arm64, and update the development guide and this record. No real datasets, Windows execution, release certification, commit, push or publication is authorized.

## Scope and acceptance

Objective: establish the first truthful accessibility and display/input qualification for the shell. The goal is evidence about the implemented P1 shell, not a claim of full WCAG conformance or assistive-technology certification.

Allowed files and interfaces: the `apps/desktop` and `modules/presentation_qt` sources and tests created by P1-006/P1-007, test fixtures, `docs/engineering/development.md` and this record. No changes to the application/domain contracts are expected.

Out of scope: full WCAG audit, screen-reader certification on every platform, Windows behavior, mobile/touch support, real scientific table/plot accessibility, a full IME language matrix and any claim that all P1 accessibility requirements are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Keyboard focus order through the main window, project tree, document tabs and preferences is recorded and usable; no focus trap is found in the tested workflows | Not run |
| AC-2 | Shell controls expose accessible names/roles/descriptions; a Qt accessibility inspection records the observed values | Not run |
| AC-3 | Text and control contrast is checked against the selected theme; non-color cues exist for tested status/selection states | Not run |
| AC-4 | IME composition is exercised in a text-entry fixture on macOS with a CJK input method; the observed behavior and limits are recorded | Not run |
| AC-5 | The shell is checked at 1x and 2x scale factors; icons, text and dock sizes remain usable and no clipped critical control is found | Not run |
| AC-6 | Detach/float and restore are exercised across two displays or a simulated display topology; an off-screen window is recovered | Not run |
| AC-7 | Automated Qt Test checks and the manual checklist are recorded with exact commands, environment, results and limitations; no Windows or certification claim is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 exit requirements; no accessibility or display test has run | [Takeover audit](p0-002-takeover-state-verification.md), [functional scope](../planning/functional-scope.md) and [board](../planning/task-board.csv) |

No automated accessibility test, manual checklist or display matrix exists. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. Qt must come from the P1-003 pinned graph. A second display or a documented simulated topology is required for the multi-monitor check.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Source inspection | No shell, accessibility test or display fixture exists | P1-006/P1-007 are prerequisites and are blocked |
| 2026-10-30 | Existing Qt probe smoke | Offscreen smoke passes in both Qt presets | Probe only; it does not test accessibility, IME, scaling or multi-monitor behavior |
| 2026-10-30 | None | Not run | Blocked task; no qualification evidence exists |

Checks not run and their reasons: keyboard navigation, accessibility inspection, contrast, IME composition, high-DPI scaling, multi-monitor detach/restore and the manual checklist. They require the shell and authorization.

## Review and disposition

Reviewer and review type: independent QA/accessibility review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: the only Qt artifact is the P1-001 probe; no shell accessibility or display behavior exists.

Blockers and unblock actions: P1-006 and P1-007 must be accepted. The user must authorize the test additions and provide or approve a second-display test setup. A CJK input method must be available for the IME check; if it is not, the task records the limitation rather than claiming a pass.

Next task / concrete next action: after the shell and preferences are accepted, run the automated and manual checks, record exact evidence and limitations, and request independent review. P1-010 reuses the same shell smoke suite on Windows only after a Windows host exists.

Files or commands needed to resume: [architecture overview](../architecture/architecture-overview.md), [development guide](../engineering/development.md), [P1-006 record](p1-006-workbench-shell-document-host.md), [P1-007 record](p1-007-preferences-appearance-focus-mode.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-006/P1-007 are blocked and no qualification is authorized |
