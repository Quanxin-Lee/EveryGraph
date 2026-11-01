#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "everygraph/data/cell.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/provenance.hpp"

namespace everygraph::data {

// One worksheet's row-identity domain: the ordered set of row identities. A row identity is
// stable within a dataset lineage, so position in this vector is never an identity.
class RowDomain {
public:
    static Result<RowDomain> create(RowDomainId id, std::vector<RowId> rows);

    [[nodiscard]] RowDomainId id() const noexcept { return id_; }
    [[nodiscard]] std::uint64_t row_count() const noexcept { return rows_.size(); }
    [[nodiscard]] const std::vector<RowId>& rows() const noexcept { return rows_; }
    [[nodiscard]] const ContentHash& content_hash() const noexcept { return content_hash_; }

    [[nodiscard]] Result<RowId> row_at(std::uint64_t index) const;
    [[nodiscard]] std::optional<std::uint64_t> index_of(RowId row) const noexcept;

    // Both forms produce a new domain under a new identity and leave this one unchanged, which
    // is what lets a same-domain binding set follow a source insertion or deletion as one
    // rectangular revision while the superseded domain stays readable.
    [[nodiscard]] Result<RowDomain> extended_with(RowDomainId revised_id,
                                                  const std::vector<RowId>& new_rows) const;
    [[nodiscard]] Result<RowDomain> reduced_by(RowDomainId revised_id,
                                               const std::vector<RowId>& removed_rows) const;

private:
    RowDomain(RowDomainId id, std::vector<RowId> rows, ContentHash content_hash)
        : id_(id), rows_(std::move(rows)), content_hash_(content_hash) {}

    RowDomainId id_;
    std::vector<RowId> rows_;
    ContentHash content_hash_;
};

class RowDomainBuilder {
public:
    static Result<RowDomainBuilder> create();

    [[nodiscard]] std::uint64_t row_count() const noexcept { return rows_.size(); }
    [[nodiscard]] std::optional<Diagnostic> append(RowId row);
    [[nodiscard]] Result<RowDomain> finish(RowDomainId id) const;

private:
    RowDomainBuilder() = default;

    std::vector<RowId> rows_;
};

enum class MappingMethod {
    same_domain,
    keyed_join,
    approved_positional,
};

[[nodiscard]] const char* to_string(MappingMethod method) noexcept;

// How a binding of a foreign-domain column pairs worksheet rows to source rows. There is no
// default here: an unrelated column is never zipped by current array position.
class RowCorrespondence {
public:
    // Keys are compared structurally. A source key that repeats is rejected instead of
    // multiplied, and a target row whose key matches nothing stays explicitly unmatched
    // without inventing a value.
    [[nodiscard]] static Result<RowCorrespondence> keyed_join(
        CorrespondenceId id, const RowDomain& target_domain, const std::vector<Cell>& target_keys,
        const RowDomain& source_domain, const std::vector<Cell>& source_keys);

    // Equal-length pairing that must be explicitly approved; a stable ID pair list is what
    // gets recorded, so a later re-sort cannot re-pair by position.
    [[nodiscard]] static Result<RowCorrespondence> approved_positional(
        CorrespondenceId id, const RowDomain& target_domain, const RowDomain& source_domain,
        bool user_approved);

    [[nodiscard]] CorrespondenceId id() const noexcept { return id_; }
    [[nodiscard]] RowDomainId target_domain() const noexcept { return target_domain_; }
    [[nodiscard]] RowDomainId source_domain() const noexcept { return source_domain_; }
    [[nodiscard]] MappingMethod method() const noexcept { return method_; }
    [[nodiscard]] const std::vector<std::optional<RowId>>& pairs() const noexcept { return pairs_; }
    [[nodiscard]] const std::vector<RowId>& unmatched_targets() const noexcept {
        return unmatched_targets_;
    }
    [[nodiscard]] const std::vector<RowId>& unmatched_sources() const noexcept {
        return unmatched_sources_;
    }
    [[nodiscard]] const ContentHash& content_hash() const noexcept { return content_hash_; }
    [[nodiscard]] std::uint64_t mapped_count() const noexcept;

    [[nodiscard]] std::optional<RowId> source_at(std::uint64_t target_index) const noexcept;
    [[nodiscard]] std::optional<RowId> source_for(const RowDomain& target_domain,
                                                  RowId target_row) const noexcept;

private:
    RowCorrespondence(CorrespondenceId id, RowDomainId target_domain, RowDomainId source_domain,
                      MappingMethod method, std::vector<std::optional<RowId>> pairs,
                      std::vector<RowId> unmatched_targets, std::vector<RowId> unmatched_sources,
                      ContentHash content_hash)
        : id_(id),
          target_domain_(target_domain),
          source_domain_(source_domain),
          method_(method),
          pairs_(std::move(pairs)),
          unmatched_targets_(std::move(unmatched_targets)),
          unmatched_sources_(std::move(unmatched_sources)),
          content_hash_(content_hash) {}

    CorrespondenceId id_;
    RowDomainId target_domain_;
    RowDomainId source_domain_;
    MappingMethod method_{MappingMethod::same_domain};
    std::vector<std::optional<RowId>> pairs_;
    std::vector<RowId> unmatched_targets_;
    std::vector<RowId> unmatched_sources_;
    ContentHash content_hash_;
};

}  // namespace everygraph::data
