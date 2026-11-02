#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "edit_fixture.hpp"

namespace {

using namespace egtest;

// One three-row sheet with a measured float64 column and an integer column, which is the smallest
// state that shows a row-shape edit rewriting every same-domain column at once while leaving the
// metadata of each binding alone.
void build_sheet(EditFixture& fixture, SimpleTable& table) {
    create_chain(fixture, table);
    seed_rows(fixture, table, rows_of({11, 12, 13}));

    ColumnSpec height;
    height.type = LogicalType::float64;
    height.label = "height";
    height.unit = "m";
    height.role = "y";
    height.display_format = "0.00";
    height.cells = {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)};
    (void)add_column(fixture, table, height);

    ColumnSpec count;
    count.type = LogicalType::int64;
    count.label = "count";
    count.cells = {int_cell(7), int_cell(8), int_cell(9)};
    (void)add_column(fixture, table, count);
}

// Inserts one row at `at` and returns the outcome with the identities the command published under.
CommandOutcome insert_row(EditFixture& fixture, SimpleTable& table, std::uint64_t at,
                         std::uint64_t new_row, SuccessorNames& names, std::uint64_t& revised) {
    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRowIndex, at);
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({new_row}));
    revised = name_row_domain(table, arguments);
    names = name_successors(table, arguments, columns);
    auto outcome = fixture.run(data::command_id::kWorksheetRowInsert, std::move(arguments),
                               row_edit_writes(fixture, table, revised, columns, names.writes));
    if (outcome.succeeded() && outcome.changed) {
        adopt(table, revised, names, columns);
    }
    return outcome;
}

}  // namespace

// One row-shape edit moves the worksheet's domain and every same-domain column in the same
// revision, because a table view must never show a column ordered by the domain the worksheet left.
EG_TEST("ALIGN", "019", a_row_insert_rewrites_every_same_domain_column_in_one_revision) {
    EditFixture fixture;
    SimpleTable table = make_table(1);
    build_sheet(fixture, table);
    const std::uint64_t prior_domain = table.domain;
    const std::vector<Cell> prior_height = column_cells(fixture, table.columns[0]);
    const std::vector<Cell> prior_count = column_cells(fixture, table.columns[1]);
    const std::uint64_t prior_height_version = table.columns[0].version;
    const std::uint64_t prior_count_version = table.columns[1].version;

    SuccessorNames names;
    std::uint64_t revised = 0;
    (void)insert_row(fixture, table, 1, 910, names, revised);

    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({11, 910, 12, 13})));
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), Cell::missing(LogicalType::float64), float64_cell(2.5),
                   float64_cell(3.5)});
    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(7), Cell::missing(LogicalType::int64), int_cell(8), int_cell(9)});

    // The two columns gain their successors in one revision, so no reader ever sees the sheet
    // split across the old and the new row domain.
    EG_REQUIRE_EQ(names.versions.size(), 2U);
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings.size(), 2U);
    EG_REQUIRE_EQ(bindings[0].revision, bindings[1].revision);
    EG_REQUIRE_EQ(bindings[0].version.raw(), names.versions[0]);
    EG_REQUIRE_EQ(bindings[1].version.raw(), names.versions[1]);
    const auto revised_rows = fixture.row_domain(data::row_domain_id(revised))->rows();
    EG_REQUIRE_EQ(row_text(revised_rows), row_text(rows_of({11, 910, 12, 13})));
    const auto view = fixture.document();
    const auto* sheet = view.find(data::worksheet_id(table.worksheet));
    EG_REQUIRE(sheet != nullptr);
    EG_REQUIRE_EQ(sheet->row_domain.raw(), revised);

    // An edit publishes a successor; the version it replaced keeps its identity and every value.
    require_cells(version_cells(*fixture.version(data::dataset_version_id(prior_height_version)),
                                data::field_id(table.columns[0].field)),
                  prior_height);
    require_cells(version_cells(*fixture.version(data::dataset_version_id(prior_count_version)),
                                data::field_id(table.columns[1].field)),
                  prior_count);
    EG_REQUIRE(table.columns[0].version != prior_height_version);
    EG_REQUIRE(table.columns[1].version != prior_count_version);
    EG_REQUIRE(fixture.object(prior_domain) != std::nullopt);
}

// The metadata a binding carries belongs to the column, not to the version its rows live in.
EG_TEST("ALIGN", "019", a_row_delete_leaves_role_unit_format_and_label_alone) {
    EditFixture fixture;
    SimpleTable table = make_table(2);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({12}));
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    (void)fixture.require_run(data::command_id::kWorksheetRowDelete, std::move(arguments),
                              row_edit_writes(fixture, table, revised, columns, names.writes));
    adopt(table, revised, names, columns);

    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({11, 13})));
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(3.5)});
    require_cells(column_cells(fixture, table.columns[1]), {int_cell(7), int_cell(9)});

    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[0].role, data::ColumnRole::y);
    EG_REQUIRE_EQ(bindings[0].display_format, std::string{"0.00"});
    EG_REQUIRE(bindings[0].unit.has_value());
    EG_REQUIRE_EQ(bindings[0].unit->canonical_text(), std::string{"m"});
    EG_REQUIRE_EQ(bindings[1].label, std::string{"count"});
    EG_REQUIRE_EQ(bindings[0].order_index, 0U);
    EG_REQUIRE_EQ(bindings[1].order_index, 1U);
}

// A move that lands where the run already sits states no difference, so the command writes nothing
// at all and the identities it named stay free for the next edit.
EG_TEST("ALIGN", "019", a_row_edit_that_states_no_difference_publishes_no_object) {
    EditFixture fixture;
    SimpleTable table = make_table(3);
    build_sheet(fixture, table);
    const std::uint64_t undo_depth = fixture.undo_depth();
    const std::uint64_t version_count = fixture.version_count();

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRowIndex, std::uint64_t{0});
    EditFixture::put(arguments, data::argument::kLength, std::uint64_t{2});
    EditFixture::put(arguments, data::argument::kTargetRowIndex, std::uint64_t{0});
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    const auto outcome =
        fixture.require_run(data::command_id::kWorksheetRowMove, std::move(arguments),
                            row_edit_writes(fixture, table, revised, columns, names.writes));

    EG_REQUIRE(!outcome.changed);
    EG_REQUIRE(outcome.events.empty());
    EG_REQUIRE(outcome.revisions.empty());
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    EG_REQUIRE_EQ(table.domain, fixture.document()
                                    .find(data::worksheet_id(table.worksheet))
                                    ->row_domain.raw());
    for (const std::uint64_t version : names.versions) {
        EG_REQUIRE(fixture.object(version) == std::nullopt);
    }
    EG_REQUIRE(fixture.object(revised) == std::nullopt);
}

// Growing a sheet names the identities of the rows it adds and may fill them in the same
// transaction, so the rows and their values arrive as one revision.
EG_TEST("ALIGN", "019", a_resize_that_grows_names_new_identities_and_fills_the_block) {
    EditFixture fixture;
    SimpleTable table = make_table(4);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kTargetRowCount, std::uint64_t{5});
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({940, 941}));
    EditFixture::put(arguments, data::argument::kValues,
                     data::encode_grid(2, 2, {data::float_form(6), data::signed_form(40),
                                              data::float_form(7), data::signed_form(50)}));
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    (void)fixture.require_run(data::command_id::kWorksheetResize, std::move(arguments),
                              row_edit_writes(fixture, table, revised, columns, names.writes));
    adopt(table, revised, names, columns);

    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)),
                  row_text(rows_of({11, 12, 13, 940, 941})));
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5), float64_cell(6),
                   float64_cell(7)});
    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(7), int_cell(8), int_cell(9), int_cell(40), int_cell(50)});
}

// A resize never invents a row identity and never pads a value block, so both ways of leaving the
// new rows unaccounted for are refused.
EG_TEST("ALIGN", "019", a_resize_that_grows_without_new_identities_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(5);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kTargetRowCount, std::uint64_t{4});
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    const auto outcome =
        fixture.run(data::command_id::kWorksheetResize, std::move(arguments),
                    row_edit_writes(fixture, table, revised, columns, names.writes));
    EG_REQUIRE_REJECTED(outcome, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(outcome);

    // The other way is a block that covers rows this resize does not introduce.
    ParameterMap padded = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(padded, data::argument::kTargetRowCount, std::uint64_t{3});
    EditFixture::put(padded, data::argument::kValues,
                     data::encode_grid(1, 2, {data::float_form(9), data::signed_form(90)}));
    const std::uint64_t shrunk = name_row_domain(table, padded);
    const SuccessorNames later_names = name_successors(table, padded, columns);
    const auto refused =
        fixture.run(data::command_id::kWorksheetResize, std::move(padded),
                    row_edit_writes(fixture, table, shrunk, columns, later_names.writes));
    EG_REQUIRE_REJECTED(refused, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(refused);
    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({11, 12, 13})));
}

// A resize that keeps the head of the sheet drops the tail from every column, which is the same
// rectangular rule an insertion follows.
EG_TEST("ALIGN", "019", a_resize_that_shrinks_keeps_the_head_of_every_column) {
    EditFixture fixture;
    SimpleTable table = make_table(6);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kTargetRowCount, std::uint64_t{2});
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    (void)fixture.require_run(data::command_id::kWorksheetResize, std::move(arguments),
                              row_edit_writes(fixture, table, revised, columns, names.writes));
    adopt(table, revised, names, columns);

    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({11, 12})));
    require_cells(column_cells(fixture, table.columns[0]), {float64_cell(1.5), float64_cell(2.5)});
    require_cells(column_cells(fixture, table.columns[1]), {int_cell(7), int_cell(8)});
}

// A position outside the sheet is refused naming the bounds, and no successor identity is spent.
EG_TEST("ALIGN", "019", a_row_edit_outside_the_sheets_bounds_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(7);
    build_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto columns = all_columns(table);
    ParameterMap past = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(past, data::argument::kRowIndex, std::uint64_t{9});
    EditFixture::put(past, data::argument::kRows, EditFixture::id_list({970}));
    const std::uint64_t revised = name_row_domain(table, past);
    const SuccessorNames names = name_successors(table, past, columns);
    const auto beyond = fixture.run(data::command_id::kWorksheetRowInsert, std::move(past),
                                    row_edit_writes(fixture, table, revised, columns, names.writes));
    EG_REQUIRE_REJECTED(beyond, DetailCode::index_out_of_range, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(beyond);

    ParameterMap empty = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(empty, data::argument::kRowIndex, std::uint64_t{0});
    EditFixture::put(empty, data::argument::kLength, std::uint64_t{0});
    EditFixture::put(empty, data::argument::kTargetRowIndex, std::uint64_t{0});
    const std::uint64_t revised_move = name_row_domain(table, empty);
    const SuccessorNames move_names = name_successors(table, empty, columns);
    const auto zero =
        fixture.run(data::command_id::kWorksheetRowMove, std::move(empty),
                    row_edit_writes(fixture, table, revised_move, columns, move_names.writes));
    EG_REQUIRE_REJECTED(zero, DetailCode::index_out_of_range, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(zero);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
}

// A move's two positions are added to the row count by subtraction, never summed by the caller:
// a run whose start and length add past the width of the count wraps back inside it, which would
// reach the row list rather than be refused here. A wrapped run either spends the whole list
// reserve (an unfriendly `internal_error`) or lands as a move that moves nothing and reports
// success, and neither states what the caller asked for.
EG_TEST("ALIGN", "019", a_row_move_whose_run_adds_past_the_row_count_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(19);
    build_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();
    const auto columns = all_columns(table);

    constexpr std::uint64_t kPast = std::numeric_limits<std::uint64_t>::max() - 2;
    ParameterMap wrapped = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(wrapped, data::argument::kRowIndex, kPast);
    EditFixture::put(wrapped, data::argument::kLength, std::uint64_t{5});
    EditFixture::put(wrapped, data::argument::kTargetRowIndex, std::uint64_t{0});
    const std::uint64_t wrapped_domain = name_row_domain(table, wrapped);
    const SuccessorNames wrapped_names = name_successors(table, wrapped, columns);
    const auto overflow =
        fixture.run(data::command_id::kWorksheetRowMove, std::move(wrapped),
                    row_edit_writes(fixture, table, wrapped_domain, columns, wrapped_names.writes));
    EG_REQUIRE_REJECTED(overflow, DetailCode::index_out_of_range, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(overflow);

    // One row at the last addressable position is the same request with no room to wrap, and is
    // refused for the same reason rather than accepted as a move that moves nothing.
    ParameterMap last = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(last, data::argument::kRowIndex, std::numeric_limits<std::uint64_t>::max());
    EditFixture::put(last, data::argument::kLength, std::uint64_t{1});
    EditFixture::put(last, data::argument::kTargetRowIndex, std::uint64_t{0});
    const std::uint64_t last_domain = name_row_domain(table, last);
    const SuccessorNames last_names = name_successors(table, last, columns);
    const auto edge = fixture.run(data::command_id::kWorksheetRowMove, std::move(last),
                                  row_edit_writes(fixture, table, last_domain, columns,
                                                  last_names.writes));
    EG_REQUIRE_REJECTED(edge, DetailCode::index_out_of_range, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(edge);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
}

// A mirror shares another binding's version identity, so rewriting it would rewrite a column the
// caller never asked to edit. The rule holds for the row-shape family, which is where it is easiest
// to reach by accident.
EG_TEST("ALIGN", "020", a_row_edit_of_a_sheet_holding_a_mirror_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(8);
    build_sheet(fixture, table);
    const ColumnLine mirror = add_mirror(fixture, table, table.columns[0]);
    const std::uint64_t version_count = fixture.version_count();

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({12}));
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    const auto outcome =
        fixture.run(data::command_id::kWorksheetRowDelete, std::move(arguments),
                    row_edit_writes(fixture, table, revised, columns, names.writes));

    EG_REQUIRE_REJECTED(outcome, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
    EG_REQUIRE_NO_WRITE(outcome);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    // The mirror stays an alias of the source version: refusing the edit changed neither binding.
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings.size(), 3U);
    EG_REQUIRE_EQ(bindings[2].id.raw(), mirror.column);
    EG_REQUIRE_EQ(bindings[2].write_policy, data::WritePolicy::mirror);
    EG_REQUIRE_EQ(bindings[2].version.raw(), table.columns[0].version);
}

// Undo restores the exact prior identities: the domain the worksheet pointed at, the version each
// binding pointed at and the row identities themselves.
EG_TEST("ID", "005", undo_restores_the_prior_domain_and_version_identities) {
    EditFixture fixture;
    SimpleTable table = make_table(9);
    build_sheet(fixture, table);
    const std::uint64_t prior_domain = table.domain;
    const std::uint64_t prior_height = table.columns[0].version;
    const std::uint64_t prior_count = table.columns[1].version;
    const std::vector<Cell> height_before = column_cells(fixture, table.columns[0]);
    const std::vector<Cell> count_before = column_cells(fixture, table.columns[1]);

    const std::uint64_t undo_depth = fixture.undo_depth();
    SuccessorNames names;
    std::uint64_t revised = 0;
    (void)insert_row(fixture, table, 0, 901, names, revised);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth + 1);

    (void)fixture.require_undo();
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth);
    EG_REQUIRE_EQ(fixture.redo_depth(), 1U);
    const auto view = fixture.document();
    const auto* sheet = view.find(data::worksheet_id(table.worksheet));
    EG_REQUIRE(sheet != nullptr);
    EG_REQUIRE_EQ(sheet->row_domain.raw(), prior_domain);
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].version.raw(), prior_height);
    EG_REQUIRE_EQ(bindings[1].version.raw(), prior_count);
    // Undo restores the metadata the binding declares too, not only the version it points at: a
    // role, unit, format or label that drifted would change how the same values read.
    EG_REQUIRE_EQ(bindings[0].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[0].role, data::ColumnRole::y);
    EG_REQUIRE_EQ(bindings[0].display_format, std::string{"0.00"});
    EG_REQUIRE(bindings[0].unit.has_value());
    EG_REQUIRE_EQ(bindings[0].unit->canonical_text(), std::string{"m"});
    EG_REQUIRE_EQ(bindings[1].label, std::string{"count"});
    EG_REQUIRE_EQ(bindings[0].order_index, 0U);
    EG_REQUIRE_EQ(bindings[1].order_index, 1U);
    EG_REQUIRE(fixture.object(revised) == std::nullopt);
    for (const std::uint64_t version : names.versions) {
        EG_REQUIRE(fixture.object(version) == std::nullopt);
    }
    require_cells(version_cells(*fixture.version(data::dataset_version_id(prior_height)),
                                data::field_id(table.columns[0].field)),
                  height_before);
    require_cells(version_cells(*fixture.version(data::dataset_version_id(prior_count)),
                                data::field_id(table.columns[1].field)),
                  count_before);
    EG_REQUIRE_EQ(row_text(fixture.row_domain(data::row_domain_id(prior_domain))->rows()),
                  row_text(rows_of({11, 12, 13})));
}

// Redo puts the edit back exactly as it was, and a fresh edit after an undo closes the branch.
EG_TEST("ID", "005", redo_reapplies_a_row_edit_and_a_new_edit_clears_the_redo_stack) {
    EditFixture fixture;
    SimpleTable table = make_table(10);
    build_sheet(fixture, table);

    const std::uint64_t undo_depth = fixture.undo_depth();
    SuccessorNames names;
    std::uint64_t revised = 0;
    (void)insert_row(fixture, table, 2, 902, names, revised);
    const std::uint64_t revised_height = table.columns[0].version;
    (void)fixture.require_undo();
    (void)fixture.require_redo();

    EG_REQUIRE_EQ(fixture.redo_depth(), 0U);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth + 1);
    EG_REQUIRE_EQ(sheet_bindings(fixture, table.worksheet)[0].version.raw(), revised_height);
    const auto view = fixture.document();
    EG_REQUIRE_EQ(view.find(data::worksheet_id(table.worksheet))->row_domain.raw(), revised);

    SuccessorNames second;
    std::uint64_t second_domain = 0;
    (void)insert_row(fixture, table, 1, 903, second, second_domain);
    EG_REQUIRE_EQ(fixture.redo_depth(), 0U);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth + 2U);
    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)),
                  row_text(rows_of({11, 903, 12, 902, 13})));
}

// A row edit takes its identities from the caller, so one that names a live identity is refused
// before anything is published rather than re-keying what already exists.
EG_TEST("ID", "005", a_row_edit_that_reuses_a_live_identity_is_refused_before_publishing) {
    EditFixture fixture;
    SimpleTable table = make_table(11);
    build_sheet(fixture, table);
    const std::uint64_t object_count = fixture.document().object_count();

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRows, EditFixture::id_list({911}));
    EditFixture::put(arguments, data::argument::kRowIndex, std::uint64_t{0});
    // The row is new, but the revised domain names the domain the worksheet already points at.
    EditFixture::put(arguments, data::argument::kRevisedDomain, table.domain);
    const SuccessorNames names = name_successors(table, arguments, columns);
    const auto on_domain =
        fixture.run(data::command_id::kWorksheetRowInsert, std::move(arguments),
                    row_edit_writes(fixture, table, table.domain, columns, names.writes));
    EG_REQUIRE_REJECTED(on_domain, DetailCode::duplicate_id, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(on_domain);
    EG_REQUIRE_EQ(fixture.document().object_count(), object_count);
    for (const std::uint64_t version : names.versions) {
        EG_REQUIRE(fixture.object(version) == std::nullopt);
    }
}

// The row list is an ordered set of stable identities: a move reorders it and renames nothing, so
// the same values travel with the same rows.
EG_TEST("ID", "013", a_row_move_reorders_the_domain_without_renaming_a_row) {
    EditFixture fixture;
    SimpleTable table = make_table(12);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kRowIndex, std::uint64_t{0});
    EditFixture::put(arguments, data::argument::kLength, std::uint64_t{2});
    EditFixture::put(arguments, data::argument::kTargetRowIndex, std::uint64_t{1});
    const std::uint64_t revised = name_row_domain(table, arguments);
    const SuccessorNames names = name_successors(table, arguments, columns);
    (void)fixture.require_run(data::command_id::kWorksheetRowMove, std::move(arguments),
                              row_edit_writes(fixture, table, revised, columns, names.writes));
    adopt(table, revised, names, columns);

    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({13, 11, 12})));
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(3.5), float64_cell(1.5), float64_cell(2.5)});
    require_cells(column_cells(fixture, table.columns[1]), {int_cell(9), int_cell(7), int_cell(8)});
    EG_REQUIRE_EQ(fixture.row_domain(data::row_domain_id(revised))->row_count(), 3U);
}

// A cell edit addresses its row by identity, so after a re-sort it still lands on the same row
// rather than on the position the user first saw.
EG_TEST("ID", "013", a_cell_edit_after_a_move_finds_its_row_by_identity) {
    EditFixture fixture;
    SimpleTable table = make_table(13);
    build_sheet(fixture, table);

    const auto columns = all_columns(table);
    ParameterMap move = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(move, data::argument::kRowIndex, std::uint64_t{0});
    EditFixture::put(move, data::argument::kLength, std::uint64_t{1});
    EditFixture::put(move, data::argument::kTargetRowIndex, std::uint64_t{2});
    const std::uint64_t revised = name_row_domain(table, move);
    const SuccessorNames names = name_successors(table, move, columns);
    (void)fixture.require_run(data::command_id::kWorksheetRowMove, std::move(move),
                              row_edit_writes(fixture, table, revised, columns, names.writes));
    adopt(table, revised, names, columns);
    EG_REQUIRE_EQ(row_text(sheet_rows(fixture, table)), row_text(rows_of({12, 13, 11})));

    ParameterMap set = EditFixture::with(data::argument::kColumn, table.columns[0].column);
    EditFixture::put(set, data::argument::kRow, std::uint64_t{11});
    EditFixture::put(set, data::argument::kValue, data::encode_value(data::signed_form(99)));
    const std::vector<ColumnLine> one{table.columns[0]};
    const SuccessorNames edited = name_successors(table, set, one);
    (void)fixture.require_run(data::command_id::kWorksheetCellSet, std::move(set),
                              fixture.declared_writes(edited.writes));
    adopt(table, edited, table.columns[0]);

    // Row 11 is now the last row, and the value followed it there.
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(2.5), float64_cell(3.5), float64_cell(99)});
}

// A new row keeps the identity it was given and holds no value until an edit declares one.
EG_TEST("ID", "013", an_inserted_row_keeps_its_identity_and_holds_no_value) {
    EditFixture fixture;
    SimpleTable table = make_table(14);
    build_sheet(fixture, table);

    SuccessorNames names;
    std::uint64_t revised = 0;
    (void)insert_row(fixture, table, 2, 907, names, revised);
    auto domain = fixture.row_domain(data::row_domain_id(revised));
    const auto at = domain->index_of(data::row_id(907));
    EG_REQUIRE(at.has_value());
    EG_REQUIRE_EQ(*at, 2U);

    // A second insert before it moves the position, never the identity.
    SuccessorNames again;
    std::uint64_t next_domain = 0;
    (void)insert_row(fixture, table, 0, 908, again, next_domain);
    auto moved = fixture.row_domain(data::row_domain_id(next_domain));
    EG_REQUIRE_EQ(row_text(moved->rows()), row_text(rows_of({908, 11, 12, 907, 13})));
    EG_REQUIRE_EQ(*moved->index_of(data::row_id(907)), 3U);
    require_cells(column_cells(fixture, table.columns[0]),
                  {Cell::missing(LogicalType::float64), float64_cell(1.5), float64_cell(2.5),
                   Cell::missing(LogicalType::float64), float64_cell(3.5)});
}
