#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "edit_fixture.hpp"

namespace {

using namespace egtest;

// A measured float64 column, a plain integer column, a one-byte column and a categorical column:
// the four ways a typed cell edit can be asked for something the field does not hold.
void build_cell_sheet(EditFixture& fixture, SimpleTable& table) {
    create_chain(fixture, table);
    seed_rows(fixture, table, rows_of({21, 22, 23}));

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

    ColumnSpec narrow;
    narrow.type = LogicalType::int8;
    narrow.label = "flags";
    narrow.cells = {int_cell(1, LogicalType::int8), int_cell(2, LogicalType::int8),
                   int_cell(3, LogicalType::int8)};
    (void)add_column(fixture, table, narrow);

    ColumnSpec kind;
    kind.type = LogicalType::categorical;
    kind.label = "kind";
    kind.dictionary = dictionary_of({"low", "high"});
    kind.cells = {category_cell(0), category_cell(1), category_cell(0)};
    (void)add_column(fixture, table, kind);
}

struct CellWrite {
    CommandOutcome outcome;
    SuccessorNames names;
    std::vector<ColumnLine> covered;
};

// One cell of one column, written through the command. The caller adopts the successor only when
// the edit published, so a refused write leaves the table record pointing at the version that
// still holds the sheet's values.
[[nodiscard]] CellWrite set_cell(EditFixture& fixture, SimpleTable& table, std::size_t position,
                                std::uint64_t row, const std::string& value,
                                const std::string& source_unit = {}) {
    const ColumnLine line = table.columns[position];
    ParameterMap arguments = EditFixture::with(data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kRow, row);
    EditFixture::put(arguments, data::argument::kValue, value);
    if (!source_unit.empty()) {
        EditFixture::put(arguments, data::argument::kSourceUnit, source_unit);
    }
    const std::vector<ColumnLine> covered{line};
    const SuccessorNames names = name_successors(table, arguments, covered);
    auto outcome = fixture.run(data::command_id::kWorksheetCellSet, std::move(arguments),
                               fixture.declared_writes(names.writes));
    return CellWrite{std::move(outcome), names, covered};
}

// Moves the caller's record of the sheet onto the versions a published edit rewrote.
inline void adopt_write(SimpleTable& table, const CellWrite& write) {
    adopt(table, 0, write.names, write.covered);
}

[[nodiscard]] CellWrite require_set(EditFixture& fixture, SimpleTable& table, std::size_t position,
                                    std::uint64_t row, const std::string& value,
                                    const std::string& source_unit = {}) {
    CellWrite write = set_cell(fixture, table, position, row, value, source_unit);
    if (!write.outcome.succeeded()) {
        throw TestFailure("cell set failed: " + (write.outcome.error.has_value()
                                                     ? write.outcome.error->message
                                                     : std::string{"no error"}));
    }
    if (write.outcome.changed) {
        adopt_write(table, write);
    }
    return write;
}

// One block paste: the anchor row and column plus the width say which columns publish successors,
// so the successor identities are named for exactly that slice, in display order. A block wider
// than the sheet still has to name one identity per declared column, so the tail reuses the last
// column's slot; the command refuses that shape before it writes anything.
[[nodiscard]] CellWrite paste_block(EditFixture& fixture, SimpleTable& table, std::size_t position,
                                    std::uint64_t row, const std::string& block,
                                    std::size_t width) {
    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kColumn, table.columns[position].column);
    EditFixture::put(arguments, data::argument::kRow, row);
    EditFixture::put(arguments, data::argument::kValues, block);
    std::vector<ColumnLine> covered;
    for (std::size_t index = 0; index < width; ++index) {
        const std::size_t at = position + index;
        covered.push_back(at < table.columns.size() ? table.columns[at] : table.columns.back());
    }
    const SuccessorNames names = name_successors(table, arguments, covered);
    auto outcome = fixture.run(data::command_id::kWorksheetCellPaste, std::move(arguments),
                               fixture.declared_writes(names.writes));
    return CellWrite{std::move(outcome), names, covered};
}

[[nodiscard]] CellWrite require_paste(EditFixture& fixture, SimpleTable& table,
                                      std::size_t position, std::uint64_t row,
                                      const std::string& block, std::size_t width) {
    CellWrite write = paste_block(fixture, table, position, row, block, width);
    if (!write.outcome.succeeded()) {
        throw TestFailure("block paste failed: " + (write.outcome.error.has_value()
                                                        ? write.outcome.error->message
                                                        : std::string{"no error"}));
    }
    if (write.outcome.changed) {
        adopt_write(table, write);
    }
    return write;
}

}  // namespace

// A cell edit restates one value and publishes a successor version for that column alone, so the
// version the sheet held before stays exactly readable.
EG_TEST("TYP", "019", a_cell_edit_publishes_a_successor_and_leaves_the_prior_version_intact) {
    EditFixture fixture;
    SimpleTable table = make_table(20);
    build_cell_sheet(fixture, table);
    const std::uint64_t prior_version = table.columns[0].version;
    const std::uint64_t count_version = table.columns[1].version;

    const auto write = require_set(fixture, table, 0, 22, data::encode_value(data::float_form(2.75)));

    EG_REQUIRE(write.outcome.changed);
    EG_REQUIRE(table.columns[0].version != prior_version);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.75), float64_cell(3.5)});
    require_cells(version_cells(*fixture.version(data::dataset_version_id(prior_version)),
                                data::field_id(table.columns[0].field)),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)});
    // Only the edited column moves: the neighbour keeps its version identity.
    EG_REQUIRE_EQ(table.columns[1].version, count_version);
    EG_REQUIRE_EQ(write.names.versions.size(), 1U);
}

// The value form names a storage class and the field decides whether it accepts one, so a text
// into a measured column is refused rather than parsed.
EG_TEST("TYP", "019", a_text_value_in_a_numeric_column_is_refused_and_the_value_stands) {
    EditFixture fixture;
    SimpleTable table = make_table(21);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> before = column_cells(fixture, table.columns[0]);

    const auto write = set_cell(fixture, table, 0, 21, data::encode_value(data::text_form("n/a")));
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[0]), before);
}

// A column that declares no missing state keeps that declaration through an edit: a blank is a
// refusal, not a zero, because a zero would read as a measurement of nothing.
EG_TEST("TYP", "019", a_blank_cell_in_a_column_that_declares_no_missing_state_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(22);
    create_chain(fixture, table);
    seed_rows(fixture, table, rows_of({24, 25}));

    ColumnSpec required;
    required.type = LogicalType::float64;
    required.label = "dose";
    required.cells = {float64_cell(1), float64_cell(2)};
    (void)add_column(fixture, table, required, true);
    const std::uint64_t prior_version = table.columns[0].version;

    const auto write = set_cell(fixture, table, 0, 24, data::encode_value(data::missing_form()));
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
    EG_REQUIRE_EQ(table.columns[0].version, prior_version);
    require_cells(column_cells(fixture, table.columns[0]), {float64_cell(1), float64_cell(2)});
}

// The same blank in a nullable column is stored as the missing state of that type, which a reader
// sees as missing and not as zero.
EG_TEST("TYP", "019", a_blank_cell_in_a_nullable_column_is_stored_as_a_missing_value) {
    EditFixture fixture;
    SimpleTable table = make_table(23);
    build_cell_sheet(fixture, table);

    (void)require_set(fixture, table, 0, 22, data::encode_value(data::missing_form()));

    const auto cells = column_cells(fixture, table.columns[0]);
    EG_REQUIRE_EQ(cells.size(), 3U);
    EG_REQUIRE(cells[1].is_missing());
    EG_REQUIRE_EQ(cells[1].type(), LogicalType::float64);
    EG_REQUIRE(!cells[1].float64_value().has_value());
    require_cells(cells, {float64_cell(1.5), Cell::missing(LogicalType::float64),
                          float64_cell(3.5)});
}

// A fraction does not fit an integer column, and the refusal names the loss rather than the type.
EG_TEST("TYP", "019", a_fraction_into_an_integer_column_is_refused_as_a_precision_loss) {
    EditFixture fixture;
    SimpleTable table = make_table(24);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> before = column_cells(fixture, table.columns[1]);

    const auto write = set_cell(fixture, table, 1, 22, data::encode_value(data::float_form(1.5)));
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::precision_loss, ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[1]), before);

    // The same column does accept an integer written as a decimal, because nothing is lost.
    (void)require_set(fixture, table, 1, 22, data::encode_value(data::float_form(11)));
    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(7), int_cell(11), int_cell(9)});
}

// An out-of-range value is refused naming the declared width, so a byte column never silently
// wraps into the value the user typed.
EG_TEST("TYP", "019", a_value_outside_the_declared_width_is_refused_naming_the_type) {
    EditFixture fixture;
    SimpleTable table = make_table(25);
    build_cell_sheet(fixture, table);

    const auto write = set_cell(fixture, table, 2, 21, data::encode_value(data::signed_form(300)));
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::number_overflow, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[2]),
                  {int_cell(1, LogicalType::int8), int_cell(2, LogicalType::int8),
                   int_cell(3, LogicalType::int8)});
}

// An empty decimal payload and one below the representable range are refusals, never numbers. The
// C parser reads an empty string as a zero and reports a magnitude below the type as an underflow
// that also lands on zero, so both would otherwise store a value the caller never wrote.
EG_TEST("TYP", "019", an_empty_or_underflowing_decimal_payload_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(70);
    build_cell_sheet(fixture, table);

    const auto empty = set_cell(fixture, table, 0, 21, data::encode_value(data::ValueForm{'f', {}}));
    EG_REQUIRE_REJECTED(empty.outcome, DetailCode::number_ambiguous, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(empty.outcome);

    const auto underflow =
        set_cell(fixture, table, 0, 21,
                 data::encode_value(data::ValueForm{'f', std::string{"1e-400"}}));
    EG_REQUIRE_REJECTED(underflow.outcome, DetailCode::number_overflow, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(underflow.outcome);

    // Neither refusal left a zero in the column the edits were aimed at.
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)});
}

// The largest value each integer form can name is one the floating widths do not hold: the
// nearest double to INT64_MAX is 2^63 and the nearest to UINT64_MAX is 2^64. Exactness cannot be
// read off a cast back through the integer, because converting a double outside the target's
// range saturates to the boundary on this host instead of disagreeing, so the range is what is
// measured. The same integers still fill an integer field, which is what makes the refusal a
// statement about precision rather than about the boundary values.
EG_TEST("TYP", "019",
        an_integer_the_float_width_cannot_hold_is_refused_as_a_precision_loss) {
    EditFixture fixture;
    SimpleTable table = make_table(37);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> height_before = column_cells(fixture, table.columns[0]);

    const auto signed_top = set_cell(
        fixture, table, 0, 21,
        data::encode_value(data::signed_form(std::numeric_limits<std::int64_t>::max())));
    EG_REQUIRE_REJECTED(signed_top.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(signed_top.outcome);

    const auto unsigned_top = set_cell(
        fixture, table, 0, 21,
        data::encode_value(data::unsigned_form(std::numeric_limits<std::uint64_t>::max())));
    EG_REQUIRE_REJECTED(unsigned_top.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(unsigned_top.outcome);

    const auto into_int64 =
        require_set(fixture, table, 1, 21,
                    data::encode_value(
                        data::signed_form(std::numeric_limits<std::int64_t>::max())));
    EG_REQUIRE(into_int64.outcome.changed);
    adopt_write(table, into_int64);

    require_cells(column_cells(fixture, table.columns[0]), height_before);
    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(std::numeric_limits<std::int64_t>::max()), int_cell(8), int_cell(9)});
}

// A decimal payload is a double before it reaches the field, so 2^63 is one past the largest
// int64 rather than the largest int64 spelled differently: the bound is strict on the upper side
// and inclusive on the lower, because -2^63 is exactly representable while 2^63 is not.
EG_TEST("TYP", "019", a_decimal_past_the_signed_integer_bound_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(38);
    build_cell_sheet(fixture, table);

    const auto past_top = set_cell(
        fixture, table, 1, 21,
        data::encode_value(data::ValueForm{'f', std::string{"9223372036854775808"}}));
    EG_REQUIRE_REJECTED(past_top.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(past_top.outcome);

    // The spelling of INT64_MAX rounds to 2^63 as a double, so it is refused too rather than
    // landing as the largest int64 its own text did not name.
    const auto rounds_past_top = set_cell(
        fixture, table, 1, 21,
        data::encode_value(data::ValueForm{'f', std::string{"9223372036854775807"}}));
    EG_REQUIRE_REJECTED(rounds_past_top.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(rounds_past_top.outcome);

    const auto bottom = require_set(
        fixture, table, 1, 22,
        data::encode_value(data::ValueForm{'f', std::string{"-9223372036854775808"}}));
    EG_REQUIRE(bottom.outcome.changed);
    adopt_write(table, bottom);

    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(7), int_cell(std::numeric_limits<std::int64_t>::min()), int_cell(9)});
}

// A category label resolves through the dictionary the field declares, and a label it does not
// have is refused rather than appended to it.
EG_TEST("TYP", "019",
        a_category_label_the_dictionary_does_not_declare_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(26);
    build_cell_sheet(fixture, table);

    const auto unknown = set_cell(fixture, table, 3, 21, data::encode_value(data::category_form("mid")));
    EG_REQUIRE_REJECTED(unknown.outcome, DetailCode::unknown_category,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(unknown.outcome);

    const auto accepted =
        require_set(fixture, table, 3, 21, data::encode_value(data::category_form("high")));
    EG_REQUIRE(accepted.outcome.changed);
    require_cells(column_cells(fixture, table.columns[3]),
                  {category_cell(1), category_cell(1), category_cell(0)});
}

// A cell edit that states the value the column already holds changes nothing at all, which is what
// makes a repeated request safe rather than a second revision.
EG_TEST("TYP", "019", a_cell_edit_that_states_the_value_the_column_holds_publishes_nothing) {
    EditFixture fixture;
    SimpleTable table = make_table(27);
    build_cell_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();
    const std::uint64_t undo_depth = fixture.undo_depth();

    const auto write = require_set(fixture, table, 0, 21, data::encode_value(data::float_form(1.5)));

    EG_REQUIRE(!write.outcome.changed);
    EG_REQUIRE(write.outcome.events.empty());
    EG_REQUIRE(write.outcome.revisions.empty());
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
}

// A reading taken in one unit lands in the unit the column declares, so the stored number and its
// unit always belong to each other.
EG_TEST("UNIT", "014", a_value_in_a_named_unit_lands_in_the_unit_the_column_declares) {
    EditFixture fixture;
    SimpleTable table = make_table(28);
    build_cell_sheet(fixture, table);

    // The row holds metres; the reading was taken in centimetres. What lands is the metre value.
    const auto write = require_set(fixture, table, 0, 21, data::encode_value(data::float_form(400)),
                                   "cm");
    EG_REQUIRE(write.outcome.changed);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(4), float64_cell(2.5), float64_cell(3.5)});

    // The declared unit is unchanged by a value that arrived in another one.
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE(bindings[0].unit.has_value());
    EG_REQUIRE_EQ(bindings[0].unit->canonical_text(), std::string{"m"});
}

// A unit of another dimension cannot be read as this column's quantity, so it is refused instead
// of being stored as a bare number with the old unit still attached.
EG_TEST("UNIT", "014", a_value_in_an_inconvertible_unit_is_refused_rather_than_reinterpreted) {
    EditFixture fixture;
    SimpleTable table = make_table(29);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> before = column_cells(fixture, table.columns[0]);

    const auto write = set_cell(fixture, table, 0, 21, data::encode_value(data::float_form(30)), "s");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::incompatible_unit,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[0]), before);
}

// A conversion scales the number before it lands, so a reading can leave the range of the type.
// The caller stated a finite number, so the infinity that would stand in for it is refused rather
// than published, while a reading inside the range still lands in the unit the column declares.
EG_TEST("UNIT", "014", a_unit_conversion_past_the_top_of_double_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(56);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> before = column_cells(fixture, table.columns[0]);

    const auto past =
        set_cell(fixture, table, 0, 21, data::encode_value(data::float_form(1e306)), "km");
    EG_REQUIRE_REJECTED(past.outcome, DetailCode::number_overflow, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(past.outcome);
    require_cells(column_cells(fixture, table.columns[0]), before);

    const auto inside = require_set(fixture, table, 0, 21,
                                    data::encode_value(data::float_form(4)), "km");
    EG_REQUIRE(inside.outcome.changed);
    adopt_write(table, inside);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(4000), float64_cell(2.5), float64_cell(3.5)});
}

// A column that declares no unit has nothing to convert into, so a unit on the value is a
// mismatch the command reports instead of dropping the unit.
EG_TEST("UNIT", "014", a_unit_on_a_value_for_a_unitless_column_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(30);
    build_cell_sheet(fixture, table);

    const auto write = set_cell(fixture, table, 1, 22, data::encode_value(data::float_form(5)), "m");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::unknown_unit, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[1]), {int_cell(7), int_cell(8), int_cell(9)});
}

// The unit, role, format, label and binding identity belong to the column, so editing one of its
// cells leaves all of them alone.
EG_TEST("UNIT", "014", a_cell_edit_leaves_the_column_unit_role_format_and_label_alone) {
    EditFixture fixture;
    SimpleTable table = make_table(31);
    build_cell_sheet(fixture, table);
    const std::uint64_t binding = table.columns[0].column;

    (void)require_set(fixture, table, 0, 23, data::encode_value(data::float_form(4)));

    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings.size(), 4U);
    EG_REQUIRE_EQ(bindings[0].id.raw(), binding);
    EG_REQUIRE_EQ(bindings[0].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[0].role, data::ColumnRole::y);
    EG_REQUIRE_EQ(bindings[0].display_format, std::string{"0.00"});
    EG_REQUIRE(bindings[0].unit.has_value());
    EG_REQUIRE_EQ(bindings[0].unit->canonical_text(), std::string{"m"});
    EG_REQUIRE_EQ(bindings[0].order_index, 0U);
    EG_REQUIRE_EQ(bindings[1].label, std::string{"count"});
    EG_REQUIRE(!bindings[1].unit.has_value());
}

// An edit carries the revisions it was written against. A newer edit already moved the column, so
// the stale one is refused by the kernel and the newer value stands untouched.
EG_TEST("ID", "008", a_stale_expected_revision_is_refused_and_the_newer_value_stands) {
    EditFixture fixture;
    SimpleTable table = make_table(32);
    build_cell_sheet(fixture, table);
    const std::uint64_t column = table.columns[0].column;
    const everygraph::foundation::Revision stale = fixture.revision_of(column);

    (void)require_set(fixture, table, 0, 21, data::encode_value(data::float_form(9)));
    EG_REQUIRE(fixture.revision_of(column) != stale);

    // The same request, re-issued against the revision the column held before that edit.
    ParameterMap arguments = EditFixture::with(data::argument::kColumn, column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{22});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(8)));
    const SuccessorNames names = name_successors(table, arguments, {table.columns[0]});
    ExpectedRevisions expected;
    expected.writes[ObjectId{names.versions[0]}] = everygraph::foundation::kInitialRevision;
    expected.writes[ObjectId{column}] = stale;
    const std::uint64_t undo_depth = fixture.undo_depth();
    const std::uint64_t events = fixture.events().size();
    const std::uint64_t last_transaction = fixture.last_action()->transaction_id;
    const auto outcome =
        fixture.run(data::command_id::kWorksheetCellSet, std::move(arguments), std::move(expected));

    EG_REQUIRE(!outcome.succeeded());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::revision_conflict);
    EG_REQUIRE(outcome.error->message.find("expected write revision") != std::string::npos);
    EG_REQUIRE_NO_WRITE(outcome);
    EG_REQUIRE(fixture.object(names.versions[0]) == std::nullopt);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth);
    EG_REQUIRE_EQ(fixture.events().size(), events);
    EG_REQUIRE_EQ(fixture.last_action()->transaction_id, last_transaction);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(9), float64_cell(2.5), float64_cell(3.5)});
}

// A block paste is anchored at a row identity and a column, and rewrites exactly the columns it
// covers: its neighbours keep their version identities.
EG_TEST("TYP", "019", a_block_paste_writes_from_its_anchor_row_and_column) {
    EditFixture fixture;
    SimpleTable table = make_table(33);
    build_cell_sheet(fixture, table);
    const std::uint64_t height_version = table.columns[0].version;
    const std::uint64_t kind_version = table.columns[3].version;

    const auto write = require_paste(fixture, table, 1, 22,
                                     data::encode_grid(2, 2, {data::signed_form(10),
                                                              data::signed_form(4),
                                                              data::signed_form(11),
                                                              data::signed_form(5)}),
                                     2);

    EG_REQUIRE(write.outcome.changed);
    EG_REQUIRE_EQ(write.names.versions.size(), 2U);
    require_cells(column_cells(fixture, table.columns[1]),
                  {int_cell(7), int_cell(10), int_cell(11)});
    require_cells(column_cells(fixture, table.columns[2]),
                  {int_cell(1, LogicalType::int8), int_cell(4, LogicalType::int8),
                   int_cell(5, LogicalType::int8)});
    EG_REQUIRE_EQ(table.columns[0].version, height_version);
    EG_REQUIRE_EQ(table.columns[3].version, kind_version);
}

// A block that would fall off the bottom or the right of the sheet is refused: a paste never
// grows the sheet and never drops the cells that had nowhere to go.
EG_TEST("TYP", "019", a_block_reaching_past_the_sheets_bounds_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(34);
    build_cell_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto past_rows = paste_block(
        fixture, table, 1, 22,
        data::encode_grid(3, 1, {data::signed_form(1), data::signed_form(2), data::signed_form(3)}),
        1);
    EG_REQUIRE_REJECTED(past_rows.outcome, DetailCode::index_out_of_range,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(past_rows.outcome);

    const auto past_columns = paste_block(
        fixture, table, 3, 21, data::encode_grid(1, 2, {data::category_form("low"),
                                                         data::category_form("high")}),
        2);
    EG_REQUIRE_REJECTED(past_columns.outcome, DetailCode::index_out_of_range,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(past_columns.outcome);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    require_cells(column_cells(fixture, table.columns[1]), {int_cell(7), int_cell(8), int_cell(9)});
}

// Every cell of a block is resolved before anything is published, so one value a column cannot
// hold refuses the whole block rather than writing the cells around it.
EG_TEST("TYP", "019", a_block_with_one_value_a_column_cannot_hold_publishes_nothing) {
    EditFixture fixture;
    SimpleTable table = make_table(35);
    build_cell_sheet(fixture, table);
    const std::vector<Cell> count_before = column_cells(fixture, table.columns[1]);
    const std::vector<Cell> narrow_before = column_cells(fixture, table.columns[2]);

    const auto write = paste_block(
        fixture, table, 1, 21,
        data::encode_grid(1, 2, {data::signed_form(5), data::text_form("n/a")}), 2);
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
    EG_REQUIRE(fixture.object(write.names.versions[1]) == std::nullopt);
    require_cells(column_cells(fixture, table.columns[1]), count_before);
    require_cells(column_cells(fixture, table.columns[2]), narrow_before);
}

// A block whose values are the ones the sheet already holds states no difference, so it publishes
// no successor at all.
EG_TEST("TYP", "019", a_block_paste_that_changes_no_value_publishes_nothing) {
    EditFixture fixture;
    SimpleTable table = make_table(36);
    build_cell_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto write = paste_block(
        fixture, table, 1, 21,
        data::encode_grid(3, 2, {data::signed_form(7), data::signed_form(1), data::signed_form(8),
                                 data::signed_form(2), data::signed_form(9),
                                 data::signed_form(3)}),
        2);
    EG_REQUIRE(write.outcome.succeeded());
    EG_REQUIRE(!write.outcome.changed);
    EG_REQUIRE(write.outcome.events.empty());
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
}
