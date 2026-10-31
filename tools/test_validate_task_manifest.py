"""Unit and integration tests for the section 0 manifest and commit-scope checker.

Each case breaks exactly one rule and asserts that the matching check reports it,
and the paired passing case asserts the check does not fire, so a green run is
evidence of a working checker rather than a no-op. The live-repository case pins
the findings P0-000 recorded by hand on 2026-10-31. Every fixture lives in a
throwaway directory, so no test can write the real board or a real task record.
"""
import contextlib
import csv
import io
import os
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

import validate_task_manifest as checker
from validate_task_manifest import (ALLOCATION_BASELINE, BOARD_PATH, ERROR, NOTICE, BoardRow, Commit,
                                   MANIFEST_HEADING, METADATA_STATUS, METADATA_TASK_ID, ManifestEntry,
                                   REVIEWER_LINE, TaskRecord, accepted_without_commit, accepted_without_review,
                                   build_history, check_board_backlog, check_board_writes, check_commit_scope,
                                   check_commit_subjects, check_duplicate_claims, check_manifest_presence,
                                   check_manifest_shape, check_normative_claims, check_repository, collect_history,
                                   is_normative, main, metadata_lines, names_independent_reviewer, normalize_path,
                                   parse_commit_log, parse_manifest, read_board, read_records)

BOARD_FIELDS = ["task_id", "title", "requirement_ids", "owner", "depends_on", "status",
                "acceptance_criteria", "record", "updated_on"]


def record_text(task_id, status, manifest=None, reviewer="none yet", board_sync=None, with_reviewer=True,
                template_form=False):
    reviewer_label = "Reviewer and review type"
    sync_label = "Board sync request for P0-000"
    if template_form:
        reviewer_label += " (independent or self-review)"
        sync_label += " (target row, state reached, proving commit; never edit the board yourself)"
    lines = [f"# {task_id}: Fixture task", "", "## Metadata", "", f"- Task ID: {task_id}",
             f"- Status: {status}", "- Owner: EveryGraph Implementation", "", "## Scope and acceptance", ""]
    if manifest is not None:
        lines += ["Change manifest (AGENTS.md section 0 rule 7; one row per exact path):", "",
                  "| Path | Action | Basis |", "|---|---|---|"]
        lines += [f"| `{path}` | {action} | {basis} |" for (path, action, basis) in manifest]
        lines += [""]
    lines += ["## Review and disposition", ""]
    if with_reviewer:
        lines += [f"{reviewer_label}: {reviewer}", ""]
    if board_sync:
        lines += [f"{sync_label}: {entry}" for entry in board_sync]
        lines += [""]
    lines += ["## Handoff and remaining work", ""]
    return "\n".join(lines)


class TreeFixture:
    """Board plus task records in a throwaway directory, never the real repository."""

    def init_tree(self, root):
        self.root = root
        (self.root / "docs" / "planning").mkdir(parents=True)
        (self.root / "docs" / "tasks").mkdir(parents=True)
        self.rows = []

    def task(self, task_id, status, manifest=None, board_status=None, **record_kwargs):
        relative = f"docs/tasks/{task_id.lower()}-fixture.md"
        (self.root / relative).write_text(record_text(task_id, status, manifest=manifest, **record_kwargs),
                                          encoding="utf-8")
        self.rows.append({"task_id": task_id, "title": f"Fixture {task_id}", "requirement_ids": "",
                          "owner": "EveryGraph Implementation", "depends_on": "",
                          "status": board_status or status, "acceptance_criteria": "fixture",
                          "record": relative, "updated_on": "2026-10-31"})
        self.write_board()
        return relative

    def ghost_row(self, task_id, status, record=None):
        self.rows.append({"task_id": task_id, "title": f"Fixture {task_id}", "requirement_ids": "",
                          "owner": "EveryGraph Implementation", "depends_on": "", "status": status,
                          "acceptance_criteria": "fixture",
                          "record": record or f"docs/tasks/{task_id.lower()}-fixture.md", "updated_on": "2026-10-31"})
        return self.write_board()

    def revise_board(self, task_id, **changes):
        for row in self.rows:
            if row["task_id"] == task_id:
                row.update(changes)
        return self.write_board()

    def write_board(self):
        with (self.root / BOARD_PATH).open("w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, BOARD_FIELDS)
            writer.writeheader()
            writer.writerows(self.rows)
        return self.root / BOARD_PATH

    def load(self):
        board, findings = read_board(self.root)
        records, record_findings = read_records(self.root, board)
        return records, board, findings + record_findings

    def codes(self, findings, level=None):
        return [item.code for item in findings if level is None or item.level == level]

    def errors(self, findings):
        return [item for item in findings if item.level == ERROR]

    def run_main(self, root=None):
        with contextlib.redirect_stdout(io.StringIO()):
            return main(["--root", str(root or self.root)])


class FixtureTree(TreeFixture, unittest.TestCase):
    def setUp(self):
        root = Path(tempfile.mkdtemp(prefix="p0014-"))
        self.addCleanup(shutil.rmtree, root, True)
        self.init_tree(root)


class ParsingTests(unittest.TestCase):
    def test_manifest_rows_are_read_as_exact_paths_with_their_basis(self):
        found, entries = parse_manifest(record_text("P0-100", "in_progress",
                                                   manifest=[("modules/data/src/a.cpp", "create", "Own task deliverable"),
                                                             ("docs/tasks/p0-100-fixture.md", "modify", "Own task record")]))
        self.assertTrue(found)
        self.assertEqual([entry.path for entry in entries], ["modules/data/src/a.cpp", "docs/tasks/p0-100-fixture.md"])
        self.assertEqual(entries[1].basis, "Own task record")
        self.assertTrue(all(entry.exact for entry in entries))

    def test_manifest_heading_without_a_table_is_not_a_manifest(self):
        found, entries = parse_manifest("Change manifest (complete it before the first write):\n\nOut of scope: none\n")
        self.assertTrue(found)
        self.assertEqual(entries, [])

    def test_record_without_any_manifest_heading_is_reported_missing(self):
        found, entries = parse_manifest(record_text("P0-100", "planned"))
        self.assertFalse(found)
        self.assertEqual(entries, [])

    def test_glob_and_placeholder_rows_stay_patterns(self):
        entries = parse_manifest(record_text("P0-100", "in_progress",
                                            manifest=[("modules/**", "create", "Own task deliverable"),
                                                      ("docs/tasks/<new-task-id>-*.md", "create", "Allocation scaffold")]))[1]
        self.assertEqual([entry.exact for entry in entries], [False, False])
        self.assertEqual(entries[1].path, "docs/tasks/<new-task-id>-*.md")

    def test_a_manifest_table_with_synonym_column_headings_is_still_read(self):
        body = ("## Change manifest\n\n| File | Operation | Why |\n|---|---|---|\n"
                "| `docs/planning/task-board.csv` | modify | Own task record |\n")
        found, entries = parse_manifest(body)
        self.assertTrue(found)
        self.assertEqual([(entry.path, entry.action, entry.basis) for entry in entries],
                         [("docs/planning/task-board.csv", "modify", "Own task record")])

    def test_a_template_quoted_inside_a_fence_is_not_the_record_s_own_metadata(self):
        body = record_text("P0-100", "in_progress",
                           manifest=[("docs/tasks/p0-100-fixture.md", "modify", "Own task record")])
        quoted = ("\n## Notes\n\nThe template says:\n\n```markdown\n- Task ID: P9-999\n- Status: canceled\n"
                  "Reviewer and review type: nobody ran one\n\n| Path | Action | Basis |\n|---|---|---|\n"
                  "| `AGENTS.md` | modify | Own task artifact |\n```\n")
        found, entries = parse_manifest(body + quoted)
        self.assertEqual([entry.path for entry in entries], ["docs/tasks/p0-100-fixture.md"])
        self.assertEqual(len(METADATA_STATUS.findall(metadata_lines(body + quoted))), 1)
        self.assertEqual(len(METADATA_TASK_ID.findall(metadata_lines(body + quoted))), 1)
        self.assertEqual(REVIEWER_LINE.search(metadata_lines(body + quoted)).group(1).strip(), "none yet")

    def test_every_manifest_table_in_a_record_is_read(self):
        body = record_text("P0-100", "in_progress",
                           manifest=[("tools/x.py", "create", "Own task deliverable")])
        body += ("\n\n## Change manifest (extended)\n\n| Path | Action | Basis |\n|---|---|---|\n"
                 "| `docs/planning/task-board.csv` | modify | Own task record |\n")
        found, entries = parse_manifest(body)
        self.assertTrue(found)
        self.assertEqual([entry.path for entry in entries], ["tools/x.py", "docs/planning/task-board.csv"])
        records = {"P0-100": TaskRecord("P0-100", "docs/tasks/p0-100-fixture.md", "in_progress", entries)}
        self.assertEqual([item.code for item in check_normative_claims(records)], ["BOARD_CLAIM_BY_NON_OWNER"])

    def test_a_heading_at_any_level_or_wrapped_in_bold_is_a_manifest_heading(self):
        for heading in ("# Change manifest", "###### Change manifest (rule 7)", "**Change manifest** (rule 7):"):
            body = record_text("P0-100", "in_progress").replace("## Scope and acceptance",
                                                                heading + "\n\n| Path | Action | Basis |\n|---|---|---|\n"
                                                                "| `tools/y.py` | create | Own task deliverable |")
            self.assertEqual([entry.path for entry in parse_manifest(body)[1]], ["tools/y.py"], heading)

    def test_path_cells_are_normalized(self):
        self.assertEqual(normalize_path("  `./docs/tasks/x.md`  "), "docs/tasks/x.md")
        self.assertEqual(normalize_path("`/absolute/x.md`"), "/absolute/x.md")
        self.assertEqual(normalize_path("docs\\tasks\\x.md"), "docs/tasks/x.md")
        self.assertEqual(normalize_path("`modules/**`"), "modules/**")

    def test_a_bold_wrapped_cell_is_the_exact_path_while_a_glob_stays_a_glob(self):
        self.assertEqual(normalize_path("**tools/bold.py**"), "tools/bold.py")
        self.assertEqual(normalize_path("**`tools/bold.py`**"), "tools/bold.py")
        self.assertEqual(normalize_path("modules/**"), "modules/**")
        self.assertEqual(normalize_path("**src/**/*.cpp**"), "**src/**/*.cpp**")
        self.assertEqual(normalize_path("`**tools/mixed.py**`"), "tools/mixed.py")
        heading = "## Change manifest (rule 7)\n"
        entries = parse_manifest(heading + "\n| Path | Action | Basis |\n|---|---|---|\n"
                                 "| `**tools/bold.py**` | create | Own task deliverable |\n"
                                 "| `src/**/*.cpp` | create | Own task deliverable |\n")[1]
        self.assertEqual([entry.exact for entry in entries], [True, False])
        self.assertEqual([entry.path for entry in entries], ["tools/bold.py", "src/**/*.cpp"])

    def test_status_reviewer_and_board_sync_lines_are_parsed(self):
        body = record_text("P0-100", "accepted", reviewer="independent read-only review by a separate subagent",
                           board_sync=["accepted, proving commit 2acd93e"])
        record = TaskRecord("P0-100", "docs/tasks/p0-100-fixture.md", "accepted", parse_manifest(body)[1],
                            checker.REVIEWER_LINE.search(body).group(1).strip(),
                            [match.group(1).strip() for match in checker.BOARD_SYNC_LINE.finditer(body)])
        self.assertEqual(record.status, "accepted")
        self.assertIn("separate subagent", record.reviewer)
        self.assertEqual(record.board_sync, ["accepted, proving commit 2acd93e"])

    def test_commit_log_records_are_split_into_paths_and_parents(self):
        log = (f"{checker.RECORD_MARK}aaaa{chr(31)}docs(p0-100): first{chr(31)}\n"
               "\ndocs/tasks/p0-100-fixture.md\ntools/x.py\n"
               f"{checker.RECORD_MARK}bbbb{chr(31)}merge: second{chr(31)}cccc dddd\n")
        commits = parse_commit_log(log)
        self.assertEqual([commit.sha for commit in commits], ["aaaa", "bbbb"])
        self.assertEqual(commits[0].paths, ("docs/tasks/p0-100-fixture.md", "tools/x.py"))
        self.assertFalse(commits[0].merge)
        self.assertTrue(commits[1].merge)
        self.assertEqual(commits[1].paths, ())

    def test_the_subject_scope_is_the_only_task_id_source(self):
        self.assertEqual(Commit("a", "docs(p0-014): add the checker").task_ids(), ["P0-014"])
        self.assertEqual(Commit("a", "feat: pin the Qt graph").task_ids(), [])
        self.assertEqual(Commit("a", "docs: sync the accepted P0-013 row").task_ids(), [])
        self.assertEqual(Commit("a", "chore(ci): run the suite").task_ids(), [])
        self.assertEqual(Commit("a", "docs(p0-000, p0-014): two scopes").task_ids(), ["P0-000", "P0-014"])

    def test_the_template_parenthetical_labels_are_parsed(self):
        body = record_text("P0-100", "accepted", template_form=True,
                           reviewer="one independent read-only subagent, not the writer.",
                           board_sync=["P0-100 reached accepted at deadbeef1, the board row is owed the flip"])
        self.assertEqual(checker.REVIEWER_LINE.search(body).group(1).strip(),
                         "one independent read-only subagent, not the writer.")
        self.assertEqual(checker.BOARD_SYNC_LINE.search(body).group(1).strip(),
                         "P0-100 reached accepted at deadbeef1, the board row is owed the flip")

    def test_is_normative_is_case_folded_for_one_file_per_entry(self):
        entry = ManifestEntry("docs/tasks/../planning/task-board.csv", "modify", "Own task deliverable")
        self.assertEqual(entry.key, BOARD_PATH)
        self.assertFalse(entry.canonical)
        self.assertTrue(is_normative(entry.key))
        self.assertEqual(ManifestEntry("./CMakeLists.txt", "modify", "Own task deliverable").key, "CMakeLists.txt")
        escaping = ManifestEntry("../../secrets/token", "create", "Own task deliverable")
        self.assertFalse(escaping.repository_relative)
        self.assertFalse(is_normative(escaping.key))
        self.assertTrue(ManifestEntry("AGENTS.MD", "modify", "x").canonical)


class TemplateRecordTests(FixtureTree):
    """The template's own label forms must not create false findings (review finding 1)."""

    def test_a_template_form_review_line_does_not_falsely_deny_the_reviewer(self):
        self.task("P0-200", "accepted", template_form=True,
                  reviewer="one independent read-only subagent, not the writer.")
        records, _, _ = self.load()
        self.assertEqual(accepted_without_review(records), [])

    def test_a_template_form_board_sync_request_is_seen_as_waiting(self):
        self.task("P0-200", "accepted", template_form=True, board_status="in_review",
                  reviewer="one independent read-only subagent, not the writer.",
                  board_sync=["P0-200 reached accepted at deadbeef1"])
        records, board, _ = self.load()
        findings = [item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_WAITING"]
        self.assertEqual(len(findings), 1)
        self.assertIn("deadbeef1", findings[0].message)

    def test_an_unfilled_sync_placeholder_on_a_quiet_record_is_not_a_waiting_request(self):
        self.task("P0-200", "planned", template_form=True,
                  board_sync=["(the row, the state reached and the proving commit go here)"])
        records, board, _ = self.load()
        self.assertEqual([item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_WAITING"], [])


class PathEscapeTests(FixtureTree):
    """Review finding 2: `./`, `..`, an absolute path or a case variant must not dodge a rule."""

    def test_a_dotted_board_claim_still_hits_the_ownership_rule(self):
        self.task("P0-200", "in_progress",
                  manifest=[("docs/tasks/../planning/task-board.csv", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        findings = check_normative_claims(records)
        self.assertEqual([item.code for item in findings], ["BOARD_CLAIM_BY_NON_OWNER"])

    def test_a_dotted_normative_claim_still_hits_the_designation_rule(self):
        self.task("P0-200", "in_progress", manifest=[("build/../CMakeLists.txt", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["NORMATIVE_CLAIM_UNDESIGNATED"])

    def test_a_non_canonical_row_is_reported_so_the_owner_can_fix_the_form(self):
        self.task("P0-200", "in_progress", manifest=[("build/../CMakeLists.txt", "modify", "P0-000 designated this writer")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_manifest_shape(records)], ["MANIFEST_PATH_NOT_CANONICAL"])
        self.assertEqual(check_normative_claims(records), [])

    def test_a_path_escaping_the_repository_root_is_reported_and_reserves_nothing(self):
        self.task("P0-200", "in_progress", manifest=[("../../secrets/token", "create", "Own task deliverable")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_manifest_shape(records)], ["MANIFEST_PATH_INVALID"])
        self.assertEqual(check_duplicate_claims(records, {"P0-200": BoardRow("P0-200", "in_progress", "x")}), [])

    def test_a_case_variant_normative_claim_is_caught(self):
        self.task("P0-200", "in_progress", manifest=[("AGENTS.MD", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["NORMATIVE_CLAIM_UNDESIGNATED"])

    def test_an_absolute_normative_claim_is_caught_and_reported_as_invalid(self):
        self.task("P0-200", "in_progress", manifest=[("/AGENTS.md", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["NORMATIVE_CLAIM_UNDESIGNATED"])
        self.assertEqual([item.code for item in check_manifest_shape(records)], ["MANIFEST_PATH_INVALID"])

    def test_a_negated_designation_does_not_count_as_a_designation(self):
        self.task("P0-200", "in_progress", manifest=[("AGENTS.md", "modify", "not designated by P0-000")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["NORMATIVE_CLAIM_UNDESIGNATED"])

    def test_a_duplicate_claim_is_found_through_a_dotted_form(self):
        self.task("P0-200", "in_progress", manifest=[("tools/x.py", "create", "Own task deliverable")])
        self.task("P0-201", "in_progress", manifest=[("tools/../tools/x.py", "create", "Own task deliverable")])
        records, board, _ = self.load()
        findings = check_duplicate_claims(records, board)
        self.assertEqual(len(findings), 2)
        self.assertEqual({item.scope for item in findings}, {"P0-200", "P0-201"})


class IndependenceHeuristicTests(unittest.TestCase):
    def test_a_named_independent_reviewer_passes(self):
        self.assertTrue(names_independent_reviewer("two independent read-only subagents reviewed the task. "
                                                   "Neither is a human reviewer."))
        self.assertTrue(names_independent_reviewer("independent read-only review by a delegated subagent with no "
                                                   "edit authority, plus a self-check by the executor."))

    def test_self_review_and_denials_do_not(self):
        self.assertFalse(names_independent_reviewer("main-agent; self-review only. No independent or human "
                                                    "review is claimed."))
        self.assertFalse(names_independent_reviewer("this is a self-review. It is not an independent review."))
        self.assertFalse(names_independent_reviewer("none yet."))

    def test_the_forms_live_accepted_records_actually_use_are_evidence(self):
        for value in ("independent EveryGraph Review subagent, read-only; it reviewed this record and the board row",
                      "two independent read-only subagents reviewed the task. Human review is not claimed.",
                      "two independent read-only subagents reviewed the task on 2026-10-31. No human reviewer "
                      "participated.",
                      "independent data/QA review by general-purpose subagents in read-only mode; no fourth review "
                      "was run by the author",
                      "**independent**, two reviewers across three rounds, neither of which wrote any part of the "
                      "implementation"):
            with self.subTest(value=value):
                self.assertTrue(names_independent_reviewer(value), value)

    def test_an_outright_absence_anywhere_in_the_value_vetoes_it(self):
        for value in ("No reviewer has been assigned. An independent review is required for this task",
                      "an independent review is required for this task. No reviewer has been assigned",
                      "no review has been performed by anyone, though an independent reviewer would be needed",
                      "no review was conducted. a separate subagent is not the writer"):
            with self.subTest(value=value):
                self.assertFalse(names_independent_reviewer(value), value)

    def test_a_reviewer_mention_the_record_then_denies_is_not_evidence(self):
        for value in ("an independent review, which nobody ran",
                      "the independent review was not performed",
                      "one independent subagent, but no review was conducted",
                      "an independent review is neither claimed nor run here",
                      "independent reviews are required for this task",
                      "an independent review was required but not obtained",
                      "independent review remains required before acceptance",
                      "two independent reviews shall be required for this task"):
            with self.subTest(value=value):
                self.assertFalse(names_independent_reviewer(value), value)
        self.assertTrue(names_independent_reviewer("one independent read-only subagent review, not the writer."))
        self.assertTrue(names_independent_reviewer("p0-014-verifier, a separate read-only agent that did not "
                                                   "implement this work"))
        self.assertTrue(names_independent_reviewer("a reviewer that was required and was run: a separate subagent"))


class ManifestPresenceTests(FixtureTree):
    def test_active_statuses_require_a_manifest(self):
        for index, status in enumerate(("in_progress", "in_review", "accepted")):
            self.task(f"P0-20{index}", status)
        records, board, _ = self.load()
        findings = check_manifest_presence(records, board)
        self.assertEqual(len(findings), 3)
        self.assertEqual({item.code for item in findings}, {"MANIFEST_MISSING"})
        self.assertTrue(all(item.level == ERROR for item in findings))

    def test_quiet_statuses_pass_without_a_manifest(self):
        for index, status in enumerate(("planned", "blocked", "canceled")):
            self.task(f"P0-30{index}", status)
        records, board, _ = self.load()
        self.assertEqual(check_manifest_presence(records, board), [])

    def test_a_manifest_satisfies_the_requirement(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        records, board, _ = self.load()
        self.assertEqual(check_manifest_presence(records, board), [])

    def test_a_record_ahead_of_its_planned_board_row_still_needs_a_manifest(self):
        self.task("P0-200", "in_progress", board_status="planned")
        records, board, _ = self.load()
        findings = check_manifest_presence(records, board)
        self.assertEqual([item.code for item in findings], ["MANIFEST_MISSING"])
        self.assertIn("the record says in_progress", findings[0].message)

    def test_a_board_row_ahead_of_its_planned_record_still_needs_a_manifest(self):
        self.task("P0-200", "planned", board_status="accepted")
        records, board, _ = self.load()
        findings = check_manifest_presence(records, board)
        self.assertEqual([item.code for item in findings], ["MANIFEST_MISSING"])
        self.assertIn("the board row says accepted", findings[0].message)


class ManifestShapeTests(FixtureTree):
    def test_pattern_rows_are_reported_as_notices(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/<new-task-id>-*.md", "create", "Allocation scaffold")])
        records, _, _ = self.load()
        findings = check_manifest_shape(records)
        self.assertEqual([item.code for item in findings], ["MANIFEST_PATH_NOT_EXACT"])
        self.assertEqual(findings[0].level, NOTICE)

    def test_an_unknown_action_is_a_notice(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "rewrite", "Own task record")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_manifest_shape(records)], ["MANIFEST_ACTION_UNKNOWN"])


class DuplicateClaimTests(FixtureTree):
    def test_two_manifests_one_path_names_both_tasks(self):
        self.task("P0-200", "in_progress", manifest=[("modules/data/src/a.cpp", "create", "Own task deliverable")])
        self.task("P0-201", "in_progress", manifest=[("modules/data/src/a.cpp", "create", "Own task deliverable")])
        records, board, _ = self.load()
        findings = check_duplicate_claims(records, board)
        self.assertEqual(len(findings), 2)
        self.assertEqual({item.scope for item in findings}, {"P0-200", "P0-201"})
        for item in findings:
            self.assertIn("P0-200, P0-201", item.message)
            self.assertIn("rule 9", item.message)

    def test_a_case_variant_claim_is_the_same_path_two_tasks_want(self):
        self.task("P0-200", "in_progress", manifest=[("tools/x.py", "create", "Own task deliverable")])
        self.task("P0-201", "in_progress", manifest=[("Tools/X.py", "create", "Own task deliverable")])
        records, board, _ = self.load()
        findings = check_duplicate_claims(records, board)
        self.assertEqual({item.scope for item in findings}, {"P0-200", "P0-201"})
        self.assertTrue(all(item.level == ERROR for item in findings))
        for item in findings:
            self.assertIn("the path `tools/x.py`", item.message, "the report shows the path as it is spelled, not case-folded")

    def test_a_row_listed_twice_in_one_manifest_is_a_notice_not_a_self_conflict(self):
        self.task("P0-200", "in_progress", manifest=[("tools/x.py", "create", "Own task deliverable"),
                                                     ("tools/x.py", "create", "Own task deliverable")])
        records, board, _ = self.load()
        findings = check_duplicate_claims(records, board)
        self.assertEqual([item.code for item in findings], ["MANIFEST_ROW_DUPLICATE"])
        self.assertEqual(findings[0].level, NOTICE)

    def test_a_canceled_task_does_not_claim_a_path(self):
        self.task("P0-200", "accepted", manifest=[("modules/data/src/a.cpp", "create", "Own task deliverable")])
        self.task("P0-201", "canceled", manifest=[("modules/data/src/a.cpp", "create", "Superseded work")])
        records, board, _ = self.load()
        self.assertEqual(check_duplicate_claims(records, board), [])

    def test_a_canceled_board_row_does_not_claim_a_path(self):
        self.task("P0-200", "accepted", manifest=[("modules/data/src/a.cpp", "create", "Own task deliverable")])
        self.task("P0-201", "accepted", board_status="canceled",
                  manifest=[("modules/data/src/a.cpp", "create", "Superseded work")])
        records, board, _ = self.load()
        self.assertEqual(check_duplicate_claims(records, board), [])

    def test_patterns_are_not_deduplicated_as_exact_claims(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/<new-id>-*.md", "create", "Allocation scaffold")])
        self.task("P0-201", "in_progress", manifest=[("docs/tasks/p0-201-fixture.md", "create", "Own task record")])
        records, board, _ = self.load()
        self.assertEqual(check_duplicate_claims(records, board), [])


class NormativeClaimTests(FixtureTree):
    def test_the_registry_covers_the_rule_8_paths(self):
        for path in ("AGENTS.md", BOARD_PATH, "docs/planning/feature-traceability.csv", "CMakeLists.txt",
                     "CMakePresets.json", "docs/engineering/development.md",
                     "docs/engineering/licensing-and-dependencies.md", "tools/validate_planning.py",
                     ".github/workflows/phase0.yml"):
            self.assertTrue(is_normative(path), path)
        self.assertFalse(is_normative("modules/data/src/a.cpp"))
        self.assertFalse(is_normative("docs/planning/functional-scope.md"))

    def test_a_normative_claim_without_a_designated_writer_basis_fails(self):
        self.task("P0-200", "in_progress", manifest=[("CMakeLists.txt", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        findings = check_normative_claims(records)
        self.assertEqual([item.code for item in findings], ["NORMATIVE_CLAIM_UNDESIGNATED"])
        self.assertIn("CMakeLists.txt", findings[0].message)

    def test_a_basis_naming_the_designation_or_the_user_passes(self):
        self.task("P0-200", "in_progress",
                  manifest=[("CMakeLists.txt", "modify", "Shared normative path; P0-000 designates P1-011 as this writer"),
                            ("AGENTS.md", "modify", "Direct user instruction on 2026-10-31"),
                            ("CMakePresets.json", "modify", "the designated writer for this revision")])
        records, _, _ = self.load()
        self.assertEqual(check_normative_claims(records), [])

    def test_a_board_claim_by_another_task_fails_even_with_a_basis(self):
        self.task("P0-200", "accepted", manifest=[(BOARD_PATH, "modify", "Direct user instruction naming P0-000")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["BOARD_CLAIM_BY_NON_OWNER"])

    def test_a_board_claim_by_p0_000_passes(self):
        self.task("P0-000", "in_progress", manifest=[(BOARD_PATH, "modify", "Designated to P0-000 by rules 4 and 8")])
        records, _, _ = self.load()
        self.assertEqual(check_normative_claims(records), [])

    def test_a_synonym_headed_board_claim_reaches_the_rule_4_error(self):
        relative = self.task("P0-200", "accepted",
                             manifest=[(BOARD_PATH, "modify", "Direct user instruction naming P0-000")])
        path = self.root / relative
        path.write_text(path.read_text(encoding="utf-8").replace("| Path | Action | Basis |", "| File | Operation | Why |"),
                        encoding="utf-8")
        records, _, _ = self.load()
        findings = check_normative_claims(records)
        self.assertEqual([item.code for item in findings], ["BOARD_CLAIM_BY_NON_OWNER"])
        self.assertEqual(findings[0].scope, "P0-200")

    def test_a_workflow_path_is_normative(self):
        self.task("P0-200", "in_progress", manifest=[(".github/workflows/phase0.yml", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        self.assertEqual([item.code for item in check_normative_claims(records)], ["NORMATIVE_CLAIM_UNDESIGNATED"])

    def test_a_denial_that_follows_the_designation_word_still_fails(self):
        self.task("P0-200", "in_progress",
                  manifest=[("AGENTS.md", "modify", "P0-000 did not designate this task for this revision"),
                            ("CMakeLists.txt", "modify", "no designation is claimed for this path here"),
                            ("CMakePresets.json", "modify", "P0-000 designates P0-200 for this revision")])
        records, _, _ = self.load()
        self.assertEqual(sorted(item.scope for item in check_normative_claims(records)), ["P0-200", "P0-200"],
                         "the two denied bases fail and the affirmative one passes")
        self.assertEqual([item.code for item in check_normative_claims(records)],
                         ["NORMATIVE_CLAIM_UNDESIGNATED", "NORMATIVE_CLAIM_UNDESIGNATED"])
        self.assertEqual(sorted(item.message for item in check_normative_claims(records)),
                         sorted([f"`{path}` is a shared normative path, so its basis column must name the designated "
                                 f"writer for this revision or the user's direct instruction; the basis said `{basis}`"
                                 for path, basis in (("AGENTS.md", "P0-000 did not designate this task for this revision"),
                                                     ("CMakeLists.txt", "no designation is claimed for this path here"))]))

    def test_a_pattern_row_that_reaches_a_normative_path_fails(self):
        self.task("P0-200", "in_progress", manifest=[("docs/**", "modify", "Own task deliverable"),
                                                     (".github/**", "modify", "Own task deliverable")])
        records, _, _ = self.load()
        findings = check_normative_claims(records)
        self.assertEqual([item.code for item in findings], ["NORMATIVE_CLAIM_BY_PATTERN", "NORMATIVE_CLAIM_BY_PATTERN"])
        self.assertIn("docs/planning/task-board.csv", findings[0].message)
        self.assertIn(".github/workflows/ci.yml", findings[1].message)

    def test_a_pattern_row_over_ordinary_sources_reaches_nothing_normative(self):
        self.task("P0-200", "in_progress", manifest=[("modules/data/**/*.cpp", "create", "Own task deliverable"),
                                                     ("docs/tasks/<new-id>-*.md", "create", "Allocation scaffold")])
        records, _, _ = self.load()
        self.assertEqual(check_normative_claims(records), [])

    def test_a_bold_or_backtick_wrapped_normative_cell_still_fails(self):
        for cell in ("**`docs/planning/task-board.csv`**", "`**docs/planning/task-board.csv`**",
                     "**docs/planning/task-board.csv**"):
            with self.subTest(cell=cell):
                self.task("P0-200", "in_progress", manifest=[(cell, "modify", "Direct user instruction")])
                self.task("P0-201", "in_progress",
                          manifest=[(cell.replace("docs/planning/task-board.csv", "docs/engineering/development.md"),
                                      "modify", "Own task artifact")])
                records, _, _ = self.load()
                by_scope = {}
                for item in check_normative_claims(records):
                    by_scope.setdefault(item.scope, []).append(item.code)
                self.assertEqual(by_scope, {"P0-200": ["BOARD_CLAIM_BY_NON_OWNER"],
                                            "P0-201": ["NORMATIVE_CLAIM_UNDESIGNATED"]}, cell)


class CommitScopeTests(unittest.TestCase):
    def setUp(self):
        manifest = [ManifestEntry("docs/tasks/p0-200-fixture.md", "modify", "Own task record"),
                    ManifestEntry("tools/x.py", "create", "Own task deliverable"),
                    ManifestEntry("docs/tasks/<new-id>-*.md", "create", "Allocation scaffold")]
        self.records = {
            "P0-200": TaskRecord("P0-200", "docs/tasks/p0-200-fixture.md", "in_progress", manifest),
            "P0-000": TaskRecord("P0-000", "docs/tasks/p0-000-fixture.md", "in_progress",
                                 [ManifestEntry("docs/tasks/p0-000-fixture.md", "modify", "Own task record")]),
            "P0-001": TaskRecord("P0-001", "docs/tasks/p0-001-fixture.md", "accepted", []),
        }

    def history(self, commits, cutoff=0):
        return build_history(commits, cutoff=cutoff)

    def test_a_path_outside_the_manifest_is_reported(self):
        commit = Commit("a" * 40, "docs(p0-200): add the checker", ("tools/x.py", "modules/data/src/a.cpp"))
        findings = check_commit_scope(self.history([commit]), self.records)
        self.assertEqual([item.scope for item in findings], ["aaaaaaa"])
        self.assertEqual(findings[0].code, "COMMIT_PATH_OUTSIDE_MANIFEST")
        self.assertIn("modules/data/src/a.cpp", findings[0].message)

    def test_manifest_paths_and_the_own_record_are_allowed_exactly(self):
        paths = ("tools/x.py", "docs/tasks/p0-200-fixture.md")
        commit = Commit("a" * 40, "docs(p0-200): add the checker", paths)
        self.assertEqual(check_commit_scope(self.history([commit]), self.records), [])

    def test_a_path_covered_only_by_a_pattern_is_a_notice_not_an_exact_allow(self):
        commit = Commit("a" * 40, "docs(p0-200): allocate a scaffold", ("docs/tasks/p0-207-other.md",))
        findings = check_commit_scope(self.history([commit]), self.records)
        self.assertEqual([item.code for item in findings], ["COMMIT_COVERED_BY_PATTERN"])
        self.assertEqual(findings[0].level, NOTICE)
        self.assertIn("docs/tasks/<new-id>-*.md", findings[0].message)

    def test_a_pattern_row_does_not_hide_a_path_outside_the_manifest(self):
        commit = Commit("a" * 40, "docs(p0-200): add the checker", ("docs/tasks/p0-207-other.md", "modules/data/src/a.cpp"))
        findings = check_commit_scope(self.history([commit]), self.records)
        self.assertEqual([item.code for item in findings], ["COMMIT_PATH_OUTSIDE_MANIFEST", "COMMIT_COVERED_BY_PATTERN"])

    def test_a_commit_by_a_task_without_a_manifest_is_a_notice(self):
        commit = Commit("b" * 40, "docs(p0-001): accept", ("docs/research/evidence.md",))
        findings = check_commit_scope(self.history([commit]), self.records)
        self.assertEqual([item.code for item in findings], ["COMMIT_SCOPE_UNVERIFIABLE"])
        self.assertEqual(findings[0].level, NOTICE)

    def test_an_unknown_task_in_a_subject_is_a_notice(self):
        commit = Commit("c" * 40, "docs(p9-999): something", ("tools/x.py",))
        findings = check_commit_scope(self.history([commit]), self.records)
        self.assertEqual([item.code for item in findings], ["COMMIT_TASK_UNKNOWN"])

    def test_the_board_path_in_another_task_commit_is_the_ownership_rule(self):
        commit = Commit("d" * 40, "docs(p0-200): sync my row", (BOARD_PATH,))
        self.assertEqual(check_commit_scope(self.history([commit]), self.records), [])
        findings = check_board_writes(self.history([commit]))
        self.assertEqual([item.code for item in findings], ["BOARD_WRITE_OUTSIDE_ALLOCATION"])

    def test_p0_000_board_writes_and_merge_commits_pass(self):
        commits = [Commit("e" * 40, "docs(p0-000): sync the board",
                          (BOARD_PATH, "docs/tasks/p0-000-fixture.md")),
                   Commit("f" * 40, "docs(p0-000): merge", (), True)]
        self.assertEqual(check_board_writes(self.history(commits)), [])
        self.assertEqual(check_commit_scope(self.history(commits), self.records), [])

    def test_a_subject_with_two_scopes_names_both(self):
        commit = Commit("a" * 40, "docs(p0-000, p0-014): sweep two tasks", ("docs/tasks/p0-200-fixture.md",))
        findings = check_commit_subjects(self.history([commit]))
        self.assertEqual([item.level for item in findings], [ERROR])
        self.assertIn("names 2 task IDs (P0-000, P0-014)", findings[0].message)

    def test_a_subject_without_a_task_id_fails_after_the_baseline(self):
        commits = [Commit("a" * 40, "docs(p0-200): scoped", ("docs/tasks/p0-200-fixture.md",)),
                   Commit("b" * 40, "docs: sweep the index", ("docs/tasks/p0-200-fixture.md",))]
        findings = check_commit_subjects(self.history(commits, cutoff=1))
        self.assertEqual([item.scope for item in findings], ["bbbbbbb"])
        self.assertEqual(findings[0].level, ERROR)

    def test_a_subject_with_a_non_task_scope_is_a_notice_before_the_baseline(self):
        commit = Commit("a" * 40, "feat: pre-rule baseline", ("AGENTS.md",))
        findings = check_commit_subjects(self.history([commit], cutoff=1))
        self.assertEqual([item.level for item in findings], [NOTICE])

    def test_path_findings_are_errors_after_the_baseline_and_notices_before(self):
        outside = ("modules/data/src/a.cpp",)
        before = Commit("a" * 40, "docs(p0-200): earlier", outside)
        after = Commit("b" * 40, "docs(p0-200): later", outside)
        findings = check_commit_scope(self.history([before, after], cutoff=1), self.records)
        self.assertEqual({item.scope: item.level for item in findings}, {"aaaaaaa": NOTICE, "bbbbbbb": ERROR})

    def test_board_writes_before_the_baseline_are_notices(self):
        commits = [Commit("a" * 40, "chore: capture the board", (BOARD_PATH,)),
                   Commit("b" * 40, "chore: later sweep", (BOARD_PATH,))]
        findings = check_board_writes(self.history(commits, cutoff=1))
        self.assertEqual([item.level for item in findings], [NOTICE, ERROR])


class SelfAcceptanceTests(FixtureTree):
    def test_accepted_without_a_reviewer_line_fails(self):
        self.task("P0-200", "accepted", with_reviewer=False)
        records, _, _ = self.load()
        findings = accepted_without_review(records)
        self.assertEqual([item.code for item in findings], ["ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW"])
        self.assertIn("names no reviewer at all", findings[0].message)

    def test_accepted_on_self_review_only_fails(self):
        self.task("P0-200", "accepted", reviewer="main-agent; self-review only. No independent review is claimed.")
        records, _, _ = self.load()
        self.assertEqual([item.code for item in accepted_without_review(records)],
                         ["ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW"])

    def test_accepted_after_independent_review_passes(self):
        self.task("P0-200", "accepted", reviewer="one independent read-only subagent review, not the writer.")
        records, _, _ = self.load()
        self.assertEqual(accepted_without_review(records), [])

    def test_accepted_while_nothing_is_committed_fails(self):
        self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.")
        records, _, _ = self.load()
        history = build_history([Commit("a" * 40, "docs(p0-000): unrelated", ("AGENTS.md",))],
                                committed_paths=["AGENTS.md"])
        findings = accepted_without_commit(records, history)
        self.assertEqual([item.code for item in findings], ["ACCEPTANCE_WITHOUT_COMMIT"])
        self.assertIn("no commit", findings[0].message)

    def test_accepted_with_an_uncommitted_record_fails(self):
        relative = self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.")
        records, _, _ = self.load()
        history = build_history([Commit("a" * 40, "docs(p0-200): start", (relative,))],
                                committed_paths=[relative], dirty_paths=[relative])
        self.assertEqual([item.code for item in accepted_without_commit(records, history)],
                         ["ACCEPTANCE_EVIDENCE_UNCOMMITTED"])

    def test_accepted_with_an_uncommitted_deliverable_fails(self):
        relative = self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.",
                             manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record"),
                                       ("tools/x.py", "create", "Own task deliverable")])
        records, _, _ = self.load()
        history = build_history([Commit("a" * 40, "docs(p0-200): accept", (relative,))],
                                committed_paths=[relative], untracked_paths=["tools/x.py"])
        findings = accepted_without_commit(records, history)
        self.assertEqual([item.code for item in findings], ["ACCEPTANCE_WITHOUT_COMMIT"])
        self.assertIn("tools/x.py", findings[0].message)

    def test_accepted_with_its_artifacts_committed_passes(self):
        relative = self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.",
                             manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record"),
                                       ("tools/x.py", "create", "Own task deliverable")])
        records, _, _ = self.load()
        history = build_history([Commit("a" * 40, "docs(p0-200): accept", (relative, "tools/x.py"))],
                                committed_paths=[relative, "tools/x.py"])
        self.assertEqual(accepted_without_commit(records, history), [])

    def test_no_commit_history_is_a_notice_not_a_failure(self):
        self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.")
        records, _, _ = self.load()
        findings = accepted_without_commit(records, build_history([]))
        self.assertEqual([item.code for item in findings], ["ACCEPTANCE_COMMIT_UNCHECKED"])
        self.assertEqual(findings[0].level, NOTICE)

    def test_a_path_committed_then_dirtied_by_another_task_is_a_notice(self):
        relative = self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.",
                             manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record"),
                                       ("tools/CMakeLists.txt", "modify", "P0-000 designates P0-200 as this writer")])
        records, _, _ = self.load()
        history = build_history([Commit("a" * 40, "docs(p0-200): accept", (relative, "tools/CMakeLists.txt"))],
                                committed_paths=[relative, "tools/CMakeLists.txt"], dirty_paths=["tools/CMakeLists.txt"])
        findings = accepted_without_commit(records, history)
        self.assertEqual([item.code for item in findings], ["ACCEPTANCE_PATH_DIRTY_AFTER_COMMIT"])
        self.assertEqual(findings[0].level, NOTICE, "the new difference belongs to whoever is writing the path now")
        self.assertIn("belongs to whoever", findings[0].message)

    def test_a_reviewer_line_that_only_repeats_the_template_parenthetical_fails(self):
        self.task("P0-200", "accepted", reviewer="(independent or self-review): none yet")
        records, _, _ = self.load()
        self.assertEqual([item.code for item in accepted_without_review(records)],
                         ["ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW"])
        self.task("P0-201", "accepted", reviewer="p0-014-verifier, a separate read-only agent (independent or self-review)")
        records, _, _ = self.load()
        self.assertEqual([item.code for item in accepted_without_review(records) if item.scope == "P0-201"], [])


class BoardBacklogTests(FixtureTree):
    def test_record_and_board_divergence_is_reported_as_a_notice(self):
        self.task("P0-200", "in_progress", board_status="blocked")
        records, board, _ = self.load()
        findings = check_board_backlog(records, board)
        self.assertEqual([item.code for item in findings], ["RECORD_BOARD_DIVERGENCE"])
        self.assertEqual(findings[0].level, NOTICE)
        self.assertIn("P0-000 syncs", findings[0].message)

    def test_a_waiting_board_sync_request_is_listed_with_its_commit(self):
        self.task("P0-200", "accepted", board_status="in_review",
                  reviewer="one independent read-only subagent review.",
                  board_sync=["P0-200 reached accepted, proving commit deadbeef1"])
        records, board, _ = self.load()
        findings = [item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_WAITING"]
        self.assertEqual(len(findings), 1)
        self.assertIn("deadbeef", findings[0].message)
        self.assertIn("owes the P0-200 row the state accepted", findings[0].message)

    def test_an_applied_board_sync_request_is_not_listed(self):
        self.task("P0-200", "accepted", reviewer="one independent read-only subagent review.",
                  board_sync=["P0-200 reached accepted, proving commit deadbeef1"])
        records, board, _ = self.load()
        self.assertEqual([item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_WAITING"], [])

    def test_a_board_sync_request_without_a_state_is_flagged(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")],
                  board_sync=["the row and the commit will be named here later"])
        records, board, _ = self.load()
        findings = [item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_WAITING"]
        self.assertEqual(len(findings), 1)
        self.assertIn("does not name a state", findings[0].message)

    def test_a_quiet_record_keeps_no_request_from_its_unfilled_template_line(self):
        self.task("P0-200", "planned", template_form=True, board_sync=[""])
        records, board, _ = self.load()
        self.assertEqual(check_board_backlog(records, board), [])

    def test_an_active_record_with_an_unfilled_request_line_is_visible(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")],
                  template_form=True, board_sync=[""])
        self.task("P0-201", "in_progress", manifest=[("docs/tasks/p0-201-fixture.md", "modify", "Own task record")],
                  board_sync=["row <target row>, state <state reached>, commit <proving commit>"])
        records, board, _ = self.load()
        findings = [item for item in check_board_backlog(records, board) if item.code == "BOARD_SYNC_UNFILLED"]
        self.assertEqual(sorted(item.scope for item in findings), ["P0-200", "P0-201"])
        self.assertEqual({item.level for item in findings}, {NOTICE})

    def test_a_fully_typed_request_line_is_not_unfilled(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")],
                  board_sync=["row P0-200 to in_progress, proving commit deadbeef"])
        records, board, _ = self.load()
        self.assertEqual([item.code for item in check_board_backlog(records, board)], [])


class RepositoryIntegrationTests(FixtureTree):
    def test_the_checker_reads_the_board_and_records_and_never_writes_them(self):
        self.task("P0-200", "accepted", reviewer="self-review only")
        path = self.write_board()
        path.write_text(path.read_text(encoding="utf-8") + "\n", encoding="utf-8")
        before = {item: (item.stat().st_mtime_ns, item.read_bytes())
                  for item in sorted(self.root.rglob("*")) if item.is_file()}
        findings, records, board, history = check_repository(self.root)
        after = {item: (item.stat().st_mtime_ns, item.read_bytes())
                 for item in sorted(self.root.rglob("*")) if item.is_file()}
        self.assertEqual(before, after)
        self.assertEqual(list(records), ["P0-200"])
        self.assertEqual(list(board), ["P0-200"])
        self.assertEqual(history.commits, [])
        self.assertNotEqual(history.unavailable, "")
        self.assertIn("MANIFEST_MISSING", self.codes(findings, ERROR))
        self.assertIn("ACCEPTANCE_WITHOUT_INDEPENDENT_REVIEW", self.codes(findings, ERROR))
        self.assertIn("HISTORY_UNAVAILABLE", self.codes(findings, NOTICE))
        self.assertEqual(self.run_main(), 1)

    def test_a_clean_fixture_exits_zero_with_only_notices(self):
        self.task("P0-200", "in_progress", reviewer="one independent read-only subagent review.",
                  manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        self.task("P0-201", "blocked")
        self.assertEqual(self.run_main(), 0)
        findings, _, _, _ = check_repository(self.root)
        self.assertEqual(self.errors(findings), [])

    def test_count_errors_prints_only_the_totals_and_keeps_the_exit_code(self):
        self.task("P0-200", "accepted", reviewer="self-review only")
        with contextlib.redirect_stdout(io.StringIO()) as handle:
            code = main(["--root", str(self.root), "--count-errors"])
        printed = handle.getvalue().strip()
        self.assertEqual(code, 1)
        self.assertRegex(printed, r"^ERRORS \d+ NOTICES \d+$")
        errors = sum(1 for item in check_repository(self.root)[0] if item.level == ERROR)
        self.assertEqual(printed.split()[1], str(errors))

    def test_a_relative_root_is_resolved_before_it_is_used(self):
        self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        previous = Path.cwd()
        os.chdir(self.root)
        try:
            with contextlib.redirect_stdout(io.StringIO()) as handle:
                code = main(["--root", ".", "--count-errors"])
        finally:
            os.chdir(previous)
        self.assertEqual(code, 0, "a relative root reads the same fixture tree; a wrong one would fail on a missing board")
        self.assertRegex(handle.getvalue().strip(), r"^ERRORS 0 NOTICES \d+$")

    def test_a_missing_board_and_an_orphan_record_are_errors(self):
        (self.root / "docs" / "tasks" / "p0-299-orphan.md").write_text(record_text("P0-299", "planned"), encoding="utf-8")
        findings, _, _, _ = check_repository(self.root)
        self.assertIn("BOARD_MISSING", self.codes(findings, ERROR))
        self.assertIn("RECORD_NOT_ON_BOARD", self.codes(findings, ERROR))

    def test_a_board_row_pointing_at_a_missing_record_is_an_error(self):
        self.ghost_row("P0-298", "planned")
        findings, _, _, _ = check_repository(self.root)
        self.assertIn("RECORD_FILE_MISSING", self.codes(findings, ERROR))

    def test_an_undecodable_board_or_record_is_a_diagnostic_not_a_traceback(self):
        relative = self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        (self.root / BOARD_PATH).write_bytes(b"\xff\xfe\x00bad csv")
        findings, _, _, _ = check_repository(self.root)
        self.assertIn("BOARD_UNREADABLE", self.codes(findings, ERROR))
        (self.root / BOARD_PATH).write_text((self.root / BOARD_PATH).read_text(encoding="utf-8", errors="replace"),
                                            encoding="utf-8")
        self.write_board()
        (self.root / relative).write_bytes(b"\xff\xfe not utf-8 at all")
        findings, records, _, _ = check_repository(self.root)
        self.assertIn("RECORD_UNREADABLE", self.codes(findings, ERROR))
        self.assertNotIn("P0-200", records)

    def test_an_invalid_board_status_is_an_error(self):
        self.task("P0-200", "planned", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        path = self.write_board()
        path.write_text(path.read_text(encoding="utf-8").replace("planned", "in-flight"), encoding="utf-8")
        records, board, findings = self.load()
        self.assertIn("BOARD_ROW_IDENTITY", self.codes(findings, ERROR))
        self.assertEqual(records["P0-200"].status, "planned")

    def test_a_record_that_disagrees_with_its_own_task_id_is_an_error(self):
        self.task("P0-200", "planned", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        relative = self.root / "docs" / "tasks" / "p0-200-fixture.md"
        relative.write_text(relative.read_text(encoding="utf-8").replace("- Task ID: P0-200", "- Task ID: P0-201"),
                            encoding="utf-8")
        _, _, findings = self.load()
        self.assertIn("RECORD_ROW_IDENTITY", self.codes(findings, ERROR))

    def test_two_status_lines_that_disagree_are_an_error_instead_of_a_silent_first_read(self):
        relative = self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        path = self.root / relative
        path.write_text(path.read_text(encoding="utf-8") + "\n- Status: accepted\n", encoding="utf-8")
        records, _, findings = self.load()
        self.assertIn("RECORD_ROW_IDENTITY", self.codes(findings, ERROR))
        self.assertEqual(records["P0-200"].status, "in_progress")

    def test_a_repeated_status_line_is_reported_even_when_the_two_agree(self):
        relative = self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        path = self.root / relative
        path.write_text(path.read_text(encoding="utf-8") + "\n- Status: in_progress\n", encoding="utf-8")
        records, _, findings = self.load()
        reported = [item for item in findings if item.code == "RECORD_ROW_IDENTITY"]
        self.assertEqual(len(reported), 1, "one record states one status, twice is a defect even if the two agree")
        self.assertIn("2 `- Status:` lines", reported[0].message)
        self.assertEqual(records["P0-200"].status, "in_progress", "the first line still governs while the defect is fixed")

    def test_a_board_row_pointing_at_a_missing_record_is_reported_once(self):
        self.task("P0-200", "planned", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        (self.root / "docs/tasks/p0-200-fixture.md").unlink()
        findings, _, _, _ = check_repository(self.root)
        missing = [item for item in findings if item.code == "RECORD_FILE_MISSING"]
        self.assertEqual([item.scope for item in missing], ["P0-200"], "one missing record is one finding")

    def test_an_undecodable_record_gets_one_finding(self):
        relative = self.task("P0-200", "in_progress", manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record")])
        (self.root / relative).write_bytes(b"\xff\xfe not utf-8 at all")
        findings, _, _, _ = check_repository(self.root)
        self.assertEqual([item.code for item in findings if item.level == ERROR], ["RECORD_UNREADABLE"])


RECORDED_UNSPECIFIED_SUBJECTS = ("747809e", "d226fa3", "41b6cf8", "54a8a7b", "8f403e4", "fca707a", "a990345",
                                 "e0c1aab", "969842a", "46eb6a4", "c5ad88b", "7b6646b", "26f391e", "7e4bc8e",
                                 "02648bb", "fad7732", "0277df4")
RECORDED_FOREIGN_BOARD_WRITES = ("b28d6f7", "77c3693", "2b1169d", "1646259", "675b92f", "2acd93e", "0277df4")


@unittest.skipUnless(shutil.which("git") and (checker.ROOT / ".git").exists(), "repository git history is unavailable")
class RecordedHistoryReproductionTests(unittest.TestCase):
    """P0-000 audited these findings by hand on 2026-10-31; the checker must reproduce them."""

    @classmethod
    def setUpClass(cls):
        cls.findings, cls.records, cls.board, cls.history = check_repository(checker.ROOT)

    def scopes(self, code, level=None):
        return {item.scope for item in self.findings if item.code == code and level in (None, item.level)}

    def test_the_history_is_read_and_ordered_against_the_section_0_baseline(self):
        self.assertEqual(self.history.baseline[:7], ALLOCATION_BASELINE)
        self.assertGreater(len(self.history.commits), 40)
        self.assertEqual(len(self.history.commits), len({commit.sha for commit in self.history.commits}))
        self.assertGreater(self.history.cutoff, 0)
        self.assertLess(self.history.cutoff, len(self.history.commits))

    def test_every_recorded_subject_without_a_task_id_is_reported(self):
        reported = self.scopes("COMMIT_SUBJECT_TASK_ID")
        for sha in RECORDED_UNSPECIFIED_SUBJECTS:
            self.assertIn(sha, reported, f"{sha} is one of the 17 subjects P0-000 recorded without a task ID")

    def test_the_recorded_post_baseline_findings_are_errors(self):
        errors = self.scopes("COMMIT_SUBJECT_TASK_ID", ERROR)
        self.assertIn("fad7732", errors)
        self.assertIn("0277df4", errors)
        self.assertNotIn("02648bb", errors, "the commit that issued the rules is excluded from them")

    def test_every_recorded_board_write_outside_p0_000_is_an_error(self):
        errors = self.scopes("BOARD_WRITE_OUTSIDE_ALLOCATION", ERROR)
        for sha in RECORDED_FOREIGN_BOARD_WRITES:
            self.assertIn(sha, errors, f"{sha} is a board write P0-000 recorded outside P0-000's scope")

    def test_p0_000_scoped_board_writes_are_not_flagged(self):
        flagged = self.scopes("BOARD_WRITE_OUTSIDE_ALLOCATION")
        for commit in self.history.commits:
            if BOARD_PATH in commit.paths and commit.task_ids() == ["P0-000"]:
                self.assertNotIn(commit.short, flagged)

    def test_tasks_without_manifests_are_named_so_their_owners_can_add_them(self):
        missing = self.scopes("MANIFEST_MISSING", ERROR)
        self.assertTrue(missing)
        for task_id in missing:
            self.assertIn(task_id, self.records)
            self.assertFalse(self.records[task_id].has_manifest)

    def test_the_live_duplicate_claims_are_all_reported_and_nothing_else_is(self):
        """Whatever the live tree claims, the checker must name exactly those conflicts.

        Other tasks add and revise manifests while this runs, so the assertion pins the
        agreement between the local claim set and the findings rather than a snapshot of
        who conflicts today; the conflicts it does report are recorded as rule 9 requests.
        """
        claimed = {}
        for task_id, record in self.records.items():
            if self.board[task_id].status == "canceled":
                continue
            for entry in record.manifest:
                if entry.exact:
                    claimed.setdefault(entry.claim_key, set()).add(task_id)
        conflicts = {key: owners for key, owners in claimed.items() if len(owners) > 1}
        self.assertGreaterEqual(len(claimed), 2, "the repository has manifests to compare")
        flagged = [item for item in self.findings if item.code == "PATH_CLAIM_DUPLICATE"]
        self.assertEqual(sum(len(owners) for owners in conflicts.values()), len(flagged))
        self.assertEqual({item.scope for item in flagged}, {t for owners in conflicts.values() for t in owners})
        for item in flagged:
            match = re.search(r"the path `([^`]+)` is claimed by ([^;]+);", item.message)
            self.assertTrue(match, item.message)
            key, writers = match.group(1).casefold(), {part.strip() for part in match.group(2).split(",")}
            self.assertEqual(conflicts[key], writers, f"{item.scope} is told every rival claimant of {key}")
            self.assertIn(item.scope, writers)

    def test_the_board_is_left_byte_identical_and_backlog_is_reported_only(self):
        before = (checker.ROOT / BOARD_PATH).read_bytes()
        findings = check_repository(checker.ROOT)[0]
        self.assertEqual(before, (checker.ROOT / BOARD_PATH).read_bytes())
        report_only = {"RECORD_BOARD_DIVERGENCE", "BOARD_SYNC_WAITING", "COMMIT_SCOPE_UNVERIFIABLE",
                       "MANIFEST_PATH_NOT_EXACT", "ACCEPTANCE_COMMIT_UNCHECKED", "HISTORY_UNAVAILABLE",
                       "BASELINE_UNRESOLVED"}
        self.assertFalse([item for item in findings if item.code in report_only and item.level == ERROR])


class RealGitHistoryTests(TreeFixture, unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not shutil.which("git"):
            raise unittest.SkipTest("git is unavailable")

    def setUp(self):
        root = Path(tempfile.mkdtemp(prefix="p0014-git-"))
        self.addCleanup(shutil.rmtree, root, True)
        self.init_tree(root)
        self.env = dict(os.environ, HOME=str(root), GIT_CONFIG_GLOBAL=os.devnull,
                        GIT_CONFIG_SYSTEM=os.devnull, GIT_AUTHOR_DATE="2026-10-31T00:00:00+08:00",
                        GIT_COMMITTER_DATE="2026-10-31T00:00:00+08:00")
        self.git("init", "-q", "--initial-branch=main")

    def git(self, *arguments):
        result = subprocess.run(["git", "-c", "user.name=P0-014 fixture", "-c", "user.email=p0-014@example.invalid",
                                 "-c", "commit.gpgsign=false", *arguments], cwd=str(self.root), env=self.env,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, f"git {arguments} failed: {result.stderr}")
        return result.stdout

    def commit(self, subject, *paths):
        for path in paths:
            full = self.root / path
            if full.exists():
                continue
            full.parent.mkdir(parents=True, exist_ok=True)
            full.write_text(f"{path} content\n", encoding="utf-8")
        self.git("add", "--", *paths)
        self.git("commit", "-q", "-m", subject, "--", *paths)

    def head(self):
        return self.git("rev-parse", "HEAD").strip()

    def test_collect_history_reads_subjects_paths_order_and_untracked_state(self):
        self.commit("docs: pre-rule snapshot", BOARD_PATH)
        first = self.head()
        self.commit("docs(p0-200): add the checker", "docs/tasks/p0-200-fixture.md", "tools/x.py")
        (self.root / "tools" / "y.py").write_text("scratch\n", encoding="utf-8")
        history = collect_history(self.root, first[:7])
        self.assertEqual([commit.subject for commit in history.commits],
                         ["docs: pre-rule snapshot", "docs(p0-200): add the checker"])
        self.assertEqual(history.commits[1].paths, ("docs/tasks/p0-200-fixture.md", "tools/x.py"))
        self.assertEqual(history.baseline, first)
        self.assertEqual(history.cutoff, 1)
        self.assertTrue(history.after_baseline(history.commits[1]))
        self.assertFalse(history.after_baseline(history.commits[0]))
        self.assertIn(BOARD_PATH, history.committed_paths)
        self.assertEqual(history.untracked_paths, frozenset({"tools/y.py"}))

    def seed_tasks(self):
        allocation = self.task("P0-000", "in_progress", manifest=[
            (BOARD_PATH, "modify", "Designated to P0-000 by rules 4 and 8"),
            ("AGENTS.md", "modify", "Direct user instruction on 2026-10-31"),
            ("docs/tasks/p0-000-fixture.md", "modify", "Own task record"),
            ("docs/tasks/<new-task-id>-*.md", "create", "Allocation scaffold allowed by rule 2")])
        checker_task = self.task("P0-200", "in_progress", reviewer="one independent read-only subagent review.",
                                 manifest=[("docs/tasks/p0-200-fixture.md", "modify", "Own task record"),
                                           ("tools/x.py", "create", "Own task deliverable")])
        self.commit("docs(p0-000): issue the rules and allocate the task",
                    "AGENTS.md", BOARD_PATH, allocation, checker_task)
        return allocation, checker_task

    def test_a_pathspec_commit_lands_inside_its_own_manifest_and_an_extra_path_does_not(self):
        _, checker_task = self.seed_tasks()
        baseline = self.head()
        self.commit("docs(p0-200): own paths only", checker_task, "tools/x.py")
        self.commit("docs(p0-200): sweeps a foreign file", "docs/research/other-task.md")
        findings, _, _, history = check_repository(self.root, baseline[:7])
        outside = [item for item in findings if item.code == "COMMIT_PATH_OUTSIDE_MANIFEST"]
        offender = [commit.short for commit in history.commits if commit.subject.endswith("foreign file")]
        clean = [commit.short for commit in history.commits if commit.subject.endswith("own paths only")]
        self.assertEqual([item.scope for item in outside], offender)
        self.assertEqual([item.level for item in outside], [ERROR])
        self.assertIn("docs/research/other-task.md", outside[0].message)
        self.assertEqual(set(clean) & {item.scope for item in outside}, set())
        self.assertEqual([item.scope for item in findings if item.code == "COMMIT_SUBJECT_TASK_ID"], [],
                         "every fixture commit carries exactly one task ID")
        self.assertEqual([item.scope for item in findings if item.code == "BOARD_WRITE_OUTSIDE_ALLOCATION"], [],
                         "only P0-000's allocation commit wrote the board")

    def test_an_unresolved_baseline_holds_every_commit_and_reports_it(self):
        allocation, _ = self.seed_tasks()
        self.commit("docs: no task id here", "README.md")
        findings, _, _, history = check_repository(self.root, "02648bb")
        self.assertEqual(history.baseline, "")
        self.assertIn("BASELINE_UNRESOLVED", [item.code for item in findings])
        subject = [item for item in findings if item.code == "COMMIT_SUBJECT_TASK_ID"]
        self.assertEqual([item.level for item in subject], [ERROR])

    def test_a_side_branch_forked_before_the_baseline_stays_a_notice_after_merging(self):
        self.commit("docs(p0-000): pre-rule snapshot", "README.md")
        self.git("checkout", "-q", "-b", "side")
        self.commit("chore: side work with no task id", "docs/research/side.md")
        side = self.head()
        self.git("checkout", "-q", "main")
        self.commit("docs(p0-000): issue the rules", BOARD_PATH)
        baseline = self.head()
        self.git("merge", "-q", "--no-ff", "side", "-m", "Merge branch 'side' into main")
        history = collect_history(self.root, baseline[:7])
        self.assertEqual(history.baseline, baseline)
        by_subject = {commit.subject: commit for commit in history.commits}
        self.assertFalse(history.after_baseline(by_subject["chore: side work with no task id"]),
                         "the side commit never carried the rules in its own history, so merging it later is not a violation")
        self.assertFalse(history.after_baseline(by_subject["docs(p0-000): issue the rules"]),
                         "the commit that issued the rules is excluded from them")
        merge = [commit for commit in history.commits if commit.merge]
        self.assertEqual(len(merge), 1)
        self.assertTrue(history.after_baseline(merge[0]), "a merge made after the baseline is held to it")
        self.assertEqual(sorted((item.scope, item.level) for item in check_commit_subjects(history)),
                         sorted([(side[:7], NOTICE), (merge[0].short, ERROR)]))

    def test_a_board_write_by_another_task_is_an_error_in_a_real_history(self):
        _, checker_task = self.seed_tasks()
        baseline = self.head()
        self.revise_board("P0-200", status="accepted")
        self.commit("docs(p0-200): sync my own row", BOARD_PATH)
        findings, _, _, _ = check_repository(self.root, baseline[:7])
        errors = [item for item in findings if item.level == ERROR]
        self.assertIn("BOARD_WRITE_OUTSIDE_ALLOCATION", [item.code for item in errors])
        self.assertNotIn("BOARD_CLAIM_BY_NON_OWNER", [item.code for item in findings],
                         "the board was written without a manifest claim, which is the commit rule, not the claim rule")


if __name__ == "__main__":
    unittest.main()
