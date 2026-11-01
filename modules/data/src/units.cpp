#include "everygraph/data/units.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace everygraph::data {
namespace {

constexpr std::int64_t kInt64Max = std::numeric_limits<std::int64_t>::max();
constexpr std::int16_t kMaxExponent = 8;

constexpr DimensionBase kOrderedBases[kDimensionBaseCount] = {
    DimensionBase::length,  DimensionBase::mass,    DimensionBase::time,   DimensionBase::current,
    DimensionBase::temperature, DimensionBase::substance, DimensionBase::luminous_intensity,
    DimensionBase::angle};

const char* base_name(DimensionBase which) noexcept {
    switch (which) {
        case DimensionBase::length:
            return "m";
        case DimensionBase::mass:
            return "kg";
        case DimensionBase::time:
            return "s";
        case DimensionBase::current:
            return "A";
        case DimensionBase::temperature:
            return "K";
        case DimensionBase::substance:
            return "mol";
        case DimensionBase::luminous_intensity:
            return "cd";
        case DimensionBase::angle:
            return "rad";
    }
    return "?";
}

std::int64_t absolute_value(std::int64_t value) noexcept {
    return value < 0 ? -value : value;
}

std::int64_t greatest_common_divisor(std::int64_t a, std::int64_t b) noexcept {
    a = absolute_value(a);
    b = absolute_value(b);
    while (b != 0) {
        const std::int64_t rest = a % b;
        a = b;
        b = rest;
    }
    return a == 0 ? 1 : a;
}

// Scale ratios stay positive integers so that a conversion can be carried out as one
// multiply and one divide, which is what keeps m to cm exact.
[[nodiscard]] std::optional<std::int64_t> checked_product(std::int64_t a, std::int64_t b) noexcept {
    if (a <= 0 || b <= 0) {
        return std::nullopt;
    }
    if (a > kInt64Max / b) {
        return std::nullopt;
    }
    return a * b;
}

Diagnostic unit_diagnostic(DetailCode code, std::string message, std::string location_text) {
    Location location;
    location.description = std::move(location_text);
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

std::string exponent_suffix(std::int16_t exponent) {
    if (exponent == 1) {
        return {};
    }
    return std::to_string(absolute_value(exponent));
}

std::string canonical_from_dimension(const Dimension& dimension) {
    if (dimension.is_dimensionless()) {
        return "1";
    }
    std::string numerator;
    std::string denominator;
    for (const DimensionBase which : kOrderedBases) {
        const std::int16_t exponent = dimension.exponent(which);
        if (exponent == 0) {
            continue;
        }
        std::string term = base_name(which);
        term += exponent_suffix(exponent);
        std::string& bucket = exponent > 0 ? numerator : denominator;
        if (!bucket.empty()) {
            bucket += "/";
        }
        bucket += term;
    }
    if (denominator.empty()) {
        return numerator;
    }
    if (numerator.empty()) {
        return "1/" + denominator;
    }
    return numerator + "/" + denominator;
}

// A scale as an exact rational where it can be one, plus the irrational factor an angle
// needs. `irrational != 1` is the only case where a conversion is not exact.
[[nodiscard]] long double to_base(double value, const Scale& scale, double offset) noexcept {
    const long double scaled =
        (static_cast<long double>(value) * static_cast<long double>(scale.numerator())) /
        static_cast<long double>(scale.denominator());
    // A declared irrational factor is part of the scale, so an angle quantity that is added,
    // multiplied or divided through the base representation carries it rather than losing it.
    const long double irrational = static_cast<long double>(scale.irrational());
    const long double factor = irrational == 1.0L ? 1.0L : irrational;
    return scaled * factor + static_cast<long double>(offset);
}

[[nodiscard]] double from_base(long double base_value, const Scale& scale, double offset) noexcept {
    const long double adjusted = base_value - static_cast<long double>(offset);
    const long double irrational = static_cast<long double>(scale.irrational());
    const long double unscaled = irrational == 1.0L ? adjusted : adjusted / irrational;
    const long double result =
        (unscaled * static_cast<long double>(scale.denominator())) /
        static_cast<long double>(scale.numerator());
    return static_cast<double>(result);
}

[[nodiscard]] double ratio_to(const Scale& from, const Scale& to, double value) noexcept {
    const long double numerator =
        static_cast<long double>(from.numerator()) * static_cast<long double>(to.denominator());
    const long double denominator =
        static_cast<long double>(to.numerator()) * static_cast<long double>(from.denominator());
    const long double scaled = (static_cast<long double>(value) * numerator) / denominator;
    return static_cast<double>(scaled * (from.irrational() / to.irrational()));
}

Unit make_unit(UnitKind kind, Dimension dimension, Scale scale, double offset,
               std::string source_text) {
    std::string canonical = canonical_from_dimension(dimension);
    return Unit{kind, dimension, std::move(scale), offset, std::move(source_text),
                std::move(canonical)};
}

Result<Scale> normalized_scale(std::int64_t numerator, std::int64_t denominator,
                               double irrational) {
    if (numerator <= 0 || denominator <= 0) {
        return Result<Scale>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "unit scale must be a positive rational", ""));
    }
    const std::int64_t divisor = greatest_common_divisor(numerator, denominator);
    return Scale{numerator / divisor, denominator / divisor, irrational};
}

}  // namespace

Dimension Dimension::multiplied_by(const Dimension& other) const noexcept {
    Dimension result;
    for (std::size_t index = 0; index < kDimensionBaseCount; ++index) {
        result.exponents_[index] = static_cast<std::int16_t>(exponents_[index] + other.exponents_[index]);
    }
    return result;
}

Dimension Dimension::divided_by(const Dimension& other) const noexcept {
    Dimension result;
    for (std::size_t index = 0; index < kDimensionBaseCount; ++index) {
        result.exponents_[index] = static_cast<std::int16_t>(exponents_[index] - other.exponents_[index]);
    }
    return result;
}

Result<Dimension> Dimension::raised_to(std::int16_t power) const noexcept {
    if (power < -kMaxExponent || power > kMaxExponent) {
        return Result<Dimension>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "unit exponent is outside the supported range",
            std::to_string(power)));
    }
    Dimension result;
    for (std::size_t index = 0; index < kDimensionBaseCount; ++index) {
        const std::int32_t product = static_cast<std::int32_t>(exponents_[index]) * power;
        if (product < -kMaxExponent || product > kMaxExponent) {
            return Result<Dimension>::failure(unit_diagnostic(
                DetailCode::unknown_unit, "compound dimension exponent is out of range", ""));
        }
        result.exponents_[index] = static_cast<std::int16_t>(product);
    }
    return result;
}

std::string Dimension::describe() const {
    std::string text;
    for (const DimensionBase which : kOrderedBases) {
        const std::int16_t exponent = this->exponent(which);
        if (exponent == 0) {
            continue;
        }
        if (!text.empty()) {
            text += ".";
        }
        text += base_name(which);
        text += "^";
        text += std::to_string(exponent);
    }
    return text.empty() ? "dimensionless" : text;
}

const char* to_string(UnitKind kind) noexcept {
    switch (kind) {
        case UnitKind::multiplicative:
            return "multiplicative";
        case UnitKind::absolute:
            return "absolute";
        case UnitKind::difference:
            return "difference";
    }
    return "unknown";
}

double Scale::factor() const noexcept {
    return static_cast<double>(numerator_) / static_cast<double>(denominator_) * irrational_;
}

Result<Scale> Scale::multiplied_by(const Scale& other) const noexcept {
    const auto numerator = checked_product(numerator_, other.numerator_);
    const auto denominator = checked_product(denominator_, other.denominator_);
    if (!numerator.has_value() || !denominator.has_value()) {
        return Result<Scale>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "compound unit scale exceeds the supported range", ""));
    }
    return normalized_scale(numerator.value(), denominator.value(), irrational_ * other.irrational_);
}

Result<Scale> Scale::divided_by(const Scale& other) const noexcept {
    const auto numerator = checked_product(numerator_, other.denominator_);
    const auto denominator = checked_product(denominator_, other.numerator_);
    if (!numerator.has_value() || !denominator.has_value()) {
        return Result<Scale>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "compound unit scale exceeds the supported range", ""));
    }
    return normalized_scale(numerator.value(), denominator.value(), irrational_ / other.irrational_);
}

Result<Scale> Scale::raised_to(std::int16_t power) const noexcept {
    if (power == 0) {
        return Scale{1, 1, 1.0};
    }
    const std::int16_t magnitude = static_cast<std::int16_t>(absolute_value(power));
    Scale current{1, 1, 1.0};
    const Scale base = power > 0 ? *this : Scale{denominator_, numerator_, irrational_ == 1.0 ? 1.0 : 1.0 / irrational_};
    for (std::int16_t index = 0; index < magnitude; ++index) {
        auto product = current.multiplied_by(base);
        if (!product.ok()) {
            return Result<Scale>::failure(product.diagnostic_copy());
        }
        current = product.take();
    }
    return current;
}

UnitRegistry::UnitRegistry(std::string version, std::vector<Atom> atoms)
    : version_(std::move(version)), atoms_(std::move(atoms)) {}

namespace {

UnitRegistry::Atom make_atom(std::string name, UnitKind kind, Dimension dimension, Scale scale, double offset) {
    UnitRegistry::Atom atom;
    atom.name = std::move(name);
    atom.kind = kind;
    atom.dimension = dimension;
    atom.scale = std::move(scale);
    atom.offset = offset;
    return atom;
}

const std::vector<UnitRegistry::Atom>& builtin_atoms() {
    static const std::vector<UnitRegistry::Atom> atoms = {
        make_atom("m", UnitKind::multiplicative, Dimension::base(DimensionBase::length), Scale{1, 1}, 0.0),
        make_atom("cm", UnitKind::multiplicative, Dimension::base(DimensionBase::length), Scale{1, 100}, 0.0),
        make_atom("mm", UnitKind::multiplicative, Dimension::base(DimensionBase::length), Scale{1, 1000}, 0.0),
        make_atom("km", UnitKind::multiplicative, Dimension::base(DimensionBase::length), Scale{1000, 1}, 0.0),
        make_atom("kg", UnitKind::multiplicative, Dimension::base(DimensionBase::mass), Scale{1, 1}, 0.0),
        make_atom("g", UnitKind::multiplicative, Dimension::base(DimensionBase::mass), Scale{1, 1000}, 0.0),
        make_atom("mg", UnitKind::multiplicative, Dimension::base(DimensionBase::mass), Scale{1, 1000000}, 0.0),
        make_atom("s", UnitKind::multiplicative, Dimension::base(DimensionBase::time), Scale{1, 1}, 0.0),
        make_atom("ms", UnitKind::multiplicative, Dimension::base(DimensionBase::time), Scale{1, 1000}, 0.0),
        make_atom("min", UnitKind::multiplicative, Dimension::base(DimensionBase::time), Scale{60, 1}, 0.0),
        make_atom("h", UnitKind::multiplicative, Dimension::base(DimensionBase::time), Scale{3600, 1}, 0.0),
        make_atom("A", UnitKind::multiplicative, Dimension::base(DimensionBase::current), Scale{1, 1}, 0.0),
        // The absolute temperatures carry their real-zero offset; 32 degF maps to the same
        // base value as 0 degC, which is the affine rule the fixtures assert.
        make_atom("K", UnitKind::absolute, Dimension::base(DimensionBase::temperature), Scale{1, 1}, 0.0),
        make_atom("degC", UnitKind::absolute, Dimension::base(DimensionBase::temperature), Scale{1, 1},
                  273.15),
        make_atom("degF", UnitKind::absolute, Dimension::base(DimensionBase::temperature), Scale{5, 9},
                  273.15 - (32.0 * 5.0 / 9.0)),
        make_atom("delta_degC", UnitKind::difference, Dimension::base(DimensionBase::temperature),
                  Scale{1, 1}, 0.0),
        make_atom("delta_degF", UnitKind::difference, Dimension::base(DimensionBase::temperature),
                  Scale{5, 9}, 0.0),
        make_atom("mol", UnitKind::multiplicative, Dimension::base(DimensionBase::substance), Scale{1, 1},
                  0.0),
        make_atom("cd", UnitKind::multiplicative, Dimension::base(DimensionBase::luminous_intensity),
                  Scale{1, 1}, 0.0),
        make_atom("rad", UnitKind::multiplicative, Dimension::base(DimensionBase::angle), Scale{1, 1}, 0.0),
        make_atom("deg", UnitKind::multiplicative, Dimension::base(DimensionBase::angle),
                  Scale{1, 1, 3.14159265358979323846 / 180.0}, 0.0),
        make_atom("percent", UnitKind::multiplicative, Dimension{}, Scale{1, 100}, 0.0),
        make_atom("1", UnitKind::multiplicative, Dimension{}, Scale{1, 1}, 0.0),
    };
    return atoms;
}

}  // namespace

const UnitRegistry& UnitRegistry::builtin() {
    static const UnitRegistry registry{"EG-UNIT-1", builtin_atoms()};
    return registry;
}

std::optional<UnitRegistry::Atom> UnitRegistry::atom(std::string_view name) const {
    for (const auto& candidate : atoms_) {
        if (candidate.name == name) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::vector<std::string> UnitRegistry::atom_names() const {
    std::vector<std::string> names;
    names.reserve(atoms_.size());
    for (const auto& atom : atoms_) {
        names.push_back(atom.name);
    }
    return names;
}

Result<Unit> UnitRegistry::parse_atom(std::string_view token) const {
    std::string_view identifier = token;
    std::int16_t exponent = 1;
    const auto digits_start = token.find_first_of("0123456789");
    if (digits_start != std::string_view::npos && digits_start > 0) {
        identifier = token.substr(0, digits_start);
        const std::string_view digits = token.substr(digits_start);
        std::int32_t parsed = 0;
        for (const char character : digits) {
            if (character < '0' || character > '9') {
                return Result<Unit>::failure(unit_diagnostic(
                    DetailCode::unknown_unit, "unit name has a non-digit after its exponent",
                    std::string{token}));
            }
            parsed = parsed * 10 + (character - '0');
            if (parsed > kMaxExponent) {
                return Result<Unit>::failure(unit_diagnostic(
                    DetailCode::unknown_unit, "unit exponent is outside the supported range",
                    std::string{token}));
            }
        }
        if (parsed == 0) {
            return Result<Unit>::failure(unit_diagnostic(
                DetailCode::unknown_unit, "unit exponent must not be zero", std::string{token}));
        }
        exponent = static_cast<std::int16_t>(parsed);
    }
    const auto found = atom(identifier);
    if (!found.has_value()) {
        return Result<Unit>::failure(
            unit_diagnostic(DetailCode::unknown_unit, "unit name is not in the declared registry",
                            std::string{token}));
    }
    if (found->kind == UnitKind::absolute && exponent != 1) {
        return Result<Unit>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "an absolute offset scale cannot be raised to a power",
            std::string{token}));
    }

    auto dimension = found->dimension.raised_to(exponent);
    if (!dimension.ok()) {
        return Result<Unit>::failure(dimension.diagnostic_copy());
    }
    auto scale = found->scale.raised_to(exponent);
    if (!scale.ok()) {
        return Result<Unit>::failure(scale.diagnostic_copy());
    }
    const std::string text{token};
    if (exponent == 1) {
        return Unit{found->kind, found->dimension, found->scale, found->offset, text, found->name};
    }
    return make_unit(UnitKind::multiplicative, dimension.take(), scale.take(), 0.0, text);
}

Result<Unit> UnitRegistry::parse(std::string_view source_text) const {
    const std::string_view trimmed = source_text;
    if (trimmed.empty()) {
        return Result<Unit>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "an empty unit is not a declared dimensionless unit", ""));
    }

    std::string_view head = trimmed;
    std::string_view tail;
    const auto slash = trimmed.find('/');
    if (slash != std::string_view::npos) {
        head = trimmed.substr(0, slash);
        tail = trimmed.substr(slash + 1);
        if (tail.empty()) {
            return Result<Unit>::failure(unit_diagnostic(
                DetailCode::unknown_unit, "a unit spelling with a slash names a denominator",
                std::string{trimmed}));
        }
        if (tail.find('/') != std::string_view::npos) {
            return Result<Unit>::failure(unit_diagnostic(
                DetailCode::unknown_unit, "a unit spelling has at most one denominator",
                std::string{trimmed}));
        }
    }

    const auto parse_sequence = [this](std::string_view sequence) -> Result<std::vector<Unit>> {
        std::vector<Unit> parts;
        std::size_t start = 0;
        if (sequence.empty()) {
            return Result<std::vector<Unit>>::failure(unit_diagnostic(
                DetailCode::unknown_unit, "unit sequence has an empty term", ""));
        }
        while (start <= sequence.size()) {
            const auto separator = sequence.find_first_of(".*", start);
            const std::string_view token = sequence.substr(
                start, separator == std::string_view::npos ? std::string_view::npos : separator - start);
            if (token.empty()) {
                return Result<std::vector<Unit>>::failure(unit_diagnostic(
                    DetailCode::unknown_unit, "unit sequence has an empty term",
                    std::string{sequence}));
            }
            auto part = parse_atom(token);
            if (!part.ok()) {
                return Result<std::vector<Unit>>::failure(part.diagnostic_copy());
            }
            parts.push_back(part.take());
            if (separator == std::string_view::npos) {
                break;
            }
            start = separator + 1;
            if (start == sequence.size()) {
                return Result<std::vector<Unit>>::failure(unit_diagnostic(
                    DetailCode::unknown_unit, "unit sequence ends with a separator",
                    std::string{sequence}));
            }
        }
        return parts;
    };

    auto numerator_parts = parse_sequence(head);
    if (!numerator_parts.ok()) {
        return Result<Unit>::failure(numerator_parts.diagnostic_copy());
    }
    std::vector<Unit> parts = numerator_parts.take();
    if (!tail.empty()) {
        auto denominator_parts = parse_sequence(tail);
        if (!denominator_parts.ok()) {
            return Result<Unit>::failure(denominator_parts.diagnostic_copy());
        }
        for (auto& part : denominator_parts.take()) {
            auto inverted_dimension = part.dimension().raised_to(-1);
            if (!inverted_dimension.ok()) {
                return Result<Unit>::failure(inverted_dimension.diagnostic_copy());
            }
            auto inverted_scale = part.scale().raised_to(-1);
            if (!inverted_scale.ok()) {
                return Result<Unit>::failure(inverted_scale.diagnostic_copy());
            }
            parts.push_back(Unit{UnitKind::multiplicative, inverted_dimension.take(),
                                 inverted_scale.take(), 0.0, part.source_text(), std::string{}});
        }
    }

    if (parts.size() == 1 && parts.front().source_text() == std::string{trimmed}) {
        return parts.front();
    }

    bool has_absolute = false;
    Dimension dimension;
    Scale scale{1, 1};
    for (const auto& part : parts) {
        if (part.is_absolute()) {
            has_absolute = true;
        }
        dimension = dimension.multiplied_by(part.dimension());
        auto product = scale.multiplied_by(part.scale());
        if (!product.ok()) {
            return Result<Unit>::failure(product.diagnostic_copy());
        }
        scale = product.take();
    }
    if (has_absolute) {
        return Result<Unit>::failure(unit_diagnostic(
            DetailCode::unknown_unit, "an absolute offset scale cannot form a compound unit",
            std::string{trimmed}));
    }
    return make_unit(UnitKind::multiplicative, dimension, scale, 0.0, std::string{trimmed});
}

Result<Quantity> convert(const Quantity& source, const Unit&target) {
    const Unit& from = source.unit;
    if (from.dimension() != target.dimension()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::incompatible_unit,
            "conversion crosses dimensions: " + from.canonical_text() + " to " +
                target.canonical_text(),
            from.source_text() + " -> " + target.source_text()));
    }
    const bool from_absolute = from.is_absolute();
    const bool to_absolute = target.is_absolute();
    if (from_absolute != to_absolute) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::incompatible_unit,
            "an absolute offset scale and a scale without a real zero are not convertible: " +
                from.canonical_text() + " to " + target.canonical_text(),
            from.source_text() + " -> " + target.source_text()));
    }
    if (!from_absolute && from.kind() != target.kind()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::incompatible_unit, "a difference scale is not convertible to a plain scale",
            from.source_text() + " -> " + target.source_text()));
    }

    Quantity result;
    if (from_absolute) {
        result.value = from_base(to_base(source.value, from.scale(), from.offset()), target.scale(),
                                 target.offset());
    } else {
        result.value = ratio_to(from.scale(), target.scale(), source.value);
    }
    result.unit = target;
    return result;
}

Result<Quantity> add(const Quantity& left, const Quantity& right) {
    if (left.unit.dimension() != right.unit.dimension()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::incompatible_unit,
            "addition crosses dimensions: " + left.unit.canonical_text() + " and " +
                right.unit.canonical_text(),
            left.unit.source_text() + " + " + right.unit.source_text()));
    }
    const bool left_absolute = left.unit.is_absolute();
    const bool right_absolute = right.unit.is_absolute();
    if (left_absolute && right_absolute) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "two absolute offset values cannot be added; convert one to a declared difference unit",
            left.unit.source_text() + " + " + right.unit.source_text()));
    }
    if (left_absolute && right.unit.kind() != UnitKind::difference) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "an absolute offset value only accepts a declared difference unit as an addend",
            left.unit.source_text() + " + " + right.unit.source_text()));
    }
    if (right_absolute && left.unit.kind() != UnitKind::difference) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "an absolute offset value only accepts a declared difference unit as an addend",
            left.unit.source_text() + " + " + right.unit.source_text()));
    }

    Quantity result;
    if (left_absolute) {
        const long double base = to_base(left.value, left.unit.scale(), left.unit.offset()) +
                                 to_base(right.value, right.unit.scale(), 0.0);
        result.value = from_base(base, left.unit.scale(), left.unit.offset());
        result.unit = left.unit;
        return result;
    }
    if (right_absolute) {
        const long double base = to_base(left.value, left.unit.scale(), 0.0) +
                                 to_base(right.value, right.unit.scale(), right.unit.offset());
        result.value = from_base(base, right.unit.scale(), right.unit.offset());
        result.unit = right.unit;
        return result;
    }
    auto moved = convert(right, left.unit);
    if (!moved.ok()) {
        return Result<Quantity>::failure(moved.diagnostic_copy());
    }
    result.value = left.value + moved.value().value;
    result.unit = left.unit;
    return result;
}

Result<Quantity> subtract(const Quantity& left, const Quantity& right) {
    if (left.unit.dimension() != right.unit.dimension()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::incompatible_unit,
            "subtraction crosses dimensions: " + left.unit.canonical_text() + " and " +
                right.unit.canonical_text(),
            left.unit.source_text() + " - " + right.unit.source_text()));
    }
    const bool left_absolute = left.unit.is_absolute();
    const bool right_absolute = right.unit.is_absolute();
    if (right_absolute) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "subtracting an absolute offset value needs a declared difference unit on both sides",
            left.unit.source_text() + " - " + right.unit.source_text()));
    }
    if (left_absolute && right.unit.kind() != UnitKind::difference) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "an absolute offset value only accepts a declared difference unit as a subtrahend",
            left.unit.source_text() + " - " + right.unit.source_text()));
    }
    Quantity negated_right;
    negated_right.value = -right.value;
    negated_right.unit = right.unit;
    return add(left, negated_right);
}

Result<Quantity> multiply(const Quantity& left, const Quantity& right) {
    if (left.unit.is_absolute() || right.unit.is_absolute()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "an absolute offset scale is not an algebraic factor; convert it to a difference unit",
            left.unit.source_text() + " * " + right.unit.source_text()));
    }
    auto scale = left.unit.scale().multiplied_by(right.unit.scale());
    if (!scale.ok()) {
        return Result<Quantity>::failure(scale.diagnostic_copy());
    }
    const Dimension dimension = left.unit.dimension().multiplied_by(right.unit.dimension());
    const Unit unit = make_unit(UnitKind::multiplicative, dimension, scale.take(), 0.0,
                                left.unit.source_text() + "*" + right.unit.source_text());
    Quantity result;
    result.value = from_base(to_base(left.value, left.unit.scale(), 0.0) *
                                 to_base(right.value, right.unit.scale(), 0.0),
                             unit.scale(), 0.0);
    result.unit = unit;
    return result;
}

Result<Quantity> divide(const Quantity& left, const Quantity& right) {
    if (left.unit.is_absolute() || right.unit.is_absolute()) {
        return Result<Quantity>::failure(unit_diagnostic(
            DetailCode::offset_unit_arithmetic,
            "an absolute offset scale is not an algebraic factor; convert it to a difference unit",
            left.unit.source_text() + " / " + right.unit.source_text()));
    }
    auto scale = left.unit.scale().divided_by(right.unit.scale());
    if (!scale.ok()) {
        return Result<Quantity>::failure(scale.diagnostic_copy());
    }
    const Dimension dimension = left.unit.dimension().divided_by(right.unit.dimension());
    const Unit unit = make_unit(UnitKind::multiplicative, dimension, scale.take(), 0.0,
                                left.unit.source_text() + "/" + right.unit.source_text());
    Quantity result;
    result.value = from_base(to_base(left.value, left.unit.scale(), 0.0) /
                                 to_base(right.value, right.unit.scale(), 0.0),
                             unit.scale(), 0.0);
    result.unit = unit;
    return result;
}

Result<Unit> as_difference_unit(const Unit& unit) {
    if (!unit.is_absolute()) {
        return unit;
    }
    std::string name = "delta_";
    name += unit.source_text();
    const auto found = UnitRegistry::builtin().atom(name);
    if (found.has_value()) {
        return Unit{found->kind, found->dimension, found->scale, found->offset, name, found->name};
    }
    return Result<Unit>::failure(unit_diagnostic(
        DetailCode::unknown_unit, "no declared difference unit exists for this absolute scale",
        unit.source_text()));
}

Unit dimensionless_unit() {
    return Unit{UnitKind::multiplicative, Dimension{}, Scale{1, 1}, 0.0, "1", "1"};
}

}  // namespace everygraph::data
