#include "everygraph/data/dataset.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "everygraph/data/datetime.hpp"

namespace everygraph::data {
namespace {

Diagnostic store_diagnostic(DetailCode code, std::string message, std::string context) {
    Location location;
    location.description = std::move(context);
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

std::uint64_t chunk_rows(const Chunk& chunk) noexcept {
    return chunk.row_count();
}

}  // namespace

const char* to_string(UnknownCategoryPolicy policy) noexcept {
    switch (policy) {
        case UnknownCategoryPolicy::reject:
            return "reject";
        case UnknownCategoryPolicy::missing:
            return "missing";
    }
    return "unknown";
}

Result<Dictionary> Dictionary::create(std::vector<std::string> labels) {
    for (std::size_t index = 0; index < labels.size(); ++index) {
        auto validated = validate_text(labels[index], TextNulPolicy::reject);
        if (!validated.ok()) {
            return Result<Dictionary>::failure(validated.diagnostic_copy());
        }
        for (std::size_t earlier = 0; earlier < index; ++earlier) {
            if (labels[earlier] == labels[index]) {
                return Result<Dictionary>::failure(store_diagnostic(
                    DetailCode::duplicate_id, "dictionary labels must be distinct",
                    "label " + labels[index]));
            }
        }
    }
    Dictionary dictionary;
    dictionary.labels = std::move(labels);
    return dictionary;
}

std::optional<std::int64_t> Dictionary::find_code(std::string_view label) const noexcept {
    for (std::size_t index = 0; index < labels.size(); ++index) {
        if (labels[index] == label) {
            return static_cast<std::int64_t>(index);
        }
    }
    return std::nullopt;
}

Result<std::int64_t> Dictionary::code_for(std::string_view label) const {
    const auto code = find_code(label);
    if (!code.has_value()) {
        return Result<std::int64_t>::failure(
            store_diagnostic(DetailCode::unknown_category,
                             "category label is not in the declared dictionary", std::string{label}));
    }
    return code.value();
}

Result<std::string> Dictionary::label_for(std::int64_t code) const {
    if (code < 0 || static_cast<std::size_t>(code) >= labels.size()) {
        return Result<std::string>::failure(store_diagnostic(
            DetailCode::unknown_category, "category code is outside the dictionary",
            std::to_string(code)));
    }
    return labels[static_cast<std::size_t>(code)];
}

std::string FieldSchema::canonical_text() const {
    std::string text = to_string(id) + "|";
    text += to_string(type);
    text += "|";
    text += nullable ? "nullable" : "required";
    text += "|";
    text += unknown_category == UnknownCategoryPolicy::reject ? "reject" : "missing";
    text += "|unit=";
    text += unit.has_value() ? unit->canonical_text() : "none";
    text += "|zone=";
    text += zone_id.has_value() ? *zone_id : "none";
    text += "|dictionary=";
    if (dictionary != nullptr) {
        for (const auto& label : dictionary->labels) {
            text += label;
            text += ",";
        }
    }
    return text;
}

Result<Cell> FieldSchema::resolve_category(std::string_view label) const {
    if (type != LogicalType::categorical) {
        return Result<Cell>::failure(
            store_diagnostic(DetailCode::type_mismatch, "only a categorical field resolves labels",
                             to_string(id)));
    }
    if (dictionary == nullptr) {
        return Result<Cell>::failure(
            store_diagnostic(DetailCode::unknown_category, "a categorical field needs a dictionary",
                             to_string(id)));
    }
    const auto code = dictionary->find_code(label);
    if (code.has_value()) {
        return Cell{type, CategoryValue{code.value()}};
    }
    if (unknown_category == UnknownCategoryPolicy::missing) {
        return Cell::missing(type);
    }
    Location location;
    location.field = id;
    location.description = std::string{label};
    return Result<Cell>::failure(Diagnostic::reject(
        DetailCode::unknown_category,
        "unseen category is rejected by the declared policy instead of mapping to a code",
        std::move(location)));
}

Result<DatasetSchema> DatasetSchema::create(std::vector<FieldSchema> fields) {
    // An empty field list is a legitimate table shape: a worksheet can exist before any
    // column is created. A version, by contrast, must declare at least one field.
    DatasetSchema schema;
    schema.fields = std::move(fields);
    for (std::size_t index = 0; index < schema.fields.size(); ++index) {
        const FieldSchema& field = schema.fields[index];
        if (!field.id.is_valid()) {
            return Result<DatasetSchema>::failure(store_diagnostic(
                DetailCode::duplicate_id, "field identity must be a non-zero opaque id",
                field.name));
        }
        for (std::size_t earlier = 0; earlier < index; ++earlier) {
            if (schema.fields[earlier].id == field.id) {
                return Result<DatasetSchema>::failure(store_diagnostic(
                    DetailCode::duplicate_id, "field identity is already declared in this schema",
                    field.name));
            }
        }
        if (field.type == LogicalType::categorical && field.dictionary == nullptr) {
            return Result<DatasetSchema>::failure(store_diagnostic(
                DetailCode::unknown_category, "a categorical field must declare its dictionary",
                field.name));
        }
        // A zoned timestamp names its zone; no other temporal form accepts one, so a naive
        // value never silently acquires an offset.
        if (field.type == LogicalType::timestamp_zoned && !field.zone_id.has_value()) {
            Location location;
            location.field = field.id;
            return Result<DatasetSchema>::failure(Diagnostic::reject(
                DetailCode::unknown_zone,
                "a zoned timestamp field must declare its zone identifier", std::move(location)));
        }
        if (field.zone_id.has_value()) {
            Location location;
            location.field = field.id;
            location.description = *field.zone_id;
            if (field.type != LogicalType::timestamp_zoned) {
                return Result<DatasetSchema>::failure(Diagnostic::reject(
                    DetailCode::type_mismatch,
                    "a zone is declared only on a zoned timestamp field", std::move(location)));
            }
            if (!TimeZoneRules::builtin().contains(*field.zone_id)) {
                return Result<DatasetSchema>::failure(Diagnostic::reject(
                    DetailCode::unknown_zone,
                    "the declared zone is not in the versioned rule table", std::move(location)));
            }
        }
    }
    return schema;
}

const FieldSchema* DatasetSchema::find(FieldId id) const noexcept {
    for (const auto& field : fields) {
        if (field.id == id) {
            return &field;
        }
    }
    return nullptr;
}

std::string DatasetSchema::canonical_text() const {
    std::string text;
    for (const auto& field : fields) {
        text += field.canonical_text();
        text += ";";
    }
    return text;
}

Result<std::shared_ptr<const DatasetVersion>> DatasetVersion::create(
    DatasetVersionId id, DatasetId dataset, std::shared_ptr<const DatasetSchema> schema,
    RowDomainId row_domain, std::vector<std::shared_ptr<const Chunk>> chunks,
    Provenance provenance, std::uint64_t row_count, foundation::Revision revision) {
    if (!id.is_valid() || !dataset.is_valid()) {
        return Result<std::shared_ptr<const DatasetVersion>>::failure(store_diagnostic(
            DetailCode::duplicate_id, "dataset and version identities must be non-zero opaque ids",
            ""));
    }
    if (schema == nullptr || schema->fields.empty()) {
        return Result<std::shared_ptr<const DatasetVersion>>::failure(
            store_diagnostic(DetailCode::unknown_field, "a version needs a schema with fields", ""));
    }
    if (!row_domain.is_valid()) {
        return Result<std::shared_ptr<const DatasetVersion>>::failure(store_diagnostic(
            DetailCode::dangling_reference, "a version must name the row-identity domain it uses",
            ""));
    }
    if (provenance.operation_id.empty()) {
        return Result<std::shared_ptr<const DatasetVersion>>::failure(store_diagnostic(
            DetailCode::dangling_reference,
            "a version must state the operation that produced it", ""));
    }

    for (const auto& chunk : chunks) {
        if (chunk == nullptr) {
            return Result<std::shared_ptr<const DatasetVersion>>::failure(
                store_diagnostic(DetailCode::dangling_reference, "a null chunk cannot be published",
                                 ""));
        }
        const FieldSchema* field = schema->find(chunk->field());
        if (field == nullptr) {
            Location location;
            location.chunk = chunk->id();
            location.field = chunk->field();
            return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                DetailCode::unknown_field, "chunk names a field that the schema does not declare",
                std::move(location)));
        }
        if (chunk->type() != field->type) {
            Location location;
            location.chunk = chunk->id();
            location.field = field->id;
            return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                DetailCode::type_mismatch,
                std::string{"chunk storage type "} + to_string(chunk->type()) +
                    " does not match the declared field type " + to_string(field->type),
                std::move(location)));
        }
        if (chunk->row_count() == 0 && row_count > 0) {
            Location location;
            location.chunk = chunk->id();
            return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                DetailCode::resource_limit, "an empty chunk cannot cover a non-empty version",
                std::move(location)));
        }
    }

    // Each declared field must be covered exactly: contiguous chunks from row zero to the
    // version's row count, with no gap and no overlap.
    for (const auto& field : schema->fields) {
        std::vector<const Chunk*> covered;
        for (const auto& chunk : chunks) {
            if (chunk->field() == field.id) {
                covered.push_back(chunk.get());
            }
        }
        if (covered.empty()) {
            if (row_count == 0) {
                continue;
            }
            Location location;
            location.field = field.id;
            location.version = id;
            return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                DetailCode::unmapped_row, "a declared field has no chunk covering the version",
                std::move(location)));
        }
        std::sort(covered.begin(), covered.end(),
                  [](const Chunk* left, const Chunk* right) {
                      return left->first_row_index() < right->first_row_index();
                  });
        std::uint64_t expected = 0;
        for (const auto* chunk : covered) {
            if (chunk->first_row_index() != expected) {
                Location location;
                location.field = field.id;
                location.chunk = chunk->id();
                location.row_index = static_cast<std::int64_t>(chunk->first_row_index());
                return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                    DetailCode::unmapped_row,
                    "chunk ranges must be contiguous, so a row is never silently skipped or "
                    "duplicated",
                    std::move(location)));
            }
            expected += chunk_rows(*chunk);
        }
        if (expected != row_count) {
            Location location;
            location.field = field.id;
            location.version = id;
            return Result<std::shared_ptr<const DatasetVersion>>::failure(Diagnostic::reject(
                DetailCode::unmapped_row,
                "chunk coverage of " + std::to_string(expected) +
                    " rows does not match the declared row count " + std::to_string(row_count),
                std::move(location)));
        }
    }

    std::string canonical;
    canonical += to_string(id) + "|" + to_string(dataset) + "|" + to_string(row_domain) + "|" +
                 std::to_string(row_count) + "|" + std::to_string(revision) + "|" +
                 schema->canonical_text() + "|" + provenance.canonical_text() + "|";
    for (const auto& chunk : chunks) {
        canonical += to_string(chunk->id());
        canonical += ":";
        canonical += chunk->content_hash().hex();
        canonical += ",";
    }
    const ContentHash content = hash_bytes(canonical);

    // The constructor is private so a version can only exist in its validated form; the
    // shared pointer takes ownership immediately.
    return std::shared_ptr<const DatasetVersion>(
        new DatasetVersion(id, dataset, std::move(schema), row_domain, std::move(chunks),
                           std::move(provenance), row_count, revision, content));
}

std::vector<ChunkId> DatasetVersion::chunk_ids() const {
    std::vector<ChunkId> ids;
    ids.reserve(chunks_.size());
    for (const auto& chunk : chunks_) {
        ids.push_back(chunk->id());
    }
    return ids;
}

Result<std::shared_ptr<const Chunk>> DatasetVersion::chunk_for(FieldId field,
                                                              std::uint64_t row_index) const {
    for (const auto& chunk : chunks_) {
        if (chunk->field() != field) {
            continue;
        }
        const auto rows = chunk->row_count();
        if (row_index >= chunk->first_row_index() && row_index < chunk->first_row_index() + rows) {
            return chunk;
        }
    }
    Location location;
    location.field = field;
    location.version = id_;
    location.row_index = static_cast<std::int64_t>(row_index);
    return Result<std::shared_ptr<const Chunk>>::failure(
        Diagnostic::reject(DetailCode::unknown_field, "no chunk of this field covers the row index",
                           std::move(location)));
}

std::vector<std::shared_ptr<const Chunk>> DatasetVersion::chunks_of(FieldId field) const {
    std::vector<std::shared_ptr<const Chunk>> found;
    for (const auto& chunk : chunks_) {
        if (chunk->field() == field) {
            found.push_back(chunk);
        }
    }
    return found;
}

Result<Cell> DatasetVersion::cell(FieldId field, std::uint64_t row_index) const {
    if (field == FieldId{}) {
        Location location;
        location.version = id_;
        return Result<Cell>::failure(Diagnostic::reject(
            DetailCode::unknown_field, "a field identity is required to read a cell",
            std::move(location)));
    }
    if (schema_->find(field) == nullptr) {
        Location location;
        location.field = field;
        location.version = id_;
        return Result<Cell>::failure(Diagnostic::reject(
            DetailCode::unknown_field, "field is not declared in this version's schema",
            std::move(location)));
    }
    if (row_index >= row_count_) {
        Location location;
        location.field = field;
        location.version = id_;
        location.row_index = static_cast<std::int64_t>(row_index);
        return Result<Cell>::failure(Diagnostic::reject(
            DetailCode::index_out_of_range,
            "row index " + std::to_string(row_index) + " is outside the version's " +
                std::to_string(row_count_) + " rows",
            std::move(location)));
    }
    auto chunk = chunk_for(field, row_index);
    if (!chunk.ok()) {
        return Result<Cell>::failure(chunk.diagnostic_copy());
    }
    const std::uint64_t local = row_index - chunk.value()->first_row_index();
    return chunk.value()->cell_at(local);
}

DatasetLease::DatasetLease(std::shared_ptr<const DatasetVersion> version,
                           std::shared_ptr<const RowDomain> row_domain)
    : version_(std::move(version)), row_domain_(std::move(row_domain)) {}

Result<RowId> DatasetLease::row_at(std::uint64_t index) const {
    return row_domain_->row_at(index);
}

Result<std::uint64_t> DatasetLease::index_of(RowId row) const {
    const auto index = row_domain_->index_of(row);
    if (!index.has_value()) {
        Location location;
        location.row = row;
        location.description = "row domain " + to_string(row_domain_->id());
        return Result<std::uint64_t>::failure(Diagnostic::reject(
            DetailCode::dangling_reference, "row identity is not in this version's domain",
            std::move(location)));
    }
    return index.value();
}

Result<Cell> DatasetLease::cell_at(FieldId field, std::uint64_t row_index) const {
    return version_->cell(field, row_index);
}

Result<Cell> DatasetLease::cell(FieldId field, RowId row) const {
    auto index = index_of(row);
    if (!index.ok()) {
        return Result<Cell>::failure(index.diagnostic_copy());
    }
    return version_->cell(field, index.value());
}

Result<std::shared_ptr<const DatasetSchema>> DataStore::schema(std::vector<FieldSchema> fields) {
    auto created = DatasetSchema::create(std::move(fields));
    if (!created.ok()) {
        return Result<std::shared_ptr<const DatasetSchema>>::failure(created.diagnostic_copy());
    }
    return std::make_shared<const DatasetSchema>(created.take());
}

void DataStore::stage(std::uint64_t transaction_id, std::shared_ptr<const DatasetVersion> version) {
    if (version == nullptr) {
        return;
    }
    staged_[transaction_id].versions.push_back(std::move(version));
}

void DataStore::stage(std::uint64_t transaction_id, std::shared_ptr<const RowDomain> domain) {
    if (domain == nullptr) {
        return;
    }
    staged_[transaction_id].domains.push_back(std::move(domain));
}

void DataStore::stage(std::uint64_t transaction_id,
                      std::shared_ptr<const RowCorrespondence> correspondence) {
    if (correspondence == nullptr) {
        return;
    }
    staged_[transaction_id].correspondences.push_back(std::move(correspondence));
}

void DataStore::apply_commit(std::uint64_t transaction_id) {
    const auto found = staged_.find(transaction_id);
    if (found == staged_.end()) {
        return;
    }
    Staged pending = std::move(found->second);
    staged_.erase(found);
    for (auto& version : pending.versions) {
        versions_.push_back(std::move(version));
    }
    for (auto& domain : pending.domains) {
        domains_.push_back(std::move(domain));
    }
    for (auto& correspondence : pending.correspondences) {
        correspondences_.push_back(std::move(correspondence));
    }
}

void DataStore::discard(std::uint64_t transaction_id) {
    staged_.erase(transaction_id);
}

bool DataStore::contains(DatasetVersionId id) const noexcept {
    return std::any_of(versions_.begin(), versions_.end(),
                       [&](const auto& version) { return version->id() == id; });
}

bool DataStore::contains(RowDomainId id) const noexcept {
    return std::any_of(domains_.begin(), domains_.end(),
                       [&](const auto& domain) { return domain->id() == id; });
}

bool DataStore::contains(CorrespondenceId id) const noexcept {
    return std::any_of(correspondences_.begin(), correspondences_.end(),
                       [&](const auto& item) { return item->id() == id; });
}

Result<std::shared_ptr<const DatasetVersion>> DataStore::version(DatasetVersionId id) const {
    for (const auto& version : versions_) {
        if (version->id() == id) {
            return version;
        }
    }
    Location location;
    location.version = id;
    return Result<std::shared_ptr<const DatasetVersion>>::failure(
        Diagnostic::reject(DetailCode::dangling_reference, "dataset version is not in the store",
                           std::move(location)));
}

Result<std::shared_ptr<const RowDomain>> DataStore::row_domain(RowDomainId id) const {
    for (const auto& domain : domains_) {
        if (domain->id() == id) {
            return domain;
        }
    }
    Location location;
    location.description = "row domain " + to_string(id);
    return Result<std::shared_ptr<const RowDomain>>::failure(Diagnostic::reject(
        DetailCode::dangling_reference, "row domain is not in the store", std::move(location)));
}

Result<std::shared_ptr<const RowCorrespondence>> DataStore::correspondence(CorrespondenceId id) const {
    for (const auto& item : correspondences_) {
        if (item->id() == id) {
            return item;
        }
    }
    return Result<std::shared_ptr<const RowCorrespondence>>::failure(store_diagnostic(
        DetailCode::dangling_reference, "row correspondence is not in the store",
        "correspondence " + to_string(id)));
}

Result<DatasetLease> DataStore::lease(DatasetVersionId id) const {
    auto version = this->version(id);
    if (!version.ok()) {
        return Result<DatasetLease>::failure(version.diagnostic_copy());
    }
    auto domain = row_domain(version.value()->row_domain());
    if (!domain.ok()) {
        return Result<DatasetLease>::failure(domain.diagnostic_copy());
    }
    return DatasetLease{version.take(), domain.take()};
}

std::vector<DatasetVersionId> DataStore::versions_of(DatasetId dataset) const {
    std::vector<DatasetVersionId> found;
    for (const auto& version : versions_) {
        if (version->dataset() == dataset) {
            found.push_back(version->id());
        }
    }
    return found;
}

std::vector<DatasetVersionId> DataStore::versions_referencing(DatasetVersionId version) const {
    std::vector<DatasetVersionId> found;
    for (const auto& candidate : versions_) {
        const Provenance& provenance = candidate->provenance();
        bool referenced = false;
        for (const auto& input : provenance.inputs) {
            if (input.version == version) {
                referenced = true;
            }
        }
        for (const auto& parent : provenance.parents) {
            if (parent == version) {
                referenced = true;
            }
        }
        if (referenced) {
            found.push_back(candidate->id());
        }
    }
    return found;
}

std::uint64_t DataStore::staged_count() const noexcept {
    std::uint64_t count = 0;
    for (const auto& entry : staged_) {
        count += entry.second.versions.size() + entry.second.domains.size() +
                 entry.second.correspondences.size();
    }
    return count;
}

}  // namespace everygraph::data
