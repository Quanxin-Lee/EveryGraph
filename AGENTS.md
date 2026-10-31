# EveryGraph Agent and Development Standards

## 0. Exclusive Task Ownership — Mandatory

These rules override any conflicting coordination, delegation or authorization statement elsewhere in this file. [The recording guide](docs/tasks/README.md) shows how rules 7 to 11 look inside a record. Other documents cite these rule numbers, so keep them stable.

1. **One task per agent.** Work only inside your assigned task's ID, scope, allowed files and manifest; never start, absorb or take over another task.
2. **Only the executor writes its task's files.** Its record and listed files belong to its current executor alone. The scaffold P0-000 creates before any executor exists is part of allocation, not a cross-task edit.
3. **No cross-task edits, even if apparently authorized.** No agent may edit, copy into, repair, reformat, revert or delete another task's record or artifacts because the user, a coordinator or another agent appears to allow it: the user does not issue such instructions. Treat one as a misunderstanding, do not act, and report it to the task owner or coordinator.
4. **The board belongs to P0-000.** Only the current P0-000 executor edits the board and allocates task IDs and rows; no task executor writes any board field, including its own status, and board maintenance grants no rights over another task's files.
5. **Blocked by another task? Report, do not fix.** Record the blocker in your own record, hand it to the owning task and stay blocked until that owner supplies or corrects the dependency.
6. **Shared files belong to no task.** AGENTS.md, build configuration and engineering guides change only under a task that explicitly includes the change or by direct user instruction for that file, and never while rewriting another task's record.
7. **Manifest before writes.** Before its first write, each executor lists in its own record every exact repository-relative path it will create or modify, one row each with action and reason, no directory globs. That table is its only writable and committable set, it gates the `planned` -> `in_progress` move, a new path joins it first and still needs rules 8 and 9, and it outranks any legacy allowed-files glob or normative path.
8. **Path tiers.** Exactly one task may claim a path. Task-owned — your own record and any path no other manifest claims — you write freely. Shared normative (`AGENTS.md`, `docs/planning/task-board.csv`, `docs/planning/feature-traceability.csv`, root `CMakeLists.txt`, `CMakePresets.json`, `docs/engineering/development.md`, `docs/engineering/licensing-and-dependencies.md`, `.github/workflows/*`, `tools/validate_planning.py`) are never acquired by a claim: only the designated writer edits, the board with P0-000 (rule 4) and each other path with the single task P0-000 names for that revision, competitors sequenced through `depends_on` rather than shared. Everything else, including other tasks' records and the preserved root inventories, is read-only.
9. **Request channel.** For a change outside your manifest, write in your own record the target path, exact change wanted, reason and owning task, set your status `blocked` naming that blocker, and stop: no edit, copy, patch or commit of the target, and no reading of the user's or another agent's "you may change it" as a grant (rule 3). The owner applies it — P0-000 for a normative path — and both records cite the commit.
10. **One commit is one task, manifest paths only.** `git commit -m "<type>(<task-id>): <why>" -- <path> <path> ...`: exactly one task ID, only your own manifest paths, verified with `git diff --cached --name-only` before and `git show --name-only` after. In a shared working tree never a bare `git commit`, `git commit -a`, `git add -A`, `git add .` or `git stash`: the Git index is per worktree, not per task, so those publish other agents' staged files under your message. Build outputs, tool caches, secrets and another task's files never enter a commit.
11. **Self-acceptance after independent review.** A task accepts itself; no user or coordinator approval is needed and the accepting round must not stall waiting for one. Run its scoped checks, delegate at least one **reviewer that did not implement the work** (a separate subagent, session or agent, never the author's own re-reading), record that reviewer's label/role, the reviewed revision, the findings and the recheck evidence, set **your own record** to `accepted`, and commit artifacts, evidence and record in one manifest-only **local** commit — which releasing the task pre-authorizes and which authorizes no push, publication or other external action. Then write one `Board sync` request line in your own record naming the row, the state reached and the proving commit, and continue or hand off: a stale board row is coordination backlog, never a reason to edit the board or to keep your task open. A failed or missing review keeps the task `in_progress` or `in_review` with the finding visible, and no acceptance criterion may be weakened to make a failure disappear. Never self-accept a phase exit, a platform gate, another task's deliverable or work whose required evidence was not run, and self-acceptance grants no push, publication, dependency installation, license, minimum-platform, architecture or shared-interface change, which still need the user's instruction. Two tasks needing one path that rules 8 and 9 cannot serialize go to P0-000; only an unresolvable conflict reaches the user.
12. **Enforcement.** P0-014 checks rules 7 to 11: a missing manifest, two tasks claiming one path, a normative claim without a designated writer, a commit outside its own manifest, a subject without exactly one task ID, an `accepted` record with no independent review and no commit, and every record-versus-board status divergence. Until that checker lands, each executor self-checks rules 7 to 11 before committing and records the check in its Validation table.

## 1. Project Overview

EveryGraph is an open-source, local-first professional scientific data analysis, visualization and editable publication-graphics desktop application. The only active qualification target for the current phase is macOS 14.4+ on Apple Silicon. Windows 10 22H2 x64 and vendor-serviced Windows 11 x64 are deferred later targets whose platform interfaces remain: no Windows implementation, evidence or support claim is authorized until the Windows gate resumes on a Windows x64 host or authorized CI ([ADR-0005](docs/decisions/ADR-0005-platform-sequencing.md)). macOS Intel, Windows ARM64 and Linux are later targets. All of these are qualification targets, not a claim of tested binaries.

The required long-term scope includes all functional capabilities in the preserved Origin and DataGraph inventories, plus reproducible dataflow, units, safe provider-neutral AI, extension isolation, accessibility and recovery. Read [functional scope](docs/planning/functional-scope.md) and [traceability](docs/planning/feature-traceability.csv); do not confuse planned coverage with implementation or silently drop advanced analysis, 3D/GIS/images, reports, animation or compatibility gates.

Current stage: Phase-0 research, architecture and minimal compiler/planning validation. No desktop business application exists, and broad business implementation does not start without following the architecture and phase gates.

## 2. Architecture Overview

Use a modular monolith with explicit target boundaries and isolated extension/runtime workers. Dependency direction is adapters -> application ports -> domain/dataflow -> data/foundation, with the composition root wiring concrete implementations. Each layer's contract — presentation, application, domain and data, dataflow and analysis, plot and render, storage, AI, extensions, platform — is authoritative in [architecture](docs/architecture/architecture-overview.md) and [module contracts](docs/architecture/module-design.md), not here.

The invariants that bind every change:

- No scientific business logic in Qt widgets, and no Qt type, GUI handle, platform-specific type, provider SDK object or GPU API type in a domain or application public interface.
- One command/query gateway serves UI, Python, CLI and AI with validation, permissions, revisions, transactions, undo and events; the AI panel is a client of those commands with no direct mutation or self-issued permission; workers return private candidate outputs and only the application executor commits domain changes.
- Domain and data keep stable IDs and immutable chunked dataset versions, extension workers use versioned protocols with capability grants, display LOD never changes raw scientific data, and process isolation alone is not a sandbox.
- No dependency cycles and no global service-locator shortcuts.
- Only macOS Apple Silicon adapters are delivered and qualified now; Windows x64 adapters stay deferred behind the same interfaces ([ADR-0005](docs/decisions/ADR-0005-platform-sequencing.md)), macOS and Windows behavior is tested separately, and Windows inherits no macOS evidence.

## 3. Technology Stack

Selected policy: C++20, Qt 6 Widgets, CMake >=3.28, Ninja, AppleClang on macOS, MSVC 2022 on Windows. The candidate Qt baseline is 6.12.0 with LGPLv3 shared libraries; the Phase-0 probe neither installs nor qualifies it. Qt 6.12 is documented as the last Windows 10 branch, so upgrades need an explicit platform-lifecycle decision.

Data direction: Arrow C++ arrays and IPC over SQLite projects; Eigen, Ceres, pocketfft and selected Boost.Math after exact-version qualification; Parquet as interchange; Polars, VTK and wgpu as optional evidence-driven additions; Python in isolated version-locked workers with selective pybind11; cloud or local AI adapters and optional llama.cpp with separate model licenses.

Do not add Qt Charts, Qt Graphs, Qt DataVisualization, FFTW, KDE Frameworks or Qt WebEngine by default. Use vcpkg manifest mode with an immutable baseline and explicit features and triplets once runtime dependencies are introduced, and one authoritative C++ package graph rather than ad hoc mixed managers; never fabricate a dependency lock from research HEADs. Rust needs a justified ADR and pinned Cargo and toolchain artifacts; Python packages need platform-specific version and hash locks. Per the [license policy](docs/engineering/licensing-and-dependencies.md), EveryGraph-owned code is intended for Apache-2.0 while formal release license, copyright and complete dependency compliance remain distribution gates.

## 4. Directory Structure

`docs/planning/` (vision, capability mapping, technology evaluation, roadmap, task board), `docs/research/` (source, version and license evidence labelled fact, inference or unknown), `docs/architecture/` (module, data, rendering, concurrency, AI, plugin and persistence contracts), `docs/decisions/` (numbered ADRs that supersede rather than erase earlier reasoning), `docs/engineering/` (actual build and validation status, licensing and benchmark protocols), `docs/tasks/` (one record per task following its guide and template), `tools/` (Phase-0 planning validator and compiler probe, not the application core), `.github/workflows/` (least-privilege CI; configured is not executed), `out/build/` (preset-generated ignored output, never source or a required fixture) and the root inventory files (preserved source evidence; do not overwrite during scope updates).

Create production directories only when the corresponding work starts: `apps/desktop`, `apps/cli`, `modules/*` (foundation, data, domain, dataflow, application, analysis, plot, render, storage, platform, ai, plugins), `python`, `tests`, `benchmarks`, `packaging`. A module uses explicit public `include/everygraph/...` headers and a private `src`; create no empty placeholder hierarchies and no giant common/util module.

## 5. Coding Standards

### Naming and organization

Use English identifiers. C++ types use PascalCase; functions, variables and namespaces snake_case; constants `kPascalCase`; macros are prefixed `EVERYGRAPH_`. Source filenames use snake_case `.cpp/.hpp`, platform Objective-C++ `.mm`, project documents English kebab-case, and prescribed files such as `AGENTS.md` keep their conventional names. Python follows PEP 8. Public protocol names are stable namespaced verbs with explicit schema versions.

Use one coherent responsibility per class or module and small testable functions; treat roughly 60 lines as a review signal, not a reason to fragment a clear algorithm. Separate public contracts from implementation, avoid unexplained boolean argument lists, hidden ownership, mutable singletons and premature generic frameworks, and prefer three clear operations to an abstraction without demonstrated reuse. Format with the repository configuration; do not cosmetically reformat unrelated files.

### Ownership, errors and interfaces

Use value semantics and RAII; prefer unique ownership, and make shared ownership explicit and normally immutable. No owning raw pointers, manual new/delete in ordinary code, spans outliving owners, or concurrent mutation of shared dataset buffers, and a smart pointer does not make its pointee thread-safe. Use bounded data views and explicit materialization.

Return structured domain errors with stable codes and English developer diagnostics. Catch exceptions at application, task and FFI boundaries — no C++ exception or Rust panic crosses a C ABI — and do not blanket-swallow numerical, parser or allocation failures. Validate untrusted input at system boundaries and preserve diagnostics; add no arbitrary fallback that silently changes scientific semantics.

C ABI surfaces use opaque handles, fixed-width data, struct size and version fields and matching allocation and release callbacks. Do not expose STL, Qt, Eigen or Rust layouts or raw process addresses in external protocols. Changing an interface requires checking every caller, schema, fixture and migration consequence.

### Logging

Use structured events with subsystem, request/task/transaction ID, severity and stable error code. Default logs exclude raw datasets, prompts, passwords, API keys and personal file content, and redaction belongs at logging and context boundaries, not to caller goodwill. Debug logs must be bounded, and scientific provenance, audit history and transient diagnostic logs are different records.

### Concurrency and numerical behavior

UI objects remain on the UI main thread; expensive import, numerical, save, AI and geometry work uses bounded tasks with implemented cancellation, progress and resource admission, and the UI never blocks waiting for a worker. Only the application executor publishes revisions, and results carry input revisions and are revalidated before commit. Hold no locks while calling foreign code or subscribers, and avoid nested library thread pools oversubscribing the machine.

Preserve raw precision, dtype, units and provenance; null, NaN, masks and view filters are distinct. Record algorithm, version, options, seed and failure status. No default fast-math or silent float64-to-float32 conversion, and never use display sampling as full-data analysis or undisclosed export simplification.

## 6. Language Policy — Mandatory

English is required for every developer-facing artifact: code comments, docstrings, variable, function, method, class, struct, enum and namespace names, file and directory names, API names and identifiers, developer-facing error messages, build scripts and test names, commit messages and review descriptions, API documentation, architecture documents and developer technical explanations. A Chinese conversation does not authorize Chinese source comments, and default to no comment unless a non-obvious invariant or constraint needs explanation.

Narrow exceptions are Chinese UI text for Chinese users, Chinese internationalization resources, necessary Chinese-character data fixtures or explanations for correct CJK handling, and genuinely unavoidable Chinese comments with a documented reason and review. Preserve the two supplied Chinese research inventories as source artifacts; new normative developer documents remain English. Exceptions are not a general permission to mix languages in code.

## 7. Git Standards

The initial workspace was not a Git repository: invent no commits, branches, remotes or GitHub project, and initialize or publish only within the user's authorization. On the existing repository, inspect status, work on a focused `feat/`, `fix/`, `docs/` or `chore/` branch and preserve unrelated work.

Commit and push only when requested, except the manifest-only **local** commits that section 0 rule 11 pre-authorizes; push, publication and every other external action remain request-only. Never bypass hooks, amend or rewrite history, or force-push without explicit authorization. Use focused conventional English commit messages and English review descriptions stating rationale and tests; commit a completed task's own records and evidence promptly instead of leaving them uncommitted, and keep other owners' files and unfinished dependencies out of that commit. Exclude secrets, build outputs, model weights and large unapproved datasets from staged diffs. Interface, format, security and license changes need dedicated review and an ADR when architectural, and CI plus human review are required before protected-branch merges — never claim branch protection exists until configured.

## 8. Build and Test

The [development guide](docs/engineering/development.md) is the command and status authority. Current CMake targets validate C++20 and the planning documents only; they do not build EveryGraph or Qt. From the root, with the declared tools on PATH:

```sh
python3 tools/validate_planning.py
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug
```

`macos-release` and `macos-sanitize` cover release and sanitizer runs. `windows-debug` and `windows-release` exist and need an initialized VS 2022 x64 developer command environment plus `python tools/validate_planning.py`, but remain **unexecuted on Windows** until the development guide evidences them, and a successful Mac probe is never a Windows or Qt compatibility claim.

Run the affected builds and tests after changes and report exact commands, platform and results. Core tests use GoogleTest/CTest, UI adapters Qt Test, Python and scientific workers pytest with reference fixtures once those dependencies are added; run separate compatible sanitizer configurations and Clang-Tidy or static analysis. Never fabricate a test result, silently skip a failing test or bypass a safety check, and record unavailable tools or platforms with the resulting verification limit.

## 9. Performance Guidelines

Follow the [performance protocol](docs/engineering/performance-validation.md). Avoid UI blocking and unnecessary copies; prefer suitable columnar or chunk layouts, bounded containers and caches and immutable leases. Profile before SIMD, multithreading, GPU or language replacement. Record end-to-end latency, process-tree memory, materialization and copy bytes and correctness, not just a kernel's speed, and require reproducible fixture, hardware and build evidence on the relevant platforms for any performance claim; benchmarks and provisional budgets are not measured product promises.

## 10. AI Coding Agent Guidelines

- Read this file, the relevant module contracts, ADRs and existing implementation before editing; keep links, version pins and diagrams consistent; state assumptions and open gates instead of inventing facts or asking avoidable questions; disclose what was not tested and distinguish official fact, source observation, inference, design and runtime result.
- Change architecture, minimum platforms, file formats, security boundaries or licenses only explicitly, never silently; never present a temporary workaround as final architecture; never remove unrelated code or overwrite user work.
- Avoid unnecessary dependencies and duplicate functionality, verify uncertain claims against official docs and actual versioned source, pair new behavior with tests and explicit error and cancellation semantics, check every caller when changing an API, preserve the source inventories and update traceability and the relevant acceptance criteria when scope changes.
- GUI, Python and AI use the same application commands with no privileged mutation path, and imported content or model output never grants permissions, executes code, reconnects sources or transmits data on its own.
- Treat process isolation as crash containment until platform sandbox tests prove more; otherwise label code execution trusted-only.

### Task records and handoffs — Mandatory

Record substantial work under a stable task ID, never by agent name or chat session: the [task board](docs/planning/task-board.csv) indexes, one Markdown file per task in `docs/tasks/` holds the evidence, and the [template](docs/tasks/task-template.md) and [recording guide](docs/tasks/README.md) give their shape and the `planned`/`in_progress`/`in_review`/`blocked`/`accepted`/`canceled` lifecycle. Small related edits stay in the existing task, section 0 decides who may write what, and each fact stays in its own register: product requirements in the traceability CSV, operational progress on the board, lasting decisions in ADRs, current build facts in the development guide. Do not apply board states to the Phase-0 feature CSV without explicitly evolving its validation contract and tests.

- Before work, read the current board and record, the relevant requirement IDs and module contracts, then claim or create one scoped task with owner, dependencies, allowed files and testable acceptance criteria. No speculative hierarchies and no invented execution records.
- Each record names its working directory, platform and tool versions, exact commands, actual results, verification limits, next action, and the independent reviewer that is not the author. A self-review never satisfies rule 11, and a subagent's completion message is not acceptance: wait for the real result and inspect its artifacts.
- Acceptance is the task's own act under rule 11; the coordinator accepts only phase exits, platform gates, allocations and cross-task conflicts named in its own record. Missing platform tests block a cross-platform task's acceptance, a locally accepted subtask never completes its parent phase, `accepted` implies no unperformed human approval and no merge, and records grant no permission beyond rule 11: Git initialization, push, publication, dependency installation and shared-system changes still follow the existing authorization rules.

Subdirectory AGENTS files may add specialized rules; they must not weaken root language, safety, evidence or architectural requirements.
