# Fitting and Peak Catalog Enumeration

Task P0-012 research deliverable. Status: independent review completed and findings resolved, 2026-10-31.

Parent requirements: `ORG-GRAPH-001`, `ORG-FIT-004`, `ORG-PEAK-005`, `EG-ADD-015`. Evidence base: [P0-003 catalog decomposition](capability-catalog-decomposition.md) and the [P0-005 2026b evidence appendix](catalog-evidence-gaps-2026b.md).

This document proposes stable child IDs and per-function evidence for 239 fitting functions, 28 one-dimensional Peak Analyzer functions and 22 surface-fitting functions. It does not add rows to the catalog register, does not close a catalog gate and does not approve implementation.

## 1. Result and limits

The enumeration establishes name, source page, parameter names, documented bound expressions where published, page-category tokens where present, and a proposed child ID for each function. The evidence is the live Origin Help build dated 2026-10-26, retrieved 2026-10-30. The binding to release 2026b is an inference because the Help site has no release banner (`U-25`, P0-005 section 11).

| Result | Count | Identifier range |
|---|---:|---|
| Fitting functions in appendix 32.1 | 239 | `EG-CAT-FIT-003` through `EG-CAT-FIT-241` |
| One-dimensional Peak Analyzer functions in appendix 32.4 | 28 | `EG-CAT-PEAK-002` through `EG-CAT-PEAK-029` |
| Surface-fitting functions in appendix 32.3 | 22 | `EG-CAT-FIT-002` plus `EG-CAT-PEAK-030` through `EG-CAT-PEAK-050` |
| Total enumerated rows | 289 | 239 fitting, 28 one-dimensional peak, 22 surface; 288 new proposed children plus one reused existing row |
| Unique source pages | 268 | 239 fitting, 7 Pro-only peak, 22 surface |

All rows except the reused `EG-CAT-FIT-002` are proposed children, not register rows. Rights, delivery, uncertainty, prerequisites, fixture and expected output are declared in sections 3.2 and 3.3 and per-row codes in sections 4, 5 and 6. Every enumerated row has a named Help page and a published parameter list; no per-function row is vendor-claim-only. The aggregate count claims and the surface 21-versus-22 conflict are the vendor-claim residues, recorded in sections 5-8.

Limits that remain after this enumeration:

- No numerical equation was independently derived and no fit was executed; the rows are name-and-parameter evidence, not scientific validation.
- No vendor runtime, manual, SDK, FDF file, sample project or function implementation was obtained. The rights status remains `R-VENDOR`.
- The live Help pages are unversioned; the 2026b binding is not pinned by a versioned manual.
- The vendor count conflicts, edition split, parameter contradictions and malformed bounds recorded in sections 4.3, 5.3, 6.3 and 8 remain open.
- No gate is closed. Section 7 records only current status and ownership.

## 2. Evidence base and method

### 2.1 Source pages

| ID | Page | Use | Version and retrieval |
|---|---|---|---|
| F4 | [32.1 Curve Fitting Functions](https://docs.originlab.com/origin-help/curve-fitting-function/) | 239 numbered fitting pages and 22 category tables | Last-Modified 2026-10-26 09:00 UTC; retrieved 2026-10-30 19:11 UTC |
| P3 | [32.4 Peak Analyzer Functions](https://docs.originlab.com/origin-help/peak-analyzer-functions/) | 28 one-dimensional names and 7 bold Pro-only markers | Last-Modified 2026-10-26 09:00 UTC; retrieved 2026-10-30 19:11 UTC |
| P4 | [32.3 Non-linear Surface Fitting](https://docs.originlab.com/origin-help/non-linear-surface-fitting-fitfunc/) | 22 surface names | Last-Modified 2026-10-26 09:00 UTC; retrieved 2026-10-30 19:11 UTC |
| Pages | 268 unique per-function pages linked in sections 4-6 | Parameter names, bound expressions, page-category tokens | HTTP 200; all 272 responses carried Last-Modified 2026-10-26 09:00:17 or 09:00:18 UTC; retrieved 2026-10-30 19:11:17-19:11:41 UTC |
| F9 | [OriginPro user guide](https://docs.originlab.com/user-guide/originpro/) | Vendor claim of 21 surface models | Not retrieved by P0-012; claim transcribed from P0-005 section 5.2; used only for the count conflict |
| P0-003 | [Catalog decomposition](capability-catalog-decomposition.md) | Child-ID schema, code meanings, default fields and gate definitions | Accepted 2026-10-30 |
| P0-005 | [Catalog evidence gaps 2026b](catalog-evidence-gaps-2026b.md) | Count floors, Pro-only markers, rights review, gate status and unknown delta | Accepted 2026-10-31 |

The Help site is live and has no release banner. The retrieval date is recorded; the page contents may change without notice. The per-row links are the authoritative source locators for this enumeration. F9 was not re-fetched by P0-012; its 21-surface-model claim is transcribed from P0-005 section 5.2.

### 2.2 Method

- Read the 32.1 sidebar numbering `32.1.1` through `32.1.239`, its 22 category headings and all category-table cells.
- Read each per-function page for `Number`, `Names`, `Lower Bounds`, `Upper Bounds`, `Category` and the page title. Record only short factual fields; do not reproduce equations, brief descriptions, sample curves, parameter meanings, FDF layouts or long vendor notes.
- Read the 32.4 table and its bold markup to identify the seven Pro-only Peak Analyzer entries; read the seven 32.4 function pages and the 21 shared 32.1 pages.
- Read the 32.3 table and its 22 function pages; use the page bounds where the table cells are blank and the page states a constraint.
- Normalize LaTeX delimiters and subscripts to plain text (`w_1` becomes `w1`); the linked page remains authoritative. Whitespace and comma spacing are normalized; malformed expressions are preserved.
- Cross-check the counts against P0-005 sections 4.2, 5.1 and 5.2; the recount in section 11 matches every published floor. Gate status and unknown deltas are transcribed from P0-005 sections 9 and 11 and P0-003 sections 10-12.
- Every per-row claim derives from the linked page. Where the page and the P0-005 summary disagree, sections 4.3, 5.3, 6.3 and 8 record the conflict rather than resolving it by inference.

### 2.3 Count cross-check

| Check | P0-005 value | P0-012 recount | Result |
|---|---:|---:|---|
| Numbered 32.1 pages | 239 | 239 | Match |
| 32.1 category headings | 22 | 22 | Match |
| 32.1 category-table cells | 307 | 307 | Match |
| Unique category-table strings | 231 | 231 | Match |
| Sidebar-only 32.1 names | 10 | 10 | Match |
| Table-only strings | 2 | 2 | Match |
| 32.4 one-dimensional names | 28 | 28 | Match |
| 32.4 Pro-only markers | 7 | 7 | Match |
| 32.4 shared names | 21 | 21 | Match |
| 32.3 surface names | 22 | 22 | Match |

## 3. Identifier allocation and common fields

### 3.1 Allocation

| Range | Count | Allocation order | Parent requirement |
|---|---:|---|---|
| `EG-CAT-FIT-003`..`EG-CAT-FIT-241` | 239 | 32.1 sidebar order `32.1.1` to `32.1.239` | `ORG-FIT-004` |
| `EG-CAT-PEAK-002`..`EG-CAT-PEAK-029` | 28 | 32.4 table order | `ORG-PEAK-005` |
| `EG-CAT-PEAK-030`..`EG-CAT-PEAK-050` | 21 | 32.3 table order, excluding `Gauss2D` | `ORG-PEAK-005` |
| `EG-CAT-FIT-002` | 1 | Existing register row for `Gauss2D`, reused | `ORG-FIT-004` |

The IDs are proposed for a future register integration. Once admitted, they are stable: no reuse, renumbering or reassignment. `EG-CAT-PEAK-051` and later are unallocated. Existing `EG-CAT-FIT-001`, `EG-CAT-FIT-002`, `EG-CAT-PEAK-001` and the `EG-CAT-GATE-*` rows are not modified by this document.

`Gauss2D` is not duplicated. The 32.3 page is the function already represented by `EG-CAT-FIT-002`; the other 21 surface functions receive new peak IDs because they are surface-fitting children of the peak parent. This is an allocation decision for review, not a claim that the register already accepts it.

### 3.2 Default fields

Unless a row or section states an exception, every proposed row uses these values:

| Field | Default | Basis |
|---|---|---|
| Parent | `ORG-FIT-004` for fitting; `ORG-PEAK-005` for peak and surface | Section 3.1 |
| Register family | `ORG-FIT-004` or `ORG-PEAK-005`, matching the parent | Existing register vocabulary; final admission belongs to the register owner |
| Member type | `fit-function` for fitting; `peak-function` for peak and surface | Existing register vocabulary |
| Source | The linked 32.1, 32.4 or 32.3 page | Per-row link |
| Source version | Live Help build 2026-10-26; retrieved 2026-10-30; 2026b binding inferred | P0-005 sections 2.3 and 5.4 |
| Rights | `R-VENDOR` | Vendor Help citation only; no FDF, implementation or redistribution right obtained. The mathematical form is a public description, but independent derivation is required |
| Delivery | `D-GATE` | No fit implementation or support claim |
| Uncertainty | `U2` | The function family and parameters are named; edition, numerical behavior and version pin remain open |
| Prerequisite | `PR-FIT`, `PR-PEAK` or `PR-SURF` (section 3.3) | Per catalog |
| Fixture | `F-SYNTH` | Synthetic analytic truth with fixed tolerances; no vendor sample |
| Expected output | `O-TABLE` | Typed table, metadata and an explicit loss report |
| Support claim | None | Research enumeration only |

The reused `EG-CAT-FIT-002` keeps the existing `fit-function` member type and `RO` code. Whether the 21 new surface rows should be `peak-function` or `fit-function` is left to the register owner, because the existing surface row `Gauss2D` is a fitting child.
### 3.3 Prerequisites, codes and exceptions

| Code | Meaning |
|---|---|
| `PR-FIT` | Typed x/y data with distinct missing, NaN and mask semantics; initial values and bounds; a candidate fit executor with convergence diagnostics; an independent numerical oracle; no vendor FDF or code |
| `PR-GLOBAL` | `PR-FIT` plus global-fit parameter sharing across datasets and per-dataset fixed constants; applies to `EG-CAT-FIT-037`, `-135`, `-148` and `-234` |
| `PR-PEAK` | `PR-FIT` plus baseline and peak-search workflow and a synthetic multi-peak mixture with known centers, widths and amplitudes |
| `PR-PRO` | `PR-PEAK` plus the vendor Pro-only Peak Analyzer workflow gate and an explicit edition decision; applies to the seven Pro-only entries |
| `PR-SURF` | Typed x/y/z grid or XYZ columns, a surface-fit executor and an independent analytic-surface oracle |
| `RV` | `R-VENDOR` as defined in P0-003 section 11.2 |
| `RO` | `R-OPEN` as defined in P0-003 section 11.2; used only by the existing `EG-CAT-FIT-002` row |
| `DG` | `D-GATE` as defined in P0-003 section 11.2 |
| `U1` | Name and family confirmed; exact variant or parameter set open |
| `U2` | Family confirmed; exact versions, extensions or variants open |
| `U3` | Vendor claim only; membership not enumerated |
| `U4` | Rights, specification or artifact identity unresolved |

The rights code for new rows is `R-VENDOR`, not `R-OPEN`: this document cites the vendor Help pages and does not grant a license to the vendor function library. The page-level mathematical form can be independently derived, but that derivation is implementation work with an independent oracle, not a copied artifact.

## 4. Fitting function catalog (`ORG-FIT-004`)

### 4.1 Structure and counts

The 32.1 appendix has 239 numbered pages in 22 category headings. The category tables contain 307 cells and 231 unique strings. Ten sidebar pages are absent from every category table, and two table strings have no exact sidebar page:

| Sidebar-only page | Child ID | Page category |
|---|---|---|
| BoxLucas2 | EG-CAT-FIT-023 | Exponential |
| ExpAssoc1 | EG-CAT-FIT-061 | Exponential |
| ExpAssoc2 | EG-CAT-FIT-062 | Exponential |
| ExpAssocDelay1 | EG-CAT-FIT-063 | Exponential |
| ExpAssocDelay2 | EG-CAT-FIT-064 | Exponential |
| NormalCDF | EG-CAT-FIT-151 | Statistics |
| PIPlatt | EG-CAT-FIT-163 | Exponential |
| PIPlatt2 | EG-CAT-FIT-164 | Exponential |
| PIWebb | EG-CAT-FIT-165 | Exponential |
| SchulzFlory | EG-CAT-FIT-198 | PFW |

| Table-only string | Resolution |
|---|---|
| `:DoubleBoltzmann` | Leading-colon typo for `DoubleBoltzmann`, which is `EG-CAT-FIT-043` in the sidebar (`U-39`) |
| `Bigaussian` | A 32.4 Pro-only Peak Analyzer function, not a 32.1 page; represented as `EG-CAT-PEAK-012` (`U-38`) |

Page-category tokens are a different classification from the 32.1 category tables. The 22 headings and the observed page-token counts are:

| Category heading | Table entries | Pages with token |
|---|---:|---:|
| Origin Basic Functions | 21 | 21 |
| Convolution | 2 | 2 |
| Exponential | 42 | 49 |
| Growth/Sigmoidal | 19 | 20 |
| Hyperbola | 5 | 5 |
| Logarithm | 5 | 5 |
| Peak Functions | 22 | 20 |
| Piecewise | 2 | 2 |
| Polynomial | 8 | 8 |
| Power | 19 | 19 |
| Rational | 15 | 15 |
| Waveform | 7 | 7 |
| Chromatography | 6 | 5 |
| Electrophysiology | 9 | 6 |
| Enzyme Kinetics | 7 | 5 |
| Pharmacology | 8 | 7 |
| Rheology | 6 | 6 |
| Spectroscopy | 7 | 7 |
| Statistics | 19 | 20 |
| Quick Fit | 11 | 0 |
| Multiple Variables | 5 | 5 |
| Enzyme Kinetics 2 | 62 | 49 |
| `PFW` (page-only token) | 0 | 17 |
| `Baseline` (page-only token) | 0 | 15 |
| `Enzyme Kinetics2` (page-only token) | 0 | 6 |

`Quick Fit` is a catalog heading with 11 entries and no page token. `PFW`, `Baseline` and `Enzyme Kinetics2` are page tokens with no catalog heading. The two classifications also differ in membership and count for headings such as `Exponential` (42 table entries, 49 pages), `Peak Functions` (22, 20) and `Enzyme Kinetics 2` (62, 49). This document records both and does not reconcile them (`U-37`).

Across the 239 pages, 106 declare no lower bound, three declare a real upper bound (`DoubleBoltzmann` `frac < 1`, `Shah` `r < 1.0`, `YldFert` `r < 1.0`), and 12 attach a vendor note after `none`. The vendor `Number` field totals 922; `SineSqr` declares three but lists four names (`U-34`).

### 4.2 Per-function enumeration

Codes are `rights / delivery / uncertainty`. All rows use `PR-FIT` unless section 4.3 names an exception. The `#` column is the 32.1 page number. Every row uses the shared source version in section 2.1 and the linked page as its source locator.

| Child ID | # | Function | Parameters | Bounds | Page category tokens | Codes |
|---:|---|---|---|---|---|---|
| EG-CAT-FIT-003 | 1 | [Allometric1](https://docs.originlab.com/origin-help/allometric1-fitfunc/) | a, b | none | Origin Basic Functions, Power | RV / DG / U2 |
| EG-CAT-FIT-004 | 2 | [Allometric2](https://docs.originlab.com/origin-help/allometric2-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-005 | 3 | [Asym2Sig](https://docs.originlab.com/origin-help/asym2sig-fitfunc/) | y0, xc, A, w1, w2, w3 | w1 > 0.0, w2 > 0.0, w3 > 0.0 | Peak Functions, Power, PFW | RV / DG / U2 |
| EG-CAT-FIT-006 | 4 | [Asymptotic1](https://docs.originlab.com/origin-help/asymptotic1-fitfunc/) | a, b, c | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-007 | 5 | [Belehradek](https://docs.originlab.com/origin-help/belehradek-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-008 | 6 | [BellPlateau](https://docs.originlab.com/origin-help/bellplateau-fitfunc/) | Vmaxlim, pKa, pKb, pKc, alpha | Vmaxlim > 0, pKa > 0, pKb > 0, pKc > 0, alpha > 0 | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-009 | 7 | [BellShaped](https://docs.originlab.com/origin-help/bellshaped-fitfunc/) | Vmaxlim, pKa, pKb | Vmaxlim > 0, pKa > 0, pKb > 0 | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-010 | 8 | [BET](https://docs.originlab.com/origin-help/bet-fitfunc/) | a, b | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-011 | 9 | [Beta](https://docs.originlab.com/origin-help/beta-fitfunc/) | y0, xc, A, w1, w2, w3 | w1 > 0.0, w2 > 1.0, w3 > 1.0 | Origin Basic Functions, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-012 | 10 | [BETMod](https://docs.originlab.com/origin-help/betmod-fitfunc/) | a, b | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-013 | 11 | [BiDoseResp](https://docs.originlab.com/origin-help/bidoseresp-fitfunc/) | A1, A2, LOGx01, LOGx02, h1, h2, p | none | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-014 | 12 | [BiHill](https://docs.originlab.com/origin-help/bihill-fitfunc/) | Pm, Ka, Ki, Ha, Hi | Pm > 0, Ka > 0, Ki > 0, Ha > 0, Hi > 0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-015 | 13 | [Bingham](https://docs.originlab.com/origin-help/bingham-fitfunc/) | y0, A | y0 >= 0, A > 0, | Rheology | RV / DG / U1 |
| EG-CAT-FIT-016 | 14 | [Biphasic](https://docs.originlab.com/origin-help/biphasic-fitfunc/) | Amin, Amax1, Amax2, x01, x02, h1, h2 | none | Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-017 | 15 | [BlNeld](https://docs.originlab.com/origin-help/blneld-fitfunc/) | a, b, c, f | none | Power | RV / DG / U2 |
| EG-CAT-FIT-018 | 16 | [BlNeldSmp](https://docs.originlab.com/origin-help/blneldsmp-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-019 | 17 | [BoltzIV](https://docs.originlab.com/origin-help/boltziv-fitfunc/) | vhalf, dx, gmax, vrev | none | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-020 | 18 | [Boltzmann](https://docs.originlab.com/origin-help/boltzmann-fitfunc/) | A1, A2, x0, dx | none | Origin Basic Functions, Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-021 | 19 | [BoxLucas1](https://docs.originlab.com/origin-help/boxlucas1-fitfunc/) | a, b | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-022 | 20 | [BoxLucas1Mod](https://docs.originlab.com/origin-help/boxlucas1mod-fitfunc/) | a, b | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-023 | 21 | [BoxLucas2](https://docs.originlab.com/origin-help/boxlucas2-fitfunc/) | a1, a2 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-024 | 22 | [Bradley](https://docs.originlab.com/origin-help/bradley-fitfunc/) | a, b | none | Logarithm | RV / DG / U2 |
| EG-CAT-FIT-025 | 23 | [Carreau](https://docs.originlab.com/origin-help/carreau-fitfunc/) | A1, A2, t, a, n | A1 > 0, A2 > 0, t > 0, a > 0, n >= 0 | Rheology | RV / DG / U2 |
| EG-CAT-FIT-026 | 24 | [Catalytic](https://docs.originlab.com/origin-help/catalytic-fitfunc/) | Vmax, Km, Ka, A | Vmax > 0, Km > 0, Ka > 0 | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-027 | 25 | [Catalytic2](https://docs.originlab.com/origin-help/catalytic2-fitfunc/) | Vmax, Km, Ka, S | Vmax > 0, Km > 0, Ka > 0 | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-028 | 26 | [CCE](https://docs.originlab.com/origin-help/cce-fitfunc/) | y0, xc1, A, w, k2, xc2, B, k3, xc3 | w > 0.0 | Peak Functions, PFW, Chromatography | RV / DG / U2 |
| EG-CAT-FIT-029 | 27 | [Chapman](https://docs.originlab.com/origin-help/chapman-fitfunc/) | a, b, c | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-030 | 28 | [ChemDena](https://docs.originlab.com/origin-help/chemdena-fitfunc/) | ui, us, li, ls, m, g | none | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-031 | 29 | [CompetitiveFull](https://docs.originlab.com/origin-help/competitivefull-fitfunc/) | Vmax, Km, Ki, I | Vmax > 0, Km > 0, Ki > 0, I > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-032 | 30 | [CompetitiveFull2](https://docs.originlab.com/origin-help/competitivefull2-fitfunc/) | Vmax, Km, Ki, S | Vmax > 0, Km > 0, Ki > 0, S > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-033 | 31 | [CompetitiveInhibition](https://docs.originlab.com/origin-help/competitiveinhibition-fitfunc/) | Vmax, Km, Ki, E, S | Vmax > 0, Km > 0, Ki > 0, E > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-034 | 32 | [CompetitiveInhibition2](https://docs.originlab.com/origin-help/competitiveinhibition2-fitfunc/) | Vmax, Km, Ki, E, I | Vmax > 0, Km > 0, Ki > 0, E > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-035 | 33 | [CompetitivePartial](https://docs.originlab.com/origin-help/competitivepartial-fitfunc/) | Vmax, Km, Ki, I, alpha | Vmax > 0, Km > 0, , alpha > 0 | Enzyme Kinetics 2 | RV / DG / U1 |
| EG-CAT-FIT-036 | 34 | [CompetitivePartial2](https://docs.originlab.com/origin-help/competitivepartial2-fitfunc/) | Vmax, Km, Ki, S, alpha | Vmax > 0, Km > 0, Ki > 0, , alpha > 0 | Enzyme Kinetics 2 | RV / DG / U1 |
| EG-CAT-FIT-037 | 35 | [CompInhib](https://docs.originlab.com/origin-help/compinhib-fitfunc/) | Vmax, Km, Ki, Ic | Vmax > 0, Km > 0, Ki >0 | Enzyme Kinetics | RV / DG / U2 |
| EG-CAT-FIT-038 | 36 | [Constant](https://docs.originlab.com/origin-help/constant-fitfunc/) | y0 | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-039 | 37 | [Cross](https://docs.originlab.com/origin-help/cross-fitfunc/) | A1, A2, t, m | A1 > 0, A2 > 0, t > 0, m >= 0 | Rheology | RV / DG / U2 |
| EG-CAT-FIT-040 | 38 | [Cubic](https://docs.originlab.com/origin-help/cubic-fitfunc/) | A, B, C, D | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-041 | 39 | [Dhyperbl](https://docs.originlab.com/origin-help/dhyperbl-fitfunc/) | P1, P2, P3, P4, P5 | none | Hyperbola | RV / DG / U2 |
| EG-CAT-FIT-042 | 40 | [DoseResp](https://docs.originlab.com/origin-help/doseresp-fitfunc/) | A1, A2, LOGx0, p | none | Growth/Sigmoidal, Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-043 | 41 | [DoubleBoltzmann](https://docs.originlab.com/origin-help/doubleboltzmann-fitfunc/) | y0, A, frac, x01, x02, k1, k2 | frac > 0; upper: frac < 1 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-044 | 42 | [DoubleExp](https://docs.originlab.com/origin-help/doubleexp-fitfunc/) | ymax1, k1, ymax2, k2 | ymax1 => 0, k1 > 0, , k2 > 0 | Enzyme Kinetics 2 | RV / DG / U1 |
| EG-CAT-FIT-045 | 43 | [DoubleExpOffset](https://docs.originlab.com/origin-help/doubleexpoffset-fitfunc/) | y0, ymax1, ymax2, k1, k2 | , , , k1 > 0, k2 > 0 | Enzyme Kinetics 2 | RV / DG / U1 |
| EG-CAT-FIT-046 | 44 | [ECS](https://docs.originlab.com/origin-help/ecs-fitfunc/) | y0, xc, A, w, a3, a4 | A > 0.0, w > 0.0 | Peak Functions, PFW, Chromatography | RV / DG / U2 |
| EG-CAT-FIT-047 | 45 | [Exp1P1](https://docs.originlab.com/origin-help/exp1p1-fitfunc/) | A | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-048 | 46 | [Exp1p2](https://docs.originlab.com/origin-help/exp1p2-fitfunc/) | A | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-049 | 47 | [Exp1p2md](https://docs.originlab.com/origin-help/exp1p2md-fitfunc/) | B | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-050 | 48 | [Exp1p3](https://docs.originlab.com/origin-help/exp1p3-fitfunc/) | A | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-051 | 49 | [Exp1P3Md](https://docs.originlab.com/origin-help/exp1p3md-fitfunc/) | B | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-052 | 50 | [Exp1P4](https://docs.originlab.com/origin-help/exp1p4-fitfunc/) | A | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-053 | 51 | [Exp1P4Md](https://docs.originlab.com/origin-help/exp1p4md-fitfunc/) | B | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-054 | 52 | [Exp2P](https://docs.originlab.com/origin-help/exp2p-fitfunc/) | a, b | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-055 | 53 | [Exp2PMod1](https://docs.originlab.com/origin-help/exp2pmod1-fitfunc/) | a, b | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-056 | 54 | [Exp2PMod2](https://docs.originlab.com/origin-help/exp2pmod2-fitfunc/) | a, b | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-057 | 55 | [Exp3P1](https://docs.originlab.com/origin-help/exp3p1-fitfunc/) | a, b, c | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-058 | 56 | [Exp3P1Md](https://docs.originlab.com/origin-help/exp3p1md-fitfunc/) | a, b, c | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-059 | 57 | [Exp3P2](https://docs.originlab.com/origin-help/exp3p2-fitfunc/) | a, b, c | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-060 | 58 | [ExpAssoc](https://docs.originlab.com/origin-help/expassoc-fitfunc/) | y0, A1, t1, A2, t2 | t1 > 0, t2 > 0 | Origin Basic Functions, Exponential | RV / DG / U2 |
| EG-CAT-FIT-061 | 59 | [ExpAssoc1](https://docs.originlab.com/origin-help/expassoc1-fitfunc/) | TD, Yb, A, Tau | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-062 | 60 | [ExpAssoc2](https://docs.originlab.com/origin-help/expassoc2-fitfunc/) | TD1, TD2, Yb, A1, A2, Tau1, Tau2 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-063 | 61 | [ExpAssocDelay1](https://docs.originlab.com/origin-help/expassocdelay1-fitfunc/) | TD, Yb, A, Tau | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-064 | 62 | [ExpAssocDelay2](https://docs.originlab.com/origin-help/expassocdelay2-fitfunc/) | TD1, TD2, Yb, A1, A2, Tau1, Tau2 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-065 | 63 | [ExpDec1](https://docs.originlab.com/origin-help/expdec1-fitfunc/) | y0, A, t | none | Origin Basic Functions, Exponential, Baseline, Electrophysiology | RV / DG / U2 |
| EG-CAT-FIT-066 | 64 | [ExpDec2](https://docs.originlab.com/origin-help/expdec2-fitfunc/) | y0, A1, t1, A2, t2 | none | Origin Basic Functions, Exponential, Baseline, Electrophysiology | RV / DG / U2 |
| EG-CAT-FIT-067 | 65 | [ExpDec3](https://docs.originlab.com/origin-help/expdec3-fitfunc/) | y0, A1, t1, A2, t2, A3, t3 | none | Origin Basic Functions, Electrophysiology | RV / DG / U2 |
| EG-CAT-FIT-068 | 66 | [ExpDecay1](https://docs.originlab.com/origin-help/expdecay1-fitfunc/) | y0, x0, A1, t1 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-069 | 67 | [ExpDecay2](https://docs.originlab.com/origin-help/expdecay2-fitfunc/) | y0, x0, A1, t1, A2, t2 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-070 | 68 | [ExpDecay3](https://docs.originlab.com/origin-help/expdecay3-fitfunc/) | y0, x0, A1, t1, A2, t2, A3, t3 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-071 | 69 | [ExpGro1](https://docs.originlab.com/origin-help/expgro1-fitfunc/) | y0, A1, t1 | none | Exponential, Baseline | RV / DG / U2 |
| EG-CAT-FIT-072 | 70 | [ExpGro2](https://docs.originlab.com/origin-help/expgro2-fitfunc/) | y0, A1, t1, A2, t2 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-073 | 71 | [ExpGro3](https://docs.originlab.com/origin-help/expgro3-fitfunc/) | y0, A1, t1, A2, t2, A3, t3 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-074 | 72 | [ExpGrow1](https://docs.originlab.com/origin-help/expgrow1-fitfunc/) | y0, x0, A1, t1 | t1 > 0.0 | Baseline, Exponential, Origin Basic Functions | RV / DG / U2 |
| EG-CAT-FIT-075 | 73 | [ExpGrow2](https://docs.originlab.com/origin-help/expgrow2-fitfunc/) | y0, x0, A1, t1, A2, t2 | none | Baseline, Exponential, Origin Basic Functions | RV / DG / U2 |
| EG-CAT-FIT-076 | 74 | [ExpGrow3Dec2](https://docs.originlab.com/origin-help/expgrow3dec2-fitfunc/) | y0, xc, Ag1, tg1, Ag2, tg2, Ag3, tg3, Ad1, td1, Ad2, td2 | 0 < xc, Ag1, tg1, Ag2, tg2, Ag3, tg3, Ad1, td1, Ad2, td2 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-077 | 75 | [ExpGrowDec](https://docs.originlab.com/origin-help/expgrowdec-fitfunc/) | y0, xc, Ag, tg, Ad, td | 0 < xc, Ag, tg, Ad, td | Exponential | RV / DG / U2 |
| EG-CAT-FIT-078 | 76 | [ExpLinear](https://docs.originlab.com/origin-help/explinear-fitfunc/) | p1, p2, p3, p4 | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-079 | 77 | [Exponential](https://docs.originlab.com/origin-help/exponential-fitfunc/) | y0, A, R0 | none | Statistics, Baseline, Exponential | RV / DG / U2 |
| EG-CAT-FIT-080 | 78 | [ExponentialCDF](https://docs.originlab.com/origin-help/exponentialcdf-fitfunc/) | y0, A, mu | A > 0, mu > 0 | Statistics, Baseline, Exponential | RV / DG / U2 |
| EG-CAT-FIT-081 | 79 | [Extreme](https://docs.originlab.com/origin-help/extreme-fitfunc/) | y0, xc, w, A | w > 0.0 | Statistics, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-082 | 80 | [FarazdaghiHarris](https://docs.originlab.com/origin-help/farazdaghiharris-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-083 | 81 | [FirstOrder](https://docs.originlab.com/origin-help/firstorder-fitfunc/) | lmt, k | lmt > 0, k > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-084 | 82 | [FirstOrderOffset](https://docs.originlab.com/origin-help/firstorderoffset-fitfunc/) | lmt, k, offset | lmt > 0, k > 0, offset > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-085 | 83 | [FreundlichEXT](https://docs.originlab.com/origin-help/freundlichext-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-086 | 84 | [GammaCDF](https://docs.originlab.com/origin-help/gamma-fitfunc/) | y0, A1, a, b | A1 > 0.0, a > 0.0, b> 0.0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-087 | 85 | [Gauss](https://docs.originlab.com/origin-help/gauss-fitfunc/) | y0, xc, w, A | w > 0.0 | Origin Basic Functions, Peak Functions, Chromatography, Electrophysiology, Statistics | RV / DG / U2 |
| EG-CAT-FIT-088 | 86 | [GaussAmp](https://docs.originlab.com/origin-help/gaussamp-fitfunc/) | y0, xc, w, A | w > 0.0 | Statistics, Spectroscopy, PFW, Peak Functions, Origin Basic Functions | RV / DG / U2 |
| EG-CAT-FIT-089 | 87 | [Gaussian](https://docs.originlab.com/origin-help/gaussian-function-fitfunc/) | y0, xc, A, w | w > 0.0 | Peak Functions, PFW | RV / DG / U2 |
| EG-CAT-FIT-090 | 88 | [GaussianLorentz](https://docs.originlab.com/origin-help/gaussianlorentz-pafunc/) | y0, xc, A1, A2, w1, w2 | w1 > 0.0, w2 > 0.0 | Multiple Variables | RV / DG / U2 |
| EG-CAT-FIT-091 | 89 | [GaussMod](https://docs.originlab.com/origin-help/gaussmod-fitfunc/) | y0, A, xc, w, t0 | w > 0.0, t0 > 0.0 | PFW, Peak Functions, Chromatography, Convolution | RV / DG / U2 |
| EG-CAT-FIT-092 | 90 | [GCAS](https://docs.originlab.com/origin-help/gcas-fitfunc/) | y0, xc, A, w, a3, a4 | w > 0.0 | Peak Functions, PFW, Chromatography | RV / DG / U2 |
| EG-CAT-FIT-093 | 91 | [Giddings](https://docs.originlab.com/origin-help/giddings-fitfunc/) | y0, xc, w, A | w > 0.0 | Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-094 | 92 | [Goldman](https://docs.originlab.com/origin-help/goldman-fitfunc/) | b, Nao, Nai, Ki, T | Nao >= 0.0, Nai >= 0.0, Ki >= 0.0, T > 0.0. | Electrophysiology | RV / DG / U2 |
| EG-CAT-FIT-095 | 93 | [Gumbel](https://docs.originlab.com/origin-help/gumbel-fitfunc/) | a, b | none | Statistics | RV / DG / U2 |
| EG-CAT-FIT-096 | 94 | [Gunary](https://docs.originlab.com/origin-help/gunary-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-097 | 95 | [Helix](https://docs.originlab.com/origin-help/helix-fitfunc/) | x0, y0, A, w, p | none | Multiple Variables | RV / DG / U2 |
| EG-CAT-FIT-098 | 96 | [Herschel](https://docs.originlab.com/origin-help/herschel-fitfunc/) | y0, K, n | y0 >= 0, K > 0, n > 0 | Rheology | RV / DG / U2 |
| EG-CAT-FIT-099 | 97 | [Hill](https://docs.originlab.com/origin-help/hill-fitfunc/) | Vmax, k, n | Vmax > 0, k>0, n>0 | Electrophysiology, Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-100 | 98 | [Hill1](https://docs.originlab.com/origin-help/hill1-fitfunc/) | START, END, k, n | k>0, n>0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-101 | 99 | [HillBurk](https://docs.originlab.com/origin-help/hillburk-fitfunc/) | Vm1, Km1, Vm2, Km2 | none | Multiple Variables | RV / DG / U2 |
| EG-CAT-FIT-102 | 100 | [Holliday](https://docs.originlab.com/origin-help/holliday-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-103 | 101 | [Holliday1](https://docs.originlab.com/origin-help/holliday1-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-104 | 102 | [Hyperbl](https://docs.originlab.com/origin-help/hyperbl-fitfunc/) | P1, P2 | none | Baseline, Origin Basic Functions, Hyperbola | RV / DG / U2 |
| EG-CAT-FIT-105 | 103 | [HyperbolaGen](https://docs.originlab.com/origin-help/hyperbolagen-fitfunc/) | a, b, c, d | none | Hyperbola | RV / DG / U2 |
| EG-CAT-FIT-106 | 104 | [HyperbolaMod](https://docs.originlab.com/origin-help/hyperbolamod-fitfunc/) | T1, T2 | none | Hyperbola | RV / DG / U2 |
| EG-CAT-FIT-107 | 105 | [InvsPoly](https://docs.originlab.com/origin-help/invspoly-fitfunc/) | y0, xc, w, A, A1, A2, A3 | w > 0.0 | Spectroscopy, PFW, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-108 | 106 | [Isoenzyme](https://docs.originlab.com/origin-help/isoenzyme-fitfunc/) | Vmax1, Vmax2, Km1, Km2 | Vmax1 > 0, Vmax2 > 0, | Enzyme Kinetics 2 | RV / DG / U1 |
| EG-CAT-FIT-109 | 107 | [Langevin](https://docs.originlab.com/origin-help/langevin-fitfunc/) | y0, xc, C | noe | Exponential | RV / DG / U1 |
| EG-CAT-FIT-110 | 108 | [LangevinMod](https://docs.originlab.com/origin-help/langevinmod-fitfunc/) | y0, xc, C, s | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-111 | 109 | [LangmuirEXT1](https://docs.originlab.com/origin-help/langmuirext1-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-112 | 110 | [LangmuirEXT2](https://docs.originlab.com/origin-help/langmuirext2-fitfunc/) | a, b, c | none | Power | RV / DG / U2 |
| EG-CAT-FIT-113 | 111 | [Laplace](https://docs.originlab.com/origin-help/laplace-fitfunc/) | y0, a, b | none | Statistics, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-114 | 112 | [Line](https://docs.originlab.com/origin-help/line-fitfunc/) | A, B | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-115 | 113 | [Line3](https://docs.originlab.com/origin-help/line3-fitfunc/) | a, b, c, d | none | Multiple Variables | RV / DG / U2 |
| EG-CAT-FIT-116 | 114 | [LineExp](https://docs.originlab.com/origin-help/lineexp-fitfunc/) | a1, a2, b1, b2, c | none | Multiple Variables | RV / DG / U2 |
| EG-CAT-FIT-117 | 115 | [LineMod](https://docs.originlab.com/origin-help/linemod-fitfunc/) | a, b | none | Polynomial | RV / DG / U2 |
| EG-CAT-FIT-118 | 116 | [Log2P1](https://docs.originlab.com/origin-help/log2p1-fitfunc/) | a, b | none | Logarithm | RV / DG / U2 |
| EG-CAT-FIT-119 | 117 | [Log2P2](https://docs.originlab.com/origin-help/log2p2-fitfunc/) | a, b | none | Logarithm | RV / DG / U2 |
| EG-CAT-FIT-120 | 118 | [Log3P1](https://docs.originlab.com/origin-help/log3p1-fitfunc/) | a, b, c | none | Logarithm | RV / DG / U2 |
| EG-CAT-FIT-121 | 119 | [Logarithm](https://docs.originlab.com/origin-help/logarithm-fitfunc/) | A | none | Logarithm | RV / DG / U2 |
| EG-CAT-FIT-122 | 120 | [Logistic](https://docs.originlab.com/origin-help/logistic-fitfunc/) | A1, A2, x0, p | p > 0.0 | Origin Basic Functions, Growth/Sigmoidal, Statistics | RV / DG / U2 |
| EG-CAT-FIT-123 | 121 | [Logistic5](https://docs.originlab.com/origin-help/logistic5-fitfunc/) | Amin, Amax, x0, h, s | s > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-124 | 122 | [Logistpk](https://docs.originlab.com/origin-help/logistipk-fitfunc/) | y0, xc, w, A | w > 0.0 | Origin Basic Functions, Growth/Sigmoidal, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-125 | 123 | [LogNormal](https://docs.originlab.com/origin-help/lognormal-fitfunc/) | y0, xc, w, A | xc > 0, w > 0 | Statistics, PFW, Peak Functions, Origin Basic Functions | RV / DG / U2 |
| EG-CAT-FIT-126 | 124 | [LognormalCDF](https://docs.originlab.com/origin-help/lognormalcdf-fitfunc/) | y0, A1, xc, w | A > 0.0, w > 0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-127 | 125 | [Lorentz](https://docs.originlab.com/origin-help/lorentz-fitfunc/) | y0, xc, w, A | w > 0.0 | Origin Basic Functions, Peak Functions, PFW, Spectroscopy, Statistics | RV / DG / U2 |
| EG-CAT-FIT-128 | 126 | [MichaelisMenten](https://docs.originlab.com/origin-help/michaelismenten-fitfunc/) | Vmax, Km | Vmax > 0.0, Km > 0.0 | Growth/Sigmoidal, Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-129 | 127 | [Mixed](https://docs.originlab.com/origin-help/mixed-fitfunc/) | Vmax, Km, Kas, Kac, A | Vmax > 0, Km > 0, Kas > 0, Kac > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-130 | 128 | [Mixed2](https://docs.originlab.com/origin-help/mixed2-fitfunc/) | Vmax, Km, Kas, Kac, S | Vmax > 0, Km > 0, Kas > 0, Kac > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-131 | 129 | [MixedFull](https://docs.originlab.com/origin-help/mixedfull-fitfunc/) | Vmax, Km, Ki, I, alpha | Vmax > 0, Km > 0, Ki > 0, I > 0, alpha > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-132 | 130 | [MixedFull2](https://docs.originlab.com/origin-help/mixedfull2-fitfunc/) | Vmax, Km, Ki, S, alpha | Vmax > 0, Km > 0, Ki > 0, S > 0, alpha > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-133 | 131 | [MixedInhibition](https://docs.originlab.com/origin-help/mixedinhibition-fitfunc/) | Vmax, Km, Ki, alpha, E, S | Vmax > 0, Km > 0, Ki > 0, alpha > 0, E > 0 , S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-134 | 132 | [MixedInhibition2](https://docs.originlab.com/origin-help/mixedinhibition2-fitfunc/) | Vmax, Km, Ki, alpha, E, I | Vmax > 0, Km > 0, Ki > 0, alpha > 0, E > 0 , I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-135 | 133 | [MixedModelInhib](https://docs.originlab.com/origin-help/mixedmodelinhib-fitfunc/) | Vmax, Km, Ki, Alpha, Ic | Vmax > 0.0, Km > 0.0, Ki > 0.0, Alpha > 0.0 | Enzyme Kinetics | RV / DG / U2 |
| EG-CAT-FIT-136 | 134 | [MixedPartial](https://docs.originlab.com/origin-help/mixedpartial-fitfunc/) | Vmax, Km, Ki, I, beta, alpha | Vmax > 0, Km > 0, Ki > 0, beta > 0, alpha > 0, I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-137 | 135 | [MixedPartial2](https://docs.originlab.com/origin-help/mixedpartial2-fitfunc/) | Vmax, Km, Ki, S, beta, alpha | Vmax > 0, Km > 0, Ki > 0, beta > 0, alpha > 0, S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-138 | 136 | [MnMolecular](https://docs.originlab.com/origin-help/mnmolecular-fitfunc/) | A, xc, k | A > 0.0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-139 | 137 | [MnMolecular1](https://docs.originlab.com/origin-help/mnmolecular1-fitfunc/) | A1, A2, k | A1 > 0.0, A2 > 0.0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-140 | 138 | [MYEGA](https://docs.originlab.com/origin-help/myega-fitfunc/) | y0, K, C | y0 > 0, K >= 0, C >= 0 | Rheology | RV / DG / U2 |
| EG-CAT-FIT-141 | 139 | [Nelder](https://docs.originlab.com/origin-help/nelder-fitfunc/) | a, b0, b1, b2 | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-142 | 140 | [NoncompetitiveFull](https://docs.originlab.com/origin-help/noncompetitivefull-fitfunc/) | Vmax, Ki, Km, I | Vmax > 0, Km > 0, Ki > 0, I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-143 | 141 | [NoncompetitiveFull2](https://docs.originlab.com/origin-help/noncompetitivefull2-fitfunc/) | Vmax, Ki, Km, S | Vmax > 0, Km > 0, Ki > 0, S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-144 | 142 | [NoncompetitiveInhibition](https://docs.originlab.com/origin-help/noncompetitiveinhibition-fitfunc/) | Vmax, Km, Ki, E, S | Vmax > 0, Km > 0, Ki > 0, E > 0 , S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-145 | 143 | [NoncompetitiveInhibition2](https://docs.originlab.com/origin-help/noncompetitiveinhibition2-fitfunc/) | Vmax, Km, Ki, E, I | Vmax > 0, Km > 0, Ki > 0, E > 0 , I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-146 | 144 | [NoncompetitivePartial](https://docs.originlab.com/origin-help/noncompetitivepartial-fitfunc/) | Vmax, Km, Ki, I, beta | Vmax > 0, Km > 0, Ki > 0, I > 0, beta > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-147 | 145 | [NoncompetitivePartial2](https://docs.originlab.com/origin-help/noncompetitivepartial2-fitfunc/) | Vmax, Km, Ki, S, beta | Vmax > 0, Km > 0, Ki > 0, S > 0, beta > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-148 | 146 | [NoncompInhib](https://docs.originlab.com/origin-help/noncompinhib-fitfunc/) | Vmax, Km, Ki, Ic | Vmax > 0, Km > 0, Ki >0 | Enzyme Kinetics | RV / DG / U2 |
| EG-CAT-FIT-149 | 147 | [Nonessencial](https://docs.originlab.com/origin-help/nonessencial-fitfunc/) | Vmax, Km, Ki, alpha, beta, A | Vmax > 0, Km > 0, Ki > 0, alpha > 0 , beta > 0 , A (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-150 | 148 | [Nonessencial2](https://docs.originlab.com/origin-help/nonessencial2-fitfunc/) | Vmax, Km, Ki, alpha, beta, A | Vmax > 0, Km > 0, Ki > 0, alpha > 0 , beta > 0 , S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-151 | 149 | [NormalCDF](https://docs.originlab.com/origin-help/normalcdf-fitfunc/) | y0, A, xc, w | A > 0.0, w > 0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-152 | 150 | [OneSiteBind](https://docs.originlab.com/origin-help/onesitebind-fitfunc/) | Bmax, K1 | none | Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-153 | 151 | [OneSiteComp](https://docs.originlab.com/origin-help/onesitecomp-fitfunc/) | A1, A2, log(x0) | none | Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-154 | 152 | [OrderedBiBiSequential](https://docs.originlab.com/origin-help/orderedbibisequential-fitfunc/) | Vmax, B, Ka, Kb | Vmax > 0, B > 0, Ka > 0, Kb > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-155 | 153 | [OrderedBiBiSequential2](https://docs.originlab.com/origin-help/orderedbibisequential2-fitfunc/) | Vmax, A, Ka, Kb | Vmax > 0, A > 0, Ka > 0, Kb > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-156 | 154 | [Parabola](https://docs.originlab.com/origin-help/parabola-fitfunc/) | A, B, C | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-157 | 155 | [Pareto](https://docs.originlab.com/origin-help/pareto-fitfunc/) | A | none | Statistics, Power | RV / DG / U2 |
| EG-CAT-FIT-158 | 156 | [Pareto2](https://docs.originlab.com/origin-help/pareto2-fitfunc/) | a, b | a > 0, b > 0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-159 | 157 | [PearsonIV](https://docs.originlab.com/origin-help/pearsoniv-fitfunc/) | y0, A, m, v, alpha, lam | m > 0.5 | Peak Functions, Statistics | RV / DG / U2 |
| EG-CAT-FIT-160 | 158 | [PearsonVII](https://docs.originlab.com/origin-help/pearsonvii-fitfunc/) | xc, A, w, mu | A > 0.0, w > 0.0, mu > 0.0 | Spectroscopy, PFW, Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-161 | 159 | [PingPongBiBiSequential](https://docs.originlab.com/origin-help/pingpongbibisequential-fitfunc/) | B, Kb, Ka, Vmax | B > 0, Ka > 0, Kb > 0, Vmax > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-162 | 160 | [PingPongBiBiSequential2](https://docs.originlab.com/origin-help/pingpongbibisequential2-fitfunc/) | A, Kb, Ka, Vmax | A > 0, Ka > 0, Kb > 0, Vmax > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-163 | 161 | [PIPlatt](https://docs.originlab.com/origin-help/piplatt-fitfunc/) | Pm, alpha | Pm > 0, alpha > 0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-164 | 162 | [PIPlatt2](https://docs.originlab.com/origin-help/piplatt2-fitfunc/) | Ps, alpha, beta | Ps > 0, alpha > 0, beta >= 0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-165 | 163 | [PIWebb](https://docs.originlab.com/origin-help/piwebb-fitfunc/) | Pm, alpha | Pm > 0, alpha > 0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-166 | 164 | [PlateauShaped](https://docs.originlab.com/origin-help/plateaushaped-fitfunc/) | Vmaxpla, Vmaxlim, pKa, pKb | Vmaxpla > 0, Vmaxlim > 0, pKa > 0, pKb > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-167 | 165 | [Poisson](https://docs.originlab.com/origin-help/poisson-fitfunc/) | y0, r | r0 > 0 | Origin Basic Functions, Statistics | RV / DG / U2 |
| EG-CAT-FIT-168 | 166 | [Poly](https://docs.originlab.com/origin-help/poly-fitfunc/) | a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 | none | Polynomial | RV / DG / U2 |
| EG-CAT-FIT-169 | 167 | [Poly4](https://docs.originlab.com/origin-help/poly4-fitfunc/) | A0, A1, A2, A3, A4 | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-170 | 168 | [Poly5](https://docs.originlab.com/origin-help/poly5-fitfunc/) | A0, A1, A2, A3, A4, A5 | none | Polynomial, Baseline | RV / DG / U2 |
| EG-CAT-FIT-171 | 169 | [Pow2P1](https://docs.originlab.com/origin-help/pow2p1-fitfunc/) | a, b | none | Power | RV / DG / U2 |
| EG-CAT-FIT-172 | 170 | [Pow2P2](https://docs.originlab.com/origin-help/pow2p2-fitfunc/) | a, b | none | Power | RV / DG / U2 |
| EG-CAT-FIT-173 | 171 | [Pow2P3](https://docs.originlab.com/origin-help/pow2p3-fitfunc/) | a, b | none | Power | RV / DG / U2 |
| EG-CAT-FIT-174 | 172 | [Power](https://docs.originlab.com/origin-help/power-fitfunc/) | A | none | Power | RV / DG / U2 |
| EG-CAT-FIT-175 | 173 | [Power0](https://docs.originlab.com/origin-help/power0-fitfunc/) | y0, xc, A, P | A > 0.0 | Power | RV / DG / U2 |
| EG-CAT-FIT-176 | 174 | [Power1](https://docs.originlab.com/origin-help/power1-fitfunc/) | xc, A, P | A > 0.0, P > 0.0 | Power | RV / DG / U2 |
| EG-CAT-FIT-177 | 175 | [Power2](https://docs.originlab.com/origin-help/power2-fitfunc/) | xc, A, pl, pu | A > 0.0, p1 > 0.0, pu > 0.0 | Power | RV / DG / U2 |
| EG-CAT-FIT-178 | 176 | [PsdVoigt1](https://docs.originlab.com/origin-help/psdvoigt1-fitfunc/) | y0, xc, A, w, mu | w > 0.0 | Spectroscopy | RV / DG / U2 |
| EG-CAT-FIT-179 | 177 | [PsdVoigt2](https://docs.originlab.com/origin-help/psdvoigt2-fitfunc/) | y0, xc, A, wG, wL, mu | wG > 0.0, wL > 0.0 | Spectroscopy | RV / DG / U2 |
| EG-CAT-FIT-180 | 178 | [Pulse](https://docs.originlab.com/origin-help/pulse-fitfunc/) | y0, x0, A, t1, P, t2 | A > 0.0, t1 > 0.0, P > 0.0, t2 > 0.0 | Origin Basic Functions, PFW | RV / DG / U2 |
| EG-CAT-FIT-181 | 179 | [PWL2](https://docs.originlab.com/origin-help/pwl2/) | a1, k1, xi, k2 | none | Piecewise | RV / DG / U2 |
| EG-CAT-FIT-182 | 180 | [PWL3](https://docs.originlab.com/origin-help/pwl3/) | a1, k1, xi1, k2, xi2, k3 | none | Piecewise | RV / DG / U2 |
| EG-CAT-FIT-183 | 181 | [RandomBiBiSequential](https://docs.originlab.com/origin-help/randombibisequential-fitfunc/) | Vmax, B, Ka, Kb, alpha | Vmax > 0, B > 0, Ka (none), Kb (none), alpha (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-184 | 182 | [RandomBiBiSequential2](https://docs.originlab.com/origin-help/randombibisequential2-fitfunc/) | Vmax, A, Ka, Kb, alpha | Vmax > 0, A > 0, Ka (none), Kb (none), alpha (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-185 | 183 | [Rational0](https://docs.originlab.com/origin-help/rational0-fitfunc/) | a, b, c | none | Origin Basic Functions, Rational | RV / DG / U2 |
| EG-CAT-FIT-186 | 184 | [Rational1](https://docs.originlab.com/origin-help/rational1-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-187 | 185 | [Rational2](https://docs.originlab.com/origin-help/rational2-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-188 | 186 | [Rational3](https://docs.originlab.com/origin-help/rational3-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-189 | 187 | [Rational4](https://docs.originlab.com/origin-help/rational4-fitfunc/) | a, b, c | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-190 | 188 | [Rational5](https://docs.originlab.com/origin-help/rational5-fitfunc/) | a, b, c, d | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-191 | 189 | [Rayleigh](https://docs.originlab.com/origin-help/rayleigh-fitfunc/) | b | b>0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-192 | 190 | [Reciprocal](https://docs.originlab.com/origin-help/reciprocal-fitfunc/) | a, b | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-193 | 191 | [Reciprocal0](https://docs.originlab.com/origin-help/reciprocal0-fitfunc/) | A | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-194 | 192 | [Reciprocal1](https://docs.originlab.com/origin-help/reciprocal1-fitfunc/) | A | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-195 | 193 | [ReciprocalMod](https://docs.originlab.com/origin-help/reciprocalmod-fitfunc/) | a, b | none | Rational | RV / DG / U2 |
| EG-CAT-FIT-196 | 194 | [RectHyperbola](https://docs.originlab.com/origin-help/recthyperbola-fitfunc/) | a, b | none | Hyperbola | RV / DG / U2 |
| EG-CAT-FIT-197 | 195 | [SawtoothWave](https://docs.originlab.com/origin-help/sawtoothwave-fitfunc/) | x0, y0, A, T | none | Waveform | RV / DG / U2 |
| EG-CAT-FIT-198 | 196 | [SchulzFlory](https://docs.originlab.com/origin-help/schulzflory-fitfunc/) | y0, xc, w, A | xc > 0, w > 0 | PFW | RV / DG / U2 |
| EG-CAT-FIT-199 | 197 | [SGompertz](https://docs.originlab.com/origin-help/sgompertz-fitfunc/) | a, xc, k | a > 0.0, k > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-200 | 198 | [Shah](https://docs.originlab.com/origin-help/shah-fitfunc/) | a, b, c, r | r > 0.0; upper: r < 1.0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-201 | 199 | [Sine](https://docs.originlab.com/origin-help/sine-fitfunc/) | xc, w, A, y0 | w > 0 | Origin Basic Functions, Waveform, PFW | RV / DG / U2 |
| EG-CAT-FIT-202 | 200 | [SineDamp](https://docs.originlab.com/origin-help/sinedamp-fitfunc/) | y0, xc, w, t0, A | w > 0.0 , t0>0.0 | Waveform, PFW | RV / DG / U2 |
| EG-CAT-FIT-203 | 201 | [SineSqr](https://docs.originlab.com/origin-help/sinesqr-fitfunc/) | xc, w, A, y0 | w > 0.0 | Waveform, PFW | RV / DG / U1 |
| EG-CAT-FIT-204 | 202 | [SingleExp](https://docs.originlab.com/origin-help/singleexp-fitfunc/) | ymax, k | ymax > 0, k > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-205 | 203 | [SingleExpOffset](https://docs.originlab.com/origin-help/singleexpoffset-fitfunc/) | ymax, y0, k | ymax > 0, y0 > 0, k > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-206 | 204 | [SinglepKaFalling](https://docs.originlab.com/origin-help/singlepkafalling-fitfunc/) | Vmaxlim, pKa | Vmaxlim > 0, pKa > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-207 | 205 | [SinglepKaOffset](https://docs.originlab.com/origin-help/singlepkaoffset-fitfunc/) | Vlim1, Vlim2, pKa | Vlim1 > 0, Vlim2 > 0, pKa > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-208 | 206 | [SinglepKaRising](https://docs.originlab.com/origin-help/singlepkarising-fitfunc/) | Vmaxlim, pKa | Vmaxlim > 0, pKa > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-209 | 207 | [SLogistic1](https://docs.originlab.com/origin-help/slogistic1-fitfunc/) | a, xc, k | xc > 0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-210 | 208 | [SLogistic2](https://docs.originlab.com/origin-help/slogistic2-fitfunc/) | y0, a, Wmax | y0 > 0.0, a > 0.0, Wmax > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-211 | 209 | [SLogistic3](https://docs.originlab.com/origin-help/slogistic3-fitfunc/) | a, b, k | a > 0.0, b > 0.0, k >0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-212 | 210 | [Specific](https://docs.originlab.com/origin-help/specific-fitfunc/) | Vmax, Km, Ka, A | Vmax > 0, Km > 0, Ka > 0, A (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-213 | 211 | [Specific2](https://docs.originlab.com/origin-help/specific2-fitfunc/) | Vmax, Km, Ka, S | Vmax > 0, Km > 0, Ka > 0, S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-214 | 212 | [SquareWave](https://docs.originlab.com/origin-help/squarewave-fitfunc/) | a, b, x0, T | none | Waveform | RV / DG / U2 |
| EG-CAT-FIT-215 | 213 | [SquareWaveMod](https://docs.originlab.com/origin-help/sqwavemod-fitfunc/) | a, b, x0, duty, T | none | Waveform | RV / DG / U2 |
| EG-CAT-FIT-216 | 214 | [SRichards1](https://docs.originlab.com/origin-help/srichards1-fitfunc/) | a, xc, d, k | a > 0.0, k > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-217 | 215 | [SRichards2](https://docs.originlab.com/origin-help/srichards2-fitfunc/) | a, xc, d, k | a > 0.0, k > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-218 | 216 | [Step](https://docs.originlab.com/origin-help/step-fitfunc/) | A, B, x1 | none | Waveform, Baseline | RV / DG / U2 |
| EG-CAT-FIT-219 | 217 | [Stirling](https://docs.originlab.com/origin-help/stirling-fitfunc/) | a, b, k | none | Exponential | RV / DG / U2 |
| EG-CAT-FIT-220 | 218 | [SubstrateActivationOrdered](https://docs.originlab.com/origin-help/substrateactivationordered-fitfunc/) | Vmax, Ks, alpha | Vmax > 0, Ks > 0, alpha > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-221 | 219 | [SubstrateActivationRandom](https://docs.originlab.com/origin-help/substrateactivationrandom-fitfunc/) | Vmax, Ks, alpha | Vmax > 0, Ks > 0, alpha > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-222 | 220 | [SubstrateInhib](https://docs.originlab.com/origin-help/substrateinhib-fitfunc/) | Vmax, Km, Ki | Km > 0.0, Ki > 0.0 | Enzyme Kinetics | RV / DG / U2 |
| EG-CAT-FIT-223 | 221 | [SWeibull1](https://docs.originlab.com/origin-help/sweibull1-fitfunc/) | A, xc, d, k | A > 0.0, k > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-224 | 222 | [SWeibull2](https://docs.originlab.com/origin-help/sweibull2-fitfunc/) | a, b, d, k | a > 0.0, b > 0.0, k > 0.0 | Growth/Sigmoidal | RV / DG / U2 |
| EG-CAT-FIT-225 | 223 | [ThermDena](https://docs.originlab.com/origin-help/thermdena-fitfunc/) | ri, rs, li, ls, Tm, dh, dcp | Tm > 0, dh > 0, dcp > 0 | Enzyme Kinetics2 | RV / DG / U2 |
| EG-CAT-FIT-226 | 224 | [TwoSiteBind](https://docs.originlab.com/origin-help/twositebind-fitfunc/) | Bmax1, Bmax2, k1, k2 | none | Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-227 | 225 | [TwoSiteComp](https://docs.originlab.com/origin-help/twositecomp-fitfunc/) | A1, A2, log(x01), log(x02), f | none | Pharmacology | RV / DG / U2 |
| EG-CAT-FIT-228 | 226 | [UncompetitiveFull](https://docs.originlab.com/origin-help/uncompetitivefull-fitfunc/) | Vmax, Km, Ki, I | Vmax > 0, Km > 0, Ki > 0, I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-229 | 227 | [UncompetitiveFull2](https://docs.originlab.com/origin-help/uncompetitivefull2-fitfunc/) | Vmax, Km, Ki, S | Vmax > 0, Km > 0, Ki > 0, S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-230 | 228 | [UncompetitiveInhibition](https://docs.originlab.com/origin-help/uncompetitiveinhibition-fitfunc/) | Vmax, Km, Ki, E, S | Vmax > 0, Km > 0, Ki > 0, E > 0, S (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-231 | 229 | [UncompetitiveInhibition2](https://docs.originlab.com/origin-help/uncompetitiveinhibition2-fitfunc/) | Vmax, Km, Ki, E, I | Vmax > 0, Km > 0, Ki > 0, E > 0, I (none) | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-232 | 230 | [UncompetitivePartial](https://docs.originlab.com/origin-help/uncompetitivepartial-fitfunc/) | Vmax, Km, Ki, I, beta | Vmax > 0, Km > 0, Ki > 0, I > 0, beta > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-233 | 231 | [UncompetitivePartial2](https://docs.originlab.com/origin-help/uncompetitivepartial2-fitfunc/) | Vmax, Km, Ki, S, beta | Vmax > 0, Km > 0, Ki > 0, S > 0, beta > 0 | Enzyme Kinetics 2 | RV / DG / U2 |
| EG-CAT-FIT-234 | 232 | [UncompInhib](https://docs.originlab.com/origin-help/uncompinhib-fitfunc/) | Vmax, Km, Kia, Ic | Vmax > 0, Km > 0, Kia >0 | Enzyme Kinetics | RV / DG / U2 |
| EG-CAT-FIT-235 | 233 | [VFT](https://docs.originlab.com/origin-help/vft-fitfunc/) | A, B, x0 | B >= 0, x0 > 0 | Rheology | RV / DG / U2 |
| EG-CAT-FIT-236 | 234 | [Voigt](https://docs.originlab.com/origin-help/voigt-fitfunc/) | y0, xc, A, wG, wL | wG > 0.0, wL > 0.0 | Origin Basic Functions, Peak Functions, PFW, Spectroscopy, Convolution | RV / DG / U2 |
| EG-CAT-FIT-237 | 235 | [Weibull](https://docs.originlab.com/origin-help/weibull-fitfunc/) | y0, a, r, u | a > 0.0, | Statistics | RV / DG / U1 |
| EG-CAT-FIT-238 | 236 | [Weibull3](https://docs.originlab.com/origin-help/weibull3-fitfunc/) | y0, xc, A, w1, w2 | w1 > 0.0, w2 > 0.0 | Peak Functions | RV / DG / U2 |
| EG-CAT-FIT-239 | 237 | [WeibullCDF](https://docs.originlab.com/origin-help/weibullcdf-fitfunc/) | y0, A1, a, b | A1 > 0.0, a > 0.0, b> 0.0 | Statistics | RV / DG / U2 |
| EG-CAT-FIT-240 | 238 | [YldFert](https://docs.originlab.com/origin-help/yldfert-fitfunc/) | a, b, r | r > 0.0; upper: r < 1.0 | Exponential | RV / DG / U2 |
| EG-CAT-FIT-241 | 239 | [YldFert1](https://docs.originlab.com/origin-help/yldfert1-fitfunc/) | a, b, k | k > 0.0 | Exponential | RV / DG / U2 |

### 4.3 Fitting anomalies and prerequisites

| Scope | Observation | Treatment |
|---|---|---|
| `EG-CAT-FIT-203` `SineSqr` | `Number: 3` but four names: `xc`, `w`, `A`, `y0` | `U1`; do not invent a fourth parameter meaning |
| `EG-CAT-FIT-015`, `-035`, `-036`, `-044`, `-045`, `-108`, `-109`, `-237` | Malformed bound expressions: trailing comma, doubled comma, `=>`, or `noe` instead of `none` | `U1`; preserve the vendor text; do not infer the missing term |
| `EG-CAT-FIT-037`, `-135`, `-148`, `-234` | Vendor note says global fit with shared parameters and per-dataset fixed constants | `PR-GLOBAL`; the note is not reproduced in full |
| `EG-CAT-FIT-043` `DoubleBoltzmann` | The 32.1 category table spells the name `:DoubleBoltzmann` | Identity resolved by the sidebar page; typo recorded as `U-39`; `U2` |
| `EG-CAT-FIT-008`, `-009`, `-026`, `-027`, `-030`, `-225` | Page token `Enzyme Kinetics2` without the space used by the catalog heading | `U2`; classification token unresolved |
| 17 PFW pages | `PFW` is not one of the 22 catalog headings | `U2`; token recorded, membership not reconciled |
| 15 Baseline pages | `Baseline` is not one of the 22 catalog headings | `U2`; token recorded, membership not reconciled |

The malformed bound rows are:

| Child ID | Function | Bound as published |
|---|---|---|
| EG-CAT-FIT-015 | Bingham | y0 >= 0, A > 0, |
| EG-CAT-FIT-035 | CompetitivePartial | Vmax > 0, Km > 0, , alpha > 0 |
| EG-CAT-FIT-036 | CompetitivePartial2 | Vmax > 0, Km > 0, Ki > 0, , alpha > 0 |
| EG-CAT-FIT-044 | DoubleExp | ymax1 => 0, k1 > 0, , k2 > 0 |
| EG-CAT-FIT-045 | DoubleExpOffset | , , , k1 > 0, k2 > 0 |
| EG-CAT-FIT-108 | Isoenzyme | Vmax1 > 0, Vmax2 > 0, |
| EG-CAT-FIT-109 | Langevin | noe |
| EG-CAT-FIT-237 | Weibull | a > 0.0, |

The PFW page IDs are: EG-CAT-FIT-005, EG-CAT-FIT-028, EG-CAT-FIT-046, EG-CAT-FIT-088, EG-CAT-FIT-089, EG-CAT-FIT-091, EG-CAT-FIT-092, EG-CAT-FIT-107, EG-CAT-FIT-125, EG-CAT-FIT-127, EG-CAT-FIT-160, EG-CAT-FIT-180, EG-CAT-FIT-198, EG-CAT-FIT-201, EG-CAT-FIT-202, EG-CAT-FIT-203, EG-CAT-FIT-236. The Baseline page IDs are: EG-CAT-FIT-038, EG-CAT-FIT-040, EG-CAT-FIT-065, EG-CAT-FIT-066, EG-CAT-FIT-071, EG-CAT-FIT-074, EG-CAT-FIT-075, EG-CAT-FIT-079, EG-CAT-FIT-080, EG-CAT-FIT-104, EG-CAT-FIT-114, EG-CAT-FIT-156, EG-CAT-FIT-169, EG-CAT-FIT-170, EG-CAT-FIT-218. The `Enzyme Kinetics2` page IDs are: EG-CAT-FIT-008, EG-CAT-FIT-009, EG-CAT-FIT-026, EG-CAT-FIT-027, EG-CAT-FIT-030, EG-CAT-FIT-225.

## 5. One-dimensional Peak Analyzer catalog (`ORG-PEAK-005`)

### 5.1 Scope and edition markers

P3 lists 28 one-dimensional functions, seven of them bold and marked Pro-only, and 21 shared with curve fitting. P0-005 sections 5.1 and 5.2 record that the over-25 claim is the one-dimensional scope and that no vendor total or surface-peak subset is published. All 28 receive a `EG-CAT-PEAK-*` child because the Peak Analyzer workflow is a distinct capability; the 21 shared rows also reference their 32.1 fitting child and do not duplicate its evidence.

The seven Pro-only entries have 32.4 pages and no 32.1 page: `BWF`, `FraserSuzuki`, `Bigaussian`, `HVL`, `ConsGaussian`, `DoniachSunjic` and `Gaussian_LorenCross`. The other 21 entries share a 32.1 page and parameter set. Pro-only status is a Peak Analyzer workflow marker from P3 and P0-005 section 4.4; it is not a general per-function edition column for the 21 shared rows.

### 5.2 Per-function enumeration

The `Edition` column records the P3 bold marker, not a claim that a shared function is available in every Origin edition. The `Shared fitting child` column names the existing or proposed 32.1 child. Every row uses `PR-PEAK`, or `PR-PRO` for the seven Pro-only rows; the last column is rights / delivery / uncertainty. Every row uses the shared source version in section 2.1 and the linked page as its source locator.

| Child ID | Function | Edition | Parameters | Bounds | Source | Shared fitting child | Codes |
|---|---|---|---|---|---|---|---|
| EG-CAT-PEAK-002 | [Asym2Sig](https://docs.originlab.com/origin-help/asym2sig-fitfunc/) | Shared | y0, xc, A, w1, w2, w3 | w1 > 0.0, w2 > 0.0, w3 > 0.0 | 32.1 | `EG-CAT-FIT-005` | RV / DG / U2 |
| EG-CAT-PEAK-003 | [ECS](https://docs.originlab.com/origin-help/ecs-fitfunc/) | Shared | y0, xc, A, w, a3, a4 | A > 0.0, w > 0.0 | 32.1 | `EG-CAT-FIT-046` | RV / DG / U2 |
| EG-CAT-PEAK-004 | [Gaussian](https://docs.originlab.com/origin-help/gaussian-function-fitfunc/) | Shared | y0, xc, A, w | w > 0.0 | 32.1 | `EG-CAT-FIT-089` | RV / DG / U2 |
| EG-CAT-PEAK-005 | [PearsonVII](https://docs.originlab.com/origin-help/pearsonvii-fitfunc/) | Shared | xc, A, w, mu | A > 0.0, w > 0.0, mu > 0.0 | 32.1 | `EG-CAT-FIT-160` | RV / DG / U2 |
| EG-CAT-PEAK-006 | [SineDamp](https://docs.originlab.com/origin-help/sinedamp-fitfunc/) | Shared | y0, xc, w, t0, A | w > 0.0 , t0>0.0 | 32.1 | `EG-CAT-FIT-202` | RV / DG / U2 |
| EG-CAT-PEAK-007 | [BWF](https://docs.originlab.com/origin-help/bwf-pafunc/) | Pro-only (P3 bold) | y0, xc, H, w, q | none | 32.4 | none (no 32.1 page) | RV / DG / U2 |
| EG-CAT-PEAK-008 | [FraserSuzuki](https://docs.originlab.com/origin-help/frasersuzuki-pafunc/) | Pro-only (P3 bold) | y0, xc, A, sig | lower 0; upper 0 (no parameter named) | 32.4 | none (no 32.1 page) | RV / DG / U1 |
| EG-CAT-PEAK-009 | [Gauss](https://docs.originlab.com/origin-help/gauss-fitfunc/) | Shared | y0, xc, w, A | w > 0.0 | 32.1 | `EG-CAT-FIT-087` | RV / DG / U2 |
| EG-CAT-PEAK-010 | [PsdVoigt1](https://docs.originlab.com/origin-help/psdvoigt1-fitfunc/) | Shared | y0, xc, A, w, mu | w > 0.0 | 32.1 | `EG-CAT-FIT-178` | RV / DG / U2 |
| EG-CAT-PEAK-011 | [SineSqr](https://docs.originlab.com/origin-help/sinesqr-fitfunc/) | Shared | xc, w, A, y0 | w > 0.0 | 32.1 | `EG-CAT-FIT-203` | RV / DG / U1 |
| EG-CAT-PEAK-012 | [Bigaussian](https://docs.originlab.com/origin-help/bigaussian-pafunc/) | Pro-only (P3 bold) | y0, xc, H, w1, w2 | w1 > 0, w2 > 0 | 32.4 | none (no 32.1 page) | RV / DG / U2 |
| EG-CAT-PEAK-013 | [GCAS](https://docs.originlab.com/origin-help/gcas-fitfunc/) | Shared | y0, xc, A, w, a3, a4 | w > 0.0 | 32.1 | `EG-CAT-FIT-092` | RV / DG / U2 |
| EG-CAT-PEAK-014 | [HVL](https://docs.originlab.com/origin-help/hvl-pafunc/) | Pro-only (P3 bold) | y0, xc, A, w, d | w>0 | 32.4 | none (no 32.1 page) | RV / DG / U2 |
| EG-CAT-PEAK-015 | [PsdVoigt2](https://docs.originlab.com/origin-help/psdvoigt2-fitfunc/) | Shared | y0, xc, A, wG, wL, mu | wG > 0.0, wL > 0.0 | 32.1 | `EG-CAT-FIT-179` | RV / DG / U2 |
| EG-CAT-PEAK-016 | [Sine](https://docs.originlab.com/origin-help/sine-fitfunc/) | Shared | xc, w, A, y0 | w > 0 | 32.1 | `EG-CAT-FIT-201` | RV / DG / U2 |
| EG-CAT-PEAK-017 | [CCE](https://docs.originlab.com/origin-help/cce-fitfunc/) | Shared | y0, xc1, A, w, k2, xc2, B, k3, xc3 | w > 0.0 | 32.1 | `EG-CAT-FIT-028` | RV / DG / U2 |
| EG-CAT-PEAK-018 | [GaussAmp](https://docs.originlab.com/origin-help/gaussamp-fitfunc/) | Shared | y0, xc, w, A | w > 0.0 | 32.1 | `EG-CAT-FIT-088` | RV / DG / U2 |
| EG-CAT-PEAK-019 | [InvsPoly](https://docs.originlab.com/origin-help/invspoly-fitfunc/) | Shared | y0, xc, w, A, A1, A2, A3 | w > 0.0 | 32.1 | `EG-CAT-FIT-107` | RV / DG / U2 |
| EG-CAT-PEAK-020 | [Pulse](https://docs.originlab.com/origin-help/pulse-fitfunc/) | Shared | y0, x0, A, t1, P, t2 | A > 0.0, t1 > 0.0, P > 0.0, t2 > 0.0 | 32.1 | `EG-CAT-FIT-180` | RV / DG / U2 |
| EG-CAT-PEAK-021 | [Voigt](https://docs.originlab.com/origin-help/voigt-fitfunc/) | Shared | y0, xc, A, wG, wL | wG > 0.0, wL > 0.0 | 32.1 | `EG-CAT-FIT-236` | RV / DG / U2 |
| EG-CAT-PEAK-022 | [ConsGaussian](https://docs.originlab.com/origin-help/consgaussian-pafunc/) | Pro-only (P3 bold) | y0, xc, A, w1, w2 | w1>0, w2>0 | 32.4 | none (no 32.1 page) | RV / DG / U2 |
| EG-CAT-PEAK-023 | [GaussMod](https://docs.originlab.com/origin-help/gaussmod-fitfunc/) | Shared | y0, A, xc, w, t0 | w > 0.0, t0 > 0.0 | 32.1 | `EG-CAT-FIT-091` | RV / DG / U2 |
| EG-CAT-PEAK-024 | [LogNormal](https://docs.originlab.com/origin-help/lognormal-fitfunc/) | Shared | y0, xc, w, A | xc > 0, w > 0 | 32.1 | `EG-CAT-FIT-125` | RV / DG / U2 |
| EG-CAT-PEAK-025 | [SchulzFlory](https://docs.originlab.com/origin-help/schulzflory-fitfunc/) | Shared | y0, xc, w, A | xc > 0, w > 0 | 32.1 | `EG-CAT-FIT-198` | RV / DG / U2 |
| EG-CAT-PEAK-026 | [Weibull3](https://docs.originlab.com/origin-help/weibull3-fitfunc/) | Shared | y0, xc, A, w1, w2 | w1 > 0.0, w2 > 0.0 | 32.1 | `EG-CAT-FIT-238` | RV / DG / U2 |
| EG-CAT-PEAK-027 | [DoniachSunjic](https://docs.originlab.com/origin-help/doniachsunjic-pafunc/) | Pro-only (P3 bold) | y0, xc, H, w, a | lower 0; upper 0 (no parameter named) | 32.4 | none (no 32.1 page) | RV / DG / U1 |
| EG-CAT-PEAK-028 | [Gaussian_LorenCross](https://docs.originlab.com/origin-help/gaussian-lorencross-pafunc/) | Pro-only (P3 bold) | y0, xc, A, w, s | w>0 | 32.4 | none (no 32.1 page) | RV / DG / U2 |
| EG-CAT-PEAK-029 | [Lorentz](https://docs.originlab.com/origin-help/lorentz-fitfunc/) | Shared | y0, xc, w, A | w > 0.0 | 32.1 | `EG-CAT-FIT-127` | RV / DG / U2 |

### 5.3 Peak anomalies

| Scope | Observation | Treatment |
|---|---|---|
| All seven Pro-only pages | No `Category` section is published on any 32.4 Pro-only page | `U2`; category membership cannot be transcribed |
| `EG-CAT-PEAK-011` `SineSqr` | Same page as 32.1 `SineSqr`: `Number: 3` but four names | `U1`; shared with `EG-CAT-FIT-203` |
| `EG-CAT-PEAK-008` `FraserSuzuki` | `Lower Bounds: 0` and `Upper Bounds: 0` with no parameter named | `U1`; the association is unresolved |
| `EG-CAT-PEAK-027` `DoniachSunjic` | `Lower Bounds: 0` and `Upper Bounds: 0` with no parameter named | `U1`; the association is unresolved |
| `EG-CAT-PEAK-012` `Bigaussian` | Appears in the 32.1 Peak Functions category table but has only a 32.4 Pro-only page and no 32.1 sidebar entry | Cross-appendix membership recorded as `U-38`; no 32.1 ID is created |
| `EG-CAT-PEAK-028` | Page title is `Gaussian LorenCross`; the function name uses `Gaussian_LorenCross` | Both forms recorded; no alias is invented |
| `EG-CAT-PEAK-007` `BWF` | No lower or upper bound is published | `none`; not an error |

No per-function Pro marker exists for the 21 shared peak rows. No surface-peak-only subset is published; section 6.1 records the six-name peak-shaped subset as a P0-005 engineering classification only.

## 6. Surface-fitting catalog (`ORG-PEAK-005`)

### 6.1 Scope and edition markers

P4 lists 22 surface-fitting names. The vendor guide F9 says 21 built-in surface models and gives `DoseResponse2D` as an example name; P4 uses `DoseResp`. Both counts satisfy the over-20 claim, but the 21-versus-22 difference and the naming difference remain open (`U-30`, P0-005 section 5.2). No per-function surface edition column is published.

P0-005 section 5.2 classifies six entries as peak-shaped: `Gauss2D`, `Gaussian2D`, `Lorentz2D`, `Voigt2D`, `Voigt2DMod` and `LogNormal2D`. This is an engineering subset, not a vendor-published surface-peak-only list. The other 16 are general surface models, including clearly non-peak forms such as `Plane`, `Poly2D`, `Polynomial2D`, `Rational2D` and `RationalTaylor`.

Surface fitting is documented as an OriginPro workflow at the feature level; no per-function edition column is published (P0-005 section 4.4). The 22 rows therefore remain `U2` for edition and numerical behavior.

`Gauss2D` reuses existing `EG-CAT-FIT-002`. The 32.3 `DoseResp` is a distinct six-parameter surface page; the 32.1 `DoseResp` is `EG-CAT-FIT-042` with four parameters. The shared name does not merge the rows (`U-40`).

### 6.2 Per-function enumeration

The bounds are transcribed from the page-level constraint text. The 32.3 index publishes names only and has no bound columns; where a function page publishes no constraint, `none published` is recorded and no bound is inferred. All new rows use `PR-SURF`; `Gauss2D` retains the existing row details and is marked `RO`. Every row uses the shared source version in section 2.1 and the linked page as its source locator.

| Child ID | Function | Parameters | Bounds | Codes | Note |
|---|---|---|---|---|---|
| EG-CAT-PEAK-030 | [Chebyshev2D](https://docs.originlab.com/origin-help/chebyshev2d-fitfunc/) | z0, A1, A2, B1, B2, C1 | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-031 | [Cosine](https://docs.originlab.com/origin-help/cosine-fitfunc/) | z0, A1, A2, B1, B2, C1 | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-032 | [DoseResp](https://docs.originlab.com/origin-help/doseresp-surface-fitfunc/) | z0, B, C, D, E, F | x, y > 0; C, E > 0 | RV / DG / U2 | Distinct 32.3 page; 32.1 `DoseResp` is `EG-CAT-FIT-042` with four parameters. |
| EG-CAT-PEAK-033 | [Exponential2D](https://docs.originlab.com/origin-help/exponential2d-fitfunc/) | z0, B, C, D | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-034 | [Extreme2D](https://docs.originlab.com/origin-help/extreme2d-fitfunc/) | z0, B, C, D, E, F | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-035 | [ExtremeCum](https://docs.originlab.com/origin-help/extremecum-fitfunc/) | z0, B, C, D, E, F, G, H | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-036 | [Fourier2D](https://docs.originlab.com/origin-help/fourier2d-fitfunc/) | z0, a, b, c, d, w1, w2 | none published | RV / DG / U2 |  |
| EG-CAT-FIT-002 | [Gauss2D](https://docs.originlab.com/origin-help/gauss2d-fitfunc/) | z0, A, xc, w1, yc, w2 | w1 > 0; w2 > 0 | RO / DG / U2 | Existing `EG-CAT-FIT-002` row; not changed. |
| EG-CAT-PEAK-037 | [GaussCum](https://docs.originlab.com/origin-help/gausscum-fitfunc/) | z0, B, C, D, E, F | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-038 | [Gaussian2D](https://docs.originlab.com/origin-help/gaussian2d-fitfunc/) | z0, A, xc, w1, yc, w2, theta | w1 > 0; w2 > 0; 0 <= theta <= 2*pi | RV / DG / U2 |  |
| EG-CAT-PEAK-039 | [LogisticCum](https://docs.originlab.com/origin-help/logisticcum-fitfunc/) | z0, B, C, D, E, F | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-040 | [LogNormal2D](https://docs.originlab.com/origin-help/lognormal2d-fitfunc/) | z0, B, C, D, E, F, G, H | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-041 | [Lorentz2D](https://docs.originlab.com/origin-help/lorentz2d-fitfunc/) | z0, A, xc, w1, yc, w2 | w1 > 0; w2 > 0 | RV / DG / U2 |  |
| EG-CAT-PEAK-042 | [Parabola2D](https://docs.originlab.com/origin-help/parabola2d-fitfunc/) | z0, a, b, c, d | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-043 | [Plane](https://docs.originlab.com/origin-help/plane-fitfunc/) | z0, a, b | none published | RV / DG / U1 | Vendor `Number: 4`; three names listed (`z0`, `a`, `b`). |
| EG-CAT-PEAK-044 | [Poly2D](https://docs.originlab.com/origin-help/poly2d-fitfunc/) | z0, a, b, c, d, f | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-045 | [Polynomial2D](https://docs.originlab.com/origin-help/polynomial2d-fitfunc/) | z0, A1, A2, A3, A4, A5, B1, B2, B3, B4, B5 | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-046 | [Power2D](https://docs.originlab.com/origin-help/power2d-fitfunc/) | z0, B, C, D, E, F | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-047 | [Rational2D](https://docs.originlab.com/origin-help/rational2d-fitfunc/) | z0, A01, B01, B02, B03, A1, A2, A2, B2, B1, B2 | none published | RV / DG / U1 | Vendor `Number: 11`; `A2` and `B2` each appear twice. |
| EG-CAT-PEAK-048 | [RationalTaylor](https://docs.originlab.com/origin-help/rationaltaylor-fitfunc/) | z0, A01, B01, B02, C02, A1, A2, B1, B2, C2 | none published | RV / DG / U2 |  |
| EG-CAT-PEAK-049 | [Voigt2D](https://docs.originlab.com/origin-help/voigt2d-fitfunc/) | z0, A, xc, w1, yc, w2, mu | w1 > 0; w2 > 0; 0 <= mu <= 1 | RV / DG / U2 |  |
| EG-CAT-PEAK-050 | [Voigt2DMod](https://docs.originlab.com/origin-help/voigt2dmod-fitfunc/) | z0, A, xc, w1, yc, w2, mu | w1 > 0; w2 > 0; 0 <= mu <= 1 | RV / DG / U2 |  |

### 6.3 Surface anomalies

| Scope | Observation | Treatment |
|---|---|---|
| `EG-CAT-PEAK-043` `Plane` | `Number: 4` but three names: `z0`, `a`, `b` | `U1`; the fourth parameter is not invented |
| `EG-CAT-PEAK-047` `Rational2D` | `Number: 11`; `A2` and `B2` each appear twice in the table | `U1`; duplicate names and the missing distinct subscripts are unresolved |
| 21 of 22 surface pages | No `Category` section | `U2`; only `Voigt2D` publishes `Surface Fitting`, which is not one of the 22 catalog headings |
| 32.3 index and function pages | The 32.3 index lists names only and has no bound columns; of the 22 function pages only `Gauss2D`, `Gaussian2D`, `Lorentz2D`, `DoseResp`, `Voigt2D` and `Voigt2DMod` publish a constraint | Only the published constraint is recorded; `none published` means no page constraint and no bound is inferred |
| F9 versus P4 | 21 models including `DoseResponse2D` versus 22 names including `DoseResp` | `U-30`; no row is added or removed |
| Surface peak subset | No vendor subset is published | Six-name subset is marked as a P0-005 engineering classification |

## 7. Gate status and integration ownership

The gate identifiers and their P0-005 status are defined in P0-003 section 10 and P0-005 section 9. The current status and closure conditions are transcribed from P0-005 section 9; no new gate evidence was obtained for the graph, export, compatibility or method/App gates. The integration-owner column is assigned under the accepted P0-004 register schema and the P0-000 board allocation, not by P0-003 or P0-005. This task does not change any gate state. The table records the current status, what P0-012 adds, and what remains before closure.

| Gate | Current status | P0-012 evidence effect | Remaining closure condition | Integration owner |
|---|---|---|---|---|
| `EG-CAT-GATE-002` graph | Open; claim basis corrected by P0-005 | None; P0-012 does not enumerate graph types | Distinct built-in membership or the vendor counting rule; a PLOT scene for every named type | EveryGraph Research, register integration; graph membership needs a separately allocated research task |
| `EG-CAT-GATE-003` fitting | Open; partially closed by P0-005 | 239 per-function rows, parameters, bounds and page links are now enumerated; the count basis, edition split and 2026b pin remain | Reconcile 170-plus/close-to-200/200-plus/239; resolve the 10 uncategorized pages and the parameter anomalies; publish the edition split and pin the 2026b binding; add reference fits | EveryGraph Research, register integration |
| `EG-CAT-GATE-004` peak | Open; partially closed by P0-005 | 28 one-dimensional and 22 surface names, Pro-only markers and parameters are now enumerated; the surface 21-versus-22 conflict, surface subset and 2026b pin remain | Resolve the surface count and subset; decide and publish edition behavior; pin the 2026b binding; add mixture and surface tests | EveryGraph Research, register integration |
| `EG-CAT-GATE-005` export | Open; claim corrected by P0-005 | None; P0-012 is fitting and peak only | Record the 16 documented and 15 listed formats, the 64-bit caveat, encoder rights and fixtures | EveryGraph Research, register integration; export research needs a separately allocated task |
| `EG-CAT-GATE-006` compatibility | Open; unchanged by P0-005 | None | Rights, versions and corpora per compatibility artifact | EveryGraph Research, register integration; compatibility research needs a separately allocated task |
| `EG-CAT-GATE-007` method/App lists | Open; unchanged by P0-005 | None | Reconcile method/option and App catalogs | EveryGraph Research, register integration; catalog research needs a separately allocated task |
| `EG-ADD-015` roll-up | Open | Enumeration advances GATE-003 and GATE-004 evidence only | All six catalog gates plus the cross-cutting gate; no partial row can close the roll-up | EveryGraph Research; P0-000 for allocation |

The register integration owner is EveryGraph Research under the accepted P0-004 schema. P0-004 is accepted and its record is read-only; admitting these proposed rows requires a separate authorized follow-up that creates or updates the catalog-child rows, extends the expectations and tests, and keeps the 428-row feature register unchanged. The contract digest re-pin belongs to P0-009. Board allocation belongs to P0-000. P0-012 edits none of those files.

No gate is closed, downgraded or converted to an acceptance claim by this document. The only status movement is evidence accumulation inside GATE-003 and GATE-004, and the gate rows remain Open.

## 8. Unknown register delta

The P0-003 section 12 and P0-005 section 11 unknown registers remain authoritative. This table records only deltas caused by the per-function enumeration. No unknown is closed by inference.

| ID | Status after P0-012 | Evidence or reason |
|---|---|---|
| `U-01` fitting membership | Page-level membership enumerated | 239 named 32.1 pages; the vendor count basis and unique-model definition remain open |
| `U-02` fitting categories and edition split | Partially resolved | 22 catalog headings and 3 page-only tokens recorded; per-function Origin versus OriginPro split remains unpublished |
| `U-03` peak membership and scope | Page-level membership enumerated | 28 one-dimensional and 22 surface names; the surface 21-versus-22, subset and 2026b pin remain open |
| `U-29` fitting appendix structure | Open | P0-012 records page-level category and parameter evidence but resolves none of the ten sidebar-only pages, the table typo or the cross-appendix name already recorded in P0-005 section 4.2 |
| `U-30` surface model count | Open | F9 says 21; P4 lists 22; `DoseResponse2D` versus `DoseResp` remains |
| `U-34` parameter-count contradictions | New | `SineSqr` `Number: 3` with four names; `Plane` `Number: 4` with three names |
| `U-35` duplicate surface parameter names | New | `Rational2D` lists `A2` and `B2` twice each; the intended distinct subscripts are not recoverable from the rendered table |
| `U-36` malformed, unassociated or mismatched bounds | New | Eight 32.1 pages have malformed bound text; `FraserSuzuki` and `DoniachSunjic` publish `0`/`0` without naming a parameter; `Poisson` bounds `r0` while the parameter is `r`; `Power2` bounds `p1` while the parameter is `pl`; `Nonessencial2` bounds `S` while the parameter is `A` |
| `U-37` page-only category tokens | New | `PFW` (17 pages), `Baseline` (15) and `Enzyme Kinetics2` (6) are page tokens with no catalog heading; `Quick Fit` is a heading with no page token |
| `U-38` cross-appendix `Bigaussian` | New | The 32.1 Peak Functions table names `Bigaussian`, but only a 32.4 Pro-only page exists |
| `U-39` catalog typo | New | `:DoubleBoltzmann` is a leading-colon typo for the sidebar page `DoubleBoltzmann` |
| `U-40` shared surface name | New | 32.1 `DoseResp` and 32.3 `DoseResp` share a name but publish four and six parameters respectively |

The unknown register does not claim that every page-level parameter is scientifically correct. It records what the vendor page publishes and where the publication is internally inconsistent.

## 9. What this document does not change

- `docs/planning/feature-traceability.csv` remains at 428 rows; the four parent requirements `ORG-FIT-004`, `ORG-PEAK-005`, `EG-ADD-015` and `ORG-GRAPH-001` remain `unresolved`.
- `docs/planning/catalog-children.csv` is unchanged; no proposed ID is admitted and no existing row is edited.
- `docs/planning/functional-scope.md`, `tools/validate_planning.py` and `tools/catalog_expectations.json` are unchanged.
- `origin-feature-inventory.md` and `datagraph-feature-inventory.md` remain unchanged.
- `docs/planning/task-board.csv` is P0-000-owned; this task did not edit it. The P0-000 board records P0-012 as `accepted` in commit `6bee412`; any later status transition belongs to P0-000.
- No other task record is edited. No product code, build configuration, dependency or platform claim is added.
- No gate is closed. No vendor runtime, manual, SDK, FDF file or sample was acquired; no rights decision is made.
- No Windows execution, push or publication occurred. P0-012 made no commit before the user authorized one on 2026-10-31; the authorized commit contains only the two P0-012 files and was not pushed. The two files were first included by another owner's snapshot commit `0277df4`, which also contains unrelated work.

## 10. Sources

Local evidence:

- [P0-003 catalog decomposition](capability-catalog-decomposition.md), accepted 2026-10-30; child-ID schema, code meanings and gate definitions.
- [P0-005 catalog evidence gaps 2026b](catalog-evidence-gaps-2026b.md), accepted 2026-10-31; counts, Pro-only markers, rights review, gate status and unknown delta.
- [Feature traceability](../planning/feature-traceability.csv), checked 2026-10-31; unchanged 428-row register.
- [Catalog children](../planning/catalog-children.csv), checked 2026-10-31; existing rows only.

Vendor pages:

- F4: [32.1 Curve Fitting Functions](https://docs.originlab.com/origin-help/curve-fitting-function/), Last-Modified 2026-10-26, retrieved 2026-10-30.
- P3: [32.4 Peak Analyzer Functions](https://docs.originlab.com/origin-help/peak-analyzer-functions/), Last-Modified 2026-10-26, retrieved 2026-10-30.
- P4: [32.3 Non-linear Surface Fitting](https://docs.originlab.com/origin-help/non-linear-surface-fitting-fitfunc/), Last-Modified 2026-10-26, retrieved 2026-10-30.
- F9: [OriginPro user guide](https://docs.originlab.com/user-guide/originpro/), not re-fetched by P0-012; 21-surface-model claim transcribed from P0-005 section 5.2.
- Representative anomaly pages: [SineSqr](https://docs.originlab.com/origin-help/sinesqr-fitfunc/), [Plane](https://docs.originlab.com/origin-help/plane-fitfunc/), [Rational2D](https://docs.originlab.com/origin-help/rational2d-fitfunc/), [FraserSuzuki](https://docs.originlab.com/origin-help/frasersuzuki-pafunc/), [DoniachSunjic](https://docs.originlab.com/origin-help/doniachsunjic-pafunc/) and [Gauss2D](https://docs.originlab.com/origin-help/gauss2d-fitfunc/).
- Every per-function page is linked in sections 4, 5 and 6; those links are the complete source list for the 289 rows and 268 unique pages.

The vendor pages are live and unversioned. This document does not claim an immutable snapshot. Quotes are limited to short factual fields.

## 11. Verification and limits

Working directory: `/Users/leo/Documents/General/EveryGraph`. Platform: macOS 27.0.1 arm64. Python: 3.13.9 and 3.14.2. No build was run because this is a documentation deliverable.

| Check | Result | Evidence and limits |
|---|---|---|
| Fetch 268 unique function pages | HTTP 200 for all 268 | 272 cached pages including four index pages; response dates 2026-10-30 19:11:17-19:11:41 UTC; all 272 responses carried Last-Modified 2026-10-26 09:00:17 or 09:00:18 UTC |
| 32.1 counts | 239 pages, 22 headings, 307 cells, 231 unique, 10 sidebar-only, 2 table-only | Parser-based count; matches P0-005 |
| 32.4 counts | 28 names, 7 bold Pro-only, 21 shared | Table-cell and bold markup; matches P0-005 |
| 32.3 counts | 22 names | Table-cell count; matches P0-005 |
| Identifier allocation | 239 FIT IDs, 28 plus 21 PEAK IDs, no collision | Sequence check; proposed only |
| Parameter parsing | 922 numbered fitting parameter slots and 923 listed fitting parameter names (`SineSqr` contributes the difference); 149 surface slots and 148 names (`Plane`); one duplicate-name surface row | Structural parse, not scientific validation |
| `python3 tools/validate_planning.py` | PASS | Result recorded in the P0-012 task record; the validator checks local links, fences, English policy and planning invariants, not vendor URLs or table semantics |
| Protected files | Unchanged | `git diff --name-only` and hash checks recorded in the task record; this document and the task record are the only P0-012 edits |

Checks not run and their reasons:

- No fit or peak numerical execution: no implementation exists and no vendor runtime was authorized.
- No equation or parameter-meaning reproduction: citation-only rights and no need to reproduce vendor expression.
- No FDF, manual or SDK inspection: not acquired and not authorized.
- No independent legal review: the rights classification is engineering evidence, not legal advice.
- No versioned 2026b snapshot: the Help site is live and unversioned.
- No surface-peak-only subset validation: no vendor page publishes one.
- No Windows execution: the task is document-only and the platform is out of scope.

