#pragma once

#include <compare>
#include <cstdint>
#include <string>

#include "everygraph/foundation/ids.hpp"

namespace everygraph::data {

// Every document, dataset, chunk and row identity is a distinct opaque tag over a
// foundation ObjectId. Array position, row number, display label and translated name
// are never identities, so a struct of this form is the only identity the public
// surfaces expose.
template <typename Tag>
class StableId {
public:
    constexpr StableId() = default;

    constexpr explicit StableId(foundation::ObjectId id) noexcept : id_(id) {}

    [[nodiscard]] constexpr foundation::ObjectId object_id() const noexcept { return id_; }
    [[nodiscard]] constexpr std::uint64_t raw() const noexcept { return id_.value; }
    [[nodiscard]] constexpr bool is_valid() const noexcept { return id_.is_valid(); }

    friend constexpr bool operator==(const StableId&, const StableId&) noexcept = default;
    friend constexpr auto operator<=>(const StableId&, const StableId&) noexcept = default;

private:
    foundation::ObjectId id_{};
};

struct ProjectTag {};
struct WorkbookTag {};
struct WorksheetTag {};
struct ColumnTag {};
struct DatasetTag {};
struct DatasetVersionTag {};
struct ChunkTag {};
struct FieldTag {};
struct RowTag {};
struct RowDomainTag {};
struct CorrespondenceTag {};

using ProjectId = StableId<ProjectTag>;
using WorkbookId = StableId<WorkbookTag>;
using WorksheetId = StableId<WorksheetTag>;
using ColumnId = StableId<ColumnTag>;
using DatasetId = StableId<DatasetTag>;
using DatasetVersionId = StableId<DatasetVersionTag>;
using ChunkId = StableId<ChunkTag>;
using FieldId = StableId<FieldTag>;
using RowId = StableId<RowTag>;
using RowDomainId = StableId<RowDomainTag>;
using CorrespondenceId = StableId<CorrespondenceTag>;

template <typename Tag>
[[nodiscard]] std::string to_string(const StableId<Tag>& id) {
    return foundation::to_string(id.object_id());
}

[[nodiscard]] inline ProjectId project_id(std::uint64_t value) {
    return ProjectId{foundation::ObjectId{value}};
}

[[nodiscard]] inline WorkbookId workbook_id(std::uint64_t value) {
    return WorkbookId{foundation::ObjectId{value}};
}

[[nodiscard]] inline WorksheetId worksheet_id(std::uint64_t value) {
    return WorksheetId{foundation::ObjectId{value}};
}

[[nodiscard]] inline ColumnId column_id(std::uint64_t value) {
    return ColumnId{foundation::ObjectId{value}};
}

[[nodiscard]] inline DatasetId dataset_id(std::uint64_t value) {
    return DatasetId{foundation::ObjectId{value}};
}

[[nodiscard]] inline DatasetVersionId dataset_version_id(std::uint64_t value) {
    return DatasetVersionId{foundation::ObjectId{value}};
}

[[nodiscard]] inline ChunkId chunk_id(std::uint64_t value) {
    return ChunkId{foundation::ObjectId{value}};
}

[[nodiscard]] inline FieldId field_id(std::uint64_t value) {
    return FieldId{foundation::ObjectId{value}};
}

[[nodiscard]] inline RowId row_id(std::uint64_t value) {
    return RowId{foundation::ObjectId{value}};
}

[[nodiscard]] inline RowDomainId row_domain_id(std::uint64_t value) {
    return RowDomainId{foundation::ObjectId{value}};
}

[[nodiscard]] inline CorrespondenceId correspondence_id(std::uint64_t value) {
    return CorrespondenceId{foundation::ObjectId{value}};
}

}  // namespace everygraph::data
