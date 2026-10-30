# P2-001: Define the P2 Data and Persistence Conformance Test Plan

## Metadata

- Task ID: P2-001
- Status: accepted
- Owner: EveryGraph Data
- Record writer: EveryGraph Data / Qoder main-agent session, 2026-10-31 (P2-001 executor)
- Related requirements: ORG-DATA-001; ORG-DATA-011; DG-DATA-005; DG-DATA-006; EG-ADD-002; EG-ADD-008; EG-ADD-010; EG-ADD-013
- Dependencies: none; this is a documentation plan that may start before P1 exits
- Created: 2026-10-31
- Updated: 2026-10-31
- Branch / worktree / base revision: `main` at `d0adda8` plus this task's uncommitted documentation changes. No new branch or worktree was created, per direct user instruction. The concurrent P0-004 and P1-003 uncommitted files belong to their owners and are excluded from this task.
- Authorization: on 2026-10-31 the user instructed the main agent to execute P2-001 on the current branch, assign the required research, implementation and independent review, complete implementation, checks, review and necessary fixes within the authorized scope, update the record and board, and obtain approval before scope expansion, architecture change, system dependency installation, Git commit/push, paid resources or external publication. Authorized writes: create `docs/engineering/data-conformance-plan.md`, edit this record, and sync the P2-001 row of the task board. All other documents, the feature register, architecture contracts, build files and product code are read-only. No Git commit or push is authorized.
- Board note: the P2-001 board row is synchronized by the main agent under the user's direct instruction. It does not allocate task IDs or edit another task's record; P0-000 remains the board owner and may correct the row.

## Scope and acceptance

Objective: turn the P2 data, import, formula and persistence contracts into an executable conformance plan before implementation. The plan defines golden fixtures, expected semantics, failure injection, tolerances and measurement procedures so later P2 tasks do not invent acceptance after the fact.

The deliverable is [data-conformance-plan.md](../engineering/data-conformance-plan.md), version 1.0. It defines the TYP, ID, UNIT, ALIGN, IO, FML, SAVE, REC, MIG and PERF fixture families; fixture manifest and evidence rules; diagnostic conventions; per-family expected results and tolerances; the Save As and recovery injection sequence; the 100MB/1GB measurement procedure; requirement traceability; open decisions; and change control. It explicitly defers operation-specific P2-004, P2-009 and P2-010 command fixtures to those task records and defers package validation (REC-015) to P6.

Allowed files and interfaces: `docs/engineering/data-conformance-plan.md`, this record, and the P2-001 board row status sync authorized above. The [development roadmap](../planning/development-roadmap.md), [feature traceability register](../planning/feature-traceability.csv), [performance protocol](../engineering/performance-validation.md), [concurrency model](../architecture/concurrency-model.md), every other task record, build files and product code are read-only.

Out of scope: writing C++ or Python test code, acquiring or generating fixture data, changing the architecture contracts, changing the 428-row register or validator, acquiring proprietary fixtures, implementing import or storage, and claiming that any P2 capability is delivered.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | The plan defines typed-value fixtures covering numeric, text, date, IEEE binary, missing, NaN, infinite, empty text, signed zero and display-versus-stored precision | Met: plan section 4, TYP-001 to TYP-018; comparison rules in section 3.4 and diagnostics in section 4.3. The plan defines fixtures, not test code |
| AC-2 | The plan defines delimiter, quoting, encoding, header, skip/region, decimal-comma, malformed-row and unsupported-construct fixtures for CSV/TXT/clipboard import with expected diagnostics | Met: plan section 8, IO-001 to IO-030, including atomic rejection, source location, encoding, clipboard and Import Special rules |
| AC-3 | The plan defines row-identity, column-identity, unit/dimension, mirror, mask and join/unmatched-row fixtures with invariant checks | Met: plan sections 5, 6 and 7, ID-001 to ID-013, UNIT-001 to UNIT-013 and ALIGN-001 to ALIGN-018, including same-domain and foreign-domain mirrors and mirror-cycle rejection |
| AC-4 | The plan defines formula DAG, cycle, invalidation, recalculation and stable-error fixtures with deterministic expected results | Met: plan section 9, FML-001 to FML-024, with precedence, typed errors, cycle rejection, invalidation counts, manual mode and registry tolerances |
| AC-5 | The plan defines atomic-save, interrupted-save, recovery, migration and reopen fixtures, including the unsaved recovery window and corruption cases | Met: plan section 10, SAVE-001 to SAVE-017, REC-001 to REC-016 and MIG-001 to MIG-008, with save/Save As and recovery injection points; REC-015 is explicitly deferred to P6 |
| AC-6 | The plan defines 100MB/1GB import and save/reopen measurement procedures, hardware/build disclosure, latency, throughput, peak memory and cancellation checks; no vendor number is used as a budget | Met: plan section 11, PERF-001 to PERF-012 and sections 11.2 to 11.4; procedure only, no measured result or vendor budget |
| AC-7 | The plan and this record pass the planning validator, contain no CJK and make no implementation or parity claim | Met: `python3 tools/validate_planning.py` passes with 64 documents and 0 CJK in the plan and record; the plan's non-claims section and its explicit P6/P3/P4 boundaries avoid implementation or parity claims |

"Met" means the plan contains the required definition. It does not mean a test, fixture, implementation or requirement row exists.

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-31 | P0-000 / allocation executor | Allocated P2-001 and created this initial record scaffold; no plan content was written | [Board](../planning/task-board.csv); [P0-000 record](p0-000-task-allocation.md) |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Claimed the authorized documentation task; read the board, record, data/import/formula/storage/performance contracts and consuming P2 records; confirmed the baseline validator is green with 63 documents | This record; [board](../planning/task-board.csv); `python3 tools/validate_planning.py` |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Wrote the 1.0 conformance plan with ten fixture families, manifest and evidence rules, diagnostic mapping, tolerances and traceability; updated this record and the P2-001 board row | [data-conformance-plan.md](../engineering/data-conformance-plan.md); this record |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Ran the planning validator after the plan (64 documents, links and English policy pass), checked board schema and record paths, and confirmed 0 CJK in both task files | Validation table below |
| 2026-10-31 | Independent reviewer / data-QA review (general-purpose subagent, read-only) | First pass: reviewed the plan, this record, the architecture and performance contracts, the eight requirement rows and the P2 task records; returned REVISE with two blocking, five major and five minor findings | Review section below |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Resolved the review findings in the plan: manifest moved into the authorized `tests/data/fixtures/` tree; Save As backup/apply/replace sequence and S10 to S12 injection points added; lazy checksum validation separated from metadata validation; NaN payload capability made a container obligation; same-domain, foreign-domain and mirror-cycle fixtures added; later-phase fixtures deferred or reworded; diagnostic outer-code mapping, manifest fields, atomic-only import mode, encoding wording, temperature and cancellation open decisions added; traceability corrected | [data-conformance-plan.md](../engineering/data-conformance-plan.md); this record |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Re-ran the planning validator and board checks after the fixes; record status set to `in_review` and the P2-001 board row synchronized to `in_review` pending second-pass verification | Validation table below; [board](../planning/task-board.csv) |
| 2026-10-31 | Independent reviewer / data-QA review (general-purpose subagent, read-only) | Second pass: verified the first-pass fixes and the record evidence; returned REVISE for one new major manifest-ownership defect and six minor findings, with AC-1 to AC-7 otherwise passing | Review section below |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Resolved the second-pass findings: replaced the single shared manifest with task-local fragments merged by the harness; added the `planned` status; mapped every used diagnostic family code; removed the remaining "by default" atomic-import wording; qualified the container bit-pattern claim; recorded Save As ownership as an open P2-006 reconciliation; corrected the review count | [data-conformance-plan.md](../engineering/data-conformance-plan.md); this record |
| 2026-10-31 | Independent reviewer / data-QA review (general-purpose subagent, read-only) | Third pass: verified the second-pass fixes and all acceptance evidence; returned ACCEPT WITH MINOR FINDINGS with no blocking defect; AC-1 to AC-7 all pass | Review section below |
| 2026-10-31 | EveryGraph Data / P2-001 executor | Resolved the third-pass minors: recorded all three review passes, replaced the undefined alignment diagnostic, required distinct manifest fragment filenames for tasks sharing `tests/data/commands/`, and added the final validator evidence; accepted the documentation scope | [data-conformance-plan.md](../engineering/data-conformance-plan.md); this record |

The plan is a specification document. No fixture file, test, module, storage container or benchmark exists as a result of this task.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64 (build 26A434); Python 3.13.9; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. The task is documentation-only and does not require a build or a platform test.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-31 | `python3 tools/validate_planning.py` before P2-001 work | Passed: 339 original IDs; 428 traced requirements; 63 documents; hashes, links, fences, English policy and matrix weights pass | Baseline for this task |
| 2026-10-31 | `python3 tools/validate_planning.py` after the first plan version | Passed: 64 documents; no broken local links; no CJK; input hashes and matrix weights pass | The new plan is included |
| 2026-10-31 | `python3 tools/validate_planning.py` after the review fixes | Passed: 339 original IDs; 428 traced requirements; 64 documents; hashes, coverage, source anchors, links, fences and English policy pass | Final plan candidate for recheck |
| 2026-10-31 | Read-only CSV/record check over `docs/planning/task-board.csv` | 32 rows, 32 unique IDs, all statuses valid, all record paths exist, all dependencies resolve; P2-001 row synchronized to the record | Custom read-only check; dependency cycles were checked by inspection and none were introduced |
| 2026-10-31 | CJK scan of the plan and this record | 0 CJK characters in both files | The validator also enforces the English policy |
| 2026-10-31 | `git status --short`; inspection of changed paths | This task changed only the new plan, this record and the P2-001 board row. The P0-004 and P1-003 files visible in the working tree belong to their owners and were not edited, staged or committed | No Git commit or push; the working tree is shared with concurrent owners |
| 2026-10-31 | `python3 tools/validate_planning.py` after the third-pass fixes | Passed on rerun: 339 original IDs; 428 traced requirements; 64 documents; hashes, coverage, source anchors, links, fences and English policy pass; 0 CJK in the plan and record | The first final attempt hit a transient broken link in the P0-004 record while its owner was rewriting `tools/test_catalog_children.py`. The P2-001 plan and record passed `document_errors` directly, no P0-004 file was edited, and the global run passed after the concurrent file returned. Final acceptance result; remote URLs, Mermaid rendering and scientific semantics are not checked |

Checks not run and their reasons: fixture code, test execution, storage failure injection, import parsing, formula evaluation, 100MB/1GB measurements, Windows execution and proprietary fixture acquisition. They require implementation tasks, dependencies and separate authorization; this task only defines the plan. The validator does not check remote URLs, Mermaid rendering or scientific semantics.

## Review and disposition

Reviewer and review type: independent data/QA review by general-purpose subagents in read-only mode on 2026-10-31, separate from the executor's self-check. Three passes were completed: the first returned REVISE with blocking and major findings, the second returned REVISE with one new major manifest-ownership defect, and the third returned ACCEPT WITH MINOR FINDINGS with no blocking defect. The third-pass minor findings were resolved and revalidated. No fourth review was run because the remaining edits were limited to record wording, one stable diagnostic code and distinct manifest fragment filenames; the final validator run is recorded below.

Reviewed files / revision: the plan at the first candidate (working tree, no commit), this record, the data model, project file format, concurrency model, module design, ADR-0004, performance protocol, functional scope section 3 and the P2-002 to P2-011 records. No Git commit exists for this task.

First-pass verdict: REVISE (2 blocking, 5 major, 5 minor). Second-pass verdict: REVISE (1 new major manifest-ownership defect and 6 minor findings; the first-pass findings were substantially resolved).

| Finding | Severity | Resolution |
|---|---|---|
| The fixture manifest at `tests/fixtures/data/manifest.json` was outside every consumer's allowed files | Blocking | The default manifest is now `tests/data/fixtures/manifest.json` inside the consumer test tree; any shared location outside allowed files requires P0-000 allocation |
| This record still had all AC results as "Not run" and stale allocation text | Blocking | This updated record records the plan, validation and review evidence and moves to `in_review`; final acceptance waits for the third-pass check, which found no blocking defect and whose minor findings are resolved |
| Save As omitted the contract's consistent backup, pinned-revision apply, validation and replace sequence, and had no Save As-specific injection | Major | Plan section 10.1 item 4 now states the full sequence and the no-plain-copy rule; S10 to S12 were added; SAVE-015, SAVE-016 and SAVE-017 cover never-saved, target-space/lock and Save As injection cases |
| Reopen was described as validating all chunk checksums before materialization, which conflicts with lazy access | Major | Plan section 10.1 item 5 separates metadata/reference validation from on-access chunk checksum validation; REC-016 confirms lazy access still detects corruption |
| NaN payload preservation allowed a blanket warning while other rules required bit-exact round-trip | Major | Plan sections 3.4 and 4.2 now require an in-memory payload guarantee, a declared container capability before SAVE-002 acceptance, and a documented limitation instead of a silent relaxation |
| Mirror fixtures did not distinguish same-domain and foreign-domain transitions and had no cycle case | Major | Plan section 7 adds the acyclic mirror invariant and ALIGN-016, ALIGN-017 and ALIGN-018 |
| Later-phase fixtures and operation-specific coverage were assigned to P2 | Major | The consumer table now limits P2-009 and P2-010 to shared invariants; REC-015 is deferred to P6; PERF-006 to PERF-009 were reworded; cancellation ownership is an explicit open decision; operation-specific edit and reshape fixtures remain in their owning records |
| Partial-import mode contradicted the open-decision text; UTF-8 and NUL were stated as absolute policies | Minor | Atomic rejection is the only authorized P2 mode; UTF-8 is the baseline and other encodings are optional capabilities; NUL and control-character handling is an open decision |
| Test-facing diagnostic codes collided with stable application codes | Minor | Plan section 3.5 adds an explicit family-detail to outer-code mapping and a review rule for unmapped codes |
| The manifest lacked status, options, expected status and policy references | Minor | The manifest example and rules now include `status`, `options`, `expected_status` and `policy_refs`; unresolved policies block fixture review |
| The board row was edited despite P0-000 board ownership | Minor | The metadata board note records the direct user instruction, limits the change to the P2-001 row and leaves P0-000 as the board owner |
| Traceability omitted UNIT from ORG-DATA-001 and did not state the EG-ADD-002 plotted-result boundary | Minor | Plan section 13 now includes UNIT and limits EG-ADD-002 to in-memory versions/provenance, with plotted-result traceability deferred to P3/P4 |

Second-pass findings and resolutions:

| Finding | Severity | Resolution |
|---|---|---|
| A single shared manifest at `tests/data/fixtures/manifest.json` would force P2-003, P2-005, P2-006, P2-007, P2-011 and other tasks to write outside their allowed files | Major | Plan section 3.2 now uses task-local manifest fragments inside each task's authorized test tree, merged read-only by the test harness; no task edits another task's fragment, and a shared manifest requires P0-000 allocation and a designated writer |
| The manifest status enum said unresolved fixtures stay `planned` but allowed only `active`, `retired` or `deferred` | Minor | `planned` is now an allowed status |
| Sixteen used diagnostic codes were not in the outer-code mapping | Minor | Plan section 3.5 now maps typed-value, import, formula status, recovery and migration detail codes to the application outer codes, including limit and downgrade cases |
| IO-017 and IO-024 still said "by default" after atomic rejection became the only authorized P2 import mode | Minor | Both fixtures now state atomic rejection without the qualifier |
| The review summary said four minor findings but the table has five | Minor | The count now says five minor findings |
| Section 4.1 stated unconditionally that IEEE bit patterns survive the native container, while NaN payload preservation is conditional on the declared container capability | Minor | Section 4.1 item 4 now scopes the container claim to the declared payload capability and refers to section 3.4 |
| P2-006 acceptance criteria do not name Save As although the plan assigns Save As fixtures to P2-006 | Minor | The plan records this as an open P2-006 reconciliation item and explicitly does not amend P2-006's scope; the record handoff repeats it |

Third-pass verdict: ACCEPT WITH MINOR FINDINGS (no blocking defect).

| Third-pass finding | Severity | Resolution |
|---|---|---|
| The review paragraph and two resolution rows still referred to a pending second pass | Minor | The reviewer paragraph now records all three passes; the resolution row and status history refer to the third pass |
| ALIGN-006 used the undefined phrase "alignment error" instead of a stable diagnostic | Minor | ALIGN-006 now requires `data.unmapped_row` naming both lengths |
| P2-004, P2-009 and P2-010 share the authorized `tests/data/commands/` directory without distinct fragment filenames | Minor | Plan section 3.2 now names distinct filenames such as `manifest-p2-004.json` and `manifest-p2-009.json` |
| The recorded validator evidence predated the second-pass edits | Minor | The final validator row below was added after the third-pass fixes and is the acceptance result |

Acceptance decision, accepting role and date: accepted by EveryGraph Data / main agent on 2026-10-31 after three independent read-only review passes. All blocking and major findings are resolved; the third-pass minor findings are resolved and the final validator run is green. Acceptance covers the documentation scope only. It does not accept P2 implementation, P2-006, PERF-009 cancellation, any requirement row or any phase gate.

## Handoff and remaining work

Last verified state: the 1.0 conformance plan exists, the planning validator passes with 64 documents and no CJK, this record is `accepted`, and the P2-001 board row is `accepted`. Three independent read-only reviews were completed. No P2 code, fixture, test or benchmark exists. No commit or push occurred.

Blockers and unblock actions: P2 implementation remains blocked on the P1 exit gate (P1-013); this plan does not authorize code. The plan's open decisions in section 14 must be resolved by their owners before the affected fixtures can be accepted. P2-006 must reconcile the Save As fixtures with its own acceptance criteria or P0-000 must allocate a follow-up; P0-000 must also settle cancellation checkpoint ownership for PERF-009. The Windows conformance run and the actual 100MB/1GB measurements remain unexecuted.

Next task / concrete next action: P2-002 creates its own manifest fragment and implementation within its own authorized scope after its dependencies and authorization are settled. P2-006 and P0-000 resolve the Save As and cancellation ownership items recorded in plan section 14.

Files or commands needed to resume: [data-conformance-plan.md](../engineering/data-conformance-plan.md), [data model](../architecture/data-model.md), [project file format](../architecture/project-file-format.md), [concurrency model](../architecture/concurrency-model.md), [performance protocol](../engineering/performance-validation.md), [task recording guide](README.md), `python3 tools/validate_planning.py`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-31 | New -> planned | Allocated by P0-000 as a startable documentation task; code implementation remains gated by P1-013 |
| 2026-10-31 | planned -> in_progress | Direct user instruction to execute P2-001 on the current branch within the record's documentation scope |
| 2026-10-31 | in_progress -> in_review | The plan, validation evidence and first-pass review resolutions are ready; second-pass verification is pending |
| 2026-10-31 | Board row synchronized to `in_review` | Direct user instruction to update the board; only the P2-001 row changed |
| 2026-10-31 | Second-pass review returned REVISE; no status change | One major manifest-ownership defect and six minor findings; the plan and record were corrected and a third-pass verification was requested |
| 2026-10-31 | Third-pass review returned ACCEPT WITH MINOR FINDINGS; no status change | No blocking defect; four minor record, diagnostic and fragment-naming findings were resolved and the final validator run is green |
| 2026-10-31 | in_review -> accepted | All seven documentation criteria are met, three independent read-only reviews were completed with findings resolved, and the final validator passes with 64 documents. The board row was synchronized to `accepted`; no implementation or requirement row is accepted |
