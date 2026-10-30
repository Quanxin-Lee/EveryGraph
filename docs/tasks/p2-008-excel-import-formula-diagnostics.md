# P2-008: Import Excel Workbooks and Diagnose Unsupported Formulas

## Metadata

- Task ID: P2-008
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-IO-003; ORG-DATA-006; DG-IO-006
- Dependencies: P1-013; P2-003; P2-005
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. Text import and the formula engine are not accepted.
- Authorization: not granted. Required: create an Excel import adapter under `modules/data`, lawful test fixtures, module-local CMake registration and this record. No Excel installation, macro execution, export, dependency installation outside the P1-003 graph, Windows execution, push or publication.

## Scope and acceptance

Objective: import XLS, XLSX and XLSM workbook sheets without Microsoft Excel, preserving typed cells, dates, missing values and stored precision, and reporting unsupported Excel formulas instead of silently converting them. The adapter consumes the P2 text/typed-data path and the formula engine's supported-function registry.

Allowed files and interfaces: new sources under `modules/data/src/import/excel/`, public headers under `modules/data/include/everygraph/data/import/`, new tests and rights-checked fixtures under `tests/data/import/excel/`, the `modules/data/CMakeLists.txt` and `tests/data/CMakeLists.txt` entries required by P1-011, and this record.

Out of scope: exporting XLSX, installed Excel or COM automation, macro execution or formula evaluation by Excel, NetCDF/MATLAB/SQL adapters, UI dialogs, Windows execution and any claim that all Excel formulas or ORG-IO-003 are complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | XLS, XLSX and XLSM files open through the pinned dependency graph without Microsoft Excel; multiple sheets and cell ranges are selectable | Not run |
| AC-2 | Numeric, text, date, boolean and missing cells preserve their types, date system, precision and cell-level provenance | Not run |
| AC-3 | A documented supported Excel-formula subset maps to the P2 formula engine; unsupported or error formulas are reported per cell without executing macros | Not run |
| AC-4 | Embedded macros, external links, active content and malformed/encrypted files are never executed; they are rejected or imported with an explicit loss report | Not run |
| AC-5 | Lawful fixtures cover legacy and OOXML formats, multiple sheets, date systems, merged cells, hidden sheets and corrupt files; no vendor file is redistributed without rights | Not run |
| AC-6 | Golden import tests pass in the three Qt-free presets; no Qt type appears in public headers; the adapter cannot write or recalculate the source workbook | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-008 and created this initial record scaffold; no Excel adapter started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No Excel reader, fixture or formula diagnostic exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. Excel is intentionally absent. Any workbook library must be resolved through the P1-003 vcpkg manifest, not an ad hoc package manager.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No Excel adapter, workbook reader or formula diagnostic exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | Text import and the formula engine are not accepted | The task is blocked before implementation |

Checks not run and their reasons: workbook import, formula diagnostics, macro/security cases, corrupt-file handling and preset runs. They require P2-003, P2-005, lawful fixtures and authorization.

## Review and disposition

Reviewer and review type: independent IO/security review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no Excel import exists. The import contract is in [P2-003](p2-003-delimited-text-import-clipboard.md); the formula registry is in [P2-005](p2-005-formula-expression-dag.md).

Blockers and unblock actions: P2-003 and P2-005 must be accepted; lawful fixtures and any workbook library rights must be reviewed. The user must authorize the adapter and ensure the build-file lock. No Excel installation or macro execution is allowed.

Next task / concrete next action: after the dependencies and fixture rights review, implement the reader and formula diagnostics, add macro/corrupt-file cases, run the three Qt-free presets, and request review. Export remains a separate task.

Files or commands needed to resume: [feature traceability](../planning/feature-traceability.csv), [functional scope](../planning/functional-scope.md), [P2-003 record](p2-003-delimited-text-import-clipboard.md), [P2-005 record](p2-005-formula-expression-dag.md), [P1-003 record](p1-003-dependency-pins-sbom.md), [licensing policy](../engineering/licensing-and-dependencies.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-003 and P2-005 are not accepted |
