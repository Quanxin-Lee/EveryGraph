#include "everygraph/data/provenance.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace everygraph::data {
namespace {

constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

}  // namespace

std::string ContentHash::hex() const {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string text;
    text.reserve(16);
    for (int shift = 60; shift >= 0; shift -= 4) {
        text += kDigits[(value >> static_cast<unsigned>(shift)) & 0xFULL];
    }
    return text;
}

ContentHash hash_bytes(std::string_view bytes) noexcept {
    // FNV-1a over the canonical encoding. Deterministic across platforms and builds, which
    // is what a lineage marker needs; it is not a cryptographic digest.
    std::uint64_t hash = kFnvOffsetBasis;
    for (const char character : bytes) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(character));
        hash *= kFnvPrime;
    }
    return ContentHash{hash};
}

std::string Provenance::canonical_text() const {
    std::string text;
    text += operation_id;
    text += "|";
    text += std::to_string(operation_version);
    text += "|";
    text += canonical_parameters;
    text += "|";
    text += unit_policy;
    text += "|";
    text += seed.has_value() ? std::to_string(*seed) : "none";
    text += "|inputs=";
    for (const auto& input : inputs) {
        text += to_string(input.version);
        text += ":";
        text += input.content_hash.hex();
        text += ",";
    }
    text += "|parents=";
    for (const auto& parent : parents) {
        text += to_string(parent);
        text += ",";
    }
    return text;
}

}  // namespace everygraph::data
