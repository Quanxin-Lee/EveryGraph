import csv
import hashlib
import re
import sys
from collections import Counter
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
INPUT_HASHES = {
    "origin-feature-inventory.md": "fd22138468ea7798ea858c86d5217a2bcd0685ff38f5622ae62fae29db9d0ff8",
    "datagraph-feature-inventory.md": "f5e6876c38cbc90505546ae19dd7786ed45b3a4e9779032de19ad8f6b9ad62fb",
}
REQUIRED_DOCUMENTS = (
    "AGENTS.md", "README.md",
    "docs/planning/project-vision.md",
    "docs/planning/technology-evaluation.md",
    "docs/planning/competitor-research.md",
    "docs/planning/development-roadmap.md",
    "docs/planning/functional-scope.md",
    "docs/architecture/architecture-overview.md",
    "docs/architecture/module-design.md",
    "docs/architecture/data-model.md",
    "docs/architecture/rendering-architecture.md",
    "docs/architecture/concurrency-model.md",
    "docs/architecture/ai-architecture.md",
    "docs/architecture/plugin-architecture.md",
    "docs/architecture/project-file-format.md",
    "docs/decisions/ADR-0001-language-selection.md",
    "docs/decisions/ADR-0002-gui-framework.md",
    "docs/decisions/ADR-0003-application-architecture.md",
    "docs/decisions/ADR-0004-data-storage.md",
    "docs/engineering/development.md",
    "docs/engineering/licensing-and-dependencies.md",
    "docs/engineering/performance-validation.md",
    "docs/engineering/validation-report.md",
)


def validate_rows(rows, sources):
    errors = []
    ids = [row["requirement_id"] for row in rows]
    errors.extend(f"Duplicate requirement: {key}" for key, count in Counter(ids).items() if count > 1)
    missing = set(sources) - set(ids)
    errors.extend(f"Missing source requirement: {key}" for key in sorted(missing))
    for row in rows:
        key = row["requirement_id"]
        if not re.fullmatch(r"(?:ORG|DG|EG)-[A-Z]+-\d{3}", key):
            errors.append(f"Invalid requirement ID: {key}")
        if row["phase"] not in {f"P{i}" for i in range(8)}:
            errors.append(f"Invalid phase: {key}")
        if row["status"] not in {"planned", "unresolved"}:
            errors.append(f"Unexpected Phase-0 delivery claim: {key}")
        if row["delivery"] not in {"native", "adapter", "platform-specific", "research-gate", "constraint"}:
            errors.append(f"Invalid delivery classification: {key}")
        if not row["module"] or len(row["acceptance"]) < 30:
            errors.append(f"Missing owner or acceptance: {key}")
        if key.startswith(("ORG-", "DG-")) and key not in sources:
            errors.append(f"Unknown original requirement: {key}")
        if key in sources:
            expected = sources[key]
            actual = (row["source_file"], row["source_line"], row["capability"])
            if actual != expected:
                errors.append(f"Source reference/title mismatch: {key}")
    return errors


def matrix_scores(rows):
    if not rows or sum(int(row["weight"]) for row in rows) != 100:
        raise ValueError("Matrix weights must sum to 100")
    routes = tuple(key for key in rows[0] if key not in {"criterion", "weight"})
    for row in rows:
        if int(row["weight"]) <= 0 or any(not 1 <= int(row[key]) <= 5 for key in routes):
            raise ValueError("Matrix weights/scores outside the allowed range")
    return {key: sum(int(row["weight"]) * int(row[key]) for row in rows) / 100 for key in routes}


def document_errors(path, root):
    text = path.read_text(encoding="utf-8")
    errors = []
    if len(re.findall(r"^```", text, re.M)) % 2:
        errors.append(f"Unbalanced code fences: {path.relative_to(root)}")
    prose = re.sub(r"^```[^\n]*\n.*?^```\s*$", "", text, flags=re.M | re.S)
    links = re.findall(r"(?<!!)\[[^\]\n]+\]\(([^\s)]+)\)", prose)
    for target in links:
        parsed = urlsplit(target)
        if parsed.scheme or not parsed.path:
            continue
        destination = (path.parent / unquote(parsed.path)).resolve()
        if not destination.is_relative_to(root.resolve()) or not destination.exists():
            errors.append(f"Broken local link: {path.relative_to(root)} -> {target}")
    if re.search(r"[\u3400-\u9fff]", text):
        errors.append(f"Unexpected CJK in normative developer document: {path.relative_to(root)}")
    return errors


def main():
    errors = []
    sources = {}
    for name, expected_hash in INPUT_HASHES.items():
        path = ROOT / name
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected_hash:
            errors.append(f"Preserved inventory changed: {name}")
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            match = re.match(r"^\| ((?:ORG|DG)-[A-Z]+-\d+) \|", line)
            if match:
                sources[match.group(1)] = (name, str(line_number), line.split("|")[3].strip())
    with (ROOT / "docs/planning/feature-traceability.csv").open(encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle))
    errors.extend(validate_rows(rows, sources))
    for row in rows:
        path = (ROOT / row["source_file"]).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file():
            errors.append(f"Invalid source path: {row['requirement_id']}")
            continue
        lines = path.read_text(encoding="utf-8").splitlines()
        line_number = int(row["source_line"])
        if not 1 <= line_number <= len(lines) or not lines[line_number - 1].startswith("|"):
            errors.append(f"Invalid source line: {row['requirement_id']}")
        elif row["requirement_id"].startswith("EG-ADD-") and row["requirement_id"] not in lines[line_number - 1]:
            errors.append(f"Addition anchor mismatch: {row['requirement_id']}")
    for name in REQUIRED_DOCUMENTS:
        path = ROOT / name
        if not path.is_file() or not path.stat().st_size:
            errors.append(f"Missing required document: {name}")
    documents = [ROOT / "AGENTS.md", ROOT / "README.md", *sorted((ROOT / "docs").rglob("*.md"))]
    for path in documents:
        if path.exists():
            errors.extend(document_errors(path, ROOT))
    with (ROOT / "docs/planning/technology-matrix.csv").open(encoding="utf-8", newline="") as handle:
        scores = matrix_scores(list(csv.DictReader(handle)))
    evaluation = (ROOT / "docs/planning/technology-evaluation.md").read_text(encoding="utf-8")
    for route, score in scores.items():
        if f"**{score:.2f}**" not in evaluation:
            errors.append(f"Matrix total missing from evaluation: {route}")
    if errors:
        print("Planning validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"PASS: {len(sources)} original IDs; {len(rows)} traced requirements; {len(documents)} documents")
    print("PASS: input hashes, coverage, source anchors, classifications, local links, fences and English policy")
    print("PASS: matrix weights total 100; " + ", ".join(f"{key}={value:.2f}" for key, value in scores.items()))
    print("Not checked: remote URL availability, Mermaid rendering, scientific semantics or application behavior")
    return 0


if __name__ == "__main__":
    sys.exit(main())
