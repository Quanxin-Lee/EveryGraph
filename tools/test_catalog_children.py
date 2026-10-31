"""Unit and integration tests for the catalog-child register validator.

Each case breaks one invariant and asserts that the matching check reports it,
so a passing register is evidence of a working test rather than a no-op.
"""
import json
import re
import shutil
import socket
import tempfile
import unittest
from pathlib import Path

import validate_catalog_children as validator
from validate_catalog_children import (ROOT, count_rows, excerpt_digest, load_expectations, normalize,
                                      read_csv_rows, validate_all, validate_anchors, validate_claim_discipline,
                                      validate_counts, validate_document_consistency, validate_fields,
                                      validate_file_form, validate_ids, validate_immutability,
                                      validate_no_duplicate_names, validate_parents, validate_schema)


class CatalogChildValidatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.expectations = load_expectations()
        cls.rows = read_csv_rows(ROOT / cls.expectations["child_document"]["path"])
        cls.register = read_csv_rows(ROOT / cls.expectations["register"]["path"])

    def setUp(self):
        self.base = json.loads(json.dumps(self.expectations))

    def child(self, child_id):
        for row in self.rows:
            if row["child_id"] == child_id:
                return dict(row)
        raise AssertionError(f"unknown child id {child_id}")

    def copy_with(self, row):
        rows = [dict(item) for item in self.rows]
        for index, item in enumerate(rows):
            if item["child_id"] == row["child_id"]:
                rows[index] = row
        return rows

    def mirror(self, *names):
        root = Path(tempfile.mkdtemp())
        for name in names:
            (root / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(ROOT / name, root / name)
        return root

    def test_committed_register_passes_every_check(self):
        self.assertEqual(validate_all(self.rows, self.register, self.expectations), [])

    def test_id_grammar_duplicates_and_sequence_retirements(self):
        broken = self.child("EG-CAT-IO-001")
        broken["child_id"] = "EG-CAT-io-01"
        self.assertTrue(any("grammar" in item for item in validate_ids([broken], self.base)))
        duplicated = validate_ids([self.child("EG-CAT-IO-001"), self.child("EG-CAT-IO-001")], self.base)
        self.assertTrue(any("Duplicate child ID" in item for item in duplicated))
        rows = [row for row in self.rows if row["child_id"] != "EG-CAT-IO-020"]
        errors = validate_ids(rows, self.base)
        self.assertTrue(any("sequence gap without a recorded retirement" in item and "EG-CAT-IO-020" in item
                            for item in errors), errors)
        self.base["child_document"]["retired_child_ids"] = ["EG-CAT-IO-020"]
        self.assertEqual(validate_ids(rows, self.base), [])
        rows = rows + [self.child("EG-CAT-IO-020")]
        self.assertTrue(any("Retired child ID reused" in item for item in validate_ids(rows, self.base)))

    def test_parent_must_be_a_known_unresolved_catalog_parent(self):
        moved = self.child("EG-CAT-IO-001")
        moved["catalog_parent"] = "ORG-IO-002"
        self.assertTrue(any("outside the seven catalog parents" in item
                            for item in validate_parents([moved], self.register, self.base)))
        planned = [dict(row) for row in self.register]
        for row in planned:
            if row["requirement_id"] == "ORG-IO-001":
                row["status"] = "planned"
        errors = validate_parents([self.child("EG-CAT-IO-001")], planned, self.base)
        self.assertTrue(any("would imply a delivery claim" in item for item in errors), errors)

    def test_register_family_must_exist_in_the_register(self):
        row = self.child("EG-CAT-IO-001")
        row["register_family"] = "ORG-NOPE-999"
        self.assertTrue(any("Register family missing" in item
                            for item in validate_parents([row], self.register, self.base)))

    def test_required_fields_enums_and_placeholders(self):
        cases = (("rights_status", "R-SECRET", "Invalid rights_status"),
                 ("membership_status", "named ", "Invalid membership_status"),
                 ("named_entry", "TBD", "placeholder required field"),
                 ("prerequisites", "", "Empty or placeholder required field"))
        for key, value, fragment in cases:
            errors = validate_fields(self.copy_with(dict(self.child("EG-CAT-IO-001"), **{key: value})), self.base)
            self.assertTrue(any(fragment in item for item in errors), (key, errors))

    def test_member_type_must_match_its_catalog_area(self):
        gate = dict(self.child("EG-CAT-GATE-007"), member_type="file-format")
        errors = validate_fields(self.copy_with(gate), self.base)
        self.assertTrue(any("does not belong to its catalog area" in item for item in errors), errors)

    def test_floors_are_scoped_to_their_own_catalog(self):
        """A gate row must not be able to fill an import floor by relabelling."""
        io_row = dict(self.child("EG-CAT-IO-020"), member_type="gate")
        gate_row = dict(self.child("EG-CAT-GATE-007"), member_type="file-format")
        rows = self.copy_with(io_row)
        rows = self.copy_with_in(gate_row, rows)
        errors = validate_counts(rows, self.base)
        self.assertTrue(any("IO_FORMAT_FAMILIES_MIN" in item for item in errors), errors)
        self.assertTrue(any("GATE_ROWS" in item for item in errors), errors)
        fields = validate_fields(rows, self.base)
        self.assertTrue(any("does not belong to its catalog area" in item for item in fields), fields)

    def copy_with_in(self, row, rows):
        for index, item in enumerate(rows):
            if item["child_id"] == row["child_id"]:
                rows[index] = row
        return rows

    def test_ascii_policy_applies_outside_the_two_source_fields(self):
        for key, marker in (("named_entry", "Kana"), ("prerequisites", "Hangul"), ("variant_parameters", "Ideograph")):
            text = {"Kana": chr(0x3042), "Hangul": chr(0xac00), "Ideograph": chr(0x4e2d)}[marker]
            errors = validate_fields(self.copy_with(dict(self.child("EG-CAT-IO-001"), **{key: text})), self.base)
            self.assertTrue(any("Non-ASCII text outside the exempt anchor fields" in item for item in errors),
                            (key, errors))
        errors = validate_fields(self.copy_with(dict(self.child("EG-CAT-IO-001"), source_name=chr(0x4e2d))), self.base)
        self.assertFalse(any("Non-ASCII" in item for item in errors), errors)

    def test_unenumerated_rows_must_state_their_gap(self):
        silent = dict(self.child("EG-CAT-PEAK-001"), variant_parameters="Some parameters and variants")
        self.assertTrue(any("must state the gap in variant_parameters" in item
                            for item in validate_fields([silent], self.base)))

    def test_fixture_and_output_codes_are_required(self):
        row = self.copy_with(dict(self.child("EG-CAT-IO-001"), fixture_plan="a scene corpus"))
        self.assertTrue(any("fixture_plan must open with an F-* code" in item for item in validate_fields(row, self.base)))

    def test_source_name_must_be_present_in_the_anchored_excerpt(self):
        invented = dict(self.child("EG-CAT-GRAPH-003"), source_name="Fabricated Graph Type",
                        named_entry="Fabricated Graph Type")
        self.assertTrue(any("Source name is not present in the anchored excerpt" in item
                            for item in validate_anchors([invented], self.base)))
        local = dict(self.child("EG-CAT-IO-001"), source_name="Fabricated Import Format")
        self.assertTrue(any("Source name is not present in the anchored excerpt" in item
                            for item in validate_anchors([local], self.base)))
        claim = dict(self.child("EG-CAT-PEAK-001"), source_name="Fabricated Library Name")
        self.assertEqual(validate_anchors([claim], self.base), [])

    def test_excerpt_hash_follows_the_normalized_excerpt(self):
        row = self.child("EG-CAT-IO-001")
        row["source_excerpt"] = row["source_excerpt"] + "  "
        self.assertEqual(validate_anchors([row], self.base), [])
        row["source_excerpt"] += " silently reworded"
        row["source_excerpt_sha256"] = excerpt_digest(row["source_excerpt"])
        self.assertTrue(any("not verbatim text of the anchored line" in item
                            for item in validate_anchors([row], self.base)))

    def test_local_anchor_drift_and_bounds(self):
        row = self.child("EG-CAT-IO-001")
        root = self.mirror(self.expectations["local_sources"]["S1"], self.expectations["local_sources"]["S2"],
                           self.expectations["local_sources"]["S9"])
        try:
            pinned = root / row["source_file"]
            lines = pinned.read_text(encoding="utf-8").splitlines()
            lines[int(row["source_line"]) - 1] = "| ORG-IO-002 | rewritten | rewritten | rewritten |"
            pinned.write_text("\n".join(lines), encoding="utf-8")
            self.assertTrue(any("not verbatim text" in item for item in validate_anchors([row], self.base, root)))
            drifted = dict(row, source_line=str(len(lines) + 5))
            self.assertTrue(any("line out of range" in item for item in validate_anchors([drifted], self.base, root)))
            pinned.unlink()
            self.assertTrue(any("Local anchor file missing" in item
                                for item in validate_anchors([row], self.base, root)))
        finally:
            shutil.rmtree(root)
        mismatched = dict(row, source_file="datagraph-feature-inventory.md")
        self.assertTrue(any("does not match evidence code" in item for item in validate_anchors([mismatched], self.base)))

    def test_external_anchor_url_date_and_registry_rules(self):
        row = self.child("EG-CAT-GRAPH-003")
        cases = (dict(row, source_url="http://originlab.example/GraphGallery", marker="not HTTPS"),
                 dict(row, source_url="https://example.com/other", marker="not in the registered page set"),
                 dict(row, source_checked="2031-01-01", marker="on or before today"),
                 dict(row, source_checked="20261031", marker="must be YYYY-MM-DD"),
                 dict(row, source_checked="2020-01-01", marker="differs from the registered S6 date"),
                 dict(row, evidence_code="S99", marker="Evidence code is not in the source register"))
        for case in cases:
            marker = case.pop("marker")
            errors = validate_anchors([case], self.base)
            self.assertTrue(any(marker in item for item in errors), (marker, errors))
        mismatched = dict(row, source_url=self.base["external_sources"]["S5"]["url"], source_name="Beeswarm Plot")
        self.assertTrue(any("External URL differs from the registered S6 URL" in item
                           for item in validate_anchors([mismatched], self.base)))

    def test_count_floor_breaks_when_a_named_row_is_deleted(self):
        rows = [row for row in self.rows if row["child_id"] != "EG-CAT-GRAPH-003"]
        self.assertTrue(any("GRAPH_NAMED_MIN" in item for item in validate_counts(rows, self.base)))

    def test_claim_only_row_cannot_satisfy_a_named_floor(self):
        row = dict(self.child("EG-CAT-GRAPH-003"), membership_status="claim_only")
        errors = validate_counts(self.copy_with(row), self.base)
        self.assertTrue(any("GRAPH_NAMED_MIN" in item for item in errors), errors)
        self.assertTrue(any("GRAPH_CLAIM_ONLY" in item for item in errors), errors)

    def test_declared_counts_are_exact(self):
        self.base["counts"]["EXPORT_NAMED"] += 1
        self.assertTrue(any("EXPORT_NAMED" in item for item in validate_counts(self.rows, self.base)))
        self.base["counts"]["TOTAL_ROWS"] += 1
        self.assertTrue(any("Register row count" in item for item in validate_counts(self.rows, self.base)))

    def test_no_silent_rights_elevation(self):
        for key, value in (("support_claim", "planned"), ("expected_output", "O-TABLE; typed table")):
            row = dict(self.child("EG-CAT-COMPAT-003"), **{key: value})
            self.assertTrue(any("elevates support or output" in item
                                for item in validate_claim_discipline([row], self.base)), key)

    def test_claim_only_rows_stay_gates_without_output(self):
        for key, value in (("delivery_class", "D-ADAPTER"), ("expected_output", "O-SCENE; scene checks")):
            row = dict(self.child("EG-CAT-FIT-001"), **{key: value})
            self.assertTrue(any("output-free gate" in item for item in validate_claim_discipline([row], self.base)), key)
        self.assertEqual(validate_claim_discipline([self.child("EG-CAT-PEAK-001")], self.base), [])

    def test_no_row_makes_a_support_claim(self):
        row = dict(self.child("EG-CAT-IO-001"), support_claim="claimed")
        self.assertTrue(any("makes a support claim" in item for item in validate_claim_discipline([row], self.base)))

    def test_vendor_count_needs_a_claim_row(self):
        for value in ("Over 100+ graph types", "more than 200 fitting functions", "close to 200 models",
                      "100 graph types per vendor"):
            inflated = dict(self.child("EG-CAT-IO-001"), named_entry=value)
            self.assertTrue(any("vendor count without a claim row" in item
                                for item in validate_claim_discipline([inflated], self.base)), value)
        for value in ("2D Gaussian surface function", "Binary 2D Array IMG / B2D"):
            row = dict(self.child("EG-CAT-IO-033"), named_entry=value)
            self.assertEqual(validate_claim_discipline([row], self.base), [], value)
        claim = dict(self.child("EG-CAT-GRAPH-074"), named_entry="Over 100+ graph types")
        self.assertEqual(validate_claim_discipline([claim], self.base), [])

    def test_duplicate_named_entries_within_a_catalog(self):
        duplicate = dict(self.child("EG-CAT-GRAPH-003"), child_id="EG-CAT-GRAPH-096")
        self.assertTrue(any("Duplicate named entry in catalog GRAPH" in item
                            for item in validate_no_duplicate_names(self.rows + [duplicate])))
        spaced = dict(self.child("EG-CAT-GRAPH-003"), child_id="EG-CAT-GRAPH-096", named_entry=" beeswarm  plot ")
        self.assertTrue(any("Duplicate named entry" in item
                            for item in validate_no_duplicate_names(self.rows + [spaced])))
        swapped = [row for row in self.rows if row["child_id"] != "EG-CAT-GRAPH-004"] + [duplicate]
        self.assertTrue(any("Duplicate named entry" in item for item in validate_no_duplicate_names(swapped)))

    def test_missing_or_renamed_columns_are_reported_not_raised(self):
        damaged = [{key: value for key, value in self.rows[0].items() if key != "membership_status"}] + self.rows[1:]
        self.assertTrue(any("lacks columns" in item and "membership_status" in item
                            for item in validate_schema(damaged, self.base)))
        self.assertTrue(any("lacks columns" in item for item in validate_all(damaged, self.register, self.base)))
        extra = [dict(row, surprise="x") for row in self.rows]
        self.assertTrue(any("undeclared columns" in item for item in validate_schema(extra, self.base)))

    def test_register_and_inventory_immutability(self):
        names = [self.expectations["register"]["path"], self.expectations["child_document"]["path"]]
        names += list(self.expectations["protected_inputs"])
        root = self.mirror(*names)
        try:
            self.assertEqual(validate_immutability(self.register, self.base, root), [])
            register_name = root / self.expectations["register"]["path"]
            register_name.write_text(register_name.read_text(encoding="utf-8") + "\nEG-CAT-IO-001,extra",
                                     encoding="utf-8")
            errors = validate_immutability(read_csv_rows(register_name), self.base, root)
            self.assertTrue(any("hash changed" in item for item in errors), errors)
            self.assertTrue(any("row count changed" in item for item in errors), errors)
            self.assertTrue(any("leaked into the feature register" in item for item in errors), errors)
            citing = [dict(row, source_file=self.expectations["child_document"]["path"]) for row in self.register]
            self.assertTrue(any("cites the child register" in item
                                for item in validate_immutability(citing, self.base, root)))
            inventory = root / "datagraph-feature-inventory.md"
            inventory.write_text(inventory.read_text(encoding="utf-8") + "\nappended", encoding="utf-8")
            self.assertTrue(any("Preserved inventory changed" in item
                                for item in validate_immutability(self.register, self.base, root)))
        finally:
            shutil.rmtree(root)

    def test_child_register_is_pinned_against_silent_rewrites(self):
        names = [self.expectations["register"]["path"], self.expectations["child_document"]["path"]]
        names += list(self.expectations["protected_inputs"])
        root = self.mirror(*names)
        try:
            register = root / self.expectations["child_document"]["path"]
            text = register.read_text(encoding="utf-8").replace("Beeswarm Plot", "Beeswarm Plot Renamed", 1)
            register.write_text(text, encoding="utf-8")
            self.assertTrue(any("pinned digest" in item
                                for item in validate_immutability(self.register, self.base, root)))
        finally:
            shutil.rmtree(root)

    def test_file_form_contract_is_read(self):
        self.assertEqual(validate_file_form(self.base), [])
        for key, value, fragment in (("row_terminator", "CRLF", "line terminator is LF"),
                                     ("schema_version", 99, "Unsupported expectations schema_version"),
                                     ("encoding", "utf-16", "encoding declaration")):
            mutated = json.loads(json.dumps(self.base))
            if key == "schema_version":
                mutated[key] = value
            else:
                mutated["child_document"][key] = value
            self.assertTrue(any(fragment in item for item in validate_file_form(mutated)), (key, fragment))

    def test_contract_completeness_is_enforced(self):
        rows = [dict(row, evidence_code="Z1") for row in self.rows]
        self.assertTrue(any("Row cites an unregistered page" in item
                            for item in validate_document_consistency(rows, self.base)))
        self.base["external_sources"]["S5"].pop("reference_only", None)
        self.assertTrue(any("S5 is cited by no row and is not marked reference_only" in item
                            for item in validate_document_consistency(self.rows, self.base)))
        self.base["evidence_document"] = "docs/research/capability-catalog-decomposition.md"
        self.assertTrue(any("superseded singular evidence_document" in item
                            for item in validate_document_consistency(self.rows, self.base)))
        self.base["notice"] = ""
        self.assertTrue(any("evidence entries, not implementation" in item
                            for item in validate_document_consistency(self.rows, self.base)))

    def test_declared_constant_drift_against_the_evidence_documents(self):
        self.base["p0_003_declared"]["GATE_ROWS"] += 1
        self.assertTrue(any("P0-003 declared constant drift for GATE_ROWS" in item
                            for item in validate_document_consistency(self.rows, self.base)))
        self.base = json.loads(json.dumps(self.expectations))
        self.base["p0_005"]["declared_floors"]["FIT_NUMBERED_PAGES"] += 1
        self.assertTrue(any("P0-005 declared floor drift for FIT_NUMBERED_PAGES" in item
                            for item in validate_document_consistency(self.rows, self.base)))
        self.base = json.loads(json.dumps(self.expectations))
        del self.base["p0_005"]["declared_floors"]["PEAK_1D_NAMED"]
        self.assertTrue(any("PEAK_1D_NAMED" in item for item in validate_document_consistency(self.rows, self.base)))

    def test_a_declared_floor_can_rise_but_never_silently_fall(self):
        self.base["counts"]["GATE_ROWS"] -= 1
        self.assertTrue(any("floor was lowered without a decision" in item
                            for item in validate_document_consistency(self.rows, self.base)))

    def test_named_additions_and_revised_control_rows_must_be_present(self):
        self.base["p0_005"]["graph_additions"] += 1
        self.assertTrue(any("section 6.4 rows changed" in item
                            for item in validate_document_consistency(self.rows, self.base)))
        self.base = json.loads(json.dumps(self.expectations))
        rows = [row for row in self.rows if row["child_id"] != "EG-CAT-FIT-002"]
        self.assertTrue(any("revised control row missing" in item
                            for item in validate_document_consistency(rows, self.base)))
        rows = [row for row in self.rows if row["child_id"] != "EG-CAT-GATE-007"]
        self.assertTrue(any("Count table drift for GATE" in item
                            for item in validate_document_consistency(rows, self.base)))

    def test_p0_012_declared_additions_reconcile_and_surface_rows_stay_unadmitted(self):
        self.assertEqual(validate_document_consistency(self.rows, self.base), [])
        config = self.base["p0_012"]
        document = (ROOT / self.base["evidence_documents"]["P0-012"]).read_text(encoding="utf-8")
        surface_rows = validator.markdown_rows(
            document,
            config["table_headings"]["SURFACE"],
            config["table_columns"]["SURFACE"],
            stop=config["table_stops"]["SURFACE"],
        )
        surface_ids = {row[0] for row in surface_rows}
        s11_ids = {row["child_id"] for row in self.rows if row["evidence_code"] == config["source_code"]}
        registered_ids = {row["child_id"] for row in self.rows}
        retained_ids = set(config["retained_surface_ids"])
        surface_floor = self.base["p0_005"]["declared_floors"][config["floor_keys"]["SURFACE"]]

        self.assertEqual(len(surface_ids), surface_floor)
        self.assertTrue(surface_ids.isdisjoint(s11_ids))
        self.assertTrue((surface_ids - retained_ids).isdisjoint(registered_ids))
        self.assertEqual(retained_ids, {"EG-CAT-FIT-002"})
        retained = next(row for row in self.rows if row["child_id"] == "EG-CAT-FIT-002")
        self.assertEqual(retained["evidence_code"], "F8")

    def test_p0_012_rejects_a_child_not_declared_by_its_source_tables(self):
        undeclared = self.child("EG-CAT-FIT-003")
        undeclared["child_id"] = "EG-CAT-FIT-242"
        errors = validate_document_consistency(self.rows + [undeclared], self.base)
        self.assertTrue(any("P0-012 admitted child IDs differ" in item and
                            "EG-CAT-FIT-242" in item for item in errors), errors)

    def test_p0_012_rejects_a_wrong_registered_count(self):
        rows = [row for row in self.rows if row["child_id"] != "EG-CAT-FIT-003"]
        errors = validate_document_consistency(rows, self.base)
        self.assertTrue(any("Count table drift for FIT" in item for item in errors), errors)

    def test_p0_005_sourced_rows_anchor_on_registered_pages(self):
        for child_id in ("EG-CAT-FIT-001", "EG-CAT-PEAK-001", "EG-CAT-GRAPH-074", "EG-CAT-GRAPH-080", "EG-CAT-GRAPH-095"):
            row = self.child(child_id)
            self.assertEqual(row["primary_anchor"], "external", child_id)
            self.assertIn(row["evidence_code"], self.expectations["external_sources"], child_id)
            self.assertEqual(row["source_url"], self.expectations["external_sources"][row["evidence_code"]]["url"])
            self.assertEqual(validate_anchors([row], self.base), [])

    def test_main_reports_success_on_the_committed_register(self):
        self.assertEqual(validator.main(), 0)

    def test_default_run_never_opens_a_network_socket(self):
        def guard(*args, **kwargs):
            raise AssertionError("the default validation run must not use the network")
        original = socket.socket, socket.create_connection
        socket.socket, socket.create_connection = guard, guard
        try:
            self.assertEqual(validate_all(self.rows, self.register, self.expectations), [])
        finally:
            socket.socket, socket.create_connection = original

    def test_the_module_imports_no_network_capable_library(self):
        sources = (ROOT / "tools/validate_catalog_children.py").read_text(encoding="utf-8")
        modules = re.findall(r"^(?:import|from)\s+([a-zA-Z0-9_]+)", sources, re.M)
        self.assertFalse({"socket", "ssl", "http", "urllib", "requests"} & set(modules), modules)

    def test_claim_only_row_cannot_be_relabelled_to_fill_a_floor(self):
        claim = dict(self.child("EG-CAT-GRAPH-074"), member_type="file-format")
        rows = self.copy_with(claim)
        errors = validate_counts(rows, self.base)
        self.assertFalse(any("IO_FORMAT_FAMILIES_MIN" in item for item in errors), errors)
        self.assertTrue(any("Member type does not belong to its catalog area" in item
                            for item in validate_fields(rows, self.base))
                        or any("IO_FORMAT_FAMILIES_MIN" in item for item in errors), errors)
        dropped = [row for row in self.rows if row["child_id"] != "EG-CAT-IO-020"] + [claim]
        self.assertTrue(any("IO_FORMAT_FAMILIES_MIN" in item for item in validate_counts(dropped, self.base)))

    def test_local_anchor_verification_date_is_pinned(self):
        row = dict(self.child("EG-CAT-IO-001"), source_checked="2020-02-02")
        errors = validate_anchors([row], self.base)
        self.assertTrue(any("Local anchor verification date differs" in item for item in errors), errors)

    def test_normalization_helpers_are_shared_by_hash_and_uniqueness(self):
        self.assertEqual(normalize("  a\t b \n c"), "a b c")
        self.assertEqual(excerpt_digest("a  b"), excerpt_digest("a b"))
        named, claims, counts = count_rows(self.rows)
        self.assertEqual(sum(counts.values()) + sum(claims.values()), len(self.rows))
        self.assertEqual(sum(named.values()) + sum(claims.values()) + sum(
            1 for row in self.rows if row["membership_status"] == "open"), len(self.rows))


if __name__ == "__main__":
    unittest.main()
