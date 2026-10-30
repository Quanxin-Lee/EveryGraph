# Comparative Architecture Research

Evidence date: 2026-10-30. This synthesis is backed by the detailed, source-pinned studies below; it does not claim runtime testing of competitors.

## Evidence and source map

- [Scientific software](../research/scientific-software.md): LabPlot, ParaView/VTK, QGIS, Origin and Igor Pro; build files, object models, numerical calls, extension interfaces and official manuals.
- [Graphics and runtime](../research/graphics-and-runtime.md): Blender, GIMP/GEGL, napari/npe2, Zed/GPUI and llama.cpp; concrete backend/execution/lifecycle source and licenses.
- [GUI and license evidence](../research/gui-and-license-evidence.md): framework release/source snapshots, platform tables, controls and exact license texts.
- Preserved [Origin inventory](../../origin-feature-inventory.md) and [DataGraph inventory](../../datagraph-feature-inventory.md): public feature observations, version and provenance caveats. They are not independent architecture evidence.

**Evidence classes:** official documented fact; source-verified observation at a pinned commit; architectural inference; unknown/unverified. A README claim, build option, inspected implementation and successful runtime test are different evidence. No private source or unobserved benchmark is inferred.

## Transfer decisions

| Project | Verified architectural mechanism | Adopt in EveryGraph | Do not assume/copy |
|---|---|---|---|
| Origin / OriginPro | Public workbook/worksheet/column and page/layer/plot APIs; LabTalk, limited Origin C, Python and COM automation | Coherent scientific object vocabulary, repeatable commands, linked results and templates | Private GUI/render/storage/scheduler internals; ISO modern C++ compatibility of Origin C; portable COM runtime |
| DataGraph | Public data/variables/drawing-command workflow; commands expose results; editable graph groups and animation variables | Declarative GraphSpec, safe expressions, linked scalar/column results, immediate interactive editing | Private language, GPU backend, storage or DAG implementation; complete 5.5 details from 5.4 manual |
| Igor Pro | Typed 1–4D Waves with scaling/units/notes, procedure language, documented C/C++ XOP entrypoints | Dataset identity independent of table, sampled-axis metadata, explicit extension lifecycle | Native Apple Silicon availability, freely reusable SDK, private compiler or zero-copy implementation |
| LabPlot | AbstractAspect document hierarchy, typed Columns, Spreadsheet versus graphical Worksheet, XYAnalysisCurve, numerical libraries | Object identity, analysis-result persistence, native scientific UX, build-level modularity | A GUI-free 'backend'; general plugin ABI inferred from Cantor; GPL implementation reuse under Apache-only terms |
| ParaView / VTK | Typed data objects, algorithms/executives, proxy properties, client/server roles and real plugin API | Typed node ports, executor/policy separation, validated metadata and explicit representation layer | MPI/remote proxies in first release, automatic streaming of every filter, a full 2D publication editor |
| Blender | Dependency invalidation/evaluation distinct from GPU backend and shader contracts; Python runtime | Dirty propagation, modular rendering resources, clear CPU/GPU lifetimes | Full Blender scene/shader framework, universal parallelism or safe Python sandbox |
| QGIS | Separate core/gui/app targets, SIP bindings, distinct data/processing/application providers | Reusable algorithm registry, lifecycle-aware Python API, declarative parameters | Qt-free core, stable universal binary plugin ABI or isolated plugin execution |
| GIMP / GEGL | GIMP plugin processes; GEGL in-process operations, required/invalidated regions, tile/cache/spill layers | Fine-grained invalidation, bounded residency, distinction between graph and process isolation | GEGL node = external process; process isolation = adversarial sandbox |
| napari | Python model/layers, Qt frontend, VisPy visuals, array capabilities and npe2 contribution lifecycle | Separate model/view/render contracts, lazy materialization boundary, declarative plugin discovery | Universal zero-copy, always-async slicing or sandboxed Python imports |
| Zed / GPUI | Rust workspace, entities/events, foreground/background executors, native platform renderers | Explicit state ownership, responsiveness discipline and modular boundaries | GPUI UI triangles = scientific plotting/export engine; all platforms use the same graphics API |
| llama.cpp | C/C++ inference API, GGML device backends and separate model artifacts | Optional model worker and backend-neutral AI provider interface | MIT runtime licenses every model; all GPU backends support identical cancellation |

## Important corrections to common simplifications

1. LabPlot's inspected 2.12.1 tag uses C++17, while its inspected development branch uses C++20. Its graphical **Worksheet** is not EveryGraph's proposed tabular Worksheet; its **Spreadsheet** is the tabular container. Copying names without semantics would create a flawed model.
2. Origin C is C-compatible with selected extensions and limitations, not proof of Origin's internal language or ISO C++20 support.
3. Qt Widgets painting is not automatically GPU accelerated; Qt Quick uses a separate scene graph. The language decision and UI toolkit decision are independent.
4. Source presence, CI configuration, release artifacts and successful platform qualification are distinct. Research pins and exact platform caveats are retained in the studies, including Igor and Zed version differences.
5. GIMP's plugin process, GEGL's operation graph, ParaView's distributed roles and Blender's dependency graph solve different problems. EveryGraph needs their boundaries, not their combined infrastructure.
6. Permissive framework components (for example GPUI) do not make surrounding GPL application code permissive. Architectural ideas can be studied without copying covered code.

## DataGraph-specific evidence limits

The preserved DataGraph inventory records 5.5 build 68, a bundled 5.4 manual, product-page-only 5.5 items, resource-only database/cloud entries and an inaccessible change-log page. Those distinctions remain active. The product's public workflow motivates EveryGraph variables, graph-command ordering, result extraction, interpolation/masks and animation. It does **not** establish how DataGraph internally schedules dependencies or renders. No unsupported claims about its implementation language, thread model or private format are made.

## What the research proves—and does not

The studies identify useful architectural contracts and concrete dependency/license/platform constraints. They do not establish EveryGraph performance, numerical equivalence, accessibility, proprietary-file compatibility, sandbox security or release readiness. Those require the [benchmark plan](../engineering/performance-validation.md), [roadmap gates](development-roadmap.md), format corpora and platform experiments. Upstream source pins are research evidence, not automatic production dependency choices.
