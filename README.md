# EveryGraph

EveryGraph is a planned open-source, local-first scientific data analysis, visualization and publication-graphics desktop application for macOS Apple Silicon and Windows x64.

**Current state:** Phase-0 architecture, research, standards and minimal toolchain validation. There is no desktop business application yet. The two supplied feature inventories remain unchanged. All 339 original IDs, 71 release-detail children and 18 additional capabilities are tracked; a populated plan is not a claim of implemented feature parity.

## Decisions

- **C++20 + Qt 6 Widgets**, candidate Qt 6.12.0 with audited LGPL shared-library distribution.
- **Modular monolith**, GUI-independent application/domain contracts and isolated extension workers.
- **Immutable columnar data + DAG**, Arrow interoperability, SQLite project container and explicit provenance/undo.
- **Owned scientific scene + CPU-first QPainter**, with SVG/PDF/PNG and later measured GPU/3D backends.
- **One command API for UI, Python, CLI and AI**, provider-neutral AI with permissions and auditable actions.
- **Apache-2.0 policy for project-owned source**, subject to formal release licensing and dependency obligations.

Initial product qualification targets: macOS 14.4+ arm64, Windows 10 22H2 x64 and serviced Windows 11 x64. These are not tested release claims. Qt's final Windows 10 branch and platform security servicing are explicit risks.

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

[ADR-0001 Language](docs/decisions/ADR-0001-language-selection.md) · [ADR-0002 GUI](docs/decisions/ADR-0002-gui-framework.md) · [ADR-0003 Modules](docs/decisions/ADR-0003-application-architecture.md) · [ADR-0004 Storage](docs/decisions/ADR-0004-data-storage.md)

## Phase-0 checks

With the tools described in the development guide installed, run from this directory:

```sh
python3 tools/validate_planning.py
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug
```

Release/sanitizer and Windows presets are documented separately with their verification status. These commands validate the compiler and planning artifacts only, not a Qt application. Runtime dependency locks, signed app packages, scientific benchmarks and minimum-platform tests remain subsequent gates.

## Preserved inputs and research

- [Origin / OriginPro feature inventory](origin-feature-inventory.md)
- [DataGraph feature inventory](datagraph-feature-inventory.md)
- [Scientific application source evidence](docs/research/scientific-software.md)
- [Graphics/runtime source evidence](docs/research/graphics-and-runtime.md)
- [GUI/platform/license evidence](docs/research/gui-and-license-evidence.md)

Developer documentation, source comments and identifiers use English. The supplied Chinese inventories are preserved research inputs; localized UI and CJK fixtures are permitted. No Git repository or remote was present at inspection, and no application implementation or publication is implied by this documentation.
