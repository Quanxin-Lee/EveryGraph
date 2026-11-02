#include <bit>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "edit_fixture.hpp"

namespace {

using namespace egtest;

// A measured column, a unitless integer column, a column of a value float32 cannot hold exactly
// and a categorical column: the four routes a declared type change has to preview.
void build_type_sheet(EditFixture& fixture, SimpleTable& table) {
    create_chain(fixture, table);
    seed_rows(fixture, table, rows_of({31, 32, 33}));

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

    ColumnSpec wide;
    wide.type = LogicalType::float64;
    wide.label = "fraction";
    wide.cells = {float64_cell(0.1), float64_cell(0.2), float64_cell(0.3)};
    (void)add_column(fixture, table, wide);

    ColumnSpec kind;
    kind.type = LogicalType::categorical;
    kind.label = "kind";
    kind.dictionary = dictionary_of({"low", "high"});
    kind.cells = {category_cell(0), category_cell(1), category_cell(0)};
    (void)add_column(fixture, table, kind);
}

// The declared change of one column: the same arguments the preview reads, plus the identities
// the successor publishes under.
[[nodiscard]] ParameterMap type_arguments(const ColumnLine& line, const std::string& target,
                                          const std::string& policy, const std::string& unit,
                                          const std::string& labels, const std::string& zone) {
    ParameterMap arguments = EditFixture::with(data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kTargetType, target);
    if (!policy.empty()) {
        EditFixture::put(arguments, data::argument::kConversion, policy);
    }
    if (!unit.empty()) {
        EditFixture::put(arguments, data::argument::kUnit, unit);
    }
    if (!labels.empty()) {
        EditFixture::put(arguments, data::argument::kDictionary, labels);
    }
    if (!zone.empty()) {
        EditFixture::put(arguments, data::argument::kZone, zone);
    }
    return arguments;
}

[[nodiscard]] CommandOutcome preview_type(EditFixture& fixture, const SimpleTable& table,
                                          std::size_t position, const std::string& target,
                                          const std::string& policy = {},
                                          const std::string& labels = {}) {
    return fixture.run(data::command_id::kWorksheetColumnTypePreview,
                       type_arguments(table.columns[position], target, policy, {}, labels, {}));
}

struct TypeWrite {
    CommandOutcome outcome;
    SuccessorNames names;
};

[[nodiscard]] TypeWrite set_type(EditFixture& fixture, SimpleTable& table, std::size_t position,
                                 const std::string& target, const std::string& policy = {},
                                 const std::string& unit = {}, const std::string& labels = {},
                                 const std::string& zone = {}) {
    const ColumnLine line = table.columns[position];
    ParameterMap arguments = type_arguments(line, target, policy, unit, labels, zone);
    const SuccessorNames names = name_successors(table, arguments, {line});
    auto outcome = fixture.run(data::command_id::kWorksheetColumnTypeSet, std::move(arguments),
                               fixture.declared_writes(names.writes));
    return TypeWrite{std::move(outcome), names};
}

}  // namespace

// The preview is the same resolution with no write at all, so the change a table view warns about
// is refused for exactly the reason the preview named.
EG_TEST("TYP", "020", a_type_preview_and_the_refused_change_report_the_same_loss) {
    EditFixture fixture;
    SimpleTable table = make_table(40);
    build_type_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto seen = preview_type(fixture, table, 0, "int64");
    EG_REQUIRE_REJECTED(seen, DetailCode::precision_loss, ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(seen);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);

    const auto write = set_type(fixture, table, 0, "int64");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::precision_loss, ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(write.outcome);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
    require_cells(column_cells(fixture, table.columns[0]),
                  {float64_cell(1.5), float64_cell(2.5), float64_cell(3.5)});
}

// A change the preview accepts still writes nothing until the caller asks for it, so a view can
// show what would happen without the user having committed to it.
EG_TEST("TYP", "020", a_type_preview_that_succeeds_still_changes_nothing) {
    EditFixture fixture;
    SimpleTable table = make_table(41);
    build_type_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto seen = preview_type(fixture, table, 1, "text");
    EG_REQUIRE(seen.succeeded());
    EG_REQUIRE(!seen.changed);
    EG_REQUIRE(seen.events.empty());
    EG_REQUIRE(seen.revisions.empty());
    EG_REQUIRE_EQ(fixture.version_count(), version_count);

    const auto write = set_type(fixture, table, 1, "text");
    EG_REQUIRE(write.outcome.succeeded());
    EG_REQUIRE(write.outcome.changed);
    adopt(table, write.names, table.columns[1]);
    require_cells(column_cells(fixture, table.columns[1]),
                  {text_cell("7"), text_cell("8"), text_cell("9")});
}

// A width the target cannot hold exactly is refused until the caller names the permission, and
// the accepted narrowing is the nearest representable value of the declared width.
EG_TEST("TYP", "020", a_narrowing_change_needs_the_callers_declared_policy) {
    EditFixture fixture;
    SimpleTable table = make_table(42);
    build_type_sheet(fixture, table);

    const auto refused = set_type(fixture, table, 2, "float32");
    EG_REQUIRE_REJECTED(refused.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(refused.outcome);

    const auto allowed = set_type(fixture, table, 2, "float32", "round");
    EG_REQUIRE(allowed.outcome.succeeded());
    EG_REQUIRE(allowed.outcome.changed);
    adopt(table, allowed.names, table.columns[2]);

    const auto cells = column_cells(fixture, table.columns[2]);
    require_cells(cells, {float32_cell(0.1F), float32_cell(0.2F), float32_cell(0.3F)});
    EG_REQUIRE_EQ(cells.front().type(), LogicalType::float32);
    const auto version = fixture.version(data::dataset_version_id(table.columns[2].version));
    EG_REQUIRE_EQ(version->schema()->fields.front().type, LogicalType::float32);
    // The field keeps its identity and its name: a type change restates values, not columns.
    EG_REQUIRE_EQ(version->schema()->fields.front().id.raw(), table.columns[2].field);
    EG_REQUIRE_EQ(version->schema()->fields.front().name, std::string{"fraction"});
}

// A tick field stores its own unit, so a numeric column never becomes a temporal one by a
// declared type change.
EG_TEST("TYP", "020", a_numeric_column_never_becomes_a_temporal_one_by_a_type_change) {
    EditFixture fixture;
    SimpleTable table = make_table(43);
    build_type_sheet(fixture, table);
    const std::vector<Cell> before = column_cells(fixture, table.columns[1]);

    EG_REQUIRE_REJECTED(preview_type(fixture, table, 1, "date"), DetailCode::type_mismatch,
                        ErrorCode::invalid_argument);
    const auto write = set_type(fixture, table, 1, "date");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::type_mismatch, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(write.outcome);
    require_cells(column_cells(fixture, table.columns[1]), before);
}

// A measured column cannot become a text column, because the text would carry no unit; and a
// type change never rescales, so it cannot declare another unit either.
EG_TEST("TYP", "020", a_type_change_never_drops_or_rekeys_the_unit_a_column_declares) {
    EditFixture fixture;
    SimpleTable table = make_table(44);
    build_type_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();

    const auto to_text = set_type(fixture, table, 0, "text");
    EG_REQUIRE_REJECTED(to_text.outcome, DetailCode::incompatible_unit,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(to_text.outcome);

    const auto rescaled = set_type(fixture, table, 0, "int64", "reject", "cm");
    EG_REQUIRE_REJECTED(rescaled.outcome, DetailCode::incompatible_unit,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rescaled.outcome);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);

    // The same column with the same unit declared is a legal change, and the unit survives it.
    const auto kept = set_type(fixture, table, 0, "float32", "round", "m");
    EG_REQUIRE(kept.outcome.succeeded());
    adopt(table, kept.names, table.columns[0]);
    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE(bindings[0].unit.has_value());
    EG_REQUIRE_EQ(bindings[0].unit->canonical_text(), std::string{"m"});
    EG_REQUIRE_EQ(bindings[0].display_format, std::string{"0.00"});
}

// A categorical change resolves every stored code through its label, so a dictionary that loses a
// label the column holds is refused with the code that names the loss, and a reordering of the
// same labels moves each value to its label's new slot.
EG_TEST("TYP", "020", a_relabelled_categorical_column_moves_each_value_through_its_label) {
    EditFixture fixture;
    SimpleTable table = make_table(45);
    build_type_sheet(fixture, table);
    const std::uint64_t version_count = fixture.version_count();
    const auto dropped = data::encode_grid(1, 1, {data::text_form("high")});

    EG_REQUIRE_REJECTED(preview_type(fixture, table, 3, "categorical", {}, dropped),
                        DetailCode::unknown_category, ErrorCode::invalid_argument);
    const auto refused = set_type(fixture, table, 3, "categorical", {}, {}, dropped);
    EG_REQUIRE_REJECTED(refused.outcome, DetailCode::unknown_category,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(refused.outcome);
    require_cells(column_cells(fixture, table.columns[3]),
                  {category_cell(0), category_cell(1), category_cell(0)});

    // A dictionary is declared only by a categorical field, so a numeric target naming one is a
    // declaration error before any value is read.
    const auto misplaced = set_type(
        fixture, table, 1, "int32", {}, {},
        data::encode_grid(1, 2, {data::text_form("low"), data::text_form("high")}));
    EG_REQUIRE_REJECTED(misplaced.outcome, DetailCode::type_mismatch,
                        ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);

    const auto relabelled = set_type(
        fixture, table, 3, "categorical", {}, {},
        data::encode_grid(1, 2, {data::text_form("high"), data::text_form("low")}));
    EG_REQUIRE(relabelled.outcome.succeeded());
    adopt(table, relabelled.names, table.columns[3]);
    // "low" was code 0 and is now code 1, so every value moved to its label's new slot.
    require_cells(column_cells(fixture, table.columns[3]),
                  {category_cell(1), category_cell(0), category_cell(1)});
    const auto version = fixture.version(data::dataset_version_id(table.columns[3].version));
    EG_REQUIRE(version->schema()->fields.front().dictionary != nullptr);
    EG_REQUIRE_EQ(version->schema()->fields.front().dictionary->labels.size(), 2U);
    EG_REQUIRE_EQ(version->schema()->fields.front().dictionary->labels[0], std::string{"high"});
}

// The largest int64 is a value float64 does not hold: the nearest double is 2^63, one past every
// integer the source field can name. A change to that width refuses the value rather than
// publishing a successor whose science differs from the version it replaced, and the largest
// integer a double does step through changes by the same route.
EG_TEST("TYP", "020",
        a_type_change_to_a_float_that_cannot_hold_the_largest_integer_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(52);
    build_type_sheet(fixture, table);

    constexpr std::int64_t kInt64Top = std::numeric_limits<std::int64_t>::max();
    ColumnSpec top;
    top.type = LogicalType::int64;
    top.label = "top";
    top.cells = {int_cell(kInt64Top), int_cell(7), int_cell(8)};
    (void)add_column(fixture, table, top);

    ColumnSpec held;
    held.type = LogicalType::int64;
    held.label = "held";
    held.cells = {int_cell(9007199254740992), int_cell(7), int_cell(8)};
    (void)add_column(fixture, table, held);
    const std::uint64_t version_count = fixture.version_count();

    const auto seen = preview_type(fixture, table, 4, "float64");
    EG_REQUIRE_REJECTED(seen, DetailCode::precision_loss, ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(seen);

    const auto write = set_type(fixture, table, 4, "float64");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(write.outcome);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
    require_cells(column_cells(fixture, table.columns[4]),
                  {int_cell(kInt64Top), int_cell(7), int_cell(8)});

    const auto accepted = set_type(fixture, table, 5, "float64");
    EG_REQUIRE(accepted.outcome.succeeded());
    EG_REQUIRE(accepted.outcome.changed);
    adopt(table, accepted.names, table.columns[5]);
    require_cells(column_cells(fixture, table.columns[5]),
                  {float64_cell(9007199254740992.0), float64_cell(7.0), float64_cell(8.0)});
    EG_REQUIRE_EQ(fixture.version_count(), version_count + 1);
}

// A non-finite state has a spelling, and `float_text` writes it, so a type change reads that
// spelling back into the same state class rather than a value: NaN never becomes a zero and an
// infinity never becomes the largest finite number. The same spelling aimed at a field that holds
// only finite values is refused, because §3.4 keeps missing, error, NaN and infinite states apart.
EG_TEST("TYP", "020",
        a_non_finite_state_round_trips_through_text_and_refuses_an_integer_field) {
    EditFixture fixture;
    SimpleTable table = make_table(53);
    build_type_sheet(fixture, table);

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    ColumnSpec state;
    state.type = LogicalType::float64;
    state.label = "state";
    state.cells = {float64_cell(nan), float64_cell(inf), float64_cell(-inf)};
    (void)add_column(fixture, table, state);

    const auto as_text = set_type(fixture, table, 4, "text");
    EG_REQUIRE(as_text.outcome.succeeded());
    EG_REQUIRE(as_text.outcome.changed);
    adopt(table, as_text.names, table.columns[4]);
    require_cells(column_cells(fixture, table.columns[4]),
                  {text_cell("nan"), text_cell("inf"), text_cell("-inf")});

    const auto into_int = set_type(fixture, table, 4, "int64");
    EG_REQUIRE_REJECTED(into_int.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(into_int.outcome);

    const auto back = set_type(fixture, table, 4, "float64");
    EG_REQUIRE(back.outcome.succeeded());
    EG_REQUIRE(back.outcome.changed);
    adopt(table, back.names, table.columns[4]);
    require_cells(column_cells(fixture, table.columns[4]),
                  {float64_cell(nan), float64_cell(inf), float64_cell(-inf)});
}

// `round` is the caller's permission to drop a fraction, not to invent an integer for a state that
// has none. An infinite or NaN reading aimed at an integer or tick field therefore stays a refusal
// with the policy named: the rounding route has no integer to hand over, and a saturated boundary
// would be a value the caller never stated.
EG_TEST("TYP", "020",
        a_declared_round_policy_refuses_a_non_finite_state_in_an_integer_field) {
    EditFixture fixture;
    SimpleTable table = make_table(54);
    build_type_sheet(fixture, table);

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    ColumnSpec state;
    state.type = LogicalType::float64;
    state.label = "state";
    state.cells = {float64_cell(nan), float64_cell(inf), float64_cell(-inf)};
    (void)add_column(fixture, table, state);
    const std::uint64_t version_count = fixture.version_count();

    const auto seen = preview_type(fixture, table, 4, "int64", "round");
    EG_REQUIRE_REJECTED(seen, DetailCode::precision_loss, ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(seen);

    const auto write = set_type(fixture, table, 4, "int64", "round");
    EG_REQUIRE_REJECTED(write.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(write.outcome);
    EG_REQUIRE(fixture.object(write.names.versions[0]) == std::nullopt);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    require_cells(column_cells(fixture, table.columns[4]),
                  {float64_cell(nan), float64_cell(inf), float64_cell(-inf)});

    // An unsigned field counts the same way a signed one does, so the reading is refused there too
    // instead of landing as the width's own boundary.
    const auto unsigned_field = set_type(fixture, table, 4, "uint64", "round");
    EG_REQUIRE_REJECTED(unsigned_field.outcome, DetailCode::precision_loss,
                        ErrorCode::numerical_failure);
    EG_REQUIRE_NO_WRITE(unsigned_field.outcome);

    // The route still rounds what it is given: half away from zero, which is the conversion the
    // caller named and no other column of the sheet touches.
    const auto rounded = set_type(fixture, table, 0, "int64", "round", "m");
    EG_REQUIRE(rounded.outcome.succeeded());
    EG_REQUIRE(rounded.outcome.changed);
    adopt(table, rounded.names, table.columns[0]);
    require_cells(column_cells(fixture, table.columns[0]),
                  {int_cell(2), int_cell(3), int_cell(4)});
}

// One state class leaves as one spelling, whatever bits installed it: a payload that arrived
// through the bit form still writes text the same family reads back, so a type change to text is
// never a one-way door for the non-finite states §3.4 keeps apart.
EG_TEST("TYP", "020", a_non_finite_payload_written_through_bits_reads_back_through_text) {
    EditFixture fixture;
    SimpleTable table = make_table(55);
    build_type_sheet(fixture, table);

    ColumnSpec state;
    state.type = LogicalType::float64;
    state.label = "state";
    state.cells = {float64_cell(std::bit_cast<double>(0xfff8000000000000ULL)),
                   float64_cell(std::bit_cast<double>(0x7ff4000000000000ULL)),
                   float64_cell(std::bit_cast<double>(0xfff0000000000000ULL))};
    (void)add_column(fixture, table, state);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    const auto as_text = set_type(fixture, table, 4, "text");
    EG_REQUIRE(as_text.outcome.succeeded());
    EG_REQUIRE(as_text.outcome.changed);
    adopt(table, as_text.names, table.columns[4]);
    require_cells(column_cells(fixture, table.columns[4]),
                  {text_cell("nan"), text_cell("nan"), text_cell("-inf")});

    const auto back = set_type(fixture, table, 4, "float64");
    EG_REQUIRE(back.outcome.succeeded());
    EG_REQUIRE(back.outcome.changed);
    adopt(table, back.names, table.columns[4]);
    require_cells(column_cells(fixture, table.columns[4]),
                  {float64_cell(nan), float64_cell(nan), float64_cell(-inf)});
}

// A mirror is an alias of a version another binding owns, so every editing surface refuses to
// rewrite it rather than changing a column the caller never named.
EG_TEST("ALIGN", "020", a_cell_edit_of_a_mirror_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(46);
    build_type_sheet(fixture, table);
    const ColumnLine mirror = add_mirror(fixture, table, table.columns[0]);
    const std::vector<Cell> before = column_cells(fixture, table.columns[0]);

    const ColumnLine line = mirror;
    ParameterMap arguments = EditFixture::with(data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{31});
    EditFixture::put(arguments, data::argument::kValue,
                     data::encode_value(data::float_form(9)));
    const SuccessorNames names = name_successors(table, arguments, {line});
    const auto outcome = fixture.run(data::command_id::kWorksheetCellSet, std::move(arguments),
                                     fixture.declared_writes(names.writes));

    EG_REQUIRE_REJECTED(outcome, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
    EG_REQUIRE_NO_WRITE(outcome);
    require_cells(column_cells(fixture, table.columns[0]), before);
}

// A block paste reads the columns it covers from the worksheet, so one mirror in the sheet is
// enough to make the whole declared block unsafe to write.
EG_TEST("ALIGN", "020", a_block_paste_covering_a_mirror_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(47);
    build_type_sheet(fixture, table);
    (void)add_mirror(fixture, table, table.columns[0]);

    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kColumn, table.columns[0].column);
    EditFixture::put(arguments, data::argument::kRow, std::uint64_t{31});
    EditFixture::put(arguments, data::argument::kValues,
                     data::encode_grid(1, 1, {data::float_form(4)}));
    const std::vector<ColumnLine> first{table.columns[0]};
    const SuccessorNames names = name_successors(table, arguments, first);
    const auto outcome =
        fixture.run(data::command_id::kWorksheetCellPaste, std::move(arguments),
                    fixture.declared_writes(names.writes));

    EG_REQUIRE_REJECTED(outcome, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
    EG_REQUIRE_NO_WRITE(outcome);
}

// A type change publishes a successor version, which a mirror does not own to give away.
EG_TEST("ALIGN", "020", a_type_change_of_a_mirror_is_refused) {
    EditFixture fixture;
    SimpleTable table = make_table(48);
    build_type_sheet(fixture, table);
    const ColumnLine mirror = add_mirror(fixture, table, table.columns[1]);

    ParameterMap arguments = type_arguments(mirror, "text", {}, {}, {}, {});
    const SuccessorNames names = name_successors(table, arguments, {mirror});
    const auto write = fixture.run(data::command_id::kWorksheetColumnTypeSet, std::move(arguments),
                                   fixture.declared_writes(names.writes));
    EG_REQUIRE_REJECTED(write, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
    EG_REQUIRE_NO_WRITE(write);

    // The preview reads the same column and refuses it the same way, without writing.
    const auto seen = fixture.run(data::command_id::kWorksheetColumnTypePreview,
                                  type_arguments(mirror, "text", {}, {}, {}, {}));
    EG_REQUIRE_REJECTED(seen, DetailCode::mirror_read_only, ErrorCode::unsupported_feature);
}

// Inserting a column at a position renumbers the columns after it in the same revision, so no
// reader ever sees two columns claiming one slot.
EG_TEST("ID", "005", a_column_insert_at_a_position_renumbers_the_columns_after_it) {
    EditFixture fixture;
    SimpleTable table = make_table(49);
    build_type_sheet(fixture, table);

    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.label = "width";
    spec.unit = "m";
    spec.cells = {float64_cell(1), float64_cell(2), float64_cell(3)};
    const ColumnLine inserted = insert_column(fixture, table, 1, spec);

    const auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings.size(), 5U);
    EG_REQUIRE_EQ(bindings[0].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[1].id.raw(), inserted.column);
    EG_REQUIRE_EQ(bindings[1].label, std::string{"width"});
    EG_REQUIRE_EQ(bindings[1].order_index, 1U);
    EG_REQUIRE_EQ(bindings[2].id.raw(), table.columns[2].column);
    EG_REQUIRE_EQ(bindings[2].order_index, 2U);
    EG_REQUIRE_EQ(bindings[4].order_index, 4U);
    // The inserted column reads as any other: its own version holds its own three values.
    require_cells(column_cells(fixture, inserted),
                  {float64_cell(1), float64_cell(2), float64_cell(3)});

    // Undo takes the binding away and puts every later column back in its previous slot. The
    // immutable version the store published stays readable, because nothing in this data model
    // rewrites a committed version.
    const std::uint64_t version_count = fixture.version_count();
    (void)fixture.require_undo();
    EG_REQUIRE(fixture.object(inserted.column) == std::nullopt);
    EG_REQUIRE(fixture.object(inserted.version) == std::nullopt);
    const auto restored = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(restored.size(), 4U);
    EG_REQUIRE_EQ(restored[0].label, std::string{"height"});
    EG_REQUIRE_EQ(restored[1].label, std::string{"count"});
    EG_REQUIRE_EQ(restored[1].order_index, 1U);
    EG_REQUIRE_EQ(fixture.version_count(), version_count);
    EG_REQUIRE(fixture.version(data::dataset_version_id(inserted.version)) != nullptr);

    (void)fixture.require_redo();
    const auto again = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(again.size(), 5U);
    EG_REQUIRE_EQ(again[1].id.raw(), inserted.column);
    EG_REQUIRE_EQ(again[2].order_index, 2U);
}

// A declared order is the whole sheet: a column left out or named twice is refused rather than
// quietly dropping or duplicating one.
EG_TEST("ID", "005", a_declared_column_order_names_every_column_once) {
    EditFixture fixture;
    SimpleTable table = make_table(50);
    build_type_sheet(fixture, table);
    const std::uint64_t undo_depth = fixture.undo_depth();

    ParameterMap dropped = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(dropped, data::argument::kColumns,
                     EditFixture::id_list({table.columns[1].column, table.columns[0].column}));
    const auto missing =
        fixture.run(data::command_id::kWorksheetColumnOrderSet, std::move(dropped),
                    fixture.declared_writes({table.columns[0].column, table.columns[1].column}));
    EG_REQUIRE_REJECTED(missing, DetailCode::unmapped_row, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(missing);

    ParameterMap twice = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(twice, data::argument::kColumns,
                     EditFixture::id_list({table.columns[0].column, table.columns[0].column,
                                           table.columns[1].column, table.columns[2].column}));
    const auto duplicated =
        fixture.run(data::command_id::kWorksheetColumnOrderSet, std::move(twice),
                    fixture.declared_writes({table.columns[0].column, table.columns[1].column,
                                             table.columns[2].column}));
    EG_REQUIRE_REJECTED(duplicated, DetailCode::duplicate_id, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(duplicated);
    EG_REQUIRE_EQ(fixture.undo_depth(), undo_depth);
}

// A reorder moves slots and renames nothing: each binding keeps its identity, its label and the
// version it points at, and undo puts the previous order back in one step.
EG_TEST("ID", "005", undo_restores_the_previous_column_order_and_every_binding_identity) {
    EditFixture fixture;
    SimpleTable table = make_table(51);
    build_type_sheet(fixture, table);
    const std::uint64_t height_version = table.columns[0].version;
    const std::uint64_t count_version = table.columns[1].version;
    const std::uint64_t fraction_version = table.columns[2].version;
    const std::uint64_t kind_version = table.columns[3].version;

    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kColumns,
                     EditFixture::id_list({table.columns[3].column, table.columns[0].column,
                                           table.columns[2].column, table.columns[1].column}));
    const std::vector<std::uint64_t> writes{table.columns[0].column, table.columns[1].column,
                                            table.columns[2].column, table.columns[3].column};
    const auto outcome = fixture.require_run(data::command_id::kWorksheetColumnOrderSet,
                                             std::move(arguments),
                                             fixture.declared_writes(writes));
    EG_REQUIRE(outcome.changed);

    // Each binding keeps its identity, its label and the version it points at; only the slot
    // moved.
    auto bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].label, std::string{"kind"});
    EG_REQUIRE_EQ(bindings[0].version.raw(), kind_version);
    EG_REQUIRE_EQ(bindings[1].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[1].version.raw(), height_version);
    EG_REQUIRE_EQ(bindings[2].label, std::string{"fraction"});
    EG_REQUIRE_EQ(bindings[2].version.raw(), fraction_version);
    EG_REQUIRE_EQ(bindings[3].label, std::string{"count"});
    EG_REQUIRE_EQ(bindings[3].version.raw(), count_version);

    (void)fixture.require_undo();
    bindings = sheet_bindings(fixture, table.worksheet);
    EG_REQUIRE_EQ(bindings[0].label, std::string{"height"});
    EG_REQUIRE_EQ(bindings[0].order_index, 0U);
    EG_REQUIRE_EQ(bindings[1].label, std::string{"count"});
    EG_REQUIRE_EQ(bindings[1].order_index, 1U);
    EG_REQUIRE_EQ(bindings[2].label, std::string{"fraction"});
    EG_REQUIRE_EQ(bindings[2].order_index, 2U);
    EG_REQUIRE_EQ(bindings[3].label, std::string{"kind"});
    EG_REQUIRE_EQ(bindings[3].order_index, 3U);
    EG_REQUIRE_EQ(fixture.redo_depth(), 1U);
}
