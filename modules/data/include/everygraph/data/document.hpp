#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/document.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/dataset.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/row_domain.hpp"
#include "everygraph/data/units.hpp"

namespace everygraph::data {

// Object type names recorded in the command kernel's document state.
namespace object_type {
inline constexpr const char* kProject = "data.project";
inline constexpr const char* kWorkbook = "data.workbook";
inline constexpr const char* kWorksheet = "data.worksheet";
inline constexpr const char* kColumnBinding = "data.column_binding";
inline constexpr const char* kDatasetVersion = "data.dataset_version";
inline constexpr const char* kRowDomain = "data.row_domain";
inline constexpr const char* kRowCorrespondence = "data.row_correspondence";
}  // namespace object_type

// Property keys shared by the command handlers and this reader. An id property holds the
// decimal text of an opaque identity, because a display label or an array position is never
// a reference.
namespace property {
inline constexpr const char* kLabel = "label";
inline constexpr const char* kParent = "parent_id";
inline constexpr const char* kOrderIndex = "order_index";
inline constexpr const char* kRole = "role";
inline constexpr const char* kUnit = "unit";
inline constexpr const char* kDisplayFormat = "display_format";
inline constexpr const char* kWritePolicy = "write_policy";
inline constexpr const char* kRowDomain = "row_domain_id";
inline constexpr const char* kDataset = "dataset_id";
inline constexpr const char* kVersion = "version_id";
inline constexpr const char* kPreviousVersion = "previous_version_id";
inline constexpr const char* kLink = "link";
inline constexpr const char* kCorrespondence = "correspondence_id";
inline constexpr const char* kRowCount = "row_count";
inline constexpr const char* kLineageIndex = "lineage_index";
inline constexpr const char* kContentHash = "content_hash";
inline constexpr const char* kTargetDomain = "target_domain_id";
inline constexpr const char* kSourceDomain = "source_domain_id";
inline constexpr const char* kMethod = "method";
}  // namespace property

enum class ColumnRole {
    unspecified,
    x,
    y,
    z,
    error,
    label,
};

[[nodiscard]] const char* to_string(ColumnRole role) noexcept;
[[nodiscard]] Result<ColumnRole> column_role_from_text(std::string_view text);

// A mirror is a binding to another column's identity, not duplicate storage, and it is
// read-only until an explicit shared-edit or detach command changes the policy.
enum class WritePolicy {
    mutable_column,
    mirror,
};

[[nodiscard]] const char* to_string(WritePolicy policy) noexcept;
[[nodiscard]] Result<WritePolicy> write_policy_from_text(std::string_view text);

// How a binding's rows line up with the rows of the version it points at.
enum class DomainLink {
    same_domain,
    correspondence,
};

[[nodiscard]] const char* to_string(DomainLink link) noexcept;
[[nodiscard]] Result<DomainLink> domain_link_from_text(std::string_view text);

[[nodiscard]] std::optional<std::uint64_t> read_id_property(const application::ObjectState& state,
                                                            std::string_view key) noexcept;
[[nodiscard]] std::optional<std::string> read_text_property(
    const application::ObjectState& state, std::string_view key) noexcept;
[[nodiscard]] std::optional<std::uint64_t> read_count_property(
    const application::ObjectState& state, std::string_view key) noexcept;

struct ProjectRecord {
    ProjectId id;
    std::string label;
    foundation::Revision revision{foundation::kInitialRevision};
};

struct WorkbookRecord {
    WorkbookId id;
    ProjectId project;
    std::string label;
    std::uint64_t order_index{0};
    foundation::Revision revision{foundation::kInitialRevision};
};

struct WorksheetRecord {
    WorksheetId id;
    WorkbookId workbook;
    std::string label;
    RowDomainId row_domain;
    std::uint64_t order_index{0};
    foundation::Revision revision{foundation::kInitialRevision};
};

struct ColumnBindingRecord {
    ColumnId id;
    WorksheetId worksheet;
    std::string label;
    std::uint64_t order_index{0};
    ColumnRole role{ColumnRole::unspecified};
    DatasetId dataset;
    DatasetVersionId version;
    DomainLink link{DomainLink::same_domain};
    std::optional<CorrespondenceId> correspondence;
    WritePolicy write_policy{WritePolicy::mutable_column};
    std::optional<Unit> unit;
    std::string display_format;
    foundation::Revision revision{foundation::kInitialRevision};
};

// The typed reading of the kernel's document state, with the reference invariants the data
// model requires. Building a view is the validation pass: a dangling, mistyped or zipped
// reference is reported rather than quietly re-keyed.
class DocumentView {
public:
    [[nodiscard]] static Result<DocumentView> build(const application::DocumentSnapshot& snapshot,
                                                    const DataStore& store);

    [[nodiscard]] const ProjectRecord* find(ProjectId id) const noexcept;
    [[nodiscard]] const WorkbookRecord* find(WorkbookId id) const noexcept;
    [[nodiscard]] const WorksheetRecord* find(WorksheetId id) const noexcept;
    [[nodiscard]] const ColumnBindingRecord* find(ColumnId id) const noexcept;

    [[nodiscard]] std::vector<WorkbookId> workbooks_of(ProjectId project) const;
    [[nodiscard]] std::vector<WorksheetId> worksheets_of(WorkbookId workbook) const;
    [[nodiscard]] std::vector<ColumnBindingRecord> bindings_of(WorksheetId worksheet) const;
    [[nodiscard]] std::vector<ColumnId> columns_using(DatasetVersionId version) const;

    // Who still points at a version: bindings and derived versions. A delete that would
    // strand one of these is rejected naming them.
    [[nodiscard]] std::vector<std::string> dependents_of(DatasetVersionId version,
                                                         const DataStore& store) const;

    [[nodiscard]] std::size_t object_count() const noexcept { return snapshot_.objects.size(); }
    [[nodiscard]] const application::DocumentSnapshot& snapshot() const noexcept {
        return snapshot_;
    }

private:
    application::DocumentSnapshot snapshot_;
    std::map<ProjectId, ProjectRecord> projects_;
    std::map<WorkbookId, WorkbookRecord> workbooks_;
    std::map<WorksheetId, WorksheetRecord> worksheets_;
    std::map<ColumnId, ColumnBindingRecord> bindings_;
    std::map<DatasetVersionId, foundation::Revision> versions_;
    std::map<RowDomainId, foundation::Revision> domains_;
    std::map<CorrespondenceId, foundation::Revision> correspondences_;
};

}  // namespace everygraph::data
