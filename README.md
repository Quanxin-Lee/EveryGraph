# EveryGraph

EveryGraph is a planned open-source, local-first scientific data analysis, visualization and publication-graphics desktop application for macOS Apple Silicon, with Windows x64 as a deferred later target.

**Current state:** Phase-0 architecture, research, standards and minimal toolchain validation. There is no desktop business application yet. The two supplied feature inventories remain unchanged. All 339 original IDs, 71 release-detail children and 18 additional capabilities are tracked; a populated plan is not a claim of implemented feature parity.

## Decisions

- **C++20 + Qt 6 Widgets**, candidate Qt 6.12.0 with audited LGPL shared-library distribution.
- **Modular monolith**, GUI-independent application/domain contracts and isolated extension workers.
- **Immutable columnar data + DAG**, Arrow interoperability, SQLite project container and explicit provenance/undo.
- **Owned scientific scene + CPU-first QPainter**, with SVG/PDF/PNG and later measured GPU/3D backends.
- **One command API for UI, Python, CLI and AI**, provider-neutral AI with permissions and auditable actions.
- **Apache-2.0 policy for project-owned source**, subject to formal release licensing and dependency obligations.

Qualification sequencing ([ADR-0005](docs/decisions/ADR-0005-platform-sequencing.md)): macOS 14.4+ arm64 is the only active qualification target for the current phase. Windows 10 22H2 x64 and serviced Windows 11 x64 are deferred later targets; their platform interfaces remain in the architecture, but no Windows implementation, evidence, packaging or support claim is authorized until a Windows x64 host or authorized CI qualifies it. No qualification target is a tested release claim. Qt's final Windows 10 branch and platform security servicing are explicit risks for the later Windows gate.

## Start here

1. [Agent and coding standards](AGENTS.md)
2. [Vision and boundaries](docs/planning/project-vision.md)
3. [Full functional scope](docs/planning/functional-scope.md) and [requirement register](docs/planning/feature-traceability.csv)
4. [Technology evaluation](docs/planning/technology-evaluation.md) and [architecture research](docs/planning/competitor-research.md)
5. [Architecture overview](docs/architecture/architecture-overview.md) and [module/command contracts](docs/architecture/module-design.md)
6. [Roadmap and acceptance gates](docs/planning/development-roadmap.md)
7. [Development commands and verified status](docs/engineering/development.md)
8. [Licensing](docs/engineering/licensing-and-dependencies.md) and [performance/numerical validation](docs/engineering/performance-validation.md)
9. [Validation report and open gates](docs/engineering/validation-report.md)

## Architecture documents

[Data model](docs/architecture/data-model.md) · [Rendering](docs/architecture/rendering-architecture.md) · [Concurrency/dataflow](docs/architecture/concurrency-model.md) · [AI](docs/architecture/ai-architecture.md) · [Plugins](docs/architecture/plugin-architecture.md) · [Project format](docs/architecture/project-file-format.md)

[ADR-0001 Language](docs/decisions/ADR-0001-language-selection.md) · [ADR-0002 GUI](docs/decisions/ADR-0002-gui-framework.md) · [ADR-0003 Modules](docs/decisions/ADR-0003-application-architecture.md) · [ADR-0004 Storage](docs/decisions/ADR-0004-data-storage.md) · [ADR-0005 Platform sequencing](docs/decisions/ADR-0005-platform-sequencing.md) · [ADR-0006 macOS P1 entry gate](docs/decisions/ADR-0006-macos-p1-entry-gate.md)

## Phase-0 checks

With the tools described in the development guide installed, run from this directory:

```sh
python3 tools/validate_planning.py
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug
```

Release/sanitizer presets are documented separately with their verification status; the Windows presets exist but are deferred and unexecuted under ADR-0005. These commands validate the compiler and planning artifacts only, not a Qt application. Runtime dependency locks, signed app packages, scientific benchmarks and minimum-platform tests remain subsequent gates.

## Preserved inputs and research

- [Origin / OriginPro feature inventory](origin-feature-inventory.md)
- [DataGraph feature inventory](datagraph-feature-inventory.md)
- [Scientific application source evidence](docs/research/scientific-software.md)
- [Graphics/runtime source evidence](docs/research/graphics-and-runtime.md)
- [GUI/platform/license evidence](docs/research/gui-and-license-evidence.md)

Developer documentation, source comments and identifiers use English. The supplied Chinese inventories are preserved research inputs; localized UI and CJK fixtures are permitted. No Git repository or remote was present at inspection, and no application implementation or publication is implied by this documentation.
