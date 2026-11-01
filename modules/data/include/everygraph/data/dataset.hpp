#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <map>
#include <utility>
#include <vector>

#include "everygraph/data/chunk.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/logical_type.hpp"
#include "everygraph/data/provenance.hpp"
#include "everygraph/data/row_domain.hpp"
#include "everygraph/data/units.hpp"

namespace everygraph::data {

// An unseen category is a declared choice, never a silent mapping onto the first label.
enum class UnknownCategoryPolicy {
    reject,
    missing,
};

[[nodiscard]] const char* to_string(UnknownCategoryPolicy policy) noexcept;

struct Dictionary {
    std::vector<std::string> labels;

    [[nodiscard]] static Result<Dictionary> create(std::vector<std::string> labels);
    [[nodiscard]] std::optional<std::int64_t> find_code(std::string_view label) const noexcept;
    [[nodiscard]] Result<std::int64_t> code_for(std::string_view label) const;
    [[nodiscard]] Result<std::string> label_for(std::int64_t code) const;
};

// A field is a typed column of a dataset with its own stable identity. Unit, role and display
// format are metadata about the values, not part of a value.
struct FieldSchema {
    FieldId id;
    std::string name;
    LogicalType type{LogicalType::float64};
    std::optional<Unit> unit;
    bool nullable{true};
    std::shared_ptr<const Dictionary> dictionary;
    UnknownCategoryPolicy unknown_category{UnknownCategoryPolicy::reject};
    std::string display_format;
    // A zoned timestamp must name its zone: the instant alone is not the declared semantics,
    // and a naive or date field never gains one.
    std::optional<std::string> zone_id;

    [[nodiscard]] std::string canonical_text() const;

    // The declared resolution of a label against this field's dictionary.
    [[nodiscard]] Result<Cell> resolve_category(std::string_view label) const;
};

struct DatasetSchema {
    std::vector<FieldSchema> fields;

    [[nodiscard]] static Result<DatasetSchema> create(std::vector<FieldSchema> fields);
    [[nodiscard]] const FieldSchema* find(FieldId id) const noexcept;
    [[nodiscard]] std::string canonical_text() const;
};

// One immutable dataset version. Everything about it is fixed at creation, including the
// chunk list, so a reader that resolved an older version keeps reading exactly what it saw.
class DatasetVersion {
public:
    [[nodiscard]] static Result<std::shared_ptr<const DatasetVersion>> create(
        DatasetVersionId id, DatasetId dataset, std::shared_ptr<const DatasetSchema> schema,
        RowDomainId row_domain, std::vector<std::shared_ptr<const Chunk>> chunks,
        Provenance provenance, std::uint64_t row_count, foundation::Revision revision);

    [[nodiscard]] DatasetVersionId id() const noexcept { return id_; }
    [[nodiscard]] DatasetId dataset() const noexcept { return dataset_; }
    [[nodiscard]] const std::shared_ptr<const DatasetSchema>& schema() const noexcept {
        return schema_;
    }
    [[nodiscard]] RowDomainId row_domain() const noexcept { return row_domain_; }
    [[nodiscard]] std::uint64_t row_count() const noexcept { return row_count_; }
    [[nodiscard]] const Provenance& provenance() const noexcept { return provenance_; }
    [[nodiscard]] foundation::Revision revision() const noexcept { return revision_; }
    [[nodiscard]] const ContentHash& content_hash() const noexcept { return content_hash_; }
    [[nodiscard]] std::vector<ChunkId> chunk_ids() const;
    [[nodiscard]] Result<std::shared_ptr<const Chunk>> chunk_for(FieldId field,
                                                                 std::uint64_t row_index) const;
    [[nodiscard]] std::vector<std::shared_ptr<const Chunk>> chunks_of(FieldId field) const;

    // Reading is by field identity plus a row index inside this version's own ordering.
    [[nodiscard]] Result<Cell> cell(FieldId field, std::uint64_t row_index) const;

private:
    DatasetVersion(DatasetVersionId id, DatasetId dataset, std::shared_ptr<const DatasetSchema> schema,
                   RowDomainId row_domain, std::vector<std::shared_ptr<const Chunk>> chunks,
                   Provenance provenance, std::uint64_t row_count, foundation::Revision revision,
                   ContentHash content_hash)
        : id_(id),
          dataset_(dataset),
          schema_(std::move(schema)),
          row_domain_(row_domain),
          chunks_(std::move(chunks)),
          provenance_(std::move(provenance)),
          row_count_(row_count),
          revision_(revision),
          content_hash_(content_hash) {}

    DatasetVersionId id_;
    DatasetId dataset_;
    std::shared_ptr<const DatasetSchema> schema_;
    RowDomainId row_domain_;
    std::vector<std::shared_ptr<const Chunk>> chunks_;
    Provenance provenance_;
    std::uint64_t row_count_{0};
    foundation::Revision revision_{foundation::kInitialRevision};
    ContentHash content_hash_;
};

// The lifetime-owning read view named by the data model. A lease holds shared references to
// the version and its row domain and hands out values by copy, so no view can outlive the
// immutable state it resolved and no buffer reference escapes.
class DatasetLease {
public:
    DatasetLease(std::shared_ptr<const DatasetVersion> version,
                 std::shared_ptr<const RowDomain> row_domain);

    [[nodiscard]] const DatasetVersion& version() const noexcept { return *version_; }
    [[nodiscard]] DatasetVersionId version_id() const noexcept { return version_->id(); }
    [[nodiscard]] std::uint64_t row_count() const noexcept { return version_->row_count(); }

    [[nodiscard]] Result<RowId> row_at(std::uint64_t index) const;
    [[nodiscard]] Result<std::uint64_t> index_of(RowId row) const;
    [[nodiscard]] Result<Cell> cell_at(FieldId field, std::uint64_t row_index) const;
    [[nodiscard]] Result<Cell> cell(FieldId field, RowId row) const;

private:
    std::shared_ptr<const DatasetVersion> version_;
    std::shared_ptr<const RowDomain> row_domain_;
};

// The immutable payload store. Staging is what a command handler does with its private
// candidate output: a staged object is not readable until the executor reports that its
// transaction committed, and a rejected or canceled transaction drops it. Only the executor can
// move a staged payload into visible state, so no caller mutates published data; the residual
// limit is that staging keys on the kernel's transaction id, which a holder of this store
// reference could name in advance, so binding a staged payload to a transaction that is
// actually running needs a capability on the kernel's execution context (P2-002's rule 9
// request to P1-004, recorded in its task record).
class DataStore {
public:
    [[nodiscard]] static Result<std::shared_ptr<const DatasetSchema>> schema(
        std::vector<FieldSchema> fields);

    void stage(std::uint64_t transaction_id, std::shared_ptr<const DatasetVersion> version);
    void stage(std::uint64_t transaction_id, std::shared_ptr<const RowDomain> domain);
    void stage(std::uint64_t transaction_id, std::shared_ptr<const RowCorrespondence> correspondence);

    [[nodiscard]] bool contains(DatasetVersionId id) const noexcept;
    [[nodiscard]] bool contains(RowDomainId id) const noexcept;
    [[nodiscard]] bool contains(CorrespondenceId id) const noexcept;

    [[nodiscard]] Result<std::shared_ptr<const DatasetVersion>> version(DatasetVersionId id) const;
    [[nodiscard]] Result<std::shared_ptr<const RowDomain>> row_domain(RowDomainId id) const;
    [[nodiscard]] Result<std::shared_ptr<const RowCorrespondence>> correspondence(
        CorrespondenceId id) const;
    [[nodiscard]] Result<DatasetLease> lease(DatasetVersionId id) const;

    [[nodiscard]] std::vector<DatasetVersionId> versions_of(DatasetId dataset) const;
    // Which stored versions consumed this one as an input or parent. A delete that would
    // strand one of them is rejected naming it.
    [[nodiscard]] std::vector<DatasetVersionId> versions_referencing(DatasetVersionId version) const;
    [[nodiscard]] std::uint64_t version_count() const noexcept { return versions_.size(); }
    [[nodiscard]] std::uint64_t staged_count() const noexcept;

private:
    friend class DataCommandExecutor;

    // Only the application executor moves a staged candidate into visible state.
    void apply_commit(std::uint64_t transaction_id);
    void discard(std::uint64_t transaction_id);

    std::vector<std::shared_ptr<const DatasetVersion>> versions_;
    std::vector<std::shared_ptr<const RowDomain>> domains_;
    std::vector<std::shared_ptr<const RowCorrespondence>> correspondences_;
    struct Staged {
        std::vector<std::shared_ptr<const DatasetVersion>> versions;
        std::vector<std::shared_ptr<const RowDomain>> domains;
        std::vector<std::shared_ptr<const RowCorrespondence>> correspondences;
    };

    std::map<std::uint64_t, Staged> staged_;
};

}  // namespace everygraph::data
