#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "edit_fixture.hpp"

namespace {

using namespace egtest;

using everygraph::foundation::ObjectId;

// A two-column sheet: enough state that one row edit touches five objects, and few enough that a
// case can name each of them.
void build_history_sheet(EditFixture& fixture, SimpleTable& table) {
    create_chain(fixture, table);
    seed_rows(fixture, table, rows_of({41, 42, 43}));

    ColumnSpec height;
    height.type = LogicalType::float64;
    height.label = "height";
    height.unit = "m";
    height.cells = {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)};
    (void)add_column(fixture, table, height);

    ColumnSpec count;
    count.type = LogicalType::int64;
    count.label = "count";
    count.cells = {int_cell(7), int_cell(8), int_cell(9)};
    (void)add_column(fixture, table, count);
}

// Inserts one row and hands back the outcome with the identities it published, so a case can name
// every object one transaction moved.
struct RowWrite {
    CommandOutcome outcome;
    SuccessorNames names;
    std::uint64_t revised{0};
    std::vector<ColumnLine> columns;
};

[[nodiscard]] RowWrite insert_row(EditFixture& fixture, SimpleTable& table, std::uint64_t at,
                                  std::uint64_t new_row) {
    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRowIndex, at);
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({new_row}));
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    auto outcome = fixture.run(data::command_id::kWorksheetRowInsert, std::move(arguments),
                               row_edit_writes(fixture, table, revised, columns, names.writes));
    if (!outcome.succeeded()) {
        throw TestFailure("row insert failed: " + (outcome.error.has_value()
                                                       ? outcome.error->message
                                                       : std::string{"no error"}));
    }
    if (outcome.changed) {
        adopt(table, revised, names, columns);
    }
    return RowWrite{std::move(outcome), names, revised, columns};
}

[[nodiscard]] std::vector<ObjectId> event_ids(const DocumentEvent& event) {
    std::vector<ObjectId> ids;
    for (const ObjectChange& change : event.changes) {
        ids.push_back(change.object_id);
    }
    return ids;
}

}  // namespace

// One row edit is one transaction: the revised domain, both successor versions and both bindings
// move together, so no reader can see a column still ordered by the domain the sheet left.
EG_TEST("ID", "015", a_row_edit_publishes_domain_columns_and_versions_in_one_revision) {
    EditFixture fixture;
    SimpleTable table = make_table(60);
    build_history_sheet(fixture, table);

    const auto write = insert_row(fixture, table, 1, 920);
    EG_REQUIRE(write.outcome.changed);
    EG_REQUIRE_EQ(write.outcome.events.size(), 1U);
    const auto& event = write.outcome.events.front();
    EG_REQUIRE_EQ(event.transaction_id, write.outcome.transaction_id);

    // Six objects moved in this one revision: the worksheet, the revised domain, both bindings
    // and both successor versions.
    EG_REQUIRE_EQ(event.changes.size(), 6U);
    EG_REQUIRE_EQ(event.changes.size(), write.outcome.revisions.size());
    for (const ObjectChange& change : event.changes) {
        const auto found = write.outcome.revisions.find(change.object_id);
        EG_REQUIRE(found != write.outcome.revisions.end());
        EG_REQUIRE_EQ(found->second, change.new_revision);
        EG_REQUIRE(change.kind == app::ChangeKind::created ||
                   change.kind == app::ChangeKind::updated);
    }
    const auto moved = event_ids(event);
    EG_REQUIRE(std::find(moved.begin(), moved.end(), ObjectId{table.worksheet}) != moved.end());
    EG_REQUIRE(std::find(moved.begin(), moved.end(), ObjectId{write.revised}) != moved.end());
    for (const ColumnLine& line : write.columns) {
        EG_REQUIRE(std::find(moved.begin(), moved.end(), ObjectId{line.column}) != moved.end());
    }
    for (const std::uint64_t version : write.names.versions) {
        EG_REQUIRE(std::find(moved.begin(), moved.end(), ObjectId{version}) != moved.end());
    }
    // The subscriber is handed the very event the caller is told about, last in publish order.
    EG_REQUIRE(!fixture.events().empty());
    EG_REQUIRE_EQ(fixture.events().back().transaction_id, write.outcome.transaction_id);
    EG_REQUIRE_EQ(fixture.events().back().changes.size(), event.changes.size());
}

// An edit of one cell names exactly the two objects it changed: the binding and the version it
// now points at. Nothing else in the sheet moved, so a dependent view has nothing else to redo.
EG_TEST("ID", "015", a_cell_edit_event_names_the_binding_and_its_new_version_only) {
    EditFixture fixture;
    SimpleTable table = make_table(61);
    build_history_sheet(fixture, table);
    const std::uint64_t column = table.columns[0].column;

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{42});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(2.75)));
    const SuccessorNames names = name_successors(table, arguments, {table.columns[0]});
    const auto outcome = fixture.require_run(data::command_id::kWorksheetCellSet,
                                             std::move(arguments),
                                             fixture.declared_writes(names.writes));

    EG_REQUIRE(outcome.changed);
    EG_REQUIRE_EQ(outcome.events.size(), 1U);
    const auto& event = outcome.events.front();
    EG_REQUIRE_EQ(event.changes.size(), 2U);
    for (const ObjectChange& change : event.changes) {
        EG_REQUIRE(change.object_id == ObjectId{column} ||
                   change.object_id == ObjectId{names.versions[0]});
        EG_REQUIRE(change.kind == app::ChangeKind::updated ||
                   change.kind == app::ChangeKind::created);
    }
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].revision, outcome.revisions.at(ObjectId{column}));
    // The neighbour did not move at all, so it appears in no event.
    const auto moved = event_ids(event);
    EG_REQUIRE(std::find(moved.begin(), moved.end(),
                         ObjectId{table.columns[1].column}) == moved.end());
}

// One undo reverses the whole transaction rather than one column of it, and puts every object back
// at a revision higher than either state it had, so a reader cannot mistake it for the old one.
EG_TEST("ID", "014", undoing_a_row_edit_reverses_the_whole_transaction) {
    EditFixture fixture;
    SimpleTable table = make_table(62);
    build_history_sheet(fixture, table);
    const std::uint64_t prior_domain = table.domain;
    const std::uint64_t prior_height = table.columns[0].version;
    const std::uint64_t prior_count = table.columns[1].version;

    const std::uint64_t undo_depth = fixture.undo_depth();
    const auto write = insert_row(fixture, table, 0, 921);
    const std::uint64_t depth = fixture.undo_depth();
    EG_REQUIRE_EQ(depth, undo_depth + 1U);

    const auto undone = fixture.require_undo();
    EG_REQUIRE(undone.changed);
    EG_REQUIRE_EQ(undone.events.size(), 1U);
    EG_REQUIRE_EQ(undone.events.front().changes.size(), write.outcome.revisions.size());
    EG_REQUIRE_EQ(fixture.undo_depth(), depth - 1U);
    EG_REQUIRE_EQ(fixture.redo_depth(), 1U);

    const auto view = fixture.document();
    const auto* sheet = view.find(data::worksheet_id(table.worksheet));
    EG_REQUIRE(sheet != nullptr);
    EG_REQUIRE_EQ(sheet->row_domain.raw(), prior_domain);
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].version.raw(), prior_height);
    EG_REQUIRE_EQ(bindings[1].version.raw(), prior_count);
    EG_REQUIRE(fixture.object(write.revised) == std::nullopt);
    for (const std::uint64_t version : write.names.versions) {
        EG_REQUIRE(fixture.object(version) == std::nullopt);
    }
    EG_REQUIRE_EQ(row_text(fixture.row_domain(data::row_domain_id(prior_domain))->rows()),
                  row_text(rows_of({41, 42, 43})));
    require_cells(version_cells(*fixture.version(bindings[0].version),
                                data::field_id(table.columns[0].field)),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)});
}

// A redo restores the edit under the very identities it first used: the domain and the two
// versions come back with the same numbers, so anything that had resolved them still matches.
EG_TEST("ID", "014", redo_restores_the_edit_under_the_same_identities) {
    EditFixture fixture;
    SimpleTable table = make_table(63);
    build_history_sheet(fixture, table);

    const auto write = insert_row(fixture, table, 2, 922);
    const std::uint64_t depth = fixture.undo_depth();
    (void)fixture.require_undo();
    EG_REQUIRE_EQ(fixture.redo_depth(), 1U);
    const auto redone = fixture.require_redo();

    EG_REQUIRE(redone.changed);
    EG_REQUIRE_EQ(fixture.redo_depth(), 0U);
    EG_REQUIRE_EQ(fixture.undo_depth(), depth);
    EG_REQUIRE(fixture.object(write.revised) != std::nullopt);
    for (const std::uint64_t version : write.names.versions) {
        EG_REQUIRE(fixture.object(version) != std::nullopt);
    }
    EG_REQUIRE_EQ(fixture.document().find(data::worksheet_id(table.worksheet))->row_domain.raw(),
                  write.revised);
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].version.raw(), write.names.versions[0]);
    EG_REQUIRE_EQ(bindings[1].version.raw(), write.names.versions[1]);
    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)),
                  row_text(rows_of({41, 42, 922, 43})));
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.5),
                   Cell::missing(LogicalType::float64), float64_cell(3.5)});
}

// Repeating the last action re-issues the same request against the revisions the objects hold now.
// A cell edit that already landed states no difference the second time, so it publishes nothing
// rather than clashing with the identities it names.
EG_TEST("ID", "014", repeating_a_cell_edit_is_a_no_op_rather_than_a_second_revision) {
    EditFixture fixture;
    SimpleTable table = make_table(64);
    build_history_sheet(fixture, table);

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, table.columns[0].column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{41});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(8)));
    const SuccessorNames names = name_successors(table, arguments, {table.columns[0]});
    const auto first = fixture.require_run(data::command_id::kWorksheetCellSet, std::move(arguments),
                                           fixture.declared_writes(names.writes));
    EG_REQUIRE(first.changed);
    adopt(table, names, table.columns[0]);
    const std::uint64_t version_count_after = fixture.version_count();
    const std::uint64_t undo_depth_after = fixture.undo_depth();

    const auto again = fixture.repeat_last();
    EG_REQUIRE(again.succeeded());
    EG_REQUIRE(!again.changed);
    EG_REQUIRE(again.events.empty());
    EG_REQUIRE(again.revisions.empty());
    EG_REQUIRE_EQ(fixture.version_count(), version_count_after);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth_after);
    EG_REQUIRE_EQ(fixture.object(names.versions[0])->revision, 1U);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(8), float64_cell(2.5), float64_cell(3.5)});
}

// A command that is not idempotent declines to repeat instead of silently applying a second edit:
// inserting the same row again would name an identity the sheet now holds.
EG_TEST("ID", "014", a_row_insert_declines_to_repeat) {
    EditFixture fixture;
    SimpleTable table = make_table(65);
    build_history_sheet(fixture, table);
    const std::uint64_t depth = fixture.undo_depth();

    const auto write = insert_row(fixture, table, 1, 923);
    EG_REQUIRE(write.outcome.changed);

    const auto again = fixture.repeat_last();
    EG_REQUIRE(!again.succeeded());
    EG_REQUIRE(again.error.has_value());
    EG_REQUIRE(again.error->code == ErrorCode::not_repeatable);
    EG_REQUIRE(again.error->message.find(data::command_id::kWorksheetRowInsert) !=
               std::string::npos);
    EG_REQUIRE_EQ(fixture.undo_depth(), depth + 1U);
    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({41, 923, 42, 43})));
}

// History has its own empty states, and each is reported with the code that says which one.
EG_TEST("ID", "014", an_empty_history_reports_nothing_to_undo_and_nothing_to_repeat) {
    EditFixture fixture;

    const auto undo = fixture.undo();
    EG_REQUIRE(!undo.succeeded());
    EG_REQUIRE(undo.error.has_value());
    EG_REQUIRE(undo.error->code == ErrorCode::nothing_to_undo);

    const auto repeat = fixture.repeat_last();
    EG_REQUIRE(!repeat.succeeded());
    EG_REQUIRE(repeat.error.has_value());
    EG_REQUIRE(repeat.error->code == ErrorCode::not_repeatable);
    EG_REQUIRE(!repeat.changed);
    EG_REQUIRE_EQ(fixture.undo_depth(), 0U);
    EG_REQUIRE_EQ(fixture.redo_depth(), 0U);

    const auto redo = fixture.redo();
    EG_REQUIRE(!redo.succeeded());
    EG_REQUIRE(redo.error.has_value());
    EG_REQUIRE(redo.error->code == ErrorCode::nothing_to_redo);
    EG_REQUIRE_EQ(fixture.redo_depth(), 0U);
}

// A mutation issued from inside an event listener is refused by the kernel: the listener sees a
// revision that is already committed and can never write over it.
EG_TEST("ID", "016", a_command_issued_from_an_event_listener_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(67);
    build_history_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    ParameterMap probe = EditFixture::with(data::argument::kColumn, table.columns[0].column);
    EditFixture::put(probe, data::argument::kRow, std::uint64_t{42});
    EditFixture::put(probe, data::argument::kValue,
                     data::encode_value(data::float_form(6)));
    const SuccessorNames probe_names = name_successors(table, probe, {table.columns[0]});
    fixture.arm_reentry_probe(data::command_id::kWorksheetCellSet, std::move(probe));

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, table.columns[0].column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{41});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(5)));
    const SuccessorNames names = name_successors(table, arguments, {table.columns[0]});
    const auto outcome = fixture.require_run(data::command_id::kWorksheetCellSet,
                                             std::move(arguments),
                                             fixture.declared_writes(names.writes));
    EG_REQUIRE(outcome.changed);
    adopt(table, names, table.columns[0]);

    const auto& inside = fixture.reentry_probe();
    EG_REQUIRE(inside.has_value());
    EG_REQUIRE(!inside->succeeded());
    EG_REQUIRE(inside->error.has_value());
    EG_REQUIRE(inside->error->code == ErrorCode::reentrant_mutation);
    EG_REQUIRE(!inside->changed);
    // The refused write published nothing: only the listener's own edit is in the store.
    EG_REQUIRE_EQ(fixture.version_count(), version_count + 1U);
    EG_REQUIRE(fixture.object(probe_names.versions[0]) == std::nullopt);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(5), float64_cell(2.5), float64_cell(3.5)});
}

// A command may only change objects the caller named in its expected revisions. An edit that
// reaches for a binding nobody declared is refused by the kernel, so no command has a side path
// into the document.
EG_TEST("ID", "016", an_undeclared_object_cannot_be_changed_by_a_command) {
    EditFixture fixture;
    SimpleTable table = make_table(68);
    build_history_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, table.columns[0].column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{43});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(7)));
    const SuccessorNames names = name_successors(table, arguments, {table.columns[0]});
    // Only the successor version is declared; the binding the command also rewrites is not.
    ExpectedRevisions undeclared;
    undeclared.writes[ObjectId{names.versions[0]}] = everygraph::foundation::kInitialRevision;
    const auto outcome = fixture.run(data::command_id::kWorksheetCellSet, std::move(arguments),
                                     std::move(undeclared));

    EG_REQUIRE(!outcome.succeeded());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::invalid_argument);
    EG_REQUIRE(outcome.error->message.find("outside its declared write set") != std::string::npos);
    EG_REQUIRE_NO_WRITE(outcome);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)});
}

// The last action records the revisions its command produced, so a client that repeats or audits
// an edit reads the same contract the editor was told about.
EG_TEST("ID", "014", the_last_action_records_the_revisions_its_command_produced) {
    EditFixture fixture;
    SimpleTable table = make_table(69);
    build_history_sheet(fixture, table);

    const auto write = insert_row(fixture, table, 0, 924);
    const auto last = fixture.last_action();
    EG_REQUIRE(last.has_value());
    EG_REQUIRE_EQ(last->command_id, std::string{data::command_id::kWorksheetRowInsert});
    EG_REQUIRE_EQ(last->transaction_id, write.outcome.transaction_id);
    EG_REQUIRE_EQ(last->result_revisions.size(), write.outcome.revisions.size());
    for (const auto& entry : write.outcome.revisions) {
        const auto found = last->result_revisions.find(entry.first);
        EG_REQUIRE(found != last->result_revisions.end());
        EG_REQUIRE_EQ(found->second, entry.second);
    }
    EG_REQUIRE(last->repeatability != app::Repeatability::repeatable);
}
