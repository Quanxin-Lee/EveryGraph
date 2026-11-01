#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/data/diagnostics.hpp"

namespace everygraph::data {

enum class DimensionBase {
    length,
    mass,
    time,
    current,
    temperature,
    substance,
    luminous_intensity,
    angle,
};

inline constexpr std::size_t kDimensionBaseCount = 8;

// A dimension is the vector of base exponents. Combining and cancelling dimensions is what
// makes m/s and kg/m3 decidable without string matching.
class Dimension {
public:
    constexpr Dimension() = default;

    [[nodiscard]] static constexpr Dimension base(DimensionBase which,
                                                  std::int16_t exponent = 1) noexcept {
        Dimension dimension;
        dimension.exponents_[static_cast<std::size_t>(which)] = exponent;
        return dimension;
    }

    [[nodiscard]] constexpr std::int16_t exponent(DimensionBase which) const noexcept {
        return exponents_[static_cast<std::size_t>(which)];
    }

    [[nodiscard]] Dimension multiplied_by(const Dimension& other) const noexcept;
    [[nodiscard]] Dimension divided_by(const Dimension& other) const noexcept;
    [[nodiscard]] Result<Dimension> raised_to(std::int16_t power) const noexcept;
    [[nodiscard]] constexpr bool is_dimensionless() const noexcept {
        for (const std::int16_t value : exponents_) {
            if (value != 0) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::string describe() const;

    friend constexpr bool operator==(const Dimension&, const Dimension&) noexcept = default;

private:
    std::array<std::int16_t, kDimensionBaseCount> exponents_{};
};

// `absolute` names a scale with an offset from a real zero, such as degC. `difference` names
// a change in that same scale, such as delta_degC, which has no offset and is therefore
// algebraically ordinary. Keeping them apart is what stops an absolute temperature from being
// added to another absolute temperature as if it were a difference.
enum class UnitKind {
    multiplicative,
    absolute,
    difference,
};

[[nodiscard]] const char* to_string(UnitKind kind) noexcept;

// Scale as an exact rational times an optional irrational factor, so a length conversion is
// bit-exact rather than 99.99999999999998, while an angle conversion keeps a declared
// tolerance instead of inventing an exact pi.
class Scale {
public:
    Scale() = default;
    Scale(std::int64_t numerator, std::int64_t denominator, double irrational = 1.0) noexcept
        : numerator_(numerator), denominator_(denominator), irrational_(irrational) {}

    [[nodiscard]] std::int64_t numerator() const noexcept { return numerator_; }
    [[nodiscard]] std::int64_t denominator() const noexcept { return denominator_; }
    [[nodiscard]] double irrational() const noexcept { return irrational_; }
    [[nodiscard]] bool is_exact() const noexcept { return irrational_ == 1.0; }
    [[nodiscard]] double factor() const noexcept;

    [[nodiscard]] Result<Scale> multiplied_by(const Scale& other) const noexcept;
    [[nodiscard]] Result<Scale> divided_by(const Scale& other) const noexcept;
    [[nodiscard]] Result<Scale> raised_to(std::int16_t power) const noexcept;

    friend bool operator==(const Scale& left, const Scale& right) noexcept {
        return left.numerator_ == right.numerator_ && left.denominator_ == right.denominator_ &&
               left.irrational_ == right.irrational_;
    }

private:
    std::int64_t numerator_{1};
    std::int64_t denominator_{1};
    double irrational_{1.0};
};

// A unit in one declared form. Conversion to the base scale is `value * factor + offset`.
class Unit {
public:
    Unit() = default;
    Unit(UnitKind kind, Dimension dimension, Scale scale, double offset,
         std::string source_text, std::string canonical_text)
        : kind_(kind),
          dimension_(dimension),
          scale_(std::move(scale)),
          offset_(offset),
          source_text_(std::move(source_text)),
          canonical_text_(std::move(canonical_text)) {}

    [[nodiscard]] UnitKind kind() const noexcept { return kind_; }
    [[nodiscard]] const Dimension& dimension() const noexcept { return dimension_; }
    [[nodiscard]] const Scale& scale() const noexcept { return scale_; }
    [[nodiscard]] double offset() const noexcept { return offset_; }
    [[nodiscard]] const std::string& source_text() const noexcept { return source_text_; }
    [[nodiscard]] const std::string& canonical_text() const noexcept { return canonical_text_; }
    [[nodiscard]] bool is_absolute() const noexcept { return kind_ == UnitKind::absolute; }
    [[nodiscard]] bool is_dimensionless() const noexcept { return dimension_.is_dimensionless(); }

    // Semantic equality: two spellings of the same scale compare equal, and the source text
    // a caller typed is not part of the identity.
    friend bool operator==(const Unit& left, const Unit& right) noexcept {
        return left.kind_ == right.kind_ && left.dimension_ == right.dimension_ &&
               left.scale_ == right.scale_ && left.offset_ == right.offset_;
    }

private:
    UnitKind kind_{UnitKind::multiplicative};
    Dimension dimension_;
    Scale scale_;
    double offset_{0.0};
    std::string source_text_;
    std::string canonical_text_;
};

struct Quantity {
    double value{0.0};
    Unit unit;
};

// The declared unit registry: the atom table, the parser for compound spellings and the
// policy that `data.offset_unit_arithmetic` rejects rather than invents a difference reading.
class UnitRegistry {
public:
    struct Atom {
        std::string name;
        UnitKind kind{UnitKind::multiplicative};
        Dimension dimension;
        Scale scale;
        double offset{0.0};
    };

    [[nodiscard]] static const UnitRegistry& builtin();

    [[nodiscard]] const std::string& version() const noexcept { return version_; }
    [[nodiscard]] Result<Unit> parse(std::string_view source_text) const;
    [[nodiscard]] std::optional<Atom> atom(std::string_view name) const;
    [[nodiscard]] std::vector<std::string> atom_names() const;

private:
    UnitRegistry(std::string version, std::vector<Atom> atoms);

    [[nodiscard]] Result<Unit> parse_atom(std::string_view token) const;

    std::string version_;
    std::vector<Atom> atoms_;
};

// Conversion is explicit and never changes a stored value in place. A target with a
// different dimension, or an absolute scale crossed with a scale that has no real zero, is
// rejected naming both units.
[[nodiscard]] Result<Quantity> convert(const Quantity& source, const Unit& target);

[[nodiscard]] Result<Quantity> add(const Quantity& left, const Quantity& right);
[[nodiscard]] Result<Quantity> subtract(const Quantity& left, const Quantity& right);
[[nodiscard]] Result<Quantity> multiply(const Quantity& left, const Quantity& right);
[[nodiscard]] Result<Quantity> divide(const Quantity& left, const Quantity& right);

// The declared way to make an absolute scale ordinary: name the difference unit instead of
// reading an absolute value as a scalar.
[[nodiscard]] Result<Unit> as_difference_unit(const Unit& unit);

// An empty unit text is not a unit. Dimensionless is a declared form, so a caller asks for
// it by name and never gets it by omission.
[[nodiscard]] Unit dimensionless_unit();

}  // namespace everygraph::data
