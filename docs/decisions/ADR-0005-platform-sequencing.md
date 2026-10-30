# ADR-0005: macOS-First Platform Sequencing with Deferred Windows Qualification

- Status: accepted sequencing decision for Phase 0 and P1 planning; no Windows implementation, evidence or support claim.
- Date: 2026-10-31.
- Decision owners: project owner and technical lead.

## Context

The long-term scope requires the same core workflows on macOS Apple Silicon and Windows x64 with explicit OS adapters ([functional scope](../planning/functional-scope.md), EG-ADD-016). The current phase has one qualified development environment: macOS 14.4+ on Apple Silicon with AppleClang, Qt 6.12 and the declared CMake/Ninja toolchain. No Windows x64 host or authorized Windows CI is available, so MSVC builds, Windows file/credential/process semantics, installer signing and packaging cannot be verified. Running both platforms in parallel would produce borrowed or fabricated evidence and split the limited qualification effort. On 2026-10-31 the project owner decided that Windows adaptation is paused, only platform interfaces remain in the lower layers, and macOS Apple Silicon is adapted first.

## Decision

1. macOS 14.4+ on Apple Silicon is the only active qualification target for the current P0/P1 phase. All acceptance evidence, packaging and support statements in this phase are macOS-scoped.
2. Windows 10 22H2 x64 and vendor-serviced Windows 11 x64 remain required later targets. They are deferred, not removed. The platform ports (`FileAccess`, `SecretStore`, `ProcessHost`, `ResourceLimiter`, `SystemIntegration`, `DisplayCapabilities`) and their interfaces remain in the architecture; domain and application public interfaces must not expose platform-specific types.
3. No Windows implementation, build execution, reused macOS evidence, packaging, signing or support claim is authorized in the current phase. Windows tasks stay tracked as deferred until a Windows x64 host or authorized CI is available.
4. The P1 exit gate is macOS-only. Windows replication tasks P1-002 and P1-010 are a later gate and do not block the macOS P1 exit P1-013.
5. The 428-row feature traceability register and its requirement rows are unchanged. Sequencing governs when a requirement is evidenced, not whether it is required.
6. No other minimum platform version, technology choice or module contract changes. The governance and architecture statements updated by this decision are AGENTS.md section 1 targets and section 2 platform line, the functional-scope P1 row, UI gate and platform sequencing paragraph, and the module-design platform ports and test statement.

## Alternatives

- Continue macOS and Windows in parallel. Rejected for the current phase because no Windows x64 host or authorized CI is available, Windows evidence would be unverifiable, and the macOS qualification chain would lose focus. It becomes the default again when the Windows gate resumes.
- Drop Windows permanently. Rejected because the preserved inventories and EG-ADD-016 require Windows x64 workflows; dropping it would silently change approved scope and traceability and create avoidable portability debt.
- Defer Windows and remove the platform ports. Rejected because platform-specific code would leak into domain or application layers and a later port would require invasive refactoring. Keeping narrow ports is the lower-cost option, though it does not by itself prove portability.

## Consequences

- Deferred evidence: MSVC builds, Windows Unicode/long-path behavior, case-insensitivity, file-sharing/delete semantics, Credential Manager, Job Objects/sandboxing, Windows packaging and installer signing remain unverified and must not be claimed.
- Portability debt: macOS-only assumptions can still leak into shared code. The interfaces remain, but a later Windows port needs a portability audit and a real Windows test run.
- Documentation: status and planning documents must describe Windows as deferred. macOS evidence cannot be presented as cross-platform evidence, and no vendor or operating-system support matrix may imply Windows qualification.
- Positive: the current phase has one verifiable target and one acceptance chain, so P1 evidence can be reproduced and reviewed on the available hardware.

## Revisit triggers

Resume the Windows gate when a Windows x64 host or authorized Windows CI becomes available, when a maintainer with Windows qualification is assigned, before any Windows support claim or cross-platform release, before any statement that the application runs on Windows, or when a shared platform-port change makes the deferred interfaces stale. Resuming requires the same probe, shell and data tests on Windows; recorded MSVC, Windows SDK, CMake, Ninja and Qt versions; separate evidence; and no reuse of macOS results. This ADR must be superseded by a new decision before any Windows support claim.

## Evidence

[AGENTS.md](../../AGENTS.md) platform targets; [functional scope](../planning/functional-scope.md) P1 sequencing and UI gate; [module contracts](../architecture/module-design.md) platform ports; [task board](../planning/task-board.csv) deferred Windows rows and the macOS P1 exit; planning validator `python3 tools/validate_planning.py`.
