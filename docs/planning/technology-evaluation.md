# Technology Evaluation and Selection

Decision date: 2026-10-30. **First choice: C++20 + Qt 6 Widgets**, with a GUI-independent application/data core, CPU-first owned scientific plotting, and isolated Python/extensions. This is an evidence-based engineering choice, not a measured claim that C++ is faster than Rust. [GUI/platform/license evidence](../research/gui-and-license-evidence.md) and [software research](competitor-research.md) contain verified upstream sources.

## 1. Language versus role

| Language | Useful role and low-level properties | Costs and limitations |
|---|---|---|
| C | Small C ABI boundaries, mature numerical/system libraries, predictable layout and explicit ownership | No built-in RAII/ownership/type-safe generic containers; manual cleanup, buffer and concurrency errors make it a poor primary language for this object-rich desktop |
| C++20/23 | RAII/deterministic destruction, value types, unique ownership, templates/concepts, contiguous containers/span, native SIMD/library ecosystem, Qt/VTK/Eigen/Ceres integration | Undefined behavior and data races remain possible; template/ABI/build complexity and review/sanitizer cost. C++23 library availability varies across AppleClang/MSVC; not baseline |
| Rust | Ownership/borrowing/lifetimes, explicit shared mutability, Send/Sync, traits/monomorphization, Cargo lockfiles and native code | Scientific/desktop integrations can require FFI and unsafe code; borrowing can complicate retained cyclic object models; fewer turnkey professional workbench controls; no stable Rust ABI |
| Swift | Strong Apple UI/AppKit interoperability, ARC/value semantics, useful macOS adapter code | SwiftUI does not solve Windows UI; two presentation implementations multiply feature/accessibility QA; ARC cycles and concurrency integration still need care |
| C# | Productive .NET/XAML with Avalonia, GC memory safety, tasks, native interop, SIMD APIs | GC/allocation latency and native scientific/NumPy boundary costs; core MIT does not cover every advanced UI control; separate runtime/package concerns |
| Python | Scientific ecosystem, notebooks, automation, algorithm prototypes and supported extension packs | Interpreter/GIL and packaging/ABI concerns; Python code is not a sandbox. NumPy kernels are native and can be fast, but bulk UI/data movement still matters |
| JS/TS | Web UI components, approachable extensions and web-facing integrations | WebView DOM/IPC/security surface and platform engine differences; numeric typed arrays can be fast but JS arrays/JSON are poor bulk interchange; not the primary numerical core |

C++ RAII means resource ownership is scoped and exception-safe, not that shared state is race-free. Use `unique_ptr` by default for exclusive ownership and `shared_ptr<const T>` only for explicit immutable shared leases; shared_ptr does not make its pointee thread-safe. Move semantics and spans avoid unnecessary transfers but do not automatically eliminate allocations. Templates/concepts provide checked generic algorithms with compile-time cost. SIMD needs contiguous/aligned data and measured compiler output; an unsafe pointer loop is not inherently faster.

Rust borrows enforce aliasing/lifetime constraints in safe code; `Send` permits ownership transfer across threads, and `Sync` means shared references can cross threads safely. Unsafe FFI must uphold those invariants. `Arc<T>` does not make arbitrary T Send/Sync, and memory safety does not prove scientific correctness, freedom from deadlocks or bounded resource use. Cargo resolves/builds Rust dependencies; native C/C++ dependencies and OS SDK packaging still need coordination. [Official Send/Sync explanation](https://doc.rust-lang.org/book/ch16-04-extensible-concurrency-sync-and-send.html).

C++ using Clang and Rust commonly use LLVM, but frontend semantics, alias analysis, panic/exception policy, allocators, code generation, target features and libraries can produce different code. MSVC uses another compiler toolchain. 'Zero-cost abstraction' means an abstraction need not impose extra runtime cost relative to an equivalent lower-level implementation, not zero cost for the algorithm. Compare identical semantics, numerical precision, safety checks, allocation and threading before attributing performance to language.

## 2. Framework and architecture routes

Qt Widgets has mature docking/MDI/model-view controls and native desktop integration. Qt Quick/QML offers a GPU-oriented declarative scene graph and virtualized editable tables but needs more workbench/docking integration. Neither is automatically a complete scientific spreadsheet/plot engine. C++ may use other UI frameworks; Rust may use Qt through a carefully maintained boundary.

Rust + egui reduces ownership hazards and provides immediate-mode GPU-friendly UI; sophisticated scientific editing, accessibility, docking and broad table behaviors need more product work. Rust + Slint gives compiled declarative UI with multiple rendering backends but control depth and conditional licensing require evaluation. Rust + Tauri uses native WebViews with a web frontend: Windows WebView2 and macOS WKWebView are not one render engine, and large arrays should not be serialized through JSON. C# + Avalonia is a credible workbench alternative, especially for an experienced .NET team; advanced control availability/licensing and scientific native bindings change the economics. SwiftUI cannot meet both initial platforms without a second UI and is excluded as the sole primary framework, not dismissed as useless.

Rust Core + C++/Qt combines memory-safe core code with mature UI, but adds two build systems, FFI lifetime/error/cancellation contracts, binding tests and duplicated type/version management. It is a migration option, not a free improvement. It must preserve the no-Qt domain boundary rather than expose Rust structs through C++ internals.

## 3. Weighted evaluation

Weights reflect a scientific desktop with large heterogeneous data, publication editing, both platforms and long-term maintenance. UI delivery (12), scientific ecosystem (10), data architecture (10) and runtime behavior (10) dominate; platform quality is 8 each; memory/concurrency safety is 8 and maintenance 7. GPU is 6 because it is important but not a prerequisite for the first correct renderer. Python is 6 and plugins 5 because extension workflows are core. Licensing is 4 **plus hard release gates**, not a low-priority issue that can be averaged away. Learning/community/extensibility are 2 each because team skills are unknown and ecosystem size is only a proxy.

Scores are ordinal **engineering assessments**, 1=poor fit/high extra work, 3=viable with material qualification, 5=strong documented fit. They are not measured throughput or probability. Actual runtime performance is unknown and receives the same neutral 3 for every route. Data/GPU scores measure architecture/ecosystem capability, not benchmark speed. Memory, startup and package-size quantities are deliberately **unmeasured for all routes**.

| Criterion | Weight | C++ Widgets | Rust egui | Rust Slint | Rust Tauri | Rust core + Qt | C# Avalonia | C++ Quick |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Actual runtime performance | 10 | 3 | 3 | 3 | 3 | 3 | 3 | 3 |
| Large-data architecture | 10 | 5 | 5 | 5 | 4 | 5 | 4 | 5 |
| UI delivery efficiency | 12 | 5 | 3 | 3 | 4 | 4 | 4 | 4 |
| macOS support | 8 | 4 | 3 | 3 | 3 | 4 | 4 | 4 |
| Windows support | 8 | 5 | 4 | 4 | 4 | 5 | 4 | 5 |
| GPU ecosystem | 6 | 4 | 4 | 4 | 3 | 4 | 4 | 5 |
| Scientific ecosystem | 10 | 5 | 3 | 3 | 3 | 4 | 3 | 5 |
| Python integration | 6 | 5 | 4 | 4 | 4 | 4 | 3 | 5 |
| Plugin integration | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 5 |
| Memory/concurrency safety | 8 | 2 | 5 | 5 | 5 | 4 | 4 | 2 |
| Maintenance cost | 7 | 4 | 3 | 3 | 3 | 2 | 4 | 3 |
| License flexibility | 4 | 4 | 5 | 2 | 5 | 4 | 4 | 4 |
| Learning cost | 2 | 3 | 3 | 3 | 4 | 2 | 4 | 3 |
| Community activity | 2 | 5 | 4 | 3 | 5 | 4 | 4 | 5 |
| Extensibility | 2 | 5 | 4 | 4 | 4 | 5 | 4 | 5 |
| **Weighted / 5** | **100** | **4.27** | **3.73** | **3.59** | **3.73** | **3.92** | **3.74** | **4.14** |
| **Normalized / 100** | | **85.4** | **74.6** | **71.8** | **74.6** | **78.4** | **74.8** | **82.8** |

Formula: `sum(weight * score) / 100`; normalized score is that value times 20. Machine-readable [matrix](technology-matrix.csv) is checked by the planning validator.

### Why the scores differ

- Widgets' built-in workbench/model-view functionality drives its UI score. Quick loses one point for docking/editor integration; egui/Slint require more custom professional controls. Tauri's web ecosystem and Avalonia's workbench options are useful, but not free turnkey parity.
- Qt's documented macOS/Windows support and long scientific desktop use support platform/science scores; macOS still needs native integration/accessibility/high-DPI qualification. Rust GUI platform source/CI exists but has narrower product-specific guarantees; current Avalonia tiers and commercial-control distinctions are in the evidence ledger.
- Rust's safe-language guarantees drive safety scores; mixed C++/Rust retains unsafe boundaries. C++ scores poorly here despite RAII and tooling.
- Hybrid maintenance/learning scores reflect two native stacks and FFI obligations. A proven mixed-language team could revise them upward. No user/team expertise has been assumed.
- Qt licensing needs LGPL compliance; Slint's GPL/commercial/conditional royalty-free terms constrain unrestricted open-source/plugin distribution more. Avalonia's core MIT does not remove control licensing review. A failed legal gate disqualifies a shipped configuration regardless of score.

### Sensitivity and decision gates

The Widgets–Quick margin is only 0.13/5: one point of UI fit (weight 12) almost closes it. The matrix therefore does not prove a universal winner; a dock/table/edit/export prototype could change the recommendation. Widgets beats hybrid by 0.35/5 under current assumptions; stronger Rust team skills, measured C++ safety costs or reusable bindings can change that. Improving one runtime-performance score from 3 to 5 adds only 0.20/5, so a microbenchmark alone cannot justify rewriting the application.

Hard gates are both-platform professional editing/accessibility, export correctness, dependency/license compliance, nonblocking command model and acceptable measured resource behavior. P1 validates desktop controls and packaging; P2 validates data layout/interchange; P3 validates CPU plotting. Reopen ADRs when those experiments contradict the assumptions, not because a language is fashionable.

## 4. Chosen stack and version policy

| Area | Selected direction | Qualification status |
|---|---|---|
| Main language | C++20, extensions off; C ABI for external binary boundaries | C++20 toolchain probe only; no application yet |
| UI | Qt 6 Widgets; candidate **6.12.0**, dynamically linked LGPL route | Exact upstream source/license/support evidence inspected; Qt not installed/tested here |
| OS floor | macOS 14.4+ arm64; Windows 10 22H2 x64 and serviced Windows 11 x64 | Product policy; actual minimum-platform qualification pending |
| Build | CMake >=3.28, presets, Ninja; AppleClang/MSVC 2022 | Local tested-tool versions/results recorded in development guide |
| Dependency manager | vcpkg manifest + pinned baseline and explicit features/triplets | Not bootstrapped/locked yet; don't invent an untested dependency graph |
| Data | Arrow C++ arrays/IPC behind ports; Parquet interchange | Library/format selected; exact release/feature closure gate P2 |
| Numeric | Eigen; Ceres for approved least squares; pocketfft; selected Boost.Math | License evidence established; exact compatible versions and methods need build/numerical qualification |
| Persistence | SQLite container with immutable IPC chunks and versioned metadata | Design selected; no implemented save engine |
| Rendering | Owned scientific scene, QPainter reference, Svg/PDF/PNG adapters | P3; GPU/VTK optional after evidence |
| Scripting | Isolated CPython workers; Python command API, selective worker-local pybind11 | Runtime/package matrix locked at P4/P6, not system-Python dependence |
| AI | Provider adapters + command gateway; optional local llama.cpp worker | P5; model/provider versions explicitly recorded |
| Tests | CTest + GoogleTest for core, Qt Test for UI adapters; pytest for Python/science | Phase 0 uses dependency-free toolchain/planning tests only |
| Project-owned source license | Apache-2.0 policy | Formal copyright/license publication and artifact compliance gate before distribution |

Qt 6.12 is documented as the last Windows 10 branch. Do not casually upgrade to a new major/minor and drop a required platform. Establish the Qt security-update route, OS servicing policy and explicit lifecycle decision first. A 6.8/6.10 fallback is not automatically a longer supported/security-maintained solution. Qt Charts/Graphs/DataVisualization, KDE Frameworks, FFTW and Qt WebEngine are **not** baseline dependencies.

## 5. Alternatives and future migration

If Widgets cannot meet validated interaction/accessibility needs, evaluate a contained Quick canvas or shell change using the same Application API; keep data/command/persistence unchanged. If C++ memory defects or isolated parser risks justify Rust, move a bounded component behind the existing protocol/C ABI after measuring FFI costs. Use Polars when complex lazy query benchmarks beat the current implementation enough to justify a second engine. Use VTK when required 3D/GIS/grid capabilities demand it, without forcing VTK into every 2D plot.

Swift/Objective-C++ stay available for macOS adapters; C# can remain an external automation client; JS/TS can serve future web integrations. None is excluded from useful roles merely because it is not the primary stack. All migrations require correctness, performance, build, license and maintenance evidence.
