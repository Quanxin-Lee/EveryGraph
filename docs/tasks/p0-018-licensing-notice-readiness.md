# P0-018: Prepare the licensing, notice and dependency-compliance evidence

## Metadata

- Task ID: P0-018
- Status: accepted
- Owner: EveryGraph Lead
- Record writer: EveryGraph Lead
- Related requirements: none — distribution and legal-governance work; the rationale is the roadmap Phase-0 exit clause "explicit legal and licensing gap with owner", [AGENTS.md](../../AGENTS.md) section 3 and the [license policy](../engineering/licensing-and-dependencies.md)
- Dependencies: P1-003
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: shared `main` worktree at `97f5f37`; unrelated P1-005 changes are present and remain untouched
- Authorization: this record allocates the task only. The executor edits the manifest paths below and produces a decision package. It does **not** change the project's license, publish a `LICENSE` or `NOTICE` file, install a dependency, contact a lawyer or make a legal conclusion; those are the user's acts under AGENTS.md section 0 rule 11 and the license policy

## Scope and acceptance

Objective: [P0-015](p0-015-p0-exit-re-verification.md) recorded the licensing gap as the one Phase-0 exit item with no owner: no `LICENSE` or `NOTICE` file is committed, and formal release license, distribution notices, corresponding-source access and complete dependency compliance remain distribution gates. [P1-003](p1-003-dependency-pins-sbom.md)'s accepted vcpkg graph and SBOM are input, not clearance. Turn that orphaned gap into an owned, reviewable evidence package: a complete dependency-license inventory derived from the accepted SBOM and the manifest, the concrete obligations each license class imposes, and a proposed notice text — held as a proposal for the user's decision rather than committed as a license.

Allowed files and interfaces: the two manifest paths below. The license policy's existing statements of intent and its Phase-0 status are corrected only where they are factually stale; no obligation is weakened or removed.

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `docs/engineering/licensing-and-dependencies.md` | modify | P0-000 designates P0-018 as the single writer of this shared normative path for this revision (rule 8) |
| `docs/tasks/p0-018-licensing-notice-readiness.md` | modify | Own task record |

Out of scope: creating `LICENSE`, `NOTICE`, `COPYING` or any other license text at the repository root (a rule 9 request to the user, not an edit); changing any dependency, version, license, minimum platform or architecture; `vcpkg.json`, `vcpkg-configuration.json`, the SBOM and the `docs/engineering/sbom/` tree (P1-003's accepted artifacts, read-only here); `.github/workflows/*`; an external legal review, any paid resource and any vendor contact; a conclusion about whether a use is lawful.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A dependency-license inventory covers every component in the accepted SBOM and every manifest dependency, with each row's source being the committed artifact it was read from; the record states the component count and how it reconciles with P1-003's count | Pass: 43 unique SBOM package IDs reconciled as 13 target + 29 host + one probe record; all 43 IDs appear in the guide inventory, and the `qtbase` direct dependency from `vcpkg.json` maps to the target SBOM row |
| AC-2 | Obligations are stated per license class actually present (at minimum the Qt LGPLv3 obligations — dynamic linking, relinking and corresponding-source access — and the permissive notices), each traceable to a cited license text or official page, with the retrieval date recorded | Pass: round-3 independent review confirms the cited summaries and exact identifier links; retrieval date 2026-10-31. Findings F-01 to F-03 were corrected and rechecked |
| AC-3 | EveryGraph-owned code's intended license is restated exactly as the policy states it, with an explicit statement that this task neither chose nor changed it | Pass: proposal repeats the policy's Apache-2.0 intent for EveryGraph-owned source and developer documentation and states P0-018 neither chose nor changed it |
| AC-4 | A proposed `NOTICE` text and a proposed `LICENSE` choice are written **inside this record** as a proposal, not committed as files, with the rule 9 request that names the target paths and asks the user to decide | Pass: both proposals and a user decision request naming `LICENSE` and `NOTICE` are recorded below; neither root file was created |
| AC-5 | The document's Phase-0 status is accurate after the edit: what is evidenced, what remains a distribution gate, and that no legal review has occurred; no support, release-readiness or clearance claim is added | Pass: the guide distinguishes the qualification inventory from a release SBOM/notice bundle and retains legal review, corresponding-source, OpenGL per-file and Qt distribution obligations as open gates; no legal review or clearance is claimed |
| AC-6 | Only the two manifest paths are modified and committed, in rule 10 pathspec form with exactly one task ID in the subject; `tools/validate_task_manifest.py` or the rule 12 self-check is recorded | Pass: the authorized local commit uses the exact two-path manifest and subject `docs(p0-018): record licensing evidence and decision proposal`; cached and committed path lists are verified in Validation. The rule 12 self-check and validator result are recorded; no push occurred |
| AC-7 | `python3 tools/validate_planning.py` passes (the licensing guide is an input to the planning validator, so a factual correction must keep its hashes and links consistent) and the record follows the English policy | Pass: planning validator exits 0 with 339 original IDs, 428 traced requirements and 96 documents |
| AC-8 | An independent reviewer that did not write the inventory checks a sample of rows against the SBOM and confirms AC-3's no-license-change statement | Pass: `p0_018_independent_review`, round 3, verdict PASS with no remaining findings; evidence recorded below |

## Decision proposal — not publication text

The policy states: **“Select Apache-2.0 for EveryGraph-owned source and developer documentation.”** P0-018 repeats that existing intent; this task neither chose nor changed it. The proposed root `LICENSE` choice is Apache License 2.0, using the canonical full text at <https://www.apache.org/licenses/LICENSE-2.0>. Copyright holder names and years are not supplied in this proposal because the rights holders have not confirmed them.

Proposed `NOTICE` draft for the user to review. It describes the P1-003 qualification target graph, not a verified release bundle:

```text
EveryGraph — Third-party notices (proposal; not release-ready)

The following components are present in the P1-003 macOS arm64 Qt qualification target graph. This inventory does not assert that every listed component will be redistributed in a release. Preserve the applicable upstream copyright and license notices and include the relevant license texts with any distribution as required by each license.

- Brotli 1.2.1 — MIT
- bzip2 1.0.8#6 — bzip2-1.0.6
- double-conversion 3.4.0 — BSD-3-Clause
- egl-registry 2025-05-27 — Apache-2.0
- FreeType 2.14.3 — FTL OR GPL-2.0-or-later
- libb2 0.98.1#7 — CC0-1.0
- libpng 1.6.59 — libpng-2.0
- md4c 0.6.0 — MIT
- OpenGL framework wrapper 2022-12-04#3 — NOASSERTION
- OpenGL registry 2026-08-03 — NOASSERTION; per-file licensing remains to be audited
- PCRE2 10.49#2 — BSD-3-Clause WITH PCRE2-exception; bundled sljit component — BSD-2-Clause
- Qt qtbase 6.12.0 — declared alternatives LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only; the project policy proposes the shared-library LGPLv3 route, subject to release verification and review
- zlib 1.3.2#2 — Zlib

The complete release notice bundle must be prepared from the files actually distributed, including exact copyright statements, applicable license texts, Qt source/build information and the sljit notice. This draft is not that bundle.
```

**User decision request (AGENTS.md section 0 rule 9; no out-of-manifest file edit is requested from this executor):**

- Target `LICENSE`: decide whether to confirm Apache-2.0 for EveryGraph-owned source and developer documentation; if confirmed, supply/confirm the copyright holder and year for the full canonical text.
- Target `NOTICE`: decide whether to use this proposal as the starting draft; before publication, reconcile the actual release file closure and add exact upstream copyright statements, license texts and required notices.
- Reason: the repository has no root `LICENSE` or `NOTICE`; the policy is an intent, while formal license publication and complete release notices remain distribution gates.
- Owner: user/project owner. No task is allocated to create or publish either root file, and no legal review has occurred.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated this task and created the scaffold with its two-path change manifest and the writer designation for the licensing guide; the user delegated this allocation decision to P0-000 and no licensing content was written | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md); [P0-015 record](p0-015-p0-exit-re-verification.md) residual table |
| 2026-10-31 | EveryGraph Lead / P0-018 executor | Verified the accepted P1-003 dependency and committed SBOM/manifest inputs; updated the designated licensing guide with SPDX row IDs, manifest cross-check, source-backed license class obligations and open release gates; added the user decision proposal here | [P1-003 record](p1-003-dependency-pins-sbom.md), [licensing guide](../engineering/licensing-and-dependencies.md), and validation below |
| 2026-10-31 | `license_source_research` / read-only researcher | Checked official Qt, GNU, SPDX and upstream license references for the exact expression classes in the accepted SBOM; made no file edits | Source URLs and retrieval date are in the [licensing guide](../engineering/licensing-and-dependencies.md#license-classes-and-text-obligations) |
| 2026-10-31 | User / project owner and EveryGraph Lead / P0-018 executor | User explicitly authorized one local manifest-only commit, with no push; executor accepted P0-018 after independent review and completed the authorized commit | User's explicit authorization in this conversation; exact path and commit verification below |

## Validation

| Date | Command or check | Result | Notes |
|---|---|---|---|
| 2026-10-31 | `python3 tools/validate_planning.py` | Pass, exit 0 | From `/Users/leo/Documents/General/EveryGraph`; 339 original IDs, 428 traced requirements, 96 documents; input hashes, coverage, source anchors, classifications, local links, fences and English policy passed. Remote URL availability, rendering and product behavior are not checked. |
| 2026-10-31 | Initial SBOM inventory one-liner | Failed; quoting-only harness error | An f-string expression escaped double quotes incorrectly and Python returned `SyntaxError`; no repository file was changed. The corrected command below passed. |
| 2026-10-31 | `python3 -c 'import json,re; from pathlib import Path; s=json.loads(Path("docs/engineering/sbom/everygraph-macos-arm64-qt.spdx.json").read_text()); d=Path("docs/engineering/licensing-and-dependencies.md").read_text(); ids={p["SPDXID"] for p in s["packages"]}; refs=set(re.findall(r"SPDXRef-Package-[A-Za-z0-9_.+-]+",d)); v=json.loads(Path("vcpkg.json").read_text()); names={x["name"] for x in v["dependencies"]}; assert len(s["packages"])==43 and ids==refs and names <= {p.get("name") for p in s["packages"]}; print("PASS:",len(s["packages"]),"records,",len(ids),"exact IDs; manifest dependencies:",sorted(names))'` | Pass, exit 0 | 43 SBOM records and 43 exact IDs; none missing or extra; manifest direct dependency is `qtbase`. |
| 2026-10-31 | `python3 -B tools/validate_task_manifest.py` | Exit 1; 35 repository-wide errors and 56 notices | Existing historical findings remain outside P0-018 ownership. The P0-018 manifest has exactly its two declared paths; the tool reports the expected record/board state divergence because only P0-000 may edit the board. Its normative-writer check is not automated. |
| 2026-10-31 | `cmake --version` | Unavailable: `cmake` is not on PATH | Not needed for documentation validation; no application build or test was run. Environment: macOS 27.0.1 arm64, Python 3.14.2, shared `main` worktree at `97f5f37`. |
| 2026-10-31 | Independent read-only review, round 1 | Findings; AC-2 not yet passed | Reviewer `p0_018_independent_review` found F-01: AFL-2.1 section 3 is the Licensor's source-copy commitment; recipient duties include section 6 notice retention/modification attribution and section 9 reasonable assent efforts. F-02: Zlib does not impose the drafted binary-documentation notice requirement; its text protects source-distribution notice and says product-documentation acknowledgment is appreciated, not required. Corrections are pending round-2 recheck. |
| 2026-10-31 | Independent read-only review, round-2 follow-up | One citation finding; AC-2 remains pending | Reviewer `p0_018_independent_review` confirmed F-01/F-02 corrections, inventory IDs and Qt 6.12 LGPL links, and found F-03: the GPL-2.0-or-later URL was invalid. The guide now links to the exact SPDX identifier page; final round-3 recheck is pending. |
| 2026-10-31 | Independent read-only review, round 3 | PASS; no remaining findings | `p0_018_independent_review` verified guide SHA-256 `c0438612fcd2a885bbb8cfca61097c125dc8189f5b53e6ac691e63af51982522` and pre-disposition record SHA-256 `f00bed69dec69490095c02e8d83defeaade968bec4b600092c30c8a1c37f356e`; checked 43/43 SPDX IDs, all 21 license/exception/status tokens, sampled target/host/probe rows, Qt LGPL terms, corrected AFL/Zlib summaries and source links, and no-license-change proposal. |
| 2026-10-31 | `git diff --check -- docs/engineering/licensing-and-dependencies.md docs/tasks/p0-018-licensing-notice-readiness.md` and `git diff --name-only -- docs/engineering/licensing-and-dependencies.md docs/tasks/p0-018-licensing-notice-readiness.md` | Pass | Exactly the two P0-018 manifest paths are modified by this task; unrelated changes remain outside its scope. |
| 2026-10-31 | `git diff --cached --name-only` before commit | Pass | Exactly `docs/engineering/licensing-and-dependencies.md` and `docs/tasks/p0-018-licensing-notice-readiness.md` were staged. |
| 2026-10-31 | `git commit -m "docs(p0-018): record licensing evidence and decision proposal" -- docs/engineering/licensing-and-dependencies.md docs/tasks/p0-018-licensing-notice-readiness.md` | Pass | One local, manifest-only acceptance commit; no push. |
| 2026-10-31 | `git show --name-only --format=fuller HEAD` | Pass | The committed path list contains exactly the two P0-018 manifest paths; the commit subject contains exactly one task ID. |

## Review and disposition

Reviewer and review type: `p0_018_independent_review`, independent read-only review. Round 1 identified F-01 (AFL-2.1 source-delivery scope) and F-02 (Zlib binary-notice claim); exact pre-correction hashes were not captured. Round 2 verified those corrections and found F-03 (invalid GPL-2.0-or-later citation). Round 3 reviewed the corrected candidate at guide SHA-256 `c0438612fcd2a885bbb8cfca61097c125dc8189f5b53e6ac691e63af51982522` and pre-disposition record SHA-256 `f00bed69dec69490095c02e8d83defeaade968bec4b600092c30c8a1c37f356e`; verdict PASS, no remaining findings. The reviewer confirmed all 43 IDs, all 21 license/exception/status tokens, representative target/host/probe rows, Qt LGPL obligations, source links and the unchanged license intent. The task self-accepts after that review; the review preceded only the status, authorization and commit-evidence updates in this record.

## Handoff and remaining work

Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself): set row P0-018 to `accepted`, proven by this manifest-only acceptance commit with subject `docs(p0-018): record licensing evidence and decision proposal`. Resolve its short hash with `git log -1 --format=%h -- docs/tasks/p0-018-licensing-notice-readiness.md` after commit. This executor did not edit the board.

Last verified state: P0-018 is accepted and its local manifest-only commit is verified. The task-board row remains P0-000-owned and awaits the sync request above. Working directory `/Users/leo/Documents/General/EveryGraph`, branch `main`; unrelated task modifications remain untouched.

Blockers and unblock actions: none for the accepted P0-018 evidence task. The separate legal review and the decision to publish a license remain distribution gates outside this task and are recorded as open rather than resolved. P0-000 owns the board update; no push was authorized or performed.

Next task / concrete next action: P0-000 consumes the `Board sync` request and updates the P0-018 row. Keep the root license decision, release notice bundle, corresponding-source delivery, OpenGL per-file audit and legal review open as distribution gates.
