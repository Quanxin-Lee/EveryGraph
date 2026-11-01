#include "everygraph/data/document.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/value.hpp"

namespace everygraph::data {
namespace {

const application::Value* find_value(const application::ParameterMap& properties,
                                     std::string_view key) noexcept {
    const auto found = properties.find(key);
    if (found == properties.end()) {
        return nullptr;
    }
    return &found->second;
}

std::optional<std::uint64_t> parse_decimal(std::string_view text) noexcept {
    std::uint64_t value = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
    return value;
}

Location text_location(std::string text) {
    Location location;
    location.description = std::move(text);
    return location;
}

Diagnostic dangling(std::string context) {
    return Diagnostic::reject(DetailCode::dangling_reference, "document reference does not resolve",
                              text_location(std::move(context)));
}

Diagnostic missing_property(std::string_view key, const application::ObjectState& state) {
    return Diagnostic::reject(
        DetailCode::dangling_reference,
        std::string{"document object is missing the "} + std::string{key} + " property",
        text_location("object " + foundation::to_string(state.id) + " of type " + state.type));
}

template <typename Tag>
std::optional<StableId<Tag>> read_reference(const application::ObjectState& state,
                                            std::string_view key) {
    const auto value = read_id_property(state, key);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return StableId<Tag>{foundation::ObjectId{*value}};
}

using Objects = std::vector<std::reference_wrapper<const application::ObjectState>>;

std::string optional_text(const application::ObjectState& state, std::string_view key) {
    return read_text_property(state, key).value_or(std::string{});
}

std::uint64_t optional_count(const application::ObjectState& state, std::string_view key) {
    return read_count_property(state, key).value_or(0);
}

}  // namespace

const char* to_string(ColumnRole role) noexcept {
    switch (role) {
        case ColumnRole::unspecified:
            return "unspecified";
        case ColumnRole::x:
            return "x";
        case ColumnRole::y:
            return "y";
        case ColumnRole::z:
            return "z";
        case ColumnRole::error:
            return "error";
        case ColumnRole::label:
            return "label";
    }
    return "unspecified";
}

Result<ColumnRole> column_role_from_text(std::string_view text) {
    if (text == "unspecified") {
        return ColumnRole::unspecified;
    }
    if (text == "x") {
        return ColumnRole::x;
    }
    if (text == "y") {
        return ColumnRole::y;
    }
    if (text == "z") {
        return ColumnRole::z;
    }
    if (text == "error") {
        return ColumnRole::error;
    }
    if (text == "label") {
        return ColumnRole::label;
    }
    return Result<ColumnRole>::failure(Diagnostic::reject(
        DetailCode::type_mismatch, "role text is not one of the declared roles",
        text_location(std::string{text})));
}

const char* to_string(WritePolicy policy) noexcept {
    switch (policy) {
        case WritePolicy::mutable_column:
            return "mutable";
        case WritePolicy::mirror:
            return "mirror";
    }
    return "mutable";
}

Result<WritePolicy> write_policy_from_text(std::string_view text) {
    if (text == "mutable") {
        return WritePolicy::mutable_column;
    }
    if (text == "mirror") {
        return WritePolicy::mirror;
    }
    return Result<WritePolicy>::failure(Diagnostic::reject(
        DetailCode::type_mismatch, "write policy text is not one of the declared policies",
        text_location(std::string{text})));
}

const char* to_string(DomainLink link) noexcept {
    switch (link) {
        case DomainLink::same_domain:
            return "same_domain";
        case DomainLink::correspondence:
            return "correspondence";
    }
    return "same_domain";
}

Result<DomainLink> domain_link_from_text(std::string_view text) {
    if (text == "same_domain") {
        return DomainLink::same_domain;
    }
    if (text == "correspondence") {
        return DomainLink::correspondence;
    }
    return Result<DomainLink>::failure(Diagnostic::reject(
        DetailCode::type_mismatch, "link text is not one of the declared links",
        text_location(std::string{text})));
}

std::optional<std::uint64_t> read_id_property(const application::ObjectState& state,
                                              std::string_view key) noexcept {
    const auto* value = find_value(state.properties, key);
    if (value == nullptr) {
        return std::nullopt;
    }
    if (const auto* text = std::get_if<std::string>(value)) {
        return parse_decimal(*text);
    }
    if (const auto* number = std::get_if<std::int64_t>(value)) {
        if (*number < 0) {
            return std::nullopt;
        }
        return static_cast<std::uint64_t>(*number);
    }
    return std::nullopt;
}

std::optional<std::string> read_text_property(const application::ObjectState& state,
                                              std::string_view key) noexcept {
    const auto* value = find_value(state.properties, key);
    if (value == nullptr) {
        return std::nullopt;
    }
    if (const auto* text = std::get_if<std::string>(value)) {
        return *text;
    }
    return std::nullopt;
}

std::optional<std::uint64_t> read_count_property(const application::ObjectState& state,
                                                 std::string_view key) noexcept {
    return read_id_property(state, key);
}

Result<DocumentView> DocumentView::build(const application::DocumentSnapshot& snapshot,
                                         const DataStore& store) {
    DocumentView view;
    view.snapshot_ = snapshot;

    // The map is keyed by identity, so its iteration order is not a dependency order. One
    // bucket pass per object kind is what lets a binding's parent worksheet exist before the
    // binding is validated, whatever identities the caller allocated.
    Objects projects;
    Objects workbooks;
    Objects worksheets;
    Objects bindings;
    Objects version_objects;
    Objects domain_objects;
    Objects correspondence_objects;
    for (const auto& entry : snapshot.objects) {
        const application::ObjectState& state = entry.second;
        if (state.id != entry.first) {
            return Result<DocumentView>::failure(Diagnostic::reject(
                DetailCode::duplicate_id,
                "document key and stored object identity disagree, so the reference cannot be "
                "trusted",
                text_location(foundation::to_string(state.id) + " as " + state.type)));
        }
        if (state.type == object_type::kProject) {
            projects.push_back(std::cref(state));
        } else if (state.type == object_type::kWorkbook) {
            workbooks.push_back(std::cref(state));
        } else if (state.type == object_type::kWorksheet) {
            worksheets.push_back(std::cref(state));
        } else if (state.type == object_type::kColumnBinding) {
            bindings.push_back(std::cref(state));
        } else if (state.type == object_type::kDatasetVersion) {
            version_objects.push_back(std::cref(state));
        } else if (state.type == object_type::kRowDomain) {
            domain_objects.push_back(std::cref(state));
        } else if (state.type == object_type::kRowCorrespondence) {
            correspondence_objects.push_back(std::cref(state));
        }
        // An object type this reader does not know is another module's state: the kernel
        // keeps it and this view ignores it rather than guessing a shape for it.
    }

    for (const auto& holder : domain_objects) {
        const application::ObjectState& state = holder.get();
        const RowDomainId id{state.id};
        if (!store.contains(id)) {
            return Result<DocumentView>::failure(
                dangling("row domain " + foundation::to_string(state.id)));
        }
        view.domains_[id] = state.revision;
    }

    for (const auto& holder : version_objects) {
        const application::ObjectState& state = holder.get();
        const auto version = read_reference<DatasetVersionTag>(state, property::kVersion);
        const auto domain = read_reference<RowDomainTag>(state, property::kRowDomain);
        if (!version.has_value()) {
            return Result<DocumentView>::failure(missing_property(property::kVersion, state));
        }
        if (!domain.has_value()) {
            return Result<DocumentView>::failure(missing_property(property::kRowDomain, state));
        }
        if (!store.contains(*version)) {
            return Result<DocumentView>::failure(
                dangling("dataset version " + to_string(*version) + " is not held by the store"));
        }
        if (!store.contains(*domain)) {
            return Result<DocumentView>::failure(dangling("dataset version " + to_string(*version) +
                                                          " names a row domain the store does "
                                                          "not hold"));
        }
        view.versions_[*version] = state.revision;
    }

    for (const auto& holder : correspondence_objects) {
        const application::ObjectState& state = holder.get();
        const CorrespondenceId id{state.id};
        const auto target = read_reference<RowDomainTag>(state, property::kTargetDomain);
        const auto source = read_reference<RowDomainTag>(state, property::kSourceDomain);
        if (!target.has_value()) {
            return Result<DocumentView>::failure(missing_property(property::kTargetDomain, state));
        }
        if (!source.has_value()) {
            return Result<DocumentView>::failure(missing_property(property::kSourceDomain, state));
        }
        if (!store.contains(id) || !store.contains(*target) || !store.contains(*source)) {
            return Result<DocumentView>::failure(
                dangling("correspondence " + foundation::to_string(state.id) + " or one of its domains"));
        }
        view.correspondences_[id] = state.revision;
    }

    for (const auto& holder : projects) {
        const application::ObjectState& state = holder.get();
        ProjectRecord record;
        record.id = ProjectId{state.id};
        record.revision = state.revision;
        record.label = optional_text(state, property::kLabel);
        view.projects_[record.id] = record;
    }

    for (const auto& holder : workbooks) {
        const application::ObjectState& state = holder.get();
        const auto parent = read_reference<ProjectTag>(state, property::kParent);
        if (!parent.has_value() || view.projects_.find(*parent) == view.projects_.end()) {
            return Result<DocumentView>::failure(
                dangling("workbook " + foundation::to_string(state.id) + " parent project"));
        }
        WorkbookRecord record;
        record.id = WorkbookId{state.id};
        record.project = *parent;
        record.revision = state.revision;
        record.label = optional_text(state, property::kLabel);
        record.order_index = optional_count(state, property::kOrderIndex);
        view.workbooks_[record.id] = record;
    }

    for (const auto& holder : worksheets) {
        const application::ObjectState& state = holder.get();
        const auto parent = read_reference<WorkbookTag>(state, property::kParent);
        if (!parent.has_value() || view.workbooks_.find(*parent) == view.workbooks_.end()) {
            return Result<DocumentView>::failure(
                dangling("worksheet " + foundation::to_string(state.id) + " parent workbook"));
        }
        const auto domain = read_reference<RowDomainTag>(state, property::kRowDomain);
        if (!domain.has_value() || !store.contains(*domain)) {
            return Result<DocumentView>::failure(
                dangling("worksheet " + foundation::to_string(state.id) + " row domain"));
        }
        WorksheetRecord record;
        record.id = WorksheetId{state.id};
        record.workbook = *parent;
        record.row_domain = *domain;
        record.revision = state.revision;
        record.label = optional_text(state, property::kLabel);
        record.order_index = optional_count(state, property::kOrderIndex);
        view.worksheets_[record.id] = record;
    }

    for (const auto& holder : bindings) {
        const application::ObjectState& state = holder.get();
        const auto worksheet = read_reference<WorksheetTag>(state, property::kParent);
        const auto version = read_reference<DatasetVersionTag>(state, property::kVersion);
        const auto dataset = read_reference<DatasetTag>(state, property::kDataset);
        if (!worksheet.has_value() || view.worksheets_.find(*worksheet) == view.worksheets_.end()) {
            return Result<DocumentView>::failure(
                dangling("binding " + foundation::to_string(state.id) + " parent worksheet"));
        }
        if (!version.has_value() || !dataset.has_value() || !store.contains(*version)) {
            return Result<DocumentView>::failure(
                dangling("binding " + foundation::to_string(state.id) + " dataset version or dataset"));
        }

        ColumnBindingRecord record;
        record.id = ColumnId{state.id};
        record.worksheet = *worksheet;
        record.version = *version;
        record.dataset = *dataset;
        record.revision = state.revision;
        record.label = optional_text(state, property::kLabel);
        record.order_index = optional_count(state, property::kOrderIndex);
        record.display_format = optional_text(state, property::kDisplayFormat);

        if (const auto role_text = read_text_property(state, property::kRole);
            role_text.has_value()) {
            auto role = column_role_from_text(*role_text);
            if (!role.ok()) {
                return Result<DocumentView>::failure(role.diagnostic_copy());
            }
            record.role = role.value();
        }
        if (const auto policy_text = read_text_property(state, property::kWritePolicy);
            policy_text.has_value()) {
            auto policy = write_policy_from_text(*policy_text);
            if (!policy.ok()) {
                return Result<DocumentView>::failure(policy.diagnostic_copy());
            }
            record.write_policy = policy.value();
        }
        if (const auto link_text = read_text_property(state, property::kLink);
            link_text.has_value()) {
            auto parsed = domain_link_from_text(*link_text);
            if (!parsed.ok()) {
                return Result<DocumentView>::failure(parsed.diagnostic_copy());
            }
            record.link = parsed.value();
        }
        if (const auto correspondence =
                read_reference<CorrespondenceTag>(state, property::kCorrespondence);
            correspondence.has_value()) {
            record.correspondence = correspondence;
        }
        if (const auto unit_text = read_text_property(state, property::kUnit);
            unit_text.has_value() && !unit_text->empty()) {
            auto parsed = UnitRegistry::builtin().parse(*unit_text);
            if (!parsed.ok()) {
                Diagnostic diagnostic = parsed.diagnostic_copy();
                Location location;
                location.column = record.id;
                location.description = *unit_text;
                diagnostic.location = std::move(location);
                return Result<DocumentView>::failure(diagnostic);
            }
            record.unit = parsed.take();
        }

        auto stored = store.version(record.version);
        if (!stored.ok()) {
            return Result<DocumentView>::failure(stored.diagnostic_copy());
        }
        const RowDomainId worksheet_domain = view.worksheets_[record.worksheet].row_domain;
        if (record.link == DomainLink::correspondence && !record.correspondence.has_value()) {
            Location location;
            location.column = record.id;
            location.version = record.version;
            return Result<DocumentView>::failure(Diagnostic::reject(
                DetailCode::positional_mapping_required,
                "a foreign-domain binding must carry an explicit correspondence",
                std::move(location)));
        }
        if (record.link == DomainLink::same_domain) {
            if (stored.value()->row_domain() != worksheet_domain) {
                Location location;
                location.column = record.id;
                location.version = record.version;
                location.description = "worksheet domain " + to_string(worksheet_domain) +
                                       " and version domain " +
                                       to_string(stored.value()->row_domain());
                return Result<DocumentView>::failure(Diagnostic::reject(
                    DetailCode::unmapped_row,
                    "a same-domain binding points at a version of another row domain, and an "
                    "unrelated column is never zipped by position",
                    std::move(location)));
            }
        } else if (record.correspondence.has_value()) {
            auto mapping = store.correspondence(*record.correspondence);
            if (!mapping.ok()) {
                return Result<DocumentView>::failure(mapping.diagnostic_copy());
            }
            if (mapping.value()->target_domain() != worksheet_domain ||
                mapping.value()->source_domain() != stored.value()->row_domain()) {
                Location location;
                location.column = record.id;
                location.description =
                    "correspondence " + to_string(*record.correspondence) + " pairs " +
                    to_string(mapping.value()->target_domain()) + " with " +
                    to_string(mapping.value()->source_domain());
                return Result<DocumentView>::failure(Diagnostic::reject(
                    DetailCode::dangling_reference,
                    "the correspondence does not pair this worksheet's domain with the version's "
                    "domain",
                    std::move(location)));
            }
        }
        view.bindings_[record.id] = record;
    }
    return view;
}

const ProjectRecord* DocumentView::find(ProjectId id) const noexcept {
    const auto found = projects_.find(id);
    return found == projects_.end() ? nullptr : &found->second;
}

const WorkbookRecord* DocumentView::find(WorkbookId id) const noexcept {
    const auto found = workbooks_.find(id);
    return found == workbooks_.end() ? nullptr : &found->second;
}

const WorksheetRecord* DocumentView::find(WorksheetId id) const noexcept {
    const auto found = worksheets_.find(id);
    return found == worksheets_.end() ? nullptr : &found->second;
}

const ColumnBindingRecord* DocumentView::find(ColumnId id) const noexcept {
    const auto found = bindings_.find(id);
    return found == bindings_.end() ? nullptr : &found->second;
}

namespace {

template <typename Record, typename Key>
std::vector<Key> ordered(const std::map<Key, Record>& records,
                         const std::function<bool(const Record&)>& belongs) {
    std::vector<const Record*> selected;
    for (const auto& entry : records) {
        if (belongs(entry.second)) {
            selected.push_back(&entry.second);
        }
    }
    std::sort(selected.begin(), selected.end(), [](const Record* left, const Record* right) {
        if (left->order_index != right->order_index) {
            return left->order_index < right->order_index;
        }
        return left->id < right->id;
    });
    std::vector<Key> ids;
    ids.reserve(selected.size());
    for (const auto* record : selected) {
        ids.push_back(record->id);
    }
    return ids;
}

}  // namespace

std::vector<WorkbookId> DocumentView::workbooks_of(ProjectId project) const {
    return ordered<WorkbookRecord, WorkbookId>(
        workbooks_, [&](const WorkbookRecord& record) { return record.project == project; });
}

std::vector<WorksheetId> DocumentView::worksheets_of(WorkbookId workbook) const {
    return ordered<WorksheetRecord, WorksheetId>(
        worksheets_, [&](const WorksheetRecord& record) { return record.workbook == workbook; });
}

std::vector<ColumnBindingRecord> DocumentView::bindings_of(WorksheetId worksheet) const {
    std::vector<const ColumnBindingRecord*> selected;
    for (const auto& entry : bindings_) {
        if (entry.second.worksheet == worksheet) {
            selected.push_back(&entry.second);
        }
    }
    std::sort(selected.begin(), selected.end(),
              [](const ColumnBindingRecord* left, const ColumnBindingRecord* right) {
                  if (left->order_index != right->order_index) {
                      return left->order_index < right->order_index;
                  }
                  return left->id < right->id;
              });
    std::vector<ColumnBindingRecord> records;
    records.reserve(selected.size());
    for (const auto* record : selected) {
        records.push_back(*record);
    }
    return records;
}

std::vector<ColumnId> DocumentView::columns_using(DatasetVersionId version) const {
    std::vector<ColumnId> found;
    for (const auto& entry : bindings_) {
        if (entry.second.version == version) {
            found.push_back(entry.first);
        }
    }
    return found;
}

std::vector<std::string> DocumentView::dependents_of(DatasetVersionId version,
                                                     const DataStore& store) const {
    std::vector<std::string> dependents;
    for (const auto& column : columns_using(version)) {
        dependents.push_back("binding " + to_string(column));
    }
    for (const auto& derived : store.versions_referencing(version)) {
        dependents.push_back("derived version " + to_string(derived));
    }
    return dependents;
}

}  // namespace everygraph::data
