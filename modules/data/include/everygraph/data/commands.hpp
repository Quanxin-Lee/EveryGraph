#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/command.hpp"
#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/data/dataset.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/document.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/row_domain.hpp"

namespace everygraph::data {

// Command names follow the `namespace.verb` convention of the application contract.
namespace command_id {
inline constexpr const char* kProjectCreate = "data.project_create";
inline constexpr const char* kWorkbookCreate = "data.workbook_create";
inline constexpr const char* kWorksheetCreate = "data.worksheet_create";
inline constexpr const char* kWorksheetChangeRows = "data.worksheet_change_rows";
inline constexpr const char* kColumnCreate = "data.column_create";
inline constexpr const char* kColumnEdit = "data.column_edit";
inline constexpr const char* kColumnRename = "data.column_rename";
inline constexpr const char* kColumnMove = "data.column_move";
inline constexpr const char* kColumnMetadata = "data.column_metadata";
inline constexpr const char* kColumnBindForeign = "data.column_bind_foreign";
inline constexpr const char* kCorrespondenceCreate = "data.correspondence_create";
inline constexpr const char* kDatasetDerive = "data.dataset_derive";
inline constexpr const char* kColumnDelete = "data.column_delete";
inline constexpr const char* kVersionDelete = "data.dataset_version_delete";
}  // namespace command_id

namespace argument {
inline constexpr const char* kProject = "project_id";
inline constexpr const char* kWorkbook = "workbook_id";
inline constexpr const char* kWorksheet = "worksheet_id";
inline constexpr const char* kColumn = "column_id";
inline constexpr const char* kDataset = "dataset_id";
inline constexpr const char* kVersion = "version_id";
inline constexpr const char* kDomain = "row_domain_id";
inline constexpr const char* kCorrespondence = "correspondence_id";
inline constexpr const char* kLabel = "label";
inline constexpr const char* kRole = "role";
inline constexpr const char* kUnit = "unit";
inline constexpr const char* kDisplayFormat = "display_format";
inline constexpr const char* kOrderIndex = "order_index";
inline constexpr const char* kLink = "link";
inline constexpr const char* kWritePolicy = "write_policy";
inline constexpr const char* kOperation = "operation";
inline constexpr const char* kColumns = "columns";
inline constexpr const char* kVersionDrafts = "version_drafts";
inline constexpr const char* kDomainDraft = "domain_draft";
inline constexpr const char* kVersionDraft = "version_draft";
inline constexpr const char* kCorrespondenceDraft = "correspondence_draft";
inline constexpr const char* kSourceVersion = "source_version_id";
inline constexpr const char* kSourceField = "source_field_id";
inline constexpr const char* kKeyColumn = "key_column_id";
inline constexpr const char* kMethod = "method";
inline constexpr const char* kApproved = "approved";
inline constexpr const char* kInputA = "input_version_a";
inline constexpr const char* kInputB = "input_version_b";
inline constexpr const char* kParameters = "canonical_parameters";
inline constexpr const char* kRows = "rows";
}  // namespace argument

using DraftId = std::uint64_t;
inline constexpr DraftId kNoDraft = 0;

// Private candidate output. A worker, an import or an editor prepares a version, a row
// domain or a correspondence here, and a command can only publish what the executor commits;
// a rejected command leaves the draft available for the corrected retry.
class DraftRegistry {
public:
    [[nodiscard]] DraftId add(std::shared_ptr<const DatasetVersion> version);
    [[nodiscard]] DraftId add(std::shared_ptr<const RowDomain> domain);
    [[nodiscard]] DraftId add(std::shared_ptr<const RowCorrespondence> correspondence);

    [[nodiscard]] std::shared_ptr<const DatasetVersion> version(DraftId id) const;
    [[nodiscard]] std::shared_ptr<const RowDomain> domain(DraftId id) const;
    [[nodiscard]] std::shared_ptr<const RowCorrespondence> correspondence(DraftId id) const;

    void drop(DraftId id);
    [[nodiscard]] std::size_t size() const noexcept;

private:
    struct Entry {
        std::shared_ptr<const DatasetVersion> version;
        std::shared_ptr<const RowDomain> domain;
        std::shared_ptr<const RowCorrespondence> correspondence;
    };

    DraftId insert(Entry entry);

    std::vector<Entry> entries_;
};

// Registers the data-module command family on the P1-004 kernel. Every mutation is a
// versioned command with declared schema, effect class, repeatability and argument
// validation, so no caller reaches the store around the gateway.
[[nodiscard]] std::optional<Diagnostic> register_data_commands(application::CommandRegistry& registry,
                                                              DataStore& store,
                                                              const DraftRegistry& drafts);

// The executor is the only writer: it runs the gateway, then adopts the staged candidates of
// the transactions that actually committed and drops the staged candidates of the ones that
// did not. Staging alone never changes what a reader can see.
class DataCommandExecutor {
public:
    DataCommandExecutor(application::CommandGateway& gateway, DataStore& store);

    [[nodiscard]] application::CommandOutcome execute(const application::CommandRequest& request,
                                                      std::stop_token stop_token = {});
    [[nodiscard]] application::CommandOutcome undo(std::stop_token stop_token = {});
    [[nodiscard]] application::CommandOutcome redo(std::stop_token stop_token = {});

    [[nodiscard]] Result<DocumentView> document() const;
    [[nodiscard]] const application::DocumentSnapshot snapshot() const;
    [[nodiscard]] const DataStore& store() const noexcept { return store_; }
    [[nodiscard]] application::CommandGateway& gateway() noexcept { return gateway_; }

private:
    application::CommandGateway& gateway_;
    DataStore& store_;
};

}  // namespace everygraph::data
