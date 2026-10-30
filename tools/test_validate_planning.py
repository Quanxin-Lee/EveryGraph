import tempfile
import unittest
from pathlib import Path

from validate_planning import document_errors, matrix_scores, validate_rows


class PlanningValidatorTests(unittest.TestCase):
    def setUp(self):
        self.row = {
            "requirement_id": "ORG-UI-001",
            "source_file": "inventory.md",
            "source_line": "1",
            "capability": "Project Explorer",
            "module": "workspace",
            "phase": "P1",
            "delivery": "native",
            "acceptance": "Create and reopen nested objects with stable identifiers.",
            "status": "planned",
        }
        self.sources = {"ORG-UI-001": ("inventory.md", "1", "Project Explorer")}

    def test_valid_traceability(self):
        self.assertEqual(validate_rows([self.row], self.sources), [])

    def test_duplicate_and_missing_identifiers(self):
        self.assertTrue(any("Duplicate" in item for item in validate_rows([self.row, self.row], self.sources)))
        self.assertTrue(any("Missing" in item for item in validate_rows([], self.sources)))

    def test_source_mismatch_and_false_delivery_claim(self):
        changed = dict(self.row, source_line="2", status="implemented")
        errors = validate_rows([changed], self.sources)
        self.assertTrue(any("mismatch" in item for item in errors))
        self.assertTrue(any("delivery claim" in item for item in errors))

    def test_unknown_phase_and_delivery(self):
        errors = validate_rows([dict(self.row, phase="P9", delivery="magic")], self.sources)
        self.assertEqual(len(errors), 2)

    def test_matrix_arithmetic_and_bounds(self):
        self.assertEqual(matrix_scores([{"criterion": "fit", "weight": "100", "route": "4"}]), {"route": 4})
        with self.assertRaises(ValueError):
            matrix_scores([{"criterion": "fit", "weight": "99", "route": "4"}])
        with self.assertRaises(ValueError):
            matrix_scores([{"criterion": "fit", "weight": "100", "route": "6"}])

    def test_links_fences_and_language(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "document.md"
            path.write_text("# Test\n[Missing](absent.md)\n```text\n", encoding="utf-8")
            errors = document_errors(path, root)
            self.assertTrue(any("Broken local link" in item for item in errors))
            self.assertTrue(any("fences" in item for item in errors))
            path.write_text("# Test\n" + chr(0x4E2D), encoding="utf-8")
            self.assertTrue(any("CJK" in item for item in document_errors(path, root)))


if __name__ == "__main__":
    unittest.main()
