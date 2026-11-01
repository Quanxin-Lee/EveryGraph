#include <cstdint>
#include <string>
#include <vector>

#include "data_test_support.hpp"

using namespace egtest;

namespace {

// Stages a version of the source sheet's shape and binds it into the target worksheet through
// an already-committed correspondence. This is the only way a foreign-domain column can join a
// worksheet: an explicit mapping, never an array-position zip.
[[nodiscard]] ColumnLine bind_foreign_column(Fixture& fixture, SimpleTable& target,
                                             const SimpleTable& source,
                                             std::uint64_t correspondence,
                                             const std::vector<Cell>& cells,
                                             const std::string& label) {
    ColumnLine line;
    line.column = target.next_column++;
    line.dataset = target.next_dataset++;
    line.field = target.next_field++;
    line.version = target.next_version++;
    line.same_domain = false;

    auto schema = schema_of({field(data::field_id(line.field), label, LogicalType::float64)});
    auto chunk = chunk_of(data::field_id(line.field), LogicalType::float64,
                         ChunkId{ObjectId{target.next_chunk++}}, cells);
    auto version = version_of(data::dataset_id(line.dataset), data::dataset_version_id(line.version),
                              data::row_domain_id(source.domain), schema, {chunk}, cells.size(),
                              provenance_of(data::command_id::kColumnBindForeign));
    const DraftId draft = fixture.stage(version);

    ParameterMap arguments = Fixture::with(data::argument::kColumn, line.column);
    Fixture::put(arguments, data::argument::kWorksheet, target.worksheet);
    Fixture::put(arguments, data::argument::kDataset, line.dataset);
    Fixture::put(arguments, data::argument::kVersionDraft, draft);
    Fixture::put(arguments, data::argument::kCorrespondence, correspondence);
    Fixture::put(arguments, data::argument::kLabel, label);
    (void)fixture.require_run(data::command_id::kColumnBindForeign, std::move(arguments),
                              fixture.declared_writes({line.column, line.version}));
    target.columns.push_back(line);
    return line;
}

// Puts a second worksheet into the same workbook and returns a table bound to its identities.
[[nodiscard]] SimpleTable attach_source_sheet(Fixture& fixture, const SimpleTable& host,
                                              std::uint64_t base) {
    SimpleTable source = make_table(base);
    source.project = host.project;
    source.workbook = host.workbook;
    create_worksheet(fixture, source, source.worksheet, source.domain, "source sheet");
    return source;
}

}  // namespace

EG_TEST("ALIGN", "001", a_row_change_moves_the_domain_and_every_same_domain_column_as_one_revision) {
    Fixture fixture;
    auto table = make_table(31);
    create_project_chain(fixture, table);
    const RowId first = data::row_id(31101);
    const RowId second = data::row_id(31102);
    const RowId third = data::row_id(31103);
    append_rows(fixture, table, {first, second});

    ColumnSpec counts;
    counts.type = LogicalType::int64;
    counts.cells = {int_cell(1), int_cell(2)};
    (void)create_column(fixture, table, counts);

    ColumnSpec weights;
    weights.type = LogicalType::float64;
    weights.cells = {float64_cell(1.5), float64_cell(2.5)};
    (void)create_column(fixture, table, weights);

    const std::uint64_t original_domain = table.domain;
    const std::uint64_t left_version = table.columns[0].version;
    const std::uint64_t right_version = table.columns[1].version;
    const std::uint64_t original_column_count = table.columns.size();
    const std::size_t depth_before = fixture.undo_depth();

    append_rows(fixture, table, {third});

    // One command, one revision: the undo stack grew by exactly one entry.
    EG_REQUIRE_EQ(fixture.undo_depth(), depth_before + 1);
    const DocumentView view = fixture.document();
    const auto* sheet = view.find(data::worksheet_id(table.worksheet));
    EG_REQUIRE(sheet != nullptr);
    EG_REQUIRE(sheet->row_domain == data::row_domain_id(table.domain));
    EG_REQUIRE(sheet->row_domain != data::row_domain_id(original_domain));

    // Every same-domain column moved to a successor ordered by the revised domain.
    const auto revised = fixture.row_domain(data::row_domain_id(table.domain));
    EG_REQUIRE_EQ(revised->row_count(), std::uint64_t{3});
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(revised->row_at(2)), third);
    auto left_lease = fixture.lease(data::dataset_version_id(table.columns[0].version));
    auto right_lease = fixture.lease(data::dataset_version_id(table.columns[1].version));
    EG_REQUIRE(left_lease.version().row_domain() == revised->id());
    EG_REQUIRE(right_lease.version().row_domain() == revised->id());
    EG_REQUIRE_EQ(left_lease.row_count(), std::uint64_t{3});
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(left_lease.cell(data::field_id(table.columns[0].field),
                                                        first)),
                       int_cell(1));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(right_lease.cell(data::field_id(table.columns[1].field),
                                                         second)),
                       float64_cell(2.5));
    // The appended row is missing, not invented, in both columns.
    EG_REQUIRE(EG_REQUIRE_TAKEN(left_lease.cell(data::field_id(table.columns[0].field), third))
                   .is_missing());
    EG_REQUIRE(EG_REQUIRE_TAKEN(right_lease.cell(data::field_id(table.columns[1].field), third))
                   .is_missing());

    // The superseded domain and its versions stay readable, so an older view is not invalidated.
    const auto superseded = fixture.row_domain(data::row_domain_id(original_domain));
    EG_REQUIRE_EQ(superseded->row_count(), std::uint64_t{2});
    auto old_left = fixture.lease(data::dataset_version_id(left_version));
    auto old_right = fixture.lease(data::dataset_version_id(right_version));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(old_left.cell_at(data::field_id(table.columns[0].field), 1)),
                       int_cell(2));
    EG_REQUIRE_CELL_EQ(
        EG_REQUIRE_TAKEN(old_right.cell_at(data::field_id(table.columns[1].field), 1)),
        float64_cell(2.5));
    EG_REQUIRE_EQ(original_column_count, std::uint64_t{2});

    // A deletion keeps the surviving identities and their values together.
    const std::uint64_t appended_left = table.columns[0].version;
    const std::uint64_t appended_domain = table.domain;
    delete_rows(fixture, table, {second});
    auto after_delete = fixture.lease(data::dataset_version_id(table.columns[0].version));
    EG_REQUIRE_EQ(after_delete.row_count(), std::uint64_t{2});
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(after_delete.row_at(0)), first);
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(after_delete.row_at(1)), third);
    EG_REQUIRE_CELL_EQ(
        EG_REQUIRE_TAKEN(after_delete.cell(data::field_id(table.columns[0].field), first)),
        int_cell(1));
    const auto deleted_domain = fixture.row_domain(data::row_domain_id(table.domain));
    EG_REQUIRE(!deleted_domain->index_of(second).has_value());

    // Undo restores the previous reference state exactly.
    auto undo = fixture.undo();
    EG_REQUIRE(undo.succeeded());
    const DocumentView restored = fixture.document();
    EG_REQUIRE(restored.find(data::worksheet_id(table.worksheet))->row_domain ==
               data::row_domain_id(appended_domain));
    EG_REQUIRE_EQ(restored.find(data::column_id(table.columns[0].column))->version,
                  data::dataset_version_id(appended_left));
    // A rejected row change leaves the previous revision intact.
    ParameterMap bogus = Fixture::with(data::argument::kWorksheet, table.worksheet);
    Fixture::put(bogus, data::argument::kDomainDraft, std::uint64_t{9999});
    Fixture::put(bogus, data::argument::kOperation, std::string{"append"});
    Fixture::put(bogus, data::argument::kColumns, std::string{""});
    Fixture::put(bogus, data::argument::kVersionDrafts, std::string{""});
    const auto rejected = fixture.run(data::command_id::kWorksheetChangeRows, std::move(bogus),
                                      fixture.declared_writes({table.worksheet}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::unknown_draft, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE(fixture.document().find(data::worksheet_id(table.worksheet))->row_domain ==
               data::row_domain_id(appended_domain));
}

EG_TEST("ALIGN", "002", a_keyed_join_pairs_by_identity_and_leaves_the_source_values_untouched) {
    Fixture fixture;
    auto target = make_table(41);
    create_project_chain(fixture, target);
    append_rows(fixture, target, {data::row_id(41101), data::row_id(41102), data::row_id(41103)});

    ColumnSpec target_keys;
    target_keys.type = LogicalType::text;
    target_keys.label = "key";
    target_keys.cells = {text_cell("alpha"), text_cell("beta"), text_cell("gamma")};
    (void)create_column(fixture, target, target_keys);

    SimpleTable source = attach_source_sheet(fixture, target, 42);
    append_rows(fixture, source, {data::row_id(42101), data::row_id(42102), data::row_id(42103)});
    ColumnSpec source_key_column;
    source_key_column.type = LogicalType::text;
    source_key_column.label = "key";
    // A different order: pairing by key must not follow position.
    source_key_column.cells = {text_cell("gamma"), text_cell("alpha"), text_cell("beta")};
    const ColumnLine source_keys = create_column(fixture, source, source_key_column);

    ColumnSpec source_values;
    source_values.type = LogicalType::float64;
    source_values.label = "value";
    source_values.cells = {float64_cell(30.0), float64_cell(10.0), float64_cell(20.0)};
    const ColumnLine values_line = create_column(fixture, source, source_values);
    const auto committed_source = fixture.version(data::dataset_version_id(values_line.version));
    const data::ContentHash source_hash = committed_source->content_hash();

    const std::uint64_t correspondence = 43001;
    ParameterMap arguments = Fixture::with(data::argument::kCorrespondence, correspondence);
    Fixture::put(arguments, data::argument::kWorksheet, target.worksheet);
    Fixture::put(arguments, data::argument::kSourceVersion, source_keys.version);
    Fixture::put(arguments, data::argument::kMethod, std::string{"keyed_join"});
    Fixture::put(arguments, data::argument::kKeyColumn, target.columns[0].column);
    Fixture::put(arguments, data::argument::kSourceField, source_keys.field);
    (void)fixture.require_run(data::command_id::kCorrespondenceCreate, std::move(arguments),
                             fixture.declared_writes({correspondence}));

    const auto mapping = fixture.correspondence(data::correspondence_id(correspondence));
    EG_REQUIRE(mapping->method() == data::MappingMethod::keyed_join);
    EG_REQUIRE(mapping->target_domain() == data::row_domain_id(target.domain));
EG_REQUIRE(mapping->source_domain() == committed_source->row_domain());
    EG_REQUIRE_EQ(mapping->pairs().size(), std::size_t{3});
    // alpha -> the source row holding alpha, which is the second row, not the first.
    EG_REQUIRE_EQ(*mapping->source_at(0), data::row_id(42102));
    EG_REQUIRE_EQ(*mapping->source_at(1), data::row_id(42103));
    EG_REQUIRE_EQ(*mapping->source_at(2), data::row_id(42101));
    EG_REQUIRE_EQ(mapping->mapped_count(), std::size_t{3});
    EG_REQUIRE(mapping->unmatched_targets().empty());
    EG_REQUIRE(mapping->unmatched_sources().empty());

    // The join lineage is recorded in the document, so a later reader sees the pairing it made.
    const app::DocumentSnapshot snapshot = fixture.gateway().snapshot();
    const auto* record = snapshot.find(ObjectId{correspondence});
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE_EQ(data::read_id_property(*record, data::property::kTargetDomain).value_or(0),
                  target.domain);
    EG_REQUIRE_EQ(data::read_id_property(*record, data::property::kSourceDomain).value_or(0),
                  source.domain);
    EG_REQUIRE_EQ(data::read_text_property(*record, data::property::kLabel).value_or(""),
                  std::string{"keyed_join"});
    EG_REQUIRE_EQ(data::read_count_property(*record, data::property::kRowCount).value_or(0),
                  std::uint64_t{3});

    // Binding the foreign column resolves values through the pairs, and the source is unchanged.
    const ColumnLine bound = bind_foreign_column(
        fixture, target, source, correspondence,
        {float64_cell(30.0), float64_cell(10.0), float64_cell(20.0)}, "joined");
    const DocumentView view = fixture.document();
    const ColumnBindingRecord* binding = view.find(data::column_id(bound.column));
    EG_REQUIRE(binding != nullptr);
    EG_REQUIRE(binding->link == data::DomainLink::correspondence);
    EG_REQUIRE(binding->correspondence.has_value());
    EG_REQUIRE(*binding->correspondence == data::correspondence_id(correspondence));
    EG_REQUIRE(binding->write_policy == data::WritePolicy::mutable_column);

    const auto after = fixture.version(data::dataset_version_id(values_line.version));
    EG_REQUIRE(after->content_hash() == source_hash);
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(committed_source->cell(
                           data::field_id(values_line.field), 0)),
                       float64_cell(30.0));

    // A target row whose key matches nothing stays explicitly unmatched, with no value invented
    // for it, and the source row that no target names is reported the other way round.
    SimpleTable partial = attach_source_sheet(fixture, target, 43);
    append_rows(fixture, partial, {data::row_id(43101), data::row_id(43102)});
    ColumnSpec partial_keys;
    partial_keys.type = LogicalType::text;
    partial_keys.label = "key";
    partial_keys.cells = {text_cell("alpha"), text_cell("epsilon")};
    const ColumnLine partial_line = create_column(fixture, partial, partial_keys);
    ParameterMap partial_join = Fixture::with(data::argument::kCorrespondence, std::uint64_t{43900});
    Fixture::put(partial_join, data::argument::kWorksheet, target.worksheet);
    Fixture::put(partial_join, data::argument::kSourceVersion, partial_line.version);
    Fixture::put(partial_join, data::argument::kMethod, std::string{"keyed_join"});
    Fixture::put(partial_join, data::argument::kKeyColumn, target.columns[0].column);
    Fixture::put(partial_join, data::argument::kSourceField, partial_line.field);
    (void)fixture.require_run(data::command_id::kCorrespondenceCreate, std::move(partial_join),
                              fixture.declared_writes({43900}));
    const auto partial_mapping = fixture.correspondence(data::correspondence_id(43900));
    EG_REQUIRE_EQ(*partial_mapping->source_at(0), data::row_id(43101));
    EG_REQUIRE(!partial_mapping->source_at(1).has_value());
    EG_REQUIRE(!partial_mapping->source_at(2).has_value());
    EG_REQUIRE_EQ(partial_mapping->unmatched_targets().size(), std::size_t{2});
    EG_REQUIRE_EQ(partial_mapping->unmatched_sources().size(), std::size_t{1});
    EG_REQUIRE_EQ(partial_mapping->unmatched_sources().front(), data::row_id(43102));
    EG_REQUIRE_EQ(partial_mapping->mapped_count(), std::size_t{1});

    // A duplicate key in the source is rejected instead of multiplying the target row.
    SimpleTable repeated = attach_source_sheet(fixture, target, 44);
    append_rows(fixture, repeated, {data::row_id(44101), data::row_id(44102)});
    ColumnSpec duplicate_keys;
    duplicate_keys.type = LogicalType::text;
    duplicate_keys.label = "key";
    duplicate_keys.cells = {text_cell("alpha"), text_cell("alpha")};
    const ColumnLine duplicate_line = create_column(fixture, repeated, duplicate_keys);
    ParameterMap clash = Fixture::with(data::argument::kCorrespondence, std::uint64_t{44900});
    Fixture::put(clash, data::argument::kWorksheet, target.worksheet);
    Fixture::put(clash, data::argument::kSourceVersion, duplicate_line.version);
    Fixture::put(clash, data::argument::kMethod, std::string{"keyed_join"});
    Fixture::put(clash, data::argument::kKeyColumn, target.columns[0].column);
    Fixture::put(clash, data::argument::kSourceField, duplicate_line.field);
    const auto rejected =
        fixture.run(data::command_id::kCorrespondenceCreate, std::move(clash),
                    fixture.declared_writes({44900}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::duplicate_join_key, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE(!fixture.store().contains(data::correspondence_id(44900)));
}

EG_TEST("ALIGN", "003", an_unapproved_positional_mapping_is_rejected_and_an_approved_one_pairs_ids) {
    Fixture fixture;
    auto target = make_table(45);
    create_project_chain(fixture, target);
    append_rows(fixture, target, {data::row_id(45101), data::row_id(45102), data::row_id(45103)});
    ColumnSpec index_column;
    index_column.type = LogicalType::int64;
    index_column.cells = {int_cell(1), int_cell(2), int_cell(3)};
    (void)create_column(fixture, target, index_column);

    SimpleTable source = attach_source_sheet(fixture, target, 46);
    append_rows(fixture, source, {data::row_id(46101), data::row_id(46102), data::row_id(46103)});
    ColumnSpec foreign_values;
    foreign_values.type = LogicalType::float64;
    foreign_values.cells = {float64_cell(7.0), float64_cell(8.0), float64_cell(9.0)};
    const ColumnLine values_line = create_column(fixture, source, foreign_values);

    // No approval: an unrelated column is never zipped by current array position.
    ParameterMap unapproved = Fixture::with(data::argument::kCorrespondence, std::uint64_t{47001});
    Fixture::put(unapproved, data::argument::kWorksheet, target.worksheet);
    Fixture::put(unapproved, data::argument::kSourceVersion, values_line.version);
    Fixture::put(unapproved, data::argument::kMethod, std::string{"approved_positional"});
    Fixture::put(unapproved, data::argument::kApproved, std::string{"false"});
    const auto rejected = fixture.run(data::command_id::kCorrespondenceCreate, std::move(unapproved),
                                      fixture.declared_writes({47001}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::positional_mapping_required,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{2});
    EG_REQUIRE(!fixture.store().contains(data::correspondence_id(47001)));

    // Creating a foreign-domain column without any mapping is rejected the same way.
    auto schema = schema_of({field(data::field_id(48010), "value", LogicalType::float64)});
    auto chunk = chunk_of(data::field_id(48010), LogicalType::float64, data::chunk_id(48011),
                         {float64_cell(1.0), float64_cell(2.0), float64_cell(3.0)});
    auto foreign_version = version_of(
        data::dataset_id(48012), data::dataset_version_id(48013),
        data::row_domain_id(source.domain), schema, {chunk}, 3,
        provenance_of(data::command_id::kColumnCreate));
    const DraftId draft = fixture.stage(foreign_version);
    ParameterMap zip_attempt = Fixture::with(data::argument::kColumn, std::uint64_t{48014});
    Fixture::put(zip_attempt, data::argument::kWorksheet, target.worksheet);
    Fixture::put(zip_attempt, data::argument::kDataset, std::uint64_t{48012});
    Fixture::put(zip_attempt, data::argument::kVersionDraft, draft);
    Fixture::put(zip_attempt, data::argument::kLabel, std::string{"zipped"});
    const auto zipped = fixture.run(data::command_id::kColumnCreate, std::move(zip_attempt),
                                    fixture.declared_writes({48014, 48013}));
    EG_REQUIRE_REJECTED(zipped, DetailCode::positional_mapping_required,
                        ErrorCode::invalid_argument);
    EG_REQUIRE(fixture.document().find(data::column_id(48014)) == nullptr);

    // Approved: the pairing is captured as stable id pairs, so a later re-sort cannot re-pair.
    ParameterMap approved = Fixture::with(data::argument::kCorrespondence, std::uint64_t{47002});
    Fixture::put(approved, data::argument::kWorksheet, target.worksheet);
    Fixture::put(approved, data::argument::kSourceVersion, values_line.version);
    Fixture::put(approved, data::argument::kMethod, std::string{"approved_positional"});
    Fixture::put(approved, data::argument::kApproved, std::string{"true"});
    (void)fixture.require_run(data::command_id::kCorrespondenceCreate, std::move(approved),
                              fixture.declared_writes({47002}));
    const auto mapping = fixture.correspondence(data::correspondence_id(47002));
    EG_REQUIRE(mapping->method() == data::MappingMethod::approved_positional);
    EG_REQUIRE_EQ(mapping->pairs().size(), std::size_t{3});
    EG_REQUIRE_EQ(*mapping->source_at(0), data::row_id(46101));
    EG_REQUIRE_EQ(*mapping->source_at(2), data::row_id(46103));
    EG_REQUIRE_EQ(mapping->mapped_count(), std::size_t{3});

    const ColumnLine bound = bind_foreign_column(
        fixture, target, source, 47002,
        {float64_cell(7.0), float64_cell(8.0), float64_cell(9.0)}, "positional");
    const DocumentView view = fixture.document();
    const ColumnBindingRecord* binding = view.find(data::column_id(bound.column));
    EG_REQUIRE(binding != nullptr);
    EG_REQUIRE(binding->link == data::DomainLink::correspondence);
    // The same-domain rule still holds: a version that shares the worksheet's row domain is a
    // same-domain column, and claiming it through the correspondence path is rejected by that
    // rule rather than by a missing draft.
    auto local_schema = schema_of({field(data::field_id(49010), "value", LogicalType::float64)});
    auto local_chunk = chunk_of(data::field_id(49010), LogicalType::float64, data::chunk_id(49011),
                               {float64_cell(1.0), float64_cell(2.0), float64_cell(3.0)});
    auto local_version = version_of(
        data::dataset_id(49012), data::dataset_version_id(49013), data::row_domain_id(target.domain),
        local_schema, {local_chunk}, 3, provenance_of(data::command_id::kColumnBindForeign));
    const DraftId local_draft = fixture.stage(local_version);
    ParameterMap mislabeled = Fixture::with(data::argument::kColumn, std::uint64_t{49001});
    Fixture::put(mislabeled, data::argument::kWorksheet, target.worksheet);
    Fixture::put(mislabeled, data::argument::kDataset, std::uint64_t{49012});
    Fixture::put(mislabeled, data::argument::kVersionDraft, local_draft);
    Fixture::put(mislabeled, data::argument::kCorrespondence, std::uint64_t{47002});
    Fixture::put(mislabeled, data::argument::kLabel, std::string{"same-domain"});
    const auto wrong = fixture.run(data::command_id::kColumnBindForeign, std::move(mislabeled),
                                  fixture.declared_writes({49001}));
    EG_REQUIRE_REJECTED(wrong, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(wrong);
    EG_REQUIRE(fixture.document().find(data::column_id(49001)) == nullptr);
    EG_REQUIRE(!fixture.store().contains(data::dataset_version_id(49013)));
}

EG_TEST("ALIGN", "006", unequal_lengths_are_rejected_naming_both_and_never_zipped_or_padded) {
    Fixture fixture;
    auto target = make_table(51);
    create_project_chain(fixture, target);
    append_rows(fixture, target, {data::row_id(51101), data::row_id(51102), data::row_id(51103)});

    // A same-domain column whose candidate holds two rows against a three-row domain.
    auto schema = schema_of({field(data::field_id(51201), "value", LogicalType::float64)});
    auto short_chunk = chunk_of(data::field_id(51201), LogicalType::float64, data::chunk_id(51202),
                               {float64_cell(1.0), float64_cell(2.0)});
    auto short_version = version_of(data::dataset_id(51203), data::dataset_version_id(51204),
                                    data::row_domain_id(target.domain), schema, {short_chunk}, 2,
                                    provenance_of(data::command_id::kColumnCreate));
    const DraftId short_draft = fixture.stage(short_version);
    ParameterMap arguments = Fixture::with(data::argument::kColumn, std::uint64_t{51205});
    Fixture::put(arguments, data::argument::kWorksheet, target.worksheet);
    Fixture::put(arguments, data::argument::kDataset, std::uint64_t{51203});
    Fixture::put(arguments, data::argument::kVersionDraft, short_draft);
    Fixture::put(arguments, data::argument::kLabel, std::string{"short"});
    const auto rejected = fixture.run(data::command_id::kColumnCreate, std::move(arguments),
                                      fixture.declared_writes({51205, 51204}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    // The rejection names both lengths, so the caller can see what did not line up.
    EG_REQUIRE(rejected.error->message.find("2 rows") != std::string::npos);
    EG_REQUIRE(rejected.error->message.find("3 rows") != std::string::npos);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{0});
    EG_REQUIRE(fixture.document().find(data::column_id(51205)) == nullptr);

    // A longer candidate is rejected the same way: nothing is padded out.
    auto long_chunk = chunk_of(data::field_id(51201), LogicalType::float64, data::chunk_id(51206),
                               {float64_cell(1.0), float64_cell(2.0), float64_cell(3.0),
                                float64_cell(4.0)});
    auto long_version = version_of(data::dataset_id(51207), data::dataset_version_id(51208),
                                   data::row_domain_id(target.domain), schema, {long_chunk}, 4,
                                   provenance_of(data::command_id::kColumnCreate));
    const DraftId long_draft = fixture.stage(long_version);
    ParameterMap longer = Fixture::with(data::argument::kColumn, std::uint64_t{51209});
    Fixture::put(longer, data::argument::kWorksheet, target.worksheet);
    Fixture::put(longer, data::argument::kDataset, std::uint64_t{51207});
    Fixture::put(longer, data::argument::kVersionDraft, long_draft);
    Fixture::put(longer, data::argument::kLabel, std::string{"long"});
    const auto over = fixture.run(data::command_id::kColumnCreate, std::move(longer),
                                 fixture.declared_writes({51209, 51208}));
    EG_REQUIRE_REJECTED(over, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    EG_REQUIRE(over.error->message.find("4 rows") != std::string::npos);
    EG_REQUIRE(over.error->message.find("3 rows") != std::string::npos);

    // Across two domains, an approved positional mapping still refuses unequal lengths.
    SimpleTable source = attach_source_sheet(fixture, target, 52);
    append_rows(fixture, source, {data::row_id(52101), data::row_id(52102)});
    ColumnSpec foreign_values;
    foreign_values.type = LogicalType::float64;
    foreign_values.cells = {float64_cell(1.0), float64_cell(2.0)};
    const ColumnLine values_line = create_column(fixture, source, foreign_values);

    ParameterMap positional = Fixture::with(data::argument::kCorrespondence, std::uint64_t{52900});
    Fixture::put(positional, data::argument::kWorksheet, target.worksheet);
    Fixture::put(positional, data::argument::kSourceVersion, values_line.version);
    Fixture::put(positional, data::argument::kMethod, std::string{"approved_positional"});
    Fixture::put(positional, data::argument::kApproved, std::string{"true"});
    const auto unequal = fixture.run(data::command_id::kCorrespondenceCreate, std::move(positional),
                                     fixture.declared_writes({52900}));
    EG_REQUIRE_REJECTED(unequal, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    EG_REQUIRE(unequal.error->message.find("3 target rows") != std::string::npos);
    EG_REQUIRE(unequal.error->message.find("2 source rows") != std::string::npos);
    EG_REQUIRE_NO_WRITE(unequal);
    EG_REQUIRE(!fixture.store().contains(data::correspondence_id(52900)));
}
