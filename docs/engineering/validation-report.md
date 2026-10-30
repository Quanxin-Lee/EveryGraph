# Phase-0 Validation Report

Review date: 2026-10-30. This report covers delivered planning/configuration artifacts, not an implemented desktop application.

## 1. Initial workspace and preservation

Initial inspection found only `origin-feature-inventory.md` and `datagraph-feature-inventory.md`. There was no source code, AGENTS, README, build configuration, Git repository or prior stack decision in the workspace. Both inventories remain byte-for-byte preserved:

| Input | Original IDs | SHA-256 |
|---|---:|---|
| Origin inventory | 215 | `fd22138468ea7798ea858c86d5217a2bcd0685ff38f5622ae62fae29db9d0ff8` |
| DataGraph inventory | 124 | `f5e6876c38cbc90505546ae19dd7786ed45b3a4e9779032de19ad8f6b9ad62fb` |

The requirement register adds 71 explicit release children and 18 EveryGraph additions: **428 tracked rows**. Duplicate concepts retain source IDs without claiming separate implementations. Catalog counts such as 100+ graphs or nearly 200 fit functions remain explicit decomposition gates, not invented complete catalogs. No capability is marked implemented.

## 2. Evidence and design review

Research uses official documentation, actual build/interface/implementation/license files and immutable source pins. Detailed evidence classes and unresolved facts appear in the three research documents. Closed-source Origin/Igor/DataGraph internals are not inferred from their interfaces. Research commits are not production dependency locks.

Main-review spot checks retrieved the pinned Qt QObject header, LabPlot CMake file and ParaView's VTK submodule reference; the latter matches the cited VTK revision. These checks supplement the research retrievals; they are not a fresh exhaustive HTTP audit of every link or a source-build test. Source-specific version discrepancies and unavailable pages are retained in the research documents.

The design separates compile-time dependency diagrams from runtime flows; keeps Qt/provider/GPU types outside application/domain contracts; routes UI/Python/AI through one gateway; preserves original data independently of LOD; defines short revision commits, cancellation and recovery durability; and leaves actual untrusted-code containment and license compliance as explicit release gates.

A separate full-document consistency review identified three concrete contract gaps, all corrected before final checks: (1) document commit versus terminal external-effect completion, including cancellation/revocation and partial receipts; (2) Save As applying unsaved state to the temporary destination without modifying the original; and (3) worksheet/mirror row-domain correspondence under sorting, insertion, deletion and unmatched rows. This is design review evidence, not executed implementation testing.

## 3. Executed and unexecuted checks

| Check | Current evidence |
|---|---|
| C++20 macOS Debug/Release | Compiler probe configured, compiled and passed in both modes on macOS 27.2 arm64 |
| macOS ASan + UBSan | Compiler probe passed; compile/link instrumentation verified; no diagnostics in that run |
| Sanitizer incompatibility guard | ASan/UBSan plus TSan rejected as intended; normal preset restored |
| Validator unit tests | Six tests passed for valid/invalid coverage, source mismatch, delivery claims, phase/classification, matrix bounds, links/fences/language |
| Full planning checker | Passed after this report was created: preserved inputs, 428 trace rows, local links, source anchors, language/fences and matrix totals |
| Full CTest including planning/unit tests | macos-debug, macos-release and macos-sanitize each configured/built and passed 3/3 tests: toolchain_probe, planning_validation, planning_validator_tests |
| Windows presets | Real configuration exists; not executed on Windows |
| GitHub Actions | Local pinned/least-privilege workflow exists; no remote repository or workflow execution |
| Qt/Arrow/Eigen/SQLite application integration | Not installed, linked or tested by the compiler-only scaffold |
| Minimum macOS 14.4 / Windows 10/11 runtime | Not tested; deployment target and documented support do not prove runtime compatibility |
| Clang-format / Clang-Tidy / TSan | Configurations/policy exist; tools or runtime checks not executed |
| Mermaid | Diagrams authored for the contracts; no renderer execution claimed |
| Scientific correctness/performance | Test protocol supplied; no application measurement or numerical result claimed |

The validator checks preserved hashes, original-ID coverage, unique IDs, source titles/line anchors, phase/status/delivery fields, local Markdown targets, balanced fences, English normative documents and weighted totals. It does not prove semantic feature completeness, remote URL availability, arbitrary Markdown syntax, numerical correctness or diagram rendering.

## 4. Required next gates

- Qualify Qt 6.12.0 and the complete dependency/feature/CRT graph on both platforms; establish a real immutable package baseline.
- Decide Windows 10/Qt maintenance horizon and qualify exact minimum/current OS builds.
- Configure a real Git/GitHub repository only when authorized, then run CI; current ARM larger-runner configuration needs availability/billing approval or an approved alternative.
- Decompose unnamed feature/method/format catalogs and close proprietary SDK/license/file/language compatibility questions.
- Implement and validate actual numerical/rendering/storage contracts, including reference results, large-data workloads and interrupted-save recovery.
- Prove OS sandbox denial behavior before offering untrusted plugins; otherwise clearly restrict execution to trusted code.
- Complete Apache publication rights, Qt replacement/signing compliance, transitive SBOM and model/font/codec/license reviews.

These are next-stage implementation/release gates, not hidden successes. The Phase-0 deliverable establishes the decisions, scope and testable contracts needed to address them.

## 5. Delivered files

37 source/configuration/document deliverables, including 26 Markdown documents, two CSV registers and seven Mermaid diagrams. Generated build outputs and OS metadata are excluded from this count.

### Root standards and configuration

- [AGENTS.md](../../AGENTS.md)
- [README.md](../../README.md)
- [CMakeLists.txt](../../CMakeLists.txt)
- [CMakePresets.json](../../CMakePresets.json)
- [.gitignore](../../.gitignore)
- [.clang-format](../../.clang-format)
- [.clang-tidy](../../.clang-tidy)
- [Phase-0 CI](../../.github/workflows/phase0.yml)

### Planning

- [project-vision.md](../planning/project-vision.md)
- [technology-evaluation.md](../planning/technology-evaluation.md)
- [technology-matrix.csv](../planning/technology-matrix.csv)
- [competitor-research.md](../planning/competitor-research.md)
- [development-roadmap.md](../planning/development-roadmap.md)
- [functional-scope.md](../planning/functional-scope.md)
- [feature-traceability.csv](../planning/feature-traceability.csv)

### Architecture

- [architecture-overview.md](../architecture/architecture-overview.md)
- [module-design.md](../architecture/module-design.md)
- [data-model.md](../architecture/data-model.md)
- [rendering-architecture.md](../architecture/rendering-architecture.md)
- [concurrency-model.md](../architecture/concurrency-model.md)
- [ai-architecture.md](../architecture/ai-architecture.md)
- [plugin-architecture.md](../architecture/plugin-architecture.md)
- [project-file-format.md](../architecture/project-file-format.md)

### Decisions

- [ADR-0001-language-selection.md](../decisions/ADR-0001-language-selection.md)
- [ADR-0002-gui-framework.md](../decisions/ADR-0002-gui-framework.md)
- [ADR-0003-application-architecture.md](../decisions/ADR-0003-application-architecture.md)
- [ADR-0004-data-storage.md](../decisions/ADR-0004-data-storage.md)

### Research, engineering and checks

- [scientific-software.md](../research/scientific-software.md)
- [graphics-and-runtime.md](../research/graphics-and-runtime.md)
- [gui-and-license-evidence.md](../research/gui-and-license-evidence.md)
- [development.md](development.md)
- [licensing-and-dependencies.md](licensing-and-dependencies.md)
- [performance-validation.md](performance-validation.md)
- [validation-report.md](validation-report.md)
- [validate_planning.py](../../tools/validate_planning.py)
- [test_validate_planning.py](../../tools/test_validate_planning.py)
- [toolchain_probe.cpp](../../tools/toolchain_probe.cpp)

Generated binaries and CMake state stay in ignored `out/build/`; no Git repository, commits, remote publication, desktop app or signed installer was created.
