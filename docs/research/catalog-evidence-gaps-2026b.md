# Catalog evidence gaps: fitting, peaks, graphs and export

**Research date: 2026-10-30 UTC (2026-10-31 local).** Scope: P0-005, following the accepted P0-003 decomposition. This appendix closes or narrows the four evidence gaps recorded in the [P0-003 evidence](capability-catalog-decomposition.md) and its unknown register. It does not modify the 428-row requirement register, the planning validator, functional scope, product code or build configuration. It does not claim catalog completeness, parity, runtime behavior or format support.

## 1. Decision summary

| Gap | Finding | Status | Residual unknown |
|---|---|---|---|
| Fitting functions | The public Origin Help appendix 32.1 states that all available built-in curve-fitting functions are listed there. It contains 239 numbered function pages in 22 categories; its category tables contain 231 unique names; per-function pages carry equations, parameters, bounds and categories. Vendor count statements remain inconsistent: more than 170, close to 200, more than 200 and 200+. | Page/name enumeration resolved; completeness not claimed; count conflict narrowed but not vendor-reconciled; 2026b release pin open | Ten numbered pages are absent from the category tables; the Origin versus OriginPro function split is unpublished; no versioned 2026b help artifact was found |
| Peak functions | The 1D Peak Analyzer table lists 28 named functions: 7 bold Pro-only entries and 21 shared with curve fitting. The surface-fitting table lists 22 names. Over 25 is the 1D Peak Analyzer scope; over 20 surface functions is the surface scope. | Name evidence and the 25-versus-20 scope conflict resolved; completeness not claimed; surface 21-versus-22 and 2026b pin open | The OriginPro guide says 21 surface models; no surface-peak-only subset is published; no versioned 2026b help artifact |
| Graph types | The P0-003 73 named rows are exactly the Graph Gallery's 68 specific filters plus five examples. They are not a partial inventory of the 100+ Plot-menu types. The current product page enumerates 111 graph labels, including duplicate pairs and two explicitly App-labelled tools. The 100+ claim is a built-in-plus-extended and Plot-menu aggregate. | The prior 27-entry-gap inference is withdrawn; the claim is corrected; App classification is partial | No unique counting rule; most labels are unlabelled for App status; the 2026b attribution of Individual Value Chart is unverified |
| Graph export formats | The export help sentence asserts 17 image formats but its table lists 16 distinct formats. The current publishing guide lists 15 because Adobe Illustrator is hidden. The X-Function text says 14 while its option list contains 16 values. No checked official page names a seventeenth graph image format. | The seventeenth-format gap is closed as a vendor documentation error; the claim is corrected to 16 documented and 15 currently listed | Current 64-bit visible set is 13 or 14, not runtime-verified; the register still says 17 until a separately authorized register revision |
| Catalog roll-up | The six catalog gates and EG-ADD-015 remain open. No vendor count is an acceptance threshold and no parity claim is available. | Open | Every gate requires its own fixtures, version matrix and review |

The four gaps are resolved in different degrees: fitting and peak membership are now evidenced; the graph comparison is corrected; the export count is corrected. Residual version, counting-basis, classification and runtime questions are retained explicitly in section 11 rather than filled by inference.

## 2. Method, evidence labels and rights

### 2.1 Evidence labels

- **Documented fact**: a statement, table entry or named item on a retrieved vendor page at the retrieval date. It is not runtime behavior.
- **Vendor claim**: a count or capability asserted by the vendor without a published complete enumeration or counting rule. It is recorded as a claim.
- **Source observation**: a statement in a preserved inventory or planning document with a file and line anchor.
- **Inference**: a proposed interpretation for later review, never presented as vendor fact.
- **Unknown**: membership, counting basis, version identity, rights or runtime behavior that the available evidence does not resolve.

### 2.2 Retrieval and counting method

All sources are public vendor HTML pages retrieved with an ordinary HTTP client on 2026-10-30 UTC. The source register has 27 entries but 26 unique URLs because F10 and G5 are the same 2026b page. The documentation site returned HTTP 200, server `GitHub.com`, `Last-Modified: Mon, 26 Oct 2026` (08:55-09:00 UTC, varying by page), and response `Date: Fri, 30 Oct 2026 16:03:20 GMT` for the catalog pages. The product and gallery pages returned HTTP 200 from `Microsoft-IIS/10.0` with response dates in the same minute. The local date at verification was 2026-10-31.

Counts in this appendix are extraction results, not vendor-published totals. The method was:

1. Fetch the page and parse its HTML with a standard parser.
2. For the fitting appendix, read the sidebar entries numbered `32.1.1` through `32.1.239` and the 22 category tables separately. Count numbered pages, category count, category-table name cells, unique names and set differences. Do not treat a category membership as a separate function.
3. For the peak and surface appendices, count table cells and inspect bold markup for the Pro-only markers.
4. For the graph gallery, read the graph-type select options and remove three navigation entries (`Overview`, `Search`, `All`) and two aggregate labels (`Analysis`, `Advanced Statistical Graphs`).
5. For the product page, read the 111 list items under the `Graph Types` heading and the 65 list items under `Extended Templates from OriginLab Website`.
6. For export, count format rows in the help table, format names in the publishing guide and option values in the X-Function reference.

A live page can change after the retrieval date. A later check must record a new retrieval date, the response headers and a new excerpt hash before it revises a count.

### 2.3 Version identity

The current vendor release is Origin/OriginPro 2026b: the product navigation names 2026b and the [2026b feature page](https://www.originlab.com/2026b) is present. The Help pages do not display a release banner, a last-updated date or a version selector. A `/2026b/` documentation path and an `/origin-help/2026b/` path returned HTTP 404. The Help build is dated 2026-10-26 by its HTTP metadata, but its exact binding to an installed 2026b build is an inference. Every source below therefore has a dated live-build identity, not a frozen release artifact. This is the main reason the versioned-2026b part of AC-1 and AC-2 remains open.

### 2.4 Rights review

| Item | Assessment |
|---|---|
| Source material | Public vendor HTML pages. No login, paywall, manual PDF, SDK, installer, sample project or proprietary file corpus was accessed. |
| Use in this appendix | Citation and short quotation only. Function names, format names and counts are factual references. Full vendor tables, equations, descriptions or manuals are not reproduced or redistributed. |
| Vendor content | Origin and OriginPro are proprietary products. The pages remain vendor-copyrighted. Product names are used nominatively for identification. |
| Capability rights | Public documentation is behavioral evidence, not a license to copy vendor implementation, function files, templates, icons, format implementations or SDKs. Fitting and peak functions remain `R-VENDOR` implementations; graph types remain EveryGraph-defined scenes or separately gated; export formats retain their own specification and encoder rights. |
| Legal status | This is an engineering rights assessment for planning, not legal advice. A distribution or implementation decision still requires the dedicated license review in the project policy. |

## 3. Source register

All rows were retrieved on 2026-10-30 UTC, with HTTP 200 unless stated. `Live help` means a public `docs.originlab.com` page without a release banner; `Live page` means a public `originlab.com` page without a release banner. Source rights are citation-only public vendor HTML for every row.

| ID | Source | Identity and version | Supports | Limits |
|---|---|---|---|---|
| F1 | [Origin and OriginPro product page](https://www.originlab.com/origin) | Live product page; navigation names 2026b; no release banner or date | Fitting, peak, graph and export marketing claims; 111 graph labels; 65 extended templates | Marketing aggregate; no counting rule; no function or format enumeration for fitting and peaks |
| F2 | [Curve and Surface Fitting](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/CurveFitting) | Live product page; no release banner | More than 170 built-in fitting functions; Pro-only surface, compare and implicit fitting | No list or count basis; conflicts with F3, F4, F11 |
| F3 | [NLFit Quick Start](https://docs.originlab.com/origin-help/nlfit-quickstart/) | Live help 15.3.1; no release banner | More than 200 built-in fitting functions | No list or count basis |
| F4 | [32.1 Curve Fitting Functions](https://docs.originlab.com/origin-help/curve-fitting-function/) | Live help appendix 32.1; Last-Modified 2026-10-26 | Complete built-in curve-fitting library; 239 numbered pages; 22 categories; 231 unique category-table names | Live and unversioned; 10 numbered pages absent from category tables; no per-function edition split |
| F5 | [32.1.85 Gauss](https://docs.originlab.com/origin-help/gauss-fitfunc/) | Live help; no release banner | Equation, four parameters, bounds, script access, function file and categories | One sampled function; not an exhaustive parameter audit |
| F6 | [32.1.87 Gaussian](https://docs.originlab.com/origin-help/gaussian-function-fitfunc/) | Live help; no release banner | Equation, four parameters and peak-function category | One sampled function |
| F7 | [32.1.125 Lorentz](https://docs.originlab.com/origin-help/lorentz-fitfunc/) | Live help; no release banner | Equation, four parameters and multiple category memberships | One sampled function |
| F8 | [32.3.8 Gauss2D](https://docs.originlab.com/origin-help/gauss2d-fitfunc/) | Live help; no release banner | Surface equation and six parameters; supports the 2D Gaussian example | One sampled surface function |
| F9 | [OriginPro user guide](https://docs.originlab.com/user-guide/originpro/) | Live guide; no release banner | 21 built-in surface fitting models including Gauss2D | Conflicts with the 22-entry surface table in F4/P4; not a per-function edition split |
| F10 | [Origin 2026b Feature Highlights](https://www.originlab.com/2026b) | Release 2026b page | 2026b release identity; no fitting-function count or built-in catalog change listed in the highlights | Highlights are not exhaustive; the page does not document a change to the 32.1 library |
| F11 | [User Defined Fitting Function tutorial](https://docs.originlab.com/tutorials/userdef-fitfunc/) | Live tutorial; no release banner | The phrase 200+ built-in fitting functions | Tutorial copy; no list or count basis |
| P2 | [Peak Analysis](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/PeakAnalysis) | Live product page; no release banner | Surface Peak Fitting PRO uses over 20 built-in functions | No names or surface-peak-only subset |
| P3 | [32.4 Peak Analyzer Functions](https://docs.originlab.com/origin-help/peak-analyzer-functions/) | Live help appendix 32.4; Last-Modified 2026-10-26 | 28 named 1D Peak Analyzer functions; 7 bold Pro-only entries; 21 shared names | No vendor total; no surface-peak subset; live and unversioned |
| P4 | [32.3 Non-linear Surface Fitting](https://docs.originlab.com/origin-help/non-linear-surface-fitting-fitfunc/) | Live help appendix 32.3; Last-Modified 2026-10-26 | 22 named surface-fitting functions | General surface library, not a surface-peak-only list; live and unversioned |
| P5 | [Peak Analyzer, Fit Peaks page](https://docs.originlab.com/origin-help/peakanalyzer-fitpeak/) | Live help 19.1.10.11; no release banner | The Fit Peaks page is for OriginPro only | Workflow edition statement, not a function list |
| P6 | [32.4.2 BWF](https://docs.originlab.com/origin-help/bwf-pafunc/) | Live help; no release banner | Pro-only peak equation and five parameters | One sampled Pro-only function |
| G2 | [Origin Graph Gallery](https://www.originlab.com/GraphGallery) | Live gallery; no release banner | 73 select options: three navigation entries, 68 specific graph filters and two aggregate labels | Rotating showcase; not a product catalog; no 100+ string in the page body |
| G3 | [10 Graphing](https://docs.originlab.com/user-guide/graphing/) | Live help; Last-Modified 2026-10-26 | Over 100 graph types using built-in templates; App labels; Plot-menu tables | Stale relative to 2025b and 2026; no Treemap, Frequency, Decision Tree, Interaction, Run Chart or Symmetry; tables include App tools |
| G4 | [Origin 2026 Feature Highlights](https://www.originlab.com/2026) | Release 2026 page | Ten named new graph types and the 2026 App list | No core/App label per type; no release date on the page |
| G5 | [Origin 2026b Feature Highlights](https://www.originlab.com/2026b) | Release 2026b page | No new graph-type section; Overlaid Contour appears under DOE; Individual Value Chart is absent | Highlights are not exhaustive; inventory attribution remains possible but unverified |
| G6 | [Origin 2025b press release](https://www.originlab.com/index.aspx?go=COMPANY/NEWSANDEVENTS/PRESSROOM&pid=5474) | Release 2025b, 19 June 2025 | Ten named new graph types in 2025b | No core/App label; marketing release |
| E1 | [Exporting Your Graph or Layout Page to an Image File](https://docs.originlab.com/origin-help/expgraph-to-image/) | Live help 23.2; Last-Modified 2026-10-26 | The 17-format assertion; a 16-row format table | Self-contradictory; live and unversioned; omits the 64-bit CGM/DXF caveat |
| E2 | [17 Publishing and Export](https://docs.originlab.com/user-guide/publishing-and-export/) | Live guide; Last-Modified 2026-10-26 | The expGraph routine exports to one of 15 file formats; lists 8 raster and 7 vector formats | Excludes hidden AI; includes CGM and DXF despite E3's 64-bit note |
| E3 | [2.7.10 expGraph](https://docs.originlab.com/x-function/ref/expgraph/) | Live X-Function reference; Last-Modified 2026-10-26 | Fourteen-format sentence; 16 option values; AI hidden since 2017; CGM and DXF not supported in 64-bit | Stale count text; option list is the stronger evidence |
| E5 | [Origin 2022b Feature Highlights](https://www.originlab.com/2022b) | Release 2022b page | SVG export was added in 2022b | Does not state a total count |
| E6 | [FAQ-828: export graph or layout as AI](https://docs.originlab.com/quick-help/export-graphs-as-ai/) | FAQ updated 2022-12-13 | AI Image type hidden from the Export Graph dialog since Origin 2017 | FAQ, not the format table |
| E7 | [Origin Technical Support Policy](https://www.originlab.com/index.aspx?go=Support/SupportPolicy) | Live policy page; no release banner | 32-bit support dropped in versions 2020 (9.7) and newer | General platform policy, not export-specific |

## 4. Fitting function catalog: ORG-FIT-004

### 4.1 What the evidence establishes

F4 is a public, vendor-authored appendix. It states: "All available built-in curve fitting functions are listed here. These functions can be accessed from the Nonlinear Curve Fit tool." Its sidebar numbers the function pages from `32.1.1 Allometric1` to `32.1.239 YldFert1`. The page has 22 category headings: Origin Basic Functions, Convolution, Exponential, Growth/Sigmoidal, Hyperbola, Logarithm, Peak Functions, Piecewise, Polynomial, Power, Rational, Waveform, Chromatography, Electrophysiology, Enzyme Kinetics, Pharmacology, Rheology, Spectroscopy, Statistics, Quick Fit, Multiple Variables and Enzyme Kinetics 2.

The per-function pages are structured. F5, for example, gives the Gauss equation, four parameters named `y0`, `xc`, `w` and `A`, a lower bound `w > 0.0`, script access `nlf_gauss` and categories "Origin Basic Functions, Peak Functions, Chromatography, Electrophysiology, Statistics". F6 gives the Gaussian FWHM form with four parameters; F7 gives Lorentz with four parameters; F8 gives the Gauss2D surface form with six parameters. The catalog therefore carries equation and parameter evidence at the function-page level, not only a name list.

### 4.2 Extracted structure and counts

| Measure | Count | Method and limit |
|---|---:|---|
| Numbered `32.1.x` function pages | 239 | Sidebar entries `32.1.1` through `32.1.239`; unique and contiguous; this is a page count, not a count of distinct selectable models |
| Category headings | 22 | `h2` headings excluding `Contents` |
| Category-table name cells | 307 | Memberships across all 22 tables; a function appears once per category membership |
| Unique category-table strings | 231 | Includes one typo and one cross-appendix name; not a clean vendor unique count |
| Numbered pages absent from category tables | 10 | `BoxLucas2`, `ExpAssoc1`, `ExpAssoc2`, `ExpAssocDelay1`, `ExpAssocDelay2`, `NormalCDF`, `PIPlatt`, `PIPlatt2`, `PIWebb`, `SchulzFlory` |
| Category-table names absent from the 32.1 numbering | 2 | `:DoubleBoltzmann` is a leading-colon typo for `DoubleBoltzmann`; `Bigaussian` is a 32.4 Peak Analyzer function |
| Surface appendix `32.3.x` entries | 22 | Separate surface catalog, not part of the 239 |
| Peak Analyzer `32.4.x` entries | 28 | Separate peak catalog, not part of the 239 |

The 239-page and 231-name counts are not interchangeable. The appendix's own category tables do not contain ten numbered pages, and they contain two strings that are not 32.1 functions in the sidebar. A future CSV integration must resolve this structure before it declares a unique function count. The 239 figure is a defensible floor for numbered function pages; the 231 figure is a defensible count of category-table names after explicit normalization; neither is a vendor-published total.

### 4.3 Count conflict

| Claim | Source | Exact wording | Version and date | Assessment |
|---|---|---|---|---|
| More than 170 | F2 | "The NLFit tool includes more than 170 built-in fitting functions, selected from a wide range of categories and disciplines." | Live product page; retrieved 2026-10-30 | Lowest current claim; likely stale or a narrower count, but no source says so |
| Close to 200 | F1 | "Select from close to 200 built-in fitting functions arranged in categories." | Live product page; current 2026b navigation; retrieved 2026-10-30 | Approximate marketing wording; below the 239-page appendix |
| More than 200 | F3 | "The nonlinear curve fitting (NLFit) tool includes more than 200 built-in fitting functions" | Live help; retrieved 2026-10-30 | Compatible with the 239-page appendix |
| 200+ | F11 | "Besides the 200+ built-in fitting functions, you can also create your own fitting functions in Origin." | Live tutorial; retrieved 2026-10-30 | Compatible with the 239-page appendix |
| 239 numbered pages | F4 | "All available built-in curve fitting functions are listed here." | Live help; Last-Modified 2026-10-26; retrieved 2026-10-30 | Strongest membership evidence; not a marketing count |

The current help, tutorial and appendix are mutually compatible at a 200-plus level. The more-than-170 page is the outlier; the close-to-200 page is a conservative approximation. This is an inference from the retrieved texts, not a vendor-confirmed reconciliation. No vendor page states which counting unit, version or edition produces 170, 200 or 239. The count conflict is narrowed to a counting-basis and version-drift problem; it is not officially reconciled.

### 4.4 Origin versus OriginPro and version identity

The 32.1 appendix carries no per-function Pro marker. Pro-only status is documented at the workflow and feature level, for example surface fitting, implicit fitting, compare models and the Peak Analyzer Fit Peaks page; it is not a per-function edition column. A per-function Origin versus OriginPro split remains unknown.

The appendix is live help with a 2026-10-26 build and no release banner. The 2026b feature highlights contain no fitting-function count and no documented change to the built-in 32.1 library. The exact binding of the live appendix to an installed 2026b build is an inference. This is the explicit version-pin gap retained under AC-1.

### 4.5 Impact on the fitting gap

The "no public catalog exists" premise is resolved: a lawful public appendix enumerates the library and its per-function equations and parameters. The full catalog is not copied into this repository; the vendor pages remain the reference. The membership evidence is resolved at the function-page level; completeness of the 239-page list is not claimed. The count-basis conflict, the 10 uncategorized pages, the per-function edition split and the 2026b release pin remain open.

## 5. Peak function catalog: ORG-PEAK-005

### 5.1 The 1D Peak Analyzer catalog

P3 states: "All functions available in the Peak Analyzer tool are listed below, some of them are shared with the built-in functions of curve fitting ... while some are unique in the Peak Analyzer (Pro Only), and these unique functions are marked bold in the list." The table has 28 name cells. Seven are bold and Pro-only: `BWF`, `FraserSuzuki`, `Bigaussian`, `HVL`, `ConsGaussian`, `DoniachSunjic` and `Gaussian_LorenCross`. The other 21 are shared names: `Asym2Sig`, `ECS`, `Gaussian`, `PearsonVII`, `SineDamp`, `Gauss`, `PsdVoigt1`, `SineSqr`, `GCAS`, `PsdVoigt2`, `Sine`, `CCE`, `GaussAmp`, `InvsPoly`, `Pulse`, `Voigt`, `GaussMod`, `LogNormal`, `SchulzFlory`, `Weibull3` and `Lorentz`.

The list is not deduplicated by the vendor. It contains both `Gaussian` and `Gauss`, and it includes waveform and spectroscopy names. After removing one possible Gaussian/Gauss duplicate, 27 entries remain, still above the over-25 claim. The count 28 is an extraction of the table, not a vendor total.

P5 states: "This page is for OriginPro only." The Fit Peaks workflow is an OriginPro workflow. P6 shows that the Pro-only BWF page has an equation and five parameters named `y0`, `xc`, `H`, `w` and `q`. F6 shows the shared Gaussian page with four parameters. Equation and parameter availability is therefore evidenced at the catalog level, not only as a claim.

### 5.2 The surface catalog

P4 lists 22 surface-fitting names: `Chebyshev2D`, `Cosine`, `DoseResp`, `Exponential2D`, `Extreme2D`, `ExtremeCum`, `Fourier2D`, `Gauss2D`, `GaussCum`, `Gaussian2D`, `LogisticCum`, `LogNormal2D`, `Lorentz2D`, `Parabola2D`, `Plane`, `Poly2D`, `Polynomial2D`, `Power2D`, `Rational2D`, `RationalTaylor`, `Voigt2D` and `Voigt2DMod`.

This is a general surface-fitting library. It includes clearly non-peak models such as `Plane`, `Poly2D`, `Polynomial2D`, `Rational2D` and `RationalTaylor`. The peak-shaped subset includes `Gauss2D`, `Gaussian2D`, `Lorentz2D`, `Voigt2D`, `Voigt2DMod` and `LogNormal2D`. No vendor page publishes a surface-peak-only count.

F9 states: "Choose from 21 built-in surface fitting models including Gauss2D, DoseResponse2D, Exponential2D, and Polynomial2D". P4 lists 22 entries and uses the name `DoseResp` rather than `DoseResponse2D`. Both 21 and 22 satisfy over 20; the one-model difference and the naming difference are an unresolved documentation inconsistency.

### 5.3 Scope reconciliation

| Claim | Source | Scope in the source | Evidence in the catalog |
|---|---|---|---|
| Over 25 built-in peak functions | F1 | Peak Fitting PRO, after baseline detection and peak finding | The 1D Peak Analyzer table has 28 entries, 7 Pro-only and 21 shared |
| Over 20 surface functions | F1 | Surface Fitting PRO | The surface table has 22 entries |
| Over 20 built-in functions | P2 | Surface Peak Fitting PRO | The surface table has 22 entries; no surface-peak subset is published |
| 25+ built-in peak functions | Preserved inventory, line 259 | Peak fitting generally | Consistent with the 28-entry 1D list |

The 25-plus and 20-plus claims describe different scopes: 1D Peak Analyzer fitting and surface fitting. They are not competing totals for one set. The scope conflict is resolved. The 21-versus-22 surface count, the absence of a surface-peak-only subset and the 2026b release pin remain open.

### 5.4 Version identity and impact

P3 and P4 are live help appendices with 2026-10-26 builds and no release banner. The exact 2026b binding is an inference. The peak name evidence is resolved; completeness is not claimed. The surface count, the surface-peak subset and the version pin remain open.

## 6. Graph type catalog: ORG-GRAPH-001

### 6.1 The 73-row correction

The [P0-003 evidence](capability-catalog-decomposition.md) section 5 records 73 named graph entries and states that the vendor claim exceeds the named-entry count by at least 27. That comparison is not valid. The 73 rows are exactly:

- the Graph Gallery's 68 specific graph-type filters; plus
- five additional examples: Cause and Effect diagram, Frequency Chart, Stem-and-Leaf, Individual Value Chart and Treemap.

The gallery select has 73 options in total: three navigation entries (`Overview`, `Search`, `All`), 68 specific filters, and two aggregate labels (`Analysis`, `Advanced Statistical Graphs`). The set comparison of the 68 specific filters against the 73 P0-003 names produced 68 matches after normalizing the ampersand in `Pie & Doughnut Chart` to `and`; exact-string comparison produced 67 of 68. The five examples are the only additions. The 73 rows are a gallery taxonomy, not 73 entries from a 100-plus Plot-menu inventory. The at-least-27 gap inference is therefore withdrawn. The umbrella claim still requires its own evidence; the prior comparison does not establish a 27-type gap.

### 6.2 The current product enumeration

The product page has 111 list items under the `Graph Types` heading. Two duplicate pairs are visible: `Treemap Plot` and `Treemap`; `Heat Map with Dendrogram` and `Heatmap with Dendrogram`. Two entries are explicitly App tools in the Help page: `Correlation Plot` and `Paired Comparison Plot`. Removing one label from each duplicate pair and the two explicit App tools leaves 107 labels (111 minus four). That arithmetic is an extraction aid, not a vendor counting rule: many entries are compound labels covering several variants, for example "Column & Bar Charts: Basic, Stacked, Grouped", "Scatter Plot (with X,Y Error Bar, Column Scatter, Drop Lines, Color Map, Size Map)" and "3D Symbol/Trajectory/Line/Vector Plot".

The product page separately lists 65 extended templates, with `Double-Y Stacked Columns` appearing twice, so 64 unique extended-template labels. It states: "In addition to built-in graph types, Origin also provides extended templates which can be found and installed from the OriginLab Website using the Template Center." Extended templates are not the same class as built-in Plot-menu types.

### 6.3 The 100-plus claim and its counting basis

The vendor uses four non-equivalent formulations:

| Formulation | Source | Exact wording |
|---|---|---|
| Built-in and extended | F1 | "With over 100 built-in and extended graph types and point-and-click customization of all elements" |
| Plot Menu | F1 | "Over 100+ graph types are available from the Plot Menu." |
| Built-in templates | G3 | "With Origin, you can create over 100 graph types using Origin's built-in graph templates." |
| Plot menu access | G3 | "All graph types are accessible from the Plot menu." |

The Help page's Plot-menu tables include App tools: it labels `Correlation Plot` and `Paired Comparison Plot` as APP tools and states that the app must be installed. The product page's `Graph Types` list includes those same two names. The product page's 111 labels therefore mix built-in-looking and explicitly App-labeled entries; the 65 extended templates are a separate list. No page states whether the count includes Apps, extended templates, style variants, compound labels, report-sheet charts or dialog commands. The 100-plus claim is credible as a label or variant count; it is not a certified count of 100-plus distinct built-in graph types.

The arithmetic is consistent with this reading. The 73-row gallery taxonomy plus 21 clear additions is about 94 at a family level, below 100; reaching 100 requires counting variants, compound labels or extended templates. That is an inference from the observed lists, not a vendor statement. The claim should not be used as a distinct-built-in-type threshold.

### 6.4 Named types absent from the 73 rows

The following current product-page or release-note names have no named row in the P0-003 73-row list. This is a source-backed list, not a complete membership list:

| Named type | Evidence, version and retrieval | Rights | Uncertainty | Classification note |
|---|---|---|---|---|
| CDF Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30. G6 uses `Normal CDF Plot` for 2025b. | `R-OWN` | `U2` | Built-in-looking; no explicit App label |
| Multi-Vari Chart | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Forest Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Poincare Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Voronoi Diagram | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Paired Comparison Plot | F1 live product page, 2026b navigation; G3 live Help build 2026-10-26; retrieved 2026-10-30 | `R-OWN` | `U4` | Explicit App tool in Help; App dependency gated |
| Tile Grid Map Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Means Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Venn Diagram | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Andrews Plot | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Manhattan Plot for GWAS | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Soil and Rocks Classification Diagrams | F1 live product page, 2026b navigation; retrieved 2026-10-30 | `R-OWN` | `U2` | Built-in-looking |
| Interaction Plot | F1 live product page, 2026b navigation; G6 2025b release, 19 June 2025; retrieved 2026-10-30 | `R-OWN` | `U2` | 2025b addition; App status unstated |
| Main Effects Plot | F1 live product page, 2026b navigation; G6 2025b release, 19 June 2025; retrieved 2026-10-30 | `R-OWN` | `U2` | 2025b addition; App status unstated |
| Run Chart | F1 live product page, 2026b navigation; G6 2025b release, 19 June 2025; retrieved 2026-10-30 | `R-OWN` | `U2` | 2025b addition; App status unstated |
| Symmetry Plot | F1 live product page, 2026b navigation; G6 2025b release, 19 June 2025; retrieved 2026-10-30 | `R-OWN` | `U2` | 2025b addition; App status unstated |
| Decision Tree | G4 2026 release page; retrieved 2026-10-30 | `R-OWN` | `U2` | 2026 graph type and 2026 App; App delivery is an inference |
| Grouped Histogram | G4 2026 release page; retrieved 2026-10-30 | `R-OWN` | `U2` | 2026 addition |
| Grouped Pareto Chart | G4 2026 release page; retrieved 2026-10-30 | `R-OWN` | `U2` | 2026 addition |
| Grouped Scatter Plot | G4 2026 release page; retrieved 2026-10-30 | `R-OWN` | `U2` | 2026 addition |
| Conclusion Charts in t-test Report Sheets | G4 2026 release page; retrieved 2026-10-30 | `R-OWN` | `U2` | 2026 addition in t-test report sheets |

The list is not exhaustive. It shows that the 73 rows are not a near-complete enumeration of the product page. It does not prove that every listed name is a distinct built-in type; several are variants, App tools or report outputs.

### 6.5 Built-in versus App-dependent

| Evidence class | Examples | Status |
|---|---|---|
| Explicit App tool in Help | `Correlation Plot`, `Paired Comparison Plot` | Documented; 2 of the 111 product labels |
| Explicit App in Help | `Graph Maker App` is pre-installed and launched from the Plot menu | Documented; not one of the 111 labels |
| Co-listed as graph type and App | `Decision Tree`, `Stem and Leaf Plot` in the 2026 release page | 2 named 2026 examples; strong inference of App delivery, not an explicit per-type statement |
| Unlabelled product entries | The remaining product labels, including the 107 after duplicate and explicit-App removal | Unknown; no complete built-in/App marker set |
| Extended templates | 65 website template entries, 64 unique labels | Separate class; not built-in; not App-labelled |

The P0-003 `D-GATE` marks for Chord, Sunburst, Network, Sankey and similar entries remain appropriate. They are not upgraded to built-in status by this evidence. The vendor pages do not provide a complete built-in versus App matrix.

### 6.6 2026b attribution

G4 lists ten new graph types in 2026: Cause-and-Effect Diagram, Frequency Chart, Double X, Decision Tree, Stem and Leaf Plot, Grouped Histogram, Grouped Pareto Chart, Grouped Scatter Plot, Improved Grouped Column Chart and Conclusion Charts in t-test report sheets. G6 lists ten new graph types in 2025b. G5 has no new graph-type section; `Overlaid Contour` appears under Design of Experiments, and `Individual Value Chart` is absent from the 2026b page. The preserved inventory attributes both Overlaid Contour and Individual Value Chart to 2026b at line 473. The official 2026b highlights do not confirm that attribution. This is a release-note discrepancy, not proof that the inventory is wrong; it is retained as an unknown.

### 6.7 Impact on the graph gap

The graph gap is corrected rather than closed. The P0-003 73-row count is a gallery taxonomy, not a partial 100-plus inventory; the prior 27-entry-gap statement is withdrawn. The current product page provides 111 labels and the Help page provides a stale built-in-template description. The 100-plus claim is credible as a label count and as a built-in-plus-extended aggregate, but it is not a certified distinct-built-in-type count. The exact membership, the unique counting rule, the App classification of most entries and the 2026b attribution remain open.

## 7. Graph export catalog: ORG-EXPORT-001

### 7.1 The documented formats

E1 states: "Origin supports these 17 image export file formats:" and then lists 16 distinct formats:

| Format | Extension | Class |
|---|---|---|
| Adobe Illustrator | `.AI` | Vector |
| Bitmap | `.BMP` | Raster |
| Computer Graphics Metafile | `.CGM` | Vector |
| AutoCAD Drawing Interchange | `.DXF` | Vector |
| Encapsulated PostScript | `.EPS` | Vector |
| Enhanced Metafile | `.EMF` | Vector |
| Graphics Image Format | `.GIF` | Raster |
| JPEG | `.JPG` | Raster |
| Zsoft PC Paintbrush Bitmap | `.PCX` | Raster |
| Portable Network Graphics | `.PNG` | Raster |
| Truevision Targa | `.TGA` | Raster |
| Portable Document Format | `.PDF` | Vector |
| Adobe PhotoShop | `.PSD` | Raster |
| Scalable Vector Graphics | `.SVG` | Vector |
| TIFF, Tag Image File | `.TIF` | Raster |
| Windows Metafile | `.WMF` | Vector |

### 7.2 Count conflict

| Claim | Source | Exact wording | Assessment |
|---|---|---|---|
| Over 15 | F1 | "Export graphs and layout pages to over 15 file formats: SVG, EPS, PDF, EMF, TIFF, PNG, JPEG, BMP, and several more" | Consistent with 15 or 16; no enumeration |
| 17 | E1 | "Origin supports these 17 image export file formats:" | Contradicted by E1's own 16-row table; no seventeenth name |
| 16 rows | E1 | The table contains AI, BMP, CGM, DXF, EPS, EMF, GIF, JPG, PCX, PNG, TGA, PDF, PSD, SVG, TIF and WMF | Strongest documented union |
| 15 | E2 | "The expGraph routine exports graphs to one of 15 file formats" | Current guide excludes hidden AI; lists 8 raster and 7 vector formats |
| 14 text, 16 options | E3 | "Totally 14 image file formats are supported for exporting to image file" while the option list has `{0}` through `{15}` | Stale count text; the option list has 16 values |
| 13 or 14 visible | Inference from E3 and E7 | AI hidden since 2017; CGM and DXF not supported in 64-bit; 32-bit dropped in 2020 | Runtime-dependent; not tested |

The format aliases do not explain the 17. `JPG` and `JPEG` are one table row; `TIF` and `TIFF` are one table row. Counting both aliases separately would produce 18, not 17. Multi-page PDF and multi-frame TIF are options on the PDF and TIF rows, not separate formats. 2026b's new export items are a Notes document as MHTML and an Origin image-window file as OGGU; neither is a graph image format.

No checked official page names a seventeenth graph image format. Candidate names such as OGG, SVGZ, JP2, WebP, HEIC, EMF+, PDF/A and PPTX were not found in any checked official export page. The 17 is best classified as a vendor documentation error or an off-by-one count, not as an undocumented format.

### 7.3 Corrected claim

The defensible statement is: Origin documents 16 graph image export formats in the export help table; the current publishing guide lists 15 because Adobe Illustrator is hidden; the X-Function reference's fourteen-format sentence is stale against its own 16 option values. A current 64-bit installation may expose 13 visible formats (16 minus hidden AI and unsupported CGM/DXF) or 14 with `@HAI=0`; that is an inference and was not runtime-tested. The register's 17-format acceptance text should be corrected in a future register revision. P0-005 does not change the register, the scope or the validator.

### 7.4 Impact on the export gap

The seventeenth-format gap is closed as a vendor documentation error. The source conflict among over 15, 17 and 16 named formats is recorded, and the claim is corrected to 16 documented and 15 currently listed. The current 64-bit visible set, the encoder licenses and the runtime behavior remain open. No export support claim is made.

## 8. Rights assessment for the revised evidence

| Catalog | Capability rights | Source use | What must not be claimed |
|---|---|---|---|
| Fitting | `R-VENDOR` implementations; equations are public mathematical descriptions | Public Help citation; short quotes; no FDF files or vendor code copied | That EveryGraph may copy Origin function files, initialization code or implementation |
| Peaks | `R-VENDOR`; Peak Analyzer Fit Peaks is OriginPro-only behavior | Public Help citation; no FDF files or vendor code copied | That the 28 names are all OriginPro-only or that the surface subset is published |
| Graphs | `R-OWN` for EveryGraph scenes; App-dependent types remain gated | Public product and Help citation; no templates, icons or assets copied | That the 100-plus claim is a certified built-in count or that App types are bundled |
| Export | Per-format rights; public specifications such as PDF, PNG, SVG and TIFF are `R-OPEN`; AI, CGM, DXF, EMF, WMF, PSD, PCX and TGA are `R-VENDOR` or legacy | Public Help citation; no format implementation, plugin or encoder copied | That a documented format is an implemented, licensed or tested encoder |

The rights review for this task is a citation-only review. It does not authorize acquisition or redistribution of a proprietary manual, SDK, template, sample project, image corpus or FDF file. It does not authorize a vendor runtime. Implementation and distribution decisions remain separate gates.

## 9. Revised control entries and gate status

These are proposed revisions to the P0-003 control rows, not changes to the authoritative register. The child IDs remain stable; no ID is reused or deleted.

| Child ID | P0-003 status | P0-005 revised evidence | Source, version and retrieval | Rights | Uncertainty | Gate effect |
|---|---|---|---|---|---|---|
| `EG-CAT-FIT-001` | Claim-only unenumerated library | Catalog located: 239 numbered 32.1 pages; 22 categories; 231 unique category-table names; per-function equations and parameters; count claims 170, close to 200, 200-plus and 239 | F4, live help build 2026-10-26; retrieved 2026-10-30; F1, F2, F3 and F11 for the claims | `R-VENDOR` | `U2` for membership; count basis and version pin remain `U3` | `EG-CAT-GATE-003` partially closed; count, edition split and version pin remain open |
| `EG-CAT-FIT-002` | 2D Gaussian caption example | Per-function Gauss2D page confirms a six-parameter surface equation; the OriginPro guide says 21 surface models | F8 live help build 2026-10-26 and F9 live OriginPro guide; retrieved 2026-10-30; 2026b current release inferred, exact binding unknown | `R-OPEN` for the mathematical form; vendor page citation only | `U2` | Supports the example; does not establish the selectable name or edition |
| `EG-CAT-PEAK-001` | Claim-only unenumerated library | 1D Peak Analyzer table has 28 names, 7 bold Pro-only and 21 shared; surface table has 22 names; over 25 is 1D and over 20 is surface | P3 and P4, live help build 2026-10-26; P2, P5, P6 and F9; retrieved 2026-10-30 | `R-VENDOR` | `U2` for membership; surface 21-versus-22 and version pin remain `U3` | `EG-CAT-GATE-004` partially closed; surface subset and version pin remain open |
| `EG-CAT-EXPORT-017` | Claim-only seventeenth format | No seventeenth documented; 16-row table and 15-format guide; retain the ID as a claim-correction row, not a format | E1, E2 and E3; live help/guide/reference build 2026-10-26; retrieved 2026-10-30 | `R-VENDOR` for the claim; documented format rights vary | `U3`; no named format; current 64-bit visible set untested | `EG-CAT-GATE-005` corrected; runtime and fixtures remain open |
| `EG-CAT-GRAPH-074` | Claim-only unenumerated types up to 100-plus | 73 P0-003 rows are 68 gallery filters plus five examples; product page has 111 labels; 100-plus is built-in-plus-extended and Plot-menu aggregate; 21 named additions identified | F1 live product page with 2026b navigation and G2 live gallery, retrieved 2026-10-30; G3 live Help build 2026-10-26; G4 2026 release page; G5 2026b release page; G6 2025b release, 19 June 2025; all retrieved 2026-10-30 | `R-OWN` for EveryGraph scenes; App-dependent types remain gated | `U3`; no unique counting rule; App classification partial | `EG-CAT-GATE-002` claim basis corrected; exact membership remains open |

The gate status is:

| Gate | P0-003 status | P0-005 status | Remaining closure condition |
|---|---|---|---|
| `EG-CAT-GATE-002` graph | Open | Claim basis corrected | Name the distinct built-in membership or state the vendor's counting rule; every named type has a PLOT scene |
| `EG-CAT-GATE-003` fitting | Open | Partially closed | Resolve the count basis, the 10 uncategorized pages, the edition split and the 2026b pin; add reference fits |
| `EG-CAT-GATE-004` peak | Open | Partially closed | Resolve the 21-versus-22 surface count, the surface-peak subset and the 2026b pin; add mixture tests |
| `EG-CAT-GATE-005` export | Open | Claim corrected | Record the 16 documented and 15 listed formats, the 64-bit caveat, encoder rights and fixtures |
| `EG-CAT-GATE-006` compatibility | Open | Unchanged | Rights, versions and corpora per artifact |
| `EG-CAT-GATE-007` method/App lists | Open | Unchanged | Reconcile method/option and App catalogs |
| `EG-ADD-015` roll-up | Open | Open | All six catalog gates and the cross-cutting gate |

## 10. Proposed expectation updates for a future catalog register

These values are evidence floors and counts for a future P0-004 integration or a later authorized task. They are not acceptance thresholds and they do not modify the validator. The original P0-003 constants remain part of the P0-003 record; this table proposes replacements for a new catalog-children register.

| Constant | Proposed value | Basis | Caution |
|---|---:|---|---|
| `FIT_NUMBERED_PAGES` | 239 | F4 sidebar `32.1.1` to `32.1.239` | Page count, not unique selectable models |
| `FIT_CATEGORY_COUNT` | 22 | F4 category headings | Category headings are not functions |
| `FIT_CATEGORY_UNIQUE_MIN` | 231 | F4 category-table unique strings | Includes a typo and a 32.4 name; normalize before use |
| `FIT_UNCATEGORIZED_PAGES` | 10 | F4 sidebar minus category tables | Names are listed in section 4.2 |
| `PEAK_1D_NAMED` | 28 | P3 table | Seven are bold Pro-only |
| `PEAK_1D_PRO_ONLY` | 7 | P3 bold entries | Vendor marks them as unique in Peak Analyzer |
| `PEAK_SURFACE_NAMED` | 22 | P4 table | Not a surface-peak-only list |
| `PEAK_SURFACE_VENDOR_CLAIM` | 21 | F9 | One-model documentation conflict |
| `GRAPH_GALLERY_FILTERS` | 68 | G2 specific select options | Three navigation and two aggregate labels are excluded |
| `GRAPH_PRODUCT_LABELS` | 111 | F1 Graph Types list | Includes duplicate pairs and App tools |
| `GRAPH_EXTENDED_TEMPLATES` | 65 | F1 extended-template list | One duplicate label; 64 unique |
| `EXPORT_NAMED` | 16 | E1 table and E3 option values | Includes hidden AI and 64-bit-unsupported CGM/DXF |
| `EXPORT_CURRENT_GUIDE` | 15 | E2 | Excludes hidden AI |
| `EXPORT_VISIBLE_64BIT_ESTIMATE` | 13 or 14 | Inference from E3 and E7 | Runtime-dependent; no test |

A validator that requires the vendor marketing counts (170, 200, 100-plus, 17) would force fabricated rows. A validator that requires the observed floors above would detect silent deletion while preserving the explicit claim rows and the unknown register.

## 11. Unknown register delta

The original P0-003 unknown register remains in place. This table records only changes and additions.

| ID | Status after P0-005 | Evidence or reason |
|---|---|---|
| `U-01` fitting membership | Partially resolved | F4 enumerates 239 numbered pages; count basis and unique-model definition remain open |
| `U-02` fitting categories and edition split | Partially resolved | 22 categories are named; per-function Origin versus OriginPro split is unpublished |
| `U-03` peak membership and 1D-versus-surface scope | Partially resolved | 28 1D and 22 surface names; 25-versus-20 scope resolved; surface 21-versus-22 and surface-peak subset open |
| `U-04` graph named-floor gap | Corrected | 73 rows are 68 gallery filters plus five examples; the 27-entry-gap inference is withdrawn; distinct built-in membership remains open |
| `U-05` graph App dependency | Partially resolved | Correlation Plot and Paired Comparison Plot are explicit Apps; Decision Tree and Stem-and-Leaf are co-listed as Apps; most labels are unlabelled |
| `U-06` seventeenth export format | Closed as a documentation error | No seventeenth format is documented; 16-row table and 16 option values |
| `U-07` export claim basis | Corrected, runtime open | 16 documented and 15 currently listed; current 64-bit visible set untested |
| `U-25` current-version identity of live Help | Open | Live Help has no release banner or version path; 2026b binding is an inference |
| `U-26` runtime behavior of every child | Open | No Origin installation or authorized runtime test |
| `U-28` fitting count basis | Open | 170, close to 200, 200-plus and 239 are not reconciled by a vendor statement |
| `U-29` fitting appendix structure | Open | Ten numbered pages are absent from category tables; one typo and one cross-appendix name appear in tables |
| `U-30` surface model count | Open | The OriginPro guide says 21; the surface table lists 22; names DoseResponse2D and DoseResp differ |
| `U-31` 2026b graph attribution | Open | The 2026b highlights do not name Individual Value Chart or Overlaid Contour as new graph types |
| `U-32` graph App classification | Open | Most of the 111 labels have no built-in or App marker |
| `U-33` current 64-bit export visible set | Open | AI is hidden; CGM and DXF are documented as unsupported in 64-bit; no runtime test |

## 12. Verification and limits

Working directory: `/Users/leo/Documents/General/EveryGraph`. Platform: macOS 27.0.1 arm64. Python: 3.13.9 and 3.14.2. Network retrieval: public HTTP only. No vendor application, manual, SDK, sample or runtime was used.

| Check | Result | Evidence and limits |
|---|---|---|
| Fetch 27 source entries (26 unique URLs) | HTTP 200 for all fetched sources | F10 and G5 are the same 2026b page; response dates 2026-10-30 16:03 UTC; docs Last-Modified 2026-10-26; live pages may change |
| Parse F4 sidebar and category tables | 239 numbered pages; 22 categories; 307 category cells; 231 unique strings; 10 sidebar-only names; 2 table-only strings | Parser-based count; the vendor publishes no total; names are listed in section 4.2 |
| Parse P3 and P4 tables | 28 peak entries with 7 bold; 22 surface entries | Table-cell count; no surface-peak subset |
| Parse G2 select and F1 graph list | 68 specific gallery filters; 111 product labels; 65 extended templates | Select options and list items; duplicates and App labels as recorded in section 6 |
| Parse E1, E2 and E3 | 16 format rows; 15 guide formats; 14 sentence versus 16 options | Table and option count; no runtime verification |
| Compare P0-003 graph names with G2 | 68 of 68 gallery filters matched after ampersand normalization; 67 exact; five examples added | Set comparison; this is the basis for withdrawing the 27-gap inference |
| `python3 tools/validate_planning.py` | Run after this appendix and the record update; result recorded in the P0-005 record | The validator checks local links, fences and English policy; it does not check external URLs or vendor counts |
| `shasum -a 256 origin-feature-inventory.md datagraph-feature-inventory.md docs/planning/feature-traceability.csv docs/planning/functional-scope.md tools/validate_planning.py` | Recorded in the P0-005 record; inventory hashes match the validator constants | Confirms the protected files were not changed by this task |

Checks not run and their reasons:

- No Origin or OriginPro runtime: the application is proprietary and not installed; no runtime access was authorized for this task.
- No frozen 2026b Help snapshot or manual: the Help site is live and unversioned; a versioned manual or SDK was not acquired.
- No exhaustive per-function equation or parameter extraction for all 239 fitting, 28 peak or 22 surface entries: one function per class was sampled to prove structure, not to reproduce the vendor library.
- No surface-peak-only subset: no official page publishes one.
- No legal review: the rights assessment is an engineering classification, not legal advice.
- No external page hashing: the appendix records retrieval dates and quotes; it does not claim immutable vendor pages.

## 13. Sources

- [F1: Origin and OriginPro product page](https://www.originlab.com/origin), retrieved 2026-10-30 UTC.
- [F2: Curve and Surface Fitting](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/CurveFitting), retrieved 2026-10-30 UTC.
- [F3: NLFit Quick Start](https://docs.originlab.com/origin-help/nlfit-quickstart/), retrieved 2026-10-30 UTC.
- [F4: 32.1 Curve Fitting Functions](https://docs.originlab.com/origin-help/curve-fitting-function/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [F5: 32.1.85 Gauss](https://docs.originlab.com/origin-help/gauss-fitfunc/), retrieved 2026-10-30 UTC.
- [F6: 32.1.87 Gaussian](https://docs.originlab.com/origin-help/gaussian-function-fitfunc/), retrieved 2026-10-30 UTC.
- [F7: 32.1.125 Lorentz](https://docs.originlab.com/origin-help/lorentz-fitfunc/), retrieved 2026-10-30 UTC.
- [F8: 32.3.8 Gauss2D](https://docs.originlab.com/origin-help/gauss2d-fitfunc/), retrieved 2026-10-30 UTC.
- [F9: OriginPro user guide](https://docs.originlab.com/user-guide/originpro/), retrieved 2026-10-30 UTC.
- [F10: Origin 2026b Feature Highlights](https://www.originlab.com/2026b), retrieved 2026-10-30 UTC.
- [F11: User Defined Fitting Function tutorial](https://docs.originlab.com/tutorials/userdef-fitfunc/), retrieved 2026-10-30 UTC.
- [P2: Peak Analysis](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/PeakAnalysis), retrieved 2026-10-30 UTC.
- [P3: 32.4 Peak Analyzer Functions](https://docs.originlab.com/origin-help/peak-analyzer-functions/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [P4: 32.3 Non-linear Surface Fitting](https://docs.originlab.com/origin-help/non-linear-surface-fitting-fitfunc/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [P5: Peak Analyzer, Fit Peaks page](https://docs.originlab.com/origin-help/peakanalyzer-fitpeak/), retrieved 2026-10-30 UTC.
- [P6: 32.4.2 BWF](https://docs.originlab.com/origin-help/bwf-pafunc/), retrieved 2026-10-30 UTC.
- [G2: Origin Graph Gallery](https://www.originlab.com/GraphGallery), retrieved 2026-10-30 UTC.
- [G3: 10 Graphing](https://docs.originlab.com/user-guide/graphing/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [G4: Origin 2026 Feature Highlights](https://www.originlab.com/2026), retrieved 2026-10-30 UTC.
- [G5: Origin 2026b Feature Highlights](https://www.originlab.com/2026b), retrieved 2026-10-30 UTC.
- [G6: Origin 2025b press release](https://www.originlab.com/index.aspx?go=COMPANY/NEWSANDEVENTS/PRESSROOM&pid=5474), published 19 June 2025, retrieved 2026-10-30 UTC.
- [E1: Exporting Your Graph or Layout Page to an Image File](https://docs.originlab.com/origin-help/expgraph-to-image/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [E2: 17 Publishing and Export](https://docs.originlab.com/user-guide/publishing-and-export/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [E3: 2.7.10 expGraph](https://docs.originlab.com/x-function/ref/expgraph/), Last-Modified 2026-10-26, retrieved 2026-10-30 UTC.
- [E5: Origin 2022b Feature Highlights](https://www.originlab.com/2022b), retrieved 2026-10-30 UTC.
- [E6: FAQ-828](https://docs.originlab.com/quick-help/export-graphs-as-ai/), updated 2022-12-13, retrieved 2026-10-30 UTC.
- [E7: Origin Technical Support Policy](https://www.originlab.com/index.aspx?go=Support/SupportPolicy), retrieved 2026-10-30 UTC.

Local evidence and planning references:

- [P0-003 evidence](capability-catalog-decomposition.md), the parent decomposition and unknown register.
- [Feature traceability](../planning/feature-traceability.csv), the unchanged 428-row register.
- [Functional scope](../planning/functional-scope.md), the catalog gate and compatibility decision axes.
- [Task board](../planning/task-board.csv), the P0-005 state.
- [Origin inventory](../../origin-feature-inventory.md), the preserved 2026b source inventory.
- [DataGraph inventory](../../datagraph-feature-inventory.md), the preserved DataGraph source inventory.
