# Capability catalog decomposition: evidence and proposed child entries

**Research date: 2026-10-30.** Scope: P0-003; a dated P0-005 follow-up is appended in section 15 without rewriting the original findings. This is a candidate evidence document for the named catalog umbrellas `ORG-IO-001`, `ORG-GRAPH-001`, `ORG-FIT-004`, `ORG-PEAK-005`, `ORG-EXPORT-001`, `EG-ADD-014` and `EG-ADD-015`. It proposes stable child identifiers and a future validator contract. It does not modify the authoritative requirement register, the planning validator, product code or build configuration, and it does not claim catalog completeness, parity or delivery.

> P0-005 follow-up: [section 15](#15-p0-005-follow-up-2026-10-30-utc-2026-10-31-local) corrects the graph 73-row interpretation and updates the fitting, peak and export evidence. The original P0-003 text is retained for history; the follow-up identifies which earlier statements are superseded.

## 1. Decision summary

| Question | Result |
|---|---|
| Can the import umbrella be enumerated? | Yes at family level: 34 named file-format families and 2 source adapters are evidenced. The vendor claim is "more than 30 data formats" and is plausibly met, but the vendor's counting basis is not published. |
| Can the graph umbrella be enumerated? | Only partially: 73 named graph entries are evidenced against a claim of "Over 100+ graph types". The named-entry count is at least 27 below a 100-type floor; because some entries are categories, the true membership gap is unknown. |
| Can the fitting umbrella be enumerated? | No. No function list is published; one surface example ("2D Gaussian") is named in a product-page caption, and the totals conflict ("close to 200" and "more than 170"). The catalog remains open. |
| Can the peak umbrella be enumerated? | No. No individual peak function is named in the retrieved vendor pages; the claim is "over 25 built-in peak functions" with a separate "over 20" surface-fitting count. The catalog remains open. |
| Can the export umbrella be enumerated? | Partially: the official help page asserts 17 formats but its table lists 16 distinct formats. One claimed format is unaccounted. |
| Can the compatibility matrix be enumerated? | Yes as a rights and decision matrix: 14 artifacts or artifact classes are named in the inventories and functional scope. Every one remains rights-blocked or rights-unverified. |
| Is a child-ID schema and validator integration proposal available? | Yes, in section 11. It is a proposal; no file or validator change was made by P0-003. |
| Did the planning validator remain green? | Yes, after adding this document. The 428-row register and the validator were not modified. |

The absence of a named entry is recorded as an explicit unknown, not filled with a guess. Vendor marketing counts are evidence of a claim, not evidence of membership.

## 2. Evidence method

### 2.1 Evidence labels

- **Documented fact**: an explicit statement or named item on a retrieved official page, at the retrieval date. It is not independently tested behavior.
- **Source observation**: a statement in a preserved local inventory or planning document, with a file and line anchor.
- **Vendor claim**: a count or capability asserted by the vendor without a published complete list. It is recorded as a claim and never treated as an achieved catalog.
- **Unknown**: membership, parameter set, version scope, identity or rights that the available evidence does not resolve.
- **Inference**: a proposed interpretation for later review, never presented as vendor fact.

The two root inventories are preserved inputs with recorded SHA-256 hashes. Their row text and line numbers are the primary anchors. Official pages were retrieved on 2026-10-30; they are live documents without immutable revision identifiers, so a future check must record a new date and excerpt hash.

### 2.2 Codes used in the child tables

| Code | Meaning |
|---|---|
| `R-OPEN` | Public or open specification. Library choice, patent and distribution review still required before shipping. |
| `R-VENDOR` | Vendor-specific or proprietary format, or a format whose specification/implementation rights were not obtained. No support promise. |
| `R-BLOCKED` | No lawful specification, SDK, manual or file corpus is available. A compatibility claim is prohibited. |
| `R-OWN` | EveryGraph-defined behavior derived from a public capability description; no vendor artifact is required. |
| `D-NATIVE` | EveryGraph-owned implementation. |
| `D-ADAPTER` | Explicit file-format or service adapter boundary. |
| `D-GATE` | Research or rights gate before any support claim. |
| `D-PLATFORM` | Platform-specific integration with stated availability. |
| `U1` | Name and family confirmed; exact variant and parameter set open. |
| `U2` | Family confirmed; exact versions, extensions or variants open. |
| `U3` | Vendor claim only; membership not enumerated. |
| `U4` | Rights, specification or artifact identity unresolved. |
| `F-TXT` | Synthetic text corpus with locale, precision and malformed cases. |
| `F-BIN` | Rights-checked binary sample corpus. |
| `F-IMG` | Rights-checked image corpus. |
| `F-DB` | Local database fixture and query corpus. |
| `F-URL` | Local HTTP fixture server. |
| `F-SCENE` | Named graph scene with declared fonts and tolerances. |
| `F-SYNTH` | Synthetic numerical dataset with analytic ground truth and fixed tolerances. |
| `F-NONE` | No fixture until rights or specification exist. |
| `O-TABLE` | Typed table, metadata and an explicit loss report. |
| `O-MATRIX` | Typed matrix or grid with metadata. |
| `O-IMAGE` | Image object and pixel/metadata checks. |
| `O-SCENE` | Scene plus PLOT mapping and reference-image checks. |
| `O-VECTOR` | Structural vector-output checks and independent-reader verification. |
| `O-RASTER` | Raster output with dimension, DPI, alpha and color checks. |
| `O-PAIR` | Round-trip pair with declared losses. |
| `O-NONE` | No output claim. |

These codes compress repeated fields. The following per-catalog defaults apply to every child row unless the row states a narrower value; the row then supplies the remaining fields explicitly.

| Catalog | Rights default | Delivery default | Fixture and expected-output default |
|---|---|---|---|
| Import (`ORG-IO-001`) | Per row | Per row | Per row |
| Graph (`ORG-GRAPH-001`) | `R-OWN` | Per row (`D-NATIVE` or `D-GATE`) | `F-SCENE` -> `O-SCENE` |
| Fitting (`ORG-FIT-004`) | Per row | `D-GATE` | Per row |
| Peak (`ORG-PEAK-005`) | `R-BLOCKED` | `D-GATE` | `F-NONE` -> `O-NONE` |
| Export (`ORG-EXPORT-001`) | Per row | `D-ADAPTER`, except `EG-CAT-EXPORT-017` (`D-GATE`) | Per row |
| Compatibility (`EG-ADD-014`) | Per row | `D-GATE` | `F-NONE` -> `O-NONE` |
| Roll-up gate (`EG-ADD-015`) | Not applicable | Not applicable | No fixture; gate status only |

Every capability child therefore has a rights status, delivery class, fixture plan, expected output and uncertainty, either in its row or through this table. The gate rows are reconciliation controls, not capability claims.

### 2.3 Source register

| Code | Source | Version and date | What it supports | Limits |
|---|---|---|---|---|
| S1 | [Origin inventory](../../origin-feature-inventory.md), rows at lines 128, 147, 240, 259 and 382; version additions at lines 456-474 | Origin/OriginPro 2026b; checked 2026-10-30; preserved hash | Requirement anchors, named families, version additions, vendor claims | Inventory explicitly does not enumerate every dialog option or catalog member |
| S2 | [DataGraph inventory](../../datagraph-feature-inventory.md), rows at lines 100, 116, 264 and 276-277 | DataGraph 5.5 build 68; 5.4 manual; checked 2026-10-30; preserved hash | `.dgraph` package and single-file forms, legacy Plot, DTG vector export, `dgraph` command line | 5.5 release notes unavailable; manual is 5.4 |
| S3 | [Origin help: exporting a graph or layout page](https://docs.originlab.com/origin-help/expgraph-to-image) | Live help page 23.2; retrieved 2026-10-30 | Export format names and raster/vector classes | Page asserts 17 formats but lists 16 distinct formats |
| S4 | [Origin user guide: import data](https://docs.originlab.com/user-guide/import-data/) | Live user guide; retrieved 2026-10-30 | Third-party format family table | No explicit version label; some historical references only |
| S5 | [Origin product page](https://www.originlab.com/Origin) | Origin/OriginPro 2026b marketing page; retrieved 2026-10-30 | Umbrella claims and an import format token list | Marketing page; counts and counting basis unverified |
| S6 | [Origin Graph Gallery](https://www.originlab.com/GraphGallery) | Live gallery; retrieved 2026-10-30 | 68 specific filter categories, one aggregate label and card examples | Rotating showcase; not a complete product catalog |
| S7 | [Origin curve fitting page](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/CurveFitting) | Live product page; retrieved 2026-10-30 | "more than 170 built-in fitting functions"; Pro-only categories | No function names or complete categories |
| S8 | [Origin peak analysis page](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/PeakAnalysis) | Live product page; retrieved 2026-10-30 | Peak fitting options; surface peak fitting "over 20" functions | No peak function names |
| S9 | [Functional scope](../planning/functional-scope.md), catalog gate line 44, compatibility gate lines 109-110 and 117-121 | Project planning specification; checked 2026-10-30 | Required child fields, compatibility decision axes, open decisions | Planning scope, not vendor evidence |
| S10 | [Feature traceability](../planning/feature-traceability.csv) | 428 rows; checked 2026-10-30 | Parent IDs, statuses, delivery classifications and acceptance text | Phase-0 `planned`/`unresolved` feature states only |
| S11 | [Task board](../planning/task-board.csv) | 7 task rows at the P0-003 check; checked 2026-10-30 | Task IDs, dependencies and task lifecycle | Task states are not feature states; the board may change in parallel |

### 2.4 Retrieval and verification limits

- No Origin or OriginPro installation was available, so no dialog, menu, function list or runtime behavior was inspected. All vendor evidence is documentary.
- The help and user-guide pages are live and unversioned. S3 and S4 contain historical references to older releases but no explicit current-version banner.
- The Graph Gallery is a rotating showcase. Its filter vocabulary was extracted, but the displayed cards are not a stable enumeration and may include App-supported types.
- No proprietary format specification, SDK, manual corpus or vendor file was acquired. Rights statuses are engineering classifications, not legal advice.
- Exact parameter sets, defaults and option availability are mostly unknown. Rows say so rather than inventing values.
- Counts below are counts of named evidence entries in this document, not counts of product features or test cases.

## 3. Count reconciliation

| Catalog | Register row | Vendor claim | Named children proposed | Claim status | Open gap |
|---|---|---|---|---:|---|
| Import formats | `ORG-IO-001` | "more than 30 data formats" (S5); "30+ Import Formats" (S1:128) | 36 (34 file-format families, 2 source adapters) | Plausibly met at family level; counting basis unpublished | Version, parameter and fixture closure per family |
| Graph types | `ORG-GRAPH-001` | "Over 100+ graph types" (S5); "100+ Graph Types" (S1:147) | 73 named entries plus 1 claim-only gap row | Not met | At least 27 entries below the 100-type floor; true gap unknown |
| Fitting functions | `ORG-FIT-004` | "close to 200" (S5) and "more than 170" (S7); "nearly 200" (S1:240) | 1 named plus 1 claim-only gap row | Unknown | Complete catalog unnamed; one caption example; count conflict unresolved |
| Peak functions | `ORG-PEAK-005` | "over 25" (S5); "over 20" surface functions (S5, S8); "25+" (S1:259) | 0 named plus 1 claim-only gap row | Unknown | Whole catalog unnamed; 1D and surface scopes not separated |
| Graph export formats | `ORG-EXPORT-001` | "over 15" (S5); "17" (S1:382, S3) | 16 named plus 1 claim-only gap row | Partially met | One asserted format is unaccounted |
| Compatibility artifacts | `EG-ADD-014` | No count claim | 14 named entries | Matrix enumerated; all rights gates open | Rights, versions and lawful corpora per artifact |
| Catalog roll-up | `EG-ADD-015` | No count claim | 7 gate entries | Open | Reconciliation of every catalog above, plus method/options and App lists |

The umbrella counts in the register remain `unresolved`. P0-003 does not change them.

## 4. Import catalog: `ORG-IO-001`

### 4.1 What the evidence establishes

S1:128 states that Origin supports "30+ Import Formats" including ASCII, CSV, Excel and binary data. The register's related rows name ASCII/CSV/TXT (`ORG-IO-002`), Excel XLS/XLSX/XLSM (`ORG-IO-003`), ADO/ODBC (`ORG-IO-004`), NetCDF and GeoTIFF (`ORG-IO-005`), MATLAB (`ORG-IO-006`), data connectors (`ORG-IO-007`), drag-and-drop and clipboard (`ORG-IO-008`), re-import and periodic import (`ORG-IO-009`), XML (`ORG-IO-011`) and the Digitizer (`ORG-IO-012`). S1:467 and S1:472 add Prism import and SVG import into worksheet cells; S1:456 records an improved Minitab connector.

S5 publishes a token list under the "more than 30 data formats" claim: `ASCII, CSV, EXCEL, Binary, PCLAMP, CDF, DCF, HPF, EPA, EDF, BDF, REC, HYP, DAT, MDF, DAT, RAW, H5, HE5, HDF5, DAT, PXP, IBW, DX, DX1, JDX, JCM, QDA, MAT, MTW, MPJ, mzData, mzXML, mzML, imzML, NC, DAT, TDM, TDMS, ABF, DAT, AB, SPE, PZFX, XML, JNB, SIE, WAV, SAV, SPC, CGM, BRUKER OPUS, ISF, SAS7BDAT`.

The generic token `Binary` in S1:128 and S5 is a category label, not a distinct family. The specific binary families are enumerated below; the generic label is not counted as an additional family.

S4 publishes a third-party family table with extensions: Thermo (SPC, CGM), pCLAMP (ABF, DAT, AB?), Princeton Instruments (SPE), Sound (WAV), NetCDF (NC), CDF (CDF), HDF5 (H5, HE, HDF5), NI DIAdem (DAT), National Instruments Famos (DAT, RAW), ETAS INCA MDF (DAT, MDF), IgorPro (PXP, IBW), Prism (PZFX, XML), MATLAB (Mat), Minitab (MTW, MPJ), KaleidaGraph (QDA), SigmaPlot (JNB), MZXML (mzData, mzXML, mzML, imzML), EarthProbe (EPA), EDF (EDF, BDF, REC, HYP), Somat SIE (SIE), JCAMP-DX (DX, DX1, JDX, JCM), HEKA (DAT), SPSS (SAV), BRUKER OPUS, SAS (sas7bdat), Tektronix (ISF), Data Translation (DCF, HPF), Binary 2D Array (IMG, B2D), Image.

The product list adds TDM/TDMS, AB and HE5, which the family table does not show as separate extensions. The family table adds GeoTIFF evidence only through S1, not S4. The two sources are compatible at family level but not identical.

**Interpretation:** the "more than 30" claim is plausibly satisfied by named families, and clearly satisfied if extensions are counted separately. Because the vendor's counting unit is unpublished, this document does not declare the claim proven. The acceptance gate still requires per-format fixtures and version scopes.

### 4.2 Child entries

The parent is `ORG-IO-001`. `Register family` maps an entry to the existing row that owns its behavior; the mapping is a proposal for review. Unless stated otherwise, an import child uses `D-ADAPTER` and a fixture/expected pair shown in its row. Source codes refer to section 2.3.

| Child ID | Named family | Register family | Prerequisites | Variant or parameters to verify | Rights | Delivery | Fixture and expected output | Uncertainty |
|---|---|---|---|---|---|---|---|---|
| `EG-CAT-IO-001` | ASCII / CSV / TXT | `ORG-IO-002` | Text decoder; typed table | Delimiter, header, skip, region, encoding and locale rules; partial import | `R-OPEN` | `D-ADAPTER` | `F-TXT` -> `O-TABLE` | `U1` |
| `EG-CAT-IO-002` | Excel XLS / XLSX / XLSM | `ORG-IO-003` | OOXML and legacy BIFF reader; no Excel install | Sheet/range selection; formula-value policy; date system; precision | `R-VENDOR` | `D-ADAPTER` | `F-BIN` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-003` | NetCDF (NC) | `ORG-IO-005`, `ORG-RSGIS-001` | NetCDF reader; matrix model | Version scope, dimension order, v3 flattening, metadata | `R-OPEN` | `D-ADAPTER` | `F-BIN` -> `O-MATRIX` | `U2` |
| `EG-CAT-IO-004` | GeoTIFF | `ORG-IO-005`, `ORG-RSGIS-002` | TIFF reader; geospatial metadata model | CRS, georeferencing tags, band layout, no-georeference case | `R-OPEN` | `D-ADAPTER` | `F-IMG` -> `O-MATRIX` | `U2` |
| `EG-CAT-IO-005` | MATLAB MAT | `ORG-IO-006` | MAT reader | MAT versions, array classes, sparse/complex/struct coverage | `R-VENDOR` | `D-ADAPTER` | `F-BIN` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-006` | XML | `ORG-IO-011` | XML parser; mapping rules | Schema or XPath mapping; Unix timestamp detection; hostile XML limits | `R-OPEN` | `D-ADAPTER` | `F-TXT` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-007` | Prism PZFX / XML | `ORG-IO-001`; S1:467 | Prism reader; rights review | PZFX versions, tables, metadata, unsupported constructs | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-008` | Minitab MTW / MPJ | `ORG-IO-001`; S1:456 | Minitab reader; rights review | Versions, worksheets, columns and metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-009` | SVG into worksheet cell | `ORG-IO-001`; S1:472 | SVG parser; cell image object | SVG subset, size, transparency; scripts and external resources must not execute | `R-OPEN` | `D-ADAPTER` | `F-TXT` -> `O-IMAGE` | `U2` |
| `EG-CAT-IO-010` | Thermo SPC / CGM | `ORG-IO-001`; S4 | Vendor reader; rights review | Chromatography variants; versions and metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-011` | pCLAMP ABF / DAT / AB? | `ORG-IO-001`; S4; S5 adds AB | Vendor reader; rights review | ABF versions; the literal `AB?` is unresolved; sweep metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-012` | Princeton Instruments SPE | `ORG-IO-001`; S4 | Vendor reader; rights review | SPE versions, ROI/binning metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-MATRIX` | `U4` |
| `EG-CAT-IO-013` | Sound WAV | `ORG-IO-001`; S4 | Audio decoder; sample model | PCM/float, channels, sample rate, metadata | `R-OPEN` | `D-ADAPTER` | `F-BIN` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-014` | CDF | `ORG-IO-001`; S4 | CDF reader; identity review | Whether this is NASA CDF or another CDF; versions | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-MATRIX` | `U4` |
| `EG-CAT-IO-015` | HDF5 H5 / HE / HE5 / HDF5 | `ORG-IO-001`; S4; S5 adds HE5 | HDF5 reader | Versions, filters, groups/datasets, HDF-EOS meaning of HE/HE5 | `R-OPEN` | `D-ADAPTER` | `F-BIN` -> `O-MATRIX` | `U2` |
| `EG-CAT-IO-016` | NI DIAdem DAT / TDM / TDMS | `ORG-IO-001`; S4; S5 | Vendor reader; rights review | TDM/TDMS schema, channel groups, versions | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-017` | NI Famos DAT / RAW | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and channel metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-018` | ETAS INCA MDF DAT / MDF | `ORG-IO-001`; S4 | Vendor reader; rights review | MDF 3.x/4.x scope, signal groups | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-019` | IgorPro PXP / IBW | `ORG-IO-001`; S4 | Vendor reader; rights review | Packed experiment vs binary wave; versions | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-020` | KaleidaGraph QDA | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and column metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-021` | SigmaPlot JNB | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and worksheet mapping | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-022` | Mass spec mzData / mzXML / mzML / imzML | `ORG-IO-001`; S4 | XML/binary reader; spectra model | Schema versions, imaging metadata, binary arrays | `R-OPEN` | `D-ADAPTER` | `F-BIN` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-023` | EarthProbe EPA | `ORG-IO-001`; S4 | Vendor reader; rights review | Format identity and versions | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-024` | EDF / BDF / REC / HYP | `ORG-IO-001`; S4 | Biosignal reader | Version variants, channels, annotations | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-025` | Somat SIE | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and channel metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-026` | JCAMP-DX DX / DX1 / JDX / JCM | `ORG-IO-001`; S4 | JCAMP reader | Version scope and compression/encoding variants | `R-OPEN` | `D-ADAPTER` | `F-TXT` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-027` | HEKA DAT | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and sweep metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-028` | SPSS SAV | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions, variable labels, missing-value codes | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-029` | BRUKER OPUS | `ORG-IO-001`; S4 | Vendor reader; rights review | No extension in the table; versions and block structure | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-030` | SAS sas7bdat | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions, compression, encoding | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-031` | Tektronix ISF | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and waveform metadata | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-032` | Data Translation DCF / HPF | `ORG-IO-001`; S4 | Vendor reader; rights review | Versions and header layout | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-TABLE` | `U4` |
| `EG-CAT-IO-033` | Binary 2D Array IMG / B2D | `ORG-IO-001`; S4 | Binary reader; matrix model | Endianness, dtype, header, dimensions | `R-VENDOR` | `D-GATE` | `F-BIN` -> `O-MATRIX` | `U2` |
| `EG-CAT-IO-034` | Image (format set unspecified) | `ORG-IO-001`; S4 | Image decoder; image document | PNG/TIFF/JPEG/BMP and image-drop formats are not enumerated by S4 | `R-VENDOR` | `D-GATE` | `F-IMG` -> `O-IMAGE` | `U3` |
| `EG-CAT-IO-035` | Database ADO / ODBC (source adapter) | `ORG-IO-004` | Platform drivers; credentials broker | Drivers, authentication, query result types, refresh policy | `R-VENDOR` | `D-PLATFORM` | `F-DB` -> `O-TABLE` | `U2` |
| `EG-CAT-IO-036` | Data connectors: Web, local, network, cloud (source adapter) | `ORG-IO-007` | HTTP/cloud adapters; credential broker | Protocols, authentication, refresh and disconnect semantics | `R-VENDOR` | `D-ADAPTER` | `F-URL` -> `O-TABLE` | `U2` |

### 4.3 Import gaps and closure conditions

- **Version scopes are unknown for most third-party formats.** The family table does not give version numbers or extension matrices beyond the shown tokens.
- **`AB?` in S4 is a literal page artifact, not a confirmed extension.** It must be resolved from a vendor manual before a decoder is scoped.
- **CDF identity is ambiguous.** The page lists NetCDF (NC) and CDF (CDF) separately; the latter must not be assumed to be NASA CDF without evidence.
- **GeoTIFF is evidenced by S1 but absent from S4's third-party table.** The evidence basis and version scope need a separate check.
- **The image family is not enumerated.** "Image" in S4 does not establish PNG/TIFF/JPEG/BMP coverage, despite those formats appearing elsewhere in the product.
- **Database and connector rows are source adapters, not file formats.** They are included so that the import catalog does not silently omit named register members, but they are excluded from the 34-family format count.
- **The generic `Binary` token is not a family.** It is a category label in S1:128 and S5; the specific binary families carry the fixture obligations. Its counting treatment is recorded under `U-08`.
- **No fixture corpus exists.** Every `F-BIN` row requires a rights-checked sample before implementation; absence of a sample keeps the row at `U4`.

## 5. Graph catalog: `ORG-GRAPH-001`

### 5.1 What the evidence establishes

S1:147 states "100+ Graph Types" and lists family names: line/symbol, column/bar, pie/doughnut, multi-axis, waterfall, contour, heatmap, 3D, grouped, categorical, hierarchical, statistical, quality, specialized, polar, vector, ternary, hydrochemistry, profile, function and image. The register already assigns those families to `ORG-GRAPH-002` through `ORG-GRAPH-014`; `ORG-GRAPH-001` is the umbrella and count claim.

S5 states "Over 100+ graph types are available from the Plot Menu." S6 exposes 68 specific graph-type filter categories plus one aggregate "Advanced Statistical Graphs" label; the aggregate is not counted as a type, and the retrieved HTML does not itself state the 100+ count. Its rotating cards add named examples such as Grouped Histogram, CDF Plot, Arrhenius Plot, Grouped Scatter Plot, Faceted Grouped Column Chart, Polar Quiver Plot, Heatmap with Dendrogram, Treemap, CUSUM Chart and Wafer Map. Those cards are examples, not a stable complete list.

S1:457 and S1:473 name additional graph types in version notes: Cause and Effect diagram, Frequency Chart, Double-X, Stem-and-Leaf (2026); Overlaid Contour and Individual Value Chart (2026b).

The 73 named entries below are the evidence-backed floor. The umbrella remains open because the claim exceeds that floor by at least 27; because some entries are categories rather than individual chart types, the true number of unnamed types is unknown.

### 5.2 Child entries

The parent is `ORG-GRAPH-001`. `Register family` is a proposed mapping to the existing family row; it does not replace that row or create a new requirement. Unless stated otherwise, every graph child uses `R-OWN`, `F-SCENE` -> `O-SCENE`, and prerequisites "typed columns plus the 2D scene, axis and layer system". Delivery is `D-NATIVE` for 2D types and `D-GATE` where a 3D backend, map data, or App dependency is unresolved.

| Child ID | Named type | Register family | Prerequisites or variant scope to verify | Delivery | Uncertainty |
|---|---|---|---|---|---|
| `EG-CAT-GRAPH-001` | Area Plot | `ORG-GRAPH-003` | Grouped/stacked areas, baseline, orientation | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-002` | Bar Chart | `ORG-GRAPH-003` | Grouped/stacked, horizontal/vertical, width and gap | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-003` | Beeswarm Plot | `ORG-GRAPH-010` | Point avoidance, grouping, categorical axis | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-004` | Bland-Altman Plot | `ORG-GRAPH-010` | Difference vs mean, limits of agreement | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-005` | Box Chart | `ORG-GRAPH-010` | Whisker rule, outliers, grouped/stacked, notched variant | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-006` | Bridge chart | `ORG-GRAPH-003` | Bridge/waterfall columns with totals | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-007` | Bubble Plot | `ORG-GRAPH-002` | X/Y/size/color mapping, size scaling | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-008` | Bullet Chart | `ORG-GRAPH-010` | Target and qualitative ranges | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-009` | Chord Diagram | `ORG-GRAPH-011` | Square matrix, ribbon weights, ordering | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-010` | Circular Packing | `ORG-GRAPH-009` | Hierarchy input, circle packing, labels | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-011` | Cluster Plot | `ORG-GRAPH-009` | Distance/linkage analysis, dendrogram order | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-012` | Contour Plot | `ORG-GRAPH-007` | XYZ/matrix input, levels, lines/fill, overlaid contour variant | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-013` | Contour Profile | `ORG-GRAPH-007` | Contour plus profile cut and markers | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-014` | Control Chart | `ORG-GRAPH-010` | X-bar/R, individuals, limits, phase rules | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-015` | Correlation Plot | `ORG-GRAPH-011` | Correlation matrix, ellipse/color encoding | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-016` | Dots Plot | `ORG-GRAPH-010` | Dot histogram, stacking, grouping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-017` | Double-Y Plot | `ORG-GRAPH-005` | Two Y scales, shared X, layer linking | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-018` | Double-X Plot | `ORG-GRAPH-005` | Shared-Y and range-independent variants (S1:457, S6) | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-019` | Ellipse | `ORG-GRAPH-015` | Data ellipse or confidence ellipse overlay | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-020` | Fence Plot | `ORG-GRAPH-006` | Offset series, projection and order | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-021` | Function Graph | `ORG-GRAPH-013` | Expression parser, domain sampling, discontinuity policy | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-022` | Graph Browser | `ORG-GRAPH-014` | Multi-channel faceting, navigation, selection | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-023` | Heatmap | `ORG-GRAPH-007` | Matrix/XYZ input, colormap, grouping, dendrogram variant | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-024` | Hierarchical Edge Bundling | `ORG-GRAPH-011` | Hierarchy plus adjacency, bundling strength | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-025` | High-Low-Close Chart | `ORG-GRAPH-010` | Open/high/low/close columns or equivalent | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-026` | Histogram | `ORG-GRAPH-010` | Binning, density/CDF/probability variants, grouped variant | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-027` | Image Profile Plot | `ORG-GRAPH-013` | Image document plus profile line | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-028` | Inset Plot | `ORG-GRAPH-009` | Inset axes, viewport, clipping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-029` | Interval Plot | `ORG-GRAPH-010` | Mean and confidence interval, grouping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-030` | Japanese Candlestick | `ORG-GRAPH-010` | Body, wick, color convention, date axis | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-031` | Kite Diagram | `ORG-GRAPH-011` | Stratigraphic kite shapes and ordering | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-032` | Line Scatter Plot | `ORG-GRAPH-002` | Line/symbol, step and smooth variants | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-033` | Lollipop Plot | `ORG-GRAPH-002` | Stem plus marker, categorical axis | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-034` | Maps | `ORG-GRAPH-011` | Projection, basemap, geospatial input and data rights; chart is `R-OWN`, basemap data rights are separate, no bundled map data | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-035` | Marginal Plot | `ORG-GRAPH-009` | Central scatter plus marginal distributions | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-036` | Multi-Panel Plot | `ORG-GRAPH-005` | Panel grid, shared/independent axes, labels | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-037` | Network Plot | `ORG-GRAPH-011` | Node/edge input, layout, weights, labels | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-038` | Parallel / Alluvial / Sankey / Ribbon Plot | `ORG-GRAPH-011` | Categorical flow, node ordering, ribbon widths; aliases Parallel, Alluvial, Sankey and Ribbon must be listed in `variant_parameters` when converted to CSV | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-039` | Pareto Chart | `ORG-GRAPH-010` | Descending bars plus cumulative line | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-040` | Pie and Doughnut Chart | `ORG-GRAPH-004` | Slices, multiple rings, labels, start angle | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-041` | Piper Diagram | `ORG-GRAPH-013` | Hydrochemistry ion geometry and diamond field | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-042` | Polar Chart | `ORG-GRAPH-012` | Theta/r mapping, angle units, zero direction | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-043` | Pyramid Plot | `ORG-GRAPH-009` | Population pyramid, mirrored grouping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-044` | Quality Control | `ORG-GRAPH-010` | QC chart family; exact variants require versioned statistical/QC documentation | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-045` | Q-Q Plot | `ORG-GRAPH-010` | Theoretical distribution, quantile scaling, reference line | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-046` | Radar Chart | `ORG-GRAPH-012` | Axes around circle, scaling, filled/line variants | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-047` | Radial Plot | `ORG-GRAPH-012` | Radial axes and categorical placement | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-048` | Ribbon Plot | `ORG-GRAPH-008` | 3D ribbon rendering and orientation | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-049` | Ridgeline Chart | `ORG-GRAPH-010` | Offset density curves, overlap and order | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-050` | Scatter Matrix | `ORG-GRAPH-009` | Pairwise scatter plus diagonal summaries | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-051` | Schoeller Diagram | `ORG-GRAPH-013` | Semi-log hydrochemistry axes and ion ordering | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-052` | Smith Chart | `ORG-GRAPH-012` | Impedance/admittance grid and curves | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-053` | Spiral Bar Chart | `ORG-GRAPH-003` | Spiral layout, angle and radius mapping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-054` | Stiff Diagram | `ORG-GRAPH-013` | Hydrochemistry polygon and ion axes | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-055` | Streamline | `ORG-GRAPH-012` | Vector field, integration step, seed points | `D-NATIVE` | `U2` |
| `EG-CAT-GRAPH-056` | Sunburst Chart | `ORG-GRAPH-009` | Hierarchy rings, angular weights, labels | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-057` | Surface Plot | `ORG-GRAPH-008` | XYZ/matrix input, mesh/colormap, hidden-line policy | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-058` | Ternary | `ORG-GRAPH-012` | Three-component normalization and coordinate mapping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-059` | Tetrahedral Plot | `ORG-GRAPH-012` | Four-component geometry and projection | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-060` | Trellis Plot | `ORG-GRAPH-009` | Faceting, shared scales, panel order | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-061` | Variability Plot | `ORG-GRAPH-010` | Variability chart, grouping, summary statistics | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-062` | Vector Plot | `ORG-GRAPH-012` | Vector field, arrow scaling, color/size mapping | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-063` | Violin Plot | `ORG-GRAPH-010` | Kernel density, bandwidth, box overlay | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-064` | Walls Plot | `ORG-GRAPH-008` | 3D walls/planes and projection | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-065` | Waterfall Plot | `ORG-GRAPH-006` | Offset or stacked series; not the same as Bridge chart | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-066` | Wind Rose | `ORG-GRAPH-012` | Direction bins, speed bins, stacked sectors | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-067` | Wireframe Surface | `ORG-GRAPH-008` | XYZ/matrix input, mesh lines, hidden-line policy | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-068` | 2D Kernel Density Plot | `ORG-GRAPH-010` | Scatter input, kernel/bandwidth, contour or raster output | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-069` | Cause and Effect diagram | `ORG-GRAPH-011` | Fishbone categories and causes (S1:457) | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-070` | Frequency Chart | `ORG-GRAPH-010` | Frequency bars and ordering (S1:457) | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-071` | Stem-and-Leaf | `ORG-GRAPH-010` | Stem/leaf split and ordering (S1:457) | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-072` | Individual Value Chart | `ORG-GRAPH-010` | Individual points with limits (S1:473) | `D-NATIVE` | `U1` |
| `EG-CAT-GRAPH-073` | Treemap | `ORG-GRAPH-009` | Hierarchy area weights and labels; gallery card only (S6) | `D-GATE` | `U2` |
| `EG-CAT-GRAPH-074` | Unenumerated graph types up to the 100+ claim | `ORG-GRAPH-001` | Claim-only gap row; membership not published | `D-GATE` | `U3` |

### 5.3 Graph gaps and closure conditions

- **The claim exceeds the named-entry count by at least 27.** The 73 named entries include gallery categories and four version-named types; the gallery itself is a rotating showcase and may include App-supported examples. The true number of unnamed types is unknown because some entries are categories.
- **Gallery category to register-family mapping is proposed, not approved.** Several categories overlap (`Ribbon Plot` versus the combined Parallel/Alluvial/Sankey/Ribbon entry; `Double-X Plot` versus the 2026 Dual-X note).
- **App dependency is unresolved.** Chord, Sunburst, Network, Sankey and similar entries may require Apps rather than built-in templates; their `D-GATE` marks that uncertainty. No App license is assumed.
- **3D types need a backend decision.** Surface, Wireframe, Walls and Ribbon are marked `D-GATE` because the renderer and 3D contract are not implemented.
- **Map data rights are unresolved.** Maps cannot be tested with a basemap whose license is unknown.
- **No PLOT fixture exists.** `F-SCENE` is a plan for named scenes, not a present corpus.

## 6. Fitting function catalog: `ORG-FIT-004`

### 6.1 Evidence and claim conflict

S1:240 states that the vendor claims "nearly 200" built-in fitting functions organized by category. S5 states "Select from close to 200 built-in fitting functions arranged in categories." S7 states "The NLFit tool includes more than 170 built-in fitting functions, selected from a wide range of categories and disciplines" and "Origin provides over 170 built-in fitting functions."

The totals conflict: "close to 200" is not the same as "more than 170," and neither page publishes a complete function list, equation, parameter domain or category membership. S7 confirms that Surface Fitting, Compare Models and Implicit Fitting are OriginPro-only, but does not enumerate categories.

The only individual fitting function named in the retrieved pages is "2D Gaussian," in a product-page caption: "Results from fitting a 2D Gaussian function to four peaks, using the Surface Fitting tool in OriginPro" (S5). The same page also names fitting operations such as Sigmoidal Fit, Exponential Fit and Polynomial Fit (S5, S7); those are tools or families, not entries in a function catalog. The caption is a named example, not evidence of catalog membership.

### 6.2 Child-entry result

The evidence does not support a complete function enumeration. Assigning names such as Lorentzian, Voigt or Arrhenius, or inventing equations and parameter domains, would violate the no-invention rule. Therefore:

- Named child entries: 1 (`EG-CAT-FIT-002`, the 2D Gaussian caption example).
- Claim-only entry: `EG-CAT-FIT-001`, tabulated below.
- The parent remains `unresolved`.
- The count conflict is recorded as an explicit unknown, not averaged or resolved in favor of one page.

| Child ID | Named entry | Register family | Prerequisites | Variant or parameters to verify | Rights | Delivery | Fixture and expected output | Uncertainty |
|---|---|---|---|---|---|---|---|---|
| `EG-CAT-FIT-001` | Unenumerated built-in fitting function library (claim only) | `ORG-FIT-004` | Lawful versioned function catalog | Totals conflict: close to 200 versus more than 170; no names, categories, equations or domains published | `R-BLOCKED` | `D-GATE` | `F-NONE` -> `O-NONE` | `U3` |
| `EG-CAT-FIT-002` | 2D Gaussian surface function (caption example) | `ORG-FIT-004` | 2D Gaussian model; surface fitting path | Caption only; confirm selectable function name, parameterization, initialization and Pro scope | `R-OPEN` | `D-GATE` | `F-SYNTH` -> `O-TABLE` (four-peak surface, reference fit and tolerance) | `U2` |

`EG-CAT-FIT-002` does not establish that the selectable built-in function is named exactly "2D Gaussian," nor that it belongs to the counted catalog. It records a discovered named variant and gives later work a stable ID.

### 6.3 Closure evidence required

1. A lawful, versioned function catalog (manual, help index or Fitting Function Organizer export) for Origin/OriginPro 2026b.
2. Per-function equation, parameter names, domains, initialization and constraints.
3. Category membership and the Origin versus OriginPro split.
4. Reference datasets and fits with tolerances, plus failure cases.
5. A reconciliation of the "close to 200" and "more than 170" claims against the catalog count.

Until items 1-5 exist, no fitting-function parity claim is permitted.

## 7. Peak function catalog: `ORG-PEAK-005`

### 7.1 Evidence and claim conflict

S1:259 states "25+ built-in peak functions" and mentions fixed or percentage-limited centers, shared parameters and stepwise iteration. S10's acceptance for `ORG-PEAK-005` turns those into required tests. S5 states "Select from over 25 built-in peak functions, or create your own peak function." The same product page separately states that Surface Fitting provides "over 20 surface functions." S8 states, under Surface Peak Fitting PRO, "Fit peaks with over 20 built-in functions or a user defined function."

The "over 25" claim is about peak fitting generally; the "over 20" claim is about surface peak fitting. They may describe different scopes, but the pages do not publish the lists or state the relationship.

### 7.2 Child-entry result

No individual peak function is named in the retrieved evidence. S1 names baseline and detection methods (second derivative, XPS, asymmetric least squares; local maximum, window search, residual and derivative detection), but those are already registered behaviors under `ORG-PEAK-001` and `ORG-PEAK-002`, not peak function names. Therefore:

- Named child entries: 0.
- Claim-only entry: `EG-CAT-PEAK-001`, tabulated below.
- The parent remains `unresolved`.
- The 1D versus surface scope difference is recorded as an unknown.

| Child ID | Named entry | Register family | Prerequisites | Variant or parameters to verify | Rights | Delivery | Fixture and expected output | Uncertainty |
|---|---|---|---|---|---|---|---|---|
| `EG-CAT-PEAK-001` | Unenumerated built-in peak function library (claim only) | `ORG-PEAK-005` | Lawful versioned peak catalog | Over 25 peak functions versus over 20 surface functions; no names, equations, domains or shared-parameter behavior published | `R-BLOCKED` | `D-GATE` | `F-NONE` -> `O-NONE` | `U3` |

### 7.3 Closure evidence required

1. A lawful, versioned peak function catalog for OriginPro 2026b, separated into 1D and surface scopes.
2. Per-function equation, parameters, domains and shared/fixed-center behavior.
3. Synthetic mixtures that test overlapping peaks, fixed or percentage-limited centers, shared parameters and stepwise fitting.
4. Reference fits with tolerances and failure diagnostics.
5. A reconciliation of the 25+ and 20+ claims.

## 8. Graph export catalog: `ORG-EXPORT-001`

### 8.1 What the evidence establishes

S1:382 claims 17 formats including PDF, EPS, SVG, EMF, PNG, TIFF, JPEG and BMP. S3 asserts "Origin supports these 17 image export file formats" and then lists:

| Format | Extension | Class |
|---|---|---|
| Adobe Illustrator | `.AI` | Vector |
| Bitmap | `.BMP` | Raster |
| Computer Graphics Metafile | `.CGM` | Vector |
| AutoCAD Drawing Interchange | `.DXF` | Vector |
| Encapsulated PostScript | `.EPS` | Vector |
| Enhanced Metafile | `.EMF` | Vector |
| Graphics Image Format | `.GIF` | Raster |
| JPEG, Joint Photographic Experts Group | `.JPG` | Raster |
| Zsoft PC Paintbrush Bitmap | `.PCX` | Raster |
| Portable Network Graphics | `.PNG` | Raster |
| Truevision Targa | `.TGA` | Raster |
| Portable Document Format | `.PDF` | Vector |
| Adobe PhotoShop | `.PSD` | Raster |
| SVG, Scalable Vector Graphics | `.SVG` | Vector |
| TIFF, Tag Image File | `.TIF` | Raster |
| Windows Metafile | `.WMF` | Vector |

The table has 16 rows. The page says 17. S5 says "over 15 file formats" and names SVG, EPS, PDF, EMF, TIFF, PNG, JPEG and BMP. Three claims therefore disagree: over 15, 17, and 16 named. The seventeenth format is not identified by any retrieved source.

### 8.2 Child entries

The parent is `ORG-EXPORT-001`. All named-format rows are `D-ADAPTER` and require a scene, a selected encoder and an independent reader. The claim-only row `EG-CAT-EXPORT-017` is `D-GATE`. `R-OPEN` means the format specification is public; it does not clear the selected encoder library. Parameters common to raster formats include width, height, DPI, color space, background and alpha. Parameters common to vector formats include page size, font embedding, transparency and element clipping. EPS transparency must be flattened under a disclosed policy or rejected, per S9:62. Rights, variant scope and uncertainty are stated per row.

| Child ID | Named format | Extension | Class | Rights | Variant or parameters to verify | Fixture and expected output | Uncertainty |
|---|---|---|---|---|---|---|---|
| `EG-CAT-EXPORT-001` | Adobe Illustrator | `.AI` | Vector | `R-VENDOR` | Version compatibility, font and transparency handling | `F-SCENE` -> `O-VECTOR` | `U2` |
| `EG-CAT-EXPORT-002` | Bitmap | `.BMP` | Raster | `R-OPEN` | Bit depth, alpha policy, DPI metadata | `F-SCENE` -> `O-RASTER` | `U1` |
| `EG-CAT-EXPORT-003` | Computer Graphics Metafile | `.CGM` | Vector | `R-VENDOR` | Profile/version, font and clipping | `F-SCENE` -> `O-VECTOR` | `U2` |
| `EG-CAT-EXPORT-004` | AutoCAD Drawing Interchange | `.DXF` | Vector | `R-VENDOR` | Version, units, entity mapping | `F-SCENE` -> `O-VECTOR` | `U2` |
| `EG-CAT-EXPORT-005` | Encapsulated PostScript | `.EPS` | Vector | `R-OPEN` | Transparency flatten or reject policy, font embedding, bounding box | `F-SCENE` -> `O-VECTOR` | `U1` |
| `EG-CAT-EXPORT-006` | Enhanced Metafile | `.EMF` | Vector | `R-VENDOR` | Record coverage, font and clipping | `F-SCENE` -> `O-VECTOR` | `U2` |
| `EG-CAT-EXPORT-007` | Graphics Image Format | `.GIF` | Raster | `R-OPEN` | Palette quantization, transparency, animation policy | `F-SCENE` -> `O-RASTER` | `U1` |
| `EG-CAT-EXPORT-008` | JPEG | `.JPG` | Raster | `R-OPEN` | Quality, chroma subsampling, metadata, no alpha | `F-SCENE` -> `O-RASTER` | `U1` |
| `EG-CAT-EXPORT-009` | Zsoft PC Paintbrush Bitmap | `.PCX` | Raster | `R-VENDOR` | Color depth, RLE, DPI | `F-SCENE` -> `O-RASTER` | `U2` |
| `EG-CAT-EXPORT-010` | Portable Network Graphics | `.PNG` | Raster | `R-OPEN` | Alpha, bit depth, DPI metadata | `F-SCENE` -> `O-RASTER` | `U1` |
| `EG-CAT-EXPORT-011` | Truevision Targa | `.TGA` | Raster | `R-VENDOR` | Alpha, origin, RLE | `F-SCENE` -> `O-RASTER` | `U2` |
| `EG-CAT-EXPORT-012` | Portable Document Format | `.PDF` | Vector | `R-OPEN` | PDF version, fonts, transparency, page boxes | `F-SCENE` -> `O-VECTOR` | `U1` |
| `EG-CAT-EXPORT-013` | Adobe PhotoShop | `.PSD` | Raster | `R-VENDOR` | Layers or flattened output, color mode, bit depth | `F-SCENE` -> `O-RASTER` | `U2` |
| `EG-CAT-EXPORT-014` | SVG, Scalable Vector Graphics | `.SVG` | Vector | `R-OPEN` | SVG version, fonts, CSS policy, clipping | `F-SCENE` -> `O-VECTOR` | `U1` |
| `EG-CAT-EXPORT-015` | TIFF, Tag Image File | `.TIF` | Raster | `R-OPEN` | Compression, alpha, DPI, multi-page policy | `F-SCENE` -> `O-RASTER` | `U1` |
| `EG-CAT-EXPORT-016` | Windows Metafile | `.WMF` | Vector | `R-VENDOR` | Record coverage, font and clipping | `F-SCENE` -> `O-VECTOR` | `U2` |
| `EG-CAT-EXPORT-017` | Unaccounted seventeenth format | Unknown | Unknown | `R-BLOCKED` | Claim-only gap row; `D-GATE`; no named format | `F-NONE` -> `O-NONE` | `U3` |

`EG-CAT-EXPORT-017` is a claim tracker, not a format. It must be deleted or replaced when the discrepancy is resolved; it must never be counted as a named format.

### 8.3 Export gaps and closure conditions

- The seventeenth format must be identified from a versioned manual or corrected by the vendor. Until then, the 17-format claim is unresolved.
- Raster output requires an independent-reader check, not only an in-app success dialog.
- Vector output requires structural checks plus an independent reader; pixel-identical comparison is not the criterion.
- Alpha and transparency behavior differs by format and must be declared per format.
- No encoder library has been selected or licensed; `R-OPEN` is not a dependency approval.

## 9. Proprietary compatibility matrix: `EG-ADD-014`

### 9.1 Decision axes

S9:117-121 separates functional equivalence, data interchange, file round-trip, language-source compatibility and binary ABI compatibility. None implies another. The matrix below records the decision axes per artifact. No row has a support claim. `R-BLOCKED` means no lawful specification, SDK, corpus or sample is available; `R-VENDOR` means vendor documentation may be public but implementation, corpus and redistribution rights are unverified. Neither status is a permanent legal conclusion.

### 9.2 Child entries

The parent is `EG-ADD-014`. Every row uses `D-GATE`, `F-NONE` -> `O-NONE`, `U4` and `support_claim=none` unless a narrower evidence path is stated. `Rights` is stated per artifact; no status is a support claim.

| Child ID | Artifact or class | Evidence anchor | Decision axes | Prerequisites before any claim | Rights | Uncertainty |
|---|---|---|---|---|---|---|
| `EG-CAT-COMPAT-001` | DataGraph `.dgraph` package | S2:100 | Read, write, round-trip | Lawful specification, versioned package fixtures, hostile-input tests, loss report | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-002` | DataGraph `.dgraph` single file | S2:100 | Read, write, round-trip | Same as package form; confirm whether the two forms share a container spec | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-003` | Origin OPJ | S1:100; S9:119 | Read, write, round-trip | Lawful specification, versioned fixtures, protected-file policy | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-004` | Origin OPJU | S1:100; S9:119 | Read, write, round-trip | Same as OPJ; version matrix | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-005` | Protected OPJ/OPJU | S1:100; S9:119 | Read, round-trip, password policy | Explicit authorization and security policy; no password bypass | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-006` | Prism PZFX/XML project or data | S1:467; S4 | Read; possibly write | Version matrix, rights review, loss report | `R-VENDOR` | `U4` |
| `EG-CAT-COMPAT-007` | Legacy Plot | S2:116 (DataGraph manual row `DG-IO-007`); S9:119 | Read; identity confirmation | Determine whether this is an Origin, DataGraph or third-party Plot format | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-008` | LabTalk source compatibility | S1:412; S9:119 | Grammar, runtime, library and API source compatibility | Grammar and runtime specification, version matrix, test corpus | `R-VENDOR` | `U4` |
| `EG-CAT-COMPAT-009` | Origin C source compatibility | S1:413; S9:119 | Source compatibility, API coverage | Compiler/runtime and API documentation, version matrix | `R-VENDOR` | `U4` |
| `EG-CAT-COMPAT-010` | Vendor Apps | S1:428-433; S9:119 | Per-App dependencies and licenses | Per-App manifest, dependency and license review | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-011` | COM / Orglab | S1:419; S9:119 | Windows COM interface and automation | Interface documentation, Windows test host, license review | `R-VENDOR` | `U4` |
| `EG-CAT-COMPAT-012` | XOP-like native binaries | S9:119 | Binary ABI compatibility | Evidence that an SDK or public ABI exists; none is established by the inventories | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-013` | Licensed engines: MATLAB, Mathematica, LabVIEW, R/Rserve | S1:415-418; S9:119 | Invocation, data exchange, deployment licensing | Installed and authorized products, version matrix, license review | `R-BLOCKED` | `U4` |
| `EG-CAT-COMPAT-014` | DataGraph DTG vector export | S2:264; S9:109 | Read, write, round-trip | Lawful DTG specification and a rights-checked corpus; distinct from the `.dgraph` project container | `R-BLOCKED` | `U4` |

`EG-CAT-COMPAT-007` cites the preserved DataGraph manual row `DG-IO-007` at S2:116 and S9:119. The artifact name is ambiguous between an Origin, DataGraph or third-party legacy Plot format; the line anchor confirms only that the name is preserved, not which format owns it.

### 9.3 Compatibility non-goals and gates

- File compatibility is not promised by functional similarity.
- DTG vector export is a separate artifact from `.dgraph` package and single-file project containers; neither implies the other.
- A native script or CLI equivalent does not promise `dgraph` argument compatibility, per S9:119.
- Vendor Apps are not EveryGraph's extension marketplace, and no App is assumed redistributable.
- COM/Orglab is Windows-specific and requires an authorized host.
- XOP-like binaries are a research question, not an ABI promise; the inventories do not establish an XOP SDK.
- No compatibility fixture may be created from a file whose redistribution rights are unknown.

## 10. Catalog roll-up gate: `EG-ADD-015`

S9:44 and the register acceptance require every named family and discovered item to become testable, with unknown lists kept open. The roll-up gate is decomposed into seven reconciliation entries: six catalog gates and one cross-cutting gate for method/option and App lists. These are gate children, not feature children. The six catalog gates are necessary but not sufficient for `EG-ADD-015`; the cross-cutting gate records the remaining obligation.

| Child ID | Gate scope | Named floor | Claim to reconcile | Closure condition | Status |
|---|---|---:|---|---|---|
| `EG-CAT-GATE-001` | Import catalog | 34 file-format families; 2 source adapters | More than 30 formats | Every family has a version matrix, rights status and fixture; counting basis documented | Open |
| `EG-CAT-GATE-002` | Graph catalog | 73 named types | Over 100+ graph types | Every named type has a PLOT scene; the remaining claimed types are named or the claim is corrected | Open |
| `EG-CAT-GATE-003` | Fitting catalog | 1 named | Close to 200 and more than 170 | Lawful versioned catalog with equations, domains and reference fits | Open |
| `EG-CAT-GATE-004` | Peak catalog | 0 named | Over 25 and over 20 surface functions | Lawful versioned catalog with mixtures and shared/fixed-center tests | Open |
| `EG-CAT-GATE-005` | Export catalog | 16 named | Over 15, 17 and 16 named | The seventeenth format is identified or the claim is corrected; every format has a fixture | Open |
| `EG-CAT-GATE-006` | Compatibility matrix | 14 named artifacts | No count claim | Each artifact has a rights, version, corpus and decision record | Open |
| `EG-CAT-GATE-007` | Method/options and App lists | 0 named | Unnamed method/option and App catalogs | Reconcile the inventories' method/option lists and App catalogs; Apps remain per-App rights gates under `EG-CAT-COMPAT-010` | Open |

`EG-ADD-015` stays `unresolved` until all seven gate entries close. A count in this table is an evidence floor, not a parity claim.

## 11. Proposed child-ID schema and validator integration

This section is a proposal. P0-003 did not create the proposed CSV, modify `tools/validate_planning.py`, change the 428-row register or alter any status.

### 11.1 Namespace

Child IDs use the grammar `EG-CAT-<AREA>-<NNN>`:

- `AREA` is one of `IO`, `GRAPH`, `FIT`, `PEAK`, `EXPORT`, `COMPAT`, `GATE`.
- `NNN` is a zero-padded three-digit sequence starting at `001`.
- IDs are immutable and never reused, including after a row is retired.
- The four-segment form intentionally does not match the register's `(ORG|DG|EG)-[A-Z]+-\d{3}` pattern. This prevents accidental import into the requirement register.
- A child ID is not a requirement ID and does not appear in `feature-traceability.csv`.
- The parent is a separate `catalog_parent` field; it is not encoded in the ID. If a row is reparented, the ID does not change.

### 11.2 Proposed file and columns

Proposed path: `docs/planning/catalog-children.csv` (not created by P0-003).

| Column | Rule |
|---|---|
| `child_id` | Grammar above; unique |
| `catalog_parent` | One of the seven P0-003 requirement IDs; must exist in the register and remain `unresolved` |
| `register_family` | Existing register ID that owns the behavior. The catalog parent is allowed for a named row only when no narrower register row exists, and for every claim-only or gate row |
| `member_type` | `file-format`, `source-adapter`, `graph-type`, `fit-function`, `peak-function`, `export-format`, `compat-artifact`, `catalog-claim` or `gate` |
| `source_name` | Exact token as written in the anchored source; may be Chinese for S1/S2, where it is exempt from the English-only code check |
| `named_entry` | English display name, or an explicit claim-only label |
| `evidence_code` | Source code such as `S3` or `S4`; must resolve to the evidence register |
| `source_file` | Repository-relative local evidence file, required for local anchors |
| `source_line` | One-based line number, required with `source_file` |
| `source_url` | HTTPS source URL, required for external anchors |
| `source_checked` | Retrieval date in `YYYY-MM-DD` form |
| `primary_anchor` | `local` or `external`; required when both anchor kinds are present |
| `source_excerpt` | Exact quoted text from the anchored line or page; may be Chinese for S1/S2 |
| `source_excerpt_sha256` | SHA-256 of `source_excerpt` after Unicode NFC, trim and whitespace collapse |
| `variant_parameters` | Known variants and parameters, or the words `not enumerated`; no invented values |
| `prerequisites` | Required model, reader, backend, rights or environment |
| `rights_status` | `R-OPEN`, `R-VENDOR`, `R-BLOCKED` or `R-OWN` |
| `delivery_class` | `D-NATIVE`, `D-ADAPTER`, `D-GATE` or `D-PLATFORM` |
| `fixture_plan` | `F-*` code plus a short specific note |
| `expected_output` | `O-*` code plus a short specific note |
| `uncertainty` | `U1`-`U4` |
| `membership_status` | `named` means named in the evidence (not rights-cleared), `claim_only` means a vendor count without membership, `open` means a known gap, `blocked` means an artifact with no lawful access |
| `support_claim` | `none`, `planned` or `claimed`; `none` is required for `R-BLOCKED` and for every claim-only row |

CSV values containing commas, quotes or newlines must be quoted. One row per stable testable entry. A row may group aliases or extensions only when they share one behavior and one fixture; record every alias in `variant_parameters`. Do not pack unrelated names into one field. Every row proposed by this document would carry `support_claim=none`; no child row is a delivery promise.

### 11.3 Source-anchor rules

1. **Local inventory anchor:** `source_file` and `source_line` must point to the physical line that contains the named entry or the claim. `source_excerpt` must be copied exactly from that line. After Unicode NFC, trim and whitespace collapse, its SHA-256 must equal `source_excerpt_sha256`.
2. **External anchor:** `source_url` must be HTTPS, `source_checked` must be a real retrieval date, and `source_excerpt` must be copied exactly from the page. The same normalized hash rule applies, so the check is reproducible offline.
3. **One primary anchor:** a row with both local and external anchors must set `primary_anchor`. The secondary anchor is informative.
4. **Line drift:** a changed inventory line breaks the hash and fails validation. It must be re-reviewed, not silently rehashed.
5. **Page drift:** external pages are live; a failed retrieval or changed excerpt is an explicit unknown, not a pass.
6. **No anchor, no row:** a named entry without a lawful anchor is not admissible. A claim-only row is admissible only with the claim's anchor and `membership_status=claim_only`.
7. **Proposed mappings:** `register_family` mappings are checked for existence, not semantic correctness. Semantic review remains a human task.
8. **Translations:** `source_name` and `source_excerpt` may be Chinese for S1/S2. `named_entry` is the English display name; the anchor check verifies the source token, not the translation.

### 11.4 Count expectations

The validator should enforce declared floors and exact claim-only counts. Floors protect against silent deletion; they do not assert completeness.

| Constant | Expected value | Meaning |
|---|---:|---|
| `IO_FORMAT_FAMILIES_MIN` | 34 | Named file-format families in section 4 |
| `IO_SOURCE_ADAPTERS` | 2 | Database and connector adapters |
| `GRAPH_NAMED_MIN` | 73 | Named graph entries in section 5 |
| `GRAPH_CLAIM_ONLY` | 1 | `EG-CAT-GRAPH-074` |
| `FIT_NAMED_MIN` | 1 | One caption example; no function list available |
| `FIT_CLAIM_ONLY` | 1 | `EG-CAT-FIT-001` |
| `PEAK_NAMED_MIN` | 0 | No function names available |
| `PEAK_CLAIM_ONLY` | 1 | `EG-CAT-PEAK-001` |
| `EXPORT_NAMED` | 16 | Named export formats |
| `EXPORT_CLAIM_ONLY` | 1 | `EG-CAT-EXPORT-017` |
| `COMPAT_NAMED` | 14 | Named compatibility artifacts |
| `GATE_ROWS` | 7 | Roll-up gates |

The vendor counts (30+, 100+, close to 200, 25+, 17) are **not** pass thresholds. A validator that requires 200 fit functions would force fabricated rows; a validator that requires only one row would permit silent collapse. Floors plus explicit claim-only rows are the proposed middle ground.

### 11.5 Proposed validator tests

A future `tools/validate_catalog_children.py`, or an extension of the existing validator, should test:

1. **ID grammar and uniqueness.** Every `child_id` matches the grammar; no duplicates; no gaps are treated as errors because retired IDs are never reused.
2. **Parent integrity.** Every `catalog_parent` exists in the register, is one of the seven P0-003 parents, and remains `unresolved`. A parent marked `planned` must fail: it would imply a delivery claim.
3. **Register immutability.** The register still has 428 rows and its expected baseline hash; the child file does not appear as a source file in any register row. A separate authorized task must update the baseline if the register legitimately changes.
4. **Required fields and enums.** All columns are present; rights, delivery, fixture, output, uncertainty, membership and support values are in their allowed sets; `variant_parameters` is non-empty and contains `not enumerated` rather than fabricated detail where evidence is absent.
5. **Evidence anchors.** Local file/line exists and `source_excerpt` is a substring of that line; the normalized excerpt hash matches. External URL is HTTPS, `source_checked` parses as a date, `source_excerpt` is stored in the CSV, and its normalized hash matches.
6. **Count floors.** Per-catalog named counts meet the constants above; claim-only counts equal the declared values.
7. **No invented names.** Every `source_name` appears in the anchored source text, unless `membership_status=claim_only` or `open`. `named_entry` is an English display name and may be a translation; the check verifies the source token and anchor, not the translation. This is a lexical check, not semantic proof.
8. **No duplicate named entries.** Within a catalog, names are unique after case and whitespace normalization; aliases must be recorded in `variant_parameters`.
9. **No silent rights elevation.** A row with `R-BLOCKED` must have `support_claim=none` and an `O-*` output of `O-NONE`; it may still be `membership_status=named` because naming and rights clearance are different facts.
10. **Document policy.** The evidence document passes the existing English-only, balanced-fence and local-link checks. The CSV receives a separate check that applies the English policy to every field except `source_name` and `source_excerpt`, which may carry source-language text. External links are reported but not fetched in CI by default.
11. **Quantitative consistency.** The count table in this document matches the CSV counts and the proposed `tools/catalog_expectations.json` constants. Until that file exists, the constants would live in the proposed validator; the current validator is unchanged and contains none. Drift fails the test.
12. **No claim inflation.** The string `100+` or `200` in prose must not be used as an achieved count without the corresponding named rows and an explicit claim status.
13. **Claim-only discipline.** Every `claim_only` row uses `D-GATE`, `support_claim=none` and non-`named` membership; it cannot satisfy a named floor.

The validator should not fetch external URLs in the default test run; network availability is not a correctness signal. A separate scheduled check can report page drift without blocking local work.

### 11.6 Migration and non-goals

1. P0-003 produces this proposal only. It does not create the CSV or change the validator.
2. A separately authorized integration task must approve the schema, create the CSV from reviewed rows and extend the validator.
3. The integration task must run the existing validator plus the new checks and leave the 428-row register unchanged.
4. Moving child rows into the register is a separate scope change. It would alter the 428-row count, the status contract and the validator's source-anchor rules, and it requires an explicit planning decision.
5. No child row implies implementation, a delivery date or a support promise.
6. No vendor count becomes an acceptance threshold.

## 12. Explicit unknown register

| ID | Unknown | Why it remains open | Next evidence |
|---|---|---|---|
| `U-01` | Complete fitting function membership | No function list in S5/S7; totals conflict; one caption example only | Lawful 2026b function catalog |
| `U-02` | Fitting categories and Origin versus OriginPro split | S7 names only three Pro-only capabilities | Versioned comparison and function organizer |
| `U-03` | Complete peak function membership; 1D versus surface scope | S5/S8 do not list functions or reconcile 25+ with 20+ | Lawful peak catalog |
| `U-04` | At least 27 named entries below the 100+ claim; true type gap unknown | Gallery is rotating and not exhaustive | Plot-menu inventory or complete gallery export |
| `U-05` | Which graph types depend on Apps | Gallery cards do not state dependency | App manifests and per-type help |
| `U-06` | The seventeenth export format | S3 asserts 17 but lists 16; S5 says over 15 | Versioned export help or vendor correction |
| `U-07` | Export claim basis | Three different counts | Versioned documentation |
| `U-08` | Import counting basis | Families, extensions, adapters and the generic `Binary` label are mixed | Vendor definition or manual index |
| `U-09` | pCLAMP `AB?` | Literal page artifact | Vendor manual |
| `U-10` | CDF identity | NetCDF and CDF are listed separately | Format specification |
| `U-11` | Image import format set | S4 says only "Image" | Image import help |
| `U-12` | GeoTIFF version and metadata scope | S1 only names the family | Geospatial import help |
| `U-13` | MATLAB MAT versions | S1 names the family; S2 requires v4 for DataGraph, not Origin | Origin import help |
| `U-14` | Proprietary import specifications and rights | No manuals or SDKs acquired | Rights review and lawful samples |
| `U-15` | `.dgraph` package and single-file specification | S2 names both forms, not their container format | Lawful specification |
| `U-16` | OPJ/OPJU and protected-file specification | S1 names the format and password protection | Lawful specification and security policy |
| `U-17` | Legacy Plot identity | The name appears in S2 and S9 without a definitive format owner | Source disambiguation |
| `U-18` | LabTalk grammar, runtime, library and API rights | S1 names the language only | Lawful language documentation |
| `U-19` | Origin C compiler, runtime and API rights | S1 names the language only | Lawful compiler/API documentation |
| `U-20` | XOP-like ABI existence | S9 raises the question; no SDK is established | Vendor SDK evidence |
| `U-21` | COM/Orglab interface and license | S1 marks it Add-on/Unconfirmed | Windows interface documentation |
| `U-22` | Vendor App licenses and dependencies | Apps may be third-party | Per-App manifests |
| `U-23` | Licensed engine deployment terms | MATLAB, Mathematica, LabVIEW and R/Rserve are separate products | License review and installed hosts |
| `U-24` | DataGraph 5.5 behavior beyond the product page | The 5.5 release notes returned 403; the manual is 5.4 | Lawful 5.5 release notes |
| `U-25` | Current-version identity of live help pages | S3/S4 have no immutable version banner | Versioned manual edition |
| `U-26` | Runtime behavior of every child | No vendor application was installed | Authorized runtime tests, if ever obtained |
| `U-27` | DTG vector export specification | S2 names DTG as a DataGraph-specific vector export; no specification or corpus | Lawful DTG specification and sample |

## 13. What this document does not change

- `origin-feature-inventory.md` and `datagraph-feature-inventory.md` remain byte-identical to their preserved hashes.
- `docs/planning/feature-traceability.csv` remains at 428 rows. No child ID, status, delivery class or acceptance text was added or changed.
- `docs/planning/functional-scope.md` is unchanged.
- `tools/validate_planning.py` is unchanged.
- No product code, build configuration, dependency or platform claim was added.
- No vendor application, proprietary manual, SDK or file corpus was acquired; S3-S8 are public web pages retrieved on 2026-10-30.
- No Git commit, push, publication or external sharing occurred.
- The seven parent requirements remain `unresolved`. The catalog-expansion gate remains open.

## 14. Sources

- S1: [Origin inventory](../../origin-feature-inventory.md), Origin/OriginPro 2026b, checked 2026-10-30. Preserved input with a recorded SHA-256 hash.
- S2: [DataGraph inventory](../../datagraph-feature-inventory.md), DataGraph 5.5 build 68 with a 5.4 manual, checked 2026-10-30. Preserved input with a recorded SHA-256 hash.
- S3: [Origin help, "Exporting Your Graph or Layout Page to an Image File"](https://docs.originlab.com/origin-help/expgraph-to-image), retrieved 2026-10-30.
- S4: [Origin user guide, "Import Data"](https://docs.originlab.com/user-guide/import-data/), retrieved 2026-10-30.
- S5: [Origin product page](https://www.originlab.com/Origin), retrieved 2026-10-30.
- S6: [Origin Graph Gallery](https://www.originlab.com/GraphGallery), retrieved 2026-10-30.
- S7: [Origin curve fitting page](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/CurveFitting), retrieved 2026-10-30.
- S8: [Origin peak analysis page](https://www.originlab.com/index.aspx?go=Products/Origin/DataAnalysis/PeakAnalysis), retrieved 2026-10-30.
- S9: [EveryGraph functional scope](../planning/functional-scope.md), checked 2026-10-30.
- S10: [EveryGraph feature traceability](../planning/feature-traceability.csv), checked 2026-10-30.
- S11: [EveryGraph task board](../planning/task-board.csv), checked 2026-10-30; task lifecycle evidence only.

## 15. P0-005 follow-up (2026-10-30 UTC, 2026-10-31 local)

This section is appended after P0-003 acceptance. It does not rewrite the original findings; it records later evidence that supersedes specific claims. The full source register, rights review, revised control entries and unknown-register delta are in [catalog evidence gaps 2026b](catalog-evidence-gaps-2026b.md).

| P0-003 finding | P0-005 evidence | Disposition |
|---|---|---|
| Section 3 and section 5: 73 named graph entries are at least 27 below a 100-type floor | The 73 rows are exactly the 68 Graph Gallery specific filters plus five examples. The product page enumerates 111 graph labels, including duplicate pairs and two explicit App tools. The 100-plus claim is a built-in-plus-extended and Plot-menu aggregate, not a distinct-built-in-type count. | The 27-entry-gap comparison is withdrawn. The named rows remain valid evidence for their sources. The graph claim and exact membership remain open. |
| Section 6: no public fitting function list exists | The 32.1 appendix enumerates 239 numbered function pages in 22 categories; per-function pages carry equations and parameters. Vendor counts are 170-plus, close to 200, 200-plus and 239. | The membership gap is closed at the numbered-page level. The count basis, ten uncategorized pages, edition split and 2026b pin remain open. |
| Section 7: no individual peak function is named | The 32.4 table lists 28 1D Peak Analyzer functions, seven Pro-only and 21 shared. The 32.3 table lists 22 surface functions. Over 25 is 1D; over 20 is surface. | The membership and 25-versus-20 scope gaps are closed. The surface 21-versus-22 count, surface-peak subset and 2026b pin remain open. |
| Section 8: the seventeenth export format is unaccounted | No seventeenth format is documented. The export table has 16 rows; the current guide lists 15; the X-Function text says 14 but lists 16 option values. | The 17th-format gap is closed as a documentation error. The claim is corrected to 16 documented and 15 listed. The register is unchanged by P0-005; runtime and encoder rights remain open. |
| Section 12: U-01 to U-07 | See the delta table in the appendix, section 11. | U-01, U-02, U-03, U-04, U-05 and U-07 are partially updated; U-06 is closed as a documentation error; U-25, U-26 and new U-28 to U-33 remain open. |

The 428-row register, validator, functional scope and inventories are unchanged. The seven catalog parents and `EG-ADD-015` remain `unresolved`; the roll-up gate remains open. No vendor count is an acceptance threshold.
