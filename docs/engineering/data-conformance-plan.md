# P2 Data and Persistence Conformance Plan

Status: normative test plan for P2 implementation, version 1.0, 2026-10-31. This document defines the fixtures, expected semantics, diagnostics, tolerances and measurement procedures that P2 data, import, formula and persistence tasks must satisfy. It does not implement tests, change an architecture contract, or claim that any P2 capability is delivered or covered.

This plan is the deliverable of [P2-001](../tasks/p2-001-data-conformance-test-plan.md). Its authorities are the [functional scope](../planning/functional-scope.md) section 3 shared gates, the [data model](../architecture/data-model.md), the [project file format](../architecture/project-file-format.md), the [concurrency model](../architecture/concurrency-model.md), the [module design](../architecture/module-design.md), [ADR-0004](../decisions/ADR-0004-data-storage.md) and the [performance protocol](performance-validation.md). If this plan conflicts with one of those contracts, the contract wins and the conflict is recorded as a plan defect.

All names of future files, APIs, fixtures and diagnostic codes are proposed conformance identifiers. They become stable only when an owning implementation task records them. No vendor name, count or speed claim is a conformance threshold.

## 1. Purpose, consumers and precedence

The plan exists so that P2 tasks do not invent acceptance after implementation. Each P2 implementation task must implement the fixture families assigned to it, assert the declared semantics and diagnostics, and record actual results. Missing families keep that task open; passing a subset is not a coverage percentage.

| Consumer task | Families this plan requires | Boundary |
|---|---|---|
| [P2-002](../tasks/p2-002-typed-data-model-immutable-versions.md) | TYP, ID, UNIT, ALIGN core | In-memory typed data model and immutable versions |
| [P2-003](../tasks/p2-003-delimited-text-import-clipboard.md) | IO (CSV, TXT, clipboard) | Qt-free text parsing and import rules |
| [P2-004](../tasks/p2-004-worksheet-edit-commands-undo.md) | ID, ALIGN invariants, edit slices of TYP and UNIT | Revision-checked edit commands and undo |
| [P2-005](../tasks/p2-005-formula-expression-dag.md) | FML, UNIT propagation | Expression and formula DAG engine |
| [P2-006](../tasks/p2-006-project-container-atomic-save-recovery.md) | SAVE, REC, MIG, round-trip of all prior families | Native SQLite and Arrow project container |
| [P2-007](../tasks/p2-007-virtual-worksheet-view.md) | Display slices of TYP, ID and ALIGN | View must not change stored semantics |
| [P2-008](../tasks/p2-008-excel-import-formula-diagnostics.md) | IO diagnostics subset, FML registry subset, rights rules | XLS, XLSX and XLSM adapter after dependencies |
| [P2-009](../tasks/p2-009-reshape-query-grouping.md) | Shared ID lineage and alignment invariants only; operation-specific merge, append, split, transpose, pivot and query fixtures remain in the P2-009 record | Reshape and query commands |
| [P2-010](../tasks/p2-010-masks-mirrors-filtered-views.md) | ALIGN, including same-domain and foreign-domain mirrors | Sorting, filtering, groups, masks and mirrors |
| [P2-011](../tasks/p2-011-large-data-performance-baseline.md) | PERF | First truthful import and save baseline |

This plan defines shared semantics and the families named in its acceptance criteria. Operation-specific command fixtures that do not change these shared invariants remain owned by the relevant task record: P2-004 for edit, redo, repeat and event cases; P2-009 for reshape and query cases; P2-010 for sort, group and mask policy cases. Those tasks may cite this plan but must not use it to replace their own command-level fixtures.

Precedence for a test expectation: architecture and format contracts, then this plan, then the consuming task record. A task that needs a different expected result must propose a plan revision with the semantic impact, not quietly change a fixture after seeing a failure. Loosening a tolerance, deleting a fixture or converting a failure into an expected failure requires data/QA review and a version change in section 15.

## 2. Non-claims and boundaries

This plan is not:

- a test implementation, a CI result, or evidence that P2 capabilities work;
- a coverage claim derived from fixture or requirement counts;
- a change to the 428-row feature register, the task board, the architecture, or the native format;
- a promise about foreign project formats, Excel formula completeness, password protection, packaging or release quality;
- a Windows, Linux or Intel qualification; procedures are written to be portable, but only executed results count;
- a performance budget. It defines how to measure; P2-011 sets the first measured baseline and later review sets budgets;
- authorization to acquire proprietary fixtures, install dependencies, execute vendor software, commit or publish.

Proprietary or personal data is not committed. Fixtures are synthetic or lawfully redistributable. Large performance data is generated and kept out of version control. Imported content is data only: macros, external links, embedded objects and formula-like text are never executed or silently activated.

## 3. Fixture, evidence and diagnostic conventions

### 3.1 Fixture identifiers

Use stable fixture IDs of the form `EG-FX-<FAMILY>-<NNN>`, where `FAMILY` is one of `TYP`, `ID`, `UNIT`, `ALIGN`, `IO`, `FML`, `SAVE`, `REC`, `MIG` or `PERF`. IDs are never reused or renumbered. A retired fixture keeps its row with a retired status and a replacement pointer. A fixture can serve several tests, but every test reports the fixture IDs it exercised.

The fixture tables in this plan use the short form `<FAMILY>-NNN`; the full ID prepends `EG-FX-`.

### 3.2 Fixture manifest

The fixture index is composed of task-local manifest fragments, not one file written by every task. The first consumer, P2-002, creates `tests/data/fixtures/manifest.json` for its own fixtures inside its allowed test tree. Each later task creates or extends a fragment inside its own allowed test tree, for example P2-003 under `tests/data/import/`, P2-005 under `tests/dataflow/`, P2-006 under `tests/storage/`, P2-004, P2-009 and P2-010 under `tests/data/commands/` with distinct filenames such as `manifest-p2-004.json`, `manifest-p2-009.json` and `manifest-p2-010.json`, and P2-011 under `benchmarks/`. A task whose record authorizes a different test location uses that location. The test harness reads all fragments and rejects duplicate fixture IDs. No task writes another task's fragment. A single shared manifest outside these trees requires P0-000 allocation and a designated writer. The storage format may change, but these fields are required:

```json
{
  "fixture_id": "EG-FX-TYP-005",
  "family": "TYP",
  "title": "Signed zero storage and comparison",
  "requirement_ids": ["DG-DATA-006"],
  "kind": "generated",
  "input": {"generator": "generate_typed_values", "generator_version": 1, "seed": 20261031},
  "sha256": "content hash of the generated fixture",
  "bytes": 0,
  "schema": {"columns": [{"name": "x", "type": "float64", "unit": "1", "role": "X"}]},
  "expected": {"values": "path or inline canonical data", "diagnostics": [], "invariants": []},
  "expected_status": "pass",
  "options": {"header": "present", "delimiter": "comma", "trim": "none"},
  "policy_refs": ["data-model:section-3"],
  "tolerance": "bit-exact",
  "environment": {"locale": "C", "timezone": "UTC"},
  "platform_scope": "all",
  "rights": "synthetic",
  "consumer_tasks": ["P2-002", "P2-006"],
  "status": "active",
  "review_status": "pending",
  "last_verified": null
}
```

Rules:

- The fragment set is the single source of expected results. A test does not copy a second golden table that can drift. Each fragment is owned by the task that creates it; a later task adds its own fragment rather than editing another owner's file.
- `sha256` and actual bytes are recorded for generated fixtures; the generator version and seed reproduce the content. The generator is deterministic across platforms.
- Inline expected data is used only for small fixtures. Large expected data is generated from the same reference model, not committed.
- A fixture marked `rights: proprietary` is invalid unless a later task records a written lawful basis and the file remains uncommitted when redistribution is not allowed.
- `expected_status` is `pass`, `expected_diagnostic` or `deferred`; a deferred fixture names its owner and target task and is excluded from the owning task's acceptance evidence.
- `options` records every parse or operation option that affects the expected result. `policy_refs` points to the contract clause or this plan section that resolves any declared policy. A fixture whose expected result depends on an unresolved policy is not reviewed or accepted; it stays `planned` with the open decision named.
- `status` is `planned`, `active`, `retired` or `deferred`. Retired IDs remain with a replacement pointer.
- `review_status` changes only through the review rules in section 15.

### 3.3 Files, text and environment

- Committed text fixtures use UTF-8 without BOM unless the case intentionally tests a BOM. Line endings are part of the fixture: cases declare LF, CRLF or CR and store bytes, not a normalized working-tree copy. Git settings must not silently rewrite fixture line endings.
- Tests set locale, timezone and decimal separator explicitly. No test depends on the host locale, host timezone, filesystem iteration order, current wall clock or a random value without a recorded seed.
- Date and time tests use IANA zone identifiers, not fixed offsets alone, when DST behavior matters. The timezone database source and version are recorded with the test result.
- Unicode tests use escaped code points in source where an editor could normalize the bytes. The specification uses code-point sequences, not a required normalization form.
- Binary fixtures are generated from a documented layout and hash; they are not opaque hand-edited blobs.

### 3.4 Golden comparison and tolerances

| Data class | Comparison rule |
|---|---|
| Stored integers, booleans, categorical codes | Exact value and storage type |
| Stored IEEE values | Exact bit pattern for round-trip and save/reopen cases, including the sign of zero and the NaN payload when the container claims bit preservation |
| NaN aggregate equality | All NaN payloads compare equal for change detection and structural equality; bit preservation is a separate round-trip assertion |
| Text | Exact code-point sequence; no implicit trim, case fold or Unicode normalization |
| Missing and error state | Exact state class: missing, error code, NaN, infinite or empty text are never interchangeable |
| IDs, roles, units, mappings, lineage | Exact; no ordering or identity tolerance |
| Parsed decimal numbers | Bit-equal to the declared scalar reference parse, except where the case intentionally tests an overflow or ambiguity diagnostic |
| Basic formula arithmetic | At most 1 ULP of the float64 result for `+`, `-`, `*`, `/` and `sqrt` in the tested build; a compiler contraction difference is resolved by a declared build setting or a reviewed fixture expectation, never by a blanket looser tolerance |
| Transcendental and library functions | Relative error at most 1e-13 unless the registered function declares a tighter or looser reviewed tolerance; the registry tolerance is part of the expected result |
| Unit conversion | Exact when the factor is exactly representable; otherwise relative error at most 1e-12 and absolute error at most 1e-12 in the target unit |
| Dates and timestamps | Exact instant and exact declared zone or offset; date-only values are not converted to an instant |
| Display formatting | Exact expected string for the declared format and C/declared locale; display never changes stored values |
| Diagnostics | Exact stable code, severity and location; message text is not a golden value |

Floating-point tolerances are for computed results. They never authorize a relaxed comparison for stored source data, identity, null semantics or error classification.

For NaN payloads, the in-memory data model must preserve the payload. The native container must record, before P2-006 acceptance, whether its Arrow IPC chunk representation preserves the payload bit pattern. When it does, SAVE-002 asserts bit-exact round-trip. When a declared boundary does not, the limitation, the warning and the affected fixture are recorded; SAVE-002 does not silently weaken a bit-exact claim, and no other TYP fixture may use that boundary to excuse a value change.

### 3.5 Diagnostics

Tests assert a stable code, severity and structured location rather than a prose message. Proposed test-facing codes are listed per family. The implementation may rename them before the owning task freezes its registry, but the final registry must be documented and tests must assert the documented code. User-facing localized text is not parsed.

Locations are at least one of: file path plus byte offset, record number, source line and column, worksheet and cell, column ID, field ID, dataset revision, chunk ID, or container table and row. A diagnostic without a usable location is a defect for import, parsing and storage failures.

Severity classes: `info`, `warning`, `error`, `fatal`. A rejected operation returns `error` or `fatal` and commits nothing. A warning may accompany a successful lossless or explicitly lossy import; a lossy import is never silent.

Proposed family prefixes: `data.`, `import.`, `formula.`, `storage.`, `recovery.` and `migration.`. A family code is a detail code carried inside the application error model; it does not replace the stable application codes in the [module design](../architecture/module-design.md). Where the application API exposes one outer code, tests assert the outer code plus the detail code and location. The required mapping is:

| Test-facing family code | Required outer application code |
|---|---|
| `data.stale_revision` | `revision_conflict` |
| `data.duplicate_id`, `data.dangling_reference`, `data.type_mismatch`, `data.unknown_unit`, `data.incompatible_unit`, `data.positional_mapping_required` | `invalid_argument` |
| `data.number_overflow`, `data.number_ambiguous`, `data.invalid_text`, `data.date_ambiguous`, `data.date_nonexistent`, `data.date_format`, `data.unmapped_row`, `data.duplicate_join_key`, `data.mirror_cycle` | `invalid_argument` |
| `data.precision_loss`, `data.overflow` | Warning detail when the operation continues; `numerical_failure` when the operation rejects the loss or overflow |
| `data.offset_unit_arithmetic` | `invalid_argument` or `unsupported_feature`, as declared by the unit registry |
| `storage.resource_limit` | `resource_exhausted` |
| `storage.save_failed`, `storage.lock_conflict` | `io_error` |
| `storage.format_unsupported`, `storage.corrupt_container`, `storage.checksum_mismatch`, `storage.reference_integrity` | `io_error` or `unsupported_feature`, as declared by the storage API |
| `formula.unknown_name`, `formula.ambiguous_name`, `formula.type_mismatch`, `formula.invalid_reference`, `formula.circular_reference`, `formula.unsupported_syntax` | `invalid_argument` |
| `formula.division_by_zero`, `formula.domain_error` | `numerical_failure` |
| `formula.manual_recalc_required` | A status detail when a value is stale; `invalid_argument` when an operation requires a current value without the manual trigger |
| `import.header_conflict`, `import.decimal_ambiguous`, `import.malformed_quote` | `invalid_argument` |
| `import.limit_exceeded` | `resource_exhausted` |
| Other `import.*` codes | `invalid_argument` for content, `io_error` for read or write failure |
| `recovery.checkpoint_unavailable`, `migration.interrupted`, `migration.failed` | `io_error` |
| `migration.unsupported_downgrade` | `unsupported_feature` |

If an implementation exposes only one code, it records the chosen mapping in its task record; a test may not simultaneously require two incompatible outer codes. Unmapped codes require review before the fixture is accepted.

### 3.6 Test tiers and evidence

| Tier | Content | Cadence | Evidence |
|---|---|---|---|
| T0 | Unit, golden and property tests for one module | Every change, Qt-free presets | Case counts, fixture IDs, exact values and tolerances |
| T1 | Command-gateway integration with revision checks and undo | Every change touching a command path | Command, expected revision, committed revision, event sequence |
| T2 | Failure injection, crash and corruption tests | Every change to storage/import failure paths; scheduled regression | Injection point, expected and observed state, recovery result |
| T3 | Performance and large-data measurements | Scheduled, declared hardware | Fixture hash, build, hardware, distributions, correctness gate |

Every run records: task ID, working tree revision or commit, working directory, exact commands, preset, OS and architecture, compiler, CMake, Ninja and dependency versions, fixture hashes, pass/fail counts, and checks not run. A passing sanitizer run on macOS does not cover Windows adapters. A build without Qt is not a Qt test. T3 is never inferred from T0 timings.

## 4. TYP: typed values and scientific semantics

Covers ORG-DATA-001, ORG-DATA-011, DG-DATA-005, DG-DATA-006 and the value side of EG-ADD-002. Consumer: P2-002, then P2-003, P2-004, P2-006 and P2-007.

### 4.1 Required invariants

1. Every column has an explicit logical type and storage type. A value is never reinterpreted because a later column happens to fit.
2. Missing (null), empty text, numeric zero, signed zero, NaN, positive infinity and negative infinity are distinct states.
3. Integer parsing rejects overflow instead of wrapping. Integer width and signedness survive save/reopen.
4. IEEE values preserve the bit pattern through in-memory versions. The native container preserves the bit pattern for every value covered by its declared payload capability; NaN payload handling is defined in section 3.4. The sign of zero is observable. If a boundary cannot preserve a payload, the limitation and warning are documented; a blanket exception is not allowed.
5. Text preserves code points. No implicit normalization, case fold or trim occurs in storage. Invalid UTF-8 is rejected with a location, not replaced by U+FFFD.
6. Dates and timestamps distinguish date-only, naive timestamp, zoned timestamp and ambiguous local time. Parsing records the chosen zone, offset and DST decision.
7. Display format is a view. Formatting, sorting by displayed text or changing decimals never writes a rounded value into storage.
8. A column's unit, role and display format are metadata, not part of the raw value; they survive rename, reorder, edit and reopen.
9. Column identity is independent of its display label, position and translated name.

### 4.2 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| TYP-001 | Signed and unsigned integer boundaries for 8, 16, 32 and 64 bits | Exact values and types; boundary values round-trip | Bit-exact | P2-002, P2-006 |
| TYP-002 | Integer token above or below the declared width | Rejected, not wrapped or truncated | `data.number_overflow` with source location | P2-003, P2-008 |
| TYP-003 | float32 and float64 values including smallest subnormal, largest finite, 1/3 and 0.1 | Exact storage type and bit pattern; no silent promotion or demotion | Bit-exact | P2-002, P2-006 |
| TYP-004 | Decimal strings with 1, 15, 17 and 18 significant digits; exponent forms | Parsed result bit-equal to the declared scalar reference; lexical token retained in import provenance | Bit-exact; precision-loss warning only if a declared format cannot represent the token | P2-003, P2-006 |
| TYP-005 | Positive zero, negative zero, addition and multiplication that produce signed zero | Sign bit preserved and observable; IEEE numeric comparison treats -0 and +0 as equal; structural identity distinguishes them | Bit-exact plus comparison assertions | P2-002, P2-005, P2-006 |
| TYP-006 | NaN with distinct payloads, positive infinity, negative infinity | NaN is not missing and not an error; infinities are not clipped; in-memory versions preserve the payload; the native container records its payload capability before SAVE-002 is accepted | Bit-exact through in-memory versions and through any boundary that claims payload preservation; an unsupported boundary requires a documented limitation and warning, not a blanket exception | P2-002, P2-006 |
| TYP-007 | Empty string, whitespace-only string, missing cell, zero, NaN, infinity in one table | All states remain distinct through edit and reopen; declared trim may convert whitespace to empty text but never to missing | Exact state class | P2-002, P2-003, P2-007 |
| TYP-008 | Unicode labels and text: combining sequences, emoji, right-to-left marks, U+4E2D, embedded NUL | Code points preserved; embedded NUL follows a declared text policy and is rejected by default; labels remain stable | Exact code points; `data.invalid_text` for NUL when rejected | P2-002, P2-003, P2-006 |
| TYP-009 | Text containing the delimiter, quotes and CRLF or LF inside a quoted field | Value contains exactly the intended characters; line count and field count unaffected | Exact text; source record location retained | P2-003 |
| TYP-010 | Invalid UTF-8 byte sequence; UTF-8 BOM; UTF-16 BOM input | Invalid bytes rejected with byte offset; BOM handling declared per import rule | `import.invalid_utf8` or `import.unsupported_encoding` | P2-003, P2-008 |
| TYP-011 | Date-only values, leap day, pre-1970 date, timestamp with explicit offset | Date-only and timestamp remain different types; instant and offset are exact | Exact; `data.date_ambiguous` for ambiguous input | P2-002, P2-003 |
| TYP-012 | Timestamp at a DST gap and a DST overlap in a named zone | Nonexistent local time rejected or resolved only by a declared policy; ambiguous local time requires an explicit choice | `data.date_nonexistent` or `data.date_ambiguous` with zone and local time | P2-002, P2-003 |
| TYP-013 | A stored value displayed with 0, 2 and 10 decimals; display sort then clear | Stored value and calculations unchanged; sorting by display is a view operation; no write-back | Stored values bit-exact; display strings exact for declared format | P2-007 |
| TYP-014 | Low-cardinality text and declared categorical data with an unseen category | Category codes stable; unseen category follows the declared policy and is not silently mapped to the first category | Exact code and diagnostic if rejected | P2-002, P2-006 |
| TYP-015 | Text-to-number mapping, numeric expression text and original token retention | Mapping is explicit; original lexical value remains available in provenance; a failed mapping does not destroy the source | Exact value and retained token; `data.number_ambiguous` when declared | P2-003 |
| TYP-016 | Empty table, header-only table, all-missing column and all-NaN column | Schema and row count are exact; operations do not invent values | Exact | P2-002, P2-003, P2-007 |
| TYP-017 | Very large and very small magnitudes and operations that overflow to infinity | IEEE behavior for arithmetic; no silent clipping; an overflow warning is declared if the implementation emits one | Bit-exact or declared `data.overflow` | P2-002, P2-005 |
| TYP-018 | Rename a column, reorder columns, change display format, reopen | Column ID, values, unit and role unchanged; label is presentation only | Exact IDs and values | P2-002, P2-004, P2-006 |

### 4.3 Typed-value diagnostics

`data.number_overflow`, `data.number_ambiguous`, `data.invalid_text`, `data.date_ambiguous`, `data.date_nonexistent`, `data.type_mismatch`, `data.unknown_unit`, `data.incompatible_unit`, `data.precision_loss` (warning only when a lossy conversion is explicitly chosen). A conversion that would lose source precision is rejected by default.

## 5. ID: identity, immutability, versions and provenance

Covers ORG-DATA-001 and the identity side of EG-ADD-002. Consumer: P2-002 and every later data task.

### 5.1 Required invariants

1. Project, Workbook, Worksheet, ColumnBinding, DatasetVersion, Chunk, row and operation identities are stable opaque values. Array position, row number, display label and translated name are not identities.
2. A new edit publishes a new immutable dataset version. Prior versions remain readable with unchanged values, metadata and identity; no in-place mutation path exists outside the application executor.
3. A version identifies its input versions, operation/algorithm identity, canonical parameters, unit policy and content hashes sufficient to reproduce the lineage. Undo and branch retain prior versions.
4. Row identity survives sorting, filtering, display reorder, column insertion and version creation. Aggregation creates output row IDs plus group membership; joins define composite lineage; resampling records input intervals.
5. Deleting a referenced object either applies a declared dependent policy in the same transaction or is rejected with dangling-reference diagnostics. Undo restores the previous reference state.
6. A stale revision check is exact: an edit built on revision N cannot overwrite revision N+1. Rejected candidates publish no revision, event or undo entry.
7. Leases and read views cannot outlive their owner. No API returns a raw pointer or span that outlives the immutable version lease.

### 5.2 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| ID-001 | Create project, workbook, worksheet, binding, dataset and chunks; save and reopen | Same opaque IDs and parent relationships after reopen | Exact | P2-002, P2-006 |
| ID-002 | Rename and reorder worksheets, columns and labels | IDs unchanged; references still resolve | Exact | P2-002, P2-004 |
| ID-003 | Edit one chunk and publish a new version while holding the old version | Old version values and metadata are unchanged; new version references the new chunk | Exact; prior version readable | P2-002 |
| ID-004 | Sort descending, filter, clear filter and reopen | Row IDs follow rows; original order and values restore; no duplicate or lost IDs | Exact IDs and order after clear | P2-004, P2-010 |
| ID-005 | Insert, delete, move and undo rows and columns | New rows get new IDs; undo restores the exact prior IDs and bindings; deletion does not renumber survivors | Exact | P2-004 |
| ID-006 | Attempt to reuse a live ID or assign a duplicate column ID | Rejected before commit | `data.duplicate_id` with both locations | P2-002, P2-006 |
| ID-007 | Delete a referenced column or dataset with dependents | Rejected or applied with an explicit policy; no dangling ID remains | `data.dangling_reference` naming dependents | P2-002, P2-004, P2-006 |
| ID-008 | Submit a cell edit with a stale expected revision | Rejected; values, undo depth, event sequence and last-action metadata unchanged | `data.stale_revision` | P2-004 |
| ID-009 | Build a formula or analysis result from two input versions | Provenance records both input hashes, parameters and the output version; recomputation is traceable | Exact IDs and hashes | P2-002, P2-005, P2-006 |
| ID-010 | Reorder columns and change roles or formats; save and reopen | Binding identity, role, unit and format remain attached to the same column ID | Exact | P2-002, P2-006 |
| ID-011 | Hold a read view, publish a new version, then read the old view | Old view returns the old snapshot and remains valid until released | Exact; no use-after-free under ASan | P2-002 |
| ID-012 | Try to mutate a published version through every public surface | No public mutation path exists; only a new version is created | Compile-time and runtime boundary checks | P2-002 |
| ID-013 | Row at position 1 after sort, filter and insertion | Position is not identity; row ID remains the source ID | Exact | P2-004, P2-007, P2-010 |

## 6. UNIT: units, dimensions and conversion

Covers EG-ADD-008 and the unit side of ORG-DATA-001. Consumer: P2-002, P2-005 and P2-006.

### 6.1 Required invariants

1. A unit carries a dimension vector, a scale factor, an optional offset and source text. Conversion is explicit and recorded in provenance.
2. Addition and subtraction require the same dimension. Multiplication and division combine dimensions. A result carries its dimension; a unit is never silently dropped.
3. Offset units are converted with offset semantics, not ordinary multiplicative algebra. Absolute temperature and temperature difference are distinct concepts; arithmetic on absolute offset units is restricted unless converted to a declared difference unit.
4. Unknown or malformed units are rejected with source location. No automatic unit invention or fallback to dimensionless.
5. Conversion never changes the stored source values in place; it produces a new version or an explicit display/analysis conversion.
6. Unit metadata survives edit, mirror, formulas and reopen.

### 6.2 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| UNIT-001 | Same unit, same value | Identity conversion; no rounding or version change unless requested | Exact | P2-002 |
| UNIT-002 | Length conversion among m, cm and mm | Exact scale conversion; result unit recorded | Exact when representable | P2-002, P2-005 |
| UNIT-003 | Time conversion among s, ms, min and h | Exact scale conversion; duration dimension preserved | Exact when representable | P2-002, P2-005 |
| UNIT-004 | Absolute temperature K, degC and degF | Correct affine conversion; 0 degC = 273.15 K; 32 degF = 0 degC | At most 1e-12 K absolute error | P2-002, P2-005 |
| UNIT-005 | Add two absolute temperatures and multiply an absolute temperature | Rejected or requires an explicit difference-unit policy; never silently treated as scalar multiplication | `data.offset_unit_arithmetic` | P2-002, P2-005 |
| UNIT-006 | Compound units such as m/s and kg/m3 | Dimension vector combines and cancels correctly; conversion factors compound | Exact or 1e-12 relative | P2-002, P2-005 |
| UNIT-007 | Add meters and seconds; convert a length to a time | Rejected before commit | `data.incompatible_unit` with both units | P2-002, P2-005 |
| UNIT-008 | Multiply and divide quantities with units | Result dimensions are correct; unit string canonicalization preserves semantics | Exact dimension; canonical text expected | P2-005 |
| UNIT-009 | Formula result from unit-bearing columns | Result carries the derived unit; dependent unit checks run before commit | Exact dimension and declared tolerance | P2-005 |
| UNIT-010 | Import with a unit mapping, edit, mirror and save/reopen | Unit is retained at every boundary; no silent unit drop | Exact unit identity | P2-003, P2-006, P2-010 |
| UNIT-011 | Axis or display conversion versus stored values | Display conversion does not modify stored data or exported raw values | Exact stored values; display text exact | P2-007 |
| UNIT-012 | Unknown unit token, malformed compound unit, empty unit | Rejected with location; empty unit is explicit dimensionless only when declared | `data.unknown_unit` | P2-002, P2-003 |
| UNIT-013 | Angle in radians and degrees; dimensionless ratio | Conversion correct; dimensionless is not confused with missing unit | 1e-12 relative | P2-002, P2-005 |

## 7. ALIGN: row alignment, masks, mirrors and joins

Covers ORG-DATA-001, the alignment contract in the [data model](../architecture/data-model.md) section 1, and the shared selection semantics consumed by P2-010. Consumer: P2-002, P2-004 and P2-010.

### 7.1 Required invariants

1. A worksheet has one row-identity domain and an ordered row set. Every binding either shares that domain or carries an explicit mapping from worksheet row ID to source row ID or unmatched.
2. Unrelated columns are never zipped by current array position. Adding a foreign-domain column requires a declared keyed join or a user-approved positional mapping captured as stable ID pairs.
3. Duplicate join keys require an explicit expansion or aggregation policy. Unmatched rows expose a missing correspondence without mutating source values.
4. Sorting and filtering change the view, not correspondence IDs. Clearing a filter restores the original order and values.
5. A mask is a scientific exclusion selection with reason and author, not a missing value and not a view filter. Analysis and plot consumers of the same selection receive identical row IDs.
6. A mirror shares immutable storage; it does not duplicate the data by default. A write through a mirror requires an explicit shared-edit or detach-to-copy command. Mirror graphs are acyclic; a mirror of a mirror resolves to one shared source, and A mirrors B mirrors A is rejected with a stable diagnostic.
7. Same-domain mirrors follow source insertions and deletions only as one validated rectangular revision. Foreign-domain mappings keep existing pairs stable; new source rows are unmatched until an explicit refresh.
8. Deleting a source with dependents reports the dangling dependents and does not silently drop them.
9. Alignment changes are transactional: a rejected mapping leaves the previous revision intact.

### 7.2 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| ALIGN-001 | Same-domain source insertion and deletion | Worksheet rows and all same-domain bindings update in one revision; IDs remain consistent | Exact | P2-002, P2-010 |
| ALIGN-002 | Foreign-domain keyed join with unique keys | Mapped rows pair by ID; source values unchanged; join lineage recorded | Exact | P2-002, P2-010 |
| ALIGN-003 | Foreign-domain positional mapping without explicit approval | Rejected; a declared mapping with stable ID pairs is accepted | `data.positional_mapping_required` | P2-002, P2-010 |
| ALIGN-004 | Join with unmatched rows on either side | Unmatched rows carry an explicit unmapped state; no null value is invented in the source | Exact state and diagnostics | P2-010 |
| ALIGN-005 | Duplicate join keys with error, first, all-with-expansion and aggregate policies | Each policy has a declared result; no implicit row multiplication | Exact row count and lineage; `data.duplicate_join_key` where rejected | P2-010 |
| ALIGN-006 | Bind two columns of unequal length without a mapping | Rejected; never zipped or padded silently | `data.unmapped_row` naming both lengths | P2-002, P2-010 |
| ALIGN-007 | Mirror a column into another worksheet | Shared immutable storage is observable; no second copy; mirror is read-only by default | Exact storage identity and values | P2-010 |
| ALIGN-008 | Edit the source of a mirror | New source version propagates through existing bindings; old versions remain readable | Exact | P2-010 |
| ALIGN-009 | Delete a mirrored source | Dependents reported; no dangling binding; undo restores | `data.dangling_reference` or declared policy | P2-010 |
| ALIGN-010 | Sort source and target differently, then edit a source value | Correspondence IDs, not positions, decide propagation | Exact row IDs and values | P2-010 |
| ALIGN-011 | Mask condition selects a subset; query selected IDs | Same stable IDs for worksheet, plot and analysis consumers; mask vs filter vs missing distinct | Exact ID set | P2-010 |
| ALIGN-012 | Clear a filter after sorting and masking | Original row order and values restore; mask and missing states unchanged | Exact | P2-010 |
| ALIGN-013 | Propagate a formula column across sheets with relative references | References adjust according to the declared structural policy; undo restores all affected sheets | Exact formula text and results | P2-010 |
| ALIGN-014 | Refresh alignment after new source rows | Recorded join policy applies; existing pairs stay stable; new unmatched rows reported | Exact | P2-010 |
| ALIGN-015 | Reject an alignment change mid-transaction | Previous revision, bindings and mask state unchanged; no partial event | Exact | P2-010 |
| ALIGN-016 | Same-domain mirror followed by source row insertion, deletion and re-sort | The mirror follows source rows as one validated rectangular revision; bound values and row IDs are consistent; undo restores the prior state | Exact | P2-010 |
| ALIGN-017 | Foreign-domain mirror after source insertion, deletion and independent sorting | Existing ID pairs stay stable; new source rows are unmatched; deleted source rows become unmatched with values retained; no positional re-pairing | Exact; `data.unmapped_row` for new or deleted rows as declared | P2-010 |
| ALIGN-018 | Mirror cycle A mirrors B, B mirrors A, including an indirect cycle through a third sheet | Rejected before commit; no partial binding or storage alias | `data.duplicate_id` or a dedicated `data.mirror_cycle` detail code | P2-010 |

## 8. IO: delimited text and clipboard import

Covers ORG-DATA-011, DG-DATA-005, DG-DATA-006 and the IO shared gate for CSV, TXT and clipboard text. Consumer: P2-003, with Excel diagnostics in P2-008.

### 8.1 Parsing rules

1. Delimiter detection considers comma, tab, semicolon, pipe and space, plus an explicit override. Detection is deterministic and reports its evidence. Ambiguous input produces a diagnostic listing candidates; it is not guessed silently.
2. Quoting follows a declared RFC 4180-style contract: doubled quotes inside a quoted field, delimiters and line breaks inside quotes, CRLF and LF preserved as source characters, and an optional trim that never changes quoted content unless explicitly requested.
3. Header mode is auto-detected only by declared rules, declared present, or declared absent. Duplicate or empty header names receive distinct stable column IDs and a documented display-label policy; they are not silently renamed to hide the conflict.
4. Skip rows, row intervals, regions, subheaders, label rows, masks and end tags are explicit rules recorded in an Import Special object. Source line numbers remain traceable.
5. Encoding is declared or detected with evidence. UTF-8 is the required baseline for text columns and committed text fixtures. UTF-16 with BOM and a declared legacy code page are optional capabilities recorded by the implementation; the supported input set is an open decision in section 14. Invalid byte sequences are rejected with byte offset; replacement characters are not silently substituted.
6. Numeric inference is explicit. Integer and floating-point tokens are distinguished; leading zeros, plus signs, exponent forms and overflow have declared outcomes. Thousands separators and decimal commas are never guessed when ambiguous.
7. Date and time parsing uses declared formats and a declared zone policy. Day-first and month-first ambiguity is diagnosed; OS locale is not an implicit format source.
8. A malformed record rejects the import atomically. Atomic rejection is the only authorized P2 mode; a partial or lossy mode requires a later allocated task and is not part of this plan. A partial mode must never be the silent default.
9. Text that looks like a formula or command, including a leading `=`, `+`, `-` or `@`, remains text on import. It is never evaluated or executed.
10. Limits are explicit and enforced with stable diagnostics: maximum field bytes, record bytes, record count, column count, decoded bytes and nesting or quoting depth. Limit failures do not leave a partial worksheet.

### 8.2 Clipboard and Import Special

Clipboard import uses the same parser and typed-value rules, with explicit delimiter, decimal separator, transpose and trim options. A failed clipboard parse leaves the destination worksheet unchanged. Import Special rules are versioned, saved and replayed deterministically; unknown rule fields are rejected rather than ignored.

### 8.3 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| IO-001 | Simple comma CSV with header and LF | Exact columns, types and rows; source lines recorded | Exact; parsed numbers bit-exact | P2-003 |
| IO-002 | Tab, semicolon, pipe and space delimited files | Detected delimiter is reported; values exact | Exact; no ambiguity | P2-003 |
| IO-003 | Explicit delimiter override on an ambiguous file | Override wins; no guessing warning | Exact | P2-003 |
| IO-004 | Quoted fields containing each delimiter | Field boundaries correct; delimiter remains in text | Exact text | P2-003 |
| IO-005 | Doubled quotes and an unclosed quote | Doubled quote becomes one quote; unclosed quote rejected with line | `import.unclosed_quote` | P2-003 |
| IO-006 | CRLF, LF and CR line endings, including inside quoted fields | Record splitting uses the declared rule; quoted line breaks preserved | Exact record count and text | P2-003 |
| IO-007 | Empty file, header-only file, trailing newline and blank lines | Empty table or header-only table represented explicitly; blank-line policy declared | Exact row and column count | P2-003 |
| IO-008 | UTF-8 with and without BOM | BOM does not become part of the first header or value; byte offset recorded | Exact | P2-003 |
| IO-009 | UTF-16LE and UTF-16BE with BOM | Decoded text exact when supported; otherwise stable unsupported-encoding diagnostic | `import.unsupported_encoding` when unsupported | P2-003 |
| IO-010 | Invalid UTF-8 sequences | Rejected; no replacement characters; byte offset reported | `import.invalid_utf8` | P2-003 |
| IO-011 | Declared legacy code page and an undeclared high-byte file | Declared decoding exact; undeclared high bytes diagnosed, not guessed | Exact or `import.unsupported_encoding` | P2-003 |
| IO-012 | Header auto, present and absent modes | Column IDs stable; absent header generates documented labels without inventing identities | Exact | P2-003 |
| IO-013 | Duplicate and empty header names | Distinct stable column IDs; display labels documented; no silent loss | `import.header_conflict` warning or error as declared | P2-003 |
| IO-014 | Skip rows, row interval, start/end region and end tag | Only the declared region imports; source line provenance is exact | Exact | P2-003 |
| IO-015 | Semicolon-delimited decimal-comma file | `1,5` becomes 1.5; quoted values unaffected | Bit-exact | P2-003 |
| IO-016 | Comma-delimited file containing `1,234` and conflicting decimal choices | Ambiguity diagnosed unless an explicit decimal/thousands rule is supplied | `import.decimal_ambiguous` | P2-003 |
| IO-017 | Row with too few or too many fields | Import rejected atomically; expected and actual counts reported | `import.field_count_mismatch` | P2-003 |
| IO-018 | Quote character in an unquoted field | Declared policy applies; malformed quoting is diagnosed, not silently repaired | `import.malformed_quote` | P2-003 |
| IO-019 | NUL bytes, binary content and invalid control characters | Rejected or explicitly escaped by a declared rule; no binary reinterpretation | `import.unsupported_construct` | P2-003 |
| IO-020 | Field, record, column and decoded-size limits | Rejected with the limit name and observed size; no partial output | `import.limit_exceeded` | P2-003 |
| IO-021 | Leading zeros, plus signs, exponent forms and integer overflow | Type and value follow declared inference; overflow rejected | Exact or `data.number_overflow` | P2-003 |
| IO-022 | Declared date formats, date-only and timestamp values | Types and instants exact; undeclared formats diagnosed | `data.date_format` or `data.date_ambiguous` | P2-003 |
| IO-023 | Formula-like text `=1+1`, `+SUM(A1)`, `@cmd` | Stored as text; no evaluation or execution | Exact text | P2-003, P2-008 |
| IO-024 | Malformed row in the middle of a valid file | Atomic rejection; no worksheet is published | `import.field_count_mismatch` with line and record | P2-003 |
| IO-025 | Successful import followed by a failed re-import | Failed attempt leaves the prior worksheet version intact | Exact; `data.stale_revision` if applicable | P2-003 |
| IO-026 | Import Special with header, skip, label, mask, subheader and end-tag rules | Same rules and result on replay; rule version recorded | Exact | P2-003 |
| IO-027 | Clipboard tabular text with transpose and trim | Parsed table matches declared options; destination atomic | Exact | P2-003 |
| IO-028 | Clipboard parse failure | Destination unchanged; diagnostic identifies the failure | Exact | P2-003 |
| IO-029 | Unicode headers, combining sequences and a right-to-left label | Text preserved; column IDs stable | Exact code points | P2-003 |
| IO-030 | Mixed line endings and a final record without newline | Record boundaries follow the declared rule; no phantom row | Exact | P2-003 |

### 8.4 Import diagnostics

`import.delimiter_ambiguous`, `import.unclosed_quote`, `import.malformed_quote`, `import.field_count_mismatch`, `import.invalid_utf8`, `import.unsupported_encoding`, `import.unsupported_construct`, `import.header_conflict`, `import.decimal_ambiguous`, `import.limit_exceeded`, plus the `data.` diagnostics for typed values. Every diagnostic carries a file and byte or line location and a record number when one exists.

## 9. FML: formulas, DAG and recalculation

Covers the formula-DAG acceptance in P2-001 and the formula contracts for P2-005. Related requirements include ORG-DATA-004, ORG-MATH-001, ORG-RECALC-002, DG-DATA-007, DG-DATA-009, DG-FIT-008 and DG-FIT-011; this plan defines only the P2 subset and does not claim those requirement rows complete.

### 9.1 Required semantics

1. The expression grammar is versioned. It supports declared literals, column references, cell references, row numbers, registered functions and the declared operator set. Unknown syntax is rejected before evaluation with a stable diagnostic; there is no host language fallback.
2. Precedence and associativity are fixed: exponentiation is right-associative and binds tighter than unary minus; multiplication and division bind tighter than addition and subtraction; comparison and text concatenation follow the declared table. Every precedence fixture has a deterministic expected value.
3. References are typed. A numeric operation on text is an error unless an explicit conversion function is used. Missing, NaN and an error are distinct: missing propagates through arithmetic as missing by default, NaN remains a numeric value, and an error propagates as an error and is never coerced to missing.
4. Division by zero and a domain error such as the square root of a negative number produce stable formula errors. IEEE overflow may produce infinity where the function contract declares it. NaN equality follows IEEE comparison, while structural change detection treats all NaNs as equal.
5. Cell and column formulas record their parsed expression, typed references and dependency edges. Renaming preserves reference identity; structural row or column edits follow a documented relative/absolute reference policy.
6. The dependency graph across cells, columns, sheets, variables and analysis-output tokens is acyclic. A cycle is rejected before commit; no partial result or cache entry is published.
7. Invalidation follows reverse edges. Only reachable dependents recompute. The cache key includes node identity and version, input revisions or content hashes, canonical parameters, selection or mask, unit policy, function-library version and random seed where deterministic seeding is declared.
8. Auto and manual recalculation are explicit modes. A reopened project in manual mode does not silently recompute without the declared trigger. A manual-trigger result records that it was manual.
9. The function library is versioned and registered. Each function declares name, arity, signatures, missing policy, error policy, determinism and numerical tolerance. Unknown functions are rejected. Functions cannot read files, network, environment, clock or process state unless a later architecture contract explicitly permits it.
10. Formula text imported from text, Excel or clipboard is data until an explicit conversion command validates it. No imported string triggers evaluation, external access or code execution.
11. Formula results are deterministic for the declared inputs, library version and build. Parallel order changes must be covered by the declared tolerance or by a deterministic mode.

### 9.2 Fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| FML-001 | Expressions combining `+`, `-`, `*`, `/`, `^`, parentheses and comparison | Precedence and associativity match the declared table | 1 ULP per basic operation | P2-005 |
| FML-002 | Unary minus with exponentiation and chained exponentiation | Right associativity and binding are exact | Exact parse tree and 1 ULP result | P2-005 |
| FML-003 | Same-sheet and cross-sheet cell references | Correct value and reference identity; missing reference reported | Exact; `formula.invalid_reference` | P2-005 |
| FML-004 | Whole-column and row-number references over typed columns | Correct values and dependency set; no accidental full-table materialization beyond the declared budget | Exact | P2-005 |
| FML-005 | Insert, delete and move rows or columns under relative and absolute references | Reference adjustment follows the documented policy; undo restores the prior expressions | Exact expression text and values | P2-005, P2-004 |
| FML-006 | Numeric operation on text, date or unit-incompatible values | Rejected with typed error; no implicit parse | `formula.type_mismatch` or `data.incompatible_unit` | P2-005 |
| FML-007 | `1/0`, `0/0`, square root of a negative value and overflow | Division and domain errors are distinct; overflow follows the declared IEEE policy; no silent missing | `formula.division_by_zero`, `formula.domain_error` | P2-005 |
| FML-008 | Missing, NaN and empty text inputs to arithmetic and aggregate functions | Missing propagates by policy; NaN is a value; empty text is not missing | Exact state and result | P2-005 |
| FML-009 | Error input feeding further formulas | Error propagates with its original code and location; not converted to missing or zero | Exact error code | P2-005 |
| FML-010 | Unknown function and ambiguous name | Rejected before evaluation; suggestions are not auto-applied | `formula.unknown_name` or `formula.ambiguous_name` | P2-005 |
| FML-011 | Direct and indirect cell cycle in one sheet | Rejected before commit; no partial cache | `formula.circular_reference` with the cycle path | P2-005 |
| FML-012 | Cycle across sheets, a column formula and an analysis-output token | Same cycle rejection across the combined graph | `formula.circular_reference` with all nodes | P2-005 |
| FML-013 | Chain A to B to C; edit A | B and C are marked stale and recompute once on the declared trigger; results match the reference | Exact recompute count; 1 ULP values | P2-005 |
| FML-014 | Unrelated branch D while A changes | D is not invalidated and does not recompute | Exact recompute count | P2-005 |
| FML-015 | Cache entry reused with unchanged inputs and rejected after an input revision change | Cache key includes input revision; stale result is never served as current | Exact | P2-005 |
| FML-016 | Manual mode after reopen, then explicit recalculation | No automatic recompute on open; explicit trigger recomputes and records manual mode | Exact | P2-005 |
| FML-017 | Auto mode after an upstream edit | Dependents recompute according to the declared debounce and commit policy; final result is complete | Exact final value and revision | P2-005 |
| FML-018 | Change the function-library version or a function's numerical contract | Cached results invalidate when numerics can change; old results remain identifiable | Exact invalidation | P2-005 |
| FML-019 | Unsupported syntax, host function call, file or network access attempt | Rejected at parse or name resolution; no host access occurs | `formula.unsupported_syntax` | P2-005 |
| FML-020 | Unicode text expressions, labels and declared text functions | Code points preserved; comparison and concatenation follow the declared semantics | Exact text | P2-005 |
| FML-021 | Numeric function cases with known analytic results | Results satisfy registry tolerances; deterministic seeds recorded | Registry tolerance, default 1e-13 relative | P2-005 |
| FML-022 | Formula commit containing a cycle or an invalid reference in one node | Whole transaction rejected; prior revision and cache unchanged; no event | Exact | P2-005 |
| FML-023 | Rename a referenced column or sheet | Reference remains valid through stable IDs; displayed formula text updates only by declared policy | Exact | P2-005 |
| FML-024 | Delete a referenced column and undo | Delete is rejected or applies a declared policy; undo restores formulas and values exactly | `data.dangling_reference` or exact restore | P2-005 |

### 9.3 Formula diagnostics

`formula.unknown_name`, `formula.ambiguous_name`, `formula.type_mismatch`, `formula.division_by_zero`, `formula.domain_error`, `formula.invalid_reference`, `formula.circular_reference`, `formula.unsupported_syntax`, `formula.manual_recalc_required`. A cycle diagnostic names every node in the cycle. A type-mismatch diagnostic names the operand and expected type.

## 10. SAVE, REC and MIG: persistence, recovery and migration

Covers EG-ADD-010 and the P2 migration hooks for EG-ADD-013. EG-ADD-013 also includes protection and package handling at P6; this plan and P2-006 do not complete that row. Consumer: P2-006; P2-011 measures the same paths.

### 10.1 Required save and open semantics

1. The native container stores a declared format version, required feature declarations, project ID and saved revision, plus object, reference, dataset, chunk, operation and asset metadata as defined by the [project file format](../architecture/project-file-format.md).
2. An explicit save commits a chosen immutable snapshot in a short transaction. Saving revision N while the user edits N+1 marks only N saved and leaves the document dirty.
3. A successful saved indication follows the database commit and required flush, not a task submission. A failed save leaves the prior file intact.
4. Save As pins the requested application revision, including unsaved metadata and required working chunks. For an existing destination it backs up that database consistently into a temporary destination on the target volume; for a never-saved project it initializes an empty container. It then applies the pinned revision metadata, references and required chunks to the destination, commits, validates, closes and atomically replaces the authorized destination. It does not save or modify the original source as an intermediate step. A plain filesystem copy of a live database is not the Save As algorithm. Concurrent edits remain dirty relative to the new saved revision. Overwrite confirmation, low disk space, target disappearance and platform file-sharing locks are handled explicitly.
5. Reopen validates magic, format version, required features, object and reference integrity, chunk lengths, decoded-size limits and integer bounds before materializing data. Required chunk checksums are validated on access, not by an unconditional full scan; metadata and any chunks needed for the initial document view are validated according to the declared open policy. A corrupt optional thumbnail may be regenerated; a corrupt required chunk is reported when accessed and is not silently fabricated. A fixture confirms that unaccessed data is not required to be scanned at open and that corruption is still detected on first access.
6. Unknown required metadata or a feature that cannot be preserved makes the project unopenable or explicitly read-only; it is never dropped and re-saved as if preserved. Unknown optional payloads are preserved byte-for-byte within quotas and remain inert.
7. The canonical file uses rollback-journal mode with a tested synchronous/durability policy, not WAL. The durability claim is limited to the tested filesystem and storage stack.
8. A second live writer or a conflicting lock is rejected with a stable diagnostic. Network or cloud-synchronized directories are not live multi-writer stores.
9. Saving during analysis or editing uses a stable snapshot; later edits remain dirty. No save reads a changing buffer without a lease.
10. No embedded content is executed during open, save, recovery or migration. Container validation never follows an embedded path or launches a file.

### 10.2 Failure-injection points

The storage tests use a fault-injection port around the file and database operations, plus a child-process kill harness for process death. At minimum, inject failure at:

| Point | Injected condition | Required outcome |
|---|---|---|
| S0 | Failure before the first write | Prior document and prior temp state unchanged |
| S1 | After temporary container creation | No destination replacement; temporary artifact reported and cleaned or preserved as diagnostics |
| S2 | After schema and metadata write | Destination invalid; prior file intact; no "saved" status |
| S3 | After one or more chunk writes | Destination invalid; prior file intact; no partial object visible |
| S4 | Before transaction commit | Transaction rolls back; prior file intact; no revision published |
| S5 | After commit but before flush acknowledgement | Reopen yields either the complete committed revision or the intact prior file; no partial object |
| S6 | After flush but before close | Same as S5; durability status reported truthfully |
| S7 | After close but before destination replace | Prior destination intact; temporary validated container remains recoverable or is reported |
| S8 | After replace but before directory flush | Reopen of the destination yields the complete committed revision or the prior file; the reported status matches the observed state |
| S9 | During reopen validation | Failure is reported with the failing check; no materialization of unvalidated data |
| S10 | Save As after backing up an existing destination, before applying the pinned revision | Prior destination intact; source original untouched; backup/target artifact reported |
| S11 | Save As after applying the pinned revision, before validation | Source original untouched; destination remains the prior valid database or is reported unusable; no partial replacement |
| S12 | Save As after validation, before atomic replace | Source original untouched; prior destination intact; validated temporary container reported or recoverable |

S10 to S12 apply when Save As replaces an existing destination. A never-saved project initializes an empty container and has no backup stage. The exact platform sequence may differ, but every point must have a defined expected state and a test. Simulated errors and process kills are separate cases. A test that only asserts "no crash" is insufficient.

### 10.3 Recovery and the unsaved window

1. The recovery checkpoint is separate from the saved document and uses the same transactional and checksum rules. Its cadence is configurable and declared by the implementation.
2. On restart, recovery is offered into a new file. The original file is never overwritten automatically. The user can inspect and preserve failed files.
3. A completed in-memory edit is not durable until a checkpoint or save acknowledgement. The UI and the command result distinguish operation completion from durable checkpoint status.
4. Recovery reports lost or corrupt chunk IDs and can recover independent intact objects where safe. It does not claim that a missing required chunk is recovered.
5. The loss window is measured or explicitly bounded by the configured checkpoint trigger. Tests use a deterministic trigger or injected clock; they do not assume a default cadence.
6. Recovery and save do not execute embedded content or follow external links.

### 10.4 Migration

1. Migration is a versioned pure transformation of a copied snapshot. The original is preserved until the transformed copy passes validation.
2. A fixture exists for every released schema version and for each supported migration chain. The plan starts with the P2 test schemas created by P2-006; later releases add public versions.
3. Interrupted migration leaves the original intact and reports whether the copy is usable. A retry starts from the original or a validated intermediate, not a half-written target.
4. A newer unsupported major version opens read-only only if essential semantics can be preserved; otherwise it fails clearly. A downgrade is rejected with a stable diagnostic.
5. Unknown optional extension payloads are preserved. Unknown executable payloads remain inert and are never loaded.
6. Missing external sources and missing analysis plugins leave original data and metadata visible with a "cannot recompute" diagnostic; no substitute algorithm is selected.
7. Credentials are not stored in the project. External-source locators, hashes and refresh policies may be stored without secrets.

### 10.5 Persistence fixtures

| Fixture | Input and purpose | Expected semantics | Tolerance / diagnostic | Consumers |
|---|---|---|---|---|
| SAVE-001 | Small project with workbook, worksheet, columns and metadata | Save, close and reopen reproduce exact objects and values | Exact | P2-006 |
| SAVE-002 | Round-trip of every TYP fixture value | Types, bits, missing states, NaN, infinities and signed zero survive | Bit-exact | P2-006 |
| SAVE-003 | Round-trip of units, IDs, row mappings, masks, mirrors and formula references | All metadata and references survive; no renumbering | Exact | P2-006 |
| SAVE-004 | Save revision N while editing N+1 | Saved revision is N; document remains dirty relative to N+1 | Exact status and revision | P2-006 |
| SAVE-005 | Failed flush after commit | No "saved" acknowledgement; observed reopen state matches S5/S6 expectation | Exact status | P2-006 |
| SAVE-006 | Save As from a project with unsaved edits, including concurrent edits after the pin | Pinned revision includes unsaved data at the pin point; later edits remain dirty; destination matches the pinned revision | Exact | P2-006 |
| SAVE-007 | Save As over an existing file | Consistent backup of the existing destination, apply pinned revision, validate, close and atomic replace; confirmation required; original source document unchanged | Exact | P2-006 |
| SAVE-015 | Save As of a never-saved project | Empty container initialized at the destination; no source database is copied or modified | Exact | P2-006 |
| SAVE-016 | Save As with low disk space, target-volume change, missing destination or a locked destination | Failure is truthful; source original unchanged; prior destination intact or explicitly reported; no plain live-file copy | `storage.resource_limit`, `storage.lock_conflict` or `storage.save_failed` | P2-006 |
| SAVE-017 | Fault injection at Save As points S10 to S12 | Each point matches section 10.2; no partial destination; source original untouched | Exact | P2-006 |
| SAVE-008 | Fault injection at S0 to S9 | Each point matches section 10.2; no partial object | Exact state and diagnostic | P2-006 |
| SAVE-009 | Process kill during save at selected points | Reopen yields the intact prior file or the complete committed destination; no mixed state | Exact; recovery report | P2-006 |
| SAVE-010 | Disk-full or quota failure during temp write or commit | Save fails with `storage.resource_limit` or `storage.save_failed`; prior file intact | Exact | P2-006 |
| SAVE-011 | Two writers or an external lock on the container | Second writer rejected; no silent overwrite | `storage.lock_conflict` | P2-006 |
| SAVE-012 | Destination disappears or is replaced between validation and rename | Save fails truthfully; no clobber of an unrelated file | `storage.save_failed` | P2-006 |
| SAVE-013 | Large column stored as bounded chunks | No single giant BLOB; chunk count and sizes reported; decoded size within declared limits | Exact limits; no full-table BLOB | P2-006, P2-011 |
| SAVE-014 | Save during an active simulated long-running candidate or edit | Save uses a stable snapshot; later edits stay dirty; no torn chunk. The P2 test uses a stand-in task; P4 analysis behavior is not claimed | Exact revisions | P2-006 |
| REC-001 | Edit after the last checkpoint, then terminate | Recovery offers the checkpoint state and reports the unsaved edit as lost; no false durability claim | Exact | P2-006 |
| REC-002 | Recovery from a checkpoint into a new file | New file is valid; original is untouched; IDs and values exact | Exact | P2-006 |
| REC-003 | Recovery when the original is corrupt | Original preserved; recovery uses the intact checkpoint; corrupt required chunks reported | Exact diagnostics | P2-006 |
| REC-004 | Kill before command commit | No revision or event is published; no recovery object for the uncommitted candidate | Exact | P2-006 |
| REC-005 | Kill after commit before save | Recovery checkpoint contains the committed revision if checkpointed; otherwise the save state is reported | Exact declared cadence | P2-006 |
| REC-006 | Corrupt optional thumbnail | Thumbnail is regenerated or omitted; required data unaffected | Exact | P2-006 |
| REC-007 | Corrupt required chunk | Open fails or reports the missing chunk; no fabricated values | `storage.checksum_mismatch` or `storage.corrupt_container` | P2-006 |
| REC-008 | Truncated container, bad page and flipped metadata byte | Detected before materialization; stable diagnostic with location | `storage.corrupt_container` | P2-006 |
| REC-009 | Chunk checksum mismatch and a replaced chunk with a consistent self-checksum | Mismatch detected; the second case is documented as accidental-corruption detection, not cryptographic authenticity | Exact limitation statement | P2-006 |
| REC-010 | Malformed schema, invalid reference and duplicate ID | Validation fails before open; no partial materialization | `storage.reference_integrity` or `storage.corrupt_container` | P2-006 |
| REC-011 | Oversized decoded chunk, negative length and integer overflow | Rejected within declared limits; no allocation based on an unvalidated length | `storage.resource_limit` | P2-006 |
| REC-012 | Unknown required feature or future major version | Open read-only only if essential semantics are preserved; otherwise clear failure | `storage.format_unsupported` | P2-006 |
| REC-013 | Unknown optional extension payload | Preserved byte-for-byte on reopen and save within quota; never executed | Exact bytes | P2-006 |
| REC-014 | Embedded script, external link or macro-like payload in metadata | Never executed or followed; inert or rejected per declared policy | Exact no-execution evidence | P2-006 |
| REC-015 | Package member path traversal, duplicate member and oversized member | Deferred to P6; package validation is not a P2 fixture and P2-006 must not claim it complete | Not a P2 acceptance fixture | P6 |
| REC-016 | Required chunk corrupted but not needed for the initial view; open then access the data | Open validation does not require an unconditional full scan; first access detects the checksum mismatch and reports the chunk; no fabricated values | `storage.checksum_mismatch` on access | P2-006 |
| MIG-001 | Migrate a declared prior test schema to the current schema | Pure transformation; values, IDs and references exact; original preserved | Exact | P2-006 |
| MIG-002 | Open a newer unsupported major version | Read-only only when essential semantics survive; otherwise clear failure; no downgrade write | `storage.format_unsupported` | P2-006 |
| MIG-003 | Interrupt migration at each copy/validate/replace stage | Original intact; target is either absent or validated; retry is safe | Exact; `migration.interrupted` | P2-006 |
| MIG-004 | Migration validation failure | Original preserved; failed copy reported; no destination replacement | Exact | P2-006 |
| MIG-005 | Unknown optional extension during migration | Preserved within quota; remains inert | Exact bytes | P2-006 |
| MIG-006 | Chain of two prior versions | Each step validated; final state exact; failure leaves the last valid copy | Exact | P2-006 |
| MIG-007 | Missing external source or analysis plugin after reopen | Original data and metadata visible; "cannot recompute" diagnostic; no substitute result | Exact | P2-006 |
| MIG-008 | Credential-like fields in project metadata | No secret is stored; redaction or rejection is explicit | Exact | P2-006 |

SAVE-006, SAVE-007 and SAVE-015 to SAVE-017 are Save As fixtures derived from the format contract. The current P2-006 acceptance criteria do not name Save As. P2-006 must reconcile its own record or P0-000 must allocate a follow-up before these fixtures are used as P2-006 acceptance evidence; this plan does not amend another task's scope.

### 10.6 Storage diagnostics

`storage.format_unsupported`, `storage.corrupt_container`, `storage.checksum_mismatch`, `storage.reference_integrity`, `storage.resource_limit`, `storage.lock_conflict`, `storage.save_failed`, `recovery.checkpoint_unavailable`, `migration.interrupted`, `migration.failed`, `migration.unsupported_downgrade`. A storage error names the container, table or chunk and the failed check. No diagnostic includes secrets or raw private data.

## 11. PERF: large-data import and save measurement

Covers the PERF shared gate and the acceptance preparation for P2-011. P2-011 owns actual measurements; this plan owns the procedure, fixtures and correctness gate. No number in this plan is a measured result or a budget.

### 11.1 Fixtures and generation

| Fixture | Content | Required record |
|---|---|---|
| PERF-001 | Numeric CSV of exactly 100,000,000 bytes | Actual bytes, rows, columns, schema, sha256, generator version and seed |
| PERF-002 | Mixed CSV of exactly 100,000,000 bytes with UTF-8 text, dates, nulls and quoted fields | Same as PERF-001 plus missing rate and locale |
| PERF-003 | Numeric CSV of exactly 1,000,000,000 bytes | Same as PERF-001 |
| PERF-004 | Mixed CSV of exactly 1,000,000,000 bytes | Same as PERF-002 |
| PERF-005 | Wide table with many columns, a long text field and repeated timestamps | Column count, max field bytes and row count |
| PERF-006 | Monotonic and non-monotonic x, gaps, outliers and spikes for import and data checks | Distribution descriptor and hash |
| PERF-007 | Project with 100MB imported data, a few worksheets and edits for save/reopen | Object counts, chunk counts, file size and edit count |
| PERF-008 | Project with 1GB imported data for save/reopen | Same as PERF-007 |
| PERF-009 | Cancellation fixture: import and save with an injected stop at defined checkpoints | Stop point, acknowledgement latency and final state correctness. The cancel interface must be provided by the import and save implementation or by a follow-up task allocated by P0-000; this plan does not amend P2-003 or P2-006 scope |
| PERF-010 | Correctness fixtures sampled from every generated dataset | Expected values and diagnostics; timing is invalid if correctness fails |
| PERF-011 | Hardware and build disclosure record | OS, CPU, core count, RAM, storage, compiler, CMake, Ninja, dependencies and power/thermal state |
| PERF-012 | Cold and warm page-cache cases | Cache state evidence; no false "cold" claim |

Generation is deterministic from a declared seed and generator version. Large files stay in an ignored build or fixture directory; they are not committed. A generator change creates a new fixture version and hash. The test may sample and verify rows after the timing run; the full correctness check can run once per fixture version and is referenced by hash.

### 11.2 Procedure

1. Use a release build without a debugger. Record the exact preset, compiler flags and dependency lock.
2. Record the actual generated bytes and sha256 before timing. A run whose fixture hash differs from the recorded one is invalid.
3. Separate cold-process and warm-process runs. Warm up explicitly. Run at least 10 independent trials, randomize variant order where it cannot change semantics, and report median and p95 with dispersion. Long 1GB runs may use fewer documented repetitions until automation exists.
4. Record process-tree peak memory, not only the main-process resident value. State the platform definition used.
5. Count bytes copied at parser, chunk-builder and container boundaries. Do not label the pipeline zero-copy.
6. Inject cancellation at declared chunk, iteration or I/O boundaries. Record acknowledgement latency and verify that the destination or document state is correct after cancellation.
7. Measure import phases separately where possible: parse, validate, build and commit. Record first table visibility.
8. Measure save phases: chunks written, journal peak, durable-save time, reopen time to first document and first data access, and recovery result after injected termination.
9. A failed, partial or canceled run does not contribute a timing value. It contributes a failure record and a correctness result.
10. Re-run the same fixture on a second declared machine class when one is available. Do not generalize from the development Mac.

### 11.3 Required metrics

Import: elapsed wall time, throughput in bytes per second, parse/validate/build/commit split, peak process-tree memory, bytes copied, first table visibility, cancellation acknowledgement and correctness.

Save/reopen: durable save time, bytes written, journal peak, file size, chunk count, reopen time to first document and first data, peak memory, recovery outcome, and crash-safety result. Concurrent edit during save records whether later edits remained dirty.

### 11.4 Budgets and acceptance

No absolute performance threshold or vendor number is adopted in this plan. P2-011 records the first baseline on declared hardware and build. Later review sets per-workload budgets. The initial regression trigger is a median slowdown greater than 10 percent or a peak-memory increase greater than 15 percent outside observed noise; correctness regressions fail regardless of speed. The design target of cooperative cancellation normally acknowledged within 250 ms at an available checkpoint is an observation target from the [performance protocol](performance-validation.md), not a P2 release promise.

## 12. Execution, CI and acceptance gates

1. Every test is traceable to at least one requirement ID and one fixture ID. Tests without a fixture are allowed only for pure API contracts and must state why.
2. A consuming P2 task records its implemented and deferred families. A deferred family needs an owner, a reason and a target task; it keeps the task or the relevant gate open.
3. Golden, property and failure-injection tests run in the Qt-free macOS debug, release and ASan/UBSan presets. TSan runs separately where supported. A Windows run is recorded separately; macOS evidence is never labeled Windows evidence.
4. Property tests compare a columnar implementation with a scalar reference over fixed seeds and report the seed, operation count and minimal failing input. A random failure without a reproducing seed is not closed.
5. Failure-injection tests declare the injection point and assert the visible state, not only the absence of a crash. Crash tests use a child process and verify reopen behavior.
6. No test is skipped to make a suite green. An accepted temporary limitation is recorded as a known gap with an owner, a reason, an expiry or follow-up task, and it is excluded from any coverage claim.
7. A failing sanitizer, leak, corruption or precision test blocks acceptance of the owning P2 task even if functional golden tests pass.
8. A test result records exact commands and environment. No result is copied from another platform, build or fixture hash.
9. Test code must not write outside a temporary test directory or the declared ignored output directory. It must not modify a committed fixture. It must not require network access.
10. Tests for import, storage and formulas treat all fixture content as untrusted input. No fixture may cause execution, network access or a path outside the test sandbox.

## 13. Requirement traceability

| Requirement | Families and sections | Primary consumer | Boundary of this plan |
|---|---|---|---|
| ORG-DATA-001 | TYP, ID, UNIT, ALIGN, SAVE | P2-002, P2-004, P2-006, P2-007 | Data model and worksheet semantics, not UI completeness |
| ORG-DATA-011 | TYP, IO | P2-002, P2-003 | Missing vs zero vs NaN vs empty text |
| DG-DATA-005 | TYP, IO | P2-002, P2-003 | Numeric, text, category and date parsing |
| DG-DATA-006 | TYP, SAVE | P2-002, P2-006 | IEEE bit round-trip and signed zero |
| EG-ADD-002 | ID, FML, SAVE | P2-002, P2-005, P2-006 | In-memory immutable versions and provenance DAG; plotted-result traceability is later P3/P4 consumer work and is not claimed here |
| EG-ADD-008 | UNIT, FML | P2-002, P2-005 | Unit conversion and dimensional validation |
| EG-ADD-010 | SAVE, REC | P2-006 | Atomic persistence and interrupted save |
| EG-ADD-013 | MIG, REC | P2-006 then P6 | P2 migration hooks only; protection and package remain P6 |
| IO shared gate | IO | P2-003, P2-008 | Text and clipboard now; Excel adapter later |
| PERF shared gate | PERF | P2-011 | Procedure only; no budget or vendor claim |

Supporting requirements used by the fixture families but not claimed complete here include ORG-DATA-004, ORG-MATH-001, ORG-RECALC-002, DG-DATA-007, DG-DATA-009, DG-FIT-008, DG-FIT-011, DG-DATA-013, ORG-PERF-001, ORG-PERF-003 and DG-PERF-002. Their parent rows remain open until their own tasks and gates pass.

## 14. Open decisions

These items are not decided by this plan. Each has an owner task and must be resolved with evidence before the affected family is accepted.

| Item | Why open | Owner |
|---|---|---|
| NaN payload preservation across Arrow IPC and the SQLite container | The architecture requires bit round-trip where the format supports it, but the exact boundary behavior is unimplemented | P2-002, P2-006 |
| Exact supported text encodings beyond UTF-8 | UTF-16 and legacy code pages are optional capabilities; no implementation evidence exists | P2-003 |
| Embedded NUL and other control characters in text | Rejection by default is proposed; the final text policy needs an implementation decision and a stable diagnostic | P2-002, P2-003 |
| Timezone database source and version policy | Parsing and DST fixtures need a pinned data source; no runtime dependency is selected | P2-002, P1-003 |
| Function-library numerical tolerance registry | Each transcendental function needs a reviewed tolerance and version; none exists yet | P2-005 |
| Absolute temperature versus temperature-difference arithmetic | The data model requires offset semantics but does not define every add, subtract or multiply policy; UNIT-005 needs a declared policy before acceptance | P2-002, P2-005 |
| Recovery checkpoint cadence and default loss window | The contract requires a declared measured window; no implementation or default exists | P2-006 |
| Chunk size and byte cap | The data model proposes a 64K-row starting experiment; the actual policy must be benchmarked | P2-002, P2-011 |
| Error-code registry ownership and final names | This plan proposes test-facing detail codes and their outer-code mapping; the implementation registry is not frozen | P1-004, P2-002 |
| Cancellation checkpoint ownership for import and save | PERF-009 needs cooperative cancellation, but P2-003 and P2-006 records do not list the interface; P0-000 must allocate it or amend the affected task scope before PERF-009 can pass | P0-000, P2-003, P2-006 |
| Save As acceptance ownership in P2-006 | This plan defines Save As fixtures from the format contract, but the current P2-006 record does not list Save As; P2-006 must reconcile its record or P0-000 must allocate a follow-up before those fixtures count as P2-006 acceptance evidence | P0-000, P2-006 |
| Excel fixture rights and supported formula subset | P2-008 needs lawful fixtures and a documented subset; neither exists | P2-008 |
| Windows execution of the conformance suites | No Windows host or CI result exists; procedures are portable but unexecuted | P1-002, P1-010, P1-013 |
| Whether a later release needs an explicitly lossy or partial import mode | Atomic rejection is the only authorized P2 mode; a later task would need its own acceptance and diagnostics | P2-003 |

## 15. Change control and review

- Plan version 1.0 is the P2-001 candidate. A change to a semantic expectation, tolerance, fixture removal or diagnostic class increments the minor version and records the reason.
- The P2-001 owner reviews the document for consistency with the contracts. An independent data/QA reviewer checks coverage against the acceptance criteria, checks that no fixture invents architecture, and checks that diagnostics and tolerances are testable.
- An implementation task may add fixtures without a plan change if the addition does not alter an existing expectation. It must update its own manifest fragment, the requirement traceability and its record; it must not edit another task's fragment.
- A failing fixture is investigated and fixed in code or in the plan. It is never deleted, skipped or relabeled as an expected failure without a recorded review. A known limitation remains visible.
- No task may cite the existence of this plan as evidence that a P2 capability, requirement row or phase is complete.

## 16. Acceptance mapping and next actions

| P2-001 criterion | Where this plan answers it |
|---|---|
| AC-1 typed-value fixtures | Section 4, TYP-001 to TYP-018, including numeric, text, date, IEEE binary, missing, NaN, infinity, empty text, signed zero and display versus stored precision |
| AC-2 delimiter, quoting, encoding, header, skip/region, decimal-comma, malformed and unsupported import fixtures | Section 8, IO-001 to IO-030, with diagnostics and atomicity rules |
| AC-3 row and column identity, units, mirrors, masks and join/unmatched fixtures | Sections 5, 6 and 7, ID-001 to ID-013, UNIT-001 to UNIT-013 and ALIGN-001 to ALIGN-018 |
| AC-4 formula DAG, cycle, invalidation, recalculation and stable-error fixtures | Section 9, FML-001 to FML-024, with deterministic expected results and tolerances |
| AC-5 atomic save, interrupted save, recovery, migration, reopen, unsaved window and corruption fixtures | Section 10, SAVE-001 to SAVE-017, REC-001 to REC-016 and MIG-001 to MIG-008. REC-015 is deferred to P6 and is not a P2 acceptance fixture |
| AC-6 100MB and 1GB procedures, hardware and build disclosure, latency, throughput, peak memory and cancellation | Section 11, PERF-001 to PERF-012, with no vendor budget |
| AC-7 validator, no CJK and no implementation or parity claim | This document and the P2-001 record; validator evidence is recorded in the task record |

Next actions after P2-001 acceptance:

1. P2-002 creates the typed-value, identity, unit and alignment implementations and its own manifest fragment at `tests/data/fixtures/manifest.json` within its allowed test tree. Each later task creates its own fragment rather than editing P2-002's file.
2. P2-003 adds the text and clipboard fixtures and implements the import diagnostics.
3. P2-005 adds the formula registry, tolerance declarations and DAG fixtures.
4. P2-006 adds the container, failure-injection harness and migration fixtures.
5. P2-011 generates the large fixtures, runs the baseline and records the first actual measurements.

Until those tasks are accepted, no P2 capability, requirement row or phase gate is complete.
