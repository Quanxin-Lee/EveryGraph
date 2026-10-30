# P1-005: Prototype Worker Handshake, Cancellation and Resource Governance

## Metadata

- Task ID: P1-005
- Status: planned
- Owner: EveryGraph Implementation
- Record writer: EveryGraph Implementation
- Related requirements: EG-ADD-018
- Dependencies: P1-004
- Created: 2026-10-30
- Updated: 2026-10-30
- Branch / worktree / base revision: not started; the repository is on `feat/p1-001-qt-baseline` at base `747809e`. A dedicated branch or worktree must be created for this task.
- Authorization: not granted. Required: add worker-protocol code and a test worker executable, CMake targets/tests, failure-injection tests and documentation updates. No Qt, network access, Python runtime, plugin loading, package installation, Git commit, push or publication is authorized.

## Scope and acceptance

Objective: prove the P1 worker handshake and cancellation contract before any Python, plugin, AI or numerical worker depends on it. The prototype uses fake bounded jobs and must show that a worker failure or cancellation cannot publish an inconsistent document revision.

This is a crash-containment and scheduling prototype. Process separation is not an OS sandbox and must not be described as one.

Allowed files and interfaces: worker-protocol headers and sources in `modules/application`, a test worker executable under `tests/` (or a dedicated test helper), CMake target/test registration, `docs/engineering/development.md` and this record. The command kernel from P1-004 may be extended only through its public interfaces.

Out of scope: Python, native plugin loading, AI providers, numerical kernels, a cross-platform process broker, OS sandbox guarantees, resource limits enforced by the operating system, Windows execution, a production scheduler and any claim that untrusted code is contained.

| Criterion | Required evidence | Result |
|---|---|---|
| AC-1 | A versioned worker message protocol defines submit, accepted, progress, cancel, result, failure, superseded and shutdown messages with schema version and task identity | Not run |
| AC-2 | An in-process worker observes a stop token at bounded checkpoints; cancellation returns a structured `canceled` result and releases its reservation | Not run |
| AC-3 | A separate test-worker process demonstrates timeout termination and crash detection; the host reports failure without committing partial output | Not run |
| AC-4 | Progress updates are bounded and coalesced; unknown totals remain indeterminate; duplicate submissions are rejected or superseded deterministically | Not run |
| AC-5 | A simple resource admission check rejects an over-budget job before execution and records a structured `resource_exhausted` outcome | Not run |
| AC-6 | The command kernel publishes no revision or event for canceled, failed, timed-out or superseded work; a previously committed revision remains intact | Not run |
| AC-7 | Failure-injection tests cover cancellation, timeout, child death, stale result after an edit, duplicate submission and bounded queue pressure; the three macOS presets and sanitizer suite pass | Not run |
| AC-8 | Documentation states the macOS-only prototype scope, the test-worker mechanism and the explicit absence of a sandbox or untrusted-code claim | Not run |

## Contributions and changes

| Date | Contributor / role | Actual work and affected files | Evidence |
|---|---|---|---|
| 2026-10-30 | EveryGraph Lead / planner | Created this scoped task from the concurrency and P1 exit contracts; no worker code or test exists | [Takeover audit](p0-002-takeover-state-verification.md), [concurrency model](../architecture/concurrency-model.md) and [board](../planning/task-board.csv) |

No implementation, test worker or failure-injection test exists. This record is a plan, not completed work.

## Validation

Working directory, OS, architecture and relevant tool versions: `/Users/leo/Documents/General/EveryGraph`; macOS 27.0.1 arm64; AppleClang 21.0.0; CMake 3.31.6 and Ninja 1.11.1.4 in `.venv`. The prototype must not require Qt or a network connection.

| Date | Exact command or manual check | Result | Evidence and limits |
|---|---|---|---|
| 2026-10-30 | Existing Qt-free suites before this task | Three presets each 3/3 passed | Baseline only; no worker protocol or test worker exists |
| 2026-10-30 | Source inspection | No worker protocol, test helper or job scheduler exists | Confirms the task has not started |
| 2026-10-30 | None | Not run | Planned task; no failure-injection test exists |

Checks not run and their reasons: worker protocol tests, cancellation, timeout, child-process death, stale-result rejection, queue bounds, resource admission and sanitizer runs. They require the authorized implementation.

## Review and disposition

Reviewer and review type: independent review of the protocol and failure-injection evidence is required. No reviewer has been assigned.

Reviewed files / revision: none.

Findings, resolutions and recheck evidence: none.

Acceptance decision, accepting role and date: pending; the task is planned and not authorized.

## Handoff and remaining work

Last verified state: the concurrency contract is documented but no worker protocol, scheduler or test helper exists. P1-004 is the command/transaction dependency and has not started.

Blockers and unblock actions: P1-004 must establish the command/transaction interfaces. The user must authorize the worker prototype, process-spawn test helper, CMake changes and failure-injection tests. Any Python or plugin worker remains out of scope until its own containment and rights review.

Next task / concrete next action: after P1-004 and authorization, implement the protocol and in-process worker, add the separate test worker, run the failure-injection suite and record the limits. P1-006 consumes the cancellation/progress contract for UI jobs after Qt is pinned.

Files or commands needed to resume: [concurrency model](../architecture/concurrency-model.md), [module design](../architecture/module-design.md), [development guide](../engineering/development.md), `CMakeLists.txt`, `CMakePresets.json`.

## Status and scope changes

| Date | Transition or scope change | Reason / authority |
|---|---|---|
| 2026-10-30 | New -> planned | Second takeover decomposition; P1-004 dependency not started and implementation not authorized |
