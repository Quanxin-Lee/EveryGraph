# ADR-0002: Use Qt 6 Widgets for the Desktop Workspace

- Status: accepted for Phase 0; exact binary/platform qualification pending.
- Date: 2026-10-30.

## Context

Dock panels, multiple documents/windows, editable large tables, menus, keyboard/IME/accessibility, scientific canvas editing and reliable export dominate the initial desktop workload. Language choice does not dictate UI choice. [Framework evidence](../research/gui-and-license-evidence.md) verifies supported platforms, mechanisms, controls, versions and module licenses.

## Decision

Use Qt 6 Widgets with a custom virtualized table model and owned scientific plot canvas. Candidate source baseline is **Qt 6.12.0**, LGPLv3 shared-library distribution, with Core/Gui/Widgets and only justified Concurrent/Network/Svg/PrintSupport dependencies. No Qt Charts/Graphs/DataVisualization or WebEngine in the baseline. Target macOS 14.4+ Apple Silicon, Windows 10 22H2 x64 and serviced Windows 11 x64. Qt remains outside domain/application interfaces.

Qt 6.12 documentation identifies it as the last branch supporting Windows 10. P1 must establish patch/security maintenance and exact platform qualification; a future Qt upgrade cannot silently drop Windows 10. Documented support is not a local runtime test.

## Alternatives

Qt Quick improves declarative/GPU-oriented UI but requires more docking/workbench integration; a contained canvas/panel remains possible later. Rust egui and Slint require more professional controls/accessibility/desktop qualification; Slint has materially conditional licensing. Tauri adds WebView/JS/IPC and engine variance. Avalonia is credible but differs in control support/licensing and scientific integration. SwiftUI needs a separate Windows presentation layer.

## Consequences

Widgets' traditional styling requires deliberate design to achieve a modern workspace. Its default paint path is not automatically GPU accelerated. Shared-library Qt distribution requires actual LGPL replacement/source/notice compliance under signing/packaging. QPainter establishes a CPU/export reference, while scientific scene ownership allows future GPU backends without replacing the document model.

## Revisit triggers and acceptance

P1 workbench/table/accessibility/multi-monitor experiments or P3 measured rendering limits may justify a Quick surface or alternative presentation adapter. The weighted advantage over Quick is small, so experiments can override ordinal scores. Preserve Application API, project formats and plot semantics. No Qt application has been built in this planning delivery.
