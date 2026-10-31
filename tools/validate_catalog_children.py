"""Check the catalog-child register against the P0-003 contract, offline.

The register at docs/planning/catalog-children.csv mirrors the reviewed child
entries of the accepted P0-003 and P0-005 evidence documents. It is an evidence
index, not a feature list: these checks prove identifier, parent, field, anchor,
count, rights, document-consistency and immutability invariants. No URL is
fetched in the default run.
"""
import csv
import hashlib
import json
import re
import sys
import unicodedata
from collections import Counter
from datetime import date
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPECTATIONS_PATH = "tools/catalog_expectations.json"
SUPPORTED_SCHEMA_VERSION = 1
RIGHTS = {"R-OPEN", "R-VENDOR", "R-BLOCKED", "R-OWN"}
DELIVERY = {"D-NATIVE", "D-ADAPTER", "D-GATE", "D-PLATFORM"}
MEMBERSHIP = {"named", "claim_only", "open", "blocked"}
SUPPORT = {"none", "planned", "claimed"}
UNCERTAINTY = {"U1", "U2", "U3", "U4"}
ANCHOR_KINDS = {"local", "external"}
OPTIONAL_FIELDS = {"source_file", "source_line", "source_url", "source_checked", "primary_anchor"}
PLACEHOLDERS = {"tbd", "n/a", "na", "-", "todo", "?"}
AREA_TYPES = {"IO": {"file-format", "source-adapter"}, "GRAPH": {"graph-type", "catalog-claim"},
              "FIT": {"fit-function", "catalog-claim"}, "PEAK": {"peak-function", "catalog-claim"},
              "EXPORT": {"export-format", "catalog-claim"}, "COMPAT": {"compat-artifact"}, "GATE": {"gate"}}
MEMBER_TYPES = {value for allowed in AREA_TYPES.values() for value in allowed}
CJK = re.compile(r"[\u3400-\u9fff]")
NON_ASCII = re.compile(r"[^\x00-\x7f]")
CHILD_GRAMMAR = re.compile(r"EG-CAT-(?:IO|GRAPH|FIT|PEAK|EXPORT|COMPAT|GATE)-\d{3}")
AREA_OF = re.compile(r"EG-CAT-([A-Z]+)-(\d{3})")
ISO_DATE = re.compile(r"\d{4}-\d{2}-\d{2}")
INFLATION = re.compile(r"(?:\b(?:over|more than|close to|nearly|at least|up to|about)\s+\d+|\b\d{2,}\s*\+|\b\d{3,}\b)")
GAP_STATEMENT = re.compile(r"(?i)(not enumerated|not published|not documented|unreconciled|unconfirmed|claim)")
HEADING_LABELS = {"Child ID", "Constant", "ID", "Catalog", "Code", "Named type", "Evidence class", "Column",
                  "Formulation", "Measure", "Gate", "Claim", "Field", "State", "Item", "Finding", "Requirement"}


def normalize(text):
    return re.sub(r"\s+", " ", unicodedata.normalize("NFC", text or "")).strip()


def excerpt_digest(text):
    return hashlib.sha256(normalize(text).encode("utf-8")).hexdigest()


def load_expectations(root=ROOT):
    return json.loads((root / EXPECTATIONS_PATH).read_text(encoding="utf-8"))


def read_csv_rows(path):
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def area_of(child_id):
    match = AREA_OF.fullmatch(child_id or "")
    return match.group(1) if match else "?"


def parent_areas(expectations):
    names = {"ORG-IO-001": "IO", "ORG-GRAPH-001": "GRAPH", "ORG-FIT-004": "FIT", "ORG-PEAK-005": "PEAK",
             "ORG-EXPORT-001": "EXPORT", "EG-ADD-014": "COMPAT", "EG-ADD-015": "GATE"}
    missing = [parent for parent in expectations["register"]["parents"] if parent not in names]
    if missing:
        raise ValueError(f"Parent without a catalog area: {missing}")
    return {parent: names[parent] for parent in expectations["register"]["parents"]}


def validate_schema(rows, expectations):
    """Report structural damage first so dependent checks cannot crash on it."""
    columns = expectations["child_document"]["columns"]
    errors = []
    for index, row in enumerate(rows, 1):
        absent = [key for key in columns if key not in row or row[key] is None]
        if absent:
            errors.append(f"Row {index} lacks columns: {absent}")
        extra = [key for key in row if key not in columns]
        if extra:
            errors.append(f"Row {index} has undeclared columns: {extra}")
    return errors


def validate_ids(rows, expectations):
    errors = [f"Invalid child ID grammar: {row.get('child_id') or '(blank)'}" for row in rows
              if not CHILD_GRAMMAR.fullmatch(row.get("child_id") or "")]
    errors += [f"Duplicate child ID: {key}" for key, count in Counter(row.get("child_id") for row in rows).items()
               if count > 1]
    retired = set(expectations["child_document"].get("retired_child_ids", []))
    errors += [f"Retired child ID reused: {child_id}" for child_id in sorted(retired)
               if any(row.get("child_id") == child_id for row in rows)]
    highest = {}
    for child_id in {row["child_id"] for row in rows if CHILD_GRAMMAR.fullmatch(row["child_id"] or "")}:
        area, number = AREA_OF.fullmatch(child_id).groups()
        highest[area] = max(highest.get(area, 0), int(number))
    seen = {row["child_id"] for row in rows}
    for area, top in sorted(highest.items()):
        for number in range(1, top + 1):
            child_id = f"EG-CAT-{area}-{number:03d}"
            if child_id not in seen and child_id not in retired:
                errors.append(f"Child ID sequence gap without a recorded retirement: {child_id}")
    return errors


def validate_parents(rows, register_rows, expectations):
    errors = []
    register = {row["requirement_id"]: row for row in register_rows}
    parents = set(expectations["register"]["parents"])
    wanted = expectations["register"]["parent_status"]
    for row in rows:
        child_id, parent = row["child_id"], row["catalog_parent"]
        if parent not in parents:
            errors.append(f"Parent outside the seven catalog parents: {child_id} -> {parent or '(blank)'}")
        elif register[parent]["status"] != wanted:
            errors.append(f"Parent status is not {wanted}, which would imply a delivery claim: {parent}")
        family = row["register_family"]
        if family and family not in register:
            errors.append(f"Register family missing from the feature register: {child_id} -> {family}")
    return errors


def validate_fields(rows, expectations):
    errors = []
    columns = expectations["child_document"]["columns"]
    exempt = set(expectations["child_document"]["english_exempt_columns"])
    for row in rows:
        child_id = row["child_id"]
        for key in columns:
            if key in OPTIONAL_FIELDS:
                continue
            value = (row.get(key) or "").strip()
            if not value or value.lower() in PLACEHOLDERS:
                errors.append(f"Empty or placeholder required field {key}: {child_id}")
        for key, allowed in (("rights_status", RIGHTS), ("delivery_class", DELIVERY), ("member_type", MEMBER_TYPES),
                             ("membership_status", MEMBERSHIP), ("support_claim", SUPPORT),
                             ("uncertainty", UNCERTAINTY), ("primary_anchor", ANCHOR_KINDS)):
            if row.get(key) and row[key] not in allowed:
                errors.append(f"Invalid {key} value: {child_id} -> {row[key]}")
        if row["member_type"] not in AREA_TYPES.get(area_of(row["child_id"]), set()):
            errors.append(f"Member type does not belong to its catalog area: {child_id} -> {row['member_type']}")
        for key, prefix in (("fixture_plan", "F"), ("expected_output", "O")):
            if not re.match(rf"{prefix}-[A-Z]+(;|\b)", row.get(key, "")):
                errors.append(f"{key} must open with an {prefix}-* code: {child_id}")
        for key, value in row.items():
            if key in exempt or not value:
                continue
            if NON_ASCII.search(value):
                errors.append(f"Non-ASCII text outside the exempt anchor fields: {child_id} [{key}]")
        if row["uncertainty"] == "U3" or row["membership_status"] in {"claim_only", "open"}:
            if not GAP_STATEMENT.search(row["variant_parameters"]):
                errors.append(f"Unenumerated row must state the gap in variant_parameters: {child_id}")
    return errors


def validate_anchors(rows, expectations, root=ROOT):
    errors = []
    local_sources, externals = expectations["local_sources"], expectations["external_sources"]
    registered_urls = {item["url"]: code for code, item in externals.items()}
    lines = {}
    for row in rows:
        child_id = row["child_id"]
        if excerpt_digest(row.get("source_excerpt", "")) != row.get("source_excerpt_sha256", ""):
            errors.append(f"Excerpt hash mismatch: {child_id}")
        if not normalize(row.get("source_name", "")):
            errors.append(f"Empty source name: {child_id}")
        elif (row["membership_status"] not in {"claim_only", "open"}
              and normalize(row["source_name"]).lower() not in normalize(row["source_excerpt"]).lower()):
            errors.append(f"Source name is not present in the anchored excerpt: {child_id}")
        code = row.get("evidence_code", "")
        if code not in local_sources and code not in externals:
            errors.append(f"Evidence code is not in the source register: {child_id} -> {code or '(blank)'}")
        if row.get("source_url"):
            if not row["source_url"].startswith("https://"):
                errors.append(f"Source URL is not HTTPS: {child_id}")
            elif row["source_url"] not in registered_urls:
                errors.append(f"Source URL is not in the registered page set: {child_id}")
        if not ISO_DATE.fullmatch(row.get("source_checked") or ""):
            errors.append(f"source_checked must be YYYY-MM-DD: {child_id} -> {row.get('source_checked')}")
        elif not valid_date(row["source_checked"]):
            errors.append(f"source_checked is not a real date on or before today: {child_id}")
        anchor = row.get("primary_anchor")
        if anchor == "local":
            if row.get("source_checked") != expectations.get("verified_inputs_checked"):
                errors.append(f"Local anchor verification date differs from the registered "
                              f"{expectations.get('verified_inputs_checked')}: {child_id}")
            errors += local_anchor_errors(row, local_sources, root, lines, child_id, code)
        elif anchor == "external":
            if code not in externals:
                errors.append(f"External anchor without a registered source code: {child_id}")
            elif externals[code]["url"] != row.get("source_url"):
                errors.append(f"External URL differs from the registered {code} URL: {child_id}")
            elif externals[code]["checked"] != row.get("source_checked"):
                errors.append(f"External retrieval date differs from the registered {code} date: {child_id}")
        else:
            errors.append(f"Primary anchor must be local or external: {child_id}")
    return errors


def local_anchor_errors(row, local_sources, root, cache, child_id, code):
    name, line = row.get("source_file", ""), row.get("source_line", "")
    if local_sources.get(code) != name:
        return [f"Local anchor file does not match evidence code {code}: {child_id}"]
    path = root / name
    if not path.is_file():
        return [f"Local anchor file missing: {child_id} -> {name}"]
    if name not in cache:
        cache[name] = path.read_text(encoding="utf-8").splitlines()
    physical = cache[name]
    if not line.isdigit() or not 1 <= int(line) <= len(physical):
        return [f"Local anchor line out of range: {child_id} -> {name}:{line}"]
    if normalize(row["source_excerpt"]) not in normalize(physical[int(line) - 1]):
        return [f"Excerpt is not verbatim text of the anchored line: {child_id}"]
    return []


def valid_date(text):
    try:
        return date.fromisoformat(text) <= date.today()
    except ValueError:
        return False


def count_rows(rows):
    named = Counter(area_of(row["child_id"]) for row in rows if row["membership_status"] == "named")
    claims = Counter(area_of(row["child_id"]) for row in rows if row["membership_status"] == "claim_only")
    counts = Counter((area_of(row["child_id"]), row["member_type"]) for row in rows
                     if row["membership_status"] != "claim_only")
    return named, claims, counts


def validate_counts(rows, expectations):
    declared, named, claims, types = expectations["counts"], *count_rows(rows)
    errors = []
    if len(rows) != declared["TOTAL_ROWS"]:
        errors.append(f"Register row count {len(rows)} differs from the declared total {declared['TOTAL_ROWS']}")
    for key, actual in (("IO_FORMAT_FAMILIES_MIN", types[("IO", "file-format")]),
                        ("IO_SOURCE_ADAPTERS", types[("IO", "source-adapter")]),
                        ("GRAPH_NAMED_MIN", named["GRAPH"]), ("FIT_NAMED_MIN", named["FIT"]),
                        ("PEAK_NAMED_MIN", named["PEAK"])):
        if actual < declared[key]:
            errors.append(f"Count floor broken: {key} requires {declared[key]}, found {actual}")
    for key, actual in (("GRAPH_CLAIM_ONLY", claims["GRAPH"]), ("FIT_CLAIM_ONLY", claims["FIT"]),
                        ("PEAK_CLAIM_ONLY", claims["PEAK"]), ("EXPORT_CLAIM_ONLY", claims["EXPORT"]),
                        ("EXPORT_NAMED", types[("EXPORT", "export-format")]),
                        ("COMPAT_NAMED", types[("COMPAT", "compat-artifact")]), ("GATE_ROWS", types[("GATE", "gate")])):
        if actual != declared[key]:
            errors.append(f"Declared count differs: {key} expects {declared[key]}, found {actual}")
    return errors


def validate_claim_discipline(rows, expectations):
    errors = []
    for row in rows:
        child_id = row["child_id"]
        if row["support_claim"] != "none":
            errors.append(f"Child row makes a support claim: {child_id} -> {row['support_claim']}")
        if row["rights_status"] == "R-BLOCKED" and (row["support_claim"] != "none"
                                                    or not row["expected_output"].startswith("O-NONE")):
            errors.append(f"Rights-blocked row elevates support or output: {child_id}")
        if row["membership_status"] == "claim_only":
            if row["delivery_class"] != "D-GATE" or not row["expected_output"].startswith("O-NONE"):
                errors.append(f"Claim-only row is not an output-free gate: {child_id}")
        else:
            inflated = [token for token in expectations["vendor_count_tokens"] if token in row["named_entry"]]
            if inflated or INFLATION.search(row["named_entry"]):
                shown = ", ".join(inflated) if inflated else row["named_entry"]
                errors.append(f"Named entry states a vendor count without a claim row: {child_id} ({shown})")
    return errors


def validate_no_duplicate_names(rows):
    grouped = Counter((area_of(row["child_id"]), normalize(row["named_entry"]).lower()) for row in rows
                      if row["membership_status"] != "claim_only")
    return [f"Duplicate named entry in catalog {area}: {name}"
            for (area, name), count in sorted(grouped.items()) if count > 1]


def validate_immutability(register_rows, expectations, root=ROOT):
    errors = []
    register_path = root / expectations["register"]["path"]
    actual = hashlib.sha256(register_path.read_bytes()).hexdigest()
    if actual != expectations["register"]["sha256"]:
        errors.append(f"Feature register hash changed: {actual}")
    if len(register_rows) != expectations["register"]["rows"]:
        errors.append(f"Feature register row count changed: {len(register_rows)}")
    if "EG-CAT-" in register_path.read_text(encoding="utf-8"):
        errors.append("Child IDs leaked into the feature register")
    child_path = expectations["child_document"]["path"]
    if any(row["source_file"] == child_path for row in register_rows):
        errors.append("Feature register cites the child register as a source")
    pinned = expectations["child_document"].get("sha256", "")
    if pinned and hashlib.sha256((root / child_path).read_bytes()).hexdigest() != pinned:
        errors.append(f"Child register differs from its pinned digest: {child_path}")
    for name, digest in expectations["protected_inputs"].items():
        if hashlib.sha256((root / name).read_bytes()).hexdigest() != digest:
            errors.append(f"Preserved inventory changed: {name}")
    return errors


def validate_file_form(expectations, root=ROOT):
    errors = []
    document = expectations["child_document"]
    if expectations.get("schema_version") != SUPPORTED_SCHEMA_VERSION:
        errors.append(f"Unsupported expectations schema_version: {expectations.get('schema_version')}")
    path = root / document["path"]
    raw = path.read_bytes()
    if document["encoding"] != "utf-8" or b"\x00" in raw:
        errors.append(f"Unexpected child register encoding declaration: {document['encoding']}")
    try:
        raw.decode("utf-8")
    except UnicodeDecodeError:
        return errors + [f"Child register is not valid {document['encoding']}"]
    declared, actual = document["row_terminator"], "CRLF" if b"\r\n" in raw else "LF"
    if declared != actual:
        errors.append(f"Child register line terminator is {actual}, declared {declared}")
    if raw.startswith(b"\xef\xbb\xbf"):
        errors.append("Child register carries a byte order mark")
    return errors


def validate_contract_completeness(rows, expectations):
    errors = []
    if not expectations.get("notice", "").strip():
        return ["The contract file must state that child rows are evidence entries, not implementation"]
    for label in ("P0-003", "P0-005", "P0-012"):
        if label not in expectations.get("evidence_documents", {}):
            errors.append(f"Evidence document for {label} is not declared")
    if "evidence_document" in expectations:
        errors.append("Contract carries a superseded singular evidence_document key")
    used = {row["evidence_code"] for row in rows if row["primary_anchor"] == "external"}
    for code, item in expectations["external_sources"].items():
        if code not in used and not item.get("reference_only"):
            errors.append(f"Registered page {code} is cited by no row and is not marked reference_only")
    for code in sorted(used - set(expectations["external_sources"])):
        errors.append(f"Row cites an unregistered page: {code}")
    return errors


def constants(text, start, stop):
    section = split_section(text, start, stop)
    if section is None:
        return None
    return {key: int(value) for key, value in re.findall(r"\|\s*`([A-Z0-9_]+)`\s*\|\s*(\d+)\s*\|", section)}


def split_section(text, start, stop):
    if start not in text:
        return None
    tail = text.split(start, 1)[1]
    return tail.split(stop, 1)[0] if stop in tail else tail


def validate_p0_012_admissions(rows, expectations, root=ROOT):
    """Return the P0-012 row deltas after matching its tables to declared floors."""
    errors = []
    config = expectations.get("p0_012", {})
    document = expectations.get("evidence_documents", {}).get("P0-012")
    source_code = config.get("source_code")
    if not document or source_code not in expectations.get("local_sources", {}):
        return ["P0-012 evidence document or local source registration is missing"], {}
    if expectations["local_sources"][source_code] != document:
        errors.append("P0-012 local source path differs from its evidence document")

    text = (root / document).read_text(encoding="utf-8")
    declared_floors = expectations.get("p0_005", {}).get("declared_floors", {})
    tables = {}
    table_ids = {}
    declared_counts = {}
    for area, heading in config.get("table_headings", {}).items():
        stop = config.get("table_stops", {}).get(area)
        columns = config.get("table_columns", {}).get(area)
        table = markdown_rows(text, heading, columns, stop=stop)
        floor_key = config.get("floor_keys", {}).get(area)
        floor = declared_floors.get(floor_key)
        if table is None or floor is None:
            errors.append(f"P0-012 {area} table or declared floor is missing")
            continue
        if len(table) != floor:
            errors.append(f"P0-012 {area} rows changed: document {len(table)}, contract {floor}")
        tables[area] = table
        table_ids[area] = {entry[0].strip("`") for entry in table}
        declared_counts[area] = floor

    expected_ids = table_ids.get("FIT", set()) | table_ids.get("PEAK", set())
    source_rows = [row for row in rows if row.get("evidence_code") == source_code]
    actual_ids = {row.get("child_id") for row in source_rows}
    if actual_ids != expected_ids:
        undeclared = sorted(actual_ids - expected_ids)
        missing = sorted(expected_ids - actual_ids)
        errors.append(f"P0-012 admitted child IDs differ from its tables: undeclared={undeclared}; missing={missing}")

    surface_ids = table_ids.get("SURFACE", set())
    retained_surface_ids = set(config.get("retained_surface_ids", []))
    if not retained_surface_ids <= surface_ids:
        errors.append("P0-012 retained surface IDs are absent from its surface table")
    registered_ids = {row.get("child_id") for row in rows}
    for child_id in sorted(retained_surface_ids - registered_ids):
        errors.append(f"Existing P0-012 surface child is missing: {child_id}")
    for child_id in sorted(retained_surface_ids & actual_ids):
        errors.append(f"P0-012 surface child must retain its existing anchor: {child_id}")
    admitted_surface_ids = surface_ids & actual_ids
    if admitted_surface_ids:
        errors.append(f"P0-012 surface rows must remain unadmitted: {sorted(admitted_surface_ids)}")
    other_surface_ids = surface_ids - retained_surface_ids
    registered_surface_ids = other_surface_ids & registered_ids
    if registered_surface_ids:
        errors.append(f"P0-012 surface rows must remain unadmitted: {sorted(registered_surface_ids)}")

    deltas = {area: count for area, count in declared_counts.items() if area in {"FIT", "PEAK"}}
    return errors, deltas


def markdown_rows(text, heading, columns, stop=None):
    section = split_section(text, heading, stop or "\n## ")
    if section is None:
        return None
    rows = []
    for line in section.splitlines():
        cells = [re.sub(r"\s+", " ", cell.replace("`", "").strip()) for cell in line.strip().strip("|").split("|")]
        if len(cells) != columns or not cells[0] or set("".join(cells)) <= set("-: ") or cells[0] in HEADING_LABELS:
            continue
        rows.append(cells)
    return rows


def validate_document_consistency(rows, expectations, root=ROOT):
    """Compare the register and contract with every accepted evidence document."""
    errors = validate_contract_completeness(rows, expectations)
    p003 = (root / expectations["evidence_documents"]["P0-003"]).read_text(encoding="utf-8")
    p005 = (root / expectations["evidence_documents"]["P0-005"]).read_text(encoding="utf-8")

    declared = constants(p003, "### 11.4 Count expectations", "### 11.5")
    if declared is None:
        errors.append("P0-003 section 11.4 constants not found")
    else:
        for key, value in declared.items():
            if expectations["p0_003_declared"].get(key) != value:
                errors.append(f"P0-003 declared constant drift for {key}: json "
                              f"{expectations['p0_003_declared'].get(key)}, document {value}")
        for key in expectations["p0_003_declared"]:
            if key not in declared:
                errors.append(f"P0-003 declared constant missing from the document: {key}")

    floors = constants(p005, "## 10. Proposed expectation updates", "## 11.")
    if floors is None:
        errors.append("P0-005 section 10 constants not found")
    else:
        for key, value in expectations["p0_005"]["declared_floors"].items():
            if floors.get(key) != value:
                errors.append(f"P0-005 declared floor drift for {key}: json {value}, document {floors.get(key)}")
        for key in sorted(set(floors) - set(expectations["p0_005"]["declared_floors"])):
            errors.append(f"P0-005 section 10 constant absent from the contract file: {key}")

    for key, baseline in expectations["p0_003_declared"].items():
        enforced = expectations["counts"].get(key)
        if enforced is not None and enforced < baseline:
            errors.append(f"A declared P0-003 floor was lowered without a decision: {key} {baseline} -> {enforced}")

    section_three = split_section(p003, "## 3. Count reconciliation", "## 4.")
    if section_three is None:
        return errors + ["P0-003 section 3 count table not found"]
    areas = parent_areas(expectations)
    additions = markdown_rows(p005, "### 6.4 Named types absent from the 73 rows", 5, stop="\n### 6.5")
    if additions is None:
        return errors + ["P0-005 section 6.4 named additions not found"]
    if len(additions) != expectations["p0_005"]["graph_additions"]:
        errors.append(f"P0-005 section 6.4 rows changed: document {len(additions)}, contract "
                      f"{expectations['p0_005']['graph_additions']}")
    p012_errors, p012_deltas = validate_p0_012_admissions(rows, expectations, root)
    errors += p012_errors
    for line in section_three.splitlines():
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) != 6 or not re.match(r"^\d+", cells[3]):
            continue
        area = areas.get(cells[1].strip("`"))
        if area is None:
            continue
        declared_count = int(re.match(r"\d+", cells[3]).group())
        if area == "GRAPH":
            declared_count += len(additions)
        declared_count += p012_deltas.get(area, 0)
        actual = len([row for row in rows if area_of(row["child_id"]) == area
                      and row["membership_status"] != "claim_only"])
        if actual != declared_count:
            errors.append(f"Count table drift for {area}: documents give {declared_count}, register {actual}")
    for child_id in sorted(set(expectations["p0_005"]["revised_control_rows"])):
        if not any(row["child_id"] == child_id for row in rows):
            errors.append(f"P0-005 revised control row missing from the register: {child_id}")
    return errors


def validate_all(rows, register_rows, expectations, root=ROOT):
    schema_errors = validate_schema(rows, expectations)
    if schema_errors:
        return schema_errors
    errors = []
    errors += validate_ids(rows, expectations)
    errors += validate_parents(rows, register_rows, expectations)
    errors += validate_fields(rows, expectations)
    errors += validate_anchors(rows, expectations, root)
    errors += validate_counts(rows, expectations)
    errors += validate_claim_discipline(rows, expectations)
    errors += validate_no_duplicate_names(rows)
    errors += validate_immutability(register_rows, expectations, root)
    errors += validate_file_form(expectations, root)
    errors += validate_document_consistency(rows, expectations, root)
    return errors


def main():
    expectations = load_expectations()
    rows = read_csv_rows(ROOT / expectations["child_document"]["path"])
    register_rows = read_csv_rows(ROOT / expectations["register"]["path"])
    errors = validate_all(rows, register_rows, expectations)
    if errors:
        print("Catalog-child validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1
    named, claims, types = count_rows(rows)
    print(f"PASS: {len(rows)} child rows; {sum(named.values())} named, {sum(claims.values())} claim-only")
    print("PASS: " + ", ".join(f"{key[0].lower()}/{key[1]}={value}" for key, value in sorted(types.items())))
    print("PASS: ID grammar, sequence retirements, parent integrity, field enums and area consistency")
    print("PASS: anchors verbatim, source names present, excerpt hashes and registered retrieval dates")
    print("PASS: count floors and claim-only counts match tools/catalog_expectations.json")
    print("PASS: declared constants and named counts agree with the P0-003, P0-005 and P0-012 evidence documents")
    print("PASS: P0-012 counts come from declared expectation floors; its surface rows remain unadmitted")
    print("PASS: register and inventory immutability, ASCII policy outside anchor fields, no support claim")
    print("Not checked: remote URL availability, semantic register-family mapping, scientific semantics")
    return 0


if __name__ == "__main__":
    sys.exit(main())
