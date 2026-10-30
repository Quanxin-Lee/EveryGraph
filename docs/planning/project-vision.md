# EveryGraph Project Vision

Decision baseline: 2026-10-30. EveryGraph is an open-source, local-first professional scientific data analysis, visualization and editable publication-graphics desktop application. It is not a generic dashboard builder, an AI chat shell, a clone of a competitor's UI, or a distributed service platform.

## Users and outcomes

Researchers, experimental scientists, engineers and analysts need to import heterogeneous measurements, preserve scientific meaning, explore data interactively, apply validated numerical methods, create editable publication figures and repeat workflows without hidden manual steps. Expert users need Python and native/external-engine integration; non-programmers need discoverable commands, inspectors and guided analysis. AI assists these same workflows with transparent, permission-controlled actions.

Success means trustworthy numbers and figures, reproducible provenance, responsive interaction, cross-platform fidelity, recoverable projects and maintainable open interfaces—not a language benchmark or a feature-count slogan. UI appearance and publication styles are independent. Accessibility, keyboard use, Unicode/CJK text, locale-aware imports and international scientific workflows are first-class requirements.

## Platforms

Initial qualification targets are **macOS 14.4+ on Apple Silicon**, **Windows 10 22H2 x64**, and **Windows 11 x64 on vendor-serviced releases**. These are chosen product floors, not claims of completed testing. The candidate Qt baseline is 6.12.0: its official matrix includes Windows 10 1809+ and macOS 14.4+, but EveryGraph deliberately narrows Windows 10 to 22H2. Qt 6.12 is documented as the last branch supporting Windows 10. The OS's own security servicing and Qt patch availability require a maintained lifecycle policy; dependency support does not make an unpatched OS safe.

P1 must qualify an actual Windows 11 release/build and minimum macOS image with all selected libraries; Windows 11 24H2 is an initial test candidate, not an indefinite supported-version promise. Test lowest supported and current serviced releases. macOS Intel, Windows ARM64 and Linux remain later qualification targets; their code paths should not be architecturally prohibited. Keep compiler/SDK build-host requirements distinct from deployment floors. See [official platform evidence](../research/gui-and-license-evidence.md).

## Full functional scope

The current instruction adopts every functional capability in the two preserved inventories as a long-term requirement, including OriginPro-level analysis, 3D, GIS, images, reports, animation, external integrations and DataGraph's variables/command-driven workflow. [Functional scope](functional-scope.md) and [traceability](feature-traceability.csv) track 339 source IDs, 71 release-detail children and 18 EveryGraph additions.

These are requirements, not implemented capabilities. Some source rows describe overlapping features, platform metadata or unmeasured vendor claims. Some families omit exact catalog membership; those remain explicit discovery gates. Functional equivalence, file interoperability, source-language compatibility and native-plugin ABI compatibility have separate acceptance meanings. Unknown proprietary rights/specifications cannot be converted into unsupported implementation promises. No claim of complete parity is allowed while those gates are open.

## Additional differentiation

- One versioned command API for UI, Python, CLI and AI, with permission checks, undo and execution history.
- Immutable data revisions, explicit units/null/mask policies and reproducible analysis DAGs.
- Provider-neutral AI with local-only enforcement and approved data transfer.
- Isolated, capability-controlled extension execution, without pretending process separation alone is a sandbox.
- Consistent scientific scene semantics across interactive display and publication export.
- Recoverable versioned project files, headless replay and published numerical/performance evidence.

## Product and delivery boundaries

P1–P3 establish a dependable desktop/data/2D foundation; they are not the full parity release. P4, P6 and P7 carry advanced analysis, automation, compatibility and domain engines. The **full-capability milestone** requires all accepted source capabilities and additional requirements to pass their tests or receive an explicit user-approved scope change. A required capability delivered as an optional installable pack must still be supported, documented and tested; 'optional packaging' is not permission to omit it.

Remote/distributed execution, live collaborative editing and plugin marketplace hosting are future infrastructure choices, not launch prerequisites. No cloud account, model service or paid numerical runtime is required for the core product. Commercial engines are user-authorized integrations, not baseline dependencies.

## Assumptions and governance

Assume a small mixed systems/UI team; no specific staffing, budget or delivery date has been supplied. Roadmap estimates are planning ranges, not commitments. Architecture decisions can be superseded through ADRs with evidence and migration consequences. Dependency licensing is assessed for the shipped artifact closure. Source and developer documentation are English; user communication and localized UI can be Chinese. Existing Chinese feature inventories remain preserved research inputs.
