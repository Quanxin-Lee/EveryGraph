# Compatibility rights and specification evidence register

Register version: `2026-10-31.2`. Evidence snapshot retrieved 2026-10-31. Owner: EveryGraph Research. Requirement: `EG-ADD-014`; roll-up gate: `EG-CAT-GATE-006`. This is an engineering evidence register, not legal advice, not a support statement and not a product-scope decision.

## 1. Purpose, authority and non-claims

### 1.1 What this register is

This register separates four compatibility questions for every named native or foreign artifact: functional equivalence, data interchange, formula or expression compatibility, and binary ABI or protocol compatibility. For each artifact it records the source and version, the rights status, the availability of a specification, a lawful fixture plan, the remaining uncertainty, an accountable role and the next evidence action.

The register operationalizes [functional scope section 6](../planning/functional-scope.md), `EG-ADD-014`, the compatibility gate `EG-CAT-GATE-006`, and the catalog children `EG-CAT-COMPAT-001` through `EG-CAT-COMPAT-014` in [catalog-children.csv](../planning/catalog-children.csv). It reads, but does not modify, the [P0-003 evidence decomposition](../tasks/p0-003-catalog-decomposition.md), the [P0-005 catalog evidence appendix](catalog-evidence-gaps-2026b.md), the preserved [Origin inventory](../../origin-feature-inventory.md) and [DataGraph inventory](../../datagraph-feature-inventory.md).

### 1.2 Coverage

| Named group in the acceptance scope | Register records |
|---|---|
| OPJ, OPJU and protected projects | CR-01, CR-02, CR-03 |
| DataGraph `.dgraph` package and single file | CR-04, CR-05 |
| DataGraph DTG | CR-06 |
| Legacy Plot | CR-07 |
| Prism | CR-08 |
| LabTalk | CR-09 |
| Origin C | CR-10 |
| XOP-like native ABI | CR-11 |
| Vendor Apps | CR-12 |
| COM and Orglab | CR-13 |
| MATLAB | CR-14, CR-15 |
| Mathematica | CR-16, CR-17 |
| LabVIEW | CR-18 |
| R and Rserve | CR-19, CR-20 |
| Documented proprietary connectors | CR-21 through CR-26 |
| Excel formula interchange (formula-axis addition) | CR-27 |
| Native EveryGraph project and CLI | Section 4 |

### 1.3 Non-claims

This register makes none of the following claims:

- that any foreign format, language, SDK, engine or ABI is supported, planned for a named release or achievable at a stated cost;
- that any file can be read, written or round-tripped, or that a round-trip preserves data, formulas, styles or identity;
- that functional similarity implies file compatibility, or that a public document grants an implementation right;
- that an open-source reader or writer is lawful, complete or compatible with the EveryGraph license;
- that a vendor count, a marketing label or a user-documentation page constitutes a specification;
- that this register substitutes for external legal review or a product-scope decision.

Every gate in this register is unresolved. A gate may close only through the evidence defined in section 3 and an explicit acceptance update by the owning task.

## 2. Register method and status vocabulary

### 2.1 Version identity and change control

The register version is a date plus a revision number (`YYYY-MM-DD.N`). It identifies a dated evidence snapshot, not a live mirror of vendor documentation. The sources were retrieved on 2026-10-31; live Help sites are unversioned, so a later check must record a new date, new excerpts and new hashes. A change to a rights status, a version scope, a fixture plan or an axis status requires a new register version and a note in the task record. The register never changes the read-only catalog, the 428-row traceability register or another task's record.

### 2.2 Evidence labels

| Label | Meaning |
|---|---|
| `S` | Source fact observed on a dated page or in a preserved repository file, with a short excerpt or anchor |
| `I` | Engineering inference drawn from the cited evidence; not a vendor statement |
| `U` | Unknown or not established by the available evidence |
| `N` | Native project statement owned by EveryGraph, not a vendor or legal fact |

Every family record separates these labels. A statement without a label is a source fact. Quotations are short, verbatim and cited in section 14.

### 2.3 Rights status

| Status | Meaning |
|---|---|
| `R-OPEN` | A public specification or public license appears to permit an independent implementation; legal review and fixture rights are still required |
| `R-VENDOR` | Public documentation exists, but implementation, corpus or redistribution rights are unverified |
| `R-ADVERSE` | Public documentation exists, but vendor terms expressly restrict reverse engineering, format parsing, competing use, bundling or redistribution; no implementation right is established |
| `R-BLOCKED` | No lawful specification, SDK, corpus or sample is available, or the artifact's identity is unresolved; a support claim is prohibited |
| `R-OWN` | EveryGraph-owned artifact, governed by the project's own schema and license policy |

`R-ADVERSE` is a register-only refinement used to keep an explicit restriction visible; it does not change any catalog rights value. No status here is a legal conclusion. `R-BLOCKED` and `R-ADVERSE` both prohibit a support claim.

### 2.4 Specification availability

| Status | Meaning |
|---|---|
| `SPEC-NONE` | No public specification; only marketing, a reverse-engineered implementation or an undocumented binary was found |
| `SPEC-DOC` | User documentation describes behavior, but no normative grammar, container schema, ABI or conformance clause was found |
| `SPEC-PARTIAL` | A public normative or semi-normative document covers part of the artifact, an older version or one interface layer |
| `SPEC-PUBLIC` | A public normative standard covers the artifact; implementation rights, version binding and completeness still require review |
| `SPEC-INTERNAL` | EveryGraph-owned internal schema, not a public interoperability contract |

### 2.5 Lawful fixture plan

| Status | Meaning |
|---|---|
| `FIX-NONE` | No lawful fixture is available; the gate cannot be tested |
| `FIX-SYNTH` | Project-authored synthetic files or scripts, created from public semantics and containing no vendor material |
| `FIX-LICENSED-LOCAL` | Files generated locally by a lawfully licensed vendor installation using synthetic data; never redistributed without separate permission |
| `FIX-PUBLIC` | A public corpus whose license permits the intended use; the license and attribution must be verified before use |
| `FIX-USER` | A user-supplied file used only for local observation; never committed, published or retained in the repository |

A fixture plan is not a fixture and does not establish that a fixture is lawful. Fixture creation follows section 8.

### 2.6 Compatibility axes and status vocabulary

| Axis | Question |
|---|---|
| Functional | Can EveryGraph perform the same user-visible operation or provide the same capability, independently of file or language interchange? |
| Data | Can textual or binary data values, structure, metadata, units and identity be read, written or round-tripped with a disclosed loss report? |
| Formula | Can stored or source-level formulas, expressions, scripts and computational semantics be interpreted or produced with equivalent behavior? |
| ABI | Can a binary interface, protocol or runtime contract be implemented or invoked across versions and platforms? |

| Axis status | Meaning |
|---|---|
| `Open` | The gate is unresolved; no compatibility claim is made |
| `Open-partial` | Partial public evidence exists, but a material version, coverage or rights gap keeps the gate unresolved |
| `N/A (U)` | No surface for this axis was identified; this is an engineering finding, not a legal conclusion, and may be revisited |

`Open` and `Open-partial` are both unresolved. They never authorize a support claim, a read/write decision or a round-trip promise.

### 2.7 Uncertainty levels

| Level | Meaning |
|---|---|
| `U1` | Identity, specification and rights are sufficient for a scoped plan; only product decisions remain |
| `U2` | Public evidence exists, but a material version, coverage or rights gap remains |
| `U3` | Identity, scope or rights are only partially evidenced; significant unknowns remain |
| `U4` | No lawful specification, SDK or corpus is available, the identity is unresolved, or an explicit restriction applies; a compatibility claim is prohibited |

## 3. Compatibility gate rules

1. The four axes are independent. Functional similarity does not imply data compatibility; data import does not imply round-trip; reading a file does not imply writing it; source compatibility does not imply binary ABI compatibility; a public document does not imply an implementation right.
2. A gate closes only when all of the following exist: a versioned specification or written vendor statement, a reviewed rights position, a lawful fixture corpus, a conformance and loss-report plan, and an independent validation result. Any missing item keeps the gate unresolved.
3. A read, write or round-trip decision requires a separate product decision. No axis status in this register constitutes that decision.
4. Protected or encrypted artifacts require explicit authorization from the artifact owner and a security policy. No password bypass, decryption workaround or protection removal is permitted.
5. A fixture may not be created from a file whose provenance or redistribution rights are unknown. Vendor manuals, SDKs and sample files are not acquired or redistributed by this task.
6. Vendor counts, marketing labels and gallery names are not compatibility evidence. They may define a research target but never close a gate.
7. External legal review is a prerequisite for closing any `R-ADVERSE`, `R-VENDOR` or GPL-adjacent gate, and for every fixture redistribution decision.
8. A native EveryGraph round-trip or schema test is evidence only for EveryGraph's own format. It never transfers to a foreign artifact.

## 4. Native reference: EveryGraph project and CLI

The native reference is recorded so that foreign gates are not confused with EveryGraph's own persistence contract. It is not a foreign compatibility claim.

- Source and version: [project file format](../architecture/project-file-format.md), chosen architecture dated 2026-10-30; `ADR-0004`; schema not frozen for public interoperability. `EG-ADD-013` covers native migration and protection.
- Container: SQLite `.egp` with versioned metadata and immutable Arrow IPC chunk BLOBs; JSON for bounded node parameters and styles; typed tile payloads for grids and images. `S`/`N`.
- Rights `R-OWN`; specification `SPEC-INTERNAL`; fixture plan `FIX-SYNTH` (project-generated, versioned migration fixtures); uncertainty `U2`.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `N/A (U)` | Native application behavior is defined by EveryGraph, not by a foreign compatibility target. No foreign claim follows. |
| Data | `Open` | Schema versioning, migration and recovery are required by `EG-ADD-013` and the P2 storage work. Native round-trip is not yet tested; it remains a native gate, not a foreign one. |
| Formula | `Open` | The native expression/formula language is not yet specified. No formula-compatibility claim is made for any foreign language. |
| ABI | `N/A (U)` | No public native ABI is planned; the plugin protocol is a separate future contract. |

- Owner: Data/storage lead.
- Next evidence action: define versioned native schema and migration fixtures under the P2 storage task; keep native round-trip evidence separate from every foreign gate.
- Native CLI caveat: `N` [functional scope](../planning/functional-scope.md) line 119 states that native script/CLI equivalents do not promise DataGraph `dgraph` argument compatibility. This register preserves that non-promise.

## 5. Foreign and compatibility family register

Every family below is unresolved on every axis except where a genuine no-surface finding is recorded as `N/A (U)`. No family has a support claim. No read, write or round-trip decision is made. Each record states source and version, rights, specification, fixture plan, uncertainty, owner and next evidence action.

### 5.1 Origin project files

#### CR-01 Origin project file (OPJ)

- Catalog and requirement: `EG-CAT-COMPAT-003`; `ORG-UI-010`; `EG-ADD-014`.
- Source and version: Origin/OriginPro 2026b release line (2026-06 per the preserved inventory). The official file-types page retrieved 2026-10-31 states: "Origin 2023 and later versions will still read the older, non-Unicode *.opj file type but you can no longer save as *.opj." The inventory preserves the OPJ/OPJU save and password requirement at `S1:100`; the compatibility decision axes are at `S9:119`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | EveryGraph must represent project windows, folders, worksheets and graphs, but no mapping study shows that its object model can represent OPJ semantics. Closure: versioned object-model mapping and a conformance corpus. |
| Data | `Open` | No public OPJ specification. liborigin is a reverse-engineered, read-only GPL-3.0 reader that describes the formats as "quite different" and covers approximately version 3.5 to 2017, with its last push in 2021. No independent writer, version matrix or loss report exists. Closure: lawful specification or written vendor statement, version matrix, read-only/write decision and loss report. |
| Formula | `Open` | OPJ can carry column formulas, analyses and scripts, but no public object schema or formula encoding is documented. Closure: object-model and formula-semantics evidence. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. Reading OPJ would not establish any ABI compatibility. |

- Vendor terms: the Origin EULA page retrieved 2026-10-31 states "You will not reverse engineer or decompile the Software" and asserts ownership of the software and documentation. No interoperability, format-research or sample-redistribution clause was found in the inspected text. `S`.
- Inference: liborigin's GPL-3.0 license is incompatible with incorporating it into an Apache-2.0 distributed work; its reverse-engineered provenance is not a lawful specification. `I`.
- Owner and next evidence action: Integration lead with external legal review. Request a written interoperability and fixture-redistribution position from OriginLab, then decide whether OPJ is a read-only legacy import or an explicit non-goal; do not claim write or round-trip.

#### CR-02 Origin project file (OPJU)

- Catalog and requirement: `EG-CAT-COMPAT-004`; `ORG-UI-010`; `EG-ADD-014`.
- Source and version: Origin/OriginPro 2026b. OPJU is the Unicode project format introduced for Origin 2018; the file-types page states that Origin 2023 and later can no longer save as OPJ. The FAQ page on converting OPJU to the older OPJ states that converted files "cannot be opened in earlier versions of Origin". `S`, retrieved 2026-10-31.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Same project-object coverage as CR-01, with no mapping study. |
| Data | `Open` | No public specification; liborigin states that OPJ and OPJU "are quite different"; no writer, version matrix or loss report. The version matrix must cover the 2018 format introduction and the 2023 save transition. |
| Formula | `Open` | Embedded formulas, analyses and scripts remain opaque without an object schema. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. |

- Vendor terms: same Origin EULA restriction as CR-01. `S`.
- Owner and next evidence action: Integration lead with external legal review. Include OPJU in the same written request as CR-01; require a version matrix before any read/write decision.

#### CR-03 Protected OPJ/OPJU

- Catalog and requirement: `EG-CAT-COMPAT-005`; `ORG-UI-010`; `EG-ADD-014`.
- Source and version: Origin password-protection Help page retrieved 2026-10-31; the preserved inventory records password-protected projects at `S1:100`. The Help page describes protection modes but publishes no encryption algorithm; it notes that an administrator mode "does not prevent someone from opening the project file". `S`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Opening and saving protected projects is in the preserved requirement, but no security policy or protection model is defined. Closure: a security policy, explicit owner authorization and a protected-file open/save decision. |
| Data | `Open` | No published protection algorithm; liborigin contains no password/decryption path. `I`: protected projects are not readable by the known open-source reader; this is untested. |
| Formula | `Open` | Protected content may include formulas and scripts; no semantics are available. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. |

- Security rule: no password bypass, no decryption workaround and no protection removal. A protected fixture may be created only by its owner with explicit authorization, and its password is never logged or committed.
- Owner and next evidence action: Integration lead plus security/platform lead and external legal review. Define a protected-file policy before any implementation; obtain an owner-authorized fixture only if the policy permits it.

### 5.2 DataGraph project containers

#### CR-04 DataGraph `.dgraph` package

- Catalog and requirement: `EG-CAT-COMPAT-001`; `DG-UI-011`; `EG-ADD-014`.
- Source and version: DataGraph 5.4 manual (September 2024) and the vendor community page "Saving Files" (published 2024-10-21), retrieved 2026-10-31. The manual states: "Files with the extension *.dgraph use an Apple standard called a package file"; it also states that files "can be programmatically created by assembling folders". DataGraph 5.5 was released 2025-07-11; the release page returned HTTP 403 during research and only a search snippet was retrievable. `S`/`U`.
- Rights `R-BLOCKED`; specification `SPEC-DOC`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | The preserved requirement covers DataGraph project save and open, but no mapping study exists. |
| Data | `Open-partial` | The package is documented at user level as an Apple package of folders, but member names, manifest, versioning and data representation are unpublished. No reader or writer was found. Closure: member schema, version matrix and loss report. |
| Formula | `Open` | DataGraph commands and analyses are stored in the project, but no schema or semantics are published. |
| ABI | `N/A (U)` | Package container; no binary ABI surface identified. |

- Rights: no public DataGraph EULA or terms were located in the inspected pages; no permission for implementation, fixture inspection or redistribution is stated. `U`.
- Owner and next evidence action: Integration lead. Request container/member documentation and written license terms from Visual Data Tools; keep the project container at `R-BLOCKED` until both exist.

#### CR-05 DataGraph `.dgraphs` single file

- Catalog and requirement: `EG-CAT-COMPAT-002`; `DG-UI-011`; `EG-ADD-014`.
- Source and version: DataGraph 5.4 manual and the "Saving Files" community page, retrieved 2026-10-31. The documentation states: "Starting with DataGraph version 5.2, you can save in a new single file format. Files saved in this format will have the extension *.dgraphs." The preserved inventory describes two `.dgraph` save forms; the documented single-file extension is `.dgraphs`, not `.dgraph`. This is a source discrepancy for the catalog owner, recorded in section 11. `S`.
- Rights `R-BLOCKED`; specification `SPEC-DOC`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | The same project capability as CR-04, with a different container form. |
| Data | `Open` | The single-file container is documented only as an option; no internal schema, versioning or reader exists. Whether it shares a member schema with the package is unknown. |
| Formula | `Open` | Same embedded command and analysis question as CR-04. |
| ABI | `N/A (U)` | Container; no binary ABI surface identified. |

- Owner and next evidence action: Integration lead. Extend the CR-04 request to confirm the `.dgraphs` extension, the container relationship and the license; report the catalog naming discrepancy to the P0-004 owner.

#### CR-06 DataGraph DTG

- Catalog and requirement: `EG-CAT-COMPAT-014`; `DG-EXPORT-005`; `EG-ADD-014`.
- Source and version: DataGraph 5.4 manual, retrieved 2026-10-31: "DTG is a graphic file that is unique to DataGraph and DataTank"; "This format keeps all of the information". The manual presents it alongside PDF, EPS and SVG but publishes no format specification. `S`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | EveryGraph plans publication vector output (PDF/EPS/SVG); DTG itself is not a required output until a product decision. No functional parity is shown. |
| Data | `Open` | No public specification, extension, versioning or independent reader/writer. A licensed export is a black-box oracle only and cannot establish an independent decoder. |
| Formula | `N/A (U)` | Graphic file; no formula surface identified. |
| ABI | `N/A (U)` | Graphic file; no binary ABI surface identified. |

- Owner and next evidence action: Integration lead. Request a DTG specification and license from Visual Data Tools; if none is available, keep DTG unresolved and make no export-parity claim.

#### CR-07 Legacy Plot

- Catalog and requirement: `EG-CAT-COMPAT-007`; `DG-IO-007`; `EG-ADD-014`.
- Source and version: DataGraph 5.4 manual, retrieved 2026-10-31, identifies "Plot" as "an earlier version of Plot, another plotting program on macOS" and states that DataGraph "has not been tested with files from the latest version ... now called Plot2". The preserved inventory at `S2:116` names the legacy Plot import. The identity is a third-party macOS application, not Origin or DataGraph. `S`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-USER`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | A migration import may be considered, but no product decision or priority exists. |
| Data | `Open` | The extension and internal format are not established; no public reader was found. Closure: confirm the format owner, extension and version, then a lawful fixture. |
| Formula | `N/A (U)` | No formula surface was identified in the available evidence. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. |

- Owner and next evidence action: Integration lead. Confirm the extension and terms with the Plot2 vendor; if identity cannot be confirmed, keep the artifact unresolved rather than guessing a decoder.

### 5.3 GraphPad Prism

#### CR-08 Prism PZFX and current Prism files

- Catalog and requirement: `EG-CAT-COMPAT-006`; `ORG-IO-001` (Prism import); the 2026b GraphPad Prism Connector; `EG-ADD-014`.
- Source and version: GraphPad Prism 11 user guide (retrieved 2026-10-31) describes PZF as a binary format and PZFX as a format openable only by Prism 5 or later, and states that "the new PRISM file format is different from any other format that Prism had previously offered in that it is an open file format. All of your data, analysis parameters, results, graphs, and layouts can be accessed and viewed even without using Prism." A separate [compatibility page](https://www.graphpad.com/guides/prism/latest/user-guide/compatibility_between_prism_versions.htm) states "Files created by Prism 11 cannot be opened by Prism 9 or earlier." The GraphPad FAQ publishes PZFX schemas and style sheets described as covering Prism versions 5 through 8; no schema or normative documentation for the current `.prism` container was located. The Origin Prism Connector v1.35 (minimum Origin 2026b) advertises import of `.xml`, `.pzfx`, `.prism` and `.prismt`; no Prism version scope is stated. `S`/`U`.
- Rights `R-ADVERSE`; specification `SPEC-PARTIAL`; fixture `FIX-SYNTH` plus `FIX-LICENSED-LOCAL`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Prism import is a preserved requirement, but no mapping study or supported-feature matrix exists. |
| Data | `Open-partial` | Public PZFX schemas cover Prism versions 5 through 8. GraphPad describes the current `.prism` container as an open file format whose data, analysis, results, graphs and layouts can be accessed without Prism, but no published schema or normative container documentation was located. Only a read path is documented by the connector; write and round-trip are unverified. Closure: current `.prism` schema or documentation, version matrix and loss report. |
| Formula | `Open` | Prism analyses and results are not described by the published PZFX schemas; formula or result semantics remain unknown. |
| ABI | `N/A (U)` | PZFX is XML and `.prism` is described as an open file format; the container's internal structure is not established by a published schema. No binary ABI surface identified. |

- Vendor terms: the 2025-06-10 GraphPad Prism Terms, as reported by the source review, state that a user may not permit a third party to parse proprietary elements of the product or proprietary file types such as `.pzfx` or `.prism` for a commercial purpose, and separately prohibit reverse engineering. The executor's spot check of the PDF currently served at the GraphPad terms URL saw an effective date of 2026-08-11 and did not re-confirm the exact parsing clause; GraphPad states that purchases after 2026-08-12 route to Dotmatics terms, which reportedly add a conditional interface-information interoperability proviso. Whether a PZFX-specific prohibition persists is `U`. This is the clearest adverse rights signal in the register. `S`/`U`.
- Owner and next evidence action: Integration lead with external legal review. Obtain a written GraphPad/Dotmatics position on independent PZFX parsing and on schema redistribution before any parser work; verify the license of the published schema archive. Keep `.prism` out of scope until a current schema exists.

### 5.4 Languages and SDKs

#### CR-09 LabTalk

- Catalog and requirement: `EG-CAT-COMPAT-008`; `ORG-AUTO-001`; `EG-ADD-014`.
- Source and version: Origin LabTalk documentation, retrieved 2026-10-31: "LabTalk is Origin's native scripting language"; the Language Reference provides a "comprehensive list and descriptions of commands, functions, macros, objects, and system variables". No formal grammar, conformance suite or runtime specification was found. The `originlab/labtalk` GitHub repository is documentation-only and has no declared license. `S`.
- Rights `R-ADVERSE`; specification `SPEC-DOC`; fixture `FIX-SYNTH` plus `FIX-USER`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open-partial` | Scripting and automation are required by the inventory, but no interpreter or subset exists. Public documentation describes commands and objects, not a complete language. |
| Data | `Open` | The dataset/object model used by scripts is not formally specified; exchanging data with a compatible runtime is unproven. |
| Formula | `Open-partial` | Commands, functions and objects are publicly documented, but runtime behavior is described by examples; there is no grammar or conformance corpus. Closure: a clean-room subset definition, a conformance corpus and a documented semantic boundary. |
| ABI | `N/A (U)` | Source-level scripting language; no binary ABI surface identified. |

- Vendor terms: the Origin EULA prohibits reverse engineering/decompiling and asserts ownership of the software and documentation; it also restricts use of OriginLab trademarks. Whether an independent interpreter written from public documentation is permitted, and whether documentation examples may be copied into a test corpus, is `U`. `S`/`U`.
- Owner and next evidence action: Compatibility/interpreter lead with external legal review. Commission a clean-room LabTalk subset definition and a legal review of documentation and example reuse before any source-compatibility claim.

#### CR-10 Origin C

- Catalog and requirement: `EG-CAT-COMPAT-009`; `ORG-AUTO-002`; `EG-ADD-014`.
- Source and version: Origin C Programming Guide and Basic Features, retrieved 2026-10-31: "Origin C is a high-level language closely based on ANSI C"; Code Builder includes a "compiler, linker, and debugger"; the compiler/runtime is bundled in Origin. No standalone compiler or runtime download, formal language specification or ABI was found. `S`.
- Rights `R-ADVERSE`; specification `SPEC-DOC`; fixture `FIX-SYNTH` plus `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Source compatibility is required by the inventory, but no compiler, runtime or API layer exists. |
| Data | `Open` | The API data types and library calls are publicly documented, but header reuse and runtime data semantics are unverified. |
| Formula | `Open` | The C-like language and Origin API are documented, but no formal language specification exists; compiled `.ocz` binaries are a separate unresolved question. |
| ABI | `N/A (U)` | Source compatibility target; compiled binaries would be a separate native ABI question not covered here. |

- Vendor terms: the same Origin EULA restrictions as CR-09. Whether public API headers may be used to build a compatibility layer is `U`. `S`/`U`.
- Owner and next evidence action: Compatibility lead with external legal review. Confirm whether any licensed standalone Origin C runtime exists; scope any effort to source-level import/conversion, not compiled-object execution.

#### CR-11 XOP-like native ABI

- Catalog and requirement: `EG-CAT-COMPAT-012`; `S9:119`; `EG-ADD-014`.
- Source and version: Origin documentation retrieved 2026-10-31. The official `xop` page describes "an X-Function (xop)" that runs operation classes and is designed for LabTalk, not an external-operation native SDK. The Origin Developer Kit page states that a developer kit is included in the Origin/OriginPro installation; it describes dialogs and resource DLLs, not a native XOP ABI. No official XOP SDK page, ABI specification, header set or download was found. Origin C can call generic external DLLs, but that is a generic DLL ABI, not an XOP contract. `S`/`U`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-NONE`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | A native extension route is a long-term requirement, but no SDK or ABI is evidenced. |
| Data | `Open` | Native extension data exchange is undefined without an ABI. |
| Formula | `Open` | Native extension entry points and operation semantics are undefined. |
| ABI | `Open` | No public evidence of a stable XOP ABI; the artifact's premise is unresolved. Do not assume a partner SDK exists. Closure: a written vendor statement identifying an SDK, its version and access terms. |

- Owner and next evidence action: Partnerships with external legal review. Ask OriginLab directly whether a partner XOP SDK or stable native ABI exists and under what terms; absent a written answer, keep the artifact unresolved and do not invent an ABI.

#### CR-12 Vendor Apps

- Catalog and requirement: `EG-CAT-COMPAT-010`; `ORG-APPS-001` through `ORG-APPS-006`; `EG-ADD-014`.
- Source and version: Origin App Development and App Distribution documentation, the Origin Apps user guide and File Exchange submission guidelines, retrieved 2026-10-31. Apps are distributed as `.opx` packages and may depend on LabTalk, Origin C, X-Functions, Python, templates and data. App pages expose a per-App "License" field; the guidelines state that the author must be the original author and that free distribution is the default, not an open-source grant. Package Manager can encrypt C/CPP files. `S`.
- Rights `R-ADVERSE`; specification `SPEC-NONE`; fixture `FIX-USER`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | EveryGraph's own extension marketplace is not vendor-App compatibility. No App runtime or per-App behavior is implemented. |
| Data | `Open` | Per-App data models and dependencies vary; no common contract exists. |
| Formula | `Open` | App logic may be LabTalk, Origin C, X-Function or Python; each carries its own unresolved language gate. |
| ABI | `Open` | `.opx`/`.ocz` packaging internals are not a public ABI; encrypted sources preclude inspection. |

- Vendor terms: per-App licenses vary and no source-redistribution grant appears on App pages; free download is not an open-source license. `I`. Encrypted App sources cannot be reused. `S`.
- Owner and next evidence action: Compatibility analyst with external legal review. Sample ten Apps for author, license, declared dependencies and encryption; require a per-App manifest before any vendor-App compatibility statement.

#### CR-13 COM Automation Server and Orglab

- Catalog and requirement: `EG-CAT-COMPAT-011`; `ORG-AUTO-008`; `EG-ADD-014`.
- Source and version: Origin Automation Server documentation and Orglab documentation/EULA, retrieved 2026-10-31. The COM documentation states that "Origin can function as an automation server" and that a client application "does not work across multiple versions." Orglab is documented as a "freely distributable COM library for directly creating Origin project files"; it is a 64-bit Windows library registered with `regsvr32`. The `originlab/orglab` GitHub repository is documentation-only and has no license. `S`.
- Rights `R-VENDOR` for COM, `R-ADVERSE` for Orglab; specification `SPEC-PARTIAL` for COM, `SPEC-DOC` for Orglab; fixture `FIX-LICENSED-LOCAL` for COM, `FIX-NONE` for Orglab; uncertainty `U4` (COM, Windows deferred) and `U3` (Orglab terms); support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | COM automation is Windows-only and deferred by `ADR-0005`; no macOS test is possible. Orglab cannot be embedded because its EULA prohibits competing use. |
| Data | `Open` | COM data exchange and Orglab project creation are undocumented at the container level; Orglab's competing-use restriction keeps it out of scope. |
| Formula | `Open` | The COM object model exposes Origin operations, but version coupling prevents a stable semantic contract. |
| ABI | `Open` | COM interop assemblies are version-coupled and do not provide a cross-version binary ABI. Orglab is a Windows COM library, not an open standard. |

- Vendor terms: the Orglab EULA retrieved 2026-10-31 states that the licensee "may not resell, rent, lease, distribute or otherwise use the Developer Software in any way that would compete with OriginLab's Origin and OriginPro products" and may not copy, decompile or reverse engineer it. Orglab must not be embedded in EveryGraph. `S` (not legal advice).
- Owner and next evidence action: Windows platform lead (deferred) plus external legal review for COM; legal owner for Orglab. Take no COM action until the Windows gate resumes; record Orglab as excluded and do not use its documentation as an implementation source.

### 5.5 Licensed engines

#### CR-14 MATLAB MAT-file data

- Catalog and requirement: `EG-CAT-IO-005`; `ORG-IO-006`; `EG-ADD-014` (licensed-engine group).
- Source and version: MathWorks "MAT File Versions" page, R2026b, retrieved 2026-10-31: "By default, all save operations create Version 7 MAT files"; "Version 7.3 MAT files use an HDF5 based format". The MAT-File Format PDF returned HTTP 403 in the retrieval environment; access conditions were not established and it was not inspected. The Library of Congress format description states that "Level 4 and later Level 5 variants have been openly documented since 1999." `S`/`U`.
- Rights `R-VENDOR`; specification `SPEC-PARTIAL`; fixture `FIX-PUBLIC` plus `FIX-LICENSED-LOCAL`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | MATLAB file import is a preserved requirement; no mapping study exists. |
| Data | `Open-partial` | Level 4 and Level 5 are publicly documented (the current PDF retrieval is gated; the Library of Congress description corroborates the historical publication). Version 7.3 is HDF5-based, but the public mapping of MATLAB classes, sparse/complex/struct values and object metadata is not confirmed. Closure: a verified version matrix and loss report. |
| Formula | `Open` | MAT files store arrays rather than formulas; however, v7.3 HDF5 structure and class metadata affect faithful reconstruction. |
| ABI | `N/A (U)` | Data file; no binary ABI surface identified. |

- Rights: MathWorks documentation is copyrighted; no implementation or redistribution grant was stated. Permissive readers (scipy BSD-3, matio BSD-2, h5py BSD-3) demonstrate practice, not a license for MATLAB-generated content. `I`.
- Owner and next evidence action: Scientific data lead. A standards researcher with authorized access must retrieve the current MAT-File Format specification and verify v7.3 scope; legal review must confirm specification use and fixture redistribution. Until then, no MAT import claim.

#### CR-15 MATLAB Engine and MATLAB language

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-005`; `EG-ADD-014`.
- Source and version: MATLAB Engine API for Python repository (branch R2026a) and MathWorks installation documentation, retrieved 2026-10-31: the engine is "licensed to you solely for use in conjunction with MathWorks products"; no standalone console runtime is offered. No public normative MATLAB language specification was found; GNU Octave is an independent GPL-3 implementation with no MathWorks compatibility grant. `S`.
- Rights `R-ADVERSE`; specification `SPEC-DOC`; fixture `FIX-NONE`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | MATLAB Console integration requires an installed, licensed MATLAB; no product route is approved. |
| Data | `Open` | Engine marshalling behavior and version matrices are undocumented for the intended use. |
| Formula | `Open` | No public normative MATLAB language specification; `.m` source compatibility is not promised. |
| ABI | `Open` | The Engine API is a licensed interface with a field-of-use condition; the wrapper license is not OSI/Apache-2.0-compatible. |

- Vendor terms: the engine wrapper is "licensed to you solely for use in conjunction with MathWorks products"; an installed MATLAB is required. `S`. Whether the wrapper may be shipped inside EveryGraph is `U`.
- Owner and next evidence action: Scientific computing lead with procurement and external legal review. Obtain a legal opinion on the field-of-use clause and CI licensing; scope any integration to invoking a user-installed MATLAB, never bundling MATLAB or claiming `.m` source compatibility.

#### CR-16 Mathematica link and Wolfram Language expressions

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-006`; `EG-ADD-014`.
- Source and version: Wolfram documentation, retrieved 2026-10-31, states that `.nb` is an "ASCII format based on the Wolfram Language expression syntax" and that the whole notebook is ultimately represented as Wolfram Language expressions. No normative `.nb` or language specification was found. `S`.
- Rights `R-VENDOR`; specification `SPEC-DOC`; fixture `FIX-SYNTH` plus `FIX-USER`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Mathematica Link is a preserved exchange capability; no implementation exists. |
| Data | `Open` | Wolfram Language expressions and notebooks are publicly described but not normatively specified; no independent reader/writer is verified. |
| Formula | `Open` | Wolfram Language semantics are extensively documented but not specified in a normative grammar or conformance suite. |
| ABI | `Open` | The actual exchange path is WSTP, recorded separately in CR-17; no ABI claim is made here. |

- Owner and next evidence action: Scientific computing lead. Define whether exchange is data-only or expression-level; verify round-trip behavior with hand-authored notebooks under a licensed engine; do not claim notebook compatibility.

#### CR-17 Wolfram Engine and WSTP

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-006`; `EG-ADD-014`.
- Source and version: Wolfram Engine terms and FAQ, retrieved 2026-10-31: the Free Wolfram Engine is for "Development but not production of software applications"; it may not be redistributed; running it "expressly to produce output for commercial or organizational use" is prohibited. WSTP is "usually distributed with the Wolfram System"; no public WSTP redistribution terms were found. Engine version 15.0 is reported by the research note but was not displayed in the executor's spot check. `S`/`U`.
- Rights `R-ADVERSE`; specification `SPEC-DOC`; fixture `FIX-NONE`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | A production integration requires a paid/OEM license; the free engine cannot be used for production output. |
| Data | `Open` | WSTP data marshalling is documented only partially; no independent protocol implementation is verified. |
| Formula | `Open` | Wolfram Language evaluation requires a licensed engine; no semantic equivalence is established. |
| ABI | `Open` | WSTP libraries are proprietary and distributed with the Wolfram System; no public wire-format specification or redistribution terms were found. |

- Owner and next evidence action: Procurement with external legal review. Obtain OEM/production and WSTP redistribution terms; otherwise support only invocation of a user-installed, licensed engine and record the restriction.

#### CR-18 NI LabVIEW

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-007`; `EG-ADD-014`.
- Source and version: NI Software License Agreement (370406AE-01, September 2026) and NI LabVIEW documentation, retrieved 2026-10-31. The SLA prohibits reverse engineering, decompiling, disassembling and attempts to derive "file formats, non-public APIs, ideas, or algorithms". NI states that "LabVIEW is currently only available for macOS in the Community Edition, beginning with LabVIEW 2025 Q3"; the Community Edition is restricted to personal, non-commercial, non-industrial use. VI Server documentation is public; ActiveX is a Windows COM interface. `S`.
- Rights `R-ADVERSE`; specification `SPEC-NONE`; fixture `FIX-USER` plus `FIX-NONE`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | LabVIEW SubVI integration requires a licensed Windows LabVIEW; macOS has only a non-commercial Community Edition, and the Run-Time Engine alone cannot run VIs. |
| Data | `Open` | The VI file format is proprietary and undocumented; the NI SLA restricts file-format discovery. No lawful parser path is established. |
| Formula | `Open` | Block-diagram semantics are not publicly specified; execution requires the LabVIEW runtime. |
| ABI | `Open` | VI Server/ActiveX automation is a Windows interface; no native macOS ActiveX path is identified. `I`. |

- Owner and next evidence action: Platform lead with external legal review. Obtain written NI permission before any VI parsing; treat commercial LabVIEW integration on macOS as unavailable until NI states otherwise; test VI Server only in a non-commercial Community Edition if that is consistent with the intended use.

#### CR-19 R language

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-004`; `EG-ADD-014`.
- Source and version: R Licenses page and R FAQ, retrieved 2026-10-31, state "R as a package is licensed under GPL-2 | GPL-3" and that the license restricts distribution of R or programs containing R code; commercial use is permitted. The R Language Definition (R 4.6.1, 2026-06-24) is public and carries a separate verbatim/modified-copy notice, not the GPL. No normative R data-serialization specification was found. `S`.
- Rights `R-OPEN`; specification `SPEC-PUBLIC`; fixture `FIX-SYNTH`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | R execution is a required integration route; no architecture or deployment decision exists. |
| Data | `Open` | R object serialization has no normative public specification; the exchange design is undefined. |
| Formula | `Open-partial` | The R Language Definition is a comprehensive public definition, but it is not an ISO/ECMA standard and its notice constrains redistribution. |
| ABI | `N/A (U)` | Source/process integration; no binary ABI target identified. |

- Rights note: GPL-2/GPL-3; a separate process that invokes a user-installed R avoids distributing R, while bundling R triggers GPL obligations. This is an engineering inference, not a legal conclusion. `I`.
- Owner and next evidence action: Scientific computing lead with external legal review. Decide separate-process versus bundled deployment under legal review; define a data-exchange contract that does not distribute R; preserve the R Language Definition notice rather than redistributing modified copies.

#### CR-20 Rserve

- Catalog and requirement: `EG-CAT-COMPAT-013` (licensed engines); `ORG-AUTO-004`; `EG-ADD-014`.
- Source and version: CRAN Rserve 1.8-19.1, published 2026-08-29, license `GPL-2`; the vendor technical documentation describes the Rserve protocols and structures for version 0.1-9, which is older than the current package. Protocols listed on CRAN include QAP, WebSockets, HTTP and HTTPS. `S`.
- Rights `R-OPEN`; specification `SPEC-PARTIAL`; fixture `FIX-SYNTH`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Rserve is a candidate invocation route; no client prototype exists. |
| Data | `Open-partial` | The QAP1 description is public but old and partial; current source is the de facto reference. |
| Formula | `Open` | R evaluation semantics remain the R-language question in CR-19; deployment licensing is unresolved. |
| ABI | `Open` | The wire protocol is documented partially; the current protocol document's license is unknown. |

- Owner and next evidence action: Integration engineer with external legal review. Diff the published protocol against Rserve 1.8-19.1, verify the protocol document's license, and keep deployment under the CR-19 review.

### 5.6 Documented proprietary connectors

#### CR-21 Minitab

- Catalog and requirement: `EG-CAT-IO-008`; `ORG-IO-001`; `EG-ADD-014` (proprietary connector group).
- Source and version: Minitab help retrieved 2026-10-31 lists project formats MPX/MPJ and worksheet formats MWX/MTW, and states that since Minitab 19 projects are saved as MPX. The Origin Minitab Connector v3.13 supports `.mpx`/`.mwx`, not the inventory's MTW/MPJ. The Minitab Software Subscription Agreement V01-01-2024 prohibits reverse engineering/decompiling and seeking non-public APIs, and prohibits use to build a competing product or service. `S`.
- Rights `R-ADVERSE`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Minitab import is a preserved requirement; no decoder or version matrix exists. |
| Data | `Open` | No public format specification; the connector and inventory name different format generations. Closure: a versioned specification and a corrected scope. |
| Formula | `Open` | Project analyses and computed columns are undocumented. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. |

- Owner and next evidence action: Integration lead with external legal review. Request format and license clarification from Minitab; reconcile the MTW/MPJ claim with the current MPX/MWX formats; review the competing-product clause before any implementation.

#### CR-22 Ivium

- Catalog and requirement: `ORG-IO-001`; functional scope line 60; `EG-ADD-014` (documented proprietary connectors).
- Source and version: Origin Ivium Connector v1.00 (minimum Origin 2026), retrieved 2026-10-31, advertises connection to Ivium `*.ids` and `*.idf` files. No public format specification was found; Ivium vendor terms and SDK terms were not inspectable; a third-party Python wrapper around the IviumSoft DLL asserts no SPDX license. `S`/`U`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U4`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Electrochemistry import is a documented connector target, but no product requirement or format evidence exists. |
| Data | `Open` | No public format documentation or lawful corpus; the vendor DLL is an interface, not a format specification. |
| Formula | `Open` | Unknown whether files contain computed signals or analysis results. |
| ABI | `Open` | A vendor DLL exists, but its terms and redistribution rights are unknown and it is not a supported ABI contract. |

- Owner and next evidence action: Integration lead. Obtain file-format and SDK terms from Ivium; use a plain CSV route for any near-term functional need without calling it format compatibility.

#### CR-23 RPC3

- Catalog and requirement: `ORG-IO-001`; functional scope line 60; `EG-ADD-014` (documented proprietary connectors).
- Source and version: Origin RPC3 Connector v1.00 (minimum Origin 2025), retrieved 2026-10-31, advertises connection to `*.rsp` files. NI's MTS DataPlugin for RPC-III 21.0 (July 2021) documents support for files created by MTS RPC III and RPC Pro, including `.rsp` and `.tim`. Origin does not name MTS; the identity is an inference. No full normative MTS specification was found. `S`/`I`/`U`.
- Rights `R-VENDOR`; specification `SPEC-PARTIAL`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Materials-testing import is a documented connector target; no implementation or product decision exists. |
| Data | `Open-partial` | A third-party NI plugin documents partial semantics; no MTS specification or version matrix was located. |
| Formula | `Open` | Unknown whether test computations are stored. |
| ABI | `Open` | The NI plugin is an implementation, not a public ABI contract. |

- Owner and next evidence action: Integration lead. Confirm the MTS identity with OriginLab and obtain the RPC III specification and license; until then keep the connector unresolved.

#### CR-24 DMD

- Catalog and requirement: `ORG-IO-001`; functional scope line 60; `EG-ADD-014` (documented proprietary connectors).
- Source and version: Origin DMD Connector v1.00 (minimum Origin 2025), retrieved 2026-10-31, advertises connection to `*.dmd` files but does not name the vendor. DEWETRON's FAQ states that data is stored in the proprietary `*.dmd` format and that a DLL is provided; DEWETRON's `pyDmdReader` is an MIT wrapper around the proprietary reader API. Origin does not state that DMD is DEWETRON; the identity is an inference. `S`/`I`.
- Rights `R-BLOCKED`; specification `SPEC-NONE`; fixture `FIX-LICENSED-LOCAL`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Measurement-data import is a documented connector target; no implementation or product decision exists. |
| Data | `Open` | No public format specification; a proprietary reader API exists, but its terms, platform support and redistribution rights are unknown. |
| Formula | `Open` | Unknown whether derived channels are stored. |
| ABI | `Open` | The reader DLL is a vendor API, not an open ABI; terms and macOS availability are unknown. |

- Owner and next evidence action: Integration lead. Obtain the DMD Reader API license and platform matrix from DEWETRON; confirm the identity with OriginLab; keep the connector unresolved.

#### CR-25 ADO/ODBC

- Catalog and requirement: `EG-CAT-IO-035`; `ORG-IO-004`; `EG-ADD-014` (connector group).
- Source and version: Microsoft ODBC overview and the ODBC 4.0 specification (MIT), retrieved 2026-10-31; the ADO reference is documented as applying to Access/Office 2013 and is a legacy Windows COM interface. Additional database drivers are separately licensed. DataGraph 5.5 is described on its product page as linking to local or online CSV, JSON and SQL sources; the driver is undocumented. `S`.
- Rights `R-VENDOR` (drivers) with `R-OPEN` for the ODBC specification; specification `SPEC-PUBLIC`; fixture `FIX-PUBLIC`; uncertainty `U2`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Database import is a preserved requirement; no macOS ODBC stack is selected. |
| Data | `Open` | The ODBC API is public, but driver behavior, query result types and credentials handling require a qualification matrix. |
| Formula | `N/A (U)` | Query results do not carry spreadsheet formulas; no formula surface identified. |
| ABI | `Open-partial` | The ODBC API specification is public; driver availability, licensing and ABI behavior on macOS remain to be qualified. ADO is a legacy Windows interface, not a macOS route. |

- Owner and next evidence action: Platform lead with data integration. Select a macOS ODBC stack, review driver licenses and test against an open-source database; do not claim ADO support on macOS.

#### CR-26 SQL and cloud data sources

- Catalog and requirement: `EG-CAT-IO-036`; `DG-IO-012`; `ORG-IO-007`; functional scope line 60; `EG-ADD-014` (connector group).
- Source and version: DataGraph 5.5 product page (2025) describes linking to local or online CSV, JSON and SQL sources; the preserved inventory describes web, local, network and cloud sources with credentials, refresh and re-import; no driver contract or version matrix is published. `S`/`U`.
- Rights `R-VENDOR`; specification `SPEC-NONE`; fixture `FIX-PUBLIC`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | Refreshable remote data is a preserved requirement; no provider list or architecture is settled. |
| Data | `Open` | No driver contract, authentication model or refresh semantics are documented; each provider has its own terms. |
| Formula | `N/A (U)` | Source data does not carry spreadsheet formulas; no formula surface identified. |
| ABI | `Open` | Provider protocols and SDKs are unspecified and individually licensed. |

- Owner and next evidence action: Data integration lead. Define a provider-neutral connector contract and a per-provider rights inventory before selecting any SDK; keep cloud variants unresolved.

### 5.7 Formula interchange

#### CR-27 Excel formula import

- Catalog and requirement: `EG-CAT-IO-002` and `ORG-IO-003` for workbook import; the 2026 cell-formula detail at `S1:472`; functional scope lines 52 and 119; `EG-ADD-014` formula-axis addition.
- Source and version: ECMA-376, 5th edition, December 2021, and ISO/IEC 29500; Microsoft Open XML formula documentation, retrieved 2026-10-31, states that "more than 300 predefined functions" are defined by the specification and refers to ISO/IEC 29500 for formula syntax. Microsoft's Open Specification Promise covers covered versions of Office Open XML and promises not to assert necessary claims. Implementation notes are partial. `S`.
- Rights `R-VENDOR`; specification `SPEC-PARTIAL`; fixture `FIX-PUBLIC` plus `FIX-SYNTH`; uncertainty `U3`; support claim none; gate unresolved.

| Axis | Status | Evidence and closure condition |
|---|---|---|
| Functional | `Open` | The inventory requires Excel-formula import with supported-function diagnostics; no parser or diagnostic plan exists. |
| Data | `N/A (U)` | Workbook data import is `EG-CAT-IO-002`; this record covers formula semantics only and does not duplicate the data gate. |
| Formula | `Open-partial` | The grammar and function names are publicly standardized, but evaluation details, volatile functions, dynamic arrays, implicit intersection, locale separators and localized function names are not fully specified. Closure: a versioned function-coverage, locale and volatility gap matrix with diagnostics. |
| ABI | `N/A (U)` | File format; no binary ABI surface identified. |

- Rights note: ECMA-376 is freely downloadable and the OSP covers necessary claims for covered versions, but third-party patents, locale data and the ECMA patent policy's exact application require legal review. `S`/`U`.
- Owner and next evidence action: Spreadsheet compatibility engineer with external legal review. Build a function/locale/volatility coverage matrix and a diagnostics policy; do not silently rewrite unsupported formulas; obtain legal review before relying on the OSP.

## 6. Gate roll-up

All 27 records are unresolved. The table is a status index, not a support matrix.

| Record | Artifact | Rights | Specification | Fixture plan | Functional | Data | Formula | ABI | Gate |
|---|---|---|---|---|---|---|---|---|---|
| CR-01 | Origin OPJ | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-02 | Origin OPJU | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-03 | Protected OPJ/OPJU | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-04 | DataGraph package | `R-BLOCKED` | `SPEC-DOC` | `FIX-LICENSED-LOCAL` | Open | Open-partial | Open | N/A (U) | Unresolved |
| CR-05 | DataGraph single file | `R-BLOCKED` | `SPEC-DOC` | `FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-06 | DataGraph DTG | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | N/A (U) | N/A (U) | Unresolved |
| CR-07 | Legacy Plot | `R-BLOCKED` | `SPEC-NONE` | `FIX-USER` | Open | Open | N/A (U) | N/A (U) | Unresolved |
| CR-08 | Prism | `R-ADVERSE` | `SPEC-PARTIAL` | `FIX-SYNTH`/`FIX-LICENSED-LOCAL` | Open | Open-partial | Open | N/A (U) | Unresolved |
| CR-09 | LabTalk | `R-ADVERSE` | `SPEC-DOC` | `FIX-SYNTH`/`FIX-USER` | Open-partial | Open | Open-partial | N/A (U) | Unresolved |
| CR-10 | Origin C | `R-ADVERSE` | `SPEC-DOC` | `FIX-SYNTH`/`FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-11 | XOP-like ABI | `R-BLOCKED` | `SPEC-NONE` | `FIX-NONE` | Open | Open | Open | Open | Unresolved |
| CR-12 | Vendor Apps | `R-ADVERSE` | `SPEC-NONE` | `FIX-USER` | Open | Open | Open | Open | Unresolved |
| CR-13 | COM and Orglab | `R-VENDOR`/`R-ADVERSE` | `SPEC-PARTIAL`/`SPEC-DOC` | `FIX-LICENSED-LOCAL`/`FIX-NONE` | Open | Open | Open | Open | Unresolved |
| CR-14 | MATLAB MAT | `R-VENDOR` | `SPEC-PARTIAL` | `FIX-PUBLIC`/`FIX-LICENSED-LOCAL` | Open | Open-partial | Open | N/A (U) | Unresolved |
| CR-15 | MATLAB engine/language | `R-ADVERSE` | `SPEC-DOC` | `FIX-NONE` | Open | Open | Open | Open | Unresolved |
| CR-16 | Mathematica link | `R-VENDOR` | `SPEC-DOC` | `FIX-SYNTH`/`FIX-USER` | Open | Open | Open | Open | Unresolved |
| CR-17 | Wolfram Engine/WSTP | `R-ADVERSE` | `SPEC-DOC` | `FIX-NONE` | Open | Open | Open | Open | Unresolved |
| CR-18 | LabVIEW | `R-ADVERSE` | `SPEC-NONE` | `FIX-USER`/`FIX-NONE` | Open | Open | Open | Open | Unresolved |
| CR-19 | R language | `R-OPEN` | `SPEC-PUBLIC` | `FIX-SYNTH` | Open | Open | Open-partial | N/A (U) | Unresolved |
| CR-20 | Rserve | `R-OPEN` | `SPEC-PARTIAL` | `FIX-SYNTH` | Open | Open-partial | Open | Open | Unresolved |
| CR-21 | Minitab | `R-ADVERSE` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | N/A (U) | Unresolved |
| CR-22 | Ivium | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | Open | Unresolved |
| CR-23 | RPC3 | `R-VENDOR` | `SPEC-PARTIAL` | `FIX-LICENSED-LOCAL` | Open | Open-partial | Open | Open | Unresolved |
| CR-24 | DMD | `R-BLOCKED` | `SPEC-NONE` | `FIX-LICENSED-LOCAL` | Open | Open | Open | Open | Unresolved |
| CR-25 | ADO/ODBC | `R-VENDOR`/`R-OPEN` | `SPEC-PUBLIC` | `FIX-PUBLIC` | Open | Open | N/A (U) | Open-partial | Unresolved |
| CR-26 | SQL/cloud | `R-VENDOR` | `SPEC-NONE` | `FIX-PUBLIC` | Open | Open | N/A (U) | Open | Unresolved |
| CR-27 | Excel formulas | `R-VENDOR` | `SPEC-PARTIAL` | `FIX-PUBLIC`/`FIX-SYNTH` | Open | N/A (U) | Open-partial | N/A (U) | Unresolved |

Roll-up conclusion: zero records have a support claim; zero records have a read, write or round-trip decision; zero records have external legal clearance. `EG-CAT-GATE-006` remains open.

## 7. Rights and license constraints

### 7.1 Vendor terms observed

| Vendor/artifact | Observed term | Status |
|---|---|---|
| Origin/OriginPro (OPJ, OPJU, LabTalk, Origin C, COM, Apps) | "You will not reverse engineer or decompile the Software"; software and documentation ownership; trademark restriction. No interoperability or sample-redistribution clause was found. | `S`; legal review open |
| Origin Orglab | Licensee may not use the developer software "in any way that would compete with OriginLab's Origin and OriginPro products"; no copy, decompile or reverse engineering. | `S`; embedding prohibited by the observed terms |
| GraphPad Prism | Reverse-engineering prohibition; the 2025-06-10 terms reportedly prohibit permitting a third party to parse proprietary file types such as `.pzfx` or `.prism` for a commercial purpose; current terms effective 2026-08-11, Dotmatics terms after 2026-08-12; exact parsing clause not re-confirmed in the spot check. | `S`/`U`; legal review mandatory |
| Minitab | Subscription agreement V01-01-2024 prohibits reverse engineering, decompiling and seeking non-public APIs, and use to build a competing product or service. | `S`; legal review mandatory |
| MATLAB Engine | Licensed "solely for use in conjunction with MathWorks products"; wrapper carries a field-of-use condition. | `S`; legal review mandatory |
| Wolfram Engine/WSTP | Free engine for development, not production or commercial/organizational output; copies may not be redistributed; WSTP ships with the Wolfram System. | `S`; legal review mandatory |
| NI LabVIEW | SLA prohibits reverse engineering, decompiling and discovering file formats or non-public APIs; macOS offered only as a non-commercial Community Edition. | `S`; legal review mandatory |
| DataGraph | No public EULA or format terms were located; no permission stated. | `U` |
| GraphPad PZFX schema archive | The schema files are published; their redistribution license was not stated in the inspected page. | `U` |
| Ivium, RPC3/MTS, DMD/DEWETRON | Vendor terms were not inspectable; a proprietary DLL/reader API exists for some artifacts but is not a format license. | `U` |

### 7.2 Open-source implementation compatibility

The following table is engineering triage for a future architecture decision. This register adopts no implementation.

| Project | Artifact | License | Apache-2.0 compatibility assessment |
|---|---|---|---|
| liborigin | OPJ/OPJU reader | GPL-3.0 repository (headers GPL-2.0-or-later) | No; copyleft and reverse-engineered provenance |
| pzfx (R package) | PZFX read/write | MIT | Yes, subject to the Prism rights question |
| rpc3-file | RPC3 read/write | BSD-2-Clause | Yes, provenance must be verified |
| pyDmdReader | DMD reader wrapper | MIT | Code yes; depends on a proprietary DLL whose terms are unknown |
| scipy | MAT Level 4/5 | BSD-3-Clause | Yes |
| matio | MAT read/write | BSD-2-Clause | Yes |
| h5py | MAT v7.3/HDF5 | BSD-3-Clause | Yes |
| mat73 | MAT v7.3 | GPL-3.0 | No; copyleft |
| GNU Octave | MATLAB-like language | GPL-3.0 | No for combined distribution; separate process only with review |
| rpy2 | R interface | GPL-2.0 | No for combined distribution |
| Rserve | R server | GPL-2.0 | No for combined distribution; separate process review |
| WolframClientForPython | Wolfram client | MIT | Code yes; requires a licensed engine |
| wstp-rs | WSTP bindings | Apache-2.0 | Bindings only; runtime proprietary |
| pylabview | VI reader | MIT | Code yes; VI-format legality is high-risk under the NI SLA |
| HyperFormula | Spreadsheet formulas | GPL-3.0 or commercial | No under the default Apache-2.0 policy |
| Apache POI | OOXML formulas | Apache-2.0 | Yes |
| openpyxl | OOXML formulas | MIT | Yes |
| SheetJS | Spreadsheet parsing | Apache-2.0 | Yes |

License compatibility is an engineering assessment, not a legal conclusion. No project in this table is approved for adoption by this register.

## 8. Lawful fixture and corpus policy

1. Fixtures are created, not acquired, unless a public corpus has a verified compatible license. Vendor manuals, SDKs, installers and sample files are not redistributed by this task.
2. A fixture created by a licensed installation uses synthetic data, records the vendor product and version, the operating system, the save options and a SHA-256 hash, and is retained outside the repository when redistribution is not permitted.
3. A public corpus is used only after its license, attribution and modification terms are recorded; GPL corpora are not mixed into Apache-2.0 test assets without review.
4. A user-supplied file is used only for local observation, is never committed and is deleted after the observation unless the owner authorizes retention.
5. Protected or encrypted fixtures require explicit owner authorization, a security policy and a documented password-handling procedure. No bypass or decryption workaround is permitted.
6. Fixture records include: fixture ID, artifact, origin (`synthetic`, `licensed-local`, `public`, `user`), product/version, OS, checksum, rights basis, redistribution flag and retention decision.
7. Every fixture plan includes a loss report and an unsupported-feature diagnostic. A round-trip fixture compares data, metadata, formulas and identity where the artifact carries them.
8. Fixture evidence does not close a gate without a reviewed specification and rights position.

## 9. External legal review: open dependency

External legal review has not been performed. No family is legally cleared. This is a required dependency for acceptance of any compatibility pack, not a task that this register can perform.

| Review question | Artifacts | Why it matters |
|---|---|---|
| Does the Origin EULA permit independent interoperability readers, format research and fixture use? | CR-01 to CR-03, CR-09 to CR-13 | The reverse-engineering ban is broad and no interoperability clause was found |
| Do the GraphPad/Dotmatics terms permit an independent PZFX parser and schema use? | CR-08 | The reported commercial-parsing restriction is the strongest adverse signal |
| Does the Minitab competing-product clause reach an open-source analysis application? | CR-21 | The clause is broader than a reverse-engineering ban |
| May a user-installed MATLAB be invoked, and may any wrapper be shipped? | CR-14, CR-15 | Field-of-use and installed-product requirements |
| What Wolfram production/OEM and WSTP redistribution terms apply? | CR-16, CR-17 | The free engine cannot be used for production output |
| May an application read user-supplied LabVIEW VIs? | CR-18 | The NI SLA restricts file-format discovery |
| Where is the GPL boundary for R, Rserve and liborigin? | CR-01, CR-19, CR-20 | Separate-process and combined-work questions |
| May vendor fixtures be retained, hashed or redistributed? | All | No vendor grants sample redistribution |
| Do ECMA-376/ISO 29500 and the Microsoft OSP cover the intended Excel-formula implementation? | CR-27 | Patent and locale-data questions remain |
| May DataGraph or other vendor documentation be used as an implementation source? | CR-04 to CR-06 | No public EULA or terms were located |
| May vendor documentation or format descriptions be used where the terms are not public or not located? | CR-07, CR-22, CR-23, CR-24 | Vendor terms or documentation licenses must be acquired before review |

Owner: product owner with external counsel (legal review role). The responsible engineering role supplies the evidence packet and questions; counsel records a written opinion per family. Until then, every affected gate remains unresolved.

## 10. Ownership and next evidence actions

| Record | Accountable role | Next evidence action | Exit evidence |
|---|---|---|---|
| CR-01 | Integration lead + legal | Request a written OriginLab interoperability and fixture position | Written statement, legal opinion and read-only/write decision |
| CR-02 | Integration lead + legal | Include OPJU in the same written request | Written statement, version matrix and decision |
| CR-03 | Integration lead + security + legal | Define a protected-file policy before implementation | Approved policy, authorized fixture and security test plan |
| CR-04 | Integration lead | Request DataGraph container/member documentation and license | Written terms, member schema and version matrix |
| CR-05 | Integration lead | Confirm `.dgraphs`, container relationship and license | Written terms and corrected scope |
| CR-06 | Integration lead | Request a DTG specification and license | Written specification or an explicit unresolved decision |
| CR-07 | Integration lead | Confirm format identity and terms with the Plot2 vendor | Identity evidence and license statement |
| CR-08 | Integration lead + legal | Obtain a GraphPad/Dotmatics written position and schema license | Written position, schema license and version matrix |
| CR-09 | Compatibility/interpreter lead + legal | Commission a clean-room subset definition and documentation-reuse review | Approved subset definition and legal opinion |
| CR-10 | Compatibility lead + legal | Confirm whether a standalone runtime exists; review header reuse | Written vendor answer and legal opinion |
| CR-11 | Partnerships + legal | Ask OriginLab whether a partner XOP SDK/ABI exists | Written SDK/ABI statement or explicit no-SDK finding |
| CR-12 | Compatibility analyst + legal | Sample ten Apps for author, license, dependencies and encryption | Per-App manifest and license review |
| CR-13 | Windows platform lead + legal (COM); legal (Orglab) | Record Orglab exclusion; defer COM until Windows resumes | Exclusion record; COM legal opinion when Windows resumes |
| CR-14 | Scientific data lead | Retrieve the current MAT-File Format spec with authorized access; verify v7.3 | Verified version matrix and legal opinion |
| CR-15 | Scientific computing lead + procurement/legal | Obtain a field-of-use and CI licensing opinion | Legal opinion and an invocation-only architecture decision |
| CR-16 | Scientific computing lead | Define data-only versus expression-level exchange; test hand-authored notebooks | Exchange contract and round-trip evidence |
| CR-17 | Procurement + legal | Obtain OEM/production and WSTP redistribution terms | Written terms or an invocation-only decision |
| CR-18 | Platform lead + legal | Obtain written NI permission before any VI parsing; verify Community Edition terms | Written permission or an explicit unresolved decision |
| CR-19 | Scientific computing lead + legal | Decide separate-process versus bundled deployment | Legal opinion and deployment decision |
| CR-20 | Integration engineer + legal | Diff the protocol document against Rserve 1.8-19.1; verify its license | Protocol matrix and license record |
| CR-21 | Integration lead + legal | Request Minitab format/license clarification; reconcile MTW/MPJ with MPX/MWX | Written terms and corrected scope |
| CR-22 | Integration lead | Obtain Ivium file-format and SDK terms | Written terms or an explicit unresolved decision |
| CR-23 | Integration lead | Confirm MTS identity with OriginLab; obtain the RPC III spec | Written identity/specification evidence |
| CR-24 | Integration lead | Obtain the DMD Reader API license and platform matrix | Written terms and platform evidence |
| CR-25 | Platform lead + data integration | Select a macOS ODBC stack and review driver licenses | Qualified driver matrix and license record |
| CR-26 | Data integration lead | Define a provider-neutral connector contract and per-provider rights inventory | Contract proposal and rights inventory |
| CR-27 | Spreadsheet compatibility engineer + legal | Build a function/locale/volatility coverage matrix; review OSP use | Coverage matrix, diagnostics plan and legal opinion |

A role is a responsibility to assign, not a named person or an existing staffed team. Every action requires separate authorization where it involves vendor contact, account creation, purchases or fixture acquisition.

## 11. Contradictions and catalog corrections reported, not applied

This register does not edit the read-only catalog, the inventories or their owners' records. The following discrepancies are reported for the owning task to decide.

| Finding | Evidence | Suggested owner action |
|---|---|---|
| DataGraph single-file `.dgraph` naming | The documentation states the single-file extension is `.dgraphs` since 5.2. `EG-CAT-COMPAT-002` is named "DataGraph .dgraph single file"; the inventory describes two `.dgraph` save forms. | P0-004 owner should consider renaming or annotating the child; no edit was made here. |
| Minitab format generations | The inventory names MTW/MPJ; current Minitab saves MPX/MWX, and Origin's connector supports `.mpx`/`.mwx`. | P0-004 owner should reconcile the named formats and version scope. |
| Prism format generations | PZFX is the legacy XML format; current Prism files use `.prism`; published schemas cover 5-8; Origin's connector imports `.xml`, `.pzfx`, `.prism` and `.prismt`. | P0-004 owner should distinguish PZFX from current `.prism` and record the version scope. |
| OPJ save availability | Origin 2023 and later can read but cannot save OPJ. A catalog requirement phrased as OPJ save or round-trip may be impossible in current Origin. | P0-004 owner and product owner should record OPJ as legacy read-only unless Origin states otherwise. |
| Legacy Plot identity | DataGraph documents the legacy Plot as another macOS plotting program now called Plot2, not Origin or DataGraph. | P0-004 owner should record the third-party identity; the import is a migration, not a graph-compatibility claim. |
| XOP premise | The official `xop` page describes an X-Function for operation classes; no public native XOP SDK or stable ABI was found. | P0-003/P0-004 owners should preserve the artifact as an unresolved research question and not assume an SDK. |
| LabVIEW macOS licensing | NI states macOS is Community Edition only from 2025 Q3, restricted to non-commercial, non-industrial use; the Run-Time Engine alone cannot run VIs. | Platform owner should record the constraint in the platform plan; no macOS commercial LabVIEW integration claim. |
| Prism terms transition | Terms effective 2025-06-10 reportedly restrict commercial parsing; current PDF effective 2026-08-11; Dotmatics terms after 2026-08-12. | Legal review must bind the exact terms version before any work. |

## 12. Unknown register

| ID | Unknown | Needed evidence |
|---|---|---|
| U-CR-01 | OPJ/OPJU object, formula and version schema | Lawful specification or written vendor statement |
| U-CR-02 | Origin interoperability, fixture and documentation-reuse rights | Legal opinion and written vendor position |
| U-CR-03 | Protected-project algorithm and authorized handling | Security policy and owner-authorized fixture |
| U-CR-04 | DataGraph member schema and versioning | Vendor documentation or lawful fixture analysis |
| U-CR-05 | DataGraph package/single-file relationship and license | Vendor statement |
| U-CR-06 | DTG specification, extension and reader/writer rights | Vendor specification or explicit no-spec finding |
| U-CR-07 | Legacy Plot format identity and extension | Vendor or format-owner confirmation |
| U-CR-08 | Current Prism `.prism` schema and analysis semantics | Current schema or vendor statement |
| U-CR-09 | Prism parsing and schema rights under current terms | Legal opinion and written vendor position |
| U-CR-10 | LabTalk grammar, runtime and example-reuse rights | Clean-room definition and legal opinion |
| U-CR-11 | Origin C runtime, header reuse and compiled-object policy | Vendor answer and legal opinion |
| U-CR-12 | Existence and terms of a native XOP SDK/ABI | Written vendor statement |
| U-CR-13 | Per-App licenses, dependencies and encrypted-source policy | Per-App manifest and legal review |
| U-CR-14 | COM cross-version ABI and macOS relevance | Windows test evidence or explicit deferral |
| U-CR-15 | MAT v7.3 mapping, current specification access and fixture rights | Authorized spec retrieval and legal review |
| U-CR-16 | MATLAB `.m` language rights and Engine field-of-use scope | Legal opinion |
| U-CR-17 | Wolfram notebook format rights and WSTP redistribution | Written terms or OEM agreement |
| U-CR-18 | LabVIEW VI format permission and macOS commercial path | Written NI permission or explicit unresolved decision |
| U-CR-19 | R data-serialization contract and deployment boundary | Architecture decision and legal review |
| U-CR-20 | Current Rserve protocol and document license | Protocol diff and license record |
| U-CR-21 | Minitab format generation and competing-product scope | Written terms and corrected catalog scope |
| U-CR-22 | Ivium format and SDK terms | Written vendor terms |
| U-CR-23 | RPC3/MTS identity and specification | Written identity and specification evidence |
| U-CR-24 | DMD/DEWETRON identity, reader API terms and macOS availability | Written terms and platform matrix |
| U-CR-25 | macOS ODBC driver stack and licenses | Qualified driver matrix |
| U-CR-26 | Provider-neutral SQL/cloud contract and per-provider rights | Connector contract and rights inventory |
| U-CR-27 | Excel formula evaluation, locale and volatility coverage | Coverage matrix and legal opinion |
| U-CR-GATE-01 | Any external legal clearance | Written counsel opinions per family |

## 13. Verification and limits

- Retrieval: public HTTPS pages and public repositories were read on 2026-10-31. Some vendor pages were JavaScript-rendered, returned HTTP 403, were login-gated or could not be parsed; those cases are marked `U` or quoted only from a search snippet.
- No vendor application was installed or run; no SDK, installer, manual PDF requiring registration, or sample corpus was downloaded. No account was created and no EULA was accepted.
- No fixture was acquired, generated or committed. No price, schedule or staffing estimate is made.
- No legal review was performed. Rights statuses are engineering triage, not legal advice.
- Live Help sites are unversioned; a later check must record a new date, new excerpts and new hashes. Vendor documentation can change without notice.
- The register does not test remote URL availability at validation time, does not render diagrams and does not claim scientific or semantic correctness of the vendor documentation.
- The catalog, inventories, functional scope, architecture documents and other task records were read only. Discrepancies are reported in section 11 and were not applied.
- Structural and planning checks are recorded in the [P0-013 task record](../tasks/p0-013-compatibility-rights-evidence.md), not duplicated here.

## 14. Source register

All sources were retrieved on 2026-10-31 unless stated otherwise. `S1` is the preserved [Origin feature inventory](../../origin-feature-inventory.md); `S2` is the [DataGraph feature inventory](../../datagraph-feature-inventory.md); `S9` is the [functional scope](../planning/functional-scope.md).

| ID | Source | Version/date | Used for |
|---|---|---|---|
| S-CR-01 | [Origin file types](https://docs.originlab.com/user-guide/origin-file-types) | Live page, retrieved 2026-10-31 | OPJ/OPJU save and read behavior |
| S-CR-02 | [Origin OPJU-to-OPJ FAQ](https://docs.originlab.com/quick-help/convert-opju-project-file-format-to-older-opj/) | Live page | OPJU version compatibility |
| S-CR-03 | [Origin OPJ protection help](https://docs.originlab.com/origin-help/opj-protect/) | Live page | Protected project behavior |
| S-CR-04 | [Origin EULA page](https://www.originlab.com/Index.aspx?go=PURCHASE&pid=1775) | 2026b context; version label not displayed | Reverse engineering and documentation ownership |
| S-CR-05 | [liborigin](https://github.com/SciDAVis/liborigin) | GPL-3.0; last push 2021-12-11 | Reverse-engineered OPJ reader |
| S-CR-06 | [DataGraph 5.4 manual](https://www.visualdatatools.com/DataGraph/Manual.pdf) | 5.4, September 2024 | Package, single file, DTG and legacy Plot |
| S-CR-07 | [DataGraph Saving Files](https://community.visualdatatools.com/datagraph/knowledge-base/saving-files/) | Published 2024-10-21 | `.dgraphs` single-file extension |
| S-CR-08 | [DataGraph 5.5 release](https://community.visualdatatools.com/datagraph/2025/07/11/datagraph-5-5/) | 2025-07-11; HTTP 403, snippet only | Version context |
| S-CR-09 | [DataGraph product page](https://www.visualdatatools.com/DataGraph) | 5.5 page, retrieved 2026-10-31 | SQL/CSV/JSON linking |
| S-CR-10 | [GraphPad PZFX schema FAQ](https://www.graphpad.com/support/faq/prism-xml-style-sheet-and-schema-that-define-the-pzfx-format/) | Live page | PZFX schemas 5-8 |
| S-CR-11 | [Prism PZF vs PZFX guide](https://www.graphpad.com/guides/prism/latest/user-guide/pzf_vs__pzfx_file_format.htm) | Prism 11 guide | PZFX legacy and current `.prism` |
| S-CR-12 | [GraphPad Prism terms](https://www.graphpad.com/docs/Prism-Terms.pdf) | Effective 2026-08-11 in spot check; 2025-06-10 terms reported | Parsing and reverse-engineering restrictions |
| S-CR-13 | [GraphPad product terms](https://www.graphpad.com/company/product-terms) | Live page | Terms transition; no clause text found |
| S-CR-14 | [Origin Prism Connector](https://www.originlab.com/fileExchange/details.aspx?fid=1070) | v1.35, minimum Origin 2026b | Prism connector scope |
| S-CR-15 | [pzfx R package](https://cran.r-project.org/package=pzfx) | 0.3.1, MIT, 2026-01-16 | PZFX read/write implementation |
| S-CR-16 | [Origin LabTalk documentation](https://docs.originlab.com/labtalk/) | Live page | LabTalk language context |
| S-CR-17 | [Origin LabTalk reference](https://docs.originlab.com/labtalk/ref/) | Live page | Commands, functions, objects |
| S-CR-18 | [originlab/labtalk repository](https://github.com/originlab/labtalk) | Documentation-only; no license | Documentation licensing evidence |
| S-CR-19 | [Origin C guide](https://docs.originlab.com/originc/guide/) | Live page | Origin C language and toolchain |
| S-CR-20 | [Origin C basic features](https://docs.originlab.com/originc/guide/basic-features) | Live page | ANSI C basis and Code Builder |
| S-CR-21 | [Origin `xop` X-Function](https://docs.originlab.com/x-function/ref/xop/) | Live page | XOP is an X-Function, not a native SDK |
| S-CR-22 | [Origin Developer Kit](https://docs.originlab.com/originc/odk/) | Live page | Developer kit scope |
| S-CR-23 | [Origin Apps user guide](https://www.originlab.com/doc/User-Guide/Apps) | Live page | Vendor App model |
| S-CR-24 | [Origin App development](https://docs.originlab.com/appdev/app-development/) | Live page | App dependencies and languages |
| S-CR-25 | [Origin App distribution](https://docs.originlab.com/appdev/app-distribution/) | Live page | `.opx` packaging and encryption |
| S-CR-26 | [Origin File Exchange submission guidelines](https://www.originlab.com/fileexchange/SubmissionHelp.aspx) | Live page | App license and authorship |
| S-CR-27 | [Origin Automation Server](https://docs.originlab.com/com/) | Live page | COM version coupling |
| S-CR-28 | [Orglab documentation](https://docs.originlab.com/orglab/orglab/) | Live page | Orglab identity and use |
| S-CR-29 | [Orglab EULA](https://docs.originlab.com/orglab/orglabeula/) | Live page | Competing-use and reverse-engineering bans |
| S-CR-30 | [ECMA-376](https://ecma-international.org/publications-and-standards/standards/ecma-376/) | 5th edition, December 2021 | OOXML formula grammar and functions |
| S-CR-31 | [Microsoft Open XML formulas](https://learn.microsoft.com/en-us/office/open-xml/spreadsheet/working-with-formulas) | Live page | 300+ predefined functions and syntax reference |
| S-CR-32 | [Microsoft Open Specification Promise](https://learn.microsoft.com/en-us/openspecs/dev_center/ms-devcentlp/1c24c7c8-28b0-4ce1-a47d-95fe1ff504bc) | Live page | Patent promise for covered versions |
| S-CR-33 | [Office implementation information](https://learn.microsoft.com/en-us/openspecs/office_standards/ms-oe376/db9b9b72-b10b-4e7e-844c-09f88c972219) | Live page | Implementation notes |
| S-CR-34 | [MAT file versions](https://www.mathworks.com/help/matlab/import_export/mat-file-versions.html) | R2026b | MAT versions and HDF5-based v7.3 |
| S-CR-35 | [MAT-File Format PDF](https://www.mathworks.com/help/pdf_doc/matlab/matfile_format.pdf) | HTTP 403 in retrieval; not inspected; access conditions not established | MAT specification availability |
| S-CR-36 | [Library of Congress MAT format description](https://www.loc.gov/preservation/digital/formats/fdd/fdd000440.shtml) | 2015 | Level 4/5 public documentation history |
| S-CR-37 | [MATLAB Engine API for Python](https://github.com/mathworks/matlab-engine-for-python) | R2026a branch | Engine field-of-use license |
| S-CR-38 | [Wolfram NB format](https://reference.wolfram.com/language/ref/format/NB.html) | Live page | Notebook expression format |
| S-CR-39 | [Wolfram notebook expressions](https://reference.wolfram.com/language/tutorial/NotebooksAsWolframLanguageExpressions.html) | Live page | Notebook as expressions |
| S-CR-40 | [Wolfram Engine terms](https://www.wolfram.com/legal/terms/wolfram-engine.html) | Engine 15.0 reported; version not displayed in spot check | Production, commercial and redistribution limits |
| S-CR-41 | [Wolfram Engine FAQ](https://www.wolfram.com/engine/faq/) | Live page | Free engine scope |
| S-CR-42 | [WSTP](https://www.wolfram.com/wstp/) | Live page | WSTP distribution |
| S-CR-43 | [WSTP introduction](https://reference.wolfram.com/language/tutorial/IntroductionToWSTP.html) | Live page | WSTP protocol context |
| S-CR-44 | [NI Software License Agreement](https://www.ni.com/content/dam/web/pdfs/legal/software_license_agreement_en.pdf) | 370406AE-01, September 2026 | Reverse engineering and file-format discovery ban |
| S-CR-45 | [NI VI Server](https://www.ni.com/docs/en-US/bundle/labview/page/accessing-labview-functionality-through-vi-server.html) | Live page | VI Server invocation |
| S-CR-46 | [NI LabVIEW and macOS compatibility](https://www.ni.com/en/support/documentation/compatibility/18/labview-and-macos-compatibility.html) | Updated 2026-07-28 | Community Edition only on macOS |
| S-CR-47 | [NI Run-Time Engine knowledge article](https://knowledge.ni.com/KnowledgeArticleDetails?id=kA00Z0000019LpbSAE) | Live page; snippet only | Run-Time Engine cannot run VIs |
| S-CR-48 | [R licenses](https://www.r-project.org/Licenses/) | Live page | R GPL-2/GPL-3 |
| S-CR-49 | [R FAQ](https://cran.r-project.org/doc/FAQ/R-FAQ.html) | R 4.6.1 | Distribution and commercial-use statements |
| S-CR-50 | [The R Language Definition](https://cran.r-project.org/doc/manuals/r-release/R-lang.html) | R 4.6.1, 2026-06-24 | Language definition and notice |
| S-CR-51 | [CRAN Rserve](https://cran.r-project.org/web/packages/Rserve/index.html) | 1.8-19.1, 2026-08-29, GPL-2 | Rserve version, license and protocols |
| S-CR-52 | [Rserve documentation](https://www.rforge.net/Rserve/doc.html) | Live page | Protocol overview |
| S-CR-53 | [Rserve technical documentation](https://www.rforge.net/Rserve/dev.html) | Protocol version 0.1-9 described | QAP1 documentation |
| S-CR-54 | [Minitab file types](https://support.minitab.com/en-us/minitab/help-and-how-to/data-input-and-output/minitab-files/minitab-file-types/) | Live page | MPX/MPJ and MWX/MTW scope |
| S-CR-55 | [Minitab subscription agreement](https://www.minitab.com/content/dam/www/en/uploadedfiles/documents/agreements/Minitab-Software-Subscription-Agreement-V01-01-2024-EN.pdf) | V01-01-2024 | Reverse engineering and competing-product terms |
| S-CR-56 | [Origin Minitab Connector](https://www.originlab.com/fileExchange/details.aspx?fid=561) | v3.13 | `.mpx`/`.mwx` connector scope |
| S-CR-57 | [Origin Ivium Connector](https://www.originlab.com/fileExchange/details.aspx?fid=1061) | v1.00, minimum Origin 2026 | `.ids`/`.idf` scope |
| S-CR-58 | [Origin RPC3 Connector](https://www.originlab.com/fileExchange/details.aspx?fid=1060) | v1.00, minimum Origin 2025 | `.rsp` scope |
| S-CR-59 | [Origin DMD Connector](https://www.originlab.com/fileExchange/details.aspx?fid=1059) | v1.00, minimum Origin 2025 | `.dmd` scope |
| S-CR-60 | [NI MTS RPC-III DataPlugin](https://www.ni.com/pdf/manuals/378601a.html) | 21.0, July 2021 | RPC III/RPC Pro format support |
| S-CR-61 | [DEWETRON OXYGEN FAQ](https://www.dewetron.com/news/oxygen-frequently-asked-questions/) | Live page | Proprietary `.dmd` and DLL |
| S-CR-62 | [pyDmdReader](https://github.com/DEWETRON/pyDmdReader) | MIT wrapper | DMD reader API wrapper |
| S-CR-63 | [Microsoft ODBC overview](https://learn.microsoft.com/en-us/sql/odbc/reference/odbc-overview) | Live page | ODBC API scope |
| S-CR-64 | [ODBC specification](https://github.com/microsoft/odbc-specification) | ODBC 4.0, MIT | Normative public API specification |
| S-CR-65 | [Microsoft ADO reference](https://learn.microsoft.com/en-us/office/client-developer/access/desktop-database-reference/microsoft-activex-data-objects-reference) | Access/Office 2013 | Legacy Windows COM interface |
| S-CR-66 | [Origin inventory](../../origin-feature-inventory.md) | Preserved source, hash-checked | `S1` anchors for OPJ/OPJU, languages, engines, Apps and connectors |
| S-CR-67 | [DataGraph inventory](../../datagraph-feature-inventory.md) | Preserved source, hash-checked | `S2` anchors for `.dgraph`, legacy Plot and DTG |
| S-CR-68 | [Functional scope](../planning/functional-scope.md) | Project specification, 2026-10-31 | `S9` compatibility gates, connectors and formula scope |
| S-CR-69 | [Catalog children](../planning/catalog-children.csv) | 172 rows, P0-004 accepted | `EG-CAT-COMPAT` and `EG-CAT-IO` artifact mapping |
| S-CR-70 | [Project file format](../architecture/project-file-format.md) | Chosen architecture, 2026-10-30 | Native container reference |
| S-CR-71 | [Licensing and dependency governance](../engineering/licensing-and-dependencies.md) | 2026-10-31 | Project license policy and dependency review context |
| S-CR-72 | [Prism file compatibility between versions](https://www.graphpad.com/guides/prism/latest/user-guide/compatibility_between_prism_versions.htm) | Prism 11 guide | Prism 11 cannot open Prism 9 and earlier files |

## Appendix A. Acceptance coverage

| Criterion | Where this register satisfies it |
|---|---|
| AC-1 | Section 1.2 maps every named group to CR records; section 5 covers CR-01 to CR-27; section 4 covers the native reference |
| AC-2 | Every CR record has four separate axis rows plus source/version, rights, specification, fixture, uncertainty, owner and next action |
| AC-3 | Every CR record is unresolved; section 6 has no support claim; section 2.3 prohibits claims for `R-BLOCKED` and `R-ADVERSE` |
| AC-4 | Section 9 records external legal review as an open dependency; section 10 names a role and next evidence action for every record |
| AC-5 | The document is new; the only other file changed by this task is the P0-013 record; validator and structural results are recorded there |
