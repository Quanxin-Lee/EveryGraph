# Origin / OriginPro 2026b 功能清单

文档版本：1.0 · 核对日期：2026-10-30 · 范围：Origin / OriginPro 2026b 公开功能盘点（非规范性研究附录）

本报告按用户明确要求使用中文；项目规范性文档仍遵循 `AGENTS.md` 的英文政策。本报告只记录竞品公开能力，不包含产品差距、优先级或对等性判断，也不构成需求、承诺或验收依据。Origin 是 Windows 应用，本机未安装，所有条目来自厂商公开资料，不等同于本机实测。

## 1. 目的、范围与证据规则

### 1.1 目的

把 Origin / OriginPro 当前公开版本的功能按官方类别和实际模块汇总成可回查的清单，供后续产品研究引用。报告覆盖产品页、2026/2026b 发布说明、Origin 与 OriginPro 对比页、图型库、官方功能子页和 App 目录中出现的模块、命令、图型、分析方法、导入导出格式、自动化入口和扩展机制。

### 1.2 粒度

- 以“命名模块、命名命令、命名方法、命名格式”为一条，不逐项枚举每个对话框的参数、默认值和快捷键。
- 同一能力只在一个主章节计数；跨章节出现时在说明中注明归属，不重复计数。
- 官方名称保留英文，中文为解释性名称；冲突时以官方名称为准。
- 一个图型家族算一条；家族内的具体图型在说明中列举，不把每个示例图都拆成独立 ID。

### 1.3 版本与平台基线

| 项目 | 值 | 来源 |
|---|---|---|
| 当前公开版本 | Origin / OriginPro 2026b；2026-06 | `O1` `O3` `O5` |
| 上一版本 | Origin / OriginPro 2026；2025-11 | `O5` |
| 平台 | Windows；不是原生 macOS 应用 | `O9` |
| 版本线 | Origin 基础版与 OriginPro | `O4` |
| 图型数量 | “100+ 内置和扩展图型”属于厂商声明 | `O1` `O6` |
| 用户规模 | “超过 100 万用户”属于厂商声明 | `O1` |
| 扩展 | App Center、File Exchange、Apps | `O1` `O7` `O8` |

### 1.4 不包含

- 不做 Origin 的本机安装、界面操作、性能测试或数值验证。
- 不把厂商声明、营销页数量或第三方评测写成已实测结论。
- 不把 OriginPro 专属能力写成基础版能力；无法由官方对比页或 PRO 标记确认时标为 `Unconfirmed`。
- 不把 Windows 平台限制写成产品缺陷；它只是平台边界。
- 不判断 Origin 与任何其他产品的优劣、差距或优先级。

## 2. 产品概览

Origin 由 OriginLab 开发，是面向科研、工程和实验数据处理的 Windows 数据分析与绘图软件。它采用多文档项目结构，把 Workbook、Matrix、Graph、Layout、Notes 等窗口保存在 OPJ/OPJU 项目中；数据、图形和分析结果可保持关联，参数或输入变化后结果可重算。OriginPro 在 Origin 基础上增加高级拟合、峰值拟合、曲面拟合、统计、信号处理、图像处理和 Apps。

| 维度 | 公开信息 | 来源 |
|---|---|---|
| 厂商 | OriginLab Corporation | `O1` |
| 平台 | Windows | `O9` |
| 当前版本 | 2026b，2026-06 | `O1` `O3` `O5` |
| 版本线 | Origin；OriginPro 增加高级分析与 Apps | `O4` |
| 工作方式 | 多文档项目、点击式定制、模板/主题、重算、批处理 | `O1` |
| 图型 | 100+ 内置和扩展图型，覆盖 2D、3D、统计、专业图 | `O1` `O6` |
| 分析 | 曲线/曲面拟合、峰值、统计、信号、数学、数据处理、RS/GIS、图像 | `O1` `O10`–`O20` |
| 编程 | LabTalk、Origin C、Python、R/Rserve、MATLAB、Mathematica、LabVIEW、COM | `O1` `O13` |
| 扩展 | App Center、File Exchange、Apps | `O1` `O7` `O8` |
| 项目文件 | OPJ、OPJU；支持密码保护 | `O1` |

## 3. 官方类别覆盖表

| 官方类别 | 官方来源 | 本文章节 | 条目数 | 状态 | 说明 |
|---|---|---|---:|---|---|
| Graphing | `O1` `O6` `O17` | §4.4、§4.5 | 15+11 | 覆盖 | 图型家族与图形定制分开计数 |
| Interface | `O1` `O21` | §4.1 | 10 | 覆盖 | 项目、窗口、暗色模式、迷你工具栏、撤销/重做 |
| Importing | `O1` `O5` | §4.3 | 12 | 覆盖 | 文件、数据库、连接器、Digitizer、批量导入 |
| Exploration | `O1` | §4.6 | 8 | 覆盖 | 缩放、读取、掩码、ROI、Browser Graph |
| Filtering | `O1` | §4.6 | 含上 | 合并 | Slicer 归入交互探索 |
| Recalculation | `O1` `O5` | §4.7 | 7 | 覆盖 | 参数重算、分析锁、依赖更新 |
| Exploratory Analysis | `O1` | §4.8 | 14 | 覆盖 | Gadgets 清单 |
| Curve/Surface Fitting | `O1` `O19` | §4.9 | 11 | 覆盖 | 线性/非线性、全局、曲面、隐函数 |
| Peak Analysis | `O1` `O20` | §4.10 | 9 | 覆盖 | 基线、寻峰、积分、峰拟合、批量 |
| Statistics | `O1` `O18` | §4.11 | 19 | 覆盖 | 描述、检验、ANOVA、多元、机器学习、生存 |
| Signal Processing | `O1` `O10` | §4.12 | 12 | 覆盖 | 变换、平滑、滤波、小波、卷积 |
| Mathematics | `O1` `O11` | §4.13 | 10 | 覆盖 | 列/单元格计算、插值、微积分、矩阵 |
| Data Processing | `O1` `O12` | §4.14 | 10 | 覆盖 | 重排、抽取、缩减、生成数据 |
| RS/GIS | `O1` | §4.15 | 4 | 覆盖 | NetCDF、GeoTIFF、处理、可视化 |
| Image Processing | `O14` | §4.15 | 6 | 覆盖 | 调整、算术、转换、几何、空间滤波 |
| Exporting/Presentation | `O1` `O16` | §4.16、§4.17 | 8+9 | 覆盖 | 报告/布局与导出分开计数 |
| Batch Processing | `O1` `O15` | §4.18 | 7 | 覆盖 | 绘图、导入、分析、模板 |
| Project/Data Management | `O1` | §4.1、§4.2 | 10+13 | 覆盖 | 项目/窗口与工作簿/矩阵分开计数 |
| Programming/Connectivity | `O1` `O13` | §4.19 | 9 | 覆盖 | LabTalk、Origin C、Python、R、MATLAB 等 |
| Apps in Origin | `O1` `O7` `O8` | §4.20 | 6 | 覆盖 | App Center/File Exchange 机制；具体 App 归功能章节 |
| 补充：2026/2026b 新增 | `O2` `O3` `O5` | §5 | 见 §5 | 覆盖 | 版本新增单列，不混入长期能力 |

条目按主章节计数，不重复；合计见 §8。覆盖表证明官方类别有对应章节，不证明每个子选项都已枚举。

## 4. 功能清单

### 4.1 界面、项目与窗口

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-UI-001 | 项目管理器 | Project Explorer / Project Browser | 以树形结构浏览项目中的文件夹、工作簿、矩阵、图形、布局和笔记，支持查找与组织 | Core | `O1` `O5` |
| ORG-UI-002 | 对象管理器 | Object Manager | 管理图层、图例、轴标题、文本框和注释等图形对象，可按类型选择并批量编辑 | Core | `O21` |
| ORG-UI-003 | 多文档窗口类型 | Window Types | 项目可包含 Workbook、Matrix、Graph、Layout、Notes、Image 等窗口 | Core | `O1` |
| ORG-UI-004 | 多窗口与浮动窗口 | Float Windows | 窗口可在 Origin 主界面内排列，也可浮动到桌面以利用多显示器空间 | Core | `O1` |
| ORG-UI-005 | 暗色模式 | Dark Mode | 将 Origin 界面切换为暗色外观；出版图形配色独立于界面外观 | Core | `O1` |
| ORG-UI-006 | 迷你工具栏 | Mini Toolbar | 在图形、工作簿或矩阵上快速执行常用操作，减少对话框切换 | Core | `O1` |
| ORG-UI-007 | 搜索与项目内查找 | Search / Find in Project Files | 按关键词搜索并执行命令；2026 起可跨最近项目或指定文件夹查找内容 | Core / 2026 新增 | `O1` `O5` |
| ORG-UI-008 | 撤销与重做 | Undo / Redo | 支持逐步撤销和重做；2026 起可用快捷键重复上一步或重开上次分析对话框 | Core / 2026 新增 | `O1` `O5` |
| ORG-UI-009 | 自定义报告样式 | Customizable Report Style | 为分析报告和结果表设置统一的标题、表格和数字格式 | Core | `O1` |
| ORG-UI-010 | 项目文件与密码保护 | OPJ/OPJU and Password Protection | 用 OPJ/OPJU 保存项目，可设置密码保护；项目中的窗口和文件夹可组织管理 | Core | `O1` |

本章节覆盖项目/窗口/界面；工作簿和矩阵容器归 §4.2，图形对象样式归 §4.5，分析结果报告归 §4.16。Object Manager 只在本章节计数。

### 4.2 数据表、矩阵与数据准备

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-DATA-001 | 工作簿与工作表 | Workbooks and Worksheets | 以工作簿组织多个工作表，支持列、行、表头和元数据 | Core | `O1` |
| ORG-DATA-002 | 矩阵簿、矩阵表与对象 | Matrix Books, Sheets and Objects | 用矩阵容器保存图像、网格和二维数据，供矩阵运算和绘图使用 | Core | `O1` |
| ORG-DATA-003 | 列与行操作 | Column and Row Operations | 插入、删除、移动、调整列宽/行高，设置列角色、单位、格式和显示 | Core | `O1` |
| ORG-DATA-004 | 单元格与列公式 | Cell and Column Formulas | 在单元格或整列使用公式计算，支持跨列引用、自动更新和批量填充 | Core | `O1` |
| ORG-DATA-005 | 快速填充与智能公式 | Flash Fill and Smart Formula | 2026 新增：从文本列提取/合并内容，直接填充或生成可更新的列公式 | Core / 2026 新增 | `O5` |
| ORG-DATA-006 | Excel 公式导入与公式查找替换 | Excel Formula Import and Formula Find/Replace | 2026 新增：把 Excel 单元格公式带入 Origin，支持高亮错误公式和在公式中查找替换 | Core / 2026 新增 | `O5` |
| ORG-DATA-007 | 排序与筛选 | Sort and Filter | 按列排序、条件筛选和显示子集；筛选默认只影响视图，不改原始数据 | Core | `O1` |
| ORG-DATA-008 | 数据分组 | Data Grouping | 按分类列或行组织重复实验/多组数据，供绘图和分析按组处理 | Core | `O1` |
| ORG-DATA-009 | 结构整理 | Merge, Append, Split, Transpose, Pivot, Stack/Unstack | 合并工作表、追加/拆分、转置、透视、堆叠/反堆叠等结构变换 | Core | `O1` `O12` |
| ORG-DATA-010 | 抽取与缩减 | Extract and Reduce | 按条件抽取行/列，或缩减重复数据、行数、列数 | Core | `O1` `O12` |
| ORG-DATA-011 | 缺失值 | Missing Values | 识别/处理缺失值；缺失值不等于零，显示格式不改变计算精度 | Core | `O1` |
| ORG-DATA-012 | 镜像列 | Mirror Columns | 2026b 新增：复制来源列并建立镜像关系，数据只存一份，可跨工作表/工作簿复用 | Core / 2026b 新增 | `O2` |
| ORG-DATA-013 | 向其他工作表添加列 | Add Columns to Other Sheets | 2026b 新增：把列或插入列复制到当前/其他工作簿的同类工作表，并复制列公式 | Core / 2026b 新增 | `O2` |

本章节覆盖数据容器、列类型/公式和结构整理；导入归 §4.3，数学变换归 §4.13，数据处理/重排归 §4.14，统计归 §4.11。一个列操作只在本章节计数。

### 4.3 导入、连接器与数据库

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-IO-001 | 多格式导入 | 30+ Import Formats | 支持 30 多种数据格式，包括 ASCII、CSV、Excel、二进制等 | Core | `O1` |
| ORG-IO-002 | ASCII/CSV/TXT | ASCII, CSV and Text | 导入纯文本数据，支持分隔符、表头、跳过行和区域格式设置 | Core | `O1` |
| ORG-IO-003 | Excel 文件 | Excel Import | 导入 XLS、XLSX、XLSM，无需安装 Microsoft Excel | Core | `O1` |
| ORG-IO-004 | 数据库导入 | Database Connector (ADO/ODBC) | 通过 Database Connector 连接数据库并导入表/查询结果 | Core / Connector | `O1` |
| ORG-IO-005 | NetCDF 与 GeoTIFF | NetCDF and GeoTIFF | 导入科学数据网格和地理栅格，供矩阵分析与 RS/GIS 使用 | Core | `O1` |
| ORG-IO-006 | MATLAB 文件 | MATLAB File Import | 导入 MATLAB 数据文件，供后续分析/绘图使用 | Core | `O1` |
| ORG-IO-007 | 数据连接器 | Data Connectors | 连接 Web、本地、网络、云盘或数据库数据源，支持更新和重导入 | Core / Connector | `O1` |
| ORG-IO-008 | 拖放与剪贴板 | Drag-and-Drop and Clipboard | 常见文件可拖放导入；从 Excel 复制粘贴可保持完整精度 | Core | `O1` |
| ORG-IO-009 | 重导入与周期导入 | Re-import and Periodic Import | 文件更新后重导入数据并重跑后续处理；2026 起支持 5 秒到 3 天的周期导入 | Core / 2026 新增 | `O1` `O5` |
| ORG-IO-010 | 移除连接器 | Remove Connector After Import | 2026b 新增：导入后可选择断开数据连接，避免继续依赖外部文件/数据库 | Core / 2026b 新增 | `O2` |
| ORG-IO-011 | XML 连接器 | XML Connector | 连接 XML 数据源；2026 起可检测 Unix 时间戳 | Core / 2026 新增 | `O5` |
| ORG-IO-012 | 图形数字化 | Digitizer | 从图形图像中手动或半自动提取数据点，支持数值、时间、日期刻度 | Core | `O1` |

本章节只计数据来源与导入；数据表结构归 §4.2，批量导入归 §4.18，RS/GIS 分析归 §4.15。Digitizer 作为导入工具只计一次，Gadgets 中的 Digitizer 是交互式分析入口，见 §4.8 的说明。

### 4.4 绘图与图型

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-GRAPH-001 | 图型总览 | 100+ Graph Types | 官方称 100+ 内置和扩展图型，覆盖线/符号、柱/条、饼/环、多轴、瀑布、等高线、热图、3D、分组、分类、层级、统计、质量、专业、极坐标、矢量、三元、水文、剖面、函数和图像等家族；数量属于厂商声明 | Core | `O1` `O6` `O17` |
| ORG-GRAPH-002 | 线图与符号图 | Line and Symbol Plots | 折线、散点、线+符号、阶梯线等基础二维图 | Core | `O1` `O6` |
| ORG-GRAPH-003 | 柱状图与条形图 | Column and Bar Charts | 单组/分组/堆叠柱状图、条形图、面积图和带连接线的柱图 | Core | `O1` `O6` |
| ORG-GRAPH-004 | 饼图与环图 | Pie and Doughnut Charts | 单组/多组饼图、环图和分类占比图 | Core | `O1` `O6` |
| ORG-GRAPH-005 | 多轴与多面板 | Multi-Axis and Multi-Panel Graphs | 双 Y、多 X/Y、多图层和多面板组合，支持共享/独立坐标轴 | Core | `O1` `O17` |
| ORG-GRAPH-006 | 瀑布图 | Waterfall Plots | 用错位/堆叠方式展示多组曲线或序列变化 | Core | `O1` `O6` |
| ORG-GRAPH-007 | 等高线与热图 | Contour and Heatmap Plots | 用矩阵/XYZ 数据绘制等高线、热图、色块和标量场 | Core | `O1` `O6` |
| ORG-GRAPH-008 | 三维图 | 3D Graphs | 3D 散点、曲面、等高线、柱状和瀑布等三维图型 | Core | `O1` `O6` |
| ORG-GRAPH-009 | 分组、分类与层级图 | Grouped, Categorical and Hierarchical Graphs | 按分组/分类/层级组织多组数据，生成分组散点、分组柱、分类图等 | Core | `O1` `O6` |
| ORG-GRAPH-010 | 统计与质量图 | Statistical and Quality Graphs | 直方图、箱线图、概率图、控制图、Pareto 等统计/质量图型 | Core | `O1` `O6` |
| ORG-GRAPH-011 | 专业图型 | Specialized Graphs | Gantt、箭头图、网络/相关图、Before-After、地图等专业图型 | Core | `O1` `O6` |
| ORG-GRAPH-012 | 极坐标、矢量与三元图 | Polar, Vector and Ternary Graphs | 极坐标、矢量场、三元/三角坐标等专业坐标图 | Core | `O1` `O6` |
| ORG-GRAPH-013 | 水文、剖面、函数与图像图 | Hydrochemistry, Profile, Function and Image Graphs | 水文化学图、剖面图、函数曲线和图像显示 | Core | `O1` `O6` |
| ORG-GRAPH-014 | 图形制作器 | Graph Maker | 2026 新增/更新的图形制作器，可用拖拽方式快速生成并分组/分面绘图 | Core / 2026 新增 | `O5` `O2` |
| ORG-GRAPH-015 | 误差棒与绘图修饰 | Error Bars and Plot Modifiers | 误差棒、数据点标签、趋势线、参考线、填充、透明度和其它绘图修饰 | Core | `O1` |

本章节覆盖图型家族和图型级修饰；坐标轴、图例、标注、模板/主题和出版布局归 §4.5；统计/拟合结果叠加归 §4.9 和 §4.11。2026/2026b 新增的具体图型见 §5。

### 4.5 图形定制、布局与出版

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-PLOT-001 | 坐标轴与刻度 | Axis Dialog and Scale Types | 设置线性、对数、概率、倒数、日期/时间等轴类型，以及范围、刻度、标签和单位 | Core | `O1` |
| ORG-PLOT-002 | 网格与参考线 | Grids and Reference Lines | 添加主/次网格、参考线、零线和自定义刻度线 | Core | `O1` `O2` |
| ORG-PLOT-003 | 轴断点与多轴 | Axis Breaks and Multiple Axes | 添加轴断点、双 Y/多 X 轴和独立缩放 | Core | `O1` `O2` |
| ORG-PLOT-004 | 图例与色标 | Legends and Color Scales | 自动/自定义图例、色标、分类图例和图层图例 | Core | `O1` `O21` |
| ORG-PLOT-005 | 标注与绘图对象 | Annotations and Drawing Objects | 添加文本、箭头、线条、形状、公式、图像和标注，支持吸附/对齐 | Core | `O1` `O2` |
| ORG-PLOT-006 | 样式与主题 | Graph Styles and Themes | 保存颜色、线型、字体、标记等样式，作为主题复用到其它图 | Core | `O1` |
| ORG-PLOT-007 | 模板与默认值 | Graph Templates and Defaults | 把定制图保存为模板，或修改默认设置以统一新图外观 | Core | `O1` |
| ORG-PLOT-008 | 页面尺寸与布局 | Page Size and Layout | 设置页面尺寸、边距、图层位置和出版尺寸，支持布局窗口组合多图 | Core | `O1` |
| ORG-PLOT-009 | 多面板与局部放大 | Multi-Panel and Insets | 组合多图层/多面板，添加局部放大图和嵌入图 | Core | `O1` |
| ORG-PLOT-010 | 图形选项 | Graph Options | 2026b 增强 Graph Options 对话框属性，集中控制图形/图层选项 | Core / 2026b 新增 | `O2` |
| ORG-PLOT-011 | 出版尺寸调整 | Resize Graph for Publishing | 按期刊/出版要求调整图形尺寸、字体和线宽 | Core | `O1` |

本章节覆盖图形外观和出版布局；具体图型归 §4.4，导出格式归 §4.17，报告/演示归 §4.16。Object Manager 归 §4.1，不在此重复计数。

### 4.6 交互探索与数据过滤

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-EXPLORE-001 | 缩放与平移 | Zoom and Pan | 在图形中放大区域、平移视图并快速恢复全图 | Core | `O1` |
| ORG-EXPLORE-002 | 数据读取器 | Data Reader | 在图上读取数据点坐标、关联数据和选中点信息 | Core | `O1` |
| ORG-EXPLORE-003 | 屏幕读取器 | Screen Reader | 读取屏幕上任意位置对应的数据/坐标，辅助检查图形 | Core | `O1` |
| ORG-EXPLORE-004 | 掩码与数据点修改 | Masking and Alter/Remove Points | 用掩码排除/标出数据点，或交互式修改、删除点；原始数据可保留 | Core | `O1` |
| ORG-EXPLORE-005 | ROI 数据抽取 | ROI-based Data Extraction | 在图形上拖出感兴趣区域，提取区域内的数据用于分析 | Core | `O1` |
| ORG-EXPLORE-006 | 浏览器图形 | Browser Graph | 对多通道/多组数据进行分面浏览和交互探索 | Core | `O1` |
| ORG-EXPLORE-007 | 图形切片筛选 | Slicer | 在图形上按 Page、Layer 或 Plot 级别筛选数据，不改源数据；分析结果随筛选更新 | Core | `O1` |
| ORG-EXPLORE-008 | 数据过滤 | Data Filter | 用条件过滤工作表中的行，供绘图和分析使用 | Core | `O1` `O12` |

本章节覆盖交互查看和筛选；数据重排/抽取归 §4.14，Gadgets 归 §4.8，分析重算归 §4.7。Slicer 只在本文计数。

### 4.7 重算与关联更新

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-RECALC-001 | 参数重开与重算 | Change Parameters and Recalculate | 随时重开分析对话框修改参数，结果和图形随之更新 | Core | `O1` |
| ORG-RECALC-002 | 输入变化触发重算 | Input-Triggered Recalculation | 输入数据或参数变化可自动触发后续分析和图形重算 | Core | `O1` |
| ORG-RECALC-003 | 分析锁 | Analysis Locks | 在图形/结果上显示分析锁，标识结果与输入/参数的关联状态 | Core | `O1` `O2` |
| ORG-RECALC-004 | 自动/手动重算 | Auto and Manual Recalculation | 选择自动更新或手动更新，控制重算时机 | Core | `O1` |
| ORG-RECALC-005 | 依赖更新 | Dependency Updates | 更新数据、参数或上游结果时，沿依赖链更新下游结果 | Core | `O1` |
| ORG-RECALC-006 | 复制操作到其它数据集 | Duplicate Operation to Other Datasets | 2026 新增：把最近分析/操作应用到指定工作表或其它数据集 | Core / 2026 新增 | `O5` |
| ORG-RECALC-007 | 重复上一步与重开对话框 | Repeat Last Action and Reopen Dialog | 2026 新增：快捷键重复上一步操作，或重开上次分析/绘图/Apps 对话框 | Core / 2026 新增 | `O5` |

本章节覆盖结果与输入/参数的关联更新；批处理和分析模板归 §4.18，统计/拟合方法本身归 §4.9 和 §4.11。分析锁的隐藏/显示是图形修饰，见 §5 的 2026b 变更。

### 4.8 Gadgets 快速分析

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-GADGET-001 | 统计 Gadget | Statistics Gadget | 在选区内交互查看统计量，移动/缩放 ROI 时即时更新并生成报告 | Core | `O1` |
| ORG-GADGET-002 | 垂直光标 | Vertical Cursor | 用垂直光标读取多条曲线在同一 X 处的 Y 值 | Core | `O1` |
| ORG-GADGET-003 | 微分 Gadget | Differentiate Gadget | 对选区曲线做微分并显示结果 | Core | `O1` |
| ORG-GADGET-004 | 数字化 Gadget | Digitizer Gadget | 从图形图像中交互提取数据点 | Core | `O1` |
| ORG-GADGET-005 | FFT Gadget | FFT Gadget | 对选区数据做快速傅里叶变换并显示频谱 | Core | `O1` |
| ORG-GADGET-006 | 积分 Gadget | Integration Gadget | 对选区曲线做积分并显示面积/结果 | Core | `O1` |
| ORG-GADGET-007 | 插值 Gadget | Interpolate Gadget | 在选区内插值并查看插值曲线/数值 | Core | `O1` |
| ORG-GADGET-008 | 交点 Gadget | Intersect Gadget | 查找曲线交点或曲线与轴/参考线的交点 | Core | `O1` |
| ORG-GADGET-009 | 快速拟合 | Quick Fit Gadget | 对选区/曲线快速执行线性或非线性拟合 | Core | `O1` |
| ORG-GADGET-010 | 快速寻峰 | Quick Peaks Gadget | 快速检测/拟合峰并查看峰参数 | Core | `O1` |
| ORG-GADGET-011 | 快速 Sigmoidal 拟合 | Quick Sigmoidal Fit Gadget | 对 S 形/剂量响应类数据快速拟合 | Core | `O1` |
| ORG-GADGET-012 | 聚类 Gadget | Cluster Gadget | 对选区数据做聚类分析 | Pro | `O1` `O4` |
| ORG-GADGET-013 | 二维积分 Gadget | 2D Integration Gadget | 对二维/矩阵选区做二维积分 | Pro | `O1` `O4` |
| ORG-GADGET-014 | 上升时间 Gadget | Rise Time Gadget | 计算信号上升时间等瞬态指标 | Pro | `O1` `O4` |

本章节只计交互式 Gadgets；完整的拟合/统计/信号方法归 §4.9、§4.11、§4.12。Digitizer 作为 Gadget 入口在此计数，导入章节不重复计 Gadget。

### 4.9 曲线、曲面与隐函数拟合

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-FIT-001 | 拟合总览 | Curve Fitting Overview | 从工作表或图形选择数据，指定数值/图形子区间，执行线性和非线性拟合 | Core | `O1` `O19` |
| ORG-FIT-002 | 多数据集拟合模式 | Independent, Global, Concatenate and Replicate Fit | 多个数据集可独立、全局、拼接或重复模式拟合 | Core | `O1` `O19` |
| ORG-FIT-003 | 全局参数共享 | Global Parameter Sharing | 在多个数据集之间共享参数，适合全局拟合 | Core | `O1` |
| ORG-FIT-004 | 内置拟合函数 | Built-in Fitting Functions | 官方称接近 200 个内置拟合函数，按类别组织 | Core / 厂商声明 | `O1` |
| ORG-FIT-005 | 自定义拟合函数 | Fitting Function Builder | 用 Fitting Function Builder 创建自定义函数并保存复用 | Core | `O1` |
| ORG-FIT-006 | 显式/隐式函数与约束 | Explicit/Implicit Functions, Bounds and Constraints | 支持显式/隐式函数、参数边界和约束，控制参数空间 | Core / Pro | `O1` `O4` |
| ORG-FIT-007 | 迭代算法 | Levenberg-Marquardt and ODR | 提供 LM 等迭代算法；Orthogonal Distance Regression 为 Pro | Core / Pro | `O1` `O4` |
| ORG-FIT-008 | 拟合报告与预测 | Fit Report and Prediction | 生成包含统计量、参数和 ANOVA 的报告表，可按拟合曲线预测新 X/Y 值 | Core | `O1` |
| ORG-FIT-009 | 模型/数据集比较 | Comparing Models and Datasets | 比较两个模型/两个数据集，或用多个模型拟合并按 F-test、AIC/BIC 排序 | Pro | `O1` `O4` |
| ORG-FIT-010 | 曲面拟合 | Surface Fitting | 对 XYZ 工作表或矩阵数据做三维曲面拟合，支持多峰/自动寻峰和 Z 预测 | Pro | `O1` `O4` |
| ORG-FIT-011 | 隐函数拟合 | Implicit Fitting | 用 ODR 拟合隐函数，支持 X/Y 误差数据 | Pro | `O1` `O4` |

本章节覆盖曲线/曲面/隐函数拟合；峰值拟合归 §4.10，Gadgets 快速拟合归 §4.8，信号平滑/滤波归 §4.12。拟合函数数量和 ODR 等 Pro 项按官方页面标注。

### 4.10 峰值分析

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-PEAK-001 | 基线检测 | Baseline Detection | 自动/手动确定基线，支持二阶导数、XPS、非对称最小二乘等方法 | Core | `O1` `O20` |
| ORG-PEAK-002 | 寻峰 | Peak Finding | 自动/手动寻找并添加峰，支持局部最大、窗口搜索、残差和导数方法 | Core | `O1` `O20` |
| ORG-PEAK-003 | 峰积分 | Peak Integration | 自动或指定积分窗口，计算峰面积等积分量 | Core | `O1` `O20` |
| ORG-PEAK-004 | 峰属性报告 | Peak Properties | 报告峰高、面积、面积百分比、质心、FWHM 等参数 | Core | `O1` |
| ORG-PEAK-005 | 峰拟合 | Peak Fitting | 用 25+ 内置峰函数进行峰拟合，可固定/限制峰中心、共享参数、逐步迭代 | Pro | `O1` `O4` |
| ORG-PEAK-006 | 峰反卷积 | Peak Deconvolution | 分辨重叠或隐藏峰，输出各分峰参数 | Pro | `O1` `O4` |
| ORG-PEAK-007 | 峰约束与参数共享 | Peak Constraints and Sharing | 固定峰中心、按百分比限制变化、跨峰共享参数 | Pro | `O1` |
| ORG-PEAK-008 | 批量峰分析 | Batch Peak Analysis | 用 Gadgets 或预设主题对多个数据集批量做峰分析 | Core / Pro | `O1` `O15` |
| ORG-PEAK-009 | 峰分析工作流 | Peak Analyzer | 把基线、寻峰、积分、拟合和报告组织成可重用的分析流程 | Core | `O1` `O20` |

本章节覆盖峰值分析；曲线/曲面拟合归 §4.9，Gadgets 快速寻峰归 §4.8。峰值拟合/反卷积等 Pro 项按官方 PRO 标记。

### 4.11 统计、质量与机器学习

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-STAT-001 | 统计顾问 | Stats Advisor App | 根据任务/数据结构推荐合适的统计检验 | App | `O1` `O4` `O7` |
| ORG-STAT-002 | 描述统计 | Descriptive Statistics | 按列或行计算均值、标准差、分位数等描述统计 | Core | `O1` `O18` |
| ORG-STAT-003 | 图形摘要 | Graphical Summary | 2026 新增：用交互式图形摘要展示分布和统计量 | Core / 2026 新增 | `O5` |
| ORG-STAT-004 | 交叉表 | Cross Tabulation | 生成列联表/交叉统计 | Pro | `O1` `O4` |
| ORG-STAT-005 | 频数统计 | Frequency Counts | 一维/二维频数、离散频数和分组计数 | Core | `O1` |
| ORG-STAT-006 | 正态性与异常值检验 | Normality and Outlier Tests | 正态性检验，以及 Grubbs、Dixon's Q 等异常值检验 | Core | `O1` |
| ORG-STAT-007 | 分布拟合 | Distribution Fit | 拟合正态、Weibull、指数、Gamma 等分布 | Pro | `O1` `O4` |
| ORG-STAT-008 | 相关分析 | Correlation and Partial Correlation | 计算相关系数和偏相关系数 | Pro | `O1` `O4` |
| ORG-STAT-009 | 参数检验 | Parametric Tests | 单样本/双样本/配对 t 检验、Welch 检验 | Core | `O1` `O18` |
| ORG-STAT-010 | 高级参数检验 | Advanced Parametric Tests | 行上的双样本/配对 t 检验、卡方、方差、比例、Poisson 率检验 | Pro | `O1` `O4` |
| ORG-STAT-011 | 非参数检验 | Nonparametric Tests | Wilcoxon、符号检验、KS、Mann-Whitney、Kruskal-Wallis、Mood 中位数、Friedman 等 | Pro | `O1` `O4` |
| ORG-STAT-012 | 方差分析 | ANOVA | 单因素/双因素 ANOVA，Tukey、Bonferroni、Dunn-Sidak、Fisher LSD、Scheffe、Holm 等均值比较 | Core | `O1` `O18` |
| ORG-STAT-013 | 高级方差分析 | Advanced ANOVA | 三因素 ANOVA、单/双因素重复测量 ANOVA | Pro | `O1` `O4` |
| ORG-STAT-014 | 多元统计 | Multivariate Analysis | PCA、K-Means、层次聚类、判别分析、偏最小二乘 | Pro | `O1` `O4` |
| ORG-STAT-015 | 机器学习与决策树 | Machine Learning and Decision Tree | 机器学习工具和决策树分析 | Pro / App | `O1` `O4` `O2` |
| ORG-STAT-016 | 生存与可靠性 | Survival and Reliability Analysis | 生存/可靠性分析、分布分析、寿命数据回归等 | Pro / App | `O1` `O4` `O2` |
| ORG-STAT-017 | 时间序列 | Time Series Analysis | 时间序列探索、平滑、分解和预测等 | Core | `O1` |
| ORG-STAT-018 | 实验设计与质量控制 | DOE and SPC | 实验设计、统计过程控制、控制图、能力分析等；部分为 App/Pro | App / Pro | `O1` `O2` `O4` |
| ORG-STAT-019 | 检验结论图 | Conclusion Charts for Hypothesis Tests | 2026 新增：为 t 检验等假设检验生成结论图 | Core / 2026 新增 | `O5` |

本章节覆盖统计方法；Gadgets 的快速统计/拟合归 §4.8，曲线拟合归 §4.9，峰值分析归 §4.10，信号处理归 §4.12。App 类统计方法按官方 App 标记，不写成基础版内置。

### 4.12 信号处理

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-SIGNAL-001 | FFT/IFFT | FFT and IFFT | 对一维信号做快速傅里叶变换/逆变换 | Core | `O1` `O10` |
| ORG-SIGNAL-002 | 短时傅里叶变换 | STFT | 用短时傅里叶变换分析非平稳信号的时频结构 | Pro | `O10` `O4` |
| ORG-SIGNAL-003 | Hilbert 变换 | Hilbert Transform | 计算解析信号、包络和瞬时相位/频率 | Pro | `O10` `O4` |
| ORG-SIGNAL-004 | 二维 FFT | 2D FFT and 2D IFFT | 对矩阵/图像做二维频域变换 | Pro | `O10` `O4` |
| ORG-SIGNAL-005 | 小波分析 | Wavelet Analysis | 一维/二维 CWT、DWT、MDWT、IDWT、去噪和平滑 | Pro | `O10` `O4` |
| ORG-SIGNAL-006 | 平滑 | Smoothing | Savitzky-Golay、相邻平均、FFT、百分位、Lowess、Loess、二项式等 | Core | `O10` |
| ORG-SIGNAL-007 | 滤波 | Filtering | FFT 低通/高通/带通/带阻/阈值滤波 | Core | `O10` |
| ORG-SIGNAL-008 | IIR 滤波器设计 | IIR Filter Design | Butterworth、Chebyshev I/II、Elliptic 等 IIR 设计 | Pro | `O10` `O4` |
| ORG-SIGNAL-009 | 二维 FFT 滤波 | 2D FFT Filter | 对二维数据/图像做频域滤波 | Pro | `O10` `O4` |
| ORG-SIGNAL-010 | 卷积与相关 | Convolution and Correlation | 一维/二维卷积和相关分析 | Core / Pro | `O10` `O4` |
| ORG-SIGNAL-011 | 相干、包络与抽取 | Coherence, Envelope and Decimation | 相干分析、包络曲线、信号抽取/降采样 | Pro | `O10` `O4` |
| ORG-SIGNAL-012 | 上升时间 Gadget | Rise Time Gadget | 交互计算信号上升时间等瞬态指标 | Pro | `O1` `O10` |

本章节覆盖信号变换、平滑、滤波和小波；数学微积分归 §4.13，Gadgets 归 §4.8，图像空间滤波归 §4.15。Pro 标记按官方子页和对比页。

### 4.13 数学与矩阵运算

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-MATH-001 | 列与单元格计算 | Column and Cell Calculations | 对列/单元格执行算术、函数和自定义表达式 | Core | `O1` `O11` |
| ORG-MATH-002 | 归一化 | Normalization | 按最大值、最小值、均值、面积、参考点等归一化数据 | Core | `O11` |
| ORG-MATH-003 | 多曲线平均 | Average Multiple Curves | 对多条曲线做平均/统计，生成代表曲线 | Core | `O11` |
| ORG-MATH-004 | 列/曲线简单数学 | Simple Math on Columns or Curves | 对列或曲线做加、减、乘、除等逐点运算 | Core | `O11` |
| ORG-MATH-005 | 一维插值/外推 | 1D Interpolation and Extrapolation | 线性、样条等一维插值/外推 | Core | `O1` `O11` |
| ORG-MATH-006 | 二维插值/外推 | 2D Interpolation and Extrapolation | 对矩阵/二维数据做插值/外推 | Core | `O11` |
| ORG-MATH-007 | 三维插值 | 3D Interpolation | 对三维散点/网格数据做插值 | Core | `O11` |
| ORG-MATH-008 | 微分 | Differentiation | 对列或曲线做数值微分 | Core | `O1` `O11` |
| ORG-MATH-009 | 积分与面积 | Integration and Area | 对列/曲线做数值积分、面积计算 | Core | `O1` `O11` |
| ORG-MATH-010 | 矩阵运算 | Matrix Operations | Set Matrix Values、矩阵求逆、收缩/扩展、自定义/中值/特殊滤波 | Core | `O11` |

本章节覆盖数学变换和矩阵运算；数据处理/重排归 §4.14，信号滤波归 §4.12，图像处理归 §4.15。归一化/插值等不重复计入其它章节。

### 4.14 数据处理与重排

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-PREP-001 | 数据过滤 | Data Filter | 用条件筛选行，供绘图和分析使用 | Core | `O12` |
| ORG-PREP-002 | 工作表查询 | Worksheet Query | 用查询条件选择/提取工作表数据 | Core | `O12` |
| ORG-PREP-003 | 堆叠与反堆叠 | Stack and Unstack Columns | 宽表/长表互转，整理分组数据结构 | Core | `O12` |
| ORG-PREP-004 | 数据透视 | Pivot Table | 按行/列分类汇总，生成透视表 | Core | `O12` |
| ORG-PREP-005 | 结构整理 | Append, Split, Transpose, Sort, Find and Replace | 追加/拆分工作表、转置、排序、查找替换 | Core | `O12` |
| ORG-PREP-006 | 添加列与镜像列 | Add Columns and Mirror Columns | 向其他工作表添加列，或建立只存一份数据的镜像列 | Core / 2026b 新增 | `O12` `O2` |
| ORG-PREP-007 | 缩减重复/行/列 | Reduce Duplicate Data, Rows or Columns | 去重、缩减行/列、合并重复记录 | Core | `O12` |
| ORG-PREP-008 | 等间距缩减 | Reduce to Evenly Spaced X | 把数据缩减到等间距 X 网格 | Pro | `O12` `O4` |
| ORG-PREP-009 | 按组缩减 | Reduce by Group | 按分组/类别缩减数据 | Pro | `O12` `O4` |
| ORG-PREP-010 | 生成数据 | Generate Data | 生成行号、均匀/正态随机数、规律/任意数字、日期、时间、文本 | Core | `O12` |

本章节覆盖数据重排、抽取、缩减和生成；表格/列容器操作归 §4.2，数学变换归 §4.13，图表筛选归 §4.6。镜像列/添加列只在本章节计一次。

### 4.15 RS/GIS 与图像处理

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-RSGIS-001 | NetCDF | NetCDF | 读取/处理 NetCDF 科学网格数据，用于矩阵分析和绘图 | Core | `O1` |
| ORG-RSGIS-002 | GeoTIFF | GeoTIFF | 读取/处理 GeoTIFF 地理栅格，用于遥感和 GIS 工作流 | Core | `O1` |
| ORG-RSGIS-003 | RS/GIS 处理与分析 | RS/GIS Processing and Analysis | 对遥感/GIS 数据做处理和分析；具体操作以官方帮助页为准 | Core | `O1` |
| ORG-RSGIS-004 | RS/GIS 可视化 | RS/GIS Visualization | 用图形/地图等方式展示地理数据；具体图型以官方帮助页为准 | Core | `O1` |
| ORG-IMAGE-001 | 图像调整 | Image Adjustment | 亮度、对比度、Gamma、色相、反相、饱和度、直方图均衡、自动色阶、颜色替换等 | Core / Pro | `O14` |
| ORG-IMAGE-002 | 图像算术变换 | Image Arithmetic Transforms | Alpha 混合、提取 XYZ、简单数学、数学函数、形态学滤波、像素逻辑、背景替换/相减等 | Pro | `O14` |
| ORG-IMAGE-003 | 图像转换 | Image Conversion | 图像转数据、彩色转灰度、数据转图像、二值化/阈值、RGB 合并/拆分、图像缩放、调色板 | Core / Pro | `O14` |
| ORG-IMAGE-004 | 几何变换 | Geometric Transforms | 自动裁剪、裁剪、翻转、偏移、缩放、旋转、剪切 | Core | `O14` |
| ORG-IMAGE-005 | 空间滤波 | Spatial Filters | 平均、清除、边缘、Gaussian、中值、噪声、锐化、Unsharp Mask | Core | `O14` |
| ORG-IMAGE-006 | 自定义滤波 | User Filter | 用自定义核/参数做图像滤波 | Pro | `O14` |

本章节把 RS/GIS 与图像处理放在同一章，因为两者都主要操作矩阵/栅格数据；矩阵容器归 §4.2，矩阵数学归 §4.13，图像窗口/报告归 §4.16。Pro 标记按官方图像处理子页。

### 4.16 报告、布局与演示

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-REPORT-001 | 布局窗口 | Layout Window | 把多张图、表格和标注组合成出版版面 | Core | `O1` |
| ORG-REPORT-002 | 报告表 | Report Tables | 把分析结果组织成可编辑、可格式化的报告表 | Core | `O1` |
| ORG-REPORT-003 | 笔记窗口 | Notes Window | 在项目中保存文本笔记、说明和富文本内容 | Core | `O1` |
| ORG-REPORT-004 | 图像窗口 | Image Window | 在项目中保存/显示图像，供布局和报告使用 | Core | `O1` |
| ORG-REPORT-005 | 幻灯片 | Slide Show | 把多张图/布局组织成幻灯片放映 | Core | `O1` |
| ORG-REPORT-006 | 影片 | Creating Movies | 把图形序列/动画导出为影片 | Core | `O1` |
| ORG-REPORT-007 | 报告样式 | Report Style | 统一报告表的标题、表头、数字格式和布局 | Core | `O1` |
| ORG-REPORT-008 | 分析/图形报告 | Analysis and Graph Reports | 自动生成分析结果报告和图形说明，供导出/分享 | Core | `O1` |

本章节覆盖报告/布局/演示；导出格式和 PPT/Word 归 §4.17，图形版面本身归 §4.5。报告表可引用分析结果，但不在本文重复定义统计方法。

### 4.17 导出与互操作

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-EXPORT-001 | 图形导出 | Graph Export | 把图形/布局导出为图像；官方称支持 17 种格式，含 PDF/EPS/SVG/EMF 和 PNG/TIFF/JPEG/BMP 等 | Core | `O1` `O16` |
| ORG-EXPORT-002 | 布局导出 | Layout Export | 导出布局窗口为图像/矢量文件 | Core | `O1` `O16` |
| ORG-EXPORT-003 | 数据导出 | Data Export | 把工作表/矩阵数据导出为文本或其它格式 | Core | `O1` |
| ORG-EXPORT-004 | 复制与嵌入 | Copy, Paste and Embed | 把图形复制到剪贴板或嵌入其它应用 | Core | `O1` |
| ORG-EXPORT-005 | 发送到 PowerPoint/Word | Send to PowerPoint or Word | 把图形/表格发送到 PowerPoint 或 Word | Core | `O1` |
| ORG-EXPORT-006 | HTML/EMF 表格 | Copy/Paste HTML or EMF Tables | 把结果表复制为 HTML 或 EMF，便于网页/文档使用 | Core | `O1` |
| ORG-EXPORT-007 | 打印 | Print | 通过系统打印流程输出图形/布局/报告 | Core | `O1` |
| ORG-EXPORT-008 | 批量导出 | Batch Export | 导出当前文件夹/项目中的多张图，支持自动命名和格式设置 | Core | `O16` |
| ORG-EXPORT-009 | Notes 导出 MHTML | Export Notes to MHTML | 2026b 新增：把 Notes 导出为 MHTML 文件 | Core / 2026b 新增 | `O2` |

本章节覆盖输出/互操作；报告/布局构建归 §4.16，批处理归 §4.18。17 种格式只列常见类别，不逐项列全。

### 4.18 批处理

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-BATCH-001 | 批量绘图 | Batch Plotting | 对多个文件/工作表按同一模板批量生成图形 | Core | `O1` `O15` |
| ORG-BATCH-002 | 批量导入 | Batch Import | 对多个文件应用同一导入设置并合并/整理数据 | Core | `O1` `O15` |
| ORG-BATCH-003 | 批量分析 | Batch Analysis | 对多个数据集重复同一分析流程 | Core | `O1` `O15` |
| ORG-BATCH-004 | 分析模板 | Analysis Templates | 把分析设置保存为模板，复用到其它数据 | Core | `O15` |
| ORG-BATCH-005 | 批量分析结果 | Batch Analysis Results | 批量分析后查看/整理各数据集结果；具体汇总形式以官方帮助为准 | Core | `O15` |
| ORG-BATCH-006 | 数据工厂 | Data Factory | 批量导入/处理/绘图的 App，支持重复工作流 | App / Pro | `O2` `O7` |
| ORG-BATCH-007 | 智能绘图与可克隆模板 | Smart Plotting and Cloneable Graph Templates | 按数据结构自动匹配图型，并用可克隆模板批量复用 | Core | `O15` |

本章节覆盖批量绘图/导入/分析和模板；单个图形的创建/定制归 §4.4/§4.5，编程式自动化归 §4.19。Data Factory 只在本章节计数，`ORG-APPS` 不重复。

### 4.19 编程、自动化与连接

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-AUTO-001 | LabTalk | LabTalk | Origin 内置脚本语言，用于自动化、批处理和自定义流程 | Core | `O1` `O13` |
| ORG-AUTO-002 | Origin C | Origin C | 用 C 语言扩展 Origin，编写函数、工具和自动化程序 | Core | `O1` `O13` |
| ORG-AUTO-003 | Python | Python | 在 Origin 中调用 Python，或从 Python 控制 Origin | Core | `O1` `O13` |
| ORG-AUTO-004 | R/Rserve | R and Rserve Console | 通过 R/Rserve 在 Origin 中执行 R 分析 | Core | `O1` `O13` |
| ORG-AUTO-005 | MATLAB Console | MATLAB Console | 在 Origin 中连接 MATLAB 并交换数据/命令 | Core | `O1` `O13` |
| ORG-AUTO-006 | Mathematica Link | Mathematica Link | 在 Origin 与 Mathematica 之间交换数据/结果 | Core | `O1` `O13` |
| ORG-AUTO-007 | LabVIEW SubVIs | LabVIEW SubVIs | 用 LabVIEW SubVI 与 Origin 交互 | Core | `O1` `O13` |
| ORG-AUTO-008 | COM/Automation Server 与 Orglab | COM/Automation Server and Orglab | 通过 COM 自动化 Origin；Orglab 允许自定义 App 通过 COM 读写文件 | Add-on / Unconfirmed | `O1` `O13` |
| ORG-AUTO-009 | 自定义菜单、工具栏与脚本 | Custom Menus, Toolbars and Scripts | 创建自定义菜单/工具栏、脚本和自动化入口 | Core | `O1` `O13` |

本章节覆盖编程和外部连接；MATLAB 文件导入归 §4.3，批处理归 §4.18。COM/Orglab 的授权方式未逐项核实，标为 Add-on/Unconfirmed。

### 4.20 Apps 与扩展

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-APPS-001 | App Center | App Center | 在 Origin 内浏览、安装和管理 Apps | Core | `O1` `O7` |
| ORG-APPS-002 | File Exchange | File Exchange | 官方 App/扩展目录，提供可下载的 App 和工具 | Core | `O1` `O8` |
| ORG-APPS-003 | 安装与更新 | Install and Update Apps | 安装、更新、启用/禁用 App，并管理依赖 | Core | `O1` |
| ORG-APPS-004 | App 分类与搜索 | App Categories and Search | 按领域/功能浏览和搜索 App | Core | `O7` |
| ORG-APPS-005 | App 开发 | App Development | 用 LabTalk/Origin C 等开发自定义 App 并发布 | Core | `O1` `O13` |
| ORG-APPS-006 | App 文档与集成 | App Documentation and Integration | App 自带说明、示例和菜单/工具栏集成 | Core | `O7` `O8` |

本章节只计 App 发现/安装/开发机制；具体 App 按主要功能归入对应章节，不在此重复计数。Graph Maker 归 §4.4，Data Factory 归 §4.18，统计类 App 归 §4.11。

### 4.21 性能与规模

| ID | 功能（中文） | 官方名称（English） | 说明 | 版本/可用性 | 来源 |
|---|---|---|---|---|---|
| ORG-PERF-001 | 大数据导入速度 | Fast Import for Large Data | 用多核架构加速大文件导入；具体速度属于厂商声明 | Core / 厂商声明 | `O1` |
| ORG-PERF-002 | 多线程列公式 | Multi-threaded Column Formula | 2026 起列向量计算支持多线程；具体提速倍数属于厂商声明 | Core / 2026 新增 / 厂商声明 | `O5` `O21` |
| ORG-PERF-003 | 项目保存速度 | Project Save Speed | 2026 起保存包含大量混合序列的项目更快；具体倍数属于厂商声明 | Core / 2026 新增 / 厂商声明 | `O5` `O21` |
| ORG-PERF-004 | 大矩阵与 3D | Large Matrix and 3D | 支持大矩阵和 3D 图形；具体上限随版本/硬件变化 | Core | `O1` |
| ORG-PERF-005 | 性能声明 | Performance Claims | 导入、计算、保存等速度数字来自厂商，未在本机验证 | 厂商声明 | `O1` `O5` |

本章节只记录性能相关能力和厂商声明；不把这些数字写成实测结论，也不推导任何后续产品的性能预算。

## 5. 版本新增

### 5.1 Origin 2026（2025-11）

| 类别 | 新增/改进 | 来源 |
|---|---|---|
| 数据 | Flash Fill、Smart Formula、项目内查找、单元格公式改进（Excel 公式导入、错误高亮、公式内查找替换） | `O5` |
| 数据连接 | 5 秒到 3 天的周期导入、XML 连接器检测 Unix 时间戳、改进 Minitab 连接器 | `O5` |
| 绘图 | 新 Graph Maker、新图型（因果图、频率图、双 X、茎叶图等）、统计图分组支持 | `O5` |
| 统计 | Graphical Summary、假设检验结论图、Logistic Regression 等新 App | `O5` |
| 界面 | 更好的工具栏/项目图标、保存/加载 Graph Options 设置、精确控制图中表格字体大小 | `O5` |
| 效率 | 多线程列公式、公式查找替换和大型项目保存提速、重复上一步/重开上次对话框 | `O5` |
| Apps | Reliability and Survival Analysis、Design of Experiments、Decision Tree、SPC、Logistic Regression、Ivium/RPC3/DMD 连接器等 | `O5` |

### 5.2 Origin 2026b（2026-06）

| 类别 | 新增/改进 | 来源 |
|---|---|---|
| 数据 | Mirror Columns、向其他工作表添加/插入列、Life Science GUI 结构化表、Prism 文件导入 | `O2` |
| 绘图 | 斜率标注、数据标注迷你对话框、跨图层参考线、按列标签行添加参考线、更多 Append 模式、吸附 | `O2` |
| 图形交互 | 隐藏分析锁、图形/浮动图重缩放、Graph Options 更多属性、轴断点迷你按钮、输出曲线改参数、当前图层指示器 | `O2` |
| 统计/质量 | 新假设检验工具、ANOVA/比例检验的汇总/诊断/功效分析、DOE 二元响应/变异性/最优设计/叠加等高线、可靠性分布/识别/概览/寿命计划/寿命回归 | `O2` |
| 通用 | 导入后移除连接器、批量复制工作表/工作簿、添加列时图形右移、Notes 导出 MHTML、结果表迷你按钮、项目内查找数据单元格 | `O2` |
| 数据细节 | Excel 粘贴/导入去除多余空格、复制本地函数名、Flash Fill 支持逗号小数点、SVG 导入工作表单元格 | `O2` |
| 新图型 | Overlaid Contour、Individual Value Chart | `O2` |
| Apps | Acceptance Sampling、Random Forest、Stability Study、Predictive Modeling Advisor、GraphPad Prism Connector、Reliability/Survival、SPC、DOE、Decision Tree、Graph Maker、Data Factory | `O2` |

版本说明：2026 和 2026b 的新增项只列官方页面明确命名的能力，不把长期既有功能写成版本新增；2026b SR1/SR2 的修补项未逐项展开。

## 6. 能力边界与未确认项

### 6.1 平台与版本

- Origin 是 Windows 应用；本机未安装，不能做界面、性能或数值验证。
- Origin 基础版与 OriginPro 的能力不同；Pro 判定优先使用官方对比页和产品页 PRO 标记，无法确认时标 `Unconfirmed`。
- App、Connector、Add-on 是不同扩展/授权形态；App 可能为第三方，安装前需单独核对许可。
- 2026/2026b 新增项与长期既有功能分开记录；版本号不等于本机可用性。

### 6.2 未确认项

| 项目 | 状态 | 原因 |
|---|---|---|
| COM/Automation Server 授权 | 未确认 | 官方页面提到 COM/Orglab，但未逐项核对授权/版本 |
| 具体 App 的许可与维护 | 未确认 | App Center/File Exchange 可能包含第三方 App |
| 100+ 图型的具体数量 | 厂商声明 | 官方页面称 100+，未逐项核对每个图型 |
| 1M+ 用户 | 厂商声明 | 营销页数字，未独立验证 |
| 导入/计算/保存速度 | 厂商声明 | 官方给出倍数，但未在本机或统一基准下复测 |
| 每个对话框选项 | 未覆盖 | 本报告按模块/命名能力汇总，不逐项枚举参数 |

### 6.3 不作出的判断

- 不判断 Origin 的功能质量、性能、稳定性或易用性。
- 不判断其与任何其他产品的优劣、差距或优先级。
- 不把官方页面出现的能力写成任何后续产品的需求、承诺或实现计划。

## 7. 来源登记

| 代码 | 来源 | 版本/日期 | 用途 | 限制 |
|---|---|---|---|---|
| `O1` | https://www.originlab.com/Origin | 2026b；核对日 2026-10-30 | 官方 19 类功能、PRO 标记、产品定位 | 营销页，不是完整手册 |
| `O2` | https://www.originlab.com/2026b | 2026b；核对日 2026-10-30 | 2026b 新增/改进 | 亮点页，非逐项发布说明 |
| `O3` | https://www.originlab.com/ReleaseNotes/index.aspx?v=2026b | 2026b；核对日 2026-10-30 | 2026b 发布说明入口 | 动态页面；以核对日为准 |
| `O4` | https://www.originlab.com/index.aspx?go=PRODUCTS&PID=1834 | 2026b；核对日 2026-10-30 | Origin vs OriginPro 对比、Pro 判定 | 未列项不得猜 Pro |
| `O5` | https://www.originlab.com/whatsnew | 2026/2026b；核对日 2026-10-30 | 版本与新增摘要 | 摘要页，非逐项清单 |
| `O6` | https://www.originlab.com/GraphGallery | 2026b；核对日 2026-10-30 | 图型家族/图型名称 | 仅名称，不证明本机行为 |
| `O7` | https://www.originlab.com/Apps | 2026b；核对日 2026-10-30 | App Center/应用 | 应用可能为第三方 |
| `O8` | https://www.originlab.com/FileExchange/ | 2026b；核对日 2026-10-30 | App/扩展目录 | 同上 |
| `O9` | https://www.originlab.com/index.aspx?go=Support/DocumentationAndHelpCenter/Installation/RunOriginonaMac | 核对日 2026-10-30 | Windows 运行说明 | 证明非原生 macOS |
| `O10` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/SignalProcessing | 核对日 2026-10-30 | 信号处理方法与 Pro 标记 | 子页可能随版本更新 |
| `O11` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/Mathematics | 核对日 2026-10-30 | 数学/插值/微积分/矩阵 | 同上 |
| `O12` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/DataManipulation | 核对日 2026-10-30 | 数据处理/重排/缩减 | 同上 |
| `O13` | https://www.originlab.com/index.aspx?go=Products/Origin/Programming | 核对日 2026-10-30 | 编程/连接 | 同上 |
| `O14` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/ImageProcessing | 核对日 2026-10-30 | 图像处理与 Pro 标记 | 同上 |
| `O15` | https://www.originlab.com/index.aspx?go=Products/Origin/HandlingRepetitiveTasks | 核对日 2026-10-30 | 批量/模板/重复任务 | 同上 |
| `O16` | https://www.originlab.com/doc/Origin-Help/ExpGraph-to-Image | 核对日 2026-10-30 | 图形导出格式（17 种） | 帮助页，非完整手册 |
| `O17` | https://www.originlab.com/index.aspx?go=Products/Origin/Graphing | 核对日 2026-10-30 | 图型家族与绘图能力 | 营销页 |
| `O18` | https://www.originlab.com/index.aspx?go=Products/Origin/Statistics | 核对日 2026-10-30 | 统计方法与 Pro 标记 | 子页可能随版本更新 |
| `O19` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/CurveFitting | 核对日 2026-10-30 | 曲线/曲面/隐函数拟合 | 同上 |
| `O20` | https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/PeakAnalysis | 核对日 2026-10-30 | 峰值分析 | 同上 |
| `O21` | https://www.originlab.com/2026C | 2026；核对日 2026-10-30 | Object Manager、图例/标注等界面细节 | 中文官方亮点页 |

## 8. 统计摘要与使用说明

### 8.1 条目统计

| 章节 | ID 前缀 | 条目数 |
|---|---|---:|
| 4.1 界面、项目与窗口 | `ORG-UI` | 10 |
| 4.2 数据表、矩阵与数据准备 | `ORG-DATA` | 13 |
| 4.3 导入、连接器与数据库 | `ORG-IO` | 12 |
| 4.4 绘图与图型 | `ORG-GRAPH` | 15 |
| 4.5 图形定制、布局与出版 | `ORG-PLOT` | 11 |
| 4.6 交互探索与数据过滤 | `ORG-EXPLORE` | 8 |
| 4.7 重算与关联更新 | `ORG-RECALC` | 7 |
| 4.8 Gadgets 快速分析 | `ORG-GADGET` | 14 |
| 4.9 曲线、曲面与隐函数拟合 | `ORG-FIT` | 11 |
| 4.10 峰值分析 | `ORG-PEAK` | 9 |
| 4.11 统计、质量与机器学习 | `ORG-STAT` | 19 |
| 4.12 信号处理 | `ORG-SIGNAL` | 12 |
| 4.13 数学与矩阵运算 | `ORG-MATH` | 10 |
| 4.14 数据处理与重排 | `ORG-PREP` | 10 |
| 4.15 RS/GIS 与图像处理 | `ORG-RSGIS` / `ORG-IMAGE` | 10 |
| 4.16 报告、布局与演示 | `ORG-REPORT` | 8 |
| 4.17 导出与互操作 | `ORG-EXPORT` | 9 |
| 4.18 批处理 | `ORG-BATCH` | 7 |
| 4.19 编程、自动化与连接 | `ORG-AUTO` | 9 |
| 4.20 Apps 与扩展 | `ORG-APPS` | 6 |
| 4.21 性能与规模 | `ORG-PERF` | 5 |
| **合计** |  | **215** |

### 8.2 使用说明

- 引用单条功能时使用 `ORG-*` ID，并同时注明来源代码；需要回查原始页面时使用 §7 的链接。
- `Pro` 表示官方对比页或产品页 PRO 标记支持；`App`、`Connector`、`Add-on`、`Unconfirmed` 分别表示扩展/授权形态；`厂商声明` 未经本机验证。
- 本报告按功能模块汇总，不逐项枚举每个对话框选项；具体参数、默认值和快捷键需回查官方帮助页。
- 本报告是竞品功能清单，不包含产品差距、优先级、路线图或需求变更。任何后续产品决策都应回到项目自身的需求、方法和验收文档。
