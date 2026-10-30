# Module Design and Application Contracts

Status: normative architectural design, not existing APIs. All identifiers below are planned names. See [dependency diagram](architecture-overview.md), [dataflow](concurrency-model.md) and [scope](../planning/functional-scope.md).

## 1. Modules and public surfaces

| Module / future target | Responsibility and public interface | Allowed dependencies |
|---|---|---|
| foundation | IDs, Error, typed units metadata, owning byte buffers, clocks; no global mutable state | C++ standard library |
| data | DatasetHandle, DatasetVersion, typed read views, chunk builders, masks, row identity | foundation; Arrow behind implementation boundary |
| domain | Project/Workbook/Graph/Layer values, specifications, invariants | foundation, data |
| dataflow | NodeDefinition, NodeInstance, dependency index, invalidation, execution-plan contract | foundation, data |
| application | CommandRegistry, CommandGateway, QueryService, transactions, revision store, undo, event streams; owns ports | domain, dataflow |
| analysis | Registered numerical node executors, diagnostics and reproducibility metadata | dataflow, data; audited numerical libraries |
| plot | GraphSpec to immutable SceneSnapshot; layout, ticks, transforms, LOD and hit-testing index | domain, data, render-contracts |
| render-contracts | Paths, glyph runs, images, clips, transforms, mesh/instancing descriptors and capabilities | foundation |
| render-qt | CPU reference renderer and vector/raster exporters | render-contracts, Qt Gui/Svg/PrintSupport |
| storage | Versioned project serialization, chunk loading, migration, save/recovery | application ports, domain/data, SQLite/Arrow IPC |
| import-export | Import probes/readers/builders, typed format options, safe external output plans | application ports, data; format-specific adapters |
| presentation-qt | Main window, document hosts, virtual table, scene editor, project/property/AI panels | application public API, render contracts; Qt |
| ai | Providers, context selection, proposal validation, workflow orchestration | application API and network/secret ports |
| scripting | Python client API and external-runtime adapters | serialized command/query protocol, data interchange |
| plugins | Discovery, version resolution, capabilities, worker broker and lifecycle | application API, platform process/resource ports |
| platform | OS files/locks/secrets/share/print/process/signing adapters | foundation and narrow application ports |
| desktop / cli | Composition roots and lifecycle; instantiate/wire concrete modules | required public modules |

Additional capability packs have explicit ownership: formula and grouping in dataflow/application; statistics/fitting/peaks/signals in analysis; scene-editor/digitizer in presentation+plot; reporting/animation in domain+application with output adapters; connectors/compatibility in import-export+plugins; grid/image/GIS/3D in data/analysis/plot backends. CSV module labels are capability owners, not a requirement to create dozens of libraries immediately.

## 2. Unified command envelope

Use versioned structured commands; public naming is `namespace.verb`, with snake_case Python wrappers. A command has:

- `command_id`, `schema_version`, `request_id`, principal/session identity and optional parent workflow ID.
- Typed arguments and object references, expected **read and write revisions**, requested result mode and cancellation policy.
- Host-resolved capabilities, effect classification and confirmation receipt. The caller cannot manufacture these grants.
- An idempotency key for retryable effectful requests, scoped to principal/project and bound to a canonical argument digest.

Example protocol shape (design example, not executable application code):

```json
{
  "command": "analysis.fit_curve",
  "schema_version": 1,
  "request_id": "example-request",
  "project_id": "example-project",
  "expected_revisions": {"dataset-a": 7},
  "arguments": {
    "x": {"dataset_id": "dataset-a", "field_id": "x"},
    "y": {"dataset_id": "dataset-a", "field_id": "y"},
    "model_id": "linear",
    "selection": {"kind": "all_valid"},
    "weighting": "none",
    "missing_policy": "pairwise_drop"
  }
}
```

IDs shown are readable example placeholders, not the final UUID wire encoding. Unknown fields/commands/versions are rejected rather than guessed. Large arrays never travel as JSON arguments; pass immutable data handles with schema and lifetime rules.

Initial command families: `project.create/open/save`, `workbook.create`, `data.import/filter/edit/formula`, `plot.create/update_style`, `analysis.describe/fit_curve`, `export.plot`, `history.undo/redo`. Later node registries add methods without bypassing the gateway. Read-only queries support bounded projections, pagination and revision-stable snapshots; they are not disguised writes.

## 3. Execution contract

1. Resolve command schema and authenticated caller; reject malformed arguments, unknown IDs and oversized requests.
2. Validate types, units, ownership, algorithm options and expected revisions.
3. Classify effects (read, reversible document write, external write/delete, external execution, network transfer); compute a human-readable plan and affected objects.
4. Request capability/confirmation where required. Bind receipt to argument digest, input revisions, destinations, expiry and principal; changed arguments invalidate it.
5. Reserve resources and run using immutable inputs. Return a task handle for long operations.
6. Validate candidate shapes, finite-value policy, convergence, diagnostics and read-set freshness.
7. Atomically commit any domain changes and undo metadata on the application executor, then publish sequenced document events. This is a `document_committed` stage, not necessarily terminal command success; a read-only export instead pins its validated snapshot without inventing a document mutation.
8. Enter `effects_pending` for separately authorized external effects. Recheck revocation immediately before each irreversible action, use temporary-output/replace protocols where applicable, and record per-effect durable receipts or failures. Only after all required effects finish does the command become terminal. A document commit followed by a failed/canceled effect reports partial completion with the committed revision and each effect's status; it does not claim either full success or complete rollback. Document undo does not undo sent mail or an external file overwrite.

Return values distinguish `accepted/running/succeeded/failed/canceled/superseded/partially_succeeded`, expose the current execution stage, and include result handles, revisions, per-effect receipts, warnings and optional document-only undo transaction IDs. Stable error codes include `invalid_argument`, `permission_denied`, `revision_conflict`, `resource_exhausted`, `canceled`, `numerical_failure`, `unsupported_feature`, `io_error`. Developer diagnostics are English; UI localization maps codes to user text without parsing error strings. Never silently turn a failed fit into successful empty data.

## 4. Undo, history and events

Small edits store inverse patches; large data edits store old/new immutable version references. A transaction groups a deliberate user action or approved AI batch into one undo item. Redo restores previously validated results where possible; recomputing a stochastic or version-sensitive operation is a new execution, not faithful redo. Undo history is bounded by actual pinned memory/disk, not only a command count; discarded undo range is visible.

Long analysis tasks do not monopolize an open mutation transaction. Prepare privately, then short commit. Undo during running work cancels/supersedes its pending publication. After an edit followed by undo and a new edit, discard only the redo branch, retaining persisted provenance needed by reports. Execution history, scientific provenance, security audit and undo stack have distinct retention rules.

Events contain transaction ID, sequence, changed IDs/revisions and lightweight descriptors, never whole copied arrays. Subscribers obtain snapshots through queries. Replay detects gaps and rebuilds from a snapshot. UI subscriptions are queued onto the main thread and automatically disposed with the document; synchronous observer reentry cannot issue nested mutations.

## 5. Numerical engine contracts

Every method declares input dtype/shape, unit rules, missing/NaN policy, required sampling assumptions, options, output/result schema, tolerances and failure semantics. Fit results include parameters, covariance/uncertainty qualification, residuals, iterations, solver termination reason, objective definition, weights, degrees of freedom and reproducibility metadata. Parameter bounds, global sharing, ODR and implicit models require distinct tested formulations; least-squares library availability alone does not implement them.

Start with Eigen for dense algebra and stable descriptive routines; Ceres for approved nonlinear least squares; pocketfft for FFT after its exact license/build integration gate. Use vetted Python scientific workers for advanced statistics where that reduces algorithm risk, but package/version/test those methods as supported product capabilities rather than requiring users to improvise scripts. Candidate SciPy/statsmodels/scikit-learn/PyWavelets/DOE/survival packages require individual API, numerical and license qualification. Do not claim a single solver covers the entire OriginPro method inventory.

No `-ffast-math` in scientific builds by default. Use stable reductions/compensated summation where appropriate and record reproducibility tolerance under parallel reduction. Float64 is the numerical default; preserve original integer/float32 data and expose explicit conversions. Reject singular/ill-conditioned results or mark their uncertainty invalid; don't just suppress warnings.

## 6. Platform ports

Expose `FileAccess` (open/atomic replace/lock/flush), `SecretStore`, `ProcessHost`, `ResourceLimiter`, `SystemIntegration` (clipboard/share/print), and `DisplayCapabilities`. Keep platform code in separate implementation files selected by build targets.

macOS: Qt/AppKit boundary, Objective-C++ only in adapters, Keychain, same-volume safe replace and flush policy, optional Metal backend, app bundle/sign/notarize. Windows: Qt/Win32 boundary, Credential Manager or equivalent audited credential store, Unicode/long paths and case-insensitivity, file-sharing/delete semantics, Job Objects plus a separately qualified sandbox, optional D3D backend, signed installer. Normalized paths are not security authorization; broker operations use granted handles and recheck link/reparse traversal.

Packaging and sandbox behavior are tested on both platforms from P1. Windows does not inherit macOS guarantees, and COM/OLE/EMF are not silently called cross-platform. macOS Intel, Windows ARM64 and Linux get separate qualification matrices when scheduled.
