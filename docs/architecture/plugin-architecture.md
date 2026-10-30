# Scripting, Plugins and External Engines

Status: design, 2026-10-30. [Application commands](module-design.md) and [AI permissions](ai-architecture.md) define shared policy. [Upstream research](../research/graphics-and-runtime.md) shows why process separation and plugin registration are not sandbox guarantees.

## 1. Extension classes

| Class | Execution | Trust/compatibility rule |
|---|---|---|
| Built-in native numerical backend | Host process | Project-reviewed, release-pinned C++ code; no arbitrary user library loading |
| Python script / plugin | Dedicated interpreter worker | User-approved environment, versioned API and explicit resource grants |
| Third-party C++ / Rust extension | Worker process by default | Message protocol; process/OS architecture and package version declared |
| Trusted performance extension | Optional worker-local native C ABI | Host loading remains exceptional, explicit architecture/security approval |
| External R/MATLAB/Mathematica/LabVIEW | Adapter-managed process/service | Installed authorized runtime and per-product licensing; no automatic redistribution |
| Declarative UI contribution | Host-rendered schema | Commands/forms/panels described as data, not arbitrary host widgets |

A plugin's language does not determine its trust. A Rust plugin can read files or invoke unsafe native libraries; a Python wheel can contain native machine code. Python is the primary scientific scripting language, but not the main UI/runtime.

## 2. Public API and data interchange

The Python `everygraph` client presents stable project/workbook/plot handles and typed command wrappers. Calls serialize to the same gateway as UI and AI. Queries return bounded metadata or data leases; bulk interchange uses Arrow IPC/shared memory with explicit schema, read-only lifetime and platform handle passing. Writable NumPy results are newly published candidates, never views into live host-mutated memory.

Use pybind11 only when a worker needs a fast local binding to approved native algorithms; it is not required to expose the entire desktop object graph. Embedding Python into the GUI process would create GIL, ABI, package and crash coupling and is not the default. Worker environments are project/user-scoped and locked; do not install arbitrary dependencies into the app's interpreter on file open. Python API semantic versions and data protocol versions are distinct from CPython ABI versions.

A native C ABI, when needed, uses versioned function tables, struct sizes, fixed-width integers, opaque handles, error codes, ownership/release callbacks and allocator provenance. No C++ STL/Eigen/Qt/Rust layout types cross it; no exceptions or Rust panics unwind across FFI. A C ABI is more stable than a C++ class ABI but not magically immutable: alignment, calling convention, architecture and lifecycle remain tested contracts.

## 3. Protocol and lifecycle

Use length-prefixed, versioned request/reply/event frames over host-created local pipes or sockets. Authenticate the worker session with an ephemeral host-issued token and OS peer checks where available. Strictly validate frame lengths, schemas, message IDs, handle ownership, expiry and resource limits. Do not expose an unauthenticated localhost command server.

Lifecycle: discover manifest without executing code -> validate signature/hash and dependency policy -> obtain user approval -> prepare environment -> launch -> negotiate protocol/capabilities -> register declarative contributions -> execute -> cancel/quiesce -> release leases -> stop. Disable prevents new work; uninstall waits for or explicitly cancels jobs. Update is atomic with rollback metadata and re-approval when permissions increase. Signatures establish publisher/integrity, not benign behavior.

Manifest fields include plugin ID/version, host API range, entrypoint/runtime, platforms/architectures, capabilities, data/algorithm schemas, license, provenance/hash and dependencies. Treat algorithm upgrades as potentially result-changing; persisted projects retain exact algorithm identity and old result provenance.

## 4. Containment design and unresolved release gate

Process isolation limits memory corruption propagation. **It does not restrict a worker running as the same user from reading arbitrary files or opening network sockets.** A capability broker alone cannot enforce denial if direct OS calls remain available.

Before advertising untrusted-plugin execution, qualify actual OS containment: Windows restricted/AppContainer-style process policy plus Job Object resource/lifecycle controls, and a supported macOS sandbox/helper distribution design with brokered file access. Exact mechanisms, entitlement interactions and native Python package compatibility require platform experiments; do not rely on deprecated private sandbox APIs. If a platform cannot enforce the promised policy, support only explicitly trusted code there and label it clearly. Do not silently relax containment.

Broker grants are narrow and revocable: dataset reads, scoped file handles, specific export destinations, approved network hosts and optional subprocess execution. Deny ambient home-directory access, credential-store access and inherited arbitrary environment variables. Shared buffers have size/offset/hash validation and bounded leases. Worker failures cannot commit half a command; outputs are inspected before host publication.

Resource limits cover CPU time, memory, disk, output/frame size, open handles and spawned processes. Cooperative cancellation has a deadline; force termination is only for the isolated process tree, followed by managed resource cleanup. A plugin cannot write its own authoritative approval/audit record.

## 5. Ecosystem and compatibility

P6 includes installed-plugin management, discovery/search/categories, dependency resolution, documentation/examples and declarative menus/toolbars. An online catalog is an optional distribution service, not required to run the app; catalog publication needs separate governance, review and supply-chain safeguards. No automatic background execution of newly installed plugins.

QGIS/napari/Blender illustrate discoverability and lifecycle, not universal secure extension APIs. Origin Apps, LabTalk, Origin C, Igor XOP and DataGraph CLI/DTG are proprietary compatibility questions, not automatically runnable in EveryGraph. Separate functional-equivalent commands from source/ABI/file compatibility and keep unresolved gates in [scope](../planning/functional-scope.md).

## 6. Tests and acceptance

Run each supported package on both first-tier platforms. Test protocol version mismatch, malicious lengths/handles, denied filesystem/network/process operations, package install/update failure, missing runtimes, revocation during work, hung/crashed plugins, canceled publication, cleanup and restart. Assert the host remains responsive and the project unchanged after failure. Numerical extension tests use the same reference fixtures as built-in methods. Distribution audits include Python wheels, native runtime dependencies and plugin/model licenses, not only a top-level plugin manifest.
