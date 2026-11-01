#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "data_test_support.hpp"

using namespace egtest;

namespace {

// The seeded scalar reference named by the conformance plan's property-test gate. The seed is
// recorded in the case output so a failure is reproducible.
constexpr std::uint64_t kPropertySeed = 20261102;
constexpr std::size_t kPropertyValues = 512;

[[nodiscard]] std::int64_t hours_micros(std::int64_t hours) {
    return hours * 3600LL * data::kMicrosPerSecond;
}

data::CivilDateTime civil(std::int32_t year, std::uint32_t month, std::uint32_t day,
                          std::uint32_t hour = 0, std::uint32_t minute = 0) {
    data::CivilDateTime local;
    local.date = data::CivilDate{year, month, day};
    local.time = data::CivilTime{hour, minute, 0, 0};
    return local;
}

}  // namespace

EG_TEST("TYP", "001", signed_integer_boundaries_round_trip_without_wrapping) {
    struct Boundary {
        LogicalType type;
        std::int64_t low;
        std::int64_t high;
        std::uint32_t bits;
    };
    const std::vector<Boundary> forms = {
        {LogicalType::int8, -128, 127, 8},
        {LogicalType::int16, -32768, 32767, 16},
        {LogicalType::int32, -2147483648LL, 2147483647LL, 32},
        {LogicalType::int64, std::numeric_limits<std::int64_t>::min(),
         std::numeric_limits<std::int64_t>::max(), 64},
    };
    std::uint64_t identity = 7000;
    for (const auto& form : forms) {
        const FieldId form_field = data::field_id(identity);
        const ChunkId form_chunk = data::chunk_id(identity + 1);
        identity += 2;
        EG_REQUIRE_EQ(data::storage_bits(form.type).value_or(0), form.bits);
        auto chunk = chunk_of(form_field, form.type, form_chunk,
                              {int_cell(form.low, form.type), int_cell(form.high, form.type),
                               int_cell(0, form.type), Cell::missing(form.type)});
        EG_REQUIRE_EQ(chunk->row_count(), std::uint64_t{4});
        EG_REQUIRE_EQ(chunk->type(), form.type);
        EG_REQUIRE_CELL_EQ(chunk_cell(*chunk, 0), int_cell(form.low, form.type));
        EG_REQUIRE_CELL_EQ(chunk_cell(*chunk, 1), int_cell(form.high, form.type));
        EG_REQUIRE_CELL_EQ(chunk_cell(*chunk, 2), int_cell(0, form.type));
        EG_REQUIRE(chunk_cell(*chunk, 3).is_missing());
        EG_REQUIRE_EQ(chunk_cell(*chunk, 0).type(), form.type);

        auto builder = ChunkBuilder::create(form_field, form.type);
        EG_REQUIRE_OK(builder);
        if (form.bits < 64U) {
            // One step past the declared width is a rejection, never a wrap.
            auto overflow = EG_REQUIRE_OK(builder).append_int(form.high + 1);
            EG_REQUIRE(overflow.has_value());
            EG_REQUIRE_DIAGNOSTIC(*overflow, DetailCode::number_overflow,
                                  ErrorCode::invalid_argument, Severity::error);
            auto underflow = EG_REQUIRE_OK(builder).append_int(form.low - 1);
            EG_REQUIRE(underflow.has_value());
            EG_REQUIRE_EQ(underflow->detail, DetailCode::number_overflow);
        } else {
            // The widest signed form has no in-band value one step past its boundary, so the
            // range table is the check and an over-wide unsigned input is a type rejection.
            EG_REQUIRE(data::in_range(form.high, form.type));
            EG_REQUIRE(data::in_range(form.low, form.type));
            EG_REQUIRE(!data::in_range(std::uint64_t{1} << 63U, form.type));
            auto rejection = EG_REQUIRE_OK(builder).append_uint(
                std::numeric_limits<std::uint64_t>::max());
            EG_REQUIRE(rejection.has_value());
            EG_REQUIRE_EQ(rejection->detail, DetailCode::type_mismatch);
        }
        EG_REQUIRE_EQ(EG_REQUIRE_OK(builder).row_count(), std::uint64_t{0});
    }
}

EG_TEST("TYP", "001", unsigned_integer_boundaries_keep_their_width_and_signedness) {
    struct Boundary {
        LogicalType type;
        std::uint64_t high;
        std::uint32_t bits;
    };
    const std::vector<Boundary> forms = {
        {LogicalType::uint8, 255ULL, 8},
        {LogicalType::uint16, 65535ULL, 16},
        {LogicalType::uint32, 4294967295ULL, 32},
        {LogicalType::uint64, std::numeric_limits<std::uint64_t>::max(), 64},
    };
    std::uint64_t identity = 7100;
    for (const auto& form : forms) {
        const FieldId form_field = data::field_id(identity);
        const ChunkId form_chunk = data::chunk_id(identity + 1);
        identity += 2;
        EG_REQUIRE_EQ(data::storage_bits(form.type).value_or(0), form.bits);
        auto chunk = chunk_of(form_field, form.type, form_chunk,
                              {uint_cell(0, form.type), uint_cell(form.high, form.type)});
        EG_REQUIRE_CELL_EQ(chunk_cell(*chunk, 0), uint_cell(0, form.type));
        EG_REQUIRE_CELL_EQ(chunk_cell(*chunk, 1), uint_cell(form.high, form.type));
        // A signed append into an unsigned field is a type rejection, not a reinterpretation.
        auto builder = ChunkBuilder::create(form_field, form.type);
        auto rejection = EG_REQUIRE_OK(builder).append_int(-1);
        EG_REQUIRE(rejection.has_value());
        EG_REQUIRE_DIAGNOSTIC(*rejection, DetailCode::type_mismatch, ErrorCode::invalid_argument,
                              Severity::error);
        auto at_boundary = EG_REQUIRE_OK(builder).append_uint(form.high);
        EG_REQUIRE(!at_boundary.has_value());
    }
}

EG_TEST("TYP", "001", columnar_storage_matches_the_scalar_reference_over_a_seeded_sequence) {
    const std::vector<LogicalType> types = {LogicalType::int32, LogicalType::uint32,
                                            LogicalType::float64, LogicalType::boolean};
    std::mt19937_64 engine{kPropertySeed};
    std::uint64_t identity = 7200;
    std::size_t comparisons = 0;
    for (const LogicalType type : types) {
        const FieldId sequence_field = data::field_id(identity);
        const ChunkId sequence_chunk = data::chunk_id(identity + 1);
        identity += 2;
        std::vector<Cell> reference;
        reference.reserve(kPropertyValues);
        for (std::size_t index = 0; index < kPropertyValues; ++index) {
            if (index % 17 == 0) {
                reference.push_back(Cell::missing(type));
                continue;
            }
            switch (type) {
                case LogicalType::int32:
                    reference.push_back(int_cell(static_cast<std::int64_t>(engine() % 4000001ULL) -
                                                     2000000LL,
                                                 type));
                    break;
                case LogicalType::uint32:
                    reference.push_back(
                        uint_cell(engine() % 4294967296ULL, type));
                    break;
                case LogicalType::float64: {
                    const double value = static_cast<double>(engine()) / 1e9 - 5000.0;
                    reference.push_back(float64_cell(index % 31 == 0 ? -value * 0.0 : value));
                    break;
                }
                default:
                    reference.push_back(bool_cell((engine() & 1U) != 0U));
                    break;
            }
        }
        auto chunk = chunk_of(sequence_field, type, sequence_chunk, reference);
        EG_REQUIRE_EQ(chunk->row_count(), kPropertyValues);
        for (std::size_t index = 0; index < kPropertyValues; ++index) {
            const Cell stored = chunk_cell(*chunk, index);
            if (!data::structurally_equal(stored, reference[index])) {
                throw TestFailure("seed " + std::to_string(kPropertySeed) + ", type " +
                                  std::string{data::to_string(type)} + ", operation count " +
                                  std::to_string(kPropertyValues) + ", minimal failing index " +
                                  std::to_string(index) + ": expected " + display(reference[index]) +
                                  ", got " + display(stored));
            }
            ++comparisons;
        }
    }
    EG_REQUIRE_EQ(comparisons, kPropertyValues * types.size());
}

EG_TEST("TYP", "003", float_storage_is_exact_and_never_promoted_or_demoted) {
    const float f32_subnormal = std::numeric_limits<float>::denorm_min();
    const double f64_subnormal = std::numeric_limits<double>::denorm_min();
    const float f32_third = 1.0F / 3.0F;
    const float f32_tenth = 0.1F;
    const double f64_third = 1.0 / 3.0;
    const double f64_tenth = 0.1;

    auto narrow = chunk_of(data::field_id(7301), LogicalType::float32, data::chunk_id(7302),
                           {float32_cell(f32_subnormal),
                            float32_cell(std::numeric_limits<float>::max()), float32_cell(f32_third),
                            float32_cell(f32_tenth)});
    const std::vector<std::uint32_t> expected_narrow = {
        std::bit_cast<std::uint32_t>(f32_subnormal),
        std::bit_cast<std::uint32_t>(std::numeric_limits<float>::max()),
        std::bit_cast<std::uint32_t>(f32_third), std::bit_cast<std::uint32_t>(f32_tenth)};
    for (std::size_t index = 0; index < expected_narrow.size(); ++index) {
        const Cell stored = chunk_cell(*narrow, index);
        EG_REQUIRE_EQ(stored.type(), LogicalType::float32);
        EG_REQUIRE_EQ(stored.bit_pattern().value_or(0),
                      static_cast<std::uint64_t>(expected_narrow[index]));
    }

    auto wide = chunk_of(data::field_id(7303), LogicalType::float64, data::chunk_id(7304),
                         {float64_cell(f64_subnormal),
                          float64_cell(std::numeric_limits<double>::max()), float64_cell(f64_third),
                          float64_cell(f64_tenth)});
    const std::vector<std::uint64_t> expected_wide = {
        std::bit_cast<std::uint64_t>(f64_subnormal),
        std::bit_cast<std::uint64_t>(std::numeric_limits<double>::max()),
        std::bit_cast<std::uint64_t>(f64_third), std::bit_cast<std::uint64_t>(f64_tenth)};
    for (std::size_t index = 0; index < expected_wide.size(); ++index) {
        const Cell stored = chunk_cell(*wide, index);
        EG_REQUIRE_EQ(stored.type(), LogicalType::float64);
        EG_REQUIRE_EQ(stored.bit_pattern().value_or(0), expected_wide[index]);
    }
    EG_REQUIRE_EQ(data::storage_bits(LogicalType::float32).value_or(0), std::uint32_t{32});
    EG_REQUIRE_EQ(data::storage_bits(LogicalType::float64).value_or(0), std::uint32_t{64});

    // A value is never reinterpreted because the other width would happen to fit.
    auto narrow_builder = ChunkBuilder::create(data::field_id(7305), LogicalType::float32);
    auto demotion = EG_REQUIRE_OK(narrow_builder).append_float64(f64_tenth);
    EG_REQUIRE(demotion.has_value());
    EG_REQUIRE_DIAGNOSTIC(*demotion, DetailCode::type_mismatch, ErrorCode::invalid_argument,
                          Severity::error);
    auto wide_builder = ChunkBuilder::create(data::field_id(7306), LogicalType::float64);
    auto promotion = EG_REQUIRE_OK(wide_builder).append_float32(f32_tenth);
    EG_REQUIRE(promotion.has_value());
    EG_REQUIRE_EQ(promotion->detail, DetailCode::type_mismatch);
}

EG_TEST("TYP", "005", signed_zero_keeps_its_sign_and_both_comparisons_hold) {
    const double positive_zero = 0.0;
    const double negative_zero = std::copysign(0.0, -1.0);
    const double product_zero = -1.0 * 0.0;
    const double quotient_zero = -1.0 / std::numeric_limits<double>::infinity();
    auto chunk = chunk_of(data::field_id(7401), LogicalType::float64, data::chunk_id(7402),
                          {float64_cell(positive_zero), float64_cell(negative_zero),
                           float64_cell(product_zero), float64_cell(quotient_zero),
                           Cell::missing(LogicalType::float64)});

    EG_REQUIRE_EQ(chunk_cell(*chunk, 0).bit_pattern().value_or(1), std::uint64_t{0});
    EG_REQUIRE_EQ(chunk_cell(*chunk, 1).bit_pattern().value_or(0),
                  std::uint64_t{0x8000000000000000ULL});
    EG_REQUIRE(!chunk_cell(*chunk, 0).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 1).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 2).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 3).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 1).is_zero());
    EG_REQUIRE(!chunk_cell(*chunk, 1).is_missing());
    EG_REQUIRE(!chunk_cell(*chunk, 1).is_nan());

    // IEEE numeric equality treats the two zeros as equal; structural identity does not.
    EG_REQUIRE(data::ieee_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 1)).value_or(false));
    EG_REQUIRE(!data::structurally_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 1)));
    // Multiplication and division that produce a negative zero store that sign, not a zero
    // that lost it.
    EG_REQUIRE(data::structurally_equal(chunk_cell(*chunk, 1), chunk_cell(*chunk, 2)));
    EG_REQUIRE(data::structurally_equal(chunk_cell(*chunk, 1), chunk_cell(*chunk, 3)));
    EG_REQUIRE(!data::structurally_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 2)));
    EG_REQUIRE(!data::structurally_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 4)));
    // A numeric zero of another storage type is a different state from either signed zero.
    EG_REQUIRE(!data::structurally_equal(chunk_cell(*chunk, 0), int_cell(0, LogicalType::int64)));
}

EG_TEST("TYP", "006", nan_payloads_and_infinities_are_values_not_missing_or_errors) {
    constexpr std::uint64_t kQuietPayloadOne = 0x7FF8000000000001ULL;
    constexpr std::uint64_t kQuietPayloadTwo = 0x7FFA5DEAD0000001ULL;
    constexpr std::uint64_t kNegativeQuietNan = 0xFFF8000000000001ULL;
    constexpr std::uint64_t kPositiveInfinity = 0x7FF0000000000000ULL;
    constexpr std::uint64_t kNegativeInfinity = 0xFFF0000000000000ULL;

    auto builder = ChunkBuilder::create(data::field_id(7501), LogicalType::float64);
    for (const std::uint64_t bits : {kQuietPayloadOne, kQuietPayloadTwo, kNegativeQuietNan,
                                     kPositiveInfinity, kNegativeInfinity}) {
        auto appended = EG_REQUIRE_OK(builder).append_bits(bits);
        EG_REQUIRE(!appended.has_value());
    }
    auto finished = EG_REQUIRE_OK(builder).finish(data::chunk_id(7502));
    auto chunk = std::make_shared<const Chunk>(EG_REQUIRE_TAKEN(std::move(finished)));

    const std::vector<std::uint64_t> patterns = {kQuietPayloadOne, kQuietPayloadTwo,
                                                 kNegativeQuietNan, kPositiveInfinity,
                                                 kNegativeInfinity};
    for (std::size_t index = 0; index < patterns.size(); ++index) {
        EG_REQUIRE_EQ(chunk_cell(*chunk, index).bit_pattern().value_or(0), patterns[index]);
    }
    // A NaN is a value: not missing, not an error, and never collapsed onto the other NaN.
    EG_REQUIRE(chunk_cell(*chunk, 0).is_nan());
    EG_REQUIRE(!chunk_cell(*chunk, 0).is_missing());
    EG_REQUIRE(chunk_cell(*chunk, 1).is_nan());
    EG_REQUIRE(!data::structurally_equal(chunk_cell(*chunk, 0), Cell::missing(LogicalType::float64)));
    // Aggregate change detection treats every NaN payload as equal while the bits stay distinct.
    EG_REQUIRE(data::structurally_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 1)));
    EG_REQUIRE(chunk_cell(*chunk, 0).bit_pattern() != chunk_cell(*chunk, 1).bit_pattern());
    EG_REQUIRE(!data::ieee_equal(chunk_cell(*chunk, 0), chunk_cell(*chunk, 1)).has_value());
    EG_REQUIRE(chunk_cell(*chunk, 2).sign_bit());
    // Infinities are not clipped to the largest finite value.
    EG_REQUIRE(chunk_cell(*chunk, 3).is_infinity());
    EG_REQUIRE(!chunk_cell(*chunk, 3).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 4).is_infinity());
    EG_REQUIRE(chunk_cell(*chunk, 4).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 3).bit_pattern().value_or(0) !=
               std::bit_cast<std::uint64_t>(std::numeric_limits<double>::max()));

    // The same payload survives publication through the command path, not only the builder.
    Fixture fixture;
    auto table = make_table(7);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(11), data::row_id(12), data::row_id(13),
                                 data::row_id(14), data::row_id(15)});
    std::vector<Cell> cells;
    for (const std::uint64_t bits : patterns) {
        cells.push_back(Cell::from_bits(LogicalType::float64, bits));
    }
    ColumnSpec spec;
    spec.type = LogicalType::float64;
    spec.cells = cells;
    ColumnLine line = create_column(fixture, table, spec);
    auto lease = fixture.lease(data::dataset_version_id(line.version));
    for (std::size_t index = 0; index < patterns.size(); ++index) {
        const Cell stored = EG_REQUIRE_TAKEN(lease.cell_at(data::field_id(line.field), index));
        EG_REQUIRE_EQ(stored.bit_pattern().value_or(0), patterns[index]);
    }

    // The bit form is accepted only when the declared type reports exactly those bits back,
    // so a narrow column never silently masks the high bits a caller named and a boolean
    // never reads 2 as 0. The canonical form is the one bit_pattern() widens to.
    auto narrow = ChunkBuilder::create(data::field_id(7521), LogicalType::int8);
    EG_REQUIRE(!EG_REQUIRE_OK(narrow).append_bits(0x7FULL).has_value());
    EG_REQUIRE(!EG_REQUIRE_OK(narrow).append_bits(0xFFFFFFFFFFFFFF80ULL).has_value());
    const auto masked = EG_REQUIRE_OK(narrow).append_bits(0x80ULL);
    EG_REQUIRE(masked.has_value());
    EG_REQUIRE_EQ(masked->detail, DetailCode::number_overflow);
    EG_REQUIRE_EQ(masked->outer, ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(masked->location.field, data::field_id(7521));
    const Chunk narrow_chunk = EG_REQUIRE_TAKEN(EG_REQUIRE_OK(narrow).finish(data::chunk_id(7522)));
    EG_REQUIRE_EQ(chunk_cell(narrow_chunk, 0).int_value().value_or(0), std::int64_t{127});
    EG_REQUIRE_EQ(chunk_cell(narrow_chunk, 1).int_value().value_or(0), std::int64_t{-128});
    EG_REQUIRE_EQ(chunk_cell(narrow_chunk, 1).bit_pattern().value_or(0),
                  std::uint64_t{0xFFFFFFFFFFFFFF80ULL});
    EG_REQUIRE_EQ(narrow_chunk.row_count(), std::uint64_t{2});

    auto wide = ChunkBuilder::create(data::field_id(7523), LogicalType::uint8);
    EG_REQUIRE(!EG_REQUIRE_OK(wide).append_bits(0xFFULL).has_value());
    EG_REQUIRE(EG_REQUIRE_OK(wide).append_bits(0x100ULL).has_value());
    auto flag = ChunkBuilder::create(data::field_id(7524), LogicalType::boolean);
    EG_REQUIRE(!EG_REQUIRE_OK(flag).append_bits(1ULL).has_value());
    EG_REQUIRE(EG_REQUIRE_OK(flag).append_bits(2ULL).has_value());
    auto single = ChunkBuilder::create(data::field_id(7525), LogicalType::float32);
    EG_REQUIRE(!EG_REQUIRE_OK(single).append_bits(0x7FC00000ULL).has_value());
    EG_REQUIRE(EG_REQUIRE_OK(single).append_bits(0x100000000ULL).has_value());
    auto words = ChunkBuilder::create(data::field_id(7526), LogicalType::text);
    const auto no_bit_form = EG_REQUIRE_OK(words).append_bits(1ULL);
    EG_REQUIRE(no_bit_form.has_value());
    EG_REQUIRE_EQ(no_bit_form->detail, DetailCode::type_mismatch);
}

EG_TEST("TYP", "007", empty_text_whitespace_missing_zero_nan_and_infinity_stay_distinct) {
    Fixture fixture;
    auto table = make_table(6);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(21), data::row_id(22), data::row_id(23),
                                 data::row_id(24), data::row_id(25), data::row_id(26),
                                 data::row_id(27)});
    ColumnSpec text_column;
    text_column.type = LogicalType::text;
    text_column.cells = {text_cell(""), text_cell("   "), Cell::missing(LogicalType::text),
                         text_cell("0"), text_cell("nan"), text_cell("inf"),
                         text_cell(" \t ")};
    ColumnLine text_line = create_column(fixture, table, text_column);

    ColumnSpec number_column;
    number_column.type = LogicalType::float64;
    number_column.cells = {float64_cell(0.0), Cell::missing(LogicalType::float64),
                           float64_cell(std::numeric_limits<double>::quiet_NaN()),
                           float64_cell(std::numeric_limits<double>::infinity()),
                           float64_cell(-std::numeric_limits<double>::infinity()),
                           float64_cell(1.0), Cell::missing(LogicalType::float64)};
    ColumnLine number_line = create_column(fixture, table, number_column);

    auto lease = fixture.lease(data::dataset_version_id(text_line.version));
    const FieldId text_field = data::field_id(text_line.field);
    EG_REQUIRE_EQ(lease.row_count(), std::uint64_t{7});
    const Cell empty_text = EG_REQUIRE_TAKEN(lease.cell_at(text_field, 0));
    const Cell whitespace = EG_REQUIRE_TAKEN(lease.cell_at(text_field, 1));
    const Cell missing = EG_REQUIRE_TAKEN(lease.cell_at(text_field, 2));
    EG_REQUIRE(!empty_text.is_missing());
    EG_REQUIRE_EQ(empty_text.text_value().value_or("x"), std::string_view{""});
    EG_REQUIRE_EQ(whitespace.text_value().value_or("x"), std::string{"   "});
    EG_REQUIRE(missing.is_missing());
    // No implicit trim, and whitespace never becomes missing.
    EG_REQUIRE(!data::structurally_equal(empty_text, whitespace));
    EG_REQUIRE(!data::structurally_equal(empty_text, missing));
    EG_REQUIRE(!data::structurally_equal(whitespace, missing));
    // A textual "0", "nan" and "inf" stay text and never merge with the numeric states.
    EG_REQUIRE(!data::structurally_equal(EG_REQUIRE_TAKEN(lease.cell_at(text_field, 3)),
                                         int_cell(0, LogicalType::int64)));
    EG_REQUIRE(!EG_REQUIRE_TAKEN(lease.cell_at(text_field, 4)).is_nan());

    auto numbers = fixture.lease(data::dataset_version_id(number_line.version));
    const FieldId number_field = data::field_id(number_line.field);
    const Cell zero = EG_REQUIRE_TAKEN(numbers.cell_at(number_field, 0));
    const Cell absent = EG_REQUIRE_TAKEN(numbers.cell_at(number_field, 1));
    const Cell not_a_number = EG_REQUIRE_TAKEN(numbers.cell_at(number_field, 2));
    const Cell positive = EG_REQUIRE_TAKEN(numbers.cell_at(number_field, 3));
    const Cell negative = EG_REQUIRE_TAKEN(numbers.cell_at(number_field, 4));
    EG_REQUIRE(zero.is_zero() && !zero.is_missing());
    EG_REQUIRE(absent.is_missing() && !absent.is_zero());
    EG_REQUIRE(not_a_number.is_nan() && !not_a_number.is_missing());
    EG_REQUIRE(positive.is_infinity() && !positive.is_nan());
    EG_REQUIRE(negative.is_infinity() && negative.sign_bit());
    for (const Cell* left : {&zero, &absent, &not_a_number, &positive, &negative}) {
        for (const Cell* right : {&zero, &absent, &not_a_number, &positive, &negative}) {
            if (left != right) {
                EG_REQUIRE(!data::structurally_equal(*left, *right));
            }
        }
    }

    // The same distinct states are still distinct after an edit published a new version.
    edit_column(fixture, table, text_line, text_column.cells);
    auto after = fixture.lease(data::dataset_version_id(text_line.version));
    EG_REQUIRE(!EG_REQUIRE_TAKEN(after.cell_at(text_field, 0)).is_missing());
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(after.cell_at(text_field, 1)).text_value().value_or("x"),
                  std::string{"   "});
    EG_REQUIRE(EG_REQUIRE_TAKEN(after.cell_at(text_field, 2)).is_missing());
}

EG_TEST("TYP", "008", unicode_text_and_labels_keep_their_code_points_without_normalizing) {
    // Escaped code points, as the plan requires where an editor could normalize the bytes.
    const std::string combining = "e\u0301";
    const std::string precomposed = "\u00E9";
    const std::string emoji = "summary \U0001F600 end";
    const std::string right_to_left = "\u200F\u0634\u0644\u0645";
    const std::string cjk_label = "\u4E2D\u6587";

    // e + U+0301 is 3 UTF-8 bytes and precomposed U+00E9 is 2: nothing is normalized away.
    EG_REQUIRE_EQ(data::validate_text(combining).value().size(), std::size_t{3});
    EG_REQUIRE_EQ(data::validate_text(precomposed).value().size(), std::size_t{2});
    EG_REQUIRE(!data::structurally_equal(text_cell(combining), text_cell(precomposed)));
    EG_REQUIRE(data::structurally_equal(text_cell(emoji), text_cell(emoji)));

    auto builder = ChunkBuilder::create(data::field_id(7601), LogicalType::text);
    for (const auto& value : {combining, precomposed, emoji, right_to_left, cjk_label}) {
        auto appended = EG_REQUIRE_OK(builder).append_text(value);
        EG_REQUIRE(!appended.has_value());
    }
    auto finished = EG_REQUIRE_OK(builder).finish(data::chunk_id(7602));
    auto chunk = std::make_shared<const Chunk>(EG_REQUIRE_TAKEN(std::move(finished)));
    const std::vector<std::string> stored = {combining, precomposed, emoji, right_to_left, cjk_label};
    for (std::size_t index = 0; index < stored.size(); ++index) {
        const Cell cell = chunk_cell(*chunk, index);
        EG_REQUIRE_EQ(cell.text_value().value_or(""), std::string_view{stored[index]});
    }

    // An embedded NUL follows the declared text policy and is rejected by default.
    auto rejecting = ChunkBuilder::create(data::field_id(7603), LogicalType::text);
    const std::string with_nul{"a\0b", 3};
    auto nul = EG_REQUIRE_OK(rejecting).append_text(with_nul);
    EG_REQUIRE(nul.has_value());
    EG_REQUIRE_DIAGNOSTIC(*nul, DetailCode::invalid_text, ErrorCode::invalid_argument,
                          Severity::error);
    EG_REQUIRE_EQ(nul->location.byte_offset.value_or(-1), std::int64_t{1});

    data::ChunkOptions allowing;
    allowing.text_policy = data::TextNulPolicy::allow;
    auto declared = ChunkBuilder::create(data::field_id(7604), LogicalType::text, allowing);
    EG_REQUIRE(!EG_REQUIRE_OK(declared).append_text(with_nul).has_value());
    auto allowing_finish = EG_REQUIRE_OK(declared).finish(data::chunk_id(7605));
    auto allowed = std::make_shared<const Chunk>(EG_REQUIRE_TAKEN(std::move(allowing_finish)));
    EG_REQUIRE_EQ(allowed->cell_at(0).value().text_value().value_or("").size(), std::size_t{3});
    EG_REQUIRE_EQ(EG_REQUIRE_OK(declared).row_count(), std::uint64_t{1});

    // Invalid UTF-8 is rejected at the byte offset of the sequence, and nothing is
    // substituted for it.
    for (const std::pair<std::string, std::int64_t>& bad : {
             std::pair{std::string{"ok\xE2\x82"}, std::int64_t{2}},
             std::pair{std::string{"ok\xC0\xAF"}, std::int64_t{2}},
             std::pair{std::string{"ok\xF5\x80\x80\x80"}, std::int64_t{2}},
             std::pair{std::string{"ok\xED\xA0\x80"}, std::int64_t{2}},
             std::pair{std::string{"\xF0\x9F\x92"}, std::int64_t{0}},
             std::pair{std::string{"\xE2\x9C\x85\x41\x80"}, std::int64_t{4}}}) {
        auto rejected = data::validate_text(bad.first);
        EG_REQUIRE(!rejected.ok());
        EG_REQUIRE_EQ(rejected.diagnostic().detail, DetailCode::invalid_text);
        EG_REQUIRE_EQ(rejected.diagnostic().location.byte_offset.value_or(-1), bad.second);
    }
    // A well-formed multi-byte string keeps its exact bytes, so no replacement character ever
    // enters storage.
    const std::string well_formed{"\xE2\x9C\x85 \xF0\x9F\x92\xA1"};
    const auto kept = data::validate_text(well_formed);
    EG_REQUIRE_OK(kept);
    EG_REQUIRE_EQ(kept.value(), well_formed);

    // A label is presentation only: a non-Latin label never changes the column identity.
    Fixture fixture;
    auto table = make_table(8);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(31)});
    ColumnSpec spec;
    spec.type = LogicalType::text;
    spec.label = cjk_label;
    spec.cells = {text_cell(emoji)};
    ColumnLine line = create_column(fixture, table, spec);
    const DocumentView view = fixture.document();
    const ColumnBindingRecord* record = view.find(data::column_id(line.column));
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE_EQ(record->label, cjk_label);
    EG_REQUIRE(record->id == data::column_id(line.column));
}

EG_TEST("TYP", "011", date_naive_and_zoned_values_keep_their_type_and_their_instant) {
    EG_REQUIRE(data::is_leap_year(2024));
    EG_REQUIRE(!data::is_leap_year(1900));
    EG_REQUIRE(data::is_leap_year(2000));
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::days_in_month(2024, 2)), std::uint32_t{29});
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::days_in_month(2023, 2)), std::uint32_t{28});

    const data::CivilDate leap{2024, 2, 29};
    const std::int64_t leap_ordinal = EG_REQUIRE_TAKEN(data::ordinal_days(leap));
    EG_REQUIRE_EQ(leap_ordinal,
                  EG_REQUIRE_TAKEN(data::ordinal_days(data::CivilDate{2024, 2, 28})) + 1);
    EG_REQUIRE_EQ(leap_ordinal,
                  EG_REQUIRE_TAKEN(data::ordinal_days(data::CivilDate{2024, 3, 1})) - 1);
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::date_from_ordinal(leap_ordinal)).day, std::uint32_t{29});
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::date_from_ordinal(leap_ordinal)).month, std::uint32_t{2});
    // The declared epoch: the day count is exact and a day outside the month is rejected.
    const std::int64_t epoch = EG_REQUIRE_TAKEN(data::ordinal_days(data::CivilDate{1970, 1, 1}));
    EG_REQUIRE_EQ(epoch, std::int64_t{0});
    EG_REQUIRE(!data::ordinal_days(data::CivilDate{2023, 2, 29}).ok());
    EG_REQUIRE_EQ(data::ordinal_days(data::CivilDate{2023, 2, 29}).diagnostic().detail,
                  DetailCode::date_format);
    const std::int64_t pre_epoch = EG_REQUIRE_TAKEN(
        data::ordinal_days(data::CivilDate{1969, 12, 31}));
    EG_REQUIRE_EQ(pre_epoch, epoch - 1);
    EG_REQUIRE(pre_epoch < epoch);
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::date_from_ordinal(pre_epoch)).year, std::int32_t{1969});

    // The tick unit is part of the type, so a day count is never read as an instant.
    EG_REQUIRE_EQ(data::tick_unit(LogicalType::date), data::TickUnit::day);
    EG_REQUIRE_EQ(data::tick_unit(LogicalType::timestamp_naive), data::TickUnit::microsecond);
    EG_REQUIRE_EQ(data::tick_unit(LogicalType::duration), data::TickUnit::microsecond);
    const Cell as_date = date_cell(leap_ordinal);
    const Cell as_naive = naive_cell(leap_ordinal);
    const Cell as_zoned = zoned_cell(leap_ordinal);
    EG_REQUIRE(!data::structurally_equal(as_date, as_naive));
    EG_REQUIRE(!data::structurally_equal(as_naive, as_zoned));
    EG_REQUIRE_EQ(data::to_string(as_date.type()), std::string{"date"});

    const data::CivilDateTime local_noon = civil(2024, 2, 29, 12, 30);
    const std::int64_t naive = EG_REQUIRE_TAKEN(data::naive_micros(local_noon));
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::civil_from_naive(naive)).date.day, std::uint32_t{29});
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(data::civil_from_naive(naive)).time.minute, std::uint32_t{30});

    const auto rules = data::TimeZoneRules::builtin();
    EG_REQUIRE_EQ(rules.version(), std::string{"EG-TZ-RULES-1"});
    const auto resolved = rules.resolve(local_noon, "Asia/Shanghai");
    EG_REQUIRE(resolved.ok());
    EG_REQUIRE_EQ(resolved.value().offset_micros, hours_micros(8));
    EG_REQUIRE_EQ(resolved.value().instant_micros, naive - hours_micros(8));
    EG_REQUIRE(!resolved.value().in_daylight_time);
    const auto observed = rules.observe(resolved.value().instant_micros, "Asia/Shanghai");
    EG_REQUIRE(observed.ok());
    EG_REQUIRE_EQ(observed.value().local.date.day, std::uint32_t{29});
    EG_REQUIRE_EQ(observed.value().offset_micros, hours_micros(8));
    // A half-hour zone is exact, not rounded to the nearest hour.
    EG_REQUIRE_EQ(rules.resolve(local_noon, "Asia/Kolkata").value().offset_micros,
                  hours_micros(5) + 30LL * 60LL * data::kMicrosPerSecond);
    // A date-only value stays a day count: the same ordinal read as a timestamp is a
    // different state class, so no zone can widen it into an instant.
    EG_REQUIRE(!data::structurally_equal(date_cell(epoch), naive_cell(epoch)));

    // The zone is part of the field declaration. A zoned field without one, a zone on any
    // other type and a zone outside the versioned table are each rejected naming the field.
    auto zoneless = DataStore::schema(
        {field(data::field_id(11901), "when", LogicalType::timestamp_zoned)});
    EG_REQUIRE(!zoneless.ok());
    EG_REQUIRE_EQ(zoneless.diagnostic().detail, DetailCode::unknown_zone);
    EG_REQUIRE(zoneless.diagnostic().location.field == data::field_id(11901));
    EG_REQUIRE_EQ(zoneless.diagnostic().outer, ErrorCode::invalid_argument);

    auto zoned_naive = DataStore::schema({field(data::field_id(11902), "when",
                                                LogicalType::timestamp_naive,
                                                nullptr, std::string{},
                                                UnknownCategoryPolicy::reject,
                                                std::string{"Europe/Berlin"})});
    EG_REQUIRE(!zoned_naive.ok());
    EG_REQUIRE_EQ(zoned_naive.diagnostic().detail, DetailCode::type_mismatch);
    EG_REQUIRE_EQ(zoned_naive.diagnostic().location.description, std::string{"Europe/Berlin"});

    auto unlisted_zone = DataStore::schema({field(data::field_id(11903), "when",
                                                  LogicalType::timestamp_zoned,
                                                  nullptr, std::string{},
                                                  UnknownCategoryPolicy::reject,
                                                  std::string{"Mars/Phobos"})});
    EG_REQUIRE(!unlisted_zone.ok());
    EG_REQUIRE_EQ(unlisted_zone.diagnostic().detail, DetailCode::unknown_zone);
    EG_REQUIRE_EQ(unlisted_zone.diagnostic().location.description, std::string{"Mars/Phobos"});

    // A declared zone enters the canonical schema text, so a later reader resolves the same
    // field and no other zone can be inferred from the stored instant.
    const std::string berlin{"Europe/Berlin"};
    const auto zoned_schema = schema_of({field(data::field_id(11904), "when",
                                               LogicalType::timestamp_zoned,
                                               nullptr, std::string{},
                                               UnknownCategoryPolicy::reject, berlin)});
    EG_REQUIRE_EQ(zoned_schema->find(data::field_id(11904))->zone_id.value_or(""), berlin);
    EG_REQUIRE(zoned_schema->canonical_text().find("zone=" + berlin) != std::string::npos);
    EG_REQUIRE(!data::structurally_equal(
        zoned_cell(resolved.value().instant_micros), naive_cell(resolved.value().instant_micros)));

    // Through the command path the declaration is what a reader of the published version sees.
    Fixture fixture;
    auto table = make_table(11);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(11001)});
    ColumnSpec zoned_spec;
    zoned_spec.type = LogicalType::timestamp_zoned;
    zoned_spec.zone = berlin;
    zoned_spec.cells = {zoned_cell(resolved.value().instant_micros)};
    const ColumnLine zoned_line = create_column(fixture, table, zoned_spec);
    const auto published = fixture.version(data::dataset_version_id(zoned_line.version));
    EG_REQUIRE(published->schema()->canonical_text().find("zone=" + berlin) != std::string::npos);
    EG_REQUIRE_EQ(published->schema()->fields.front().zone_id.value_or(""), berlin);
    const Cell stored = EG_REQUIRE_TAKEN(published->cell(data::field_id(zoned_line.field), 0));
    EG_REQUIRE(data::structurally_equal(stored, zoned_spec.cells.front()));
    EG_REQUIRE_EQ(stored.tick_value().value_or(0), resolved.value().instant_micros);
}

EG_TEST("TYP", "012", dst_gap_and_overlap_need_a_declared_resolution_with_the_zone) {
    const auto rules = data::TimeZoneRules::builtin();

    // New York springs forward at 2026-03-08 02:00, so 02:30 does not exist.
    const data::CivilDateTime gap = civil(2026, 3, 8, 2, 30);
    const auto rejected_gap = rules.resolve(gap, "America/New_York");
    EG_REQUIRE(!rejected_gap.ok());
    EG_REQUIRE_DIAGNOSTIC(rejected_gap.diagnostic(), DetailCode::date_nonexistent,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE(rejected_gap.diagnostic().location.description.find("America/New_York") !=
               std::string::npos);
    const auto before = rules.resolve(gap, "America/New_York", data::LocalTimeResolution::pre_transition);
    const auto after = rules.resolve(gap, "America/New_York", data::LocalTimeResolution::post_transition);
    EG_REQUIRE(before.ok() && after.ok());
    EG_REQUIRE_EQ(before.value().offset_micros, hours_micros(-5));
    EG_REQUIRE_EQ(after.value().offset_micros, hours_micros(-4));
    EG_REQUIRE_EQ(after.value().instant_micros, before.value().instant_micros - hours_micros(1));
    EG_REQUIRE(!before.value().in_daylight_time);
    EG_REQUIRE(after.value().in_daylight_time);

    // Berlin falls back at 2026-10-25 03:00 daylight time, so 02:30 happens twice.
    const data::CivilDateTime overlap = civil(2026, 10, 25, 2, 30);
    const auto rejected_overlap = rules.resolve(overlap, "Europe/Berlin");
    EG_REQUIRE(!rejected_overlap.ok());
    EG_REQUIRE_DIAGNOSTIC(rejected_overlap.diagnostic(), DetailCode::date_ambiguous,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE(rejected_overlap.diagnostic().message.find("twice") != std::string::npos);
    const auto first_pass =
        rules.resolve(overlap, "Europe/Berlin", data::LocalTimeResolution::pre_transition);
    const auto second_pass =
        rules.resolve(overlap, "Europe/Berlin", data::LocalTimeResolution::post_transition);
    EG_REQUIRE_EQ(first_pass.value().offset_micros, hours_micros(2));
    EG_REQUIRE_EQ(second_pass.value().offset_micros, hours_micros(1));
    EG_REQUIRE(first_pass.value().in_daylight_time);
    EG_REQUIRE(!second_pass.value().in_daylight_time);
    EG_REQUIRE_EQ(first_pass.value().instant_micros + 3600LL * data::kMicrosPerSecond,
                  second_pass.value().instant_micros);

    // A southern-hemisphere zone resolves with the same declared rule.
    const auto sydney_standard = rules.resolve(civil(2026, 4, 5, 4, 30), "Australia/Sydney");
    EG_REQUIRE(sydney_standard.ok());
    EG_REQUIRE_EQ(sydney_standard.value().offset_micros, hours_micros(10));
    // Sydney's fall-back overlap is the same declared rule in the southern hemisphere.
    const auto sydney_overlap = rules.resolve(civil(2026, 4, 5, 2, 30), "Australia/Sydney");
    EG_REQUIRE(!sydney_overlap.ok());
    EG_REQUIRE_EQ(sydney_overlap.diagnostic().detail, DetailCode::date_ambiguous);
    const auto sydney_gap = rules.resolve(civil(2026, 10, 4, 2, 30), "Australia/Sydney");
    EG_REQUIRE(!sydney_gap.ok());
    EG_REQUIRE_EQ(sydney_gap.diagnostic().detail, DetailCode::date_nonexistent);

    // An unlisted zone identifier is rejected rather than treated as UTC.
    const auto unknown = rules.resolve(civil(2026, 6, 1, 12, 0), "Europe/Paris");
    EG_REQUIRE(!unknown.ok());
    EG_REQUIRE_DIAGNOSTIC(unknown.diagnostic(), DetailCode::unknown_zone,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE(!rules.contains("Europe/Paris"));
    EG_REQUIRE(rules.contains("UTC"));
}

EG_TEST("TYP", "014", categorical_codes_are_stable_and_an_unseen_category_is_declared) {
    const auto labels = dictionary_of({"control", "treated", "follow-up"});
    EG_REQUIRE_EQ(labels->code_for("control").value(), std::int64_t{0});
    EG_REQUIRE_EQ(labels->code_for("follow-up").value(), std::int64_t{2});
    EG_REQUIRE_EQ(labels->label_for(1).value(), std::string{"treated"});
    EG_REQUIRE(!labels->code_for("other").ok());

    FieldSchema declared = field(data::field_id(7701), "arm", LogicalType::categorical, labels);
    auto rejecting = data::DatasetSchema::create({declared});
    EG_REQUIRE_OK(rejecting);
    auto missing_policy = declared;
    missing_policy.unknown_category = UnknownCategoryPolicy::missing;
    const Cell unseen = EG_REQUIRE_TAKEN(missing_policy.resolve_category("other"));
    EG_REQUIRE(unseen.is_missing());
    // A missing category is never silently mapped onto the first label.
    EG_REQUIRE(!data::structurally_equal(unseen, category_cell(0)));

    const auto unseen_rejected = declared.resolve_category("other");
    EG_REQUIRE(!unseen_rejected.ok());
    EG_REQUIRE_DIAGNOSTIC(unseen_rejected.diagnostic(), DetailCode::unknown_category,
                          ErrorCode::invalid_argument, Severity::error);
    EG_REQUIRE_EQ(unseen_rejected.diagnostic().location.field.value_or(data::FieldId{}),
                  data::field_id(7701));
    EG_REQUIRE_EQ(declared.resolve_category("treated").value().category_code().value_or(-1),
                  std::int64_t{1});

    // A categorical field without a dictionary is rejected at declaration.
    auto bare = data::DatasetSchema::create({field(data::field_id(7702), "arm",
                                                   LogicalType::categorical)});
    EG_REQUIRE(!bare.ok());
    EG_REQUIRE_EQ(bare.diagnostic().detail, DetailCode::unknown_category);

    // Duplicate dictionary labels are rejected rather than resolved to the first match.
    auto duplicate = Dictionary::create({"control", "control"});
    EG_REQUIRE(!duplicate.ok());
    EG_REQUIRE_EQ(duplicate.diagnostic().detail, DetailCode::duplicate_id);

    // Codes stay stable across a published version and its successor.
    Fixture fixture;
    auto table = make_table(9);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(41), data::row_id(42), data::row_id(43)});
    ColumnSpec spec;
    spec.type = LogicalType::categorical;
    spec.dictionary = labels;
    spec.cells = {category_cell(0), category_cell(1), category_cell(2)};
    ColumnLine line = create_column(fixture, table, spec);
    auto lease = fixture.lease(data::dataset_version_id(line.version));
    for (std::size_t index = 0; index < 3; ++index) {
        EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(lease.cell_at(data::field_id(line.field), index))
                          .category_code()
                          .value_or(-1),
                      static_cast<std::int64_t>(index));
    }
    edit_column(fixture, table, line, {category_cell(2), category_cell(1), category_cell(0)});
    auto after = fixture.lease(data::dataset_version_id(line.version));
    EG_REQUIRE_EQ(EG_REQUIRE_TAKEN(after.cell_at(data::field_id(line.field), 0)).category_code()
                      .value_or(-1),
                  std::int64_t{2});
    auto negative = ChunkBuilder::create(data::field_id(7703), LogicalType::categorical);
    auto negative_code = EG_REQUIRE_OK(negative).append_category_code(-1);
    EG_REQUIRE(negative_code.has_value());
    EG_REQUIRE_EQ(negative_code->detail, DetailCode::unknown_category);
}

EG_TEST("TYP", "016", empty_all_missing_and_all_nan_tables_are_exact_and_invent_nothing) {
    const FieldId column_field = data::field_id(7801);
    auto schema = schema_of({field(column_field, "value", LogicalType::float64)});

    // A table with a declared schema and no rows is a real shape, not a failure.
    auto empty = data::DatasetVersion::create(
        data::dataset_version_id(7802), data::dataset_id(7803), schema, data::row_domain_id(7804),
        {}, provenance_of("data.column_create"), 0, 0);
    EG_REQUIRE(empty.ok());
    EG_REQUIRE_EQ(empty.value()->row_count(), std::uint64_t{0});
    EG_REQUIRE(empty.value()->chunk_ids().empty());
    EG_REQUIRE(!empty.value()->cell(column_field, 0).ok());
    EG_REQUIRE_EQ(empty.value()->cell(column_field, 0).diagnostic().detail,
                  DetailCode::index_out_of_range);

    std::vector<Cell> all_missing(8, Cell::missing(LogicalType::float64));
    auto missing_chunk = chunk_of(column_field, LogicalType::float64, data::chunk_id(7805),
                                 all_missing);
    EG_REQUIRE_EQ(missing_chunk->row_count(), std::uint64_t{8});
    for (std::size_t index = 0; index < 8; ++index) {
        EG_REQUIRE(chunk_cell(*missing_chunk, index).is_missing());
    }

    std::vector<Cell> all_nan(8, Cell::from_bits(LogicalType::float64, 0x7FF8000000000007ULL));
    auto nan_chunk = chunk_of(column_field, LogicalType::float64, data::chunk_id(7806), all_nan);
    for (std::size_t index = 0; index < 8; ++index) {
        EG_REQUIRE(chunk_cell(*nan_chunk, index).is_nan());
        EG_REQUIRE(!chunk_cell(*nan_chunk, index).is_missing());
    }
    auto nan_version = data::DatasetVersion::create(
        data::dataset_version_id(7807), data::dataset_id(7803), schema, data::row_domain_id(7804),
        {nan_chunk}, provenance_of("data.column_create"), 8, 0);
    EG_REQUIRE_OK(nan_version);
    EG_REQUIRE_EQ(nan_version.value()->row_count(), std::uint64_t{8});

    // Coverage must be exact: an empty chunk cannot cover rows, and a declared field with no
    // chunk for a non-empty version is rejected instead of read as missing.
    auto gap = chunk_of(column_field, LogicalType::float64, data::chunk_id(7808),
                       {float64_cell(1.0)}, 1);
    auto uncovered = data::DatasetVersion::create(
        data::dataset_version_id(7809), data::dataset_id(7803), schema, data::row_domain_id(7804),
        {gap}, provenance_of("data.column_create"), 2, 0);
    EG_REQUIRE(!uncovered.ok());
    EG_REQUIRE_EQ(uncovered.diagnostic().detail, DetailCode::unmapped_row);

    auto no_chunks = data::DatasetVersion::create(
        data::dataset_version_id(7810), data::dataset_id(7803), schema, data::row_domain_id(7804),
        {}, provenance_of("data.column_create"), 3, 0);
    EG_REQUIRE(!no_chunks.ok());
    EG_REQUIRE_EQ(no_chunks.diagnostic().detail, DetailCode::unmapped_row);

    auto mismatched = chunk_of(column_field, LogicalType::float32, data::chunk_id(7811),
                              {float32_cell(1.0F)});
    auto wrong_type = data::DatasetVersion::create(
        data::dataset_version_id(7812), data::dataset_id(7803), schema, data::row_domain_id(7804),
        {mismatched}, provenance_of("data.column_create"), 1, 0);
    EG_REQUIRE(!wrong_type.ok());
    EG_REQUIRE_EQ(wrong_type.diagnostic().detail, DetailCode::type_mismatch);

    // An empty worksheet with no columns is a legal document state.
    Fixture fixture;
    auto table = make_table(10);
    create_project_chain(fixture, table);
    const DocumentView view = fixture.document();
    EG_REQUIRE(view.find(data::worksheet_id(table.worksheet)) != nullptr);
    EG_REQUIRE(view.bindings_of(data::worksheet_id(table.worksheet)).empty());
    auto schemaless = data::DatasetSchema::create({});
    EG_REQUIRE_OK(schemaless);
    EG_REQUIRE(schemaless.value().fields.empty());
}

EG_TEST("TYP", "017", extreme_magnitudes_keep_ieee_behavior_without_clipping) {
    const double largest = std::numeric_limits<double>::max();
    const double overflowed = largest * 1e308;
    const double smallest = std::numeric_limits<double>::min();
    const double underflowed = smallest * 1e-308;
    const float largest_single = std::numeric_limits<float>::max();
    const double promoted_overflow = static_cast<double>(largest_single) * 1e300;

    EG_REQUIRE(std::isinf(overflowed));
    auto chunk = chunk_of(data::field_id(7901), LogicalType::float64, data::chunk_id(7902),
                          {float64_cell(largest), float64_cell(overflowed),
                           float64_cell(-overflowed), float64_cell(smallest),
                           float64_cell(underflowed),
                           float64_cell(std::numeric_limits<double>::denorm_min())});
    EG_REQUIRE(chunk_cell(*chunk, 1).is_infinity());
    EG_REQUIRE(!chunk_cell(*chunk, 1).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 2).is_infinity());
    EG_REQUIRE(chunk_cell(*chunk, 2).sign_bit());
    EG_REQUIRE(chunk_cell(*chunk, 0).bit_pattern().value_or(0) !=
               chunk_cell(*chunk, 1).bit_pattern().value_or(0));
    EG_REQUIRE(!chunk_cell(*chunk, 4).is_missing());
    EG_REQUIRE_EQ(chunk_cell(*chunk, 4).bit_pattern().value_or(1), std::uint64_t{0});
    EG_REQUIRE(chunk_cell(*chunk, 4).is_zero());

    auto single = chunk_of(data::field_id(7903), LogicalType::float32, data::chunk_id(7904),
                           {float32_cell(largest_single),
                            float32_cell(static_cast<float>(promoted_overflow)),
                            float32_cell(std::numeric_limits<float>::denorm_min())});
    EG_REQUIRE(chunk_cell(*single, 1).is_infinity());
    EG_REQUIRE_EQ(chunk_cell(*single, 0).bit_pattern().value_or(0),
                  static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(largest_single)));
    // The overflow is stored as the IEEE infinity it produced; it is never clipped back to the
    // largest finite value and no silent narrowing happens at the storage boundary.
    EG_REQUIRE(std::isinf(static_cast<double>(promoted_overflow)));
    // Storage itself is lossless, so it reports no overflow diagnostic: the value it was given
    // is the value it keeps. A producer that computes an overflow gets the IEEE result.
    auto builder = ChunkBuilder::create(data::field_id(7905), LogicalType::float64);
    EG_REQUIRE(!EG_REQUIRE_OK(builder).append_float64(overflowed).has_value());
    EG_REQUIRE(!EG_REQUIRE_OK(builder).append_float64(underflowed).has_value());
}

EG_TEST("TYP", "018", rename_reorder_and_display_format_never_touch_values_units_or_ids) {
    Fixture fixture;
    auto table = make_table(5);
    create_project_chain(fixture, table);
    append_rows(fixture, table, {data::row_id(51), data::row_id(52), data::row_id(53)});

    ColumnSpec first;
    first.type = LogicalType::float64;
    first.label = "concentration";
    first.unit = "kg/m3";
    first.role = "y";
    first.cells = {float64_cell(1.25), float64_cell(-2.5), float64_cell(0.0)};
    ColumnLine first_line = create_column(fixture, table, first);

    ColumnSpec second;
    second.type = LogicalType::float64;
    second.label = "time";
    second.unit = "s";
    second.role = "x";
    second.order_index = 1;
    second.cells = {float64_cell(0.0), float64_cell(0.5), float64_cell(1.0)};
    ColumnLine second_line = create_column(fixture, table, second);

    const auto before = fixture.lease(data::dataset_version_id(first_line.version));
    std::vector<Cell> before_cells;
    for (std::size_t index = 0; index < before.row_count(); ++index) {
        before_cells.push_back(
            EG_REQUIRE_TAKEN(before.cell_at(data::field_id(first_line.field), index)));
    }

    ParameterMap rename = Fixture::with(data::argument::kColumn, first_line.column);
    Fixture::put(rename, data::argument::kLabel, std::string{"c A \u00B7 mg"});
    Fixture::put(rename, data::argument::kOrderIndex, std::uint64_t{7});
    Fixture::put(rename, data::argument::kDisplayFormat, std::string{"0.000"});
    (void)fixture.require_run(data::command_id::kColumnRename, std::move(rename),
                              fixture.declared_writes({first_line.column}));

    ParameterMap metadata = Fixture::with(data::argument::kColumn, first_line.column);
    Fixture::put(metadata, data::argument::kRole, std::string{"z"});
    (void)fixture.require_run(data::command_id::kColumnMetadata, std::move(metadata),
                              fixture.declared_writes({first_line.column}));

    const DocumentView view = fixture.document();
    const ColumnBindingRecord* record = view.find(data::column_id(first_line.column));
    EG_REQUIRE(record != nullptr);
    EG_REQUIRE_EQ(record->label, std::string{"c A \u00B7 mg"});
    EG_REQUIRE_EQ(record->order_index, std::uint64_t{7});
    EG_REQUIRE_EQ(record->display_format, std::string{"0.000"});
    EG_REQUIRE_EQ(record->role, data::ColumnRole::z);
    // The unit and the values belong to the column identity, not to its label or position.
    const auto declared_unit = data::UnitRegistry::builtin().parse("kg/m3");
    EG_REQUIRE(declared_unit.ok());
    EG_REQUIRE(record->unit.has_value());
    EG_REQUIRE(*record->unit == declared_unit.value());
    EG_REQUIRE(record->version == data::dataset_version_id(first_line.version));
    EG_REQUIRE(record->dataset == data::dataset_id(first_line.dataset));

    auto after = fixture.lease(data::dataset_version_id(record->version.raw()));
    EG_REQUIRE_EQ(after.row_count(), before.row_count());
    for (std::size_t index = 0; index < before_cells.size(); ++index) {
        EG_REQUIRE_CELL_EQ(EG_REQUIRE_TAKEN(after.cell_at(data::field_id(first_line.field), index)),
                           before_cells[index]);
    }
    const auto ordered = view.bindings_of(data::worksheet_id(table.worksheet));
    EG_REQUIRE_EQ(ordered.size(), std::size_t{2});
    EG_REQUIRE(ordered.front().id == data::column_id(second_line.column));
    // The in-memory half only: saving and reopening the same identities is P2-006's fixture.
}
