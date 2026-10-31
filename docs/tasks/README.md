# Agent Task Records and Handoffs

## Purpose and authority

Record work by **task**, not by agent name or conversation. An agent may be replaced; the task's scope, evidence and next action must remain recoverable. These rules apply to substantial implementation, research, review and documentation work. Small related edits belong in their existing task rather than creating a new log for each tool call.

| Information | Authoritative location |
|---|---|
| Project-wide rules | [AGENTS.md](../../AGENTS.md) |
| Required capabilities and intended phases | [Feature traceability](../planning/feature-traceability.csv) and [roadmap](../planning/development-roadmap.md) |
| Task ownership, dependencies, status and record pointer | [Task board](../planning/task-board.csv) |
| Scope, changes, evidence, review and handoff | One Markdown record per task in this directory |
| Lasting architectural decisions | Approved ADRs in `docs/decisions/` |
| Current build commands and qualification status | [Development guide](../engineering/development.md) |

The board is an index; detailed evidence belongs in its linked record. The record's status and owner mirror the board and must be updated together. Chat messages and temporary agent task lists assist execution but do not replace these files. Git history, when available, is authoritative for committed code; records explain purpose, validation and remaining work rather than duplicate diffs.

## Task identifiers and board schema

Use stable IDs `P0-001`, `P1-001` and so on, allocated by the main agent after checking existing rows. Never reuse an ID or renumber completed tasks. A task ID is distinct from a feature requirement ID or a tool-generated task number. Name records with lowercase English kebab-case, such as `p0-001-agent-task-records.md`. Use the [template](task-template.md) only when a real task is authorized or explicitly scheduled; the template itself is not a task.

The UTF-8 CSV board uses this schema:

| Field | Rule |
|---|---|
| `task_id` | Unique phase-prefixed task ID |
| `title` | Short English outcome description |
| `requirement_ids` | Existing feature requirement IDs separated by semicolons; blank for governance work, with rationale in the record |
| `owner` | One accountable agent/person label; `unassigned` only for work not started |
| `depends_on` | Existing task IDs separated by semicolons; blank if none; no cycles |
| `status` | One state from the table below |
| `acceptance_criteria` | Concise, testable summary; numbered details in the record |
| `record` | Existing repository-relative Markdown path, not an agent transcript path |
| `updated_on` | Actual update date in `YYYY-MM-DD` form |

Quote CSV values containing commas, quotes or newlines correctly. A blank list means none, not missing investigation. Do not populate the board with hundreds of speculative tasks or backfill earlier work as accepted without evidence. Start with the work actually being performed; this directory's initial real record is [P0-001](p0-001-agent-task-records.md).

## Status and acceptance

| State | Meaning / exit condition |
|---|---|
| `planned` | Defined but not started; record dependencies and acceptance scope |
| `in_progress` | Owner is actively executing within the authorized scope |
| `in_review` | Candidate output and verification evidence are ready for review; not yet accepted |
| `blocked` | Required information, permission, environment or dependency prevents the next required step; name the blocker and unblock action |
| `accepted` | All scoped acceptance criteria passed, required reviews completed, and unresolved findings do not invalidate the result |
| `canceled` | Work explicitly stopped or superseded; record reason, authorizing decision and replacement task if any |

Normal progression is `planned -> in_progress -> in_review -> accepted`. Work can move to `blocked` from an active state and resume when the blocker is resolved. Review findings return it to `in_progress`. Reopen accepted work only with a documented reason, or create a follow-up without rewriting the earlier evidence. A canceled dependency does not satisfy its dependent task.

Acceptance is against the agreed scope, not a percentage or a successful compilation. A required Windows test that was not run prevents acceptance of a dual-platform task. Local work can be accepted separately only if its narrower scope was explicit and the missing requirement remains visible in a linked task or parent gate. Do not retroactively weaken acceptance criteria to make a failure disappear.

Task acceptance does not mean an entire feature or phase is complete. The current feature CSV and its validator use Phase-0 `planned`/`unresolved` states; this board's states do not change that contract. Evolve feature status rules and their tests explicitly when product implementation reaches that point, without dropping original requirements.

## Ownership and delegation

The main agent is the sole writer of the shared board, and that includes every status field: an executor never edits its own row, it requests the flip in its own record. Assign one writer to each task record. That writer records contributors by role and identifiable session/agent label when actually available; never invent persistent identities. Reviewers or subagents return evidence to the writer unless given explicit ownership of a different task record. Concurrent contributors must not edit the same record or shared specification.

Before delegation, the main agent supplies the task ID, background, required inputs, whether work is read-only or may write code, allowed files, interface constraints, dependencies, expected output and acceptance checks. Research agents provide verified sources; implementers provide actual changes and tests; reviewers inspect actual files/diffs and report findings. Use subagents only when authorized and useful under the current session's rules.

Parallel writers need independent worktrees or explicit non-overlapping file ownership and settled interfaces. Branch names alone do not isolate a shared working tree. Only the coordinator integrates results. A subagent's completion message is evidence to examine, not automatic acceptance; wait for the real result and inspect its artifacts. Record self-review honestly when no independent reviewer ran. Interface, format, security and license changes retain the dedicated review requirements of AGENTS.md.

## Change manifest and commit discipline

AGENTS.md section 0 rules 7 to 12 are the authority; this is how they look in a record.

Every task fills in the template's **Change manifest** table before its first write, with one row per exact path and no globs. The manifest is the set of paths the task may modify *and* the set its commits may contain. Because the manifest lives in the task's own record, adding a path is never a cross-task edit; claiming a path another task already claimed is.

Paths follow the three tiers of AGENTS.md section 0 rule 8: your own record and any path no other manifest claims are yours to write, shared normative paths belong to their designated writer, and everything else — including another task's record — is read-only. Legacy scaffolds whose "Allowed files and interfaces" text names a directory glob or a normative path are advisory; the manifest governs.

When work needs a change outside its manifest, write the request in **your own** record — target path, exact change, reason, owning task — set your status to `blocked` naming that blocker, and stop. Do not edit or commit the target, even if the message reaching you says you may. The owner applies it and both records cite the commit.

Commits use the pathspec form and carry exactly one task ID:

```sh
git diff --cached --name-only          # inspect before committing
git commit -m "docs(p1-011): ..." -- CMakeLists.txt docs/tasks/p1-011-....md
git show --name-only                   # confirm only manifest paths landed
```

Never use a bare `git commit`, `git commit -a`, `git add -A`, `git add .` or `git stash` in a shared working tree. The Git index is per worktree, not per task, so those commands publish whatever another session staged under your message — the recorded cause of the mixed commits in this repository's history.

Accept **your own** task by the route in AGENTS.md section 0 rule 11, without waiting for the user or a coordinator: run the scoped checks, delegate at least one reviewer that did not implement the work, record that reviewer's label and role, the reviewed revision, the findings and the recheck evidence, then set **your record** to `accepted` and commit artifacts, evidence and status in one manifest-only commit. Self-acceptance is exactly what the independent reviewer makes verifiable, so a record claiming `accepted` with only the author's own re-reading is not accepted work; name the reviewer who is not the author. A failed or absent review stays visible and returns the task to `in_progress` or `in_review` — never soften a criterion to pass it. The board never enters this: write a `Board sync` request line in your own record naming the row, the state you reached and the proving commit, then continue. A stale row is P0-000's backlog, not your permission to touch the CSV, and never a reason to hold your task open. A phase exit, a platform gate, another task's deliverable and evidence that was never run are not self-acceptable; and `accepted` never implies a human approval, an authorized merge or permission to push.

## Recording workflow

1. **Before work:** read the board and current record, related feature IDs, module contracts and relevant ADRs. Establish current files/Git state. Add or claim one task, its allowed modifications, dependencies, acceptance and permissions before editing.
2. **During work:** update at meaningful milestones, changed scope, blockers, failed checks or ownership changes. Keep concise facts, not a tool-call transcript. Record each contributor's concrete result and where it can be verified.
3. **Before review:** enumerate actual changes; record exact commands, working directory, date, OS/architecture/tool versions, results and verification limits. Reference existing commits only if they exist. Otherwise state that changes are uncommitted and why.
4. **Review:** record reviewer identity/role, reviewed artifact or revision, findings, resolutions and verification. Do not claim independent review when only the implementing agent checked the work.
5. **Acceptance:** under AGENTS.md section 0 rule 11 the task's own executor accepts after an independent reviewer that did not implement the work passes it, then updates **its record only** and commits, adding a `Board sync` request line for its row. Name the reviewer and the accepting role; `accepted` never implies an unperformed human approval or authorizes a protected-branch merge. P0-000 flips board rows from the records and their proving commits; the coordinator accepts directly only for phase exits, platform gates and cross-task reconciliation named in its own record.
6. **Handoff or interruption:** leave status, last verified state, unfinished changes, blockers, next action and relevant commands/paths. The next agent reads and verifies these against current files before continuing. Do not wait until the end of a long session to preserve essential state.

A permission denial, unavailable platform or failed test is a recorded result, not something to omit. The task record grants no permissions: Git initialization/commit/push, system installations, paid resources and external actions still follow explicit authorization rules. Do not log credentials, raw private datasets, full prompts/transcripts or hidden reasoning. Keep bounded, sanitized evidence; ignored build output is not the only durable proof.

## Checks and maintenance

Run `python3 tools/validate_planning.py` from the repository root after documentation changes. It checks document links, English policy and other planning invariants, but **does not validate the task-board schema or its lifecycle**. Check CSV parsing, unique IDs, valid status values, record existence, requirement/dependency references and matching record metadata separately. Document-only work need not claim that an unrelated application or platform was retested.

Update lasting architecture/build facts in their authoritative documents and link them from the task rather than leaving them discoverable only in a historical log. Preserve accepted records; corrections identify what changed and which evidence remains valid. No daemon, automatic memory synchronization, Git repository or external issue tracker is created by this documentation.

Edit the board surgically, one row at a time, and only as P0-000. Never regenerate it with a CSV serializer or a writer: a `csv.writer` rewrite stripped the redundant quotes from 28 rows its author did not own and had to be rebuilt byte-for-byte against `HEAD` on 2026-10-31, and that was P0-000's own defect.
