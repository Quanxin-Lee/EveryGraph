# Project File Format and Recovery

Status: chosen architecture, schema not frozen for public interoperability. Date: 2026-10-30. See [ADR-0004](../decisions/ADR-0004-data-storage.md).

## 1. Comparison and choice

| Candidate | Strength | Limitation for an editable scientific project |
|---|---|---|
| JSON only | Human-readable, extensible metadata | Huge numeric payloads, float/NaN ambiguity, slow rewrite, no multi-object transaction |
| Custom binary | Compact and controllable | New parsers, migrations, corruption recovery and tooling burden |
| Parquet only | Compressed columnar analytical data, selective reading | Not an object graph or transactional cell-edit store; row-group rewrite cost |
| ZIP + JSON + Arrow | Portable package, individual standard members | ZIP updates/atomic replacement, large rewrites, extraction/path-traversal and multi-file consistency |
| SQLite + typed chunk payloads | Single-file distribution, transactions, indexes, migration and inspection | BLOB copies, page/journal amplification, not direct Arrow mmap; large-save benchmarks required |

**Choose a SQLite container (`.egp`) with versioned metadata and immutable Arrow IPC chunk BLOBs.** It is an application document, not a shared multi-user database. JSON remains the bounded representation of node parameters/styles; Arrow IPC encodes tabular buffers; typed tile payloads extend grids/images. Parquet is an import/export format. Do not put the entire data table in one gigantic BLOB.

A native package representation is also a required P6 interchange capability: an immutable directory/export package containing a manifest and chunks, produced from a consistent snapshot. It is not a second writable database design. ZIP packaging can be added for transfer with strict member validation. Foreign OPJ/OPJU/.dgraph/DTG compatibility is a separate unresolved gate, never implied by native format support.

## 2. Logical schema

All IDs are opaque stable identifiers; schema definitions are versioned independently from command schemas.

| Table | Purpose / constraints |
|---|---|
| `format_info` | Magic/application ID, format major/minor, minimum reader, required feature IDs, project ID, saved revision |
| `objects` | ID, type/version, parent ID, order and bounded metadata; foreign-key constraints |
| `references` | Typed edges between document objects/data/operations, with deletion policy |
| `dataset_versions` | Immutable schema, row-count/shape, revision, lineage and unit/type metadata |
| `chunks` | Content hash, codec/format version, uncompressed size, checksum and bounded BLOB |
| `dataset_chunks` | Ordered chunk references with field/range information |
| `operations` | Algorithm ID/version, inputs, canonical parameters, selection policy, seed and output IDs |
| `execution_records` | Status/diagnostics/provenance references; secrets and hidden model reasoning excluded |
| `assets` | Embedded image/font-permission metadata/report attachments with type/size/hash |
| `extensions` | Namespaced plugin payload and version; never auto-executed |

Record content hashes during chunk creation. Checksums detect accidental corruption, not authenticity against an adversary who can replace both content and checksum. Validate table references, chunk lengths, integer overflows, decoded size limits and required feature versions before materialization.

## 3. Save/open semantics

The default writable document uses SQLite rollback-journal mode (`DELETE`) and a tested synchronous/durability policy (`FULL`; evaluate macOS full-flush needs). **Do not use WAL for the portable canonical file in the initial design.** A short metadata/data transaction commits a chosen immutable document snapshot. Unsaved application edits remain in memory/managed working chunks until explicit save or separate recovery checkpoint. Saving revision N while the user edits N+1 marks only N saved; the UI remains dirty.

Import/analysis builds private chunks outside the transaction. The storage executor writes necessary immutable chunks and object/reference changes in a transaction; cancellation before commit rolls back and leaves the prior saved revision. The commit itself is non-cancelable. A successful 'saved' indication follows the database commit and required flush, not merely a task submission.

Save As first pins the requested application revision, including unsaved metadata and private working chunks. Back up the existing database consistently into a temporary destination on the target volume (or initialize an empty container for a never-saved project), then apply that pinned revision's metadata, references and required chunks **to the destination**, commit, validate, close and atomically replace the authorized destination through the platform port. Do not save or modify the original as an intermediate step. Concurrent edits remain dirty relative to the newly saved revision. A plain filesystem copy of a live DB is not the save algorithm. Confirm overwrite and handle Windows file-sharing locks, low disk space and target disappearance. No stronger cross-filesystem or power-loss guarantee is claimed than the tested filesystem/storage stack provides.

Opening parses metadata with quotas, checks supported features and validates required chunk checksums on access. Corrupt optional thumbnails can be regenerated; corrupt source chunks cannot. Unsupported major versions open read-only only if essential semantics can be preserved; otherwise fail clearly. Never drop unknown required metadata and then save a damaged project.

## 4. Migration and forward compatibility

Migration is a versioned pure transformation of a copied snapshot, with original preserved until successful validation. Maintain fixtures for every publicly released major/minor format and migration chain. Format major changes denote incompatible semantics; minor additions use explicit required/optional feature declarations. Unknown optional extension payloads are preserved byte-for-byte with quotas; unknown executable payloads remain inert.

A missing analysis plugin leaves original data, node metadata and last valid result visible with a 'cannot recompute' diagnostic. Do not substitute a different algorithm silently. External sources record locator, content hash and refresh policy; credentials stay outside the project. Default project saves embed required data for portability. Deliberate externally linked projects advertise missing/offline dependencies and can create a self-contained archive.

## 5. Crash recovery

Maintain an application-managed recovery checkpoint separate from the saved user document, containing the last checkpointed domain revision and referenced immutable chunks. Use the same transactional/checksum rules. Recovery cadence is a measured loss window; a completed in-memory edit is **not** guaranteed durable until checkpoint/save acknowledgement. The UI distinguishes operation completion from durable checkpoint status.

On restart offer recovery into a new file; never overwrite the original automatically. Preserve failed files, report lost/corrupt chunk IDs and recover independent intact objects where safe. Undo records may be checkpointed, but scientific provenance is retained independently of an undo-history limit. Garbage collection deletes only unreferenced managed chunks after current document, undo, saves, tasks and recovery references have been accounted for.

Test crashes at each transaction/save/migration stage, disk-full, truncation, checksum mismatch, missing external sources, malformed schema and lock contention. Network/cloud-synced directories are not supported as live multi-writer stores; use a local working copy and explicit synchronized export. SQLite's atomicity depends on functioning OS locking/flush/storage, not immunity to arbitrary hardware corruption.

## 6. Password protection

Native protection is required for P6, **not provided by plain SQLite**. The preferred direction is a separately versioned authenticated-encryption envelope around a closed snapshot, using an audited maintained library with a password KDF and authenticated chunk stream; select exact library/parameters via a security ADR and benchmark. No custom cryptography and no misleading 'password' metadata field.

Design must cover password change, wrong-password/tamper rejection, KDF parameter bounds, protected recovery files, key lifetime and user-approved plaintext working storage. An unlocked application necessarily exposes plaintext in memory, and OS swap/core dumps can matter. Do not promise protection from a compromised host. Foreign protected-file compatibility requires lawful SDK/specification access and remains unresolved.

## Sources and scope of evidence

- [SQLite atomic commit](https://www.sqlite.org/atomiccommit.html): transaction mechanics and filesystem assumptions.
- [SQLite backup](https://www.sqlite.org/backup.html): supported consistent-copy API.
- [SQLite WAL](https://www.sqlite.org/wal.html): operational constraints to review before any later WAL adoption.
- [Arrow columnar format](https://arrow.apache.org/docs/format/Columnar.html): buffer/type representation, not a complete project format.

These sources support mechanisms. No EveryGraph crash-recovery, multi-gigabyte save or migration implementation has yet been tested.
