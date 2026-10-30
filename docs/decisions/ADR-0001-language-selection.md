# ADR-0001: Use C++20 for the Trusted Native Core

- Status: accepted for Phase 0; production qualification pending.
- Date: 2026-10-30.
- Decision owners: technical lead and future maintainers.

## Context

EveryGraph needs native macOS/Windows desktop integration, professional scientific data/plotting, numerical libraries, Python extensions and long-lived project files. Performance cannot be decided from language rankings. [Evaluation](../planning/technology-evaluation.md) separates language, framework, measured unknowns and ordinal engineering fit.

## Decision

Use C++20 for the application/domain/data/analysis/plotting core, with RAII, explicit ownership, immutable cross-thread snapshots, target-level standard settings and no compiler extensions. Do not require C++23 library features until both supported toolchains pass corresponding probes. Use C ABI or versioned worker messages at external/native language boundaries. Python is the primary scientific scripting language in separate workers. Rust remains allowed for a justified isolated component, not a second mandatory core stack at launch.

## Alternatives

C provides useful ABI/numerical integration but too much manual ownership burden for the primary application. Rust materially improves safe-code memory/concurrency guarantees; Rust+Qt adds FFI/build/lifetime overhead, while Rust-native UI choices require more professional workbench work. C#/Avalonia is viable but introduces different scientific native/NumPy and UI-control licensing costs. SwiftUI cannot be the sole Windows UI. JS/TS are useful web/UI clients, not the default numerical storage/compute engine.

## Consequences

C++ receives the lowest safety score among leading routes; RAII does not erase undefined behavior. Require ownership-focused review, no mutable shared data, sanitizer/fuzz/race tests, safe parsing and narrow FFI. Existing numerical/Qt integration reduces binding and deployment work. No claim of C++ runtime superiority over Rust is made.

## Revisit triggers

Measured memory-safety incident cost, an isolated parser/security need, a mature reusable Rust integration, or whole-workload benchmarks may justify a bounded Rust component or a larger change. A migration must preserve command, dataset, project-file and numerical semantics and demonstrate maintainability benefits after FFI/build costs. Do not rewrite for novelty.

## Evidence

[Scientific application source research](../research/scientific-software.md), [graphics/runtime source research](../research/graphics-and-runtime.md), [weighted matrix](../planning/technology-matrix.csv). Compiler probes verify toolchain features only, not this architecture's runtime performance.
