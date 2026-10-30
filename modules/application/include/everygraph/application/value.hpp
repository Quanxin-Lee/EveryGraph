#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace everygraph::application {

using Value = std::variant<std::monostate, std::int64_t, double, bool, std::string>;
using ParameterMap = std::map<std::string, Value, std::less<>>;

[[nodiscard]] std::string canonical_parameters(const ParameterMap& parameters);
[[nodiscard]] std::string canonical_digest(const ParameterMap& parameters);
[[nodiscard]] bool values_equal(const Value& left, const Value& right);
[[nodiscard]] bool parameters_equal(const ParameterMap& left, const ParameterMap& right);

class ParameterSnapshot {
public:
    ParameterSnapshot();
    explicit ParameterSnapshot(ParameterMap values);

    [[nodiscard]] const ParameterMap& values() const noexcept;
    [[nodiscard]] const std::string& canonical() const noexcept;
    [[nodiscard]] const std::string& digest() const noexcept;

private:
    std::shared_ptr<const ParameterMap> values_;
    std::string canonical_;
    std::string digest_;
};

}  // namespace everygraph::application
