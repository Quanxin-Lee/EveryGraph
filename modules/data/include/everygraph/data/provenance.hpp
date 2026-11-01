#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/ids.hpp"

namespace everygraph::data {

// A content hash recorded when a chunk or version is created. It is a change-detection and
// lineage marker, not authenticity: an adversary who can replace the content can also
// replace this value, which is what the storage contract says about checksums.
struct ContentHash {
    std::uint64_t value{0};

    [[nodiscard]] std::string hex() const;

    friend constexpr bool operator==(const ContentHash&, const ContentHash&) noexcept = default;
    friend constexpr auto operator<=>(const ContentHash&, const ContentHash&) noexcept = default;
};

[[nodiscard]] ContentHash hash_bytes(std::string_view bytes) noexcept;

// One upstream version in a lineage edge, with the content hash that identifies exactly what
// was consumed.
struct VersionInput {
    DatasetVersionId version;
    ContentHash content_hash;

    friend bool operator==(const VersionInput&, const VersionInput&) = default;
};

// Every published version states how it was produced, so a later task can reproduce the
// lineage without reading an implementation's private history.
struct Provenance {
    std::string operation_id;
    std::uint32_t operation_version{1};
    std::string canonical_parameters;
    std::vector<VersionInput> inputs;
    std::string unit_policy;
    std::optional<std::uint64_t> seed;
    std::vector<DatasetVersionId> parents;

    [[nodiscard]] std::string canonical_text() const;

    friend bool operator==(const Provenance&, const Provenance&) = default;
};

}  // namespace everygraph::data
