# EveryGraph GUI and license evidence

**Research date: 2026-10-30. Scope: architecture research only; no stack selection.**

This document distinguishes **documented facts**, **engineering implications**, and **unmeasured quantities**. “Supported,” “tested,” “can compile,” and “works on an old operating system” are not interchangeable. Platform documentation was inspected on the research date; versioned documentation can still change. Release tags and source commits below identify the inspected artifacts, not a proposed dependency lockfile. GitHub evidence was retrieved with `gh`; other evidence comes from official project/vendor documentation or Eigen’s authoritative GitLab repository. No application was built or benchmarked. License interpretations are engineering compliance cautions, **not legal advice or guarantees**.

## 1. Qt: version-specific desktop support

The actual OS/compiler tables, rather than a generic “Qt supports Windows/macOS” statement, give these combinations:

| Qt documentation | Windows 10 and Windows 11, x64 | Native Apple Silicon and Intel macOS | Windows ARM distinction |
|---|---|---|---|
| [6.8](https://doc.qt.io/qt-6.8/supported-platforms.html) | Windows 10 **1809+**, Windows 11; `x86_64`; **MSVC 2022 or MinGW-w64 13.1** | **macOS 12+**; `arm64`, `x86_64`, `x86_64h`; **Xcode 15 / macOS 14 SDK or higher** | ARM64: MSVC 2022; ARM64EC explicitly unsupported |
| [6.10](https://doc.qt.io/qt-6.10/supported-platforms.html) | Windows 10 **1809+**, Windows 11; `x86_64`; **MSVC 2022 or MinGW-w64 13.1** | **macOS 13+**; the same three architectures; **Xcode 15 / macOS 14 SDK or higher** | ARM64: MSVC 2022; ARM64EC explicitly unsupported |
| [6.12](https://doc.qt.io/qt-6.12/supported-platforms.html) | Windows 10 **1809+**, Windows 11; `x86_64`; **MSVC 2022 or MinGW-w64 15.1** | **macOS 14.4+**; the same three architectures; **Xcode 16 / macOS 15 SDK or higher** | ARM64: MSVC 2022; ARM64EC: MSVC 2026, without installer packages and with module exclusions |

**Important current fact:** the 6.12 page says, “Qt 6.12 will be the last version to support Windows 10.” Its ARM64EC exclusions include WebView, WebEngine, and Quick 3D Physics. These are not exclusions for Windows x64. Apple Silicon means an actual `arm64` build, not an Intel build running through Rosetta.

Qt explicitly warns that individual modules can have narrower support and that patch releases may retire configurations. Build-host SDK requirements differ from deployment targets. Neither an old Qt support row nor Microsoft’s browser-runtime support extends the underlying OS security lifecycle. Qt also supports specified Linux distributions and mobile/embedded configurations; their existence does not imply desktop feature parity.

**Implication:** EveryGraph must choose a minimum macOS version and a Windows 10 maintenance horizon before selecting a Qt branch. Revalidate the exact patch, compiler, architecture, deployment tools, and optional modules together.

## 2. GUI mechanisms and desktop-workbench capabilities

Here, **docking** means rearrangeable, persistent tool/document panels, possibly detachable into OS windows. **MDI** means documents managed within a parent workspace, not merely multiple top-level windows. **Virtualization** means limiting realized/rendered UI elements; it does not automatically provide lazy file loading, bounded model memory, or fast sorting.

### Qt Widgets versus Qt Quick/QML

**Widgets:** C++ retained widget hierarchy, signals/slots, styles and delegates. Ordinary QWidget painting uses QPainter and a backing store; it is not automatically a GPU scene graph or one native OS control per widget. GPU canvases can be integrated deliberately. [QWidget](https://doc.qt.io/qt-6.12/qwidget.html) documents backing-store painting and non-native “alien” widgets.

Widgets supplies the strongest directly documented built-in workbench primitives in this comparison: [QDockWidget](https://doc.qt.io/qt-6.12/qdockwidget.html) with QMainWindow docking/floating/tabification, and [QMdiArea](https://doc.qt.io/qt-6.12/qmdiarea.html) with subwindow/tabbed views, tiling, and cascading. [QTableView/model-view](https://doc.qt.io/qt-6.12/model-view-programming.html) separates storage from delegate painting and can use a lazy model. It is not equivalent to allocating QTableWidget items or persistent QWidget editors for every cell. Spreadsheet selection, formulas, undo, and data-loading policies remain application work.

**Quick/QML:** declarative QML object trees and bindings, JavaScript where used, and C++ integration; rendering goes through the retained [Qt Quick scene graph](https://doc.qt.io/qt-6.12/qtquick-visualcanvas-scenegraph.html). Qt 6’s rendering hardware interface supports Metal, Direct3D, Vulkan, and OpenGL. Documented defaults include Direct3D 11 on Windows and Metal on macOS. A software scene-graph adaptation is distinct from asking an RHI backend to use a software graphics adapter.

[Quick TableView](https://doc.qt.io/qt-6.12/qml-qtquick-tableview.html) creates viewport-needed delegates and reuses them by default. Editing via `editDelegate`/`editTriggers` exists since Qt 6.5: “QML TableView cannot edit” is obsolete. Reused delegates must not accidentally retain another cell’s state. Sorting and persistence require model/application coordination. Quick does not supply QDockWidget/QMdiArea equivalents in its standard control set; plan custom/third-party docking or a Widgets-hosted hybrid. Hybrid rendering, focus, accessibility, and detached-window behavior need prototyping.

### egui / eframe

[egui 0.37.0](https://github.com/emilk/egui/tree/0.37.0) is immediate-mode Rust UI: application code describes the UI during updates; egui produces textured triangles rather than a native control tree. eframe handles integration and offers OpenGL/glow or wgpu rendering. wgpu can use native D3D12, Metal, Vulkan, or supported OpenGL paths; it is not necessarily running in a browser.

The README identifies Windows, macOS, Linux, web, and Android integration. The pinned [CI configuration](https://github.com/emilk/egui/blob/0.37.0/.github/workflows/rust.yml) names Windows x64 and `aarch64-apple-darwin` targets, including dependency checks, Windows checks, and macOS GPU tests. This establishes target attention, **not a Qt-like guarantee covering every Windows 10/11 build**; freeze backend/Rust versions and test both required OS families.

Built-in panels and egui windows are not a full OS-window docking/MDI framework. [egui_dock](https://github.com/Adanos020/egui_dock/tree/cdcdb359af4b74f8cc7921091db7217f0759e4f7) is a separately versioned MIT docking option; compatibility with the selected egui version still needs checking. [TableBody::rows/heterogeneous_rows](https://github.com/emilk/egui/blob/0.37.0/crates/egui_extras/src/table.rs) renders visible rows only. Heterogeneous row heights still require iteration to calculate extent; row virtualization is not evidence of column virtualization or constant-time updates.

### Slint

[Slint 1.18.1](https://github.com/slint-ui/slint/tree/v1.18.1) uses declarative `.slint` definitions, normally compiled ahead of time into host-language code, with retained component state; an interpreter also exists. Window-system backends and renderers are separate choices. Its [renderer documentation](https://github.com/slint-ui/slint/blob/v1.18.1/docs/astro/src/content/docs/guide/backends-and-renderers/backends_and_renderers.mdx) distinguishes software/QPainter, OpenGL FemtoVG, wgpu-based FemtoVG, Skia, and experimental Vello. GPU availability and feature completeness differ; “Slint is always GPU rendered” is false.

Pinned platform guides explicitly test [Windows 10 x86-64 and Windows 11 x86-64/aarch64](https://github.com/slint-ui/slint/blob/v1.18.1/docs/astro/src/content/docs/guide/platforms/desktop/windows/general.mdx), and [macOS 14, 15, and 26 on aarch64](https://github.com/slint-ui/slint/blob/v1.18.1/docs/astro/src/content/docs/guide/platforms/desktop/macos/general.mdx). That Mac table does not establish equivalent current Intel testing. Linux, mobile, web, and embedded backends also exist.

StandardTableView provides column/row models, selection and sorting callbacks; [ListView](https://github.com/slint-ui/slint/blob/v1.18.1/docs/astro/src/content/docs/reference/std-widgets/views/listview.mdx) instantiates visible elements only. The inspected [table reference](https://github.com/slint-ui/slint/blob/v1.18.1/docs/astro/src/content/docs/reference/std-widgets/views/standardtableview.mdx) does not establish full two-axis spreadsheet virtualization. No comparable built-in IDE docking/MDI suite was established from these references; treat it as additional work, not impossibility.

### Tauri

Tauri is a Rust application/window/IPC layer around a **web frontend**, not a native widget toolkit. DOM/CSS layout, JavaScript, canvas/WebGL, and browser compositing depend on the chosen frontend and WebView. Tauri supplies neither spreadsheet virtualization nor document docking: those belong to separately licensed web components or custom code. Multiple WebView windows are not automatically MDI, and cross-window state/drag behavior needs design.

The [WebView documentation](https://v2.tauri.app/reference/webview-versions/) distinguishes **WebView2/Chromium on Windows**, **WKWebView/WebKit on macOS**, and **WebKitGTK on Linux**. Windows runtime installation/update policy, macOS OS-managed WebKit, and Linux distribution packages are different deployment contracts. Tauri 2 also supports mobile, with corresponding system WebViews. GPU acceleration and WebGPU availability cannot be assumed identical across those engines.

Windows 10/11 x64 is a viable target with a supported WebView2 runtime. Microsoft’s [current policy](https://learn.microsoft.com/en-us/deployedge/microsoft-edge-supported-operating-systems) explicitly continues Windows 10 **22H2** Edge/WebView2 updates until **at least October 2028**, without requiring ESU enrollment for those updates. This does not patch Windows itself. Tauri’s WebView page still contains historical Windows 7 wording; Microsoft records support ending at Edge 109, so do not adopt that statement as a current baseline.

Native Apple Silicon and Intel/universal builds are explicit in the pinned [CLI](https://github.com/tauri-apps/tauri/blob/tauri-v2.12.5/crates/tauri-cli/src/build.rs): `aarch64-apple-darwin`, `x86_64-apple-darwin`, `universal-apple-darwin`. Set deployment targets against actual WebKit features and native dependencies, not the age at which WKWebView first existed.

### Avalonia

Avalonia is retained-mode .NET/XAML UI, drawing its own controls rather than wrapping every OS control. Its pinned platform guides document Skia over [ANGLE/Direct3D 11 on Windows](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/docs/platform-specific-guides/windows.md), and [Metal, then OpenGL/software fallback on macOS](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/docs/platform-specific-guides/macos.md). Software fallback and backend selection remain configuration-dependent.

The [current support table](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/docs/supported-platforms.mdx) is materially more specific than “Windows and Mac supported”: Windows 11 **24H2 x64/ARM64 is Tier 1**; Windows 11 22H2 and Windows 10 **22H2 x64 are Tier 2/best effort**. Earlier Windows builds are Tier 3/commercial support. macOS 26 ARM64/x64 is Tier 1; macOS 14/15 ARM64/x64 is Tier 2. Desktop requires .NET 8+. Linux, mobile, and WebAssembly support have separate tiers/runtime requirements.

Current [TableView](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/controls/data-display/structured-data/tableview.md) is **read-only**, virtualizes/recycles rows, and explicitly **does not virtualize columns**. The [DataGrid guide](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/controls/data-display/structured-data/datagrid.md) marks DataGrid deprecated and points advanced editing toward TreeDataGrid. Current [TreeDataGrid documentation](https://github.com/AvaloniaUI/avalonia-docs/blob/a62289076eb7d501a228373a8340982f21e541e8/controls/data-display/structured-data/treedatagrid/index.md) requires a license key; do not extrapolate core Avalonia’s MIT license to it. [Dock](https://github.com/wieslawsoltes/Dock/tree/78da173545501e796f37bc8528e9e55d9cbfd991) is a separate MIT workbench docking system with floating windows; validate document semantics and version compatibility.

### SwiftUI

[SwiftUI](https://developer.apple.com/documentation/swiftui.md) is Apple’s declarative, state-driven UI framework, integrated with AppKit/UIKit. It supports native Apple Silicon macOS applications, **not Windows 10/11 or Linux**. Framework availability starts at macOS 10.15, but [Table](https://developer.apple.com/documentation/swiftui/table.md) requires macOS 12; newer APIs have newer deployment requirements. Other Apple platforms have their own availability annotations.

Rendering combines SwiftUI drawing and platform composition; Metal shader integration exists, but “every control is GPU-only” is unjustified. Apple’s [drawingGroup documentation](https://developer.apple.com/documentation/swiftui/view/drawinggroup(opaque:colormode:).md) explicitly distinguishes SwiftUI-rasterized content from Core Animation-composited controls.

Window/document scenes and split views support document apps, not a documented general-purpose IDE docking or classic MDI suite. AppKit/custom integration may supply missing behavior. Tables offer selection, scrolling and sort descriptors; the application sorts the data. Apple’s [performance session](https://developer.apple.com/videos/play/wwdc2023/10160/) distinguishes lazy row content from eagerly gathered identities, so lazy UI does not imply bounded work for all datasets. A Windows requirement necessitates a second UI or another framework.

## 3. Performance evidence: what remains unknown

**For every candidate above, EveryGraph cold/warm startup time, idle/loaded RSS, peak memory, and compressed/installed package size are unknown. No comparable benchmark was performed.** Vendor adjectives, demo sizes, GitHub stars, and architectural labels do not establish rankings.

Mechanisms suggest test variables, not measurements: Widgets may avoid browser/runtime overhead; Quick, egui and Slint can incur GPU initialization; Tauri usually avoids shipping a complete browser but still uses browser subprocesses; Avalonia packaging changes with framework-dependent/self-contained/AOT deployment; SwiftUI relies on OS frameworks. None establishes a numerical advantage. Qt WebEngine, Python, VTK, models, fonts, debug symbols, and GPU backends can dominate the final bundle.

A useful comparison must use identical functionality and data: document/docking workspace, editable wide table, large plot, file import, and export. Test release builds on Windows 10 22H2 x64, Windows 11 x64, and native Apple Silicon at the chosen minimum macOS version. Record time to first **interactive** window, cold and warm launches, process-tree memory, private/working-set and macOS footprint metrics, GPU memory, frame-time percentiles, input latency, and incremental loading. Report distributions and hardware/driver details, not one run. Include WebView child processes and clean-machine runtime installation costs; separate downloads, installed size, shared prerequisites, and optional components. Test remote desktop, software fallback, accessibility, IME, and high-DPI behavior. Dataset sizes and acceptance thresholds remain decisions for the main agent.

## 4. Release/activity evidence and its limits

These observations demonstrate identifiable recent work, not quality, security, support longevity, or performance:

| Project | Inspected release / source pin | Activity evidence |
|---|---|---|
| Qt | qtbase **v6.12.0**, `025bdad181de241e81bf853c8a2d7bf3d19261a9` | Tagged source commit 2026-09-26; current version-specific support documentation inspected |
| egui | [0.37.0](https://github.com/emilk/egui/releases/tag/0.37.0), `841a328ba2dd8f6996beb7189ebb7fc00e668a2a` | Release published 2026-10-15; platform CI definitions inspected |
| Slint | [v1.18.1](https://github.com/slint-ui/slint/releases/tag/v1.18.1), `372cf0ee5577c3dfec309a45e7b778ba4e81b734` | Release published 2026-09-21 |
| Tauri | [tauri-v2.12.5](https://github.com/tauri-apps/tauri/releases/tag/tauri-v2.12.5), `993038c49185307dfc895ab7657d77ca0d5ed7c7` | Published 2026-10-21; CLI v2.12.4 is a distinct release, not the framework version |
| Avalonia | [12.1.4](https://github.com/AvaloniaUI/Avalonia/releases/tag/12.1.4), `2d79ba55fae1b1e71c69f93a668080c9029e6a31` | Published 2026-10-09; documentation commit `a62289076eb7d501a228373a8340982f21e541e8`, 2026-10-27 |
| SwiftUI | Apple’s [June/September 2026 update record](https://developer.apple.com/documentation/updates/swiftui.md) | June entries explicitly identify Xcode 27+ features; no public framework source commit to pin |

For Qt, additional inspected v6.12.0 pins are qtdeclarative `4332fbbb4c340463dc8cee36dda0c85d932122d8`, qtsvg `883a58532078d003cefef0a549bfe9f885ce41f5`, and qtgraphs `3ef93c85b14a8a759c8c2ad596e2753eb1952666`. Source commit dates are not substituted for release publication dates. Documentation may run ahead of, or describe a different release from, the selected binary.

## 5. GUI licensing: module and product boundaries

### Qt module-by-module check

Both 6.10 and 6.12 module documentation were inspected where available. The following is about the **Qt-authored library code**, not every dependency or tool:

| Modules, each checked | Documented open-source route and caution |
|---|---|
| [Core](https://doc.qt.io/qt-6.12/qtcore-index.html), [Gui](https://doc.qt.io/qt-6.12/qtgui-index.html), [Widgets](https://doc.qt.io/qt-6.12/qtwidgets-index.html) | LGPLv3 available; documentation also lists GPLv2 and commercial licensing |
| [Concurrent](https://doc.qt.io/qt-6.12/qtconcurrent-index.html), [Network](https://doc.qt.io/qt-6.12/qtnetwork-index.html) | LGPLv3 available; Network also lists third-party licenses, including MPL-2.0 data |
| [Svg](https://doc.qt.io/qt-6.12/qtsvg-index.html), [PrintSupport](https://doc.qt.io/qt-6.12/qtprintsupport-index.html) | LGPLv3 available; preserve third-party notices and applicable trademark restrictions |
| [Qml](https://doc.qt.io/qt-6.12/qtqml-index.html), [Quick](https://doc.qt.io/qt-6.12/qtquick-index.html) | LGPLv3 available; **not** the same license scope as every QML-related add-on |
| [Charts, 6.10](https://doc.qt.io/qt-6.10/qtcharts-index.html) | **GPLv3 or commercial; no LGPL option.** Deprecated since 6.10 |
| [Graphs, 6.12](https://doc.qt.io/qt-6.12/qtgraphs-index.html) | **GPLv3 or commercial; no LGPL option**, despite being the replacement recommendation |
| [DataVisualization, 6.10](https://doc.qt.io/qt-6.10/qtdatavisualization-index.html) | **GPLv3 or commercial; no LGPL option.** Deprecated since 6.10 |
| [WebEngine, 6.10](https://doc.qt.io/qt-6.10/qtwebengine-licensing.html) | Qt code offers LGPLv3/GPLv2/GPLv3/commercial alternatives; Chromium and dependencies carry additional licenses, including LGPLv2.1 portions. **Not blanket GPL-only**, and not a single-license dependency |

Representative [qtbase SPDX](https://github.com/qt/qtbase/blob/025bdad181de241e81bf853c8a2d7bf3d19261a9/src/corelib/kernel/qobject.h) explicitly says `LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only`. Inspected Quick and Svg headers agree. [Graphs](https://github.com/qt/qtgraphs/blob/3ef93c85b14a8a759c8c2ad596e2753eb1952666/src/graphs2d/xychart/qxyseries.h), [Charts v6.10.0](https://github.com/qt/qtcharts/blob/v6.10.0/src/charts/qchart.h), and [DataVisualization v6.10.0](https://github.com/qt/qtdatavis3d/blob/v6.10.0/src/datavisualization/engine/q3dscatter.h) explicitly offer commercial **OR GPL-3.0-only**. WebEngine was pinned to **v6.10.3**, `9d3164c951523dfe151cc3799b6a8945de666171`; its [header](https://github.com/qt/qtwebengine/blob/9d3164c951523dfe151cc3799b6a8945de666171/src/core/api/qwebenginepage.h) confirms the alternatives. Its requested 6.12 license page/tag were unavailable; no 6.12 module availability claim is made.

Qt’s [licensing overview](https://doc.qt.io/qt-6.12/licensing.html) also separates tools, documentation, examples, commercial components, and third-party code. A GPL-licensed build tool with an output exception is not automatically a GPL runtime dependency. Audit what is actually redistributed.

### Slint is not simply MIT

The pinned [license overview](https://github.com/slint-ui/slint/blob/v1.18.1/LICENSE.md) offers the framework under **GPL-3.0-only**, the **Slint Royalty-free Desktop, Mobile, and Web Applications License 2.0**, or commercial terms. MIT applies to documentation/examples, not the whole runtime/toolchain.

The [exact royalty-free text](https://github.com/slint-ui/slint/blob/v1.18.1/LICENSES/LicenseRef-Slint-Royalty-free-2.0.md) permits application distribution but requires either the specified accessible AboutSlint widget or a readily discoverable public attribution badge. It excludes embedded systems, standalone redistribution of Slint, and applications exposing Slint APIs in whole or part; notices must remain. This matters for EveryGraph’s possible plugin/scripting APIs and source redistribution. “Free of royalties” is not an unrestricted OSI-style permissive license. The [commercial text](https://github.com/slint-ui/slint/blob/v1.18.1/LICENSES/LicenseRef-Slint-Software-3.0.md), internally version **3.0.5**, adds paid-plan/seat conditions and embedded royalty provisions/exceptions. Obtain review of the actual applicable agreement.

### Other GUI licenses

[egui](https://github.com/emilk/egui/tree/0.37.0), [Tauri](https://github.com/tauri-apps/tauri/tree/tauri-v2.12.5), and wgpu offer **MIT OR Apache-2.0**; retain the chosen license’s required notices. [Avalonia core](https://github.com/AvaloniaUI/Avalonia/blob/12.1.4/licence.md) is **MIT**; commercial controls and separate docking packages require independent review. SwiftUI is an Apple SDK/system framework governed by Apple’s [Xcode/SDK agreements](https://developer.apple.com/support/terms/), not the Swift language’s open-source license. None of these labels licenses third-party fonts, WebViews, frontend components, model weights, or every transitive dependency.

## 6. Numerical, data, embedding, and rendering license ledger

Each link below points to an inspected license at an exact source commit. Dates are **commit dates**, unless labeled release. These are evidence snapshots, not recommendations to ship unreleased branch heads.

| Component / inspected pin | Exact license finding and distribution caution |
|---|---|
| **Eigen 5.0.1**, released 2025-11-11; [COPYING.MPL2](https://gitlab.com/libeigen/eigen/-/blob/bc3b39870ecb690a623a3f49149a358b95c5781d/COPYING.MPL2), [COPYING.README](https://gitlab.com/libeigen/eigen/-/blob/bc3b39870ecb690a623a3f49149a358b95c5781d/COPYING.README) | **MPL-2.0**, with separately identified MPL-compatible third-party code. File-level source obligations apply, including modified covered headers; header-only does not mean obligation-free. Older Eigen/version-specific exceptions and external backends require separate review. |
| **Arrow / C++ Parquet**, 2026-10-30; [LICENSE.txt](https://github.com/apache/arrow/blob/3f1fd722818375c9287f505803d15c101529ea02/LICENSE.txt), [NOTICE.txt](https://github.com/apache/arrow/blob/3f1fd722818375c9287f505803d15c101529ea02/NOTICE.txt) | **Apache-2.0** primary license, plus extensive bundled third-party texts. Keep applicable NOTICE material, license copies and modification notices. Compression, allocators, cloud SDKs and packaging features alter the closure. |
| **Parquet format specification**, [LICENSE](https://github.com/apache/parquet-format/blob/8c91fac200074c47af7ea5f1a87b0511245bb8a9/LICENSE) | **Apache-2.0**. A format is not a blanket license for every Parquet implementation; Arrow C++, Rust implementations and Java libraries are separate artifacts. |
| **Polars**, 2026-10-30; [LICENSE](https://github.com/pola-rs/polars/blob/110354aabfa68bc248e9430564b100554fa586f3/LICENSE) | **MIT**; preserve notices. Optional engines, connectors and dependencies need their own inventory. |
| **FFTW**, 2026-10-17; [COPYRIGHT](https://github.com/FFTW/fftw3/blob/a7b679c528330cf6258253d6dcf5ec90552e6c93/COPYRIGHT), [COPYING](https://github.com/FFTW/fftw3/blob/a7b679c528330cf6258253d6dcf5ec90552e6c93/COPYING) | **GPL-2.0-or-later**, not LGPL or MIT. The grant says version 2 “or … any later version.” [Non-free licensing](https://www.fftw.org/faq/section1.html) is separately available from MIT, the institution—not an MIT-license grant. Exclude from default permissive distribution. |
| **VTK**, 2026-10-30; [Copyright.txt](https://github.com/Kitware/VTK/blob/f0d2b5797ad64ed7104bf309c87db12a98d69d6a/Copyright.txt) | **BSD-3-Clause** primary license: source/binary notices and non-endorsement. ThirdParty code, optional readers/codecs/rendering modules, and their dependencies are not covered by that shorthand. |
| **CPython**, 2026-10-30; [LICENSE](https://github.com/python/cpython/blob/663229430d74dc8f0676ed568abc59fb0171df5a/LICENSE) | **PSF License Version 2**, with retained BeOpen/CNRI/CWI historical agreements and other incorporated-code licenses. Retain the complete license bundle and summarize changes to Python when applicable. Documentation examples have additional PSF/0BSD choices; that is not the entire interpreter’s license. |
| **pybind11**, 2026-10-29; [LICENSE](https://github.com/pybind/pybind11/blob/18bbe9d7c9b0009b992e812e1b59e7a0d4474dbd/LICENSE) | **BSD-3-Clause**. Binding code does not relicense CPython, extension modules, or bundled Python packages. |
| **SQLite**, 2026-10-30; [LICENSE.md](https://github.com/sqlite/sqlite/blob/fdae0f54427d4320f2acdec7edc3e8eb9842dd58/LICENSE.md) | Core/amalgamation **public-domain dedication**, not MIT/Apache. Repository build tooling has BSD-style exceptions. [Official policy](https://sqlite.org/copyright.html) discusses title assurance/jurisdictions; separately sold extensions are not automatically public domain. |
| **Ceres**, 2026-10-20; [LICENSE](https://github.com/ceres-solver/ceres-solver/blob/07a952622c2118433c783b1c3bd3fabe68420fc5/LICENSE) | **BSD-3-Clause** primary license; file-level exceptions/REUSE metadata explicitly exist. Audit selected SuiteSparse, BLAS/LAPACK, Eigen, and accelerator dependencies rather than inferring their licenses. |
| **Boost**, 2026-10-30; [LICENSE_1_0.txt](https://github.com/boostorg/boost/blob/a71862f3c888a011a8a01b4c9c4a665424f50390/LICENSE_1_0.txt) | **BSL-1.0**. License text contains an exception to notice inclusion for solely machine-executable object code. Source redistribution retains notices. Superproject pin alone does not audit every selected submodule/dependency. |
| **pocketfft, C++ branch**, 2026-10-18; [LICENSE.md](https://github.com/mreineck/pocketfft/blob/4d865cba24359b05b37da4e75aee24498c3b78e7/LICENSE.md) | **BSD-3-Clause**, with source/binary notice and non-endorsement conditions. A candidate avoiding FFTW’s GPL licensing constraint; no numerical/performance equivalence is asserted. |
| **wgpu**, 2026-10-29; [MIT](https://github.com/gfx-rs/wgpu/blob/49325ca0a13bdc0787b1601138096cce743dd29e/LICENSE.MIT), [Apache](https://github.com/gfx-rs/wgpu/blob/49325ca0a13bdc0787b1601138096cce743dd29e/LICENSE.APACHE) | **MIT OR Apache-2.0**. Backend SDKs, shader tooling, translation layers and native dependencies need separate review. |
| **llama.cpp**, 2026-10-30; [LICENSE](https://github.com/ggml-org/llama.cpp/blob/889f2ec96deacbc2d8a9a67dbe181eaa43a2fb3b/LICENSE) | **MIT** for the project code; model weights, tokenizers, datasets and accelerator redistributables have independent terms. An MIT inference engine does not make downloaded weights MIT. |

Eigen’s old GitHub mirror stopped in 2019 and is not evidence of upstream inactivity. The authoritative GitLab upstream had commit [`3561d16a4e632e7545564ddecb734d90bc5e5f46`](https://gitlab.com/libeigen/eigen/-/commit/3561d16a4e632e7545564ddecb734d90bc5e5f46) on 2026-10-30. Recent library commits above establish activity only; a small mature library need not publish frequently to remain useful.

## 7. Compatibility cautions for an Apache-2.0 EveryGraph

1. **Separate authorship from distribution.** EveryGraph-owned files can use Apache-2.0 while dependencies retain their licenses. Do not label the entire installer “Apache-2.0 only.” Maintain exact-version dependency/source inventories, license texts, applicable NOTICE files, modifications, and optional-feature provenance. Apache-2.0 includes patent provisions and redistribution conditions, not merely attribution.

2. **Prefer a deliberate LGPL Qt shared-library route if selected.** The inspected [LGPL-3.0 text](https://github.com/qt/qtbase/blob/025bdad181de241e81bf853c8a2d7bf3d19261a9/LICENSES/LGPL-3.0-only.txt) §4 permits combined works under chosen terms subject to conditions. Provide prominent Qt notices and GPLv3/LGPLv3 texts; comply with corresponding-source obligations for distributed Qt, including modifications and necessary build material. Preserve recipients’ ability to modify the library and reverse-engineer for debugging those modifications. A suitable §4(d)(1) shared-library mechanism must actually work with an interface-compatible modified Qt—not just have a `.dll`/`.dylib` filename.

3. **Test replacement and installation, not just linking.** Signing, library validation, packaging, updater behavior, and store terms can obstruct replacement. Assess §4(e)/GPLv3 installation-information requirements where applicable. Static linking is not categorically forbidden, but §4(d)(0) demands an appropriate relinking/source/object-code route and adds compliance work. Apache source availability alone is not proof of a reproducible, usable replacement path. Commercial Qt remains a separately reviewed alternative.

4. **Keep Eigen’s MPL boundary explicit.** MPL §3.1–3.4 requires covered source, including changes, to remain available under MPL with notices and recipient access information. A larger work may use other terms. Keep Eigen changes in identifiable covered files, provide the exact corresponding source, and avoid copying covered implementation into apparently Apache-only files. Unmodified covered source availability also matters when distributing executable forms; “publish only our modifications” is incomplete.

5. **No bundled GPL-only libraries by default.** Gate FFTW, Qt Charts/Graphs/DataVisualization and other GPL-only runtime additions behind explicit architecture/licensing approval or a suitable commercial grant. [Apache’s compatibility explanation](https://www.apache.org/licenses/GPL-compatibility.html) allows Apache-2.0 code in a GPLv3 combination, not GPL code redistributed as Apache-only. FFTW’s later-version option can matter, but does not remove copyleft. A separate process is not an automatic legal exemption.

6. **Audit the shipped closure and distribution channel.** Include plugins, Python wheels, codecs, generated runtime support, fonts, datasets, model assets, and WebView/SDK redistribution terms. Recheck Slint API-exposure/embedded restrictions and Avalonia commercial controls before designing extensions around them. Have qualified counsel review the concrete artifacts and channel. These cautions do not certify compatibility or replace the applicable texts.

## 8. Decision boundary and remaining limitations

The main agent should choose the stack after weighing minimum OS policy, spreadsheet editing/width, docking, accessibility, plotting/export, team skills, and compliance cost. This research establishes capability mechanisms and licensing boundaries, not production fitness. No complete transitive SBOM, store-contract review, accessibility audit, security assessment, or benchmark was performed. Some official pages moved, denied automated retrieval, or lacked a requested release; authoritative source files were used where available and gaps are stated above. No absent control was proven impossible to build. Revalidate live documentation and every shipped artifact before release.
