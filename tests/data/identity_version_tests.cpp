#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "data_test_support.hpp"

using namespace egtest;

namespace {

// Keeps the identity plan's read-back assertion short: a record's identity must equal the
// opaque id the caller allocated, not a position or a label.
template <typename Tag>
[[nodiscard]] std::string identity_text(const data::StableId<Tag>& id) {
    return data::to_string(id);
}

}  // namespace

EG_TEST("ID", "001", identities_and_parent_relationships_are_opaque_and_stable) {
    Fixture fixture;
    auto table = make_table(1);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(1001), data::row_id(1002)});

    ColumnSpec spec;
    spec.type = LogicalType::int32;
    spec.cells = {int_cell(7, LogicalType::int32), int_cell(-9, LogicalType::int32)};
    const ColumnLine line = create_column(fixture, table, spec);

    const DocumentView view = fixture.document();
    EG_REQUIRE(view.find(data::project_id(table.project)) != nullptr);
    EG_REQUIRE(view.find(data::workbook_id(table.workbook)) != nullptr);
    EG_REQUIRE(view.find(data::worksheet_id(table.worksheet)) != nullptr);
    const ColumnBindingRecord* binding = view.find(data::column_id(line.column));
    EG_REQUIRE(binding != nullptr);
    EG_REQUIRE(binding->id == data::column_id(line.column));
    EG_REQUIRE(binding->worksheet == data::worksheet_id(table.worksheet));
    EG_REQUIRE(binding->dataset == data::dataset_id(line.dataset));
    EG_REQUIRE(binding->version == data::dataset_version_id(line.version));

    auto version = fixture.version(data::dataset_version_id(line.version));
    EG_REQUIRE(fixture.store().contains(data::dataset_version_id(line.version)));
    EG_REQUIRE_EQ(version->id(), data::dataset_version_id(line.version));
    EG_REQUIRE_EQ(version->dataset(), data::dataset_id(line.dataset));
    EG_REQUIRE_EQ(version->row_domain(), data::row_domain_id(table.domain));
    EG_REQUIRE_EQ(version->chunk_ids().size(), std::size_t{1});
    // The chunk list is reached by field identity, and the two views of it name the same
    // opaque id, so a chunk is never addressed as an array position.
    const ChunkId listed = version->chunk_ids().front();
    EG_REQUIRE(listed.is_valid());
    const FieldId only_field = version->schema()->fields.front().id;
    const auto by_field = version->chunks_of(only_field);
    EG_REQUIRE_EQ(by_field.size(), std::size_t{1});
    EG_REQUIRE(by_field.front()->id() == listed);
    EG_REQUIRE_EQ(version->chunks_of(data::field_id(only_field.raw() + 1)).size(), std::size_t{0});

    // No identity is an array position: the same document read again holds the same opaque ids.
    const DocumentView again = fixture.document();
    EG_REQUIRE_EQ(again.object_count(), view.object_count());
    EG_REQUIRE(again.find(data::column_id(line.column)) != nullptr);
    EG_REQUIRE_EQ(again.find(data::column_id(line.column))->version,
                  view.find(data::column_id(line.column))->version);
    EG_REQUIRE_EQ(again.workbooks_of(data::project_id(table.project)).size(), std::size_t{1});
    EG_REQUIRE_EQ(again.worksheets_of(data::workbook_id(table.workbook)).size(), std::size_t{1});
    // Saving and reopening the same identities is P2-006's half of this fixture.
}

EG_TEST("ID", "002", rename_reorder_and_label_editing_never_move_an_identity) {
    Fixture fixture;
    auto table = make_table(2);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(2001), data::row_id(2002)});

    ColumnSpec first;
    first.type = LogicalType::float64;
    first.label = "first";
    first.cells = {float64_cell(1.0), float64_cell(2.0)};
    ColumnLine first_line = create_column(fixture, table, first);

    ColumnSpec second;
    second.type = LogicalType::float64;
    second.label = "second";
    second.order_index = 1;
    second.cells = {float64_cell(3.0), float64_cell(4.0)};
    const ColumnLine second_line = create_column(fixture, table, second);

    const std::vector<ColumnBindingRecord> before =
        fixture.document().bindings_of(data::worksheet_id(table.worksheet));
    EG_REQUIRE_EQ(before.size(), std::size_t{2});
    EG_REQUIRE(before[0].id == data::column_id(first_line.column));
    EG_REQUIRE(before[1].id == data::column_id(second_line.column));

    // The translated or display name is presentation only; the identity behind it is unchanged.
    ParameterMap rename = Fixture::with(data::argument::kColumn, first_line.column);
    Fixture::put(rename, data::argument::kLabel, std::string{"\u00C9lan"});
    Fixture::put(rename, data::argument::kOrderIndex, std::uint64_t{9});
    (void)fixture.require_run(data::command_id::kColumnRename, std::move(rename),
                              fixture.declared_writes({first_line.column}));

    ParameterMap move = Fixture::with(data::argument::kColumn, second_line.column);
    Fixture::put(move, data::argument::kOrderIndex, std::uint64_t{0});
    (void)fixture.require_run(data::command_id::kColumnMove, std::move(move),
                              fixture.declared_writes({second_line.column}));

    const std::vector<ColumnBindingRecord> after =
        fixture.document().bindings_of(data::worksheet_id(table.worksheet));
    EG_REQUIRE_EQ(after.size(), std::size_t{2});
    EG_REQUIRE(after[0].id == data::column_id(second_line.column));
    EG_REQUIRE(after[1].id == data::column_id(first_line.column));
    EG_REQUIRE_EQ(after[1].label, std::string{"\u00C9lan"});
    // References still resolve to the same version and the same values.
    EG_REQUIRE(after[1].version == data::dataset_version_id(first_line.version));
    EG_REQUIRE_EQ(fixture.document().columns_using(data::dataset_version_id(first_line.version))
                      .size(),
                  std::size_t{1});
    auto lease = fixture.lease(data::dataset_version_id(first_line.version));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(lease.cell_at(data::field_id(first_line.field), 0)),
                       float64_cell(1.0));
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{2});
}

EG_TEST("ID", "003", an_edit_publishes_a_successor_and_the_prior_version_stays_readable) {
    Fixture fixture;
    auto table = make_table(3);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(3001), data::row_id(3002), data::row_id(3003)});

    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.unit = "m";
    spec.cells = {float64_cell(1.0), float64_cell(2.0), float64_cell(3.0)};
    ColumnLine line = create_column(fixture, table, spec);

    // Hold the old version while the edit publishes a new one.
    const std::shared_ptr<const DatasetVersion> old_version =
        fixture.version(data::dataset_version_id(line.version));
    const std::vector<ChunkId> old_chunks = old_version->chunk_ids();
    const data::ContentHash old_hash = old_version->content_hash();
    const std::string old_schema = old_version->schema()->canonical_text();
    const std::size_t versions_before = fixture.version_count();

    edit_column(fixture, table, line,
                {float64_cell(1.0), float64_cell(2.5), float64_cell(3.0)});

    const std::shared_ptr<const DatasetVersion> new_version =
        fixture.version(data::dataset_version_id(line.version));
    EG_REQUIRE(new_version->id() != old_version->id());
    EG_REQUIRE(new_version->content_hash() != old_hash);
    EG_REQUIRE(new_version->chunk_ids() != old_chunks);

    // The held version is untouched: same identity, same schema, same values, same hash.
    EG_REQUIRE(fixture.store().contains(old_version->id()));
    EG_REQUIRE_EQ(old_version->schema()->canonical_text(), old_schema);
    EG_REQUIRE_EQ(old_version->row_count(), std::uint64_t{3});
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(old_version->cell(data::field_id(line.field), 1)),
                       float64_cell(2.0));
    EG_REQUIRE_EQ(old_version->content_hash(), old_hash);
    // The successor records the version it replaces, both as a provenance parent and as a
    // document-level previous-version reference.
    const Provenance& provenance = new_version->provenance();
    EG_REQUIRE(std::find(provenance.parents.begin(), provenance.parents.end(), old_version->id()) !=
               provenance.parents.end());
    // A snapshot is a value: the pointer a reader takes from a temporary would dangle, so the
    // document copy is held while its object is inspected.
    const app::DocumentSnapshot snapshot = fixture.gateway().snapshot();
    const auto state = snapshot.find(ObjectId{line.version});
    EG_REQUIRE(state != nullptr);
    EG_REQUIRE_EQ(data::read_id_property(*state, data::property::kPreviousVersion).value_or(0),
                  old_version->id().raw());
    EG_REQUIRE_EQ(fixture.version_count(), versions_before + 1);

    // The binding now resolves the successor, and its metadata survived the edit.
    const DocumentView edited_document = fixture.document();
    const ColumnBindingRecord* record = edited_document.find(data::column_id(line.column));
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE(record->version == new_version->id());
    EG_REQUIRE(record->unit.has_value());
    EG_REQUIRE(record->unit == new_version->schema()->find(data::field_id(line.field))->unit);

    // A successor may not re-declare the field. An edit whose schema differs from the version it
    // replaces is rejected, so a column cannot quietly change type, unit or field identity
    // through a version bump.
    const std::uint64_t drift_version = table.next_version++;
    const FieldId drift_field = data::field_id(line.field + 1);
    auto drift_chunk = chunk_of(drift_field, LogicalType::float64,
                                ChunkId{ObjectId{table.next_chunk++}},
                                {float64_cell(1.0), float64_cell(2.0), float64_cell(3.0)});
    auto drift_schema = schema_of({field(drift_field, "value", LogicalType::float64)});
    auto drift = version_of(
        data::dataset_id(line.dataset), data::dataset_version_id(drift_version),
        new_version->row_domain(), drift_schema, {drift_chunk}, 3,
        provenance_of(data::command_id::kColumnEdit, {new_version->id()}));
    ParameterMap drifted = Fixture::with(data::argument::kColumn, line.column);
    Fixture::put(drifted, data::argument::kVersionDraft, fixture.stage(drift));
    const auto rejected_schema = fixture.run(data::command_id::kColumnEdit, std::move(drifted),
                                             fixture.declared_writes({line.column, drift_version}));
    EG_REQUIRE_REJECTED(rejected_schema, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected_schema);
    EG_REQUIRE_EQ(fixture.version_count(), versions_before + 1);
    EG_REQUIRE(!fixture.store().contains(data::dataset_version_id(drift_version)));
    const DocumentView unchanged = fixture.document();
    EG_REQUIRE(unchanged.find(data::column_id(line.column))->version == new_version->id());
}

EG_TEST("ID", "006", a_live_identity_is_rejected_before_commit_and_nothing_is_published) {
    Fixture fixture;
    auto table = make_table(4);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(4001)});

    ColumnSpec spec;
    spec.type = LogicalType::int64;
    spec.cells = {int_cell(1)};
    const ColumnLine line = create_column(fixture, table, spec);

    const std::size_t undo_before = fixture.undo_depth();
    const std::uint64_t versions_before = fixture.version_count();
    const auto sequence_before = fixture.gateway().last_sequence();

    // Reusing a live column identity, with a fresh candidate version behind it.
    ColumnLine clash;
    clash.column = line.column;
    clash.dataset = table.next_dataset++;
    clash.field = table.next_field++;
    clash.version = table.next_version++;
    auto clash_schema = schema_of({field(data::field_id(clash.field), "value", LogicalType::int64)});
    auto clash_chunk = chunk_of(data::field_id(clash.field), LogicalType::int64,
                                ChunkId{ObjectId{table.next_chunk++}}, {int_cell(2)});
    auto clash_version = version_of(data::dataset_id(clash.dataset),
                                    data::dataset_version_id(clash.version),
                                    data::row_domain_id(table.domain), clash_schema, {clash_chunk},
                                    1, provenance_of(data::command_id::kColumnCreate));
    const DraftId clash_draft = fixture.stage(clash_version);

    ParameterMap arguments = Fixture::with(data::argument::kColumn, clash.column);
    Fixture::put(arguments, data::argument::kWorksheet, table.worksheet);
    Fixture::put(arguments, data::argument::kDataset, clash.dataset);
    Fixture::put(arguments, data::argument::kVersionDraft, clash_draft);
    Fixture::put(arguments, data::argument::kLabel, std::string{"reuse"});
    Fixture::put(arguments, data::argument::kRole, std::string{"unspecified"});
    const auto rejected = fixture.run(data::command_id::kColumnCreate, std::move(arguments),
                                      fixture.declared_writes({clash.column, clash.version}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::duplicate_id, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_before);
    EG_REQUIRE_EQ(fixture.version_count(), versions_before);
    EG_REQUIRE(fixture.gateway().last_sequence() == sequence_before);
    // The rejected candidate's payload is not readable, and the staged count fell back.
    EG_REQUIRE(!fixture.store().contains(data::dataset_version_id(clash.version)));
    EG_REQUIRE_EQ(fixture.staged_count(), std::uint64_t{0});
    // The live binding is untouched: a rejected create with its identity does not repoint it.
    const DocumentView after = fixture.document();
    const ColumnBindingRecord* survivor = after.find(data::column_id(line.column));
    EG_REQUIRE(survivor != nullptr);
    EG_REQUIRE(survivor->version == data::dataset_version_id(line.version));
    // The plan asks for both sides of the clash: the identity the caller attempted is in the
    // location, and the live object that already holds it is named with its type and revision.
    EG_REQUIRE(rejected.error->message.find(std::string{"column "} + std::to_string(clash.column)) !=
               std::string::npos);
    EG_REQUIRE(rejected.error->message.find(data::object_type::kColumnBinding) != std::string::npos);
    EG_REQUIRE(rejected.error->message.find(std::to_string(survivor->revision)) != std::string::npos);

    // A worksheet row-domain identity that is already live is rejected the same way.
    ParameterMap worksheet = Fixture::with(data::argument::kWorksheet, table.worksheet + 1);
    Fixture::put(worksheet, data::argument::kWorkbook, table.workbook);
    Fixture::put(worksheet, data::argument::kDomain, table.domain);
    Fixture::put(worksheet, data::argument::kLabel, std::string{"clash"});
    const auto domain_clash = fixture.run(data::command_id::kWorksheetCreate, std::move(worksheet),
                                         fixture.declared_writes({table.worksheet + 1, table.domain}));
    EG_REQUIRE_REJECTED(domain_clash, DetailCode::duplicate_id, ErrorCode::invalid_argument);
    EG_REQUIRE(fixture.document().find(data::worksheet_id(table.worksheet + 1)) == nullptr);
}

EG_TEST("ID", "007", a_delete_that_would_strand_a_dependent_names_it_and_undoes_cleanly) {
    Fixture fixture;
    auto table = make_table(5);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(5001), data::row_id(5002)});

    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.cells = {float64_cell(1.0), float64_cell(2.0)};
    const ColumnLine line = create_column(fixture, table, spec);

    const std::size_t undo_before = fixture.undo_depth();
    ParameterMap delete_version =
        Fixture::with(data::argument::kVersion, line.version);
    const auto rejected = fixture.run(data::command_id::kVersionDelete, std::move(delete_version),
                                     fixture.declared_writes({line.version}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::dangling_reference, ErrorCode::invalid_argument);
    // The rejection names the dependent it would have stranded.
    EG_REQUIRE(rejected.error->message.find(identity_text(data::column_id(line.column))) !=
               std::string::npos);
    EG_REQUIRE_NO_WRITE(rejected);
    EG_REQUIRE(fixture.store().contains(data::dataset_version_id(line.version)));

    // Dropping the binding first leaves the version unreferenced, so it can then go, and undo
    // restores the exact prior reference state.
    ParameterMap delete_column = Fixture::with(data::argument::kColumn, line.column);
    (void)fixture.require_run(data::command_id::kColumnDelete, std::move(delete_column),
                              fixture.declared_writes({line.column}));
    EG_REQUIRE(fixture.document().find(data::column_id(line.column)) == nullptr);
    ParameterMap delete_now = Fixture::with(data::argument::kVersion, line.version);
    (void)fixture.require_run(data::command_id::kVersionDelete, std::move(delete_now),
                              fixture.declared_writes({line.version}));
    EG_REQUIRE(fixture.document().find(data::column_id(line.column)) == nullptr);

    auto undo_version = fixture.undo();
    EG_REQUIRE(undo_version.succeeded());
    auto undo_column = fixture.undo();
    EG_REQUIRE(undo_column.succeeded());
    const app::DocumentSnapshot restored = fixture.gateway().snapshot();
    EG_REQUIRE(restored.find(ObjectId{line.column}) != nullptr);
    EG_REQUIRE(restored.find(ObjectId{line.version}) != nullptr);
    EG_REQUIRE(fixture.store().contains(data::dataset_version_id(line.version)));
    // Undoing both deletes returns the history to exactly the depth it had before them and
    // hands those two entries to redo, so the depth is arithmetic, not a bound.
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_before);
    EG_REQUIRE_EQ(fixture.redo_depth(), std::uint64_t{2});
    auto redo_column = fixture.redo();
    EG_REQUIRE(redo_column.succeeded());
    auto redo_version_delete = fixture.redo();
    EG_REQUIRE(redo_version_delete.succeeded());
    // Redo replays in the original order and moves both entries back, leaving nothing to redo.
    const app::DocumentSnapshot redone = fixture.gateway().snapshot();
    EG_REQUIRE(redone.find(ObjectId{line.column}) == nullptr);
    EG_REQUIRE(redone.find(ObjectId{line.version}) == nullptr);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_before + 2);
    EG_REQUIRE_EQ(fixture.redo_depth(), std::uint64_t{0});
    // Deleting a document reference does not drop the stored payload: an immutable version stays
    // readable for as long as undo may restore it, so the storage lifecycle is this record's
    // declared follow-up, not a side effect of the delete.
    EG_REQUIRE(fixture.store().contains(data::dataset_version_id(line.version)));
}

EG_TEST("ID", "009", a_derived_version_records_both_inputs_with_their_content_hashes) {
    Fixture fixture;
    auto table = make_table(11);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(11001), data::row_id(11002)});

    ColumnSpec left_spec;
    left_spec.type = LogicalType::float64;
    left_spec.cells = {float64_cell(2.0), float64_cell(4.0)};
    const ColumnLine left = create_column(fixture, table, left_spec);

    ColumnSpec right_spec;
    right_spec.type = LogicalType::float64;
    right_spec.cells = {float64_cell(3.0), float64_cell(5.0)};
    const ColumnLine right = create_column(fixture, table, right_spec);

    const auto left_version = fixture.version(data::dataset_version_id(left.version));
    const auto right_version = fixture.version(data::dataset_version_id(right.version));
    const std::string parameters = "mean(x,y) v1";

    ColumnSpec derived;
    derived.type = LogicalType::float64;
    derived.cells = {float64_cell(2.5), float64_cell(4.5)};
    derived.label = "mean";
    const std::uint64_t derived_dataset = table.next_dataset++;
    const std::uint64_t derived_field = table.next_field++;
    const std::uint64_t derived_version = table.next_version++;
    auto schema = schema_of({field(data::field_id(derived_field), "value", derived.type)});
    auto chunk = chunk_of(data::field_id(derived_field), derived.type,
                         ChunkId{ObjectId{table.next_chunk++}}, derived.cells);
    Provenance record = provenance_of("analysis.mean", {}, 
                                      {{left_version->id(), left_version->content_hash()},
                                       {right_version->id(), right_version->content_hash()}},
                                      parameters);
    auto version = version_of(data::dataset_id(derived_dataset),
                             data::dataset_version_id(derived_version),
                             data::row_domain_id(table.domain), schema, {chunk},
                             derived.cells.size(), record);
    const DraftId draft = fixture.stage(version);

    ParameterMap arguments = Fixture::with(data::argument::kDataset, derived_dataset);
    Fixture::put(arguments, data::argument::kVersionDraft, draft);
    Fixture::put(arguments, data::argument::kInputA, left.version);
    Fixture::put(arguments, data::argument::kInputB, right.version);
    Fixture::put(arguments, data::argument::kParameters, parameters);
    (void)fixture.require_run(data::command_id::kDatasetDerive, std::move(arguments),
                              fixture.declared_writes({derived_version}));

    const auto published = fixture.version(data::dataset_version_id(derived_version));
    EG_REQUIRE_EQ(published->provenance().inputs.size(), std::size_t{2});
    EG_REQUIRE(published->provenance().inputs[0].version == left_version->id());
    EG_REQUIRE(published->provenance().inputs[0].content_hash == left_version->content_hash());
    EG_REQUIRE(published->provenance().inputs[1].version == right_version->id());
    EG_REQUIRE_EQ(published->provenance().canonical_parameters, parameters);
    const auto consumers = fixture.store().versions_referencing(left_version->id());
    EG_REQUIRE_EQ(consumers.size(), std::size_t{1});
    EG_REQUIRE(consumers[0] == published->id());

    // An input recorded with a content hash that is not the stored version's is rejected.
    Provenance tampered = record;
    tampered.inputs[0].content_hash = data::hash_bytes("a different payload");
    auto tampered_version = version_of(data::dataset_id(table.next_dataset++),
                                      data::dataset_version_id(table.next_version++),
                                      data::row_domain_id(table.domain), schema, {chunk},
                                      derived.cells.size(), tampered);
    const DraftId tampered_draft = fixture.stage(tampered_version);
    ParameterMap tampered_arguments =
        Fixture::with(data::argument::kDataset, tampered_version->dataset().raw());
    Fixture::put(tampered_arguments, data::argument::kVersionDraft, tampered_draft);
    Fixture::put(tampered_arguments, data::argument::kInputA, left.version);
    Fixture::put(tampered_arguments, data::argument::kInputB, right.version);
    Fixture::put(tampered_arguments, data::argument::kParameters, parameters);
    const auto tampered_outcome =
        fixture.run(data::command_id::kDatasetDerive, std::move(tampered_arguments),
                    fixture.declared_writes({tampered_version->id().raw()}));
    EG_REQUIRE_REJECTED(tampered_outcome, DetailCode::dangling_reference, ErrorCode::invalid_argument);

    // An input the candidate did not record at all is rejected the same way.
    const auto unrecorded = fixture.run(
        data::command_id::kDatasetDerive,
        [&]() {
            ParameterMap attempt = Fixture::with(data::argument::kDataset, derived_dataset);
            Fixture::put(attempt, data::argument::kVersionDraft, draft);
            Fixture::put(attempt, data::argument::kInputA, left.version);
            Fixture::put(attempt, data::argument::kInputB, table.worksheet);
            Fixture::put(attempt, data::argument::kParameters, parameters);
            return attempt;
        }(),
        fixture.declared_writes({derived_version}));
    EG_REQUIRE_REJECTED(unrecorded, DetailCode::dangling_reference, ErrorCode::invalid_argument);
}

EG_TEST("ID", "010", role_unit_and_format_follow_the_binding_through_reorder) {
    Fixture fixture;
    auto table = make_table(12);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(12001), data::row_id(12002)});

    ColumnSpec x;
    x.type = LogicalType::float64;
    x.label = "t";
    x.unit = "s";
    x.role = "x";
    x.display_format = "0.0";
    x.cells = {float64_cell(0.0), float64_cell(1.0)};
    const ColumnLine x_line = create_column(fixture, table, x);

    ColumnSpec y;
    y.type = LogicalType::float64;
    y.label = "v";
    y.unit = "cm";
    y.role = "y";
    y.order_index = 1;
    y.cells = {float64_cell(10.0), float64_cell(20.0)};
    const ColumnLine y_line = create_column(fixture, table, y);

    // Swap the positions and re-declare one role: the metadata travels with the identity.
    ParameterMap move_x = Fixture::with(data::argument::kColumn, x_line.column);
    Fixture::put(move_x, data::argument::kOrderIndex, std::uint64_t{5});
    (void)fixture.require_run(data::command_id::kColumnMove, std::move(move_x),
                              fixture.declared_writes({x_line.column}));
    ParameterMap role = Fixture::with(data::argument::kColumn, x_line.column);
    Fixture::put(role, data::argument::kRole, std::string{"error"});
    (void)fixture.require_run(data::command_id::kColumnMetadata, std::move(role),
                              fixture.declared_writes({x_line.column}));

    const DocumentView view = fixture.document();
    const auto ordered = view.bindings_of(data::worksheet_id(table.worksheet));
    EG_REQUIRE_EQ(ordered.size(), std::size_t{2});
    EG_REQUIRE(ordered[0].id == data::column_id(y_line.column));
    EG_REQUIRE(ordered[1].id == data::column_id(x_line.column));
    EG_REQUIRE_EQ(ordered[1].order_index, std::uint64_t{5});
    EG_REQUIRE_EQ(ordered[1].role, data::ColumnRole::error);
    EG_REQUIRE_EQ(ordered[1].display_format, std::string{"0.0"});
    EG_REQUIRE_EQ(ordered[0].role, data::ColumnRole::y);
    const auto declared_x_unit = data::UnitRegistry::builtin().parse("s");
    const auto declared_y_unit = data::UnitRegistry::builtin().parse("cm");
    EG_REQUIRE(*ordered[1].unit == EG_REQUIRE_OK(declared_x_unit));
    EG_REQUIRE(*ordered[0].unit == EG_REQUIRE_OK(declared_y_unit));
    // Metadata-only commands published no new version, so the values are still the originals.
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{2});
    auto lease = fixture.lease(data::dataset_version_id(x_line.version));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(lease.cell_at(data::field_id(x_line.field), 1)),
                       float64_cell(1.0));
    // Saving and reopening this metadata is P2-006's half of this fixture.
}

EG_TEST("ID", "011", a_lease_keeps_the_snapshot_it_resolved_after_a_successor_is_published) {
    Fixture fixture;
    auto table = make_table(13);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(13001), data::row_id(13002)});

    ColumnSpec spec;
    spec.type = LogicalType::text;
    spec.cells = {text_cell("alpha"), text_cell("beta")};
    ColumnLine line = create_column(fixture, table, spec);

    // The view a reader resolved first, kept alive across the successor.
    const DatasetLease held = fixture.lease(data::dataset_version_id(line.version));
    const std::shared_ptr<const RowDomain> held_domain =
        fixture.row_domain(data::row_domain_id(table.domain));
    const RowId first_row = EG_REQUIRE_TAKEN(held.row_at(0));

    edit_column(fixture, table, line, {text_cell("alpha"), text_cell("gamma")});

    // The held view still returns the snapshot it resolved, and remains valid until released.
    EG_REQUIRE_EQ(held.row_count(), std::uint64_t{2});
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(held.cell_at(data::field_id(line.field), 1)),
                       text_cell("beta"));
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(held.row_at(0)), first_row);
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(held_domain->row_at(1)), data::row_id(13002));
    // A cell read by row identity resolves through the domain, not an array position.
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(held.cell(data::field_id(line.field), first_row)),
                       text_cell("alpha"));
    auto current = fixture.lease(data::dataset_version_id(line.version));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(current.cell_at(data::field_id(line.field), 1)),
                       text_cell("gamma"));
    // Both views order the same row identities, so the reader never resolved a position.
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(current.row_at(0)), first_row);
}

EG_TEST("ID", "012", no_public_surface_mutates_a_published_version) {
    // Compile-time boundary: every read hands out an owned const payload or a value copy, so no
    // reference, span or buffer outlives the version it came from and none is writable.
    static_assert(!std::is_default_constructible_v<DatasetVersion>);
    static_assert(std::is_same_v<decltype(std::declval<const DataStore&>().version(
                                     std::declval<DatasetVersionId>())),
                                 Result<std::shared_ptr<const DatasetVersion>>>);
    static_assert(std::is_same_v<decltype(std::declval<const DatasetLease&>().cell_at(
                                     std::declval<FieldId>(), std::uint64_t{0})),
                                 Result<Cell>>);
    static_assert(std::is_same_v<decltype(std::declval<const DatasetVersion&>().cell(
                                     std::declval<FieldId>(), std::uint64_t{0})),
                                 Result<Cell>>);
    static_assert(std::is_same_v<decltype(std::declval<const Chunk&>().cell_at(std::uint64_t{0})),
                                 Result<Cell>>);
    static_assert(std::is_same_v<decltype(std::declval<const DatasetVersion&>().schema()),
                                 const std::shared_ptr<const DatasetSchema>&>);
    static_assert(std::is_same_v<decltype(std::declval<const RowDomain&>().rows()),
                                 const std::vector<RowId>&>);
    // A chunk is published only by its builder, which fixes the digest over the exact buffer it
    // hands over, so no caller can assemble a chunk whose hash disagrees with its content.
    static_assert(!std::is_constructible_v<Chunk, ChunkId, FieldId, LogicalType, std::uint64_t,
                                           data::ColumnBuffer, std::vector<bool>,
                                           data::ContentHash>);

    Fixture fixture;
    auto table = make_table(14);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(14001)});

    // Staging a candidate is not a mutation: nothing a reader can see changes.
    ColumnSpec spec;
    spec.type = LogicalType::int64;
    spec.cells = {int_cell(5)};
    const std::uint64_t staged_dataset = table.next_dataset++;
    const std::uint64_t staged_field = table.next_field++;
    const std::uint64_t staged_version = table.next_version++;
    auto schema = schema_of({field(data::field_id(staged_field), "value", spec.type)});
    auto chunk = chunk_of(data::field_id(staged_field), spec.type,
                         ChunkId{ObjectId{table.next_chunk++}}, spec.cells);
    auto candidate = version_of(data::dataset_id(staged_dataset),
                               data::dataset_version_id(staged_version),
                               data::row_domain_id(table.domain), schema, {chunk},
                               spec.cells.size(), provenance_of(data::command_id::kColumnCreate));
    const DraftId draft = fixture.stage(candidate);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{0});
    EG_REQUIRE(!fixture.store().contains(data::dataset_version_id(staged_version)));
    EG_REQUIRE(!fixture.store().lease(data::dataset_version_id(staged_version)).ok());
    EG_REQUIRE(fixture.gateway().snapshot().find(ObjectId{staged_version}) == nullptr);

    // A command that fails discards its staged payload instead of publishing it.
    ParameterMap broken = Fixture::with(data::argument::kColumn, table.next_column);
    Fixture::put(broken, data::argument::kWorksheet, table.worksheet + 99);
    Fixture::put(broken, data::argument::kDataset, staged_dataset);
    Fixture::put(broken, data::argument::kVersionDraft, draft);
    Fixture::put(broken, data::argument::kLabel, std::string{"orphan"});
    const auto rejected = fixture.run(data::command_id::kColumnCreate, std::move(broken),
                                     fixture.declared_writes({table.next_column, staged_version}));
    EG_REQUIRE_REJECTED(rejected, DetailCode::dangling_reference, ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{0});
    EG_REQUIRE_EQ(fixture.staged_count(), std::uint64_t{0});
    EG_REQUIRE(!fixture.store().contains(data::dataset_version_id(staged_version)));

    // The same draft stays available for the corrected retry, and only the executor commits.
    ParameterMap corrected = Fixture::with(data::argument::kColumn, table.next_column);
    Fixture::put(corrected, data::argument::kWorksheet, table.worksheet);
    Fixture::put(corrected, data::argument::kDataset, staged_dataset);
    Fixture::put(corrected, data::argument::kVersionDraft, draft);
    Fixture::put(corrected, data::argument::kLabel, std::string{"value"});
    (void)fixture.require_run(data::command_id::kColumnCreate, std::move(corrected),
                              fixture.declared_writes({table.next_column, staged_version}));
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{1});
    EG_REQUIRE(fixture.store().contains(data::dataset_version_id(staged_version)));

    // A mirror aliases that one committed version instead of copying it, and it is read-only:
    // the shared version cannot gain a successor through the alias. The mirror graph, refresh
    // and cycle fixtures stay on P2-010; this is only the mutation boundary.
    const std::uint64_t mirror_column = table.next_column + 1;
    ParameterMap alias = Fixture::with(data::argument::kColumn, mirror_column);
    Fixture::put(alias, data::argument::kWorksheet, table.worksheet);
    Fixture::put(alias, data::argument::kDataset, staged_dataset);
    Fixture::put(alias, data::argument::kLabel, std::string{"alias"});
    Fixture::put(alias, data::argument::kWritePolicy, std::string{"mirror"});
    Fixture::put(alias, data::argument::kSourceVersion, staged_version);
    const auto mirrored = fixture.require_run(data::command_id::kColumnCreate, std::move(alias),
                                              fixture.declared_writes({mirror_column}));
    EG_REQUIRE(mirrored.succeeded());
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{1});
    const DocumentView shared = fixture.document();
    const ColumnBindingRecord* source_binding = shared.find(data::column_id(table.next_column));
    const ColumnBindingRecord* alias_binding = shared.find(data::column_id(mirror_column));
    EG_REQUIRE(source_binding != nullptr);
    EG_REQUIRE(alias_binding != nullptr);
    EG_REQUIRE(alias_binding->version == data::dataset_version_id(staged_version));
    EG_REQUIRE(alias_binding->version == source_binding->version);
    EG_REQUIRE(alias_binding->write_policy == data::WritePolicy::mirror);
    EG_REQUIRE_EQ(shared.columns_using(data::dataset_version_id(staged_version)).size(),
                  std::size_t{2});

    ParameterMap through_alias = Fixture::with(data::argument::kColumn, mirror_column);
    Fixture::put(through_alias, data::argument::kVersionDraft, draft);
    const auto refused = fixture.run(data::command_id::kColumnEdit, std::move(through_alias),
                                     fixture.declared_writes({mirror_column}));
    EG_REQUIRE_REJECTED(refused, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
    EG_REQUIRE_NO_WRITE(refused);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{1});
    EG_REQUIRE_EQ(fixture.staged_count(), std::uint64_t{0});
}
