#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "../data_test_support.hpp"
#include "everygraph/application/event.hpp"
#include "everygraph/data/commands/worksheet_edit.hpp"

namespace egtest {

using app::DocumentEvent;
using app::ObjectChange;

// The command stack the edit tests drive: one registry, one store, one draft registry and one
// executor holding both the accepted create-and-publish family and this task's edit family, so
// a row, cell or column edit travels the same path a table view uses.
//
// `egtest::Fixture` cannot be reused for this: it registers only the accepted family and the
// gateway copies the registry at construction, so a second family cannot be added afterwards.
// The store and drafts are the same public types, and every helper of `data_test_support.hpp`
// that takes no fixture is used unchanged.
class EditFixture {
public:
    explicit EditFixture(std::size_t max_undo_entries = 128) {
        auto registered = data::register_data_commands(registry_, store_, drafts_);
        if (registered.has_value()) {
            throw TestFailure("create-family registration failed: " +
                              registered->to_error().message);
        }
        registered = data::register_worksheet_edit_commands(registry_, store_, drafts_);
        if (registered.has_value()) {
            throw TestFailure("edit-family registration failed: " + registered->to_error().message);
        }
        app::GatewayOptions options;
        options.max_undo_entries = max_undo_entries;
        gateway_.emplace(registry_, ValidatedDocument{}, options);
        executor_.emplace(*gateway_, store_);
        gateway_->subscribe([this](const DocumentEvent& event) { events_.push_back(event); });
    }

    EditFixture(const EditFixture&) = delete;
    EditFixture& operator=(const EditFixture&) = delete;

    [[nodiscard]] CommandOutcome run(std::string name, ParameterMap arguments,
                                     ExpectedRevisions expected = {}) {
        CommandRequest request;
        request.command_id = std::move(name);
        request.schema_version = 1;
        request.request_id = "request-" + std::to_string(++request_counter_);
        request.principal = "test-principal";
        request.project_id = "test-project";
        request.expected_revisions = std::move(expected);
        request.arguments = std::move(arguments);
        return executor_->execute(request);
    }

    [[nodiscard]] CommandOutcome require_run(std::string name, ParameterMap arguments,
                                             ExpectedRevisions expected = {}) {
        const std::string label = name;
        auto outcome = run(std::move(name), std::move(arguments), std::move(expected));
        if (!outcome.succeeded()) {
            throw TestFailure("command " + label + " failed: " +
                              (outcome.error.has_value() ? outcome.error->message
                                                         : std::string{"no error"}));
        }
        return outcome;
    }

    [[nodiscard]] CommandOutcome undo() { return executor_->undo(); }
    [[nodiscard]] CommandOutcome redo() { return executor_->redo(); }
    [[nodiscard]] CommandOutcome repeat_last() {
        return gateway_->repeat_last("repeat-" + std::to_string(++request_counter_));
    }

    [[nodiscard]] CommandOutcome require_undo() {
        auto outcome = undo();
        if (!outcome.succeeded()) {
            throw TestFailure("undo failed: " + (outcome.error.has_value()
                                                     ? outcome.error->message
                                                     : std::string{"no error"}));
        }
        return outcome;
    }
    [[nodiscard]] CommandOutcome require_redo() {
        auto outcome = redo();
        if (!outcome.succeeded()) {
            throw TestFailure("redo failed: " + (outcome.error.has_value()
                                                     ? outcome.error->message
                                                     : std::string{"no error"}));
        }
        return outcome;
    }

    [[nodiscard]] DocumentView document() {
        auto view = executor_->document();
        if (!view.ok()) {
            throw TestFailure("document view rejected: " + view.diagnostic().to_error().message);
        }
        return view.take();
    }

    [[nodiscard]] std::shared_ptr<const DatasetVersion> version(DatasetVersionId id) {
        auto found = store_.version(id);
        if (!found.ok()) {
            throw TestFailure("version is not visible: " + found.diagnostic().to_error().message);
        }
        return found.take();
    }

    [[nodiscard]] std::shared_ptr<const RowDomain> row_domain(RowDomainId id) {
        auto found = store_.row_domain(id);
        if (!found.ok()) {
            throw TestFailure("row domain is not visible: " + found.diagnostic().to_error().message);
        }
        return found.take();
    }

    [[nodiscard]] std::optional<ObjectState> object(std::uint64_t id) const {
        const app::DocumentSnapshot snapshot = executor_->snapshot();
        const auto* state = snapshot.find(ObjectId{id});
        if (state == nullptr) {
            return std::nullopt;
        }
        return *state;
    }

    [[nodiscard]] Revision revision_of(std::uint64_t id) const {
        const auto state = object(id);
        return state.has_value() ? state->revision : everygraph::foundation::kInitialRevision;
    }

    [[nodiscard]] std::uint64_t undo_depth() const { return gateway_->undo_depth(); }
    [[nodiscard]] std::uint64_t redo_depth() const { return gateway_->redo_depth(); }
    [[nodiscard]] std::uint64_t version_count() const { return store_.version_count(); }
    [[nodiscard]] std::uint64_t staged_count() const { return store_.staged_count(); }
    [[nodiscard]] const std::vector<DocumentEvent>& events() const noexcept { return events_; }
    [[nodiscard]] std::optional<app::LastActionRecord> last_action() const {
        return gateway_->last_action();
    }

    // The kernel refuses a mutation issued while it is publishing an event. The probe holds the
    // outcome instead of throwing from inside the listener, so a case can assert the code.
    void arm_reentry_probe(std::string name, ParameterMap arguments) {
        probe_command_ = std::move(name);
        probe_arguments_ = std::move(arguments);
        gateway_->subscribe([this](const DocumentEvent&) {
            if (!probe_command_.has_value() || probe_result_.has_value()) {
                return;
            }
            const std::string command = *probe_command_;
            const ParameterMap arguments = probe_arguments_;
            probe_command_ = std::nullopt;
            probe_result_ = run(command, arguments);
        });
    }
    [[nodiscard]] const std::optional<CommandOutcome>& reentry_probe() const noexcept {
        return probe_result_;
    }

    DataStore& store() noexcept { return store_; }
    DraftRegistry& drafts() noexcept { return drafts_; }
    CommandGateway& gateway() noexcept { return *gateway_; }

    [[nodiscard]] DraftId stage(std::shared_ptr<const DatasetVersion> version) {
        return drafts_.add(std::move(version));
    }
    [[nodiscard]] DraftId stage(std::shared_ptr<const RowDomain> domain) {
        return drafts_.add(std::move(domain));
    }

    [[nodiscard]] static ParameterMap with(std::string_view key, std::uint64_t value) {
        ParameterMap arguments;
        put(arguments, key, value);
        return arguments;
    }

    static void put(ParameterMap& arguments, std::string_view key, std::uint64_t value) {
        arguments[std::string{key}] = Value{std::to_string(value)};
    }

    static void put(ParameterMap& arguments, std::string_view key, std::string value) {
        arguments[std::string{key}] = Value{std::move(value)};
    }

    [[nodiscard]] static std::string id_list(const std::vector<std::uint64_t>& ids) {
        std::string text;
        for (const std::uint64_t id : ids) {
            if (!text.empty()) {
                text += ";";
            }
            text += std::to_string(id);
        }
        return text;
    }

    [[nodiscard]] ExpectedRevisions declared_writes(const std::vector<std::uint64_t>& ids) const {
        const app::DocumentSnapshot snapshot = executor_->snapshot();
        ExpectedRevisions expected;
        for (const std::uint64_t id : ids) {
            const auto* state = snapshot.find(ObjectId{id});
            expected.writes[ObjectId{id}] =
                state == nullptr ? everygraph::foundation::kInitialRevision : state->revision;
        }
        return expected;
    }

    [[nodiscard]] ExpectedRevisions expect_revision(std::uint64_t id, Revision revision) {
        ExpectedRevisions expected;
        expected.writes[ObjectId{id}] = revision;
        return expected;
    }

private:
    CommandRegistry registry_;
    DataStore store_;
    DraftRegistry drafts_;
    std::optional<CommandGateway> gateway_;
    std::optional<DataCommandExecutor> executor_;
    std::vector<DocumentEvent> events_;
    std::uint64_t request_counter_{0};
    std::optional<std::string> probe_command_;
    ParameterMap probe_arguments_;
    std::optional<CommandOutcome> probe_result_;
};

// ---------------------------------------------------------------------------
// Scenario setup. The identities come from `make_table`, so a test that needs a second
// worksheet allocates another base rather than reusing an id.
// ---------------------------------------------------------------------------

inline void create_chain(EditFixture& fixture, SimpleTable& table) {
    ParameterMap project = EditFixture::with(data::argument::kProject, table.project);
    EditFixture::put(project, data::argument::kLabel, std::string{"project"});
    (void)fixture.require_run(data::command_id::kProjectCreate, std::move(project),
                              fixture.declared_writes({table.project}));

    ParameterMap workbook = EditFixture::with(data::argument::kWorkbook, table.workbook);
    EditFixture::put(workbook, data::argument::kProject, table.project);
    EditFixture::put(workbook, data::argument::kLabel, std::string{"workbook"});
    (void)fixture.require_run(data::command_id::kWorkbookCreate, std::move(workbook),
                              fixture.declared_writes({table.workbook}));

    ParameterMap worksheet = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(worksheet, data::argument::kWorkbook, table.workbook);
    EditFixture::put(worksheet, data::argument::kDomain, table.domain);
    EditFixture::put(worksheet, data::argument::kLabel, std::string{"worksheet"});
    (void)fixture.require_run(data::command_id::kWorksheetCreate, std::move(worksheet),
                              fixture.declared_writes({table.worksheet, table.domain}));
}

// Stages a candidate version and asks the accepted create command to publish it as a column.
// `required` builds the field non-nullable, which is how a test reaches the rule that a blank
// cell in a column that declares no missing state is refused rather than stored as a zero.
[[nodiscard]] inline ColumnLine add_column(EditFixture& fixture, SimpleTable& table,
                                           const ColumnSpec& spec, bool required = false) {
    ColumnLine line;
    line.column = table.next_column++;
    line.dataset = table.next_dataset++;
    line.field = table.next_field++;
    line.version = table.next_version++;
    line.same_domain = !spec.foreign_domain.has_value();

    const RowDomainId domain = spec.foreign_domain.has_value()
                                   ? data::row_domain_id(*spec.foreign_domain)
                                   : data::row_domain_id(table.domain);
    std::vector<FieldSchema> fields{
        field(data::field_id(line.field), spec.label, spec.type, spec.dictionary, spec.unit,
              spec.unknown_category, spec.zone)};
    fields.front().nullable = !required;
    auto schema = schema_of(std::move(fields));
    auto chunk = chunk_of(data::field_id(line.field), spec.type,
                          ChunkId{ObjectId{table.next_chunk++}}, spec.cells);
    auto version = version_of(data::dataset_id(line.dataset),
                              data::dataset_version_id(line.version), domain, schema, {chunk},
                              spec.cells.size(), provenance_of(data::command_id::kColumnCreate));
    const DraftId draft = fixture.stage(version);

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kDataset, line.dataset);
    EditFixture::put(arguments, data::argument::kVersionDraft, draft);
    EditFixture::put(arguments, data::argument::kLabel, spec.label);
    EditFixture::put(arguments, data::argument::kRole, spec.role);
    // Each column joins the end of the display order, so a case that wants another order asks
    // for it with the order command rather than by inventing a slot.
    EditFixture::put(arguments, data::argument::kOrderIndex,
                     static_cast<std::uint64_t>(table.columns.size()));
    if (!spec.unit.empty()) {
        EditFixture::put(arguments, data::argument::kUnit, spec.unit);
    }
    if (!spec.display_format.empty()) {
        EditFixture::put(arguments, data::argument::kDisplayFormat, spec.display_format);
    }
    (void)fixture.require_run(data::command_id::kColumnCreate, std::move(arguments),
                              fixture.declared_writes({line.column, line.version}));
    table.columns.push_back(line);
    return line;
}

// Aliases a committed version as a read-only mirror, which is the only form the accepted create
// family offers. No new version is published, so the sheet holds two bindings over one version
// identity and the edit family has to refuse to rewrite either of them.
[[nodiscard]] inline ColumnLine add_mirror(EditFixture& fixture, SimpleTable& table,
                                           const ColumnLine& source) {
    ColumnLine line;
    line.column = table.next_column++;
    line.dataset = source.dataset;
    line.field = source.field;
    line.version = source.version;
    // A mirror is not one of the columns a row-shape edit rewrites: its version identity is the
    // source's, so naming a successor for it would spend an identity on a column nobody edits.
    line.same_domain = false;

    ParameterMap arguments = EditFixture::with(data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kDataset, line.dataset);
    EditFixture::put(arguments, data::argument::kSourceVersion, line.version);
    EditFixture::put(arguments, data::argument::kWritePolicy, std::string{"mirror"});
    EditFixture::put(arguments, data::argument::kLabel, std::string{"mirror"});
    EditFixture::put(arguments, data::argument::kOrderIndex,
                     static_cast<std::uint64_t>(table.columns.size()));
    (void)fixture.require_run(data::command_id::kColumnCreate, std::move(arguments),
                              fixture.declared_writes({line.column}));
    table.columns.push_back(line);
    return line;
}

// Publishes a candidate version as a column at a declared position. Every column at or after the
// slot gains one order index in the same revision, so the caller's record of the sheet is
// renumbered too: `all_columns` must stay in display order, because the row-shape commands name
// their successors in it.
[[nodiscard]] inline ColumnLine insert_column(EditFixture& fixture, SimpleTable& table,
                                              std::uint64_t order_index, const ColumnSpec& spec) {
    ColumnLine line;
    line.column = table.next_column++;
    line.dataset = table.next_dataset++;
    line.field = table.next_field++;
    line.version = table.next_version++;
    line.same_domain = !spec.foreign_domain.has_value();

    const RowDomainId domain = spec.foreign_domain.has_value()
                                   ? data::row_domain_id(*spec.foreign_domain)
                                   : data::row_domain_id(table.domain);
    auto schema = schema_of({field(data::field_id(line.field), spec.label, spec.type,
                                   spec.dictionary, spec.unit, spec.unknown_category, spec.zone)});
    auto chunk = chunk_of(data::field_id(line.field), spec.type,
                          ChunkId{ObjectId{table.next_chunk++}}, spec.cells);
    auto version = version_of(data::dataset_id(line.dataset), data::dataset_version_id(line.version),
                              domain, schema, {chunk}, spec.cells.size(),
                              provenance_of(data::command_id::kWorksheetColumnInsert));
    const DraftId draft = fixture.stage(version);

    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kColumn, line.column);
    EditFixture::put(arguments, data::argument::kDataset, line.dataset);
    EditFixture::put(arguments, data::argument::kVersionDraft, draft);
    EditFixture::put(arguments, data::argument::kOrderIndex, order_index);
    EditFixture::put(arguments, data::argument::kLabel, spec.label);
    if (!spec.role.empty()) {
        EditFixture::put(arguments, data::argument::kRole, spec.role);
    }
    if (!spec.unit.empty()) {
        EditFixture::put(arguments, data::argument::kUnit, spec.unit);
    }
    if (!spec.display_format.empty()) {
        EditFixture::put(arguments, data::argument::kDisplayFormat, spec.display_format);
    }
    std::vector<std::uint64_t> writes{line.column, line.version};
    for (const ColumnLine& held : table.columns) {
        writes.push_back(held.column);
    }
    (void)fixture.require_run(data::command_id::kWorksheetColumnInsert, std::move(arguments),
                              fixture.declared_writes(writes));
    const auto at = table.columns.begin() + static_cast<std::ptrdiff_t>(order_index);
    table.columns.insert(at, line);
    return line;
}

// The row identities the worksheet's current domain holds, in its own order.
[[nodiscard]] inline std::vector<RowId> sheet_rows(EditFixture& fixture, const SimpleTable& table) {
    return fixture.row_domain(data::row_domain_id(table.domain))->rows();
}

// Seeds the worksheet with a row domain of exactly these identities, through the accepted
// draft-based row-change command, so the edit commands are tested against a sheet that already
// holds rows. No column exists yet, so no successor is published.
inline void seed_rows(EditFixture& fixture, SimpleTable& table, const std::vector<RowId>& rows) {
    const RowDomainId revised_id = data::row_domain_id(table.next_domain++);
    auto domain = domain_of(revised_id, rows);
    const DraftId domain_draft = fixture.stage(domain);

    ParameterMap arguments = EditFixture::with(data::argument::kWorksheet, table.worksheet);
    EditFixture::put(arguments, data::argument::kDomainDraft, domain_draft);
    EditFixture::put(arguments, data::argument::kOperation, std::string{"append"});
    EditFixture::put(arguments, data::argument::kColumns, std::string{});
    EditFixture::put(arguments, data::argument::kVersionDrafts, std::string{});
    (void)fixture.require_run(data::command_id::kWorksheetChangeRows, std::move(arguments),
                              fixture.declared_writes({table.worksheet, revised_id.raw()}));
    table.rows = rows;
    table.domain = revised_id.raw();
}

// Names the successor identities an edit command rewrites its columns under: one version and one
// chunk per listed column, in display order. `writes` are the declared-write-set entries they add.
struct SuccessorNames {
    std::vector<std::uint64_t> versions;
    std::vector<std::uint64_t> writes;
};

[[nodiscard]] inline SuccessorNames name_successors(SimpleTable& table, ParameterMap& arguments,
                                                    const std::vector<ColumnLine>& lines) {
    SuccessorNames names;
    std::vector<std::uint64_t> chunks;
    for (const ColumnLine& line : lines) {
        const std::uint64_t version = table.next_version++;
        names.versions.push_back(version);
        names.writes.push_back(version);
        names.writes.push_back(line.column);
        chunks.push_back(table.next_chunk++);
    }
    EditFixture::put(arguments, data::argument::kSuccessorVersions,
                     EditFixture::id_list(names.versions));
    EditFixture::put(arguments, data::argument::kSuccessorChunks, EditFixture::id_list(chunks));
    return names;
}

// A row-shape edit builds and publishes its own revised row domain, under this identity.
[[nodiscard]] inline std::uint64_t name_row_domain(SimpleTable& table, ParameterMap& arguments) {
    const std::uint64_t revised = table.next_domain++;
    EditFixture::put(arguments, data::argument::kRevisedDomain, revised);
    return revised;
}

// Moves the caller's record of the sheet onto what an edit published. A single-column edit leaves
// the worksheet's domain alone, so a revised domain of zero changes nothing.
inline void adopt(SimpleTable& table, std::uint64_t revised_domain, const SuccessorNames& names,
                  const std::vector<ColumnLine>& lines) {
    if (revised_domain != 0) {
        table.domain = revised_domain;
    }
    for (std::size_t index = 0; index < lines.size() && index < names.versions.size(); ++index) {
        for (ColumnLine& stored : table.columns) {
            if (stored.column == lines[index].column) {
                stored.version = names.versions[index];
            }
        }
    }
}

// A single-column edit names one line and changes no domain.
inline void adopt(SimpleTable& table, const SuccessorNames& names, const ColumnLine& line) {
    adopt(table, 0, names, {line});
}

// The write set a row-shape edit declares: the worksheet, the revised domain, and every column
// plus the successor version it is named under.
[[nodiscard]] inline ExpectedRevisions row_edit_writes(EditFixture& fixture, SimpleTable& table,
                                                       std::uint64_t revised_domain,
                                                       const std::vector<ColumnLine>& lines,
                                                       const std::vector<std::uint64_t>& extra) {
    std::vector<std::uint64_t> ids{table.worksheet, revised_domain};
    for (const ColumnLine& line : lines) {
        ids.push_back(line.column);
    }
    ids.insert(ids.end(), extra.begin(), extra.end());
    return fixture.declared_writes(ids);
}

[[nodiscard]] inline std::vector<ColumnLine> all_columns(const SimpleTable& table) {
    std::vector<ColumnLine> editable;
    for (const ColumnLine& line : table.columns) {
        if (line.same_domain) {
            editable.push_back(line);
        }
    }
    return editable;
}

// The worksheet's bindings as the accepted typed view reads them, in display order. This is where
// a case checks that an edit left each column's label, role, unit, format and binding identity
// intact, because the view is the reading a table view itself uses.
[[nodiscard]] inline std::vector<ColumnBindingRecord> sheet_bindings(EditFixture& fixture,
                                                                     std::uint64_t worksheet) {
    return fixture.document().bindings_of(data::worksheet_id(worksheet));
}

// The row identities a case expects, spelled by their numbers.
[[nodiscard]] inline std::vector<RowId> rows_of(std::initializer_list<std::uint64_t> values) {
    std::vector<RowId> rows;
    rows.reserve(values.size());
    for (const std::uint64_t value : values) {
        rows.push_back(data::row_id(value));
    }
    return rows;
}

// A row list and a cell list as text, so a failed equality names what the sheet holds instead of
// only the source line.
[[nodiscard]] inline std::string row_text(const std::vector<RowId>& rows) {
    std::string text;
    for (const RowId& row : rows) {
        if (!text.empty()) {
            text += ",";
        }
        text += data::to_string(row);
    }
    return text;
}

[[nodiscard]] inline std::string cell_text(const std::vector<Cell>& cells) {
    std::string text;
    for (const Cell& cell : cells) {
        if (!text.empty()) {
            text += "|";
        }
        text += data::describe(cell);
    }
    return text;
}

// Cell-by-cell structural equality: exact type, exact state class and exact bits.
inline void require_cells(const std::vector<Cell>& actual, const std::vector<Cell>& expected) {
    EG_REQUIRE_EQ(actual.size(), expected.size());
    for (std::size_t index = 0; index < actual.size(); ++index) {
        EG_REQUIRE_CELL_EQ(actual[index], expected[index]);
    }
}

// The cells one column of the current version holds, in its own row order.
[[nodiscard]] inline std::vector<Cell> column_cells(EditFixture& fixture, const ColumnLine& line) {
    auto current = fixture.version(data::dataset_version_id(line.version));
    return version_cells(*current, data::field_id(line.field));
}

}  // namespace egtest
