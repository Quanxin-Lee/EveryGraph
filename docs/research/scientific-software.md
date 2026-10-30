# Scientific desktop software: architectural evidence

**Research date: 2026-10-30.** Scope: evidence for planning EveryGraph, a professional open-source scientific desktop targeting native macOS Apple Silicon and Windows x64. This document does not select EveryGraph's architecture or implement application code.

## 1. Evidence method and revision boundaries

The following labels distinguish different strengths of evidence throughout:

- **Fact — documented:** an explicit statement in an official manual or public interface description, verified by retrieving that source. This is not an independently tested runtime result.
- **Observation — source:** a declaration, implementation, build setting, or license notice actually inspected at the stated commit. A declaration establishes an interface, not every implementation property.
- **Inference:** a potentially transferable lesson, not a claim that EveryGraph should adopt the corresponding technology.
- **Unverified:** an unanswered question, unsupported interpretation, or behavior requiring builds, experiments, or further legal review.

GitHub metadata, trees, and file contents were accessed through `gh api`. Source references below use immutable, full commit SHAs; official web manuals are live documents retrieved on the research date. Neither repository push times nor default-branch commit dates are treated as release announcements. The preserved Origin inventory supplied a programming-page discovery URL, not evidence for its claims; the official pages were checked independently.

### Inspected source baselines

| Project | Ref and observed commit | Scope and version qualification |
|---|---|---|
| LabPlot | `2.12.1`: `c666870ec037d33502b82f51b6bda0f24538810a` | Principal object-model and numerical-source baseline. Its root CMake file actually declares `2.12.0`; the tag/build-version discrepancy is preserved, not silently corrected. |
| LabPlot | `2.11.1`: `b187116f4e17966f88ca92f6a9bfed6b24fb1335` | Historical build comparison only. |
| LabPlot | observed `HEAD`: `8ad518df113e1a1d4b2254de1d4dd3afb2c05be7` | Development build declares `2.12.80`; inspected to avoid describing C++17 as a timeless requirement. |
| ParaView | `v6.2.0`: `fbc86500975a664d629883ccc78be0bd377507ff` | `version.txt` declares `6.2.0`. |
| VTK | `v9.7.1`: `a970ec452229b1a0953684c00ecd9fe612778b8a` | Also the exact VTK gitlink returned for the inspected ParaView tag, rather than an unrelated VTK HEAD. |
| QGIS | observed `HEAD`: `4595d764ccf8149604def9a249232f9c2a9f608a` | CMake declares `4.3.0`, release name `Master`: a development snapshot, not evidence of a released QGIS 4.3. |

Build/version evidence: [LabPlot tagged build][L-build], [historical build][L-old], [development build][L-head], [ParaView version][P-version], [ParaView VTK gitlink][P-vtk], and [QGIS build][Q-build]. QGIS cookbook citations intentionally use the versioned **3.40** manual; source observations below belong to the newer pinned snapshot. Origin and Igor have no inspected private source revision.

## 2. LabPlot: document objects, scientific columns, and analysis curves

### Observation — source: build technology is version-specific

| Inspected baseline | Language/build declarations | Qt and KDE declarations |
|---|---|---|
| `2.11.1` | C++17; CMake ≥3.17 | Searches Qt6 then Qt5; Qt5 minimum `5.12.0`, KF5 minimum `5.32.0`. Its Qt6 branch uses Qt minimum `6.2.2` and KF minimum `5.240.0`, with fallback to Qt5 if the framework discovery fails. |
| `2.12.1` tag | C++17, C99; extensions disabled; CMake ≥3.17 | Explicit Qt6 minimum `6.5.0`; KF6/ECM minimum `6.3.0`. |
| Development `2.12.80` | C++20, C99; CMake ≥3.17 | Qt6 minimum `6.5.0`; KF6/ECM minimum `6.5.0`; a separate Qt6GuiPrivate lookup for Qt ≥6.10. |

These are actual source requirements, not guesses from a KDE affiliation. The tagged build requires Qt Core, Concurrent, Gui, PrintSupport, Sql, Widgets, and Test, plus private GUI headers. KDE dependencies include Archive, Config, I18n, KIO, XmlGui, and other frameworks. `KDEInstallDirs`, `KDECMakeSettings`, and `KDECompilerSettings` participate in the build. Consequently, “a Qt application” understates its integration and packaging dependencies. [L-build][L-build] [L-old][L-old] [L-head][L-head]

The tagged Flatpak manifest specifies `org.kde.Platform` runtime `6.8`, but also identifies itself as **nightly**. That is a packaging-track observation, not the Qt patch version of a released Windows or macOS binary. CI includes Qt6 Craft templates for Windows x86-64 and macOS x86-64/arm64; this verifies configured targets, not successful builds or native-runtime performance. [Packaging and CI][L-package] [CI source][L-ci]

### Observation — source: composition, not a single spreadsheet class

`src/backend/core/AbstractAspect.h` defines a `QObject`-derived base, an `AspectType` enumeration, `parentAspect()`, child insertion/removal, and `undoStack()`. This is concrete evidence of a shared document-object tree and editing infrastructure. `Project` derives from `Folder`, overrides `undoStack()`, and exposes XML save/load methods. It is not merely the visible main window. [AbstractAspect][L-aspect] [Project][L-project]

`Workbook : AbstractPart` groups **Spreadsheet and Matrix** objects; `Workbook.cpp` explicitly describes that role, restricts paste types accordingly, and constructs those children during loading. A LabPlot **Worksheet** is different: `Worksheet : AbstractPart` exposes `QGraphicsScene* scene()` and manages graphical content. Therefore, a workbook tab is not interchangeable with a worksheet drawing page. The official workbook manual independently describes the multi-tab grouping. [Workbook][L-workbook] [Worksheet][L-worksheet] [Workbook manual][L-workbook-manual]

`Spreadsheet : AbstractDataSource` provides the tabular container, while `Column : AbstractColumn` holds scientific column behavior. `AbstractColumn::ColumnMode` includes Double, Integer (`qint32`), BigInt (`qint64`), Text, DateTime, Month, and Day. These are deliberately typed modes, not a universal string-cell representation. The public column APIs expose typed access/replacement, formulas, labels, and serialization. This observation does not establish arbitrary mixed types within one column. [Spreadsheet][L-sheet] [Column][L-column] [Column modes][L-modes]

Plotting is a second hierarchy: `CartesianPlot : AbstractPlot`, `XYCurve : Plot`, and `XYAnalysisCurve : XYCurve`. `CartesianPlot` owns plot configuration rather than numerical storage; curves refer to data and render it. `XYAnalysisCurve` exposes source-curve selection, `recalculate()`, source-change notifications, and persistence. `Axis : WorksheetElement` handles ranges, ticks, and labels; `CartesianCoordinateSystem : AbstractCoordinateSystem` supplies `mapLogicalToScene()` and `mapSceneToLogical()`. `Worksheet`'s graphics scene and these plot classes establish substantial Qt graphics coupling inside directories named `backend`; that name should not be read as “GUI-independent numerical kernel.” [CartesianPlot][L-plot] [XYCurve][L-curve] [XYAnalysisCurve][L-analysis] [Axis][L-axis] [Coordinate system][L-coordinate]

### Observation — source: numerical integration and extension boundaries

The root build requires GSL and conditionally discovers FFTW3, Eigen3, libcerf, and Cantor. `src/CMakeLists.txt` builds static `labplotnsllib`, `labplotbackendlib`, and `labplotlib`, with explicit numerical-source lists and links. ROOT-related configuration concerns an importer and compression dependencies; it is not evidence that CERN ROOT is LabPlot's general analysis engine. [L-build][L-build] [Library targets][L-targets]

The evidence extends beyond dependency names: `XYFitCurve.cpp` calls `gsl_multifit_wlinear` and GSL statistics/BLAS functions. `nsl_dft_transform()` uses FFTW under `HAVE_FFTW3` and GSL FFT otherwise. Its FFTW branch explicitly ignores `stride`; the alternative accepts it. That difference illustrates why interchangeable numerical backends require behavioral tests, not just successful linking. [Fit implementation][L-fit] [DFT implementation][L-dft]

`Notebook::init()` instantiates the KDE part `kf6/parts/cantorpart`, obtains `Cantor::WorksheetAccessInterface`, connects session/model signals, and translates suitable variables into Columns. This is a verified **Cantor integration boundary**, including Cantor plugins; it does not establish a general LabPlot plugin ABI for arbitrary plot types or analysis algorithms. [Notebook implementation][L-notebook]

### Fact — documented; inference; unverified

**Fact — documented:** the live notebook manual describes different integration mechanisms: external executables for some systems, embedded interpreters for R/Python/Julia, and platform-specific availability. The live C++ SDK page describes embedding LabPlot functionality in C++ applications; its concepts page warns that not all internal or GUI components are public SDK API. Neither page promises a general runtime plugin system. These live pages, despite a “2.12” manual banner, must not be assumed to describe precisely every capability in the `2.12.1` tag. [Notebook manual][L-notebook-manual] [SDK][L-sdk] [SDK concepts][L-sdk-concepts]

**Inference:** useful patterns include persistent object identity, typed columns distinct from their editors, explicit analysis-result objects, and numerical adapters with equivalent-semantics tests. **Non-transferable constraints:** the aspect hierarchy's Qt/graphics dependencies, KDE packaging footprint, Cantor interpreter/version coupling, and GPL-covered code cannot be treated as free architectural shortcuts. **Unverified:** stable third-party plugin ABI, SDK binary compatibility, full headless separation, and the exact Qt patch versions shipped on either EveryGraph target.

## 3. ParaView and VTK: typed data, execution, control, and presentation

### Fact — documented; observation — source: data and execution are separate

The official ParaView data guide distinguishes structured, polygonal, unstructured, and tabular data and associated arrays. The VTK book explains source/filter/mapper networks and demand-driven updates; it is conceptual documentation, not a specification of every current implementation detail. [Data guide][P-data-manual] [VTK book][V-book]

At the pinned VTK revision, `vtkDataObject` is the general data-object abstraction. `vtkDataSet` adds geometry/topology and point/cell attributes. `vtkImageData` represents a regular image/volume structure, `vtkPolyData` vertices/lines/polygons/strips, and `vtkUnstructuredGrid` arbitrary cell combinations. `vtkTable` is a separate tabular data object with column arrays and row-count invariants, not simply another mesh. `vtkDataArray` defines numeric arrays with native-type subclasses and tuple/component access. [DataObject][V-object] [DataSet][V-dataset] [ImageData][V-image] [PolyData][V-poly] [UnstructuredGrid][V-grid] [Table][V-table] [DataArray][V-array]

`vtkAlgorithm` supplies input/output ports, `SetInputConnection()`, `FillInputPortInformation()`, `ProcessRequest()`, and update entry points. `vtkExecutive` controls execution/data flow; algorithm implementation and execution policy are distinct. The inspected `vtkAlgorithm::Update(int)` delegates to its executive, while `CreateDefaultExecutive()` returns `vtkCompositeDataPipeline` unless a prototype overrides it. Describing the default simply as `vtkStreamingDemandDrivenPipeline` would miss this implementation detail. The streaming executive separately documents partial updates by pieces/sub-extents. [Algorithm interface][V-algorithm] [Algorithm implementation][V-algorithm-impl] [Executive][V-executive] [Streaming executive][V-streaming]

This is not proof that every filter is streaming, parallel, or compatible with every dataset. `vtkPolyData` explicitly documents filter-specific cell-type restrictions. Typed connections permit validation; they do not remove semantic constraints or automatically provide out-of-core execution.

### Observation — source: server manager is an application control layer

ParaView adds a layer above VTK's algorithms. `Remoting/ServerManager/vtkSMProxy.h` describes a proxy managing one or more VTK objects through properties; `UpdateVTKObjects()` pushes changes. Composite proxies and exposed subproxy properties are supported. `vtkSMProperty` defines commands, domains of acceptable values, information-only properties, and checked versus unchecked values. The latter distinction allows prospective UI edits and validation without immediately committing values to execution state. [Proxy][P-proxy] [Property][P-property]

A concrete declarative example is `Remoting/Application/Resources/filters_filterscore.xml`: the `PolylineLength` `SourceProxy` names `vtkAppendArcLength`, binds `Input` to `SetInputConnection`, and constrains its `DataTypeDomain` to `vtkPolyData`. This is executable-facing metadata connecting UI-visible identity, a processing class, and input validation—not merely menu configuration. [Proxy XML][P-xml]

`Qt/Core/pqApplicationCore.h` describes initializing the application core after `QApplication`, then using facilities including `pqObjectBuilder` and `pqUndoStack` to work with ServerManager. Qt is the client presentation/application layer, not VTK's dataset definition. The inspected CMake Qt selector supports Auto/5/6 and prefers Qt6 in Auto mode. MPI and Python are explicit build options, so their availability must not be inferred from the product name alone. [Qt client][P-qt-client] [Qt selector][P-qt-build] [Build options][P-options]

### Fact — documented; observation — source: distributed rendering and real plugins

The official parallel-visualization manual distinguishes **client**, **data server**, and **render server** roles. Roles can share a process; `pvserver` commonly combines data processing and rendering. MPI partitions work across processes. Remote rendering returns rendered images; local rendering requires delivering renderable geometry to the client. Partition boundaries and ghost cells matter to algorithm correctness. This is more precise than saying the client never contains data or that remote visualization simply streams the whole dataset. [Parallel architecture][P-parallel]

`vtkPVRenderView` contains remote-rendering and LOD thresholds and per-process rendering coordination. VTK separately supplies renderer/window abstractions and concrete rendering implementations: `vtkOpenGLPolyDataMapper` explicitly uses OpenGL; `Rendering/OpenGL2/CMakeLists.txt` selects platform/headless paths including Cocoa, Win32, EGL, and OSMesa-related window classes. These inspected paths do not establish that all VTK rendering uses OpenGL or that native Metal rendering is supplied. [Render view][P-render] [Render window][V-window] [OpenGL mapper][V-gl] [Rendering build][V-render-build]

Unlike an inferred extension mechanism, ParaView's plugin system is directly documented and represented in source. `vtkPVPlugin` exposes plugin version and client/server requirements; `paraview_add_plugin()` accepts `SERVER_MANAGER_XML`, `UI_INTERFACES`, modules, and required-side flags. The official Plugin Howto distinguishes algorithm/server plugins from GUI/client plugins and requires matching the host ParaView version. A plugin may need installation on both sides. [Plugin interface][P-plugin] [Plugin build API][P-plugin-build] [Plugin Howto][P-plugin-manual]

**Inference:** separate typed payloads, algorithm contracts, execution policy, application properties, and representations. Declarative properties can support validation and multiple front ends. **Non-transferable constraints:** distributed proxy lifecycles, MPI deployment, mesh topology, compositing, and native-plugin version coupling impose costs unnecessary for many desktop plots. **Unverified:** target-specific graphics reliability, vector-export fidelity, responsiveness on large tabular workloads, and any advantage over a simpler desktop execution model. No benchmark or rendering experiment was performed.

## 4. QGIS: reusable libraries and multiple kinds of extension

### Observation — source: core/gui/app is a build boundary

`src/CMakeLists.txt` separately adds core, analysis, providers, GUI, application, Python, server, and processing-command components under build conditions. Their own build files define `qgis_core`, `qgis_gui`, and `qgis_app`; the desktop executable links application code, which in turn links core and GUI. This is concrete reuse structure, but not proof that core has no Qt or presentation-adjacent types. The pinned development build requires C++20, Qt6 ≥6.4.0, SIP, and PyQt6 when the relevant bindings are enabled. It must not be described using the Qt5/PyQt5 stack of the 3.40 cookbook. [Q-build][Q-build] [Component build][Q-components] [Core target][Q-core] [GUI target][Q-gui] [App target][Q-app]

`python/CMakeLists.txt` generates and builds `qgis._core` against `qgis_core` and `qgis._gui` against core plus GUI. SIP annotations in public headers cover ownership transfer, subclass conversion, exceptions, and GIL handling; examples include `SIP_TRANSFER`, `SIP_FACTORY`, and `SIP_HOLDGIL` in processing APIs. Python integration is therefore a deliberately maintained binding contract, not automatic exposure of all C++ internals. [Bindings build][Q-bindings] [Algorithm bindings][Q-algorithm]

### Observation — source: distinguish three extension responsibilities

1. **Data providers:** `QgsProviderRegistry` associates provider keys and metadata with data access, exposing `createProvider()` and `providerMetadata()`. This is different from a registry of analysis algorithms. [Data-provider registry][Q-data-provider]
2. **Processing providers:** `QgsProcessingProvider::loadAlgorithms()` groups/registers algorithms through `addAlgorithm()`. `QgsProcessingRegistry` registers providers and resolves algorithm IDs. Its implementation calls `provider->load()`, takes QObject parent ownership, and unloads/deletes providers on removal. Lifecycle and ownership are part of the contract. [Processing provider][Q-processing-provider] [Registry implementation][Q-registry]
3. **Application plugins:** `python/utils.py` imports plugin packages, calls `classFactory(iface)`, starts `initGui()`, and calls `unload()` during removal. A plugin can install processing functionality without making the processing abstraction identical to a GUI plugin. [Plugin lifecycle][Q-plugins]

`QgsProcessingAlgorithm` declares parameter/output definitions, `initAlgorithm()`, execution methods, context, and feedback. Its documentation separates main-thread preparation/post-processing from `runPrepared()` execution and warns against reusing an already-run instance. This is stronger evidence for execution isolation than merely observing an asynchronous progress bar. It is not permission to use arbitrary GUI objects in worker threads. [Algorithm contract][Q-algorithm]

### Fact — documented; inference; unverified

**Fact — documented:** the versioned cookbook explains standalone application initialization, Python plugin metadata/lifecycle, and processing providers. `qgisMinimumVersion` belongs to the plugin compatibility declaration; examples register and remove processing providers. [Cookbook introduction][Q-manual] [Plugin cookbook][Q-plugin-manual] [Processing cookbook][Q-processing-manual]

**Inference:** data access, reusable algorithms, and desktop customization deserve separate contracts. Binding ownership and execution phases should be designed explicitly. **Non-transferable constraints:** GIS coordinate systems, provider ecosystems, Qt/Python distribution, and a large application-specific API are not a ready-made general scientific desktop framework. **Unverified:** broad binary compatibility across QGIS versions or parity between the historical cookbook and every current binding. The inspected in-process loading is not evidence of a sandbox for untrusted plugins.

## 5. Origin: public automation architecture, not private implementation

### Fact — documented

Origin C is **not full modern C++**. The official compatibility FAQ says it supports nearly complete ANSI C syntax with minor exceptions. The Basic Features guide calls it closely based on ANSI C, with selected C++ features including classes, references, overloads, default arguments, and mid-stream declarations, plus selected C# constructs. “C-compatible language with extensions and limitations” is safer than claiming either strict complete ANSI compliance or ISO C++17/20 support. [Compatibility FAQ][O-c-faq] [Language guide][O-c]

The guide documents Code Builder's editor, compiler, linker, and debugger, and calling suitable compiled Origin C functions from LabTalk. That establishes an extension-development workflow—not the compiler technology, machine-code format, or language used throughout Origin's private engine. LabTalk is the application's scripting interface for commands, object properties, and automation. The official programming page also documents embedded Python. [O-c][O-c] [Programming overview][O-programming]

Public objects provide a useful interaction model. The Origin C workbook guide identifies **`WorksheetPage` as the class for a workbook**, containing Worksheets containing Columns; it does not introduce a separate intermediate `Workbook` object. The graph guide describes `GraphPage` containing `GraphLayer` objects and `Project.GraphPages` iteration. LabTalk exposes page, worksheet, and layer objects. The Python reference lists wrappers including `WBook`, `WSheet`, `GPage`, `GLayer`, and `Plot`. These are interface-level views of documents and plots, not evidence that the private storage hierarchy has identical classes or ownership. [Workbook guide][O-workbook] [Graph guide][O-graphs] [LabTalk objects][O-objects] [Python wrappers][O-python-classes]

External Python is a distinct deployment path: official examples use `originpro`; the OriginExt guide explicitly states that OriginExt is built on Origin's COM/Automation interface and documents a 64-bit Origin 2019b-or-later requirement for that guide. The COM manual describes clients launching Origin, exchanging worksheet/matrix data, sending commands, retrieving graph images, and saving projects. This is application automation, not evidence of a portable, independently redistributable analysis engine. [External Python examples][O-python-examples] [OriginExt][O-originext] [COM manual][O-com]

### Inference and unverified boundaries

**Inference:** a discoverable workbook/layer/plot object vocabulary and consistent scripting/GUI operations can make repeatable science approachable. Separate external automation from embedded scripting, including lifecycle, error propagation, and data-transfer costs.

**Non-transferable constraints:** Windows COM coupling, installed-product requirements, vendor-specific languages, and proprietary numerical components cannot become native Apple Silicon capabilities simply by adding Python wrappers. **Unverified:** Origin's internal C++ standard, GUI toolkit, dependency scheduler, memory representation, rendering engine, plugin ABI, and private numerical architecture. No source, build, or license files from Origin's proprietary application were inspected; public wrapper documentation does not disclose those internals. No redistributability inference is made from API availability.

## 6. Igor Pro: waves, procedures, and a native extension SDK

### Fact — documented

The Waves manual defines waves as named data objects with types and metadata, independent of how they are displayed. Waves support one to four dimensions, numeric and text data, real/complex numeric forms, floating-point precisions, and signed/unsigned integer types. Dimension scaling represents uniformly sampled coordinates, for example `x = x0 + p*dx`; irregular sampling uses separate X/Y waves. Units, dimension labels, and a wave note travel with the data. Tables display/edit waves rather than serving as their sole storage identity. [Waves manual][I-waves] [Tables manual][I-tables]

The same manual distinguishes assignment from dependency formulas: `:=` retains an expression and updates the dependent wave when referenced objects change. It also documents important numerical semantics: NaN cannot be represented in integer waves, and operations differ in their handling of nonfinite values. These are public behavior contracts, not evidence for a particular internal dependency-graph implementation. [I-waves][I-waves]

Procedures are authored in procedure windows, including an experiment-associated main procedure and reusable included `.ipf` files. The official page says user-defined functions are compiled for speed, with parameters and return values. It also documents structures, menu definitions, and GUI callbacks. This does not establish LLVM, a JIT, native machine-code compilation, or equivalence to C++. [Procedures][I-procedures]

The XOP Toolkit is an explicit C/C++ extension SDK containing `XOPSupport`, examples, and a manual. Extensions can add operations, functions, menus, dialogs, and windows. Toolkit 7 documents Igor ≥6.20; Toolkit 8 documents Igor ≥8.00 and long-name support. Those are toolkit-page compatibility statements, not universal compatibility guarantees. [Toolkit][I-xop]

A published sample is unusually concrete: `XOPMain()` initializes the interface and sets `XOPEntry`; `RegisterOperation()` registers an operation; `WavePoints()`, `WaveType()`, and `WaveData()` access wave data; `WaveHandleModified()` tells Igor to update displays. This exposes an extension's lifecycle and mutation notification responsibility without exposing the host's complete internals. The sample also warns about raw-data-pointer/heap behavior. [Official XOP sample][I-xop-example]

### Platform qualification, inference, and unknowns

**Fact — documented:** the retrieved Igor Pro 10 announcement describes a **64-bit Windows x64 application**, Windows 10 or later, Qt6, and no macOS execution except Windows virtual environments; XOPs must also be 64-bit. A separate, older-context ARM article describes Igor 9 running through Rosetta2 and says there was no concrete native ARM-port plan at that writing. Its “currently shipping” wording is not carried forward as a 2026 release claim. Neither page establishes a native Apple Silicon Igor 10. [Igor 10 announcement][I-platform] [ARM discussion][I-arm]

**Inference:** keep scientific arrays and sampling metadata independent of tables/plots, and make external mutation notifications explicit. **Non-transferable constraints:** raw SDK pointers, host-specific object handles, binary architecture matching, and commercially supplied SDK terms require separate evaluation. **Unverified:** Igor's private storage engine, renderer internals, compiler implementation, scheduling architecture, or freely redistributable XOPSupport licensing. The SDK page offers purchase/licensed-user downloads; that is not an open-source grant.

## 7. Cross-project constraints and remaining verification

### Observation — source: licensing must be file-aware

LabPlot's inspected aspect, curve, and DFT files carry `GPL-2.0-or-later`; the repository includes the corresponding license text and other licenses. ParaView and VTK `Copyright.txt` files contain three-clause BSD terms; inspected VTK `vtkTable.h` additionally carries a Sandia-specific license identifier. Neither project's top-level license eliminates third-party/component review. [LabPlot license][L-license] [ParaView license][P-license] [VTK license][V-license] [V-table][V-table]

QGIS `COPYING` includes GPLv2 and a Qt linking exception. Inspected processing headers say GPLv2-or-later, whereas Python build metadata emits `GPL-2.0-Only` in one packaging branch. Preserve these observations rather than collapsing them into an unsupported universal licensing conclusion. Any reuse needs a selected-file and dependency audit; architectural learning is not the same decision as copying or linking code. [QGIS license][Q-license] [Q-bindings][Q-bindings]

### Inference: lessons available without selecting a stack

The strongest common patterns are **data separate from presentation**, **explicit analysis/operation contracts**, **application-level object identity**, and **documented extension lifecycle**. The implementations differ materially: LabPlot document aspects, VTK execution networks, QGIS provider registries, Origin automation objects, and Igor waves are not interchangeable abstractions. A document tree need not also be an execution graph; a computational backend need not be a dynamically loaded plugin.

### Unverified: work deliberately not performed

No application was installed, built, run, or benchmarked. CI status, signed binaries, notarization, dependency architectures, Qt patch versions in distributed applications, plugin security, numerical equivalence, accessibility, export quality, and file-format round trips remain untested. Several historical manual URLs returned 404; successful replacement URLs are cited instead. Live manuals can drift from pinned code, especially LabPlot SDK pages and QGIS examples. Closed-source internals remain unknown. Architecture choices and subsequent target-platform experiments belong to the main planning effort.

## Sources

All sources below were retrieved or inspected on **2026-10-30**. GitHub source links are pinned to observed commits; manual links are live unless a documentation version is embedded in the URL.

[L-build]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/CMakeLists.txt
[L-old]: https://github.com/KDE/labplot/blob/b187116f4e17966f88ca92f6a9bfed6b24fb1335/CMakeLists.txt
[L-head]: https://github.com/KDE/labplot/blob/8ad518df113e1a1d4b2254de1d4dd3afb2c05be7/CMakeLists.txt
[L-package]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/.flatpak-manifest.json
[L-ci]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/.gitlab-ci.yml
[L-aspect]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/core/AbstractAspect.h
[L-project]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/core/Project.h
[L-workbook]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/core/Workbook.cpp
[L-worksheet]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/Worksheet.h
[L-workbook-manual]: https://docs.labplot.org/en/data_containers/data_containers_workbook.html
[L-sheet]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/spreadsheet/Spreadsheet.h
[L-column]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/core/column/Column.h
[L-modes]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/core/AbstractColumn.h
[L-plot]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/CartesianPlot.h
[L-curve]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/XYCurve.h
[L-analysis]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h
[L-axis]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/Axis.h
[L-coordinate]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h
[L-targets]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/CMakeLists.txt
[L-fit]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/worksheet/plots/cartesian/XYFitCurve.cpp
[L-dft]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/nsl/nsl_dft.c
[L-notebook]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/src/backend/notebook/Notebook.cpp
[L-notebook-manual]: https://docs.labplot.org/en/computational_notebooks.html
[L-sdk]: https://docs.labplot.org/en/sdk/cpp.html
[L-sdk-concepts]: https://docs.labplot.org/en/sdk/concepts.html
[L-license]: https://github.com/KDE/labplot/blob/c666870ec037d33502b82f51b6bda0f24538810a/LICENSES/GPL-2.0-or-later.txt
[P-version]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/version.txt
[P-vtk]: https://api.github.com/repos/Kitware/ParaView/contents/VTK?ref=fbc86500975a664d629883ccc78be0bd377507ff
[P-data-manual]: https://docs.paraview.org/en/latest/UsersGuide/understandingData.html
[P-proxy]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Remoting/ServerManager/vtkSMProxy.h
[P-property]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Remoting/ServerManager/vtkSMProperty.h
[P-xml]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Remoting/Application/Resources/filters_filterscore.xml
[P-qt-client]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Qt/Core/pqApplicationCore.h
[P-qt-build]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/CMake/paraviewQt.cmake
[P-options]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/CMake/ParaViewOptions.cmake
[P-parallel]: https://docs.paraview.org/en/latest/ReferenceManual/parallelDataVisualization.html
[P-render]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Remoting/Views/vtkPVRenderView.h
[P-plugin]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Remoting/Core/vtkPVPlugin.h
[P-plugin-build]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/CMake/ParaViewPlugin.cmake
[P-plugin-manual]: https://www.paraview.org/paraview-docs/latest/cxx/PluginHowto.html
[P-license]: https://github.com/Kitware/ParaView/blob/fbc86500975a664d629883ccc78be0bd377507ff/Copyright.txt
[V-book]: https://book.vtk.org/en/latest/VTKBook/04Chapter4.html
[V-object]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkDataObject.h
[V-dataset]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkDataSet.h
[V-image]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkImageData.h
[V-poly]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkPolyData.h
[V-grid]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkUnstructuredGrid.h
[V-table]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/DataModel/vtkTable.h
[V-array]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/Core/vtkDataArray.h
[V-algorithm]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/ExecutionModel/vtkAlgorithm.h
[V-algorithm-impl]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/ExecutionModel/vtkAlgorithm.cxx
[V-executive]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/ExecutionModel/vtkExecutive.h
[V-streaming]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Common/ExecutionModel/vtkStreamingDemandDrivenPipeline.h
[V-window]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Rendering/Core/vtkRenderWindow.h
[V-gl]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Rendering/OpenGL2/vtkOpenGLPolyDataMapper.h
[V-render-build]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Rendering/OpenGL2/CMakeLists.txt
[V-license]: https://github.com/Kitware/VTK/blob/a970ec452229b1a0953684c00ecd9fe612778b8a/Copyright.txt
[Q-build]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/CMakeLists.txt
[Q-components]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/CMakeLists.txt
[Q-core]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/core/CMakeLists.txt
[Q-gui]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/gui/CMakeLists.txt
[Q-app]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/app/CMakeLists.txt
[Q-bindings]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/python/CMakeLists.txt
[Q-algorithm]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/core/processing/qgsprocessingalgorithm.h
[Q-data-provider]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/core/providers/qgsproviderregistry.h
[Q-processing-provider]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/core/processing/qgsprocessingprovider.h
[Q-registry]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/src/core/processing/qgsprocessingregistry.cpp
[Q-plugins]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/python/utils.py
[Q-manual]: https://docs.qgis.org/3.40/en/docs/pyqgis_developer_cookbook/intro.html
[Q-plugin-manual]: https://docs.qgis.org/3.40/en/docs/pyqgis_developer_cookbook/plugins/plugins.html
[Q-processing-manual]: https://docs.qgis.org/3.40/en/docs/pyqgis_developer_cookbook/processing.html
[Q-license]: https://github.com/qgis/QGIS/blob/4595d764ccf8149604def9a249232f9c2a9f608a/COPYING
[O-c-faq]: https://docs.originlab.com/quick-help/ansi-c-compatibility/
[O-c]: https://docs.originlab.com/originc/guide/basic-features/
[O-programming]: https://www.originlab.com/index.aspx?go=Products/Origin/Programming
[O-workbook]: https://docs.originlab.com/originc/guide/workbooks-worksheets-and-worksheet-columns/
[O-graphs]: https://docs.originlab.com/originc/guide/graphs/
[O-objects]: https://docs.originlab.com/labtalk/tutorials/tutorial-objs/
[O-python-classes]: https://docs.originlab.com/originpro/annotated.html
[O-python-examples]: https://docs.originlab.com/externalpython/external-python-code-samples/
[O-originext]: https://docs.originlab.com/externalpython/originext/
[O-com]: https://docs.originlab.com/com/
[I-waves]: https://docs.wavemetrics.com/igorpro/igor-basics/waves
[I-tables]: https://docs.wavemetrics.com/igorpro/igor-basics/tables
[I-procedures]: https://www.wavemetrics.com/products/igorpro/programming/procedures
[I-xop]: https://www.wavemetrics.com/products/xoptoolkit
[I-xop-example]: https://www.wavemetrics.com/products/xoptoolkit/examples
[I-platform]: https://www.wavemetrics.com/news/igor-pro-10-released
[I-arm]: https://www.wavemetrics.com/news/igor-and-apple-arm-processors
