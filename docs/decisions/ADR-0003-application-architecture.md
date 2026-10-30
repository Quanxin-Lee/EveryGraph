# ADR-0003: Modular Monolith with One Command Gateway

- Status: accepted architectural contract.
- Date: 2026-10-30.

## Context

EveryGraph is initially a local desktop, but needs UI/Python/AI automation, asynchronous analysis, undo/reproducibility and safe extension boundaries. Starting with distributed microservices would add operations/protocol failure modes without a demonstrated user need.

## Decision

Use explicit CMake module targets with inward dependencies: adapters -> application ports -> domain/dataflow -> data/foundation. The desktop composition root wires them. The application gateway owns schema/permission/revision validation, execution planning, publication and history; all user-facing clients call it. Data snapshots are immutable and workers return candidates. The application executor serializes short commits. The plot compiler and backends communicate through a GUI/GPU-independent scene contract.

Use separate processes only for concrete runtime/trust/failure boundaries (Python, third-party native code, optional inference/external engines). OS sandbox qualification is separate from process isolation. No network service is required for normal work.

## Alternatives

A UI-centric monolith is simpler initially but entangles scientific state with widgets and creates separate automation logic. Microservices add deployment and consistency cost. An all-plugin architecture pushes essential compatibility/versioning work into every feature. A fully event-sourced document is not required: use transactional state plus provenance/undo records, with explicit save/recovery semantics.

## Consequences

More initial contracts and tests are required than a single main-window implementation. Modules can remain statically linked without becoming a public binary ABI. Declarative node/command schemas support future clients/remote execution, but network scheduling is not designed in advance. Avoid service-locator/global-state shortcuts and mutable buffers shared across tasks.

## Verification

See [module dependencies](../architecture/architecture-overview.md), [command contracts](../architecture/module-design.md), [DAG/concurrency](../architecture/concurrency-model.md), [AI flow](../architecture/ai-architecture.md) and [plugins](../architecture/plugin-architecture.md). Fitness tests require no cycles/Qt types in core APIs, uniform command semantics, stale-result rejection, atomic undo/publication and cross-platform failure tests. These application tests are planned, not yet implemented.
