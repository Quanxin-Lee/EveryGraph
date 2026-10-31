# Task Record Template

Copy the structure below into an English kebab-case task file, replace all template values, and add its repository-relative path to the [task board](../planning/task-board.csv). This file is not an active task or evidence of completed work. Follow the [recording workflow](README.md).

```markdown
# Pn-NNN: Task title

## Metadata

- Task ID: Pn-NNN
- Status: planned
- Owner: unassigned
- Record writer: unassigned
- Related requirements: existing IDs, or none with a governance rationale
- Dependencies: existing task IDs, or none
- Created: YYYY-MM-DD
- Updated: YYYY-MM-DD
- Branch / worktree / base revision: actual values, or not applicable with reason
- Authorization: actual requested scope and separately approved actions

## Scope and acceptance

Objective:

Allowed files and interfaces:

Change manifest (AGENTS.md section 0 rule 7; complete it before the first write and extend it only through rules 8 and 9. One row per exact repository-relative path, no directory globs. Only these paths may be modified or committed):

| Path | Action | Basis |
|---|---|---|
| `exact/path` | create / modify | Own task artifact / user instruction for this shared normative path / granted request in <task-id> |

Out of scope:

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | Specific observable result | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|

Record source URLs and exact versions for research findings.
Record existing commit IDs only; otherwise identify uncommitted changes.

## Validation

Working directory, OS, architecture and relevant tool versions:

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|

Checks not run and their reasons:

## Review and disposition

Reviewer and review type (independent or self-review): name the reviewer that did not implement this work; a self-review cannot support acceptance under AGENTS.md section 0 rule 11.
Reviewed files / revision:
Findings, resolutions and recheck evidence:
Acceptance decision, accepting role and date (or pending): self-accepted by this task's executor after the independent review above, or the gate that still blocks it.

## Handoff and remaining work

Commits (each with this task ID in the subject and only manifest paths; state that no commit exists and why when true):
Board sync request for P0-000 (target row, state reached, proving commit; never edit the board yourself):
Last verified state:
Blockers and unblock actions:
Next task / concrete next action:
Files or commands needed to resume:

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
```
