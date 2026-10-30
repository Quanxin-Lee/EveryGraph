# ADR-0004: Immutable Columnar Data in a Transactional Project Container

- Status: accepted design; schema, exact library versions and performance qualification pending.
- Date: 2026-10-30.

## Context

Scientific columns must retain dtype/units/nulls/precision, be shared by plots and analyses, support edits/undo and scale beyond eager row objects. Projects also contain documents, relationships, operations and assets. A dataframe interchange format alone is not an editable project format.

## Decision

Use immutable chunked columnar dataset versions, Arrow arrays/IPC behind internal ports, chunk-level copy-on-write, stable IDs and explicit row lineage. Use Eigen workspaces for dense algebra, not heterogeneous storage. Polars is optional after query evidence; Parquet is analytical interchange, not the transactional document database.

Choose `.egp` as a SQLite container with versioned object/reference/operation metadata and bounded immutable Arrow IPC chunk BLOBs. Initial canonical files use rollback journals, not WAL. Explicit save commits a snapshot; recovery checkpoints are separate and have a documented durability window. Save As uses a consistent backup/temp/replace sequence. Package export/protection comes later without defining a competing live data model.

## Alternatives

JSON-only is inadequate for large numeric payloads and atomic edits. Custom binary serialization increases migration/recovery tooling cost. Parquet-only lacks project-object transactions. ZIP+JSON+Arrow is a useful interchange package but complicates incremental saves and atomic multi-member updates. SQLite row-per-cell storage loses the columnar compute/interop benefits; one giant BLOB loses chunk-level updates.

## Consequences

SQLite BLOB reading generally copies into the cache; this is not end-to-end zero-copy/mmap. Copy-on-write, undo pinning, content hashes and GC require byte budgets. A single-file container simplifies transaction consistency but journal/write amplification and multi-gigabyte saves need benchmarks. Null/NaN/masks and unit semantics are explicit. Unknown extensions are preserved inertly, and migrations operate on copies.

## Verification and revisit triggers

[Data model](../architecture/data-model.md) and [file format](../architecture/project-file-format.md) define lifetimes/schema/recovery. Benchmark edit/save/import/load on 100MB/1GB fixtures and large projects; failure-inject every save/migration stage. If measured SQLite chunk costs violate budgets, consider a generation-based package with atomic manifest publication, but only with a migration and crash-consistency protocol. Do not trade data integrity for an unmeasured storage trend.
