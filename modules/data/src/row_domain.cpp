#include "everygraph/data/row_domain.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace everygraph::data {
namespace {

std::string rows_text(const std::vector<RowId>& rows) {
    std::string text;
    for (const auto& row : rows) {
        text += to_string(row);
        text += ",";
    }
    return text;
}

std::string pairs_text(const std::vector<std::optional<RowId>>& pairs) {
    std::string text;
    for (const auto& pair : pairs) {
        text += pair.has_value() ? to_string(*pair) : "unmatched";
        text += ",";
    }
    return text;
}

Diagnostic duplicate_id_diagnostic(RowId row, const std::string& context) {
    Location location;
    location.row = row;
    location.description = context;
    return Diagnostic::reject(DetailCode::duplicate_id, "row identity is already in this domain",
                              std::move(location));
}

Diagnostic join_diagnostic(DetailCode code, std::string message, std::string context) {
    Location location;
    location.description = std::move(context);
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

}  // namespace

const char* to_string(MappingMethod method) noexcept {
    switch (method) {
        case MappingMethod::same_domain:
            return "same_domain";
        case MappingMethod::keyed_join:
            return "keyed_join";
        case MappingMethod::approved_positional:
            return "approved_positional";
    }
    return "unknown";
}

Result<RowDomain> RowDomain::create(RowDomainId id, std::vector<RowId> rows) {
    if (!id.is_valid()) {
        return Result<RowDomain>::failure(
            join_diagnostic(DetailCode::duplicate_id, "row domain identity must be non-zero", ""));
    }
    std::vector<RowId> copy = std::move(rows);
    for (std::size_t index = 0; index < copy.size(); ++index) {
        if (!copy[index].is_valid()) {
            return Result<RowDomain>::failure(
                join_diagnostic(DetailCode::duplicate_id, "row identity must be non-zero", ""));
        }
        for (std::size_t earlier = 0; earlier < index; ++earlier) {
            if (copy[earlier] == copy[index]) {
                return Result<RowDomain>::failure(duplicate_id_diagnostic(copy[index], "create"));
            }
        }
    }
    const ContentHash content = hash_bytes(rows_text(copy));
    return RowDomain{id, std::move(copy), content};
}

Result<RowId> RowDomain::row_at(std::uint64_t index) const {
    if (index >= rows_.size()) {
        Location location;
        location.row_index = static_cast<std::int64_t>(index);
        location.description = "row domain " + to_string(id_);
        return Result<RowId>::failure(Diagnostic::reject(
            DetailCode::index_out_of_range, "row index is outside the domain", std::move(location)));
    }
    return rows_[index];
}

std::optional<std::uint64_t> RowDomain::index_of(RowId row) const noexcept {
    const auto found = std::find(rows_.begin(), rows_.end(), row);
    if (found == rows_.end()) {
        return std::nullopt;
    }
    return static_cast<std::uint64_t>(found - rows_.begin());
}

Result<RowDomain> RowDomain::extended_with(RowDomainId revised_id,
                                           const std::vector<RowId>& new_rows) const {
    std::vector<RowId> combined = rows_;
    combined.reserve(combined.size() + new_rows.size());
    for (const auto& row : new_rows) {
        if (std::find(combined.begin(), combined.end(), row) != combined.end()) {
            return Result<RowDomain>::failure(duplicate_id_diagnostic(row, "append"));
        }
        combined.push_back(row);
    }
    return RowDomain::create(revised_id, std::move(combined));
}

Result<RowDomain> RowDomain::reduced_by(RowDomainId revised_id,
                                        const std::vector<RowId>& removed_rows) const {
    std::vector<RowId> kept;
    kept.reserve(rows_.size());
    for (const auto& row : rows_) {
        if (std::find(removed_rows.begin(), removed_rows.end(), row) == removed_rows.end()) {
            kept.push_back(row);
        }
    }
    return RowDomain::create(revised_id, std::move(kept));
}

Result<RowDomainBuilder> RowDomainBuilder::create() {
    return RowDomainBuilder{};
}

std::optional<Diagnostic> RowDomainBuilder::append(RowId row) {
    if (!row.is_valid()) {
        return join_diagnostic(DetailCode::duplicate_id, "row identity must be non-zero", "");
    }
    if (std::find(rows_.begin(), rows_.end(), row) != rows_.end()) {
        return duplicate_id_diagnostic(row, "builder");
    }
    rows_.push_back(row);
    return std::nullopt;
}

Result<RowDomain> RowDomainBuilder::finish(RowDomainId id) const {
    return RowDomain::create(id, rows_);
}

Result<RowCorrespondence> RowCorrespondence::keyed_join(CorrespondenceId id,
                                                        const RowDomain& target_domain,
                                                        const std::vector<Cell>& target_keys,
                                                        const RowDomain& source_domain,
                                                        const std::vector<Cell>& source_keys) {
    if (!id.is_valid()) {
        return Result<RowCorrespondence>::failure(join_diagnostic(
            DetailCode::duplicate_id, "correspondence identity must be non-zero", ""));
    }
    if (target_keys.size() != target_domain.row_count()) {
        return Result<RowCorrespondence>::failure(join_diagnostic(
            DetailCode::unmapped_row,
            "target key list length " + std::to_string(target_keys.size()) +
                " does not match the target domain length " +
                std::to_string(target_domain.row_count()),
            "target domain " + to_string(target_domain.id())));
    }
    if (source_keys.size() != source_domain.row_count()) {
        return Result<RowCorrespondence>::failure(join_diagnostic(
            DetailCode::unmapped_row,
            "source key list length " + std::to_string(source_keys.size()) +
                " does not match the source domain length " +
                std::to_string(source_domain.row_count()),
            "source domain " + to_string(source_domain.id())));
    }

    std::vector<std::optional<RowId>> pairs(target_keys.size(), std::nullopt);
    std::vector<RowId> unmatched_targets;
    std::vector<bool> source_consumed(source_keys.size(), false);

    for (std::size_t target_index = 0; target_index < target_keys.size(); ++target_index) {
        const Cell& key = target_keys[target_index];
        if (key.is_missing()) {
            unmatched_targets.push_back(target_domain.rows()[target_index]);
            continue;
        }
        std::size_t matches = 0;
        std::size_t first_match = 0;
        for (std::size_t source_index = 0; source_index < source_keys.size(); ++source_index) {
            const Cell& candidate = source_keys[source_index];
            if (candidate.is_missing()) {
                continue;
            }
            if (candidate.type() != key.type()) {
                return Result<RowCorrespondence>::failure(join_diagnostic(
                    DetailCode::type_mismatch,
                    std::string{"join key types differ: "} + to_string(key.type()) + " and " +
                        to_string(candidate.type()),
                    "source row " +
                        to_string(source_domain.rows()[source_index])));
            }
            if (structurally_equal(candidate, key)) {
                ++matches;
                first_match = source_index;
                if (matches > 1) {
                    return Result<RowCorrespondence>::failure(join_diagnostic(
                        DetailCode::duplicate_join_key,
                        "join key repeats in the source domain, so an expansion or aggregation "
                        "policy is required",
                        "source row " + to_string(source_domain.rows()[source_index])));
                }
            }
        }
        if (matches == 0) {
            unmatched_targets.push_back(target_domain.rows()[target_index]);
            continue;
        }
        pairs[target_index] = source_domain.rows()[first_match];
        source_consumed[first_match] = true;
    }

    std::vector<RowId> unmatched_sources;
    for (std::size_t source_index = 0; source_index < source_keys.size(); ++source_index) {
        if (!source_consumed[source_index]) {
            unmatched_sources.push_back(source_domain.rows()[source_index]);
        }
    }

    const auto content = hash_bytes(to_string(id) + "|" + to_string(target_domain.id()) + "|" +
                                    to_string(source_domain.id()) + "|" +
                                    std::string{to_string(MappingMethod::keyed_join)} + "|" +
                                    pairs_text(pairs) + "|" + rows_text(unmatched_targets) + "|" +
                                    rows_text(unmatched_sources));
    return RowCorrespondence{id,
                             target_domain.id(),
                             source_domain.id(),
                             MappingMethod::keyed_join,
                             std::move(pairs),
                             std::move(unmatched_targets),
                             std::move(unmatched_sources),
                             content};
}

Result<RowCorrespondence> RowCorrespondence::approved_positional(CorrespondenceId id,
                                                                 const RowDomain& target_domain,
                                                                 const RowDomain& source_domain,
                                                                 bool user_approved) {
    if (!user_approved) {
        Location location;
        location.description = "target domain " + to_string(target_domain.id()) + " with source domain " +
                               to_string(source_domain.id());
        return Result<RowCorrespondence>::failure(Diagnostic::reject(
            DetailCode::positional_mapping_required,
            "a foreign-domain column needs a keyed join or an explicitly approved positional "
            "mapping captured as stable id pairs",
            std::move(location)));
    }
    if (!id.is_valid()) {
        return Result<RowCorrespondence>::failure(join_diagnostic(
            DetailCode::duplicate_id, "correspondence identity must be non-zero", ""));
    }
    if (target_domain.row_count() != source_domain.row_count()) {
        return Result<RowCorrespondence>::failure(join_diagnostic(
            DetailCode::unmapped_row,
            "positional mapping cannot zip or pad unequal lengths: " +
                std::to_string(target_domain.row_count()) + " target rows and " +
                std::to_string(source_domain.row_count()) + " source rows",
            "target domain " + to_string(target_domain.id()) + ", source domain " +
                to_string(source_domain.id())));
    }

    std::vector<std::optional<RowId>> pairs;
    pairs.reserve(source_domain.row_count());
    for (const auto& row : source_domain.rows()) {
        pairs.push_back(row);
    }
    const auto content = hash_bytes(to_string(id) + "|" + to_string(target_domain.id()) + "|" +
                                    to_string(source_domain.id()) + "|" +
                                    std::string{to_string(MappingMethod::approved_positional)} +
                                    "|" + pairs_text(pairs));
    return RowCorrespondence{id, target_domain.id(), source_domain.id(),
                             MappingMethod::approved_positional, std::move(pairs), {}, {}, content};
}

std::optional<RowId> RowCorrespondence::source_at(std::uint64_t target_index) const noexcept {
    if (target_index >= pairs_.size()) {
        return std::nullopt;
    }
    return pairs_[target_index];
}

std::optional<RowId> RowCorrespondence::source_for(const RowDomain& target_domain,
                                                   RowId target_row) const noexcept {
    // The pairs vector is indexed by the target domain order, which is immutable, so this
    // position is a lookup of a stable identity and never an identity itself.
    if (target_domain.id() != target_domain_) {
        return std::nullopt;
    }
    const auto index = target_domain.index_of(target_row);
    if (!index.has_value()) {
        return std::nullopt;
    }
    return source_at(index.value());
}

std::uint64_t RowCorrespondence::mapped_count() const noexcept {
    std::uint64_t count = 0;
    for (const auto& pair : pairs_) {
        if (pair.has_value()) {
            ++count;
        }
    }
    return count;
}

}  // namespace everygraph::data
