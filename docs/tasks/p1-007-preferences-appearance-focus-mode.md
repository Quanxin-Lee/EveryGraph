# P1-007: Implement Preferences, Appearance and Focus Mode

## Metadata

- Task ID: P1-007
- Status: blocked
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: DG-UI-008; DG-UI-009; ORG-UI-005
- Dependencies: P1-006
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated branch or worktree must be created after P1-006.
- Authorization: not granted. Required: extend the Qt shell with a preferences surface, settings persistence, appearance and focus-mode behavior, Qt Test coverage and documentation. No real project data, cloud settings, Windows execution, commit, push or publication is authorized.

## Scope and acceptance

Objective: make the P1 shell configurable without mixing user preferences with scientific document state. Interface settings must persist across restarts, invalid values must fall back with diagnostics, and dark appearance must not alter saved publication scene colors or exported backgrounds.

Allowed files and interfaces: the `apps/desktop` and `modules/presentation_qt` sources created by P1-006, Qt Test targets, CMake target/test registration, `docs/engineering/development.md` and this record. The P1-004 command kernel remains the only document writer.

Out of scope: real project persistence, plot/scene color editing, scientific defaults, cloud synchronization, multi-user profiles, Windows execution and any claim that all preferences or all accessibility behavior is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A preferences surface persists interface, focus-mode, function-library and default-format settings across an application restart using a documented local store | Not run |
| AC-2 | Invalid or unknown settings fall back to documented defaults and emit a diagnostic without crashing or silently changing scientific state | Not run |
| AC-3 | Dark appearance changes shell chrome only; a fake scene color and export background remain unchanged in the corresponding test | Not run |
| AC-4 | Data-only and commands-only focus modes can be toggled; panels can be resized/rearranged; keyboard access remains possible on a small-screen fixture | Not run |
| AC-5 | Qt Test covers settings round-trip, invalid-value fallback, appearance isolation and focus-mode toggling; the existing suites remain green | Not run |
| AC-6 | The development guide records the settings location/schema, commands, results and limits; no Windows or release claim is made | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the P1 workspace and appearance requirements; no settings or UI code started | [Takeover audit](p0-002-takeover-state-verification.md), [functional scope](../planning/functional-scope.md) and [board](../planning/task-board.csv) |

No preferences surface, settings store or test exists. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. Qt must come from the P1-003 pinned graph; the Homebrew probe is not the final dependency route.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Source inspection | No shell or settings implementation exists | P1-006 is the prerequisite and is blocked |
| 2026-10-30 | Existing Qt probe tests | Qt presets each 4/4 passed | Probe only; no preferences or focus-mode behavior exists |
| 2026-10-30 | None | Not run | Blocked task; no implementation test exists |

Checks not run and their reasons: settings persistence, invalid-value fallback, dark-mode isolation, focus-mode keyboard access, small-screen layout and Qt Test. They require P1-006 and authorization.

## Review and disposition

Reviewer and review type: independent UI review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not started.

## Handoff and remaining work

Last verified state: no preferences, appearance or focus-mode behavior exists; P1-006 is blocked by P1-001/P1-003/P1-004.

Blockers and unblock actions: P1-006 must build the shell and document host. The user must authorize the preferences surface, local settings store and Qt Test additions. A decision on the settings schema and migration policy is required before persistence is implemented; this task should not invent a project-file format.

Next task / concrete next action: after P1-006, implement the preferences and focus-mode behavior with fake scene colors, run Qt Test, and request independent review. P1-008 consumes the keyboard/focus behavior for the broader accessibility qualification.

Files or commands needed to resume: [architecture overview](../architecture/architecture-overview.md), [module design](../architecture/module-design.md), [development guide](../engineering/development.md), [P1-006 record](p1-006-workbench-shell-document-host.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> blocked | P1-006 is blocked and no UI/settings implementation is authorized |
