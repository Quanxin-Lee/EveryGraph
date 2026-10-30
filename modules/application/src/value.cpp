#include "everygraph/application/value.hpp"

#include <bit>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>

namespace everygraph::application {
namespace {

static_assert(sizeof(double) == sizeof(std::uint64_t),
              "value equality requires a 64-bit double representation");

void append_value(std::string& output, const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        output += "n;";
        return;
    }
    if (const auto* integer = std::get_if<std::int64_t>(&value)) {
        output += "i:";
        output += std::to_string(*integer);
        output += ';';
        return;
    }
    if (const auto* number = std::get_if<double>(&value)) {
        output += "d:";
        if (std::isnan(*number)) {
            output += "nan";
        } else if (std::isinf(*number)) {
            output += (*number < 0.0) ? "-inf" : "+inf";
        } else {
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            stream << std::hexfloat << *number;
            output += stream.str();
        }
        output += ';';
        return;
    }
    if (const auto* boolean = std::get_if<bool>(&value)) {
        output += *boolean ? "b:1;" : "b:0;";
        return;
    }
    const auto& text = std::get<std::string>(value);
    output += "s:";
    output += std::to_string(text.size());
    output += ':';
    output += text;
    output += ';';
}

std::string digest_canonical(const std::string& canonical) {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char character : canonical) {
        const auto byte = static_cast<unsigned char>(character);
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= 1099511628211ULL;
    }
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::hex << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
}

}  // namespace

std::string canonical_parameters(const ParameterMap& parameters) {
    std::string output;
    output += '{';
    for (const auto& entry : parameters) {
        output += "k:";
        output += std::to_string(entry.first.size());
        output += ':';
        output += entry.first;
        output += ';';
        append_value(output, entry.second);
    }
    output += '}';
    return output;
}

std::string canonical_digest(const ParameterMap& parameters) {
    return digest_canonical(canonical_parameters(parameters));
}

bool values_equal(const Value& left, const Value& right) {
    if (left.index() != right.index()) {
        return false;
    }
    if (std::holds_alternative<std::monostate>(left)) {
        return true;
    }
    if (const auto* left_integer = std::get_if<std::int64_t>(&left)) {
        return *left_integer == std::get<std::int64_t>(right);
    }
    if (const auto* left_number = std::get_if<double>(&left)) {
        const double right_number = std::get<double>(right);
        if (std::isnan(*left_number) && std::isnan(right_number)) {
            return true;
        }
        return std::bit_cast<std::uint64_t>(*left_number) ==
               std::bit_cast<std::uint64_t>(right_number);
    }
    if (const auto* left_boolean = std::get_if<bool>(&left)) {
        return *left_boolean == std::get<bool>(right);
    }
    return std::get<std::string>(left) == std::get<std::string>(right);
}

bool parameters_equal(const ParameterMap& left, const ParameterMap& right) {
    if (left.size() != right.size()) {
        return false;
    }
    auto left_iterator = left.begin();
    auto right_iterator = right.begin();
    while (left_iterator != left.end()) {
        if (left_iterator->first != right_iterator->first ||
            !values_equal(left_iterator->second, right_iterator->second)) {
            return false;
        }
        ++left_iterator;
        ++right_iterator;
    }
    return true;
}

ParameterSnapshot::ParameterSnapshot() : ParameterSnapshot(ParameterMap{}) {}

ParameterSnapshot::ParameterSnapshot(ParameterMap values)
    : values_(std::make_shared<const ParameterMap>(std::move(values))),
      canonical_(canonical_parameters(*values_)),
      digest_(digest_canonical(canonical_)) {}

const ParameterMap& ParameterSnapshot::values() const noexcept { return *values_; }

const std::string& ParameterSnapshot::canonical() const noexcept { return canonical_; }

const std::string& ParameterSnapshot::digest() const noexcept { return digest_; }

}  // namespace everygraph::application
