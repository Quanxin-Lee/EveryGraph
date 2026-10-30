# P2-003: Import Delimited Text and Clipboard Data

## Metadata

- Task ID: P2-003
- Status: blocked
- Owner: EveryGraph Data
- Record writer: not yet assigned; the initial scaffold was written by P0-000 at allocation
- Related requirements: ORG-IO-002; ORG-IO-008; DG-IO-001; DG-IO-002; DG-IO-003; DG-IO-004; DG-IO-005; DG-IO-009
- Dependencies: P2-002
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: not started; `main` at the P0-000 allocation commit on 2026-10-31. The P2 data core and conformance plan are not accepted.
- Authorization: not granted. Required: create text-import sources under `modules/data`, tests under `tests/data`, module-local CMake registration and this record. No Excel, NetCDF, SQL, UI, dependency change, Windows execution, push or publication.

## Scope and acceptance

Objective: implement the Qt-free parsing and import core for delimited text and clipboard text, producing typed worksheet data through the P2 data model. The task covers the shared semantics of CSV/TXT and paste, not platform clipboard integration or Excel.

Allowed files and interfaces: new sources under `modules/data/src/import/` and public headers under `modules/data/include/everygraph/data/import/`, new tests under `tests/data/import/`, the `modules/data/CMakeLists.txt` and `tests/data/CMakeLists.txt` entries required by the P1-011 topology, and this record. The data model public interfaces are consumed; no other module is changed.

Out of scope: Excel/XLS/XLSM, clipboard OS integration, drag-and-drop UI, import scheduling/connectors, export, plotting, Windows execution and any claim that ORG-IO-001 or the 30+ format catalog is complete.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Delimiter detection and explicit override cover comma, tab, semicolon, pipe and space; quoting, embedded delimiters, line endings and encodings are parsed without corruption | Not run |
| AC-2 | Header, skip-row, row-interval and region rules preserve source-line provenance and do not partially import on a malformed record | Not run |
| AC-3 | Decimal-comma locales, signed zero, missing fields, dates and stored precision are handled explicitly; display formatting never changes stored values | Not run |
| AC-4 | Clipboard text parsing supports delimiter, decimal separator, transpose and trim options; a failed parse leaves the destination unchanged | Not run |
| AC-5 | Import Special rules (header, skip, label, mask, subheader, end-tag) are saved and replayed deterministically | Not run |
| AC-6 | Golden fixtures cover valid, ambiguous, malformed and adversarial text; diagnostics contain record/line positions and stable error codes | Not run |
| AC-7 | Tests pass in `macos-debug`, `macos-release` and `macos-sanitize`; no Qt type appears in public headers | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-003 and created this initial record scaffold; no import code started | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |

No parser, importer or test exists. This record is an allocation plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. No Excel or vendor application is used.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | Allocation-time source inspection | No import module, text parser or clipboard parsing core exists | Confirms the task is not started |
| 2026-10-31 | Allocation-time dependency state | The data model is not implemented | The task is blocked before implementation |

Checks not run and their reasons: parser tests, locale fixtures, malformed-file diagnostics, clipboard-option tests and preset runs. They require P2-002 and authorization.

## Review and disposition

Reviewer and review type: independent data/IO review is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is blocked and not authorized.

## Handoff and remaining work

Last verified state: no delimited-text import exists. The relevant contracts are [data model](../architecture/data-model.md) section 1 and the functional scope IO gate.

Blockers and unblock actions: P2-002 must provide the typed data model; P2-001 should define the fixtures. The user must authorize the implementation and ensure the build-file lock; do not run concurrently with another task editing `modules/data/CMakeLists.txt`. Excel and OS clipboard integration remain separate tasks.

Next task / concrete next action: after P2-002, implement the parser and import commands, add golden and malformed fixtures, run the three Qt-free presets, and request review. P2-008 consumes the import path for Excel; P2-007 consumes imported tables for the view.

Files or commands needed to resume: [data model](../architecture/data-model.md), [functional scope](../planning/functional-scope.md), [feature traceability](../planning/feature-traceability.csv), [P2-001 record](p2-001-data-conformance-test-plan.md), [P2-002 record](p2-002-typed-data-model-immutable-versions.md).

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> blocked | Allocated by P0-000; P2-002 is not accepted |
