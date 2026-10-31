"""Check the AGENTS.md section 0 manifest, path-ownership and commit-scope rules.

Section 0 rules 7 to 12 bind every task record and every commit: an active task
declares the exact paths it writes, one path has one claiming manifest, a shared
normative path belongs to its designated writer, and a commit carries exactly one
task ID plus only that task's manifest paths. This tool reads the task board and
the task records with the repository Python, fetches nothing, writes nothing, and
reports each violation scoped to its task or commit.

A record that lags its board row is P0-000 coordination backlog rather than a
defect in the task that noticed it, so record-versus-board divergence and the
waiting ``Board sync`` requests are reported as notices. Notices never change the
exit code and this tool never edits the board.

Exit codes: 0 when nothing reached error level, 1 when at least one finding is an
error, and argparse's own 2 for a bad command line. A repository that passes is a
repository whose manifests and commits are in order, not a repository whose work is
accepted: this tool checks the discipline of who may write what, not scientific or
product quality.
"""
import argparse
import csv
import fnmatch
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BOARD_PATH = "docs/planning/task-board.csv"
TASKS_DIR = "docs/tasks"
ALLOCATION_OWNER = "P0-000"
ALLOCATION_BASELINE = "02648bb"
VALID_STATUSES = ("planned", "in_progress", "in_review", "blocked", "accepted", "canceled")
MANIFEST_REQUIRED_STATUSES = ("in_progress", "in_review", "accepted")
ERROR = "error"
NOTICE = "notice"

NORMATIVE_PATHS = frozenset({
    "AGENTS.md",
    BOARD_PATH,
    "docs/planning/feature-traceability.csv",
    "CMakeLists.txt",
    "CMakePresets.json",
    "docs/engineering/development.md",
    "docs/engineering/licensing-and-dependencies.md",
    "tools/validate_planning.py",
})
NORMATIVE_PREFIXES = (".github/workflows/",)
NORMATIVE_CASED = frozenset(name.casefold() for name in NORMATIVE_PATHS)
# One representative path under each normative prefix, so a pattern row is tested against the
# directory rule 8 protects even though its file names are not known to this tool.
NORMATIVE_MATCHES = (*sorted(NORMATIVE_PATHS), ".github/workflows/ci.yml")

TASK_ID_TEXT = re.compile(r"P\d+-\d{3}\Z")
SUBJECT_SCOPE = re.compile(r"^\s*[A-Za-z][A-Za-z0-9_-]*(?:\(([^)]*)\))?!?:")
# The manifest table's three columns, by the heading names records actually use for them.
PATH_HEADERS = frozenset({"path", "file", "target"})
ACTION_HEADERS = frozenset({"action", "operation", "change"})
BASIS_HEADERS = frozenset({"basis", "reason", "why", "authority"})
SCOPE_TASK_IDS = re.compile(r"P\d+-\d{3}")
RECORD_NAME = re.compile(r"[pP]\d+-\d{3}-[a-z0-9][a-z0-9._-]*\.md\Z")
MANIFEST_HEADING = re.compile(r"^\s{0,3}(?:#{1,6}\s*)?\**\s*Change manifest", re.IGNORECASE)
TABLE_ROW = re.compile(r"^\s*\|(.*)\|\s*$")
SEPARATOR_ROW = re.compile(r"^\s*\|(?:\s*:?-{3,}:?\s*\|)+\s*$")
METADATA_STATUS = re.compile(r"^- Status:\s*(.+?)\s*$", re.MULTILINE)
METADATA_TASK_ID = re.compile(r"^- Task ID:\s*(.+?)\s*$", re.MULTILINE)
REVIEWER_LINE = re.compile(r"^[ \t]*(?:[-*][ \t]*)?\**[ \t]*Reviewer(?:[ \t]+and[ \t]+review[ \t]+type)?"
                           r"(?:[ \t]*\([^)]*\))?[ \t]*\**[ \t]*[:\-][ \t]*(.+?)[ \t]*$",
                           re.IGNORECASE | re.MULTILINE)
BOARD_SYNC_LINE = re.compile(r"^[ \t]*(?:[-*][ \t]*)?\**[ \t]*Board sync request(?:[ \t]+for[ \t]+P\d+-\d{3})?"
                             r"(?:[ \t]*\([^)]*\))?[ \t]*\**[ \t]*[:\-][ \t]*(.+?)[ \t]*$",
                             re.IGNORECASE | re.MULTILINE)
BOARD_SYNC_HEADING = re.compile(r"^[ \t]*(?:[-*][ \t]*)?\**[ \t]*Board sync request.*[:\-][ \t]*\**[ \t]*$",
                                re.IGNORECASE | re.MULTILINE)
INDEPENDENT = re.compile(r"\b(?:independents?|separate|disinterested|non-author|not the (?:writer|author|implementer))\b",
                         re.IGNORECASE)
ABSENT_REVIEW = re.compile(r"\bno\s+reviewer\s+(?:has|have|was|were|is|are|remains|will|shall)\b"
                           r"|\bno\s+review\s+(?:has|have|was|were|is|are|had)\s+(?:been\s+)?"
                           r"(?:run|ran|performed|conducted|done|taken\s+place)\b", re.IGNORECASE)
LOCAL_DENIAL = re.compile(r"\b(?:nobody|no one)\b"
                          r"|\b(?:not|never|nor)\s+(?:\w+\s+){0,2}(?:run|ran|performed|perform|conduct|conducted|"
                          r"claimed|done)\w*\b|\b(?:is|are|was|were|will\s+be|shall\s+be|remains)\s+required\b",
                          re.IGNORECASE)
NEGATION = re.compile(r"\b(?:not|no|never|without|neither|only|lack|lacks|lacking)\b", re.IGNORECASE)
DESIGNATED_BASIS = re.compile(r"\b(?:P0-000|designated|designation|direct user instruction)\b", re.IGNORECASE)
BASIS_NEGATION = re.compile(r"\b(?:not|no|never|without)\b", re.IGNORECASE)
STATUS_WORD = re.compile(r"\b(?:planned|in_progress|in_review|blocked|accepted|canceled)\b", re.IGNORECASE)
TRANSITION_STATE = re.compile(r"\bto[^A-Za-z_]{0,4}\b(planned|in_progress|in_review|blocked|accepted|canceled)\b",
                              re.IGNORECASE)
COMMIT_HASH = re.compile(r"\b[0-9a-f]{7,40}\b")
RECORD_MARK = "\x01"
SENTENCE_BREAK = re.compile(r"[.;:]")
GLOB_CHARS = "*?["
PLACEHOLDER = re.compile(r"<[^>]*>")
HEADER_SCAN_LIMIT = 20


def normalize_path(raw):
    """Clean one manifest cell without touching its glob characters.

    Backticks carry no path meaning, so they are removed wherever they sit; `**bold**`
    wrappers are peeled until the cell stops changing, because a record can nest them in
    either order. Stripping a bare `*` would turn `modules/**` into an exact claim and hide
    the claim rule 7 dislikes, so a cell whose inner text still holds an asterisk stays a
    pattern.
    """
    path = str(raw).strip().replace("`", "").strip()
    for _ in range(4):
        bold = re.fullmatch(r"\*\*(\S[^*]*)\*\*", path)
        peeled = bold.group(1).strip() if bold else path
        if peeled == path:
            break
        path = peeled
    return path.replace("\\", "/").removeprefix("./")


def is_pattern(path):
    return any(char in path for char in GLOB_CHARS)


def pattern_glob(path):
    return PLACEHOLDER.sub("*", path)


def split_cells(line):
    match = TABLE_ROW.match(line)
    if match is None:
        return []
    return [cell.strip() for cell in match.group(1).split("|")]


def header_columns(cells):
    """Identify a manifest table's columns by heading name, accepting the record's synonyms.

    A `## Change manifest` table whose first column is headed `File` or `Target` rather than
    `Path` still declares writable paths, so reading only the literal word `Path` would let a
    record hide a rule 8 claim behind a heading synonym.
    """
    lowered = [cell.strip("* `").lower() for cell in cells]
    path_index = next((index for index, name in enumerate(lowered) if name in PATH_HEADERS), None)
    action_index = next((index for index, name in enumerate(lowered) if name in ACTION_HEADERS), None)
    if path_index is None or action_index is None:
        return None
    basis_index = next((index for index, name in enumerate(lowered) if name in BASIS_HEADERS), None)
    return {"path": path_index, "action": action_index, "basis": basis_index}


def table_cell(cells, index):
    return cells[index].strip() if index is not None and len(cells) > index else ""


@dataclass(frozen=True)
class ManifestEntry:
    path: str
    action: str
    basis: str

    @property
    def key(self):
        """The comparison form, so `./`, `..` or a leading slash cannot dodge a normative or duplicate claim.

        Lexical only: no filesystem call, so a path that escapes the repository root keeps its
        literal text and is reported as not repository-relative instead of silently resolving.
        """
        candidate = os.path.normpath(self.path.lstrip("/")).replace("\\", "/")
        return self.path if candidate.startswith("..") else candidate

    @property
    def exact(self):
        return bool(self.path) and not is_pattern(self.path)

    @property
    def claim_key(self):
        """One file is one claim, even when two tasks spell its path in different cases."""
        return self.key.casefold()

    @property
    def canonical(self):
        return self.path == self.key

    @property
    def repository_relative(self):
        return not self.path.startswith("/") and not self.key.startswith("..")


@dataclass
class TaskRecord:
    task_id: str
    relative: str
    status: str
    manifest: list = field(default_factory=list)
    reviewer: str = ""
    board_sync: list = field(default_factory=list)
    board_sync_unfilled: bool = False

    @property
    def has_manifest(self):
        return bool(self.manifest)

    def exact_paths(self):
        return [entry.key for entry in self.manifest if entry.exact]

    def patterns(self):
        return [entry.path for entry in self.manifest if entry.path and not entry.exact]

    def match_kind(self, path):
        """Return how a path is covered: `record`, `exact`, the pattern, or None."""
        if path == self.relative:
            return "record"
        if any(entry.exact and entry.key == path for entry in self.manifest):
            return "exact"
        for pattern in self.patterns():
            if fnmatch.fnmatchcase(path, pattern_glob(pattern)):
                return pattern
        return None

    def allows(self, path):
        return self.match_kind(path) is not None


@dataclass(frozen=True)
class BoardRow:
    task_id: str
    status: str
    record: str
    owner: str = ""
    title: str = ""


@dataclass(frozen=True)
class Commit:
    sha: str
    subject: str
    paths: tuple = ()
    merge: bool = False

    @property
    def short(self):
        return self.sha[:7]

    def task_ids(self):
        scope = SUBJECT_SCOPE.match(self.subject)
        if scope is None or scope.group(1) is None:
            return []
        return SCOPE_TASK_IDS.findall(scope.group(1).upper())


@dataclass
class History:
    commits: list = field(default_factory=list)
    committed_paths: frozenset = frozenset()
    dirty_paths: frozenset = frozenset()
    untracked_paths: frozenset = frozenset()
    baseline: str = ""
    cutoff: int = 0
    unavailable: str = ""
    order: dict = field(default_factory=dict)
    after: frozenset = None

    def after_baseline(self, commit):
        """Whether the baseline is missing from this commit's ancestry, so the rules applied to it.

        An ancestry set beats a position in the log, because a branch that forked before the
        baseline and merged after it would otherwise be read as an older commit than it is.
        """
        if self.after is not None:
            return commit.sha in self.after
        return self.order.get(commit.sha, self.cutoff) >= self.cutoff

    def by_task(self, task_id):
        return [commit for commit in self.commits if task_id in commit.task_ids()]

    def touched(self, path):
        return [commit for commit in self.commits if path in commit.paths]


def build_history(commits, committed_paths=(), dirty_paths=(), untracked_paths=(), baseline="", cutoff=0,
                  after=None):
    items = list(commits)
    return History(items, frozenset(committed_paths), frozenset(dirty_paths), frozenset(untracked_paths),
                   baseline, cutoff, "", {commit.sha: index for index, commit in enumerate(items)},
                   None if after is None else frozenset(after))


@dataclass(frozen=True)
class Finding:
    level: str
    code: str
    scope: str
    message: str

    def render(self):
        return f"{self.level.upper():<6} {self.code:<36} {self.scope:<10} {self.message}"


def run_git(root, arguments):
    command = ["git", "-c", "core.quotepath=false", *arguments]
    try:
        result = subprocess.run(command, cwd=str(root), capture_output=True, text=True, timeout=120)
    except (OSError, subprocess.SubprocessError) as error:
        return None, f"git {' '.join(arguments)}: {type(error).__name__}: {error}"
    if result.returncode != 0:
        return None, f"git {' '.join(arguments)}: {result.stderr.strip() or f'exit {result.returncode}'}"
    return result.stdout, ""


def parse_manifest(text):
    """Read every manifest table in a record, in document order.

    The heading is accepted as any Markdown heading level or as the template's bold
    lead-in, and a record that splits its paths over a second table is read whole
    rather than up to its first heading. A row whose path cell holds a glob or a
    `<placeholder>` stays a pattern instead of becoming an exact claim.
    """
    lines = text.splitlines()
    entries, heading_found = [], False
    for index, line in enumerate(lines):
        if not MANIFEST_HEADING.match(line):
            continue
        heading_found = True
        columns, header_row = manifest_table(lines, index)
        if columns is None:
            continue
        for offset in range(header_row + 1, len(lines)):
            if SEPARATOR_ROW.match(lines[offset]):
                continue
            cells = split_cells(lines[offset])
            if not cells or len(cells) <= columns["path"]:
                break
            path = normalize_path(cells[columns["path"]])
            if not path or path.lower() == "path":
                continue
            entries.append(ManifestEntry(path, normalize_path(table_cell(cells, columns["action"])).lower(),
                                         table_cell(cells, columns["basis"])))
    return heading_found, entries


def manifest_table(lines, heading_index):
    """Return the manifest table columns and header row after one heading, or None."""
    for offset in range(heading_index + 1, min(heading_index + HEADER_SCAN_LIMIT, len(lines))):
        columns = header_columns(split_cells(lines[offset]))
        if columns is not None:
            return columns, offset
    return None, -1


def metadata_lines(text):
    """The record with fenced code blocks removed, so a quoted template is not its own metadata.

    A record that shows the template's `- Status: planned` line or a manifest example inside a
    fence is demonstrating a form, not declaring its state, its ID or a claim on a path. Reading
    those lines as the record's own would accuse an innocent task of two statuses or of a second
    manifest row, so fenced regions are blanked before any line pattern runs.
    """
    return re.sub(r"^(```|~~~)[^\n]*\n.*?^\1[ \t]*$", " ", text, flags=re.M | re.S)


def read_text_file(path):
    """Return the text of one repository file, or None plus the reason it is unreadable."""
    try:
        return path.read_text(encoding="utf-8"), ""
    except (OSError, UnicodeDecodeError) as error:
        return None, f"{type(error).__name__}: {error}"


def read_board(root):
    path = root / BOARD_PATH
    if not path.is_file():
        return {}, [Finding(ERROR, "BOARD_MISSING", "-", f"{BOARD_PATH} is not readable")]
    text, reason = read_text_file(path)
    if text is None:
        return {}, [Finding(ERROR, "BOARD_UNREADABLE", "-", f"{BOARD_PATH} could not be decoded: {reason}")]
    rows = list(csv.DictReader(text.splitlines(True)))
    board, findings, seen = {}, [], set()
    for number, row in enumerate(rows, 2):
        task_id = (row.get("task_id") or "").strip()
        status = (row.get("status") or "").strip()
        record = normalize_path(row.get("record") or "")
        if not TASK_ID_TEXT.match(task_id):
            findings.append(Finding(ERROR, "BOARD_ROW_IDENTITY", f"line-{number}", f"row has task_id `{task_id}`"))
            continue
        if task_id in seen:
            findings.append(Finding(ERROR, "BOARD_ROW_IDENTITY", task_id, "the task_id appears in more than one board row"))
            continue
        seen.add(task_id)
        if status not in VALID_STATUSES:
            findings.append(Finding(ERROR, "BOARD_ROW_IDENTITY", task_id,
                                    f"board status `{status or 'empty'}` is not one of {', '.join(VALID_STATUSES)}"))
        if not record or not (root / record).exists():
            findings.append(Finding(ERROR, "RECORD_FILE_MISSING", task_id,
                                    f"the board points at `{record or 'nothing'}`, which is not a readable file"))
        board[task_id] = BoardRow(task_id, status, record, (row.get("owner") or "").strip(), (row.get("title") or "").strip())
    return board, findings


def read_records(root, board):
    findings, records = [], {}
    unreadable = set()
    wanted = {row.record: row.task_id for row in board.values() if row.record}
    directory = root / TASKS_DIR
    if not directory.is_dir():
        return {}, [Finding(ERROR, "TASKS_DIR_MISSING", "-", f"{TASKS_DIR} is not readable")]
    for path in sorted(directory.glob("*.md")):
        relative = f"{TASKS_DIR}/{path.name}"
        task_id = wanted.get(relative)
        if task_id is None:
            if RECORD_NAME.search(path.name):
                findings.append(Finding(ERROR, "RECORD_NOT_ON_BOARD", path.name[:-3],
                                        f"{relative} is named like a task record but has no board row"))
            continue
        text, reason = read_text_file(path)
        if text is None:
            unreadable.add(task_id)
            findings.append(Finding(ERROR, "RECORD_UNREADABLE", task_id, f"{relative} could not be decoded: {reason}"))
            continue
        prose = metadata_lines(text)
        _, manifest = parse_manifest(prose)
        statuses = [match.group(1).strip() for match in METADATA_STATUS.finditer(prose)]
        ids = [match.group(1).strip() for match in METADATA_TASK_ID.finditer(prose)]
        reviewer_match = REVIEWER_LINE.search(prose)
        status = statuses[0] if statuses else ""
        if not statuses:
            findings.append(Finding(ERROR, "RECORD_ROW_IDENTITY", task_id, "the record has no `- Status:` metadata line"))
        elif len(statuses) > 1:
            findings.append(Finding(ERROR, "RECORD_ROW_IDENTITY", task_id,
                                    f"the record carries {len(statuses)} `- Status:` lines reading "
                                    f"{', '.join(sorted(set(statuses)))}; rule 7 wants one record to state one status"))
        elif status not in VALID_STATUSES:
            findings.append(Finding(ERROR, "RECORD_ROW_IDENTITY", task_id,
                                    f"record status `{status}` is not one of {', '.join(VALID_STATUSES)}"))
        if len(set(ids)) > 1:
            findings.append(Finding(ERROR, "RECORD_ROW_IDENTITY", task_id,
                                    f"the record names itself {', '.join(sorted(set(ids)))} in "
                                    f"{len(ids)} `- Task ID:` lines"))
        elif ids and ids[0] != task_id:
            findings.append(Finding(ERROR, "RECORD_ROW_IDENTITY", task_id,
                                    f"the record names itself {ids[0]} while the board row is {task_id}"))
        sync_requests = [value for value in (match.group(1).strip() for match in BOARD_SYNC_LINE.finditer(prose)) if value]
        records[task_id] = TaskRecord(task_id, relative, status, manifest,
                                      reviewer_match.group(1).strip() if reviewer_match else "",
                                      sync_requests,
                                      bool(BOARD_SYNC_HEADING.search(prose)) and not sync_requests)
    for task_id, row in sorted(board.items()):
        if task_id in records or task_id in unreadable:
            continue
        if not row.record or not (root / row.record).is_file():
            continue
        findings.append(Finding(ERROR, "RECORD_FILE_MISSING", task_id, f"board row {row.record} was not read as a record"))
    return records, findings


def affirmative(text, pattern, negation=NEGATION, both_sides=False):
    """True when `text` carries `pattern` in a clause that does not deny it.

    A record that says `No independent review was run` must not read as a review, and a basis
    that says `not designated by P0-000` must not read as a designation. The denial normally
    precedes the keyword; `both_sides` also reads after it, which the basis column needs
    because `P0-000 did not designate this task` denies the designation in that order. A
    review line is left prefix-only, because `a separate subagent, not the writer` states
    independence with the same word.
    """
    for match in pattern.finditer(text):
        before = SENTENCE_BREAK.split(text[max(0, match.start() - 60):match.start()])[-1]
        after = SENTENCE_BREAK.split(text[match.end():match.end() + 60])[0]
        if negation.search(before) or (both_sides and negation.search(after)):
            continue
        return True
    return False


def names_independent_reviewer(text):
    """True when the reviewer evidence names an independent reviewer and does not deny the review.

    Parentheses hold the template's own hints (`(independent or self-review)`), so they are
    removed before the mention is read, and a value that only restates the label is not
    evidence. An outright absence (`No reviewer has been assigned`) vetoes the whole value,
    while a denial is read only in the sentence that carries the mention, so `two independent
    subagents reviewed the task ... Human review is not claimed` stays evidence: rule 11 asks
    for a reviewer that is not the author, not for a human one.
    """
    stripped = re.sub(r"\([^)]*\)", " ", text)
    if ABSENT_REVIEW.search(stripped):
        return False
    for match in INDEPENDENT.finditer(stripped):
        before = SENTENCE_BREAK.split(stripped[max(0, match.start() - 80):match.start()])[-1]
        after = SENTENCE_BREAK.split(stripped[match.end():match.end() + 80])[0]
        if NEGATION.search(before) or LOCAL_DENIAL.search(before + match.group(0) + after):
            continue
        return True
    return False


def is_normative(path):
    """Whether a manifest path reaches a rule 8 normative file, case-insensitively.

    A case-folded comparison is deliberate: `AGENTS.MD` is a different string but the same
    file on the macOS case-insensitive filesystem this project qualifies on.
    """
    lowered = path.casefold()
    return lowered in NORMATIVE_CASED or lowered.startswith(NORMATIVE_PREFIXES)


def check_manifest_presence(records, board):
    findings = []
    for task_id, record in sorted(records.items()):
        row = board.get(task_id)
        active = [f"the board row says {row.status}" for row in (row,) if row and row.status in MANIFEST_REQUIRED_STATUSES]
        active += [f"the record says {record.status}"] if record.status in MANIFEST_REQUIRED_STATUSES else []
        if not active or record.has_manifest:
            continue
        findings.append(Finding(ERROR, "MANIFEST_MISSING", task_id,
                                f"rule 7 needs a Change manifest table of exact paths before the first write, and the task is "
                                f"active because {' and '.join(active)}"))
    return findings


def check_manifest_shape(records):
    findings = []
    for task_id, record in sorted(records.items()):
        for pattern in record.patterns():
            findings.append(Finding(NOTICE, "MANIFEST_PATH_NOT_EXACT", task_id,
                                    f"manifest row `{pattern}` is a pattern, not the exact repository-relative path rule 7 asks for"))
        for entry in record.manifest:
            if not entry.repository_relative:
                findings.append(Finding(NOTICE, "MANIFEST_PATH_INVALID", task_id,
                                        f"manifest row `{entry.path}` is absolute or escapes the repository root, so it "
                                        f"reserves nothing and no commit path can match it"))
            elif not entry.canonical:
                findings.append(Finding(NOTICE, "MANIFEST_PATH_NOT_CANONICAL", task_id,
                                        f"manifest row `{entry.path}` is compared as its normalized form `{entry.key}`, "
                                        f"which is the path rule 7 should record instead"))
            if entry.action and entry.action not in {"create", "modify"}:
                findings.append(Finding(NOTICE, "MANIFEST_ACTION_UNKNOWN", task_id,
                                        f"manifest row `{entry.path}` has action `{entry.action}`, expected create or modify"))
    return findings


def check_duplicate_claims(records, board=None):
    """Report one path claimed by more than one live manifest, naming every claimant.

    A canceled task is no longer a writer, so its rows stop reserving paths; the board
    row is checked too because a task can be retired there before its record catches up.
    Two rows in one manifest are a record defect rather than a rule 8 conflict, so they
    get their own notice and never name the same task as its own competing writer.
    """
    claims, shown, findings = {}, {}, []
    for task_id, record in sorted(records.items()):
        row = (board or {}).get(task_id)
        statuses = {record.status, row.status if row else record.status}
        if "canceled" in statuses:
            continue
        for entry in record.manifest:
            if entry.exact:
                claims.setdefault(entry.claim_key, []).append(task_id)
                shown.setdefault(entry.claim_key, entry.key)
    for key, owners in sorted(claims.items()):
        path = shown[key]
        writers = list(dict.fromkeys(owners))
        for task_id in writers:
            if owners.count(task_id) > 1:
                findings.append(Finding(NOTICE, "MANIFEST_ROW_DUPLICATE", task_id,
                                        f"one manifest lists `{path}` in {owners.count(task_id)} rows; rule 7 wants one row "
                                        f"per exact path"))
        if len(writers) > 1:
            for task_id in writers:
                findings.append(Finding(ERROR, "PATH_CLAIM_DUPLICATE", task_id,
                                        f"the path `{path}` is claimed by {', '.join(writers)}; rule 8 allows one claiming "
                                        f"task, so {task_id} must request the change through rule 9 rather than write it"))
    return findings


def check_normative_claims(records):
    findings = []
    for task_id, record in sorted(records.items()):
        for entry in record.manifest:
            if entry.exact:
                if not is_normative(entry.key):
                    continue
                covered = [entry.key]
            else:
                covered = [path for path in NORMATIVE_MATCHES
                           if fnmatch.fnmatchcase(path.casefold(), entry.claim_key.casefold())]
                if not covered:
                    continue
                # One finding per row: the remedy for a pattern that reaches a normative path is
                # to delete the row, not to add a basis column to it.
                findings.append(Finding(ERROR, "NORMATIVE_CLAIM_BY_PATTERN", task_id,
                                        f"manifest row `{entry.path}` is a pattern that reaches the shared normative "
                                        f"path{'' if len(covered) == 1 else 's'} "
                                        f"{', '.join(sorted(f'`{path}`' for path in covered))}; rule 8 says a normative "
                                        f"path is never acquired by a manifest claim, so this row reserves nothing and must "
                                        f"be removed or replaced by the exact path its designated writer owns"))
                continue
            for path in covered:
                if path.casefold() == BOARD_PATH.casefold():
                    if task_id != ALLOCATION_OWNER:
                        findings.append(Finding(ERROR, "BOARD_CLAIM_BY_NON_OWNER", task_id,
                                                "the task board is P0-000's artifact under rule 4 and no other task may claim it in a "
                                                "manifest, whatever the basis column says"))
                    continue
                if not affirmative(entry.basis, DESIGNATED_BASIS, BASIS_NEGATION, both_sides=True):
                    findings.append(Finding(ERROR, "NORMATIVE_CLAIM_UNDESIGNATED", task_id,
                                            f"`{path}` is a shared normative path, so its basis column must name the designated "
                                            f"writer for this revision or the user's direct instruction; the basis said `{entry.basis or 'nothing'}`"))
    return findings


def check_commit_subjects(history):
    findings = []
    for commit in history.commits:
        ids = commit.task_ids()
        if len(ids) == 1:
            continue
        reason = "names no task ID" if not ids else f"names {len(ids)} task IDs ({', '.join(ids)})"
        findings.append(Finding(ERROR if history.after_baseline(commit) else NOTICE, "COMMIT_SUBJECT_TASK_ID", commit.short,
                                f"rule 10 wants exactly one task ID in the subject; this subject {reason}: {commit.subject}"))
    return findings


def check_board_writes(history):
    findings = []
    for commit in history.commits:
        if BOARD_PATH not in commit.paths:
            continue
        ids = commit.task_ids()
        if ids == [ALLOCATION_OWNER]:
            continue
        scope = ", ".join(ids) if ids else "no task ID"
        findings.append(Finding(ERROR if history.after_baseline(commit) else NOTICE, "BOARD_WRITE_OUTSIDE_ALLOCATION",
                                commit.short, f"this commit writes {BOARD_PATH} under {scope}; rule 4 reserves every board "
                                               f"field, including a task's own status, to P0-000"))
    return findings


def check_commit_scope(history, records):
    findings = []
    for commit in history.commits:
        ids = commit.task_ids()
        if len(ids) != 1:
            continue
        task_id = ids[0]
        record = records.get(task_id)
        if record is None:
            findings.append(Finding(NOTICE, "COMMIT_TASK_UNKNOWN", commit.short,
                                    f"the subject scopes this commit to {task_id}, which has no readable task record"))
            continue
        if not record.has_manifest:
            findings.append(Finding(NOTICE, "COMMIT_SCOPE_UNVERIFIABLE", commit.short,
                                    f"{task_id} has no change manifest, so its {len(commit.paths)} committed paths cannot be "
                                    f"scope-checked; MANIFEST_MISSING already names the task"))
            continue
        level = ERROR if history.after_baseline(commit) else NOTICE
        relied = []
        for path in commit.paths:
            if path.casefold() == BOARD_PATH.casefold():
                continue
            kind = record.match_kind(path)
            if kind in (None, "exact", "record"):
                if kind is None:
                    findings.append(Finding(level, "COMMIT_PATH_OUTSIDE_MANIFEST", commit.short,
                                            f"`{path}` is outside the {task_id} manifest, and rule 10 limits a commit to its "
                                            f"own manifest paths"))
                continue
            relied.append((kind, path))
        for pattern in sorted({pattern for pattern, _ in relied}):
            covered = [path for used, path in relied if used == pattern]
            findings.append(Finding(NOTICE, "COMMIT_COVERED_BY_PATTERN", commit.short,
                                    f"{task_id} committed {', '.join(covered[:6])}{' and more' if len(covered) > 6 else ''} "
                                    f"under the manifest pattern `{pattern}` rather than an exact row; rule 2 allows a "
                                    f"pre-assignment scaffold, but the exact path is what would pin it"))
    return findings


def accepted_without_review(records):
    findings = []
    for task_id, record in sorted(records.items()):
        if record.status != "accepted":
            continue
        if not record.reviewer:
            findings.append(Finding(ERROR, "ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW", task_id,
                                    "the record is accepted while its review section names no reviewer at all; rule 11 needs a "
                                    "reviewer that did not implement the work"))
            continue
        if not names_independent_reviewer(record.reviewer):
            findings.append(Finding(ERROR, "ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW", task_id,
                                    f"the review line names no reviewer other than the implementer: {record.reviewer[:160]}"))
    return findings


def accepted_without_commit(records, history):
    findings = []
    for task_id, record in sorted(records.items()):
        if record.status != "accepted":
            continue
        if not history.commits:
            findings.append(Finding(NOTICE, "ACCEPTANCE_COMMIT_UNCHECKED", task_id,
                                    "commit history is unavailable, so the acceptance commit cannot be verified"))
            continue
        if not history.touched(record.relative) and not history.by_task(task_id):
            findings.append(Finding(ERROR, "ACCEPTANCE_WITHOUT_COMMIT", task_id,
                                    "the record is accepted while no commit carries its task ID or touches its record path"))
            continue
        if record.relative in history.dirty_paths or record.relative in history.untracked_paths:
            findings.append(Finding(ERROR, "ACCEPTANCE_EVIDENCE_UNCOMMITTED", task_id,
                                    "the record is accepted while its own file differs from HEAD, so rule 11's accepting commit "
                                    "has not been made"))
            continue
        for path in record.exact_paths():
            if path not in history.committed_paths:
                findings.append(Finding(ERROR, "ACCEPTANCE_WITHOUT_COMMIT", task_id,
                                        f"manifest path `{path}` has never been committed while the record claims acceptance"))
            elif path in history.dirty_paths or path in history.untracked_paths:
                findings.append(Finding(NOTICE, "ACCEPTANCE_PATH_DIRTY_AFTER_COMMIT", task_id,
                                        f"manifest path `{path}` is committed for this task and now differs from HEAD again; "
                                        f"the new difference belongs to whoever is writing it, so it is not this acceptance's "
                                        f"defect, but rule 10 means this task must commit its own next change alone"))
    return findings


def requested_state(request):
    """The one state a `Board sync` line asks P0-000 to write, or None when it names none or several.

    Reading the first status word was not enough: a line that describes the state it is leaving as
    well as the one it reaches (`from in_progress to accepted`, `set the row to accepted; it
    currently reads planned`) named its old state first, so the request matched the very row it was
    complaining about and the backlog went silent. The word a `to` introduces is therefore the
    target, and a bare state is used only when the line carries exactly one; anything else is
    reported rather than guessed at, because rule 11 asks for one row, one state and one commit.
    """
    moved = {match.group(1).lower() for match in TRANSITION_STATE.finditer(request)}
    if len(moved) == 1:
        return moved.pop()
    states = {match.group(0).lower() for match in STATUS_WORD.finditer(request)}
    return states.pop() if len(states) == 1 else None


def ambiguous_states(request):
    """The distinct states a request names, when it names more than one and targets neither."""
    states = sorted({match.group(0).lower() for match in STATUS_WORD.finditer(request)})
    return states if len(states) > 1 else []


def check_board_backlog(records, board):
    findings = []
    for task_id in sorted(set(records) | set(board)):
        record, row = records.get(task_id), board.get(task_id)
        if record is None or row is None:
            continue
        if record.status != row.status:
            findings.append(Finding(NOTICE, "RECORD_BOARD_DIVERGENCE", task_id,
                                    f"the record says {record.status} while the board row says {row.status}; P0-000 syncs the "
                                    f"row from the record and its proving commit"))
        for request in record.board_sync:
            requested = requested_state(request)
            if requested is None:
                found = ", ".join(ambiguous_states(request))
                if found:
                    findings.append(Finding(NOTICE, "BOARD_SYNC_AMBIGUOUS", task_id,
                                            f"the Board sync request names {found} without saying which is the state "
                                            f"reached, so no row can be synced from it; rule 11 wants one state: "
                                            f"{request[:160]}"))
                    continue
                if record.status not in MANIFEST_REQUIRED_STATUSES:
                    continue
                if PLACEHOLDER.search(request):
                    findings.append(Finding(NOTICE, "BOARD_SYNC_UNFILLED", task_id,
                                            "the Board sync line is still the template's placeholder, so P0-000 has no "
                                            "target state or proving commit to sync from; rule 11 wants the row, the state "
                                            "and the commit named"))
                else:
                    findings.append(Finding(NOTICE, "BOARD_SYNC_WAITING", task_id,
                                            f"the Board sync request does not name a state: {request[:160]}"))
                continue
            if requested != row.status:
                commit = COMMIT_HASH.search(request)
                findings.append(Finding(NOTICE, "BOARD_SYNC_WAITING", task_id,
                                        f"P0-000 still owes the {task_id} row the state {requested} "
                                        f"(the board says {row.status}, proving commit "
                                        f"{commit.group(0) if commit else 'not stated'})"))
        if record.board_sync_unfilled and not record.board_sync and record.status in MANIFEST_REQUIRED_STATUSES:
            findings.append(Finding(NOTICE, "BOARD_SYNC_UNFILLED", task_id,
                                    "the Board sync line is still the template's placeholder, so P0-000 has no target row, "
                                    "state or proving commit to sync from; rule 11 wants the request written at acceptance"))
    return findings


def parse_commit_log(text):
    commits, record, paths = [], None, []

    def flush():
        if record is not None:
            commits.append(Commit(record[0], record[1], tuple(paths), record[2]))

    for line in text.splitlines():
        if line.startswith(RECORD_MARK):
            flush()
            fields = line[len(RECORD_MARK):].split("\x1f")
            parents = fields[2].split() if len(fields) >= 3 else []
            record = (fields[0].strip(), fields[1].strip(), len(parents) > 1) if len(fields) >= 3 else None
            paths = []
        elif line.strip() and record is not None:
            paths.append(line.strip())
    flush()
    return commits


def collect_history(root, since=ALLOCATION_BASELINE):
    probe, error = run_git(root, ["rev-parse", "--git-dir"])
    if probe is None:
        return History(unavailable=error or "this working tree is not a git repository")
    log, error = run_git(root, ["log", "--reverse", f"--pretty=format:{RECORD_MARK}%H%x1f%s%x1f%P", "--name-only"])
    if log is None:
        return History(unavailable=error)
    commits = parse_commit_log(log)
    committed, _ = run_git(root, ["ls-tree", "-r", "--name-only", "HEAD"])
    dirty, _ = run_git(root, ["diff", "--name-only", "HEAD"])
    untracked, _ = run_git(root, ["ls-files", "--others", "--exclude-standard"])
    resolved, _ = run_git(root, ["rev-parse", "--verify", "--quiet", f"{since}^{{commit}}"])
    baseline = resolved.strip() if resolved else ""
    cutoff = 0
    for index, commit in enumerate(commits):
        if baseline and commit.sha.startswith(baseline):
            cutoff = index + 1
            break
    else:
        baseline = ""
    # Held to the rules are the commits that already carry the baseline in their own history, so a
    # side branch made before it was issued stays a notice even when it lands after the merge.
    after, _ = run_git(root, ["rev-list", "--ancestry-path", f"{baseline}..HEAD"]) if baseline else (None, "")
    return build_history(commits, (committed or "").splitlines(), (dirty or "").splitlines(),
                         (untracked or "").splitlines(), baseline, cutoff,
                         None if after is None else after.splitlines())


def check_repository(root=ROOT, since=ALLOCATION_BASELINE):
    board, findings = read_board(root)
    records, record_findings = read_records(root, board)
    findings.extend(record_findings)
    history = collect_history(root, since)
    findings.extend(check_manifest_presence(records, board))
    findings.extend(check_manifest_shape(records))
    findings.extend(check_duplicate_claims(records, board))
    findings.extend(check_normative_claims(records))
    findings.extend(accepted_without_review(records))
    findings.extend(accepted_without_commit(records, history))
    findings.extend(check_board_backlog(records, board))
    if history.unavailable:
        findings.append(Finding(NOTICE, "HISTORY_UNAVAILABLE", "-",
                                f"the commit checks were skipped: {history.unavailable}"))
    else:
        if not history.baseline:
            findings.append(Finding(NOTICE, "BASELINE_UNRESOLVED", "-",
                                    f"the section 0 baseline `{since}` is not in this history, so every commit is held to the rules"))
        findings.extend(check_commit_subjects(history))
        findings.extend(check_board_writes(history))
        findings.extend(check_commit_scope(history, records))
    return findings, records, board, history


def codes(findings, level=None):
    counts = {}
    for item in findings:
        if level is None or item.level == level:
            counts[item.code] = counts.get(item.code, 0) + 1
    return dict(sorted(counts.items()))


def summarize(findings, records, board, history):
    errors = [item for item in findings if item.level == ERROR]
    notices = [item for item in findings if item.level == NOTICE]
    lines = []
    if errors:
        lines.append("Errors (AGENTS.md section 0 violated):")
        lines.extend(item.render() for item in sorted(errors, key=lambda item: (item.code, item.scope)))
        lines.append("")
    if notices:
        lines.append("Notices (P0-000 backlog and verification limits; reported, never written by this tool):")
        lines.extend(item.render() for item in sorted(notices, key=lambda item: (item.code, item.scope)))
        lines.append("")
    held = sum(1 for commit in history.commits if history.after_baseline(commit))
    lines.append(f"Summary: {len(errors)} errors and {len(notices)} notices across findings {codes(findings)}")
    lines.append(f"Scope: {len(records)} task records, {len(board)} board rows, {len(history.commits)} commits, "
                 f"{held} of them after the section 0 baseline {history.baseline[:7] or 'unresolved'}")
    lines.append("Not checked: whether P0-000 really designated a normative-path writer, the substance behind a review line, "
                 "the committed paths of a task that has no manifest, merge commits (which carry no paths of their own, so "
                 "their component commits are the checked ones), a task ID written outside the subject's scope parentheses "
                 "(rule 10 puts the task ID in the scope, so `feat(x): docs(p0-014): <subject>` reads as naming none), and anything "
                 "outside the board and the task records. Findings before the baseline are notices by design, so a pre-rule "
                 "commit never fails a run")
    return lines


def main(argv=None):
    parser = argparse.ArgumentParser(description="Check the AGENTS.md section 0 change-manifest and commit-scope rules")
    parser.add_argument("--root", default=str(ROOT), help="repository root, default: this tool's parent directory")
    parser.add_argument("--since", default=ALLOCATION_BASELINE,
                        help=f"commits after this revision are held to the rules (default {ALLOCATION_BASELINE}, which issued them)")
    parser.add_argument("--count-errors", action="store_true",
                        help="print only `ERRORS <n> NOTICES <n>` for a CI step that reads a count instead of the report")
    arguments = parser.parse_args(argv)
    root = Path(arguments.root).resolve()
    findings, records, board, history = check_repository(root, arguments.since)
    if arguments.count_errors:
        errors = sum(1 for item in findings if item.level == ERROR)
        print(f"ERRORS {errors} NOTICES {sum(1 for item in findings if item.level == NOTICE)}")
    else:
        print(f"Task manifest and commit-scope validation for {root}\n")
        for line in summarize(findings, records, board, history):
            print(line)
    return 1 if any(item.level == ERROR for item in findings) else 0


if __name__ == "__main__":
    sys.exit(main())
