#include <bit>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "data_test_support.hpp"

using namespace egtest;

namespace {

[[nodiscard]] data::Quantity quantity(std::string text, double value) {
    data::Quantity item;
    item.value = value;
    item.unit = EG_REQUIRE_TAKEN(data::UnitRegistry::builtin().parse(text));
    return item;
}

// The plan's unit tolerance: exact when the factor is exactly representable, otherwise at
// most 1e-12 relative and 1e-12 absolute in the target unit.
void require_close(double actual, double expected, const char* what) {
    const double absolute = std::fabs(actual - expected);
    const double relative = expected == 0.0 ? absolute : absolute / std::fabs(expected);
    if (!(absolute <= 1e-12 || relative <= 1e-12)) {
        throw TestFailure(std::string{what} + ": expected " + std::to_string(expected) +
                          ", got " + std::to_string(actual) + " (absolute " +
                          std::to_string(absolute) + ", relative " + std::to_string(relative) + ")");
    }
}

}  // namespace

EG_TEST("UNIT", "001", an_identity_conversion_changes_no_value_and_no_stored_data) {
    const auto meters = data::UnitRegistry::builtin().parse("m");
    EG_REQUIRE_OK(meters);
    const auto same = data::UnitRegistry::builtin().parse("m");
    EG_REQUIRE_OK(same);

    const auto moved = data::convert(quantity("m", 5.25), EG_REQUIRE_OK(meters));
    EG_REQUIRE(moved.ok());
    // Identity conversion is bit-exact: no rounding and no second representation.
    EG_REQUIRE_EQ(moved.value().value, 5.25);
    EG_REQUIRE(moved.value().unit == EG_REQUIRE_OK(same));

    // Declaring a unit on a column is metadata; the stored values are never rewritten.
    Fixture fixture;
    auto table = make_table(21);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(21001), data::row_id(21002)});
    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.unit = "m";
    spec.cells = {float64_cell(5.25), float64_cell(-7.5)};
    ColumnLine line = create_column(fixture, table, spec);
    const auto before = fixture.version(data::dataset_version_id(line.version))->content_hash();
    const auto declared = data::convert(quantity("m", 5.25), EG_REQUIRE_OK(meters));
    EG_REQUIRE_OK(declared);
    auto lease = fixture.lease(data::dataset_version_id(line.version));
    EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(lease.cell_at(data::field_id(line.field), 0)),
                       float64_cell(5.25));
    EG_REQUIRE_EQ(fixture.version(data::dataset_version_id(line.version))->content_hash(), before);
    EG_REQUIRE_EQ(fixture.version_count(), std::uint64_t{1});
}

EG_TEST("UNIT", "002", length_conversions_are_bit_exact_in_both_directions) {
    const struct {
        const char* from;
        const char* to;
        double value;
        double expected;
    } cases[] = {
        {"m", "cm", 1.0, 100.0},
        {"m", "mm", 1.0, 1000.0},
        {"cm", "mm", 2.5, 25.0},
        {"km", "m", 2.0, 2000.0},
        {"mm", "m", 1.0, 0.001},
        {"cm", "m", 125.0, 1.25},
        {"m", "m", 0.1, 0.1},
    };
    for (const auto& item : cases) {
        const auto target = data::UnitRegistry::builtin().parse(item.to);
        EG_REQUIRE_OK(target);
        const auto moved = data::convert(quantity(item.from, item.value), EG_REQUIRE_OK(target));
        EG_REQUIRE(moved.ok());
        // Exactly representable factors are bit-exact, not approximately equal.
        EG_REQUIRE_EQ(moved.value().value, item.expected);
        EG_REQUIRE_EQ(std::bit_cast<std::uint64_t>(moved.value().value),
                      std::bit_cast<std::uint64_t>(item.expected));
        const auto back = data::UnitRegistry::builtin().parse(item.from);
        EG_REQUIRE_OK(back);
        const auto round_trip = data::convert(moved.value(), EG_REQUIRE_OK(back));
        EG_REQUIRE_OK(round_trip);
        EG_REQUIRE_EQ(round_trip.value().value, item.value);
    }
    const auto centimeters = data::UnitRegistry::builtin().parse("cm");
    EG_REQUIRE_OK(centimeters);
    EG_REQUIRE(EG_REQUIRE_OK(centimeters).scale().is_exact());
    EG_REQUIRE_EQ(EG_REQUIRE_OK(centimeters).scale().numerator(), std::int64_t{1});
    EG_REQUIRE_EQ(EG_REQUIRE_OK(centimeters).scale().denominator(), std::int64_t{100});
    // The dimension travels with the result, so a length never becomes a bare number.
    EG_REQUIRE_EQ(EG_REQUIRE_OK(centimeters).dimension(),
                  EG_REQUIRE_OK(data::UnitRegistry::builtin().parse("m")).dimension());
}

EG_TEST("UNIT", "003", time_conversions_keep_the_duration_dimension_and_are_exact) {
    const struct {
        const char* from;
        const char* to;
        double value;
        double expected;
    } cases[] = {
        {"h", "s", 1.0, 3600.0},
        {"h", "ms", 1.0, 3600000.0},
        {"min", "s", 1.5, 90.0},
        {"s", "ms", 0.25, 250.0},
        {"ms", "s", 1.0, 0.001},
        {"min", "ms", 2.0, 120000.0},
    };
    for (const auto& item : cases) {
        const auto target = data::UnitRegistry::builtin().parse(item.to);
        EG_REQUIRE_OK(target);
        const auto moved = data::convert(quantity(item.from, item.value), EG_REQUIRE_OK(target));
        EG_REQUIRE(moved.ok());
        EG_REQUIRE_EQ(moved.value().value, item.expected);
        EG_REQUIRE_EQ(moved.value().unit.dimension(),
                      data::Dimension::base(data::DimensionBase::time));
    }
    // Adding across the same dimension moves through the declared scale, not a guess.
    const auto sum = data::add(quantity("min", 1.0), quantity("s", 90.0));
    EG_REQUIRE_OK(sum);
    EG_REQUIRE_EQ(sum.value().value, 2.5);
    EG_REQUIRE(sum.value().unit == quantity("min", 0.0).unit);
}

EG_TEST("UNIT", "004", absolute_temperatures_convert_with_affine_not_multiplicative_rules) {
    const auto kelvin = data::UnitRegistry::builtin().parse("K");
    const auto celsius = data::UnitRegistry::builtin().parse("degC");
    const auto fahrenheit = data::UnitRegistry::builtin().parse("degF");
    EG_REQUIRE_OK(kelvin);
    EG_REQUIRE_OK(celsius);
    EG_REQUIRE_OK(fahrenheit);
    EG_REQUIRE(EG_REQUIRE_OK(kelvin).is_absolute());
    EG_REQUIRE(EG_REQUIRE_OK(celsius).is_absolute());

    // 0 degC is exactly 273.15 K, and 32 degF is 0 degC.
    const auto freezing = data::convert(quantity("degC", 0.0), EG_REQUIRE_OK(kelvin));
    EG_REQUIRE_OK(freezing);
    require_close(freezing.value().value, 273.15, "0 degC in kelvin");
    const auto to_celsius = data::convert(quantity("degF", 32.0), EG_REQUIRE_OK(celsius));
    EG_REQUIRE_OK(to_celsius);
    require_close(to_celsius.value().value, 0.0, "32 degF in degC");
    const auto boiling = data::convert(quantity("degC", 100.0), EG_REQUIRE_OK(kelvin));
    EG_REQUIRE_OK(boiling);
    require_close(boiling.value().value, 373.15, "100 degC in kelvin");
    const auto back = data::convert(quantity("K", 273.15), EG_REQUIRE_OK(celsius));
    EG_REQUIRE_OK(back);
    require_close(back.value().value, 0.0, "273.15 K in degC");
    const auto body = data::convert(quantity("degC", 37.0), EG_REQUIRE_OK(fahrenheit));
    EG_REQUIRE_OK(body);
    require_close(body.value().value, 98.6, "37 degC in degF");
    // A multiplicative-only reading of an affine scale would put 100 degF at 100/1.8 degC;
    // the offset rule is what makes the assertions above come out as they do.
    EG_REQUIRE(std::fabs(body.value().value - 100.0 / 1.8) > 1e-6);
    // The offset is recorded as part of the unit, so two spellings of one scale agree.
    EG_REQUIRE(EG_REQUIRE_OK(celsius).offset() != 0.0);
    EG_REQUIRE(EG_REQUIRE_OK(kelvin).offset() == 0.0);
    EG_REQUIRE(EG_REQUIRE_OK(celsius).dimension() == EG_REQUIRE_OK(fahrenheit).dimension());
}

EG_TEST("UNIT", "005", absolute_offset_arithmetic_is_rejected_until_a_difference_unit_is_declared) {
    // Two absolute temperatures are not addable as scalars.
    const auto sum = data::add(quantity("degC", 20.0), quantity("degC", 5.0));
    EG_REQUIRE(!sum.ok());
    EG_REQUIRE_DIAGNOSTIC(sum.diagnostic(), DetailCode::offset_unit_arithmetic,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE_EQ(sum.diagnostic().location.description, std::string{"degC + degC"});

    const auto difference = data::subtract(quantity("degC", 20.0), quantity("degC", 5.0));
    EG_REQUIRE(!difference.ok());
    EG_REQUIRE_DIAGNOSTIC(difference.diagnostic(), DetailCode::offset_unit_arithmetic,
                          ErrorCode::invalid_argument, Severity::error);

    const auto scaled = data::multiply(quantity("degC", 20.0), quantity("1", 2.0));
    EG_REQUIRE(!scaled.ok());
    EG_REQUIRE_DIAGNOSTIC(scaled.diagnostic(), DetailCode::offset_unit_arithmetic,
                          ErrorCode::invalid_argument, Severity::error);

    // The declared way out: name the difference unit, then the arithmetic is ordinary.
    const auto delta = data::as_difference_unit(EG_REQUIRE_OK(data::UnitRegistry::builtin().parse(
        "degC")));
    EG_REQUIRE_OK(delta);
    EG_REQUIRE_EQ(delta.value().source_text(), std::string{"delta_degC"});
    EG_REQUIRE(delta.value().kind() == data::UnitKind::difference);
    EG_REQUIRE(!delta.value().is_absolute());

    const auto warmed = data::add(quantity("degC", 20.0), quantity("delta_degC", 5.0));
    EG_REQUIRE_OK(warmed);
    EG_REQUIRE_EQ(warmed.value().value, 25.0);
    EG_REQUIRE(warmed.value().unit == quantity("degC", 0.0).unit);

    const auto span = data::subtract(quantity("degC", 20.0), quantity("delta_degC", 5.0));
    EG_REQUIRE_OK(span);
    EG_REQUIRE_EQ(span.value().value, 15.0);

    // Two differences are ordinary quantities and their result keeps the temperature dimension.
    const auto spans = data::add(quantity("delta_degC", 3.0), quantity("delta_degC", 4.0));
    EG_REQUIRE_OK(spans);
    EG_REQUIRE_EQ(spans.value().value, 7.0);
    EG_REQUIRE_EQ(spans.value().unit.dimension(),
                  data::Dimension::base(data::DimensionBase::temperature));
    // A Fahrenheit difference is a declared difference unit too, not an absolute scale.
    const auto fahrenheit_delta = data::add(quantity("degF", 68.0), quantity("delta_degF", 9.0));
    EG_REQUIRE_OK(fahrenheit_delta);
    require_close(fahrenheit_delta.value().value, 77.0, "68 degF plus 9 delta_degF");
}

EG_TEST("UNIT", "006", compound_units_combine_and_cancel_their_dimensions_and_scales) {
    const auto metres_per_second = data::UnitRegistry::builtin().parse("m/s");
    EG_REQUIRE_OK(metres_per_second);
    const auto dimension = EG_REQUIRE_OK(metres_per_second).dimension();
    EG_REQUIRE_EQ(dimension.exponent(data::DimensionBase::length), std::int16_t{1});
    EG_REQUIRE_EQ(dimension.exponent(data::DimensionBase::time), std::int16_t{-1});
    EG_REQUIRE(!dimension.is_dimensionless());

    const auto kilograms_per_cubic_metre = data::UnitRegistry::builtin().parse("kg/m3");
    EG_REQUIRE_OK(kilograms_per_cubic_metre);
    const auto density = EG_REQUIRE_OK(kilograms_per_cubic_metre).dimension();
    EG_REQUIRE_EQ(density.exponent(data::DimensionBase::mass), std::int16_t{1});
    EG_REQUIRE_EQ(density.exponent(data::DimensionBase::length), std::int16_t{-3});

    // Product spellings parse the same way as the slash form.
    // Two spellings of one numerator set and one denominator are one unit: the source text is
    // not part of the identity.
    const auto dot_form = data::UnitRegistry::builtin().parse("kg.m/s2");
    EG_REQUIRE_OK(dot_form);
    const auto from_slash = data::UnitRegistry::builtin().parse("m.kg/s2");
    EG_REQUIRE_OK(from_slash);
    EG_REQUIRE(dot_form.value() == from_slash.value());
    EG_REQUIRE_EQ(dot_form.value().canonical_text(), from_slash.value().canonical_text());
    EG_REQUIRE_EQ(dot_form.value().dimension().exponent(data::DimensionBase::time),
                  std::int16_t{-2});
    // The single-denominator rule makes kg/(s2*m) a different unit, not a re-spelling.
    const auto denominator_group = data::UnitRegistry::builtin().parse("kg/s2*m");
    EG_REQUIRE_OK(denominator_group);
    EG_REQUIRE(!(dot_form.value() == denominator_group.value()));
    EG_REQUIRE_EQ(denominator_group.value().dimension().exponent(data::DimensionBase::length),
                  std::int16_t{-1});

    // Conversion factors compound: 36 km/h is exactly 10 m/s.
    const auto target = data::UnitRegistry::builtin().parse("m/s");
    EG_REQUIRE_OK(target);
    const auto speed = data::convert(quantity("km/h", 36.0), EG_REQUIRE_OK(target));
    EG_REQUIRE_OK(speed);
    EG_REQUIRE_EQ(speed.value().value, 10.0);
    const auto grams_per_cm3 = data::convert(quantity("kg/m3", 1000.0),
                                            EG_REQUIRE_OK(data::UnitRegistry::builtin().parse(
                                                "g/cm3")));
    EG_REQUIRE_OK(grams_per_cm3);
    EG_REQUIRE_EQ(grams_per_cm3.value().value, 1.0);

    // Cancelling a dimension against itself is what makes m/m a declared dimensionless result.
    const auto ratio = data::divide(quantity("m", 3.0), quantity("m", 4.0));
    EG_REQUIRE_OK(ratio);
    EG_REQUIRE(ratio.value().unit.is_dimensionless());
    EG_REQUIRE_EQ(ratio.value().value, 0.75);

    // Multiplying two lengths gives an area dimension, and dividing it back cancels.
    const auto area = data::multiply(quantity("m", 2.0), quantity("m", 3.0));
    EG_REQUIRE_OK(area);
    EG_REQUIRE_EQ(area.value().unit.dimension().exponent(data::DimensionBase::length),
                  std::int16_t{2});
    EG_REQUIRE_EQ(area.value().value, 6.0);
    const auto back = data::divide(area.value(), quantity("m", 2.0));
    EG_REQUIRE_OK(back);
    EG_REQUIRE_EQ(back.value().unit.dimension(),
                  data::Dimension::base(data::DimensionBase::length));
    EG_REQUIRE_EQ(back.value().value, 3.0);

    // The result carries its dimension: a unit is never silently dropped.
    const auto velocity = data::divide(quantity("m", 10.0), quantity("s", 4.0));
    EG_REQUIRE_OK(velocity);
    EG_REQUIRE_EQ(velocity.value().unit.dimension().exponent(data::DimensionBase::time),
                  std::int16_t{-1});
    EG_REQUIRE_EQ(velocity.value().unit.source_text(), std::string{"m/s"});
}

EG_TEST("UNIT", "007", incompatible_units_are_rejected_naming_both_sides) {
    const auto sum = data::add(quantity("m", 1.0), quantity("s", 1.0));
    EG_REQUIRE(!sum.ok());
    EG_REQUIRE_DIAGNOSTIC(sum.diagnostic(), DetailCode::incompatible_unit,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE_EQ(sum.diagnostic().location.description, std::string{"m + s"});

    const auto difference = data::subtract(quantity("kg", 2.0), quantity("m", 2.0));
    EG_REQUIRE(!difference.ok());
    EG_REQUIRE_EQ(difference.diagnostic().detail, DetailCode::incompatible_unit);

    const auto moved = data::convert(quantity("m", 1.0),
                                     EG_REQUIRE_OK(data::UnitRegistry::builtin().parse("s")));
    EG_REQUIRE(!moved.ok());
    EG_REQUIRE_DIAGNOSTIC(moved.diagnostic(), DetailCode::incompatible_unit,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE_EQ(moved.diagnostic().location.description, std::string{"m -> s"});

    // An absolute scale and a scale without a real zero are not convertible either.
    const auto crossed = data::convert(quantity("degC", 20.0),
                                       EG_REQUIRE_OK(data::UnitRegistry::builtin().parse(
                                           "delta_degC")));
    EG_REQUIRE(!crossed.ok());
    EG_REQUIRE_EQ(crossed.diagnostic().detail, DetailCode::incompatible_unit);

    // Nothing is committed by a rejected unit operation: no version and no event appears.
    Fixture fixture;
    auto table = make_table(22);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(22001)});
    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.unit = "m";
    spec.cells = {float64_cell(1.0)};
    const ColumnLine line = create_column(fixture, table, spec);
    const std::uint64_t versions_before = fixture.version_count();
    const auto again = data::convert(quantity("m", 1.0),
                                     EG_REQUIRE_OK(data::UnitRegistry::builtin().parse("s")));
    EG_REQUIRE(!again.ok());
    EG_REQUIRE_EQ(fixture.version_count(), versions_before);
    const DocumentView view = fixture.document();
    const ColumnBindingRecord* record = view.find(data::column_id(line.column));
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE(record->unit.has_value());
    EG_REQUIRE(record->unit == quantity("m", 0.0).unit);
    (void)record;
}

EG_TEST("UNIT", "012", unknown_malformed_and_empty_units_are_rejected_with_a_location) {
    const std::vector<std::string> rejected = {
        "furlong",      // not in the registry
        "m/",           // empty denominator term
        "/s",           // empty numerator term
        "m//s",         // two denominators
        "m/s/x",        // more than one denominator
        "kg m",         // an unseparated pair
        "m-",           // a trailing separator
        "degC2",        // an absolute scale cannot be raised
        "m/0",          // an unknown atom
        "2",            // a bare exponent with no atom
        "",             // empty is not a declared dimensionless unit
    };
    for (const auto& text : rejected) {
        const auto parsed = data::UnitRegistry::builtin().parse(text);
        EG_REQUIRE(!parsed.ok());
        EG_REQUIRE_DIAGNOSTIC(parsed.diagnostic(), DetailCode::unknown_unit,
                              ErrorCode::invalid_argument, Severity::error);
        // The rejected spelling is what the diagnostic reports, so a caller can find it.
        EG_REQUIRE_EQ(parsed.diagnostic().severity, Severity::error);
    }
    EG_REQUIRE_EQ(data::UnitRegistry::builtin().parse("m/").diagnostic().location.description,
                  std::string{"m/"});

    // Dimensionless is declared by name, never invented from an omission.
    const auto declared_dimensionless = data::UnitRegistry::builtin().parse("1");
    EG_REQUIRE_OK(declared_dimensionless);
    EG_REQUIRE(EG_REQUIRE_OK(declared_dimensionless).is_dimensionless());
    EG_REQUIRE(data::dimensionless_unit() == EG_REQUIRE_OK(declared_dimensionless));
    const auto percent = data::UnitRegistry::builtin().parse("percent");
    EG_REQUIRE_OK(percent);
    EG_REQUIRE(EG_REQUIRE_OK(percent).is_dimensionless());
    const auto percented = data::convert(quantity("percent", 50.0), EG_REQUIRE_OK(percent));
    EG_REQUIRE_OK(percented);
    EG_REQUIRE_EQ(percented.value().value, 50.0);
    const auto as_ratio = data::convert(quantity("percent", 50.0),
                                        EG_REQUIRE_OK(data::UnitRegistry::builtin().parse("1")));
    EG_REQUIRE_OK(as_ratio);
    EG_REQUIRE_EQ(as_ratio.value().value, 0.5);

    // A command that declares an unknown unit is rejected and changes no metadata.
    Fixture fixture;
    auto table = make_table(23);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(23001)});
    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.unit = "m";
    spec.cells = {float64_cell(1.0)};
    ColumnLine line = create_column(fixture, table, spec);

    ParameterMap attempt = Fixture::with(data::argument::kColumn, line.column);
    Fixture::put(attempt, data::argument::kUnit, std::string{"furlong"});
    const auto rejected_unit = fixture.run(data::command_id::kColumnMetadata, std::move(attempt),
                                          fixture.declared_writes({line.column}));
    EG_REQUIRE_REJECTED(rejected_unit, DetailCode::unknown_unit, ErrorCode::invalid_argument);
    EG_REQUIRE_NO_WRITE(rejected_unit);
    const DocumentView view = fixture.document();
    const ColumnBindingRecord* record = view.find(data::column_id(line.column));
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE(record->unit == quantity("m", 0.0).unit);
}

EG_TEST("UNIT", "013", angles_convert_within_tolerance_and_dimensionless_is_not_missing) {
    const auto radians = data::UnitRegistry::builtin().parse("rad");
    const auto degrees = data::UnitRegistry::builtin().parse("deg");
    EG_REQUIRE_OK(radians);
    EG_REQUIRE_OK(degrees);
    EG_REQUIRE(EG_REQUIRE_OK(degrees).dimension() == EG_REQUIRE_OK(radians).dimension());
    // A degree is an irrational fraction of a radian, so the tolerance rule applies.
    EG_REQUIRE(!EG_REQUIRE_OK(degrees).scale().is_exact());

    const auto half_turn = data::convert(quantity("deg", 180.0), EG_REQUIRE_OK(radians));
    EG_REQUIRE_OK(half_turn);
    require_close(half_turn.value().value, 3.14159265358979323846, "180 deg in rad");
    const auto turn = data::convert(quantity("deg", 360.0), EG_REQUIRE_OK(radians));
    EG_REQUIRE_OK(turn);
    require_close(turn.value().value, 6.28318530717958647692, "360 deg in rad");
    const auto back = data::convert(quantity("rad", 1.0), EG_REQUIRE_OK(degrees));
    EG_REQUIRE_OK(back);
    require_close(back.value().value, 57.29577951308232087679, "1 rad in deg");
    const auto round_trip = data::convert(back.value(), EG_REQUIRE_OK(radians));
    EG_REQUIRE_OK(round_trip);
    require_close(round_trip.value().value, 1.0, "rad -> deg -> rad");

    // An angle is a dimension, so it is not dimensionless and not missing.
    EG_REQUIRE(!EG_REQUIRE_OK(degrees).is_dimensionless());
    EG_REQUIRE(EG_REQUIRE_OK(data::UnitRegistry::builtin().parse("1")).is_dimensionless());
    const auto ratio = data::divide(quantity("deg", 90.0), quantity("deg", 45.0));
    EG_REQUIRE_OK(ratio);
    EG_REQUIRE(ratio.value().unit.is_dimensionless());
    EG_REQUIRE_EQ(ratio.value().value, 2.0);

    // Missing metadata stays absent: a column without a unit reports none, not "1".
    Fixture fixture;
    auto table = make_table(24);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(24001)});
    ColumnSpec plain;
    plain.type = LogicalType::float64;
    plain.cells = {float64_cell(2.0)};
    const ColumnLine unitless = create_column(fixture, table, plain);
    ColumnSpec dimensionless;
    dimensionless.type = LogicalType::float64;
    dimensionless.unit = "1";
    dimensionless.cells = {float64_cell(2.0)};
    const ColumnLine declared = create_column(fixture, table, dimensionless);

    const DocumentView view = fixture.document();
    const ColumnBindingRecord* absent = view.find(data::column_id(unitless.column));
    const ColumnBindingRecord* present = view.find(data::column_id(declared.column));
    EG_REQUIRE(absent != nullptr && present != nullptr);
    EG_REQUIRE(!absent->unit.has_value());
    EG_REQUIRE(present->unit.has_value());
    EG_REQUIRE(present->unit.value().is_dimensionless());
}
