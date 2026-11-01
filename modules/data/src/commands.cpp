#include "everygraph/data/commands.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/document.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::data {
namespace {

constexpr std::uint32_t kSchemaVersion = 1;

using Error = foundation::Error;
using ObjectId = foundation::ObjectId;
using ParameterMap = application::ParameterMap;
using CommandTransaction = application::CommandTransaction;
using CommandRequest = application::CommandRequest;
using ExecutionContext = application::ExecutionContext;
using ObjectState = application::ObjectState;
using Value = application::Value;

Diagnostic reject(DetailCode code, std::string message, std::string context = {}) {
    Location location;
    location.description = std::move(context);
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

std::optional<Error> fail(DetailCode code, std::string message, std::string context = {}) {
    return reject(code, std::move(message), std::move(context)).to_error();
}

std::optional<Error> from_kernel(const std::optional<Error>& error, DetailCode code,
                                std::string context) {
    if (!error.has_value()) {
        return std::nullopt;
    }
    return fail(code, error->message, std::move(context));
}

// Reads of the command envelope's typed arguments. An identity travels as the decimal text of
// an opaque id, so no argument is ever a display label or an array position.
class Args {
public:
    explicit Args(const ParameterMap& values) : values_(values) {}

    std::optional<Error> id(std::string_view key, std::uint64_t& out, bool required = true) const {
        std::string text;
        if (auto error = this->text(key, text, required)) {
            return error;
        }
        if (text.empty()) {
            return std::nullopt;
        }
        std::uint64_t value = 0;
        const char* begin = text.data();
        const char* end = text.data() + text.size();
        const auto parsed = std::from_chars(begin, end, value);
        if (parsed.ec != std::errc{} || parsed.ptr != end) {
            return fail(DetailCode::type_mismatch,
                        "identity argument is not the decimal text of an opaque id",
                        std::string{key});
        }
        out = value;
        return std::nullopt;
    }

    std::optional<Error> text(std::string_view key, std::string& out, bool required = true) const {
        const auto found = values_.find(key);
        if (found == values_.end()) {
            if (required) {
                return fail(DetailCode::type_mismatch, "required argument is missing",
                            std::string{key});
            }
            return std::nullopt;
        }
        const auto* text = std::get_if<std::string>(&found->second);
        if (text == nullptr) {
            return fail(DetailCode::type_mismatch, "argument is not text", std::string{key});
        }
        out = *text;
        return std::nullopt;
    }

    std::optional<Error> optional_text(std::string_view key, std::optional<std::string>& out) const {
        const auto found = values_.find(key);
        if (found == values_.end()) {
            return std::nullopt;
        }
        const auto* text = std::get_if<std::string>(&found->second);
        if (text == nullptr) {
            return fail(DetailCode::type_mismatch, "argument is not text", std::string{key});
        }
        out = *text;
        return std::nullopt;
    }

    bool contains(std::string_view key) const { return values_.contains(key); }

    std::optional<Error> ids(std::string_view key, std::vector<std::uint64_t>& out) const {
        out.clear();
        std::string text;
        if (auto error = this->text(key, text)) {
            return error;
        }
        if (text.empty()) {
            return std::nullopt;
        }
        std::size_t start = 0;
        while (true) {
            const auto separator = text.find(';', start);
            const std::size_t stop = separator == std::string::npos ? text.size() : separator;
            std::uint64_t value = 0;
            const char* begin = text.data() + start;
            const char* end = text.data() + stop;
            const auto parsed = std::from_chars(begin, end, value);
            if (start >= stop || parsed.ec != std::errc{} || parsed.ptr != end) {
                return fail(DetailCode::type_mismatch,
                            "id list entry is not the decimal text of an opaque id",
                            std::string{text});
            }
            out.push_back(value);
            if (separator == std::string_view::npos) {
                break;
            }
            start = separator + 1;
        }
        return std::nullopt;
    }

    std::optional<Error> all_ids(std::initializer_list<std::string_view> keys) const {
        for (std::string_view key : keys) {
            std::uint64_t value = 0;
            if (auto error = this->id(key, value)) {
                return error;
            }
        }
        return std::nullopt;
    }

private:
    const ParameterMap& values_;
};

void set_id(ParameterMap& properties, std::string_view key, std::uint64_t value) {
    properties[std::string{key}] = Value{std::to_string(value)};
}

void set_text(ParameterMap& properties, std::string_view key, std::string value) {
    properties[std::string{key}] = Value{std::move(value)};
}

std::optional<Error> check_unique(const CommandTransaction& transaction, std::uint64_t id,
                                 std::string context) {
    const auto* existing = transaction.find(ObjectId{id});
    if (existing == nullptr) {
        return std::nullopt;
    }
    // Both sides of the clash are named: the identity the caller attempted and the live object
    // that already holds it, with its revision.
    return fail(DetailCode::duplicate_id,
                "this identity is already in use and identities are never reused",
                std::move(context) + "; existing object " +
                    foundation::to_string(ObjectId{id}) + " of type " + existing->type +
                    " at revision " + std::to_string(existing->revision));
}

std::optional<Error> create_object(CommandTransaction& transaction, std::uint64_t id,
                                  std::string_view type, ParameterMap properties,
                                  std::string context) {
    if (id == 0) {
        return fail(DetailCode::duplicate_id, "identity must be a non-zero opaque id",
                    std::move(context));
    }
    auto error = transaction.create_object(ObjectId{id}, std::string{type}, std::move(properties));
    if (error.has_value()) {
        return fail(DetailCode::duplicate_id, error->message, std::move(context));
    }
    return std::nullopt;
}

std::optional<Error> require_object(const CommandTransaction& transaction, std::uint64_t id,
                                    std::string_view expected_type, const ObjectState*& out,
                                    std::string context) {
    const auto* state = transaction.find(ObjectId{id});
    if (state == nullptr) {
        return fail(DetailCode::dangling_reference,
                    "the object this command refers to does not exist", std::move(context));
    }
    if (state->type != expected_type) {
        return fail(DetailCode::type_mismatch,
                    std::string{"the referenced object has type "} + state->type + " instead of " +
                        std::string{expected_type},
                    std::move(context));
    }
    out = state;
    return std::nullopt;
}

std::optional<Error> require_property(const ObjectState& state, std::string_view key,
                                     std::uint64_t& out, std::string context) {
    const auto value = read_id_property(state, key);
    if (!value.has_value()) {
        return fail(DetailCode::dangling_reference,
                    std::string{"the referenced object does not carry a readable "} +
                        std::string{key} + " property",
                    std::move(context));
    }
    out = *value;
    return std::nullopt;
}

std::optional<Error> write_property(CommandTransaction& transaction, std::uint64_t id,
                                 std::string_view key, std::string value, std::string context) {
    auto error = transaction.set_property(ObjectId{id}, std::string{key},
                                         Value{std::move(value)});
    return from_kernel(error, DetailCode::dangling_reference, std::move(context));
}

std::optional<Error> annotate(const Diagnostic& diagnostic, std::string context) {
    Diagnostic annotated = diagnostic;
    annotated.location.description = std::move(context);
    return annotated.to_error();
}

std::optional<Error> validate_label_text(std::string_view label, std::string context) {
    if (label.empty()) {
        return std::nullopt;
    }
    auto validated = validate_text(label, TextNulPolicy::reject);
    if (!validated.ok()) {
        return annotate(validated.diagnostic(), std::move(context));
    }
    return std::nullopt;
}

std::optional<Error> validate_unit_text(std::string_view unit, std::string context) {
    if (unit.empty()) {
        return std::nullopt;
    }
    auto parsed = UnitRegistry::builtin().parse(unit);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    return std::nullopt;
}

std::optional<Error> check_choice(std::string_view text,
                                 const std::initializer_list<std::string_view> allowed,
                                 std::string context) {
    if (std::find(allowed.begin(), allowed.end(), text) != allowed.end()) {
        return std::nullopt;
    }
    return fail(DetailCode::type_mismatch, "argument is not one of the declared choices",
                std::move(context));
}

// The metadata a caller may declare about a binding. Each entry is present only when the
// caller named it, so a rename never disturbs a unit and a unit change never disturbs a role.
struct Metadata {
    std::optional<std::string> label;
    std::optional<std::string> role;
    std::optional<std::string> unit;
    std::optional<std::string> display_format;
    std::optional<std::uint64_t> order_index;
};

std::optional<Error> collect_metadata(const Args& args, std::uint64_t column, Metadata& out) {
    if (auto error = args.optional_text(argument::kLabel, out.label)) {
        return error;
    }
    if (out.label.has_value()) {
        if (auto error = validate_label_text(*out.label, "column " + std::to_string(column))) {
            return error;
        }
    }
    if (auto error = args.optional_text(argument::kRole, out.role)) {
        return error;
    }
    if (out.role.has_value()) {
        if (auto error = check_choice(*out.role, {"unspecified", "x", "y", "z", "error", "label"},
                                     "role of column " + std::to_string(column))) {
            return error;
        }
    }
    if (auto error = args.optional_text(argument::kUnit, out.unit)) {
        return error;
    }
    if (out.unit.has_value()) {
        if (auto error = validate_unit_text(*out.unit, "unit of column " + std::to_string(column))) {
            return error;
        }
    }
    if (auto error = args.optional_text(argument::kDisplayFormat, out.display_format)) {
        return error;
    }
    std::uint64_t order_index = 0;
    if (auto error = args.id(argument::kOrderIndex, order_index, false)) {
        return error;
    }
    if (args.contains(argument::kOrderIndex)) {
        out.order_index = order_index;
    }
    return std::nullopt;
}

void write_metadata(ParameterMap& properties, const Metadata& metadata) {
    if (metadata.label.has_value()) {
        set_text(properties, property::kLabel, *metadata.label);
    }
    if (metadata.role.has_value()) {
        set_text(properties, property::kRole, *metadata.role);
    }
    if (metadata.unit.has_value()) {
        set_text(properties, property::kUnit, *metadata.unit);
    }
    if (metadata.display_format.has_value()) {
        set_text(properties, property::kDisplayFormat, *metadata.display_format);
    }
    if (metadata.order_index.has_value()) {
        set_id(properties, property::kOrderIndex, *metadata.order_index);
    }
}

std::optional<Error> apply_metadata(CommandTransaction& transaction, std::uint64_t column,
                                   const Metadata& metadata) {
    const std::string context = "column " + std::to_string(column);
    if (metadata.label.has_value()) {
        if (auto error = write_property(transaction, column, property::kLabel, *metadata.label,
                                       context)) {
            return error;
        }
    }
    if (metadata.role.has_value()) {
        if (auto error = write_property(transaction, column, property::kRole, *metadata.role,
                                       context)) {
            return error;
        }
    }
    if (metadata.unit.has_value()) {
        if (auto error = write_property(transaction, column, property::kUnit, *metadata.unit,
                                       context)) {
            return error;
        }
    }
    if (metadata.display_format.has_value()) {
        if (auto error = write_property(transaction, column, property::kDisplayFormat,
                                       *metadata.display_format, context)) {
            return error;
        }
    }
    if (metadata.order_index.has_value()) {
        if (auto error = write_property(transaction, column, property::kOrderIndex,
                                       std::to_string(*metadata.order_index), context)) {
            return error;
        }
    }
    return std::nullopt;
}

// Records one immutable version as a document object and stages its payload for the
// transaction that publishes it. Nothing here is readable until the executor commits.
std::optional<Error> publish_version(CommandTransaction& transaction, DataStore& store,
                                    std::uint64_t transaction_id,
                                    const std::shared_ptr<const DatasetVersion>& version,
                                    std::optional<std::uint64_t> previous, std::string context) {
    if (version == nullptr) {
        return fail(DetailCode::unknown_draft, "the candidate version is not available",
                    std::move(context));
    }
    if (auto error = check_unique(transaction, version->id().raw(),
                                 context + " version " + to_string(version->id()))) {
        return error;
    }
    ParameterMap properties;
    set_id(properties, property::kVersion, version->id().raw());
    set_id(properties, property::kDataset, version->dataset().raw());
    set_id(properties, property::kRowDomain, version->row_domain().raw());
    set_id(properties, property::kRowCount, version->row_count());
    set_text(properties, property::kContentHash, version->content_hash().hex());
    set_text(properties, property::kLabel, version->provenance().operation_id);
    if (previous.has_value()) {
        set_id(properties, property::kPreviousVersion, *previous);
    }
    if (auto error = create_object(transaction, version->id().raw(), object_type::kDatasetVersion,
                                  std::move(properties), "version " + to_string(version->id()))) {
        return error;
    }
    store.stage(transaction_id, version);
    return std::nullopt;
}

std::optional<Error> register_definition(application::CommandRegistry& registry,
                                        application::CommandDefinition definition) {
    definition.supported_schema_versions = {kSchemaVersion};
    const std::string command_id_copy = definition.command_id;
    auto error = registry.register_command(std::move(definition));
    if (error.has_value()) {
        return fail(DetailCode::duplicate_id, error->message, command_id_copy);
    }
    return std::nullopt;
}


std::optional<Error> version_cells(const DataStore& store, std::uint64_t version_id, FieldId field,
                                   const RowDomain& domain, std::vector<Cell>& out) {
    auto version = store.version(DatasetVersionId{ObjectId{version_id}});
    if (!version.ok()) {
        return version.diagnostic().to_error();
    }
    if (version.value()->row_domain() != domain.id()) {
        return fail(DetailCode::unmapped_row,
                    "this version is ordered by a different row-identity domain",
                    "version " + to_string(version.value()->id()));
    }
    if (version.value()->schema()->find(field) == nullptr) {
        return fail(DetailCode::unknown_field, "this field is not declared by the version",
                    "version " + to_string(version.value()->id()));
    }
    out.clear();
    out.reserve(domain.row_count());
    for (std::uint64_t index = 0; index < domain.row_count(); ++index) {
        auto cell = version.value()->cell(field, index);
        if (!cell.ok()) {
            return cell.diagnostic().to_error();
        }
        out.push_back(cell.take());
    }
    return std::nullopt;
}

std::optional<Error> single_field(const DatasetVersion& version, FieldId& out, std::string context) {
    if (version.schema()->fields.size() != 1) {
        return fail(DetailCode::type_mismatch,
                    "this operation reads a single-field dataset version, which is how a worksheet "
                    "column is stored",
                    std::move(context));
    }
    out = version.schema()->fields.front().id;
    return std::nullopt;
}

// An edit keeps the field it edits: identity, logical type, unit, dictionary, unseen-category
// policy and declared zone all travel unchanged into the successor, so an edit never drops a
// unit, reinterprets a value or re-keys a dictionary.
std::optional<Error> check_same_schema(const DatasetVersion& previous,
                                       const DatasetVersion& successor, std::string context) {
    if (successor.schema()->canonical_text() == previous.schema()->canonical_text()) {
        return std::nullopt;
    }
    return fail(DetailCode::type_mismatch,
                "a successor version must declare the same field as the version it replaces",
                std::move(context) + ": was [" + previous.schema()->canonical_text() +
                    "] now [" + successor.schema()->canonical_text() + "]");
}

// The kernel hands out the object list by value, so this returns copies rather than
// references into a temporary.
std::vector<ObjectState> bindings_of(const CommandTransaction& transaction,
                                     std::uint64_t worksheet) {
    std::vector<ObjectState> found;
    for (const auto& state : transaction.objects()) {
        if (state.type != object_type::kColumnBinding) {
            continue;
        }
        const auto parent = read_id_property(state, property::kParent);
        if (parent.has_value() && *parent == worksheet) {
            found.push_back(state);
        }
    }
    return found;
}

}  // namespace

DraftId DraftRegistry::add(std::shared_ptr<const DatasetVersion> version) {
    Entry entry;
    entry.version = std::move(version);
    return insert(std::move(entry));
}

DraftId DraftRegistry::add(std::shared_ptr<const RowDomain> domain) {
    Entry entry;
    entry.domain = std::move(domain);
    return insert(std::move(entry));
}

DraftId DraftRegistry::add(std::shared_ptr<const RowCorrespondence> correspondence) {
    Entry entry;
    entry.correspondence = std::move(correspondence);
    return insert(std::move(entry));
}

DraftId DraftRegistry::insert(Entry entry) {
    entries_.push_back(std::move(entry));
    return static_cast<DraftId>(entries_.size());
}

std::shared_ptr<const DatasetVersion> DraftRegistry::version(DraftId id) const {
    if (id == kNoDraft || id > entries_.size()) {
        return nullptr;
    }
    return entries_[static_cast<std::size_t>(id - 1)].version;
}

std::shared_ptr<const RowDomain> DraftRegistry::domain(DraftId id) const {
    if (id == kNoDraft || id > entries_.size()) {
        return nullptr;
    }
    return entries_[static_cast<std::size_t>(id - 1)].domain;
}

std::shared_ptr<const RowCorrespondence> DraftRegistry::correspondence(DraftId id) const {
    if (id == kNoDraft || id > entries_.size()) {
        return nullptr;
    }
    return entries_[static_cast<std::size_t>(id - 1)].correspondence;
}

void DraftRegistry::drop(DraftId id) {
    if (id == kNoDraft || id > entries_.size()) {
        return;
    }
    entries_[static_cast<std::size_t>(id - 1)] = Entry{};
}

std::size_t DraftRegistry::size() const noexcept {
    std::size_t count = 0;
    for (const auto& entry : entries_) {
        if (entry.version != nullptr || entry.domain != nullptr || entry.correspondence != nullptr) {
            ++count;
        }
    }
    return count;
}

std::optional<Diagnostic> register_data_commands(application::CommandRegistry& registry,
                                                DataStore& store, const DraftRegistry& drafts) {
    const auto report = [](std::optional<Error> error) -> std::optional<Diagnostic> {
        if (!error.has_value()) {
            return std::nullopt;
        }
        Diagnostic diagnostic;
        diagnostic.detail = DetailCode::type_mismatch;
        diagnostic.outer = error->code;
        diagnostic.severity = Severity::error;
        diagnostic.location.description = error->message;
        return diagnostic;
    };

    // data.project_create
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kProjectCreate;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t project = 0;
            if (auto error = args.id(argument::kProject, project)) {
                return error;
            }
            std::string label;
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            return validate_label_text(label, "project " + std::to_string(project));
        };
        definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                               const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t project = 0;
            std::string label;
            if (auto error = args.id(argument::kProject, project)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            if (auto error = check_unique(transaction, project, "project " + std::to_string(project))) {
                return error;
            }
            ParameterMap properties;
            set_text(properties, property::kLabel, std::move(label));
            return create_object(transaction, project, object_type::kProject, std::move(properties),
                                 "project " + std::to_string(project));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.workbook_create
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorkbookCreate;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = args.all_ids({argument::kWorkbook, argument::kProject})) {
                return error;
            }
            std::string label;
            return args.text(argument::kLabel, label);
        };
        definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                               const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t workbook = 0;
            std::uint64_t project = 0;
            std::uint64_t order_index = 0;
            std::string label;
            if (auto error = args.id(argument::kWorkbook, workbook)) {
                return error;
            }
            if (auto error = args.id(argument::kProject, project)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            if (auto error = args.id(argument::kOrderIndex, order_index, false)) {
                return error;
            }
            const ObjectState* parent = nullptr;
            if (auto error = require_object(transaction, project, object_type::kProject, parent,
                                           "project " + std::to_string(project))) {
                return error;
            }
            if (auto error = check_unique(transaction, workbook, "workbook " + std::to_string(workbook))) {
                return error;
            }
            ParameterMap properties;
            set_text(properties, property::kLabel, std::move(label));
            set_id(properties, property::kParent, project);
            set_id(properties, property::kOrderIndex, order_index);
            return create_object(transaction, workbook, object_type::kWorkbook,
                                std::move(properties), "workbook " + std::to_string(workbook));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.worksheet_create starts a worksheet with an empty row-identity domain.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetCreate;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = args.all_ids({argument::kWorksheet, argument::kWorkbook,
                                           argument::kDomain})) {
                return error;
            }
            std::string label;
            return args.text(argument::kLabel, label);
        };
        definition.execute = [&store](CommandTransaction& transaction, const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t workbook = 0;
            std::uint64_t domain = 0;
            std::uint64_t order_index = 0;
            std::string label;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kWorkbook, workbook)) {
                return error;
            }
            if (auto error = args.id(argument::kDomain, domain)) {
                return error;
            }
            if (auto error = args.id(argument::kOrderIndex, order_index, false)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            const ObjectState* parent = nullptr;
            if (auto error = require_object(transaction, workbook, object_type::kWorkbook, parent,
                                           "workbook " + std::to_string(workbook))) {
                return error;
            }
            if (auto error = check_unique(transaction, worksheet, "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            if (auto error = check_unique(transaction, domain, "row domain " + std::to_string(domain))) {
                return error;
            }
            auto created = RowDomain::create(RowDomainId{ObjectId{domain}}, std::vector<RowId>{});
            if (!created.ok()) {
                return created.diagnostic().to_error();
            }
            std::shared_ptr<const RowDomain> staged =
                std::make_shared<const RowDomain>(created.take());
            store.stage(context.transaction_id, staged);

            ParameterMap domain_properties;
            set_id(domain_properties, property::kRowCount, staged->row_count());
            set_text(domain_properties, property::kContentHash, staged->content_hash().hex());
            if (auto error = create_object(transaction, domain, object_type::kRowDomain,
                                          std::move(domain_properties),
                                          "row domain " + std::to_string(domain))) {
                return error;
            }
            ParameterMap properties;
            set_text(properties, property::kLabel, std::move(label));
            set_id(properties, property::kParent, workbook);
            set_id(properties, property::kRowDomain, domain);
            set_id(properties, property::kOrderIndex, order_index);
            return create_object(transaction, worksheet, object_type::kWorksheet,
                                std::move(properties), "worksheet " + std::to_string(worksheet));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.column_create: a new same-domain column, or a mirror aliasing a committed version.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kColumnCreate;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t column = 0;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.all_ids({argument::kWorksheet, argument::kDataset})) {
                return error;
            }
            std::string label;
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            if (auto error = validate_label_text(label, "column " + std::to_string(column))) {
                return error;
            }
            Metadata metadata;
            if (auto error = collect_metadata(args, column, metadata)) {
                return error;
            }
            std::optional<std::string> policy;
            if (auto error = args.optional_text(argument::kWritePolicy, policy)) {
                return error;
            }
            if (!policy.has_value()) {
                std::uint64_t draft = 0;
                return args.id(argument::kVersionDraft, draft);
            }
            if (auto error = check_choice(*policy, {"mutable", "mirror"},
                                         "write policy of column " + std::to_string(column))) {
                return error;
            }
            if (*policy == "mirror") {
                std::uint64_t source = 0;
                return args.id(argument::kSourceVersion, source);
            }
            std::uint64_t draft = 0;
            return args.id(argument::kVersionDraft, draft);
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                             const CommandRequest& request,
                                             const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t column = 0;
            std::uint64_t worksheet = 0;
            std::uint64_t dataset = 0;
            std::uint64_t draft_id = 0;
            std::uint64_t source_version_id = 0;
            std::string label;
            std::optional<std::string> policy;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kDataset, dataset)) {
                return error;
            }
            if (auto error = args.id(argument::kVersionDraft, draft_id, false)) {
                return error;
            }
            if (auto error = args.id(argument::kSourceVersion, source_version_id, false)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kWritePolicy, policy)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            Metadata metadata;
            if (auto error = collect_metadata(args, column, metadata)) {
                return error;
            }
            metadata.label = std::move(label);

            const ObjectState* parent = nullptr;
            if (auto error = require_object(transaction, worksheet, object_type::kWorksheet, parent,
                                           "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            std::uint64_t worksheet_domain = 0;
            if (auto error = require_property(*parent, property::kRowDomain, worksheet_domain,
                                             "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            if (auto error = check_unique(transaction, column, "column " + std::to_string(column))) {
                return error;
            }

            const bool mirror = policy.has_value() && *policy == "mirror";
            std::shared_ptr<const DatasetVersion> version;
            if (mirror) {
                auto stored = store.version(DatasetVersionId{ObjectId{source_version_id}});
                if (!stored.ok()) {
                    return stored.diagnostic().to_error();
                }
                version = stored.take();
            } else {
                version = drafts.version(draft_id);
                if (version == nullptr) {
                    return fail(DetailCode::unknown_draft,
                                "a new column names the candidate version it publishes",
                                "version_draft " + std::to_string(draft_id));
                }
                if (version->dataset() != DatasetId{ObjectId{dataset}}) {
                    return fail(DetailCode::type_mismatch,
                                "the candidate version belongs to another dataset",
                                "column " + std::to_string(column));
                }
            }
            if (version->row_domain() != RowDomainId{ObjectId{worksheet_domain}}) {
                return fail(DetailCode::positional_mapping_required,
                            "this column's rows come from another row-identity domain, so it must "
                            "be bound through an explicit correspondence",
                            "column " + std::to_string(column));
            }
            auto stored_domain = store.row_domain(RowDomainId{ObjectId{worksheet_domain}});
            if (!stored_domain.ok()) {
                return stored_domain.diagnostic().to_error();
            }
            if (version->row_count() != stored_domain.value()->row_count()) {
                return fail(DetailCode::unmapped_row,
                            "the column's " + std::to_string(version->row_count()) +
                                " rows do not match the worksheet domain's " +
                                std::to_string(stored_domain.value()->row_count()) +
                                " rows, so nothing is zipped or padded",
                            "column " + std::to_string(column));
            }

            ParameterMap properties;
            set_id(properties, property::kParent, worksheet);
            set_id(properties, property::kDataset, dataset);
            set_id(properties, property::kVersion, version->id().raw());
            set_text(properties, property::kWritePolicy, mirror ? "mirror" : "mutable");
            set_text(properties, property::kLink, "same_domain");
            write_metadata(properties, metadata);
            if (auto error = create_object(transaction, column, object_type::kColumnBinding,
                                          std::move(properties), "column " + std::to_string(column))) {
                return error;
            }
            if (mirror) {
                // A mirror is a binding to the same identity: no new version is published and
                // no storage is duplicated.
                return std::nullopt;
            }
            return publish_version(transaction, store, context.transaction_id, version,
                                   std::nullopt, "column " + std::to_string(column));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.column_edit publishes an immutable successor version for one column.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kColumnEdit;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            return args.all_ids({argument::kColumn, argument::kVersionDraft});
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                             const CommandRequest& request,
                                             const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t column = 0;
            std::uint64_t draft_id = 0;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.id(argument::kVersionDraft, draft_id)) {
                return error;
            }
            const ObjectState* binding = nullptr;
            if (auto error = require_object(transaction, column, object_type::kColumnBinding, binding,
                                           "column " + std::to_string(column))) {
                return error;
            }
            std::uint64_t current_version_id = 0;
            if (auto error = require_property(*binding, property::kVersion, current_version_id,
                                             "column " + std::to_string(column))) {
                return error;
            }
            const auto binding_policy = read_text_property(*binding, property::kWritePolicy);
            if (binding_policy.has_value() && *binding_policy == "mirror") {
                return fail(DetailCode::mirror_read_only,
                            "a mirror binding shares one version identity and is read-only by "
                            "default, so a value edit needs the shared-edit or detach-to-copy "
                            "command",
                            "column " + std::to_string(column));
            }
            auto current = store.version(DatasetVersionId{ObjectId{current_version_id}});
            if (!current.ok()) {
                return current.diagnostic().to_error();
            }
            auto version = drafts.version(draft_id);
            if (version == nullptr) {
                return fail(DetailCode::unknown_draft, "the candidate version is not available",
                            "version_draft " + std::to_string(draft_id));
            }
            if (version->dataset() != current.value()->dataset()) {
                return fail(DetailCode::type_mismatch, "an edit stays inside its own dataset",
                            "column " + std::to_string(column));
            }
            if (auto error = check_same_schema(*current.value(), *version,
                                               "column " + std::to_string(column))) {
                return error;
            }
            if (version->row_domain() != current.value()->row_domain() ||
                version->row_count() != current.value()->row_count()) {
                return fail(DetailCode::unmapped_row,
                            "an edit keeps the row-identity domain and the row count, so row "
                            "identity survives the operation",
                            "column " + std::to_string(column));
            }
            const auto& parents = version->provenance().parents;
            if (std::find(parents.begin(), parents.end(), current.value()->id()) == parents.end()) {
                return fail(DetailCode::dangling_reference,
                            "an edit must record the version it replaces as a provenance parent",
                            "column " + std::to_string(column));
            }
            if (auto error = publish_version(transaction, store, context.transaction_id, version,
                                            current_version_id, "column " + std::to_string(column))) {
                return error;
            }
            return write_property(transaction, column, property::kVersion,
                               std::to_string(version->id().raw()),
                               "column " + std::to_string(column));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.column_rename, data.column_move and data.column_metadata change metadata only.
    for (const auto* name : {command_id::kColumnRename, command_id::kColumnMove,
                            command_id::kColumnMetadata}) {
        application::CommandDefinition definition;
        definition.command_id = name;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t column = 0;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            Metadata metadata;
            if (auto error = collect_metadata(args, column, metadata)) {
                return error;
            }
            const bool any = metadata.label.has_value() || metadata.role.has_value() ||
                             metadata.unit.has_value() || metadata.display_format.has_value() ||
                             metadata.order_index.has_value();
            if (!any) {
                return fail(DetailCode::type_mismatch,
                            "a metadata command changes at least one declared property");
            }
            return std::nullopt;
        };
        definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                               const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t column = 0;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            const ObjectState* binding = nullptr;
            if (auto error = require_object(transaction, column, object_type::kColumnBinding, binding,
                                           "column " + std::to_string(column))) {
                return error;
            }
            Metadata metadata;
            if (auto error = collect_metadata(args, column, metadata)) {
                return error;
            }
            return apply_metadata(transaction, column, metadata);
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.correspondence_create declares how a foreign-domain column pairs its rows.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kCorrespondenceCreate;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t correspondence = 0;
            if (auto error = args.id(argument::kCorrespondence, correspondence)) {
                return error;
            }
            if (auto error = args.all_ids({argument::kWorksheet, argument::kSourceVersion})) {
                return error;
            }
            std::string method;
            if (auto error = args.text(argument::kMethod, method)) {
                return error;
            }
            if (auto error = check_choice(method, {"keyed_join", "approved_positional"},
                                         "mapping method");
                error.has_value()) {
                return error;
            }
            if (method == "keyed_join") {
                return args.all_ids({argument::kKeyColumn, argument::kSourceField});
            }
            std::string approved;
            if (auto error = args.text(argument::kApproved, approved)) {
                return error;
            }
            return check_choice(approved, {"true", "false"}, "positional mapping approval");
        };
        definition.execute = [&store](CommandTransaction& transaction, const CommandRequest& request,
                                      const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t correspondence_id = 0;
            std::uint64_t worksheet = 0;
            std::uint64_t source_version_id = 0;
            std::uint64_t key_column_id = 0;
            std::uint64_t source_field_id = 0;
            std::string method;
            std::string approved;
            if (auto error = args.id(argument::kCorrespondence, correspondence_id)) {
                return error;
            }
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kSourceVersion, source_version_id)) {
                return error;
            }
            if (auto error = args.text(argument::kMethod, method)) {
                return error;
            }
            const bool keyed = method == "keyed_join";
            if (keyed) {
                if (auto error = args.id(argument::kKeyColumn, key_column_id)) {
                    return error;
                }
                if (auto error = args.id(argument::kSourceField, source_field_id)) {
                    return error;
                }
            } else {
                if (auto error = args.text(argument::kApproved, approved)) {
                    return error;
                }
                if (approved != "true") {
                    return fail(DetailCode::positional_mapping_required,
                                "a positional mapping between unrelated domains needs the "
                                "caller's explicit approval",
                                "correspondence " + std::to_string(correspondence_id));
                }
            }
            if (auto error =
                    check_unique(transaction, correspondence_id,
                                 "correspondence " + std::to_string(correspondence_id))) {
                return error;
            }
            const ObjectState* worksheet_state = nullptr;
            if (auto error = require_object(transaction, worksheet, object_type::kWorksheet,
                                           worksheet_state, "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            std::uint64_t worksheet_domain = 0;
            if (auto error = require_property(*worksheet_state, property::kRowDomain,
                                             worksheet_domain,
                                             "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            auto target = store.row_domain(RowDomainId{ObjectId{worksheet_domain}});
            if (!target.ok()) {
                return target.diagnostic().to_error();
            }
            auto source_version = store.version(DatasetVersionId{ObjectId{source_version_id}});
            if (!source_version.ok()) {
                return source_version.diagnostic().to_error();
            }
            auto source = store.row_domain(source_version.value()->row_domain());
            if (!source.ok()) {
                return source.diagnostic().to_error();
            }
            if (source.value()->id() == target.value()->id()) {
                return fail(DetailCode::type_mismatch,
                            "this column shares the worksheet's row domain, so it is bound as a "
                            "same-domain column instead",
                            "correspondence " + std::to_string(correspondence_id));
            }

            std::shared_ptr<const RowCorrespondence> mapping;
            if (keyed) {
                const ObjectState* key_state = nullptr;
                if (auto error = require_object(transaction, key_column_id,
                                               object_type::kColumnBinding, key_state,
                                               "key column " + std::to_string(key_column_id))) {
                    return error;
                }
                std::uint64_t key_version_id = 0;
                if (auto error = require_property(*key_state, property::kVersion, key_version_id,
                                                 "key column " + std::to_string(key_column_id))) {
                    return error;
                }
                auto key_version = store.version(DatasetVersionId{ObjectId{key_version_id}});
                if (!key_version.ok()) {
                    return key_version.diagnostic().to_error();
                }
                FieldId key_field{};
                if (auto error = single_field(*key_version.value(), key_field,
                                              "key column " + std::to_string(key_column_id))) {
                    return error;
                }
                std::vector<Cell> target_keys;
                if (auto error = version_cells(store, key_version_id, key_field, *target.value(),
                                              target_keys)) {
                    return error;
                }
                const FieldId source_field{ObjectId{source_field_id}};
                std::vector<Cell> source_keys;
                if (auto error = version_cells(store, source_version_id, source_field,
                                              *source.value(), source_keys)) {
                    return error;
                }
                auto created = RowCorrespondence::keyed_join(
                    CorrespondenceId{ObjectId{correspondence_id}}, *target.value(), target_keys,
                    *source.value(), source_keys);
                if (!created.ok()) {
                    return created.diagnostic().to_error();
                }
                mapping = std::make_shared<const RowCorrespondence>(created.take());
            } else {
                auto created = RowCorrespondence::approved_positional(
                    CorrespondenceId{ObjectId{correspondence_id}}, *target.value(), *source.value(),
                    true);
                if (!created.ok()) {
                    return created.diagnostic().to_error();
                }
                mapping = std::make_shared<const RowCorrespondence>(created.take());
            }
            store.stage(context.transaction_id, mapping);

            ParameterMap properties;
            set_id(properties, property::kTargetDomain, target.value()->id().raw());
            set_id(properties, property::kSourceDomain, source.value()->id().raw());
            set_id(properties, property::kVersion, source_version_id);
            set_id(properties, property::kRowCount, mapping->pairs().size());
            set_text(properties, property::kContentHash, mapping->content_hash().hex());
            set_text(properties, property::kLabel, mapping->method() == MappingMethod::keyed_join
                                                       ? "keyed_join"
                                                       : "approved_positional");
            return create_object(transaction, correspondence_id, object_type::kRowCorrespondence,
                                std::move(properties),
                                "correspondence " + std::to_string(correspondence_id));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.column_bind_foreign binds a column whose rows come from another domain.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kColumnBindForeign;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t column = 0;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.all_ids({argument::kWorksheet, argument::kDataset,
                                            argument::kVersionDraft, argument::kCorrespondence})) {
                return error;
            }
            std::string label;
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            return validate_label_text(label, "column " + std::to_string(column));
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                             const CommandRequest& request,
                                             const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t column = 0;
            std::uint64_t worksheet = 0;
            std::uint64_t dataset = 0;
            std::uint64_t draft_id = 0;
            std::uint64_t correspondence_id = 0;
            std::string label;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kDataset, dataset)) {
                return error;
            }
            if (auto error = args.id(argument::kVersionDraft, draft_id)) {
                return error;
            }
            if (auto error = args.id(argument::kCorrespondence, correspondence_id)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label)) {
                return error;
            }
            Metadata metadata;
            if (auto error = collect_metadata(args, column, metadata)) {
                return error;
            }
            metadata.label = std::move(label);

            const ObjectState* parent = nullptr;
            if (auto error = require_object(transaction, worksheet, object_type::kWorksheet, parent,
                                           "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            std::uint64_t worksheet_domain = 0;
            if (auto error = require_property(*parent, property::kRowDomain, worksheet_domain,
                                             "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            if (auto error = check_unique(transaction, column, "column " + std::to_string(column))) {
                return error;
            }
            auto version = drafts.version(draft_id);
            if (version == nullptr) {
                return fail(DetailCode::unknown_draft,
                            "a foreign-domain column names the candidate version it publishes",
                            "version_draft " + std::to_string(draft_id));
            }
            if (version->dataset() != DatasetId{ObjectId{dataset}}) {
                return fail(DetailCode::type_mismatch, "the candidate version belongs to another dataset",
                            "column " + std::to_string(column));
            }
            if (version->row_domain() == RowDomainId{ObjectId{worksheet_domain}}) {
                return fail(DetailCode::type_mismatch,
                            "this column shares the worksheet's row domain, so create it as a "
                            "same-domain column instead",
                            "column " + std::to_string(column));
            }
            auto mapping = store.correspondence(CorrespondenceId{ObjectId{correspondence_id}});
            if (!mapping.ok()) {
                return mapping.diagnostic().to_error();
            }
            if (mapping.value()->target_domain() != RowDomainId{ObjectId{worksheet_domain}} ||
                mapping.value()->source_domain() != version->row_domain()) {
                return fail(DetailCode::dangling_reference,
                            "the correspondence does not pair this worksheet's domain with the "
                            "column's domain",
                            "column " + std::to_string(column));
            }
            const std::size_t unmatched_targets = mapping.value()->unmatched_targets().size();
            if (unmatched_targets == mapping.value()->pairs().size()) {
                return fail(DetailCode::unmapped_row,
                            "every worksheet row is unmatched, so this column pairs nothing",
                            "column " + std::to_string(column));
            }

            ParameterMap properties;
            set_id(properties, property::kParent, worksheet);
            set_id(properties, property::kDataset, dataset);
            set_id(properties, property::kVersion, version->id().raw());
            set_id(properties, property::kCorrespondence, correspondence_id);
            set_text(properties, property::kWritePolicy, "mutable");
            set_text(properties, property::kLink, "correspondence");
            write_metadata(properties, metadata);
            if (auto error = create_object(transaction, column, object_type::kColumnBinding,
                                          std::move(properties), "column " + std::to_string(column))) {
                return error;
            }
            return publish_version(transaction, store, context.transaction_id, version,
                                   std::nullopt, "column " + std::to_string(column));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.worksheet_change_rows moves a worksheet to a revised row domain in one revision.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetChangeRows;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t worksheet = 0;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.all_ids({argument::kDomainDraft})) {
                return error;
            }
            std::string operation;
            if (auto error = args.text(argument::kOperation, operation)) {
                return error;
            }
            if (auto error = check_choice(operation, {"append", "delete"}, "row operation")) {
                return error;
            }
            std::vector<std::uint64_t> columns;
            if (auto error = args.ids(argument::kColumns, columns)) {
                return error;
            }
            std::vector<std::uint64_t> drafts;
            return args.ids(argument::kVersionDrafts, drafts);
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                             const CommandRequest& request,
                                             const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t domain_draft_id = 0;
            std::string operation;
            std::vector<std::uint64_t> columns;
            std::vector<std::uint64_t> version_drafts;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kDomainDraft, domain_draft_id)) {
                return error;
            }
            if (auto error = args.text(argument::kOperation, operation)) {
                return error;
            }
            if (auto error = args.ids(argument::kColumns, columns)) {
                return error;
            }
            if (auto error = args.ids(argument::kVersionDrafts, version_drafts)) {
                return error;
            }
            if (columns.size() != version_drafts.size()) {
                return fail(DetailCode::unmapped_row,
                            "the column list and the candidate version list must have equal length",
                            "worksheet " + std::to_string(worksheet));
            }
            const ObjectState* worksheet_state = nullptr;
            if (auto error = require_object(transaction, worksheet, object_type::kWorksheet,
                                           worksheet_state, "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            std::uint64_t current_domain_id = 0;
            if (auto error = require_property(*worksheet_state, property::kRowDomain, current_domain_id,
                                             "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            auto current = store.row_domain(RowDomainId{ObjectId{current_domain_id}});
            if (!current.ok()) {
                return current.diagnostic().to_error();
            }
            auto revised = drafts.domain(domain_draft_id);
            if (revised == nullptr) {
                return fail(DetailCode::unknown_draft, "the revised row domain is not available",
                            "domain_draft " + std::to_string(domain_draft_id));
            }
            if (auto error = check_unique(transaction, revised->id().raw(),
                                         "row domain " + to_string(revised->id()))) {
                return error;
            }

            const auto& old_rows = current.value()->rows();
            const auto& new_rows = revised->rows();
            const bool appending = operation == "append";
            if (appending) {
                if (new_rows.size() <= old_rows.size() ||
                    !std::equal(old_rows.begin(), old_rows.end(), new_rows.begin())) {
                    return fail(DetailCode::unmapped_row,
                                "an append keeps every existing row identity in order and adds new "
                                "identities after them",
                                "worksheet " + std::to_string(worksheet));
                }
            } else if (new_rows.size() >= old_rows.size()) {
                return fail(DetailCode::unmapped_row,
                            "a delete removes at least one row identity and keeps the survivors in "
                            "order",
                            "worksheet " + std::to_string(worksheet));
            } else {
                std::size_t matched = 0;
                for (const auto& row : old_rows) {
                    if (matched < new_rows.size() && new_rows[matched] == row) {
                        ++matched;
                    }
                }
                if (matched != new_rows.size()) {
                    return fail(DetailCode::unmapped_row,
                                "the revised domain must be the current domain with the deleted row "
                                "identities removed",
                                "worksheet " + std::to_string(worksheet));
                }
            }

            // Every same-domain binding moves in the same revision, so the caller must list
            // exactly those columns. A correspondence-linked binding keeps its pairs and is
            // re-paired by the refresh command instead.
            std::vector<std::uint64_t> required;
            for (const auto& state : bindings_of(transaction, worksheet)) {
                const auto link = read_text_property(state, property::kLink);
                if (!link.has_value() || link == "same_domain") {
                    required.push_back(state.id.value);
                }
            }
            std::vector<std::uint64_t> listed = columns;
            std::sort(listed.begin(), listed.end());
            std::sort(required.begin(), required.end());
            if (listed != required) {
                return fail(DetailCode::unmapped_row,
                            "this command must declare a candidate version for every same-domain "
                            "column of the worksheet, and no others",
                            "worksheet " + std::to_string(worksheet));
            }

            std::vector<std::shared_ptr<const DatasetVersion>> published;
            for (std::size_t index = 0; index < columns.size(); ++index) {
                const std::uint64_t column = columns[index];
                auto version = drafts.version(version_drafts[index]);
                if (version == nullptr) {
                    return fail(DetailCode::unknown_draft, "the candidate version is not available",
                                "version_draft " + std::to_string(version_drafts[index]));
                }
                const ObjectState* binding = nullptr;
                if (auto error = require_object(transaction, column, object_type::kColumnBinding,
                                               binding, "column " + std::to_string(column))) {
                    return error;
                }
                std::uint64_t current_version_id = 0;
                std::uint64_t dataset_id = 0;
                if (auto error = require_property(*binding, property::kVersion, current_version_id,
                                                 "column " + std::to_string(column))) {
                    return error;
                }
                if (auto error = require_property(*binding, property::kDataset, dataset_id,
                                                 "column " + std::to_string(column))) {
                    return error;
                }
                auto previous = store.version(DatasetVersionId{ObjectId{current_version_id}});
                if (!previous.ok()) {
                    return previous.diagnostic().to_error();
                }
                if (version->dataset() != previous.value()->dataset() ||
                    dataset_id != previous.value()->dataset().raw()) {
                    return fail(DetailCode::type_mismatch, "a row change stays inside its dataset",
                                "column " + std::to_string(column));
                }
                if (auto error = check_same_schema(*previous.value(), *version,
                                                   "column " + std::to_string(column))) {
                    return error;
                }
                if (version->row_domain() != revised->id() ||
                    version->row_count() != revised->row_count()) {
                    return fail(DetailCode::unmapped_row,
                                "each candidate version must be ordered by the revised row domain",
                                "column " + std::to_string(column));
                }
                const auto& parents = version->provenance().parents;
                if (std::find(parents.begin(), parents.end(), previous.value()->id()) == parents.end()) {
                    return fail(DetailCode::dangling_reference,
                                "a row change records the version it replaces as a provenance parent",
                                "column " + std::to_string(column));
                }
                published.push_back(std::move(version));
            }

            store.stage(context.transaction_id, revised);
            ParameterMap domain_properties;
            set_id(domain_properties, property::kRowCount, revised->row_count());
            set_text(domain_properties, property::kContentHash, revised->content_hash().hex());
            if (auto error = create_object(transaction, revised->id().raw(), object_type::kRowDomain,
                                          std::move(domain_properties),
                                          "row domain " + to_string(revised->id()))) {
                return error;
            }
            for (std::size_t index = 0; index < columns.size(); ++index) {
                if (auto error =
                        publish_version(transaction, store, context.transaction_id, published[index],
                                        std::nullopt, "column " + std::to_string(columns[index]))) {
                    return error;
                }
                if (auto error = write_property(transaction, columns[index], property::kVersion,
                                             std::to_string(published[index]->id().raw()),
                                             "column " + std::to_string(columns[index]))) {
                    return error;
                }
            }
            return write_property(transaction, worksheet, property::kRowDomain,
                               std::to_string(revised->id().raw()),
                               "worksheet " + std::to_string(worksheet));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.dataset_derive publishes a version whose provenance names two input versions.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kDatasetDerive;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            return args.all_ids({argument::kDataset, argument::kVersionDraft, argument::kInputA,
                                 argument::kInputB});
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                             const CommandRequest& request,
                                             const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t dataset = 0;
            std::uint64_t draft_id = 0;
            std::uint64_t input_a = 0;
            std::uint64_t input_b = 0;
            if (auto error = args.id(argument::kDataset, dataset)) {
                return error;
            }
            if (auto error = args.id(argument::kVersionDraft, draft_id)) {
                return error;
            }
            if (auto error = args.id(argument::kInputA, input_a)) {
                return error;
            }
            if (auto error = args.id(argument::kInputB, input_b)) {
                return error;
            }
            std::string parameters;
            if (auto error = args.text(argument::kParameters, parameters)) {
                return error;
            }
            auto version = drafts.version(draft_id);
            if (version == nullptr) {
                return fail(DetailCode::unknown_draft, "the candidate version is not available",
                            "version_draft " + std::to_string(draft_id));
            }
            if (version->dataset() != DatasetId{ObjectId{dataset}}) {
                return fail(DetailCode::type_mismatch, "the derived version names another dataset",
                            "dataset " + std::to_string(dataset));
            }
            if (version->provenance().canonical_parameters != parameters) {
                return fail(DetailCode::type_mismatch,
                            "the candidate version records different canonical parameters",
                            "dataset " + std::to_string(dataset));
            }
            for (const std::uint64_t input : {input_a, input_b}) {
                auto stored = store.version(DatasetVersionId{ObjectId{input}});
                if (!stored.ok()) {
                    return stored.diagnostic().to_error();
                }
                const auto& inputs = version->provenance().inputs;
                const auto found = std::find_if(inputs.begin(), inputs.end(),
                                                [&](const VersionInput& candidate) {
                                                    return candidate.version == stored.value()->id();
                                                });
                if (found == inputs.end()) {
                    return fail(DetailCode::dangling_reference,
                                "the derived version must record this input version in its provenance",
                                "input version " + std::to_string(input));
                }
                if (found->content_hash != stored.value()->content_hash()) {
                    return fail(DetailCode::dangling_reference,
                                "the recorded input content hash does not match the stored version",
                                "input version " + std::to_string(input));
                }
            }
            if (version->provenance().inputs.size() != 2) {
                return fail(DetailCode::dangling_reference,
                            "a derived version records exactly its two input versions",
                            "dataset " + std::to_string(dataset));
            }
            return publish_version(transaction, store, context.transaction_id, version,
                                   std::nullopt, "dataset " + std::to_string(dataset));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    // data.column_delete and data.dataset_version_delete refuse to strand a dependent.
    for (const auto* name : {command_id::kColumnDelete, command_id::kVersionDelete}) {
        const bool delete_column = name == std::string_view{command_id::kColumnDelete};
        application::CommandDefinition definition;
        definition.command_id = name;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [delete_column](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t target = 0;
            return args.id(delete_column ? argument::kColumn : argument::kVersion, target);
        };
        definition.execute = [&store](CommandTransaction& transaction, const CommandRequest& request,
                                     const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            const bool deleting_column =
                request.command_id == command_id::kColumnDelete;
            std::uint64_t id = 0;
            std::uint64_t version_id = 0;
            if (deleting_column) {
                if (auto error = args.id(argument::kColumn, id)) {
                    return error;
                }
                const ObjectState* binding = nullptr;
                if (auto error = require_object(transaction, id, object_type::kColumnBinding, binding,
                                               "column " + std::to_string(id))) {
                    return error;
                }
                if (auto error = require_property(*binding, property::kVersion, version_id,
                                                 "column " + std::to_string(id))) {
                    return error;
                }
            } else {
                if (auto error = args.id(argument::kVersion, version_id)) {
                    return error;
                }
                const ObjectState* version_state = nullptr;
                if (auto error = require_object(transaction, version_id, object_type::kDatasetVersion,
                                               version_state, "version " + std::to_string(version_id))) {
                    return error;
                }
                id = version_id;
            }

            const DatasetVersionId target{ObjectId{version_id}};
            auto report_dependents = [&](const std::vector<std::string>& dependents) -> std::optional<Error> {
                if (dependents.empty()) {
                    return std::nullopt;
                }
                std::string names;
                for (const auto& dependent : dependents) {
                    if (!names.empty()) {
                        names += "; ";
                    }
                    names += dependent;
                }
                return fail(DetailCode::dangling_reference,
                            "this delete would strand a dependent, so no dangling id is left behind",
                            std::move(names));
            };

            std::vector<std::string> dependents;
            if (!deleting_column) {
                for (const auto& state : transaction.objects()) {
                    if (state.type != object_type::kColumnBinding) {
                        continue;
                    }
                    const auto referenced = read_id_property(state, property::kVersion);
                    if (referenced.has_value() && *referenced == version_id) {
                        dependents.push_back("binding " + to_string(ColumnId{state.id}));
                    }
                }
            }
            for (const auto& derived : store.versions_referencing(target)) {
                dependents.push_back("derived version " + to_string(derived));
            }
            if (auto error = report_dependents(dependents)) {
                return error;
            }
            auto error = transaction.erase_object(ObjectId{id});
            return from_kernel(error, DetailCode::dangling_reference,
                               "delete target " + std::to_string(id));
        };
        if (auto error = register_definition(registry, definition)) {
            return report(error);
        }
    }

    return std::nullopt;
}

DataCommandExecutor::DataCommandExecutor(application::CommandGateway& gateway, DataStore& store)
    : gateway_(gateway), store_(store) {}

application::CommandOutcome DataCommandExecutor::execute(const application::CommandRequest& request,
                                                         std::stop_token stop_token) {
    auto outcome = gateway_.execute(request, stop_token);
    if (outcome.transaction_id.has_value()) {
        if (outcome.succeeded() && outcome.changed) {
            store_.apply_commit(*outcome.transaction_id);
        } else {
            store_.discard(*outcome.transaction_id);
        }
    }
    return outcome;
}

application::CommandOutcome DataCommandExecutor::undo(std::stop_token stop_token) {
    auto outcome = gateway_.undo(stop_token);
    if (outcome.transaction_id.has_value() && !outcome.succeeded()) {
        store_.discard(*outcome.transaction_id);
    }
    return outcome;
}

application::CommandOutcome DataCommandExecutor::redo(std::stop_token stop_token) {
    auto outcome = gateway_.redo(stop_token);
    if (outcome.transaction_id.has_value() && !outcome.succeeded()) {
        store_.discard(*outcome.transaction_id);
    }
    return outcome;
}

Result<DocumentView> DataCommandExecutor::document() const {
    return DocumentView::build(gateway_.snapshot(), store_);
}

const application::DocumentSnapshot DataCommandExecutor::snapshot() const {
    return gateway_.snapshot();
}

}  // namespace everygraph::data
