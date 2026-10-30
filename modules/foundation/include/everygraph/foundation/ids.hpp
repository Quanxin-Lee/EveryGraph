#pragma once

#include <compare>
#include <cstdint>
#include <string>

namespace everygraph::foundation {

struct ObjectId {
    std::uint64_t value{0};

    [[nodiscard]] constexpr bool is_valid() const noexcept { return value != 0; }

    friend constexpr bool operator==(const ObjectId&, const ObjectId&) noexcept = default;
    friend constexpr auto operator<=>(const ObjectId&, const ObjectId&) noexcept = default;
};

[[nodiscard]] std::string to_string(ObjectId id);

using Revision = std::uint64_t;
inline constexpr Revision kInitialRevision = 0;

using TransactionId = std::uint64_t;
inline constexpr TransactionId kInvalidTransactionId = 0;

using EventSequence = std::uint64_t;
inline constexpr EventSequence kNoEventSequence = 0;

}  // namespace everygraph::foundation
