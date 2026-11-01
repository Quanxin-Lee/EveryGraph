#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/data/cell.hpp"
#include "everygraph/data/chunk.hpp"
#include "everygraph/data/commands.hpp"
#include "everygraph/data/dataset.hpp"
#include "everygraph/data/datetime.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/document.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/logical_type.hpp"
#include "everygraph/data/provenance.hpp"
#include "everygraph/data/row_domain.hpp"
#include "everygraph/data/units.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace egtest {

namespace data = everygraph::data;
namespace app = everygraph::application;

using app::CommandGateway;
using app::CommandOutcome;
using app::CommandRegistry;
using app::CommandRequest;
using app::ExpectedRevisions;
using app::ObjectState;
using app::ParameterMap;
using app::ValidatedDocument;
using app::Value;
using data::Cell;
using data::Chunk;
using data::ChunkBuilder;
using data::ChunkId;
using data::ColumnBindingRecord;
using data::CorrespondenceId;
using data::DataCommandExecutor;
using data::DataStore;
using data::DatasetId;
using data::DatasetLease;
using data::DatasetSchema;
using data::DatasetVersion;
using data::DatasetVersionId;
using data::DetailCode;
using data::Diagnostic;
using data::Dictionary;
using data::DocumentView;
using data::DraftId;
using data::DraftRegistry;
using data::FieldId;
using data::FieldSchema;
using data::LogicalType;
using data::ProjectId;
using data::Provenance;
using data::Result;
using data::RowCorrespondence;
using data::RowDomain;
using data::RowDomainId;
using data::RowId;
using data::Severity;
using data::UnknownCategoryPolicy;
using data::VersionInput;
using data::WorkbookId;
using data::WorksheetId;
using everygraph::foundation::Error;
using everygraph::foundation::ErrorCode;
using everygraph::foundation::ObjectId;
using everygraph::foundation::Revision;

// ---------------------------------------------------------------------------
// Dependency-free test harness. The P1 kernel tests use the same shape: no
// test framework is a declared dependency yet, so a case is a registered
// function and an assertion is a throw.
// ---------------------------------------------------------------------------

struct TestFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

using TestFunction = std::function<void()>;

struct TestCase {
    std::string name;
    TestFunction function;
};

inline std::vector<TestCase>& test_cases() {
    static std::vector<TestCase> cases;
    return cases;
}

struct TestRegistrar {
    TestRegistrar(std::string name, TestFunction function) {
        test_cases().push_back(TestCase{std::move(name), std::move(function)});
    }
};

// A case name carries the full `EG-FX-<FAMILY>-<NNN>` identifier it exercises, which is how
// the conformance plan requires a test to report the fixtures it covered.
#define EG_TEST(family, number, name)                                                     \
    static void name();                                                                   \
    [[maybe_unused]] static const egtest::TestRegistrar name##_registrar(                 \
        "EG-FX-" family "-" number ": " #name, name);                                      \
    static void name()

#define EG_REQUIRE(expression)                                                            \
    do {                                                                                  \
        if (!(expression)) {                                                              \
            std::ostringstream stream;                                                     \
            stream << __FILE__ << ':' << __LINE__ << ": requirement failed: " << #expression; \
            throw egtest::TestFailure(stream.str());                                       \
        }                                                                                 \
    } while (false)

template <typename T>
std::string display(const T& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

inline std::string display(const std::string& value) {
    return "\"" + value + "\"";
}

inline std::string display(std::string_view value) {
    return "\"" + std::string{value} + "\"";
}

template <typename Tag>
std::string display(const data::StableId<Tag>& value) {
    return data::to_string(value);
}

template <typename T>
std::string display(const std::optional<T>& value) {
    return value.has_value() ? display(*value) : std::string{"nullopt"};
}

inline std::string display(const data::Dimension& dimension) {
    return dimension.describe();
}

inline std::string display(const data::Unit& unit) {
    return unit.source_text().empty() ? unit.canonical_text() : unit.source_text();
}

inline std::string display(const data::ContentHash& hash) {
    return hash.hex();
}

inline std::string display(const Cell& cell) {
    return data::describe(cell);
}

inline std::string display(LogicalType type) {
    return std::string{data::to_string(type)};
}

inline std::string display(DetailCode code) {
    return std::string{data::to_string(code)};
}

inline std::string display(Severity severity) {
    return std::string{data::to_string(severity)};
}

inline std::string display(ErrorCode code) {
    return std::string{everygraph::foundation::to_string(code)};
}

inline std::string display(data::TickUnit unit) {
    return std::string{data::to_string(unit)};
}

inline std::string display(data::ColumnRole role) {
    return std::string{data::to_string(role)};
}

inline std::string display(data::MappingMethod method) {
    return std::string{data::to_string(method)};
}

inline std::string display(data::UnknownCategoryPolicy policy) {
    return std::string{data::to_string(policy)};
}

inline std::string display(data::UnitKind kind) {
    return std::string{data::to_string(kind)};
}

inline std::string display(data::WritePolicy policy) {
    return std::string{data::to_string(policy)};
}

inline std::string display(data::DomainLink link) {
    return std::string{data::to_string(link)};
}

template <typename Actual, typename Expected>
void require_equal(const Actual& actual, const Expected& expected, const char* actual_expression,
                   const char* expected_expression, const char* file, int line) {
    if (!(actual == expected)) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected " +
                          actual_expression + " == " + expected_expression + ", got " +
                          display(actual) + " and " + display(expected));
    }
}

#define EG_REQUIRE_EQ(actual, expected) \
    require_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

// Structural golden comparison for a stored value: exact type, exact state class and exact
// bits, with all NaN payloads equal and signed zeros distinct, as the plan's section 3.4 says.
inline void require_cell_equal(const Cell& actual, const Cell& expected, const char* file, int line) {
    if (!data::structurally_equal(actual, expected)) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected cell " +
                          display(expected) + ", got " + display(actual));
    }
}

#define EG_REQUIRE_CELL_EQ(actual, expected) \
    require_cell_equal((actual), (expected), __FILE__, __LINE__)

// Checks a Result and reports its detail code and location when it failed, so a failing
// golden case names the diagnostic instead of only the source line.
template <typename T>
const T& require_ok(const Result<T>& result, const char* expression, const char* file, int line) {
    if (!result.ok()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected " +
                          expression + " to succeed, got " +
                          data::to_string(result.diagnostic().detail) + " severity=" +
                          data::to_string(result.diagnostic().severity) + " message=" +
                          result.diagnostic().message + " at " +
                          result.diagnostic().location.describe());
    }
    return result.value();
}

template <typename T>
T& require_ok(Result<T>& result, const char* expression, const char* file, int line) {
    if (!result.ok()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected " +
                          expression + " to succeed, got " +
                          data::to_string(result.diagnostic().detail) + " message=" +
                          result.diagnostic().message + " at " +
                          result.diagnostic().location.describe());
    }
    return result.value();
}

#define EG_REQUIRE_OK(expression) require_ok((expression), #expression, __FILE__, __LINE__)

// Moves the value out of a Result temporary, so a reader never keeps a reference into the
// Result expression it came from.
template <typename T>
[[nodiscard]] T require_taken(Result<T>&& result, const char* expression, const char* file,
                              int line) {
    if (!result.ok()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected " +
                          expression + " to succeed, got " +
                          data::to_string(result.diagnostic().detail) + " message=" +
                          result.diagnostic().message + " at " +
                          result.diagnostic().location.describe());
    }
    return result.take();
}

#define EG_REQUIRE_TAKEN(expression) require_taken((expression), #expression, __FILE__, __LINE__)

[[nodiscard]] inline Error require_failure(const Diagnostic& diagnostic, const char* file,
                                                      int line) {
    (void)file;
    (void)line;
    return diagnostic.to_error();
}

template <typename T>
[[nodiscard]] Error require_failure(const Result<T>& result, const char* expression,
                                               const char* file, int line) {
    if (result.ok()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected " +
                          expression + " to fail");
    }
    return require_failure(result.diagnostic(), file, line);
}

#define EG_REQUIRE_FAILURE(expression) require_failure((expression), #expression, __FILE__, __LINE__)

// Asserts the documented detail code, severity and outer application code, which is what the
// plan requires in place of a prose message match.
inline void require_diagnostic(const Diagnostic& diagnostic, DetailCode expected_detail,
                               ErrorCode expected_outer, Severity expected_severity,
                               const char* file, int line) {
    if (diagnostic.detail != expected_detail || diagnostic.outer != expected_outer ||
        diagnostic.severity != expected_severity) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected diagnostic " +
                          data::to_string(expected_detail) + "/" +
                          everygraph::foundation::to_string(expected_outer) + "/" +
                          data::to_string(expected_severity) + ", got " +
                          data::to_string(diagnostic.detail) + "/" +
                          everygraph::foundation::to_string(diagnostic.outer) + "/" +
                          data::to_string(diagnostic.severity) + " message=" + diagnostic.message);
    }
}

#define EG_REQUIRE_DIAGNOSTIC(diagnostic, detail, outer, severity) \
    require_diagnostic((diagnostic), (detail), (outer), (severity), __FILE__, __LINE__)

// Asserts the observable outcome of a rejected command: the outer code, plus the `data.`
// detail code and its location, which travel inside the kernel's single English message.
inline void require_rejected(const CommandOutcome& outcome, DetailCode expected_detail,
                             ErrorCode expected_outer, const char* file, int line) {
    if (outcome.succeeded()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) +
                          ": expected the command to be rejected");
    }
    if (!outcome.error.has_value()) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) +
                          ": a rejected command must carry a structured error");
    }
    if (outcome.error->code != expected_outer) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) + ": expected outer code " +
                          everygraph::foundation::to_string(expected_outer) + ", got " +
                          everygraph::foundation::to_string(outcome.error->code) + " message=" +
                          outcome.error->message);
    }
    const std::string expected{data::to_string(expected_detail)};
    if (outcome.error->message.find(expected) == std::string::npos) {
        throw TestFailure(std::string{file} + ":" + std::to_string(line) +
                          ": expected the message to carry detail code " + expected + ", got " +
                          outcome.error->message);
    }
}

#define EG_REQUIRE_REJECTED(outcome, detail, outer) \
    require_rejected((outcome), (detail), (outer), __FILE__, __LINE__)

// Asserts that a rejection left the store and the document exactly as they were.
#define EG_REQUIRE_NO_WRITE(outcome)                              \
    do {                                                          \
        EG_REQUIRE(!(outcome).succeeded());                       \
        EG_REQUIRE(!(outcome).changed);                           \
        EG_REQUIRE((outcome).events.empty());                     \
        EG_REQUIRE((outcome).revisions.empty());                  \
    } while (false)

// ---------------------------------------------------------------------------
// Payload builders
// ---------------------------------------------------------------------------

// One written form per logical type, so a golden case states the value and its type together
// and never relies on an implicit conversion to pick the storage form.
[[nodiscard]] inline Cell int_cell(std::int64_t value, LogicalType type = LogicalType::int64) {
    return Cell{type, data::IntValue{value}};
}

[[nodiscard]] inline Cell uint_cell(std::uint64_t value, LogicalType type = LogicalType::uint64) {
    return Cell{type, data::UIntValue{value}};
}

[[nodiscard]] inline Cell float32_cell(float value) {
    return Cell{LogicalType::float32, data::Float32Value{value}};
}

[[nodiscard]] inline Cell float64_cell(double value) {
    return Cell{LogicalType::float64, data::Float64Value{value}};
}

[[nodiscard]] inline Cell bool_cell(bool value) {
    return Cell{LogicalType::boolean, data::BoolValue{value}};
}

[[nodiscard]] inline Cell text_cell(std::string value) {
    return Cell{LogicalType::text, data::TextValue{std::move(value)}};
}

[[nodiscard]] inline Cell category_cell(std::int64_t code) {
    return Cell{LogicalType::categorical, data::CategoryValue{code}};
}

[[nodiscard]] inline Cell date_cell(std::int64_t days) {
    return Cell{LogicalType::date, data::DateValue{days}};
}

[[nodiscard]] inline Cell duration_cell(std::int64_t microseconds) {
    return Cell{LogicalType::duration, data::DurationValue{microseconds}};
}

[[nodiscard]] inline Cell naive_cell(std::int64_t microseconds) {
    return Cell{LogicalType::timestamp_naive, data::TimestampValue{microseconds}};
}

[[nodiscard]] inline Cell zoned_cell(std::int64_t microseconds) {
    return Cell{LogicalType::timestamp_zoned, data::TimestampValue{microseconds}};
}

[[nodiscard]] inline FieldSchema field(FieldId id, std::string name, LogicalType type,
                                       std::shared_ptr<const Dictionary> dictionary = nullptr,
                                       std::string unit_text = {},
                                       UnknownCategoryPolicy unknown_category =
                                           UnknownCategoryPolicy::reject,
                                       std::optional<std::string> zone_id = std::nullopt) {
    FieldSchema declared;
    declared.id = id;
    declared.name = std::move(name);
    declared.type = type;
    declared.dictionary = std::move(dictionary);
    declared.unknown_category = unknown_category;
    declared.zone_id = std::move(zone_id);
    if (!unit_text.empty()) {
        auto parsed = data::UnitRegistry::builtin().parse(unit_text);
        declared.unit = std::move(EG_REQUIRE_OK(parsed));
    }
    return declared;
}

[[nodiscard]] inline std::shared_ptr<const DatasetSchema> schema_of(std::vector<FieldSchema> fields) {
    auto created = DataStore::schema(std::move(fields));
    return EG_REQUIRE_OK(created);
}

[[nodiscard]] inline std::shared_ptr<const Dictionary> dictionary_of(
    std::vector<std::string> labels) {
    auto created = Dictionary::create(std::move(labels));
    return std::make_shared<const Dictionary>(EG_REQUIRE_OK(created));
}

// Appends one cell through the builder path its own logical type names, so a golden case
// stores the value the same way a producer would rather than through a bit reinterpretation.
[[nodiscard]] inline std::optional<Diagnostic> append_cell(ChunkBuilder& builder, const Cell& cell) {
    if (cell.is_missing()) {
        return builder.append_missing();
    }
    switch (cell.type()) {
        case LogicalType::int8:
        case LogicalType::int16:
        case LogicalType::int32:
        case LogicalType::int64:
            return builder.append_int(cell.int_value().value_or(0));
        case LogicalType::uint8:
        case LogicalType::uint16:
        case LogicalType::uint32:
        case LogicalType::uint64:
            return builder.append_uint(cell.uint_value().value_or(0));
        case LogicalType::float32:
            return builder.append_float32(cell.float32_value().value_or(0.0F));
        case LogicalType::float64:
            return builder.append_float64(cell.float64_value().value_or(0.0));
        case LogicalType::boolean:
            return builder.append_bool(cell.bool_value().value_or(false));
        case LogicalType::text:
            return builder.append_text(cell.text_value().value_or(std::string_view{}));
        case LogicalType::categorical:
            return builder.append_category_code(cell.category_code().value_or(0));
        case LogicalType::date:
        case LogicalType::duration:
        case LogicalType::timestamp_naive:
        case LogicalType::timestamp_zoned:
            return builder.append_ticks(cell.tick_value().value_or(0));
    }
    return builder.append_missing();
}

[[nodiscard]] inline std::shared_ptr<const Chunk> chunk_of(
    FieldId id, LogicalType type, ChunkId chunk_id, const std::vector<Cell>& cells,
    std::uint64_t first_row_index = 0, data::ChunkOptions options = {}) {
    auto builder = ChunkBuilder::create(id, type, options);
    for (const auto& cell : cells) {
        auto rejection = append_cell(EG_REQUIRE_OK(builder), cell);
        if (rejection.has_value()) {
            throw TestFailure("chunk build rejected a cell: " + rejection->to_error().message);
        }
    }
    auto finished = EG_REQUIRE_OK(builder).finish(chunk_id, first_row_index);
    return std::make_shared<const Chunk>(EG_REQUIRE_TAKEN(std::move(finished)));
}

[[nodiscard]] inline Cell chunk_cell(const Chunk& chunk, std::uint64_t index) {
    return EG_REQUIRE_TAKEN(chunk.cell_at(index));
}

[[nodiscard]] inline Provenance provenance_of(std::string operation,
                                              std::vector<DatasetVersionId> parents = {},
                                              std::vector<VersionInput> inputs = {},
                                              std::string parameters = "canonical") {
    Provenance record;
    record.operation_id = std::move(operation);
    record.operation_version = 1;
    record.canonical_parameters = std::move(parameters);
    record.parents = std::move(parents);
    record.inputs = std::move(inputs);
    record.unit_policy = "declared";
    return record;
}

[[nodiscard]] inline std::shared_ptr<const DatasetVersion> version_of(
    DatasetId dataset, DatasetVersionId id, RowDomainId domain,
    std::shared_ptr<const DatasetSchema> schema, std::vector<std::shared_ptr<const Chunk>> chunks,
    std::uint64_t row_count, Provenance provenance, Revision revision = 0) {
    auto created = DatasetVersion::create(id, dataset, std::move(schema), domain,
                                         std::move(chunks), std::move(provenance), row_count,
                                         revision);
    return EG_REQUIRE_OK(created);
}

[[nodiscard]] inline std::shared_ptr<const RowDomain> domain_of(RowDomainId id,
                                                                std::vector<RowId> rows) {
    auto created = RowDomain::create(id, std::move(rows));
    return std::make_shared<const RowDomain>(EG_REQUIRE_OK(created));
}

// Reads every cell of one field of a version, in the version's own row order.
[[nodiscard]] inline std::vector<Cell> version_cells(const DatasetVersion& version, FieldId field) {
    std::vector<Cell> cells;
    cells.reserve(version.row_count());
    for (std::uint64_t index = 0; index < version.row_count(); ++index) {
        cells.push_back(EG_REQUIRE_TAKEN(version.cell(field, index)));
    }
    return cells;
}

// ---------------------------------------------------------------------------
// The command fixture: one kernel, one store, one draft registry and the data
// command family, driven only through the executor.
// ---------------------------------------------------------------------------

class Fixture {
public:
    explicit Fixture(std::size_t max_undo_entries = 128) {
        auto registered = data::register_data_commands(registry_, store_, drafts_);
        if (registered.has_value()) {
            throw TestFailure("command registration failed: " + registered->to_error().message);
        }
        app::GatewayOptions options;
        options.max_undo_entries = max_undo_entries;
        gateway_.emplace(registry_, ValidatedDocument{}, options);
        executor_.emplace(*gateway_, store_);
    }

    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;

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

    [[nodiscard]] DocumentView document() {
        auto view = executor_->document();
        if (!view.ok()) {
            throw TestFailure("document view rejected: " + view.diagnostic().to_error().message);
        }
        return view.take();
    }

    [[nodiscard]] DatasetLease lease(DatasetVersionId id) {
        auto found = store_.lease(id);
        if (!found.ok()) {
            throw TestFailure("lease rejected: " + found.diagnostic().to_error().message);
        }
        return found.take();
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

    [[nodiscard]] std::shared_ptr<const RowCorrespondence> correspondence(CorrespondenceId id) {
        auto found = store_.correspondence(id);
        if (!found.ok()) {
            throw TestFailure("correspondence is not visible: " +
                              found.diagnostic().to_error().message);
        }
        return found.take();
    }

    [[nodiscard]] std::uint64_t undo_depth() const { return gateway_->undo_depth(); }
    [[nodiscard]] std::uint64_t redo_depth() const { return gateway_->redo_depth(); }
    [[nodiscard]] std::uint64_t version_count() const { return store_.version_count(); }
    [[nodiscard]] std::uint64_t staged_count() const { return store_.staged_count(); }

    DataStore& store() noexcept { return store_; }
    DraftRegistry& drafts() noexcept { return drafts_; }
    CommandGateway& gateway() noexcept { return *gateway_; }

    [[nodiscard]] DraftId stage(std::shared_ptr<const DatasetVersion> version) {
        return drafts_.add(std::move(version));
    }
    [[nodiscard]] DraftId stage(std::shared_ptr<const RowDomain> domain) {
        return drafts_.add(std::move(domain));
    }
    [[nodiscard]] DraftId stage(std::shared_ptr<const RowCorrespondence> correspondence) {
        return drafts_.add(std::move(correspondence));
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

    // The write set a command declares: one entry per object it will create or change, each at
    // the revision the caller expects, so no command can touch an object nobody named. A new
    // object is expected at the initial revision.
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
    std::uint64_t request_counter_{0};
};

// ---------------------------------------------------------------------------
// Worksheet scenarios
// ---------------------------------------------------------------------------

// One column of a scenario sheet: the identities this fixture allocated for it.
struct ColumnLine {
    std::uint64_t column{0};
    std::uint64_t dataset{0};
    std::uint64_t field{0};
    std::uint64_t version{0};
    bool same_domain{true};
};

// The identities of one scenario worksheet. Ids are allocated from a per-table base so a
// test that needs a foreign-domain source can build a second table without a collision.
struct SimpleTable {
    std::uint64_t project{0};
    std::uint64_t workbook{0};
    std::uint64_t worksheet{0};
    // The row domain the worksheet points at, and the next free domain identity. A row change
    // always produces a new domain identity, so the superseded one stays readable.
    std::uint64_t domain{0};
    std::uint64_t next_domain{0};
    std::uint64_t next_column{0};
    std::uint64_t next_dataset{0};
    std::uint64_t next_field{0};
    std::uint64_t next_chunk{0};
    std::uint64_t next_version{0};
    std::vector<RowId> rows;
    std::vector<ColumnLine> columns;
};

[[nodiscard]] inline SimpleTable make_table(std::uint64_t base) {
    SimpleTable table;
    table.project = base * 1000ULL + 1ULL;
    table.workbook = base * 1000ULL + 2ULL;
    table.worksheet = base * 1000ULL + 3ULL;
    table.domain = base * 1000ULL + 4ULL;
    table.next_domain = base * 1000ULL + 5ULL;
    table.next_column = base * 1000ULL + 100ULL;
    table.next_dataset = base * 1000ULL + 200ULL;
    table.next_field = base * 1000ULL + 300ULL;
    table.next_chunk = base * 1000ULL + 400ULL;
    table.next_version = base * 1000ULL + 500ULL;
    return table;
}

inline void create_project_chain(Fixture& fixture, SimpleTable& table,
                                 const std::string& label = "worksheet") {
    ParameterMap project = Fixture::with(data::argument::kProject, table.project);
    Fixture::put(project, data::argument::kLabel, std::string{"project"});
    (void)fixture.require_run(data::command_id::kProjectCreate, std::move(project),
                          fixture.declared_writes({table.project}));

    ParameterMap workbook = Fixture::with(data::argument::kWorkbook, table.workbook);
    Fixture::put(workbook, data::argument::kProject, table.project);
    Fixture::put(workbook, data::argument::kLabel, std::string{"workbook"});
    (void)fixture.require_run(data::command_id::kWorkbookCreate, std::move(workbook),
                          fixture.declared_writes({table.workbook}));

    ParameterMap worksheet = Fixture::with(data::argument::kWorksheet, table.worksheet);
    Fixture::put(worksheet, data::argument::kWorkbook, table.workbook);
    Fixture::put(worksheet, data::argument::kDomain, table.domain);
    Fixture::put(worksheet, data::argument::kLabel, label);
    (void)fixture.require_run(data::command_id::kWorksheetCreate, std::move(worksheet),
                          fixture.declared_writes({table.worksheet, table.domain}));
}

inline void create_worksheet(Fixture& fixture, SimpleTable& table, std::uint64_t worksheet,
                             std::uint64_t domain, const std::string& label) {
    ParameterMap arguments = Fixture::with(data::argument::kWorksheet, worksheet);
    Fixture::put(arguments, data::argument::kWorkbook, table.workbook);
    Fixture::put(arguments, data::argument::kDomain, domain);
    Fixture::put(arguments, data::argument::kLabel, label);
    (void)fixture.require_run(data::command_id::kWorksheetCreate, std::move(arguments),
                              fixture.declared_writes({worksheet, domain}));
}

struct ColumnSpec {
    LogicalType type{LogicalType::float64};
    std::vector<Cell> cells;
    std::string label{"value"};
    std::string unit;
    std::string role{"unspecified"};
    std::string display_format;
    std::uint64_t order_index{0};
    std::shared_ptr<const Dictionary> dictionary;
    UnknownCategoryPolicy unknown_category{UnknownCategoryPolicy::reject};
    // The zone a zoned timestamp field declares. No other type accepts one.
    std::optional<std::string> zone;
    // The row domain the candidate version is ordered by. Empty means the worksheet's own
    // domain, which is what a same-domain column uses.
    std::optional<std::uint64_t> foreign_domain;
};

// Builds the schema, chunk and candidate version, stages them as private output, then asks
// the command to publish. Nothing is visible until the executor commits.
[[nodiscard]] inline ColumnLine create_column(Fixture& fixture, SimpleTable& table,
                                              const ColumnSpec& spec) {
    ColumnLine line;
    line.column = table.next_column++;
    line.dataset = table.next_dataset++;
    line.field = table.next_field++;
    line.version = table.next_version++;
    const ChunkId chunk_id{ObjectId{table.next_chunk++}};
    line.same_domain = !spec.foreign_domain.has_value();

    const RowDomainId domain = spec.foreign_domain.has_value()
                                   ? data::row_domain_id(*spec.foreign_domain)
                                   : data::row_domain_id(table.domain);
    auto schema = schema_of(
        {field(data::field_id(line.field), spec.label, spec.type, spec.dictionary, spec.unit,
               spec.unknown_category, spec.zone)});
    auto chunk = chunk_of(data::field_id(line.field), spec.type, chunk_id, spec.cells);
    auto version = version_of(data::dataset_id(line.dataset), data::dataset_version_id(line.version),
                              domain, schema, {chunk}, spec.cells.size(),
                              provenance_of(data::command_id::kColumnCreate));
    const DraftId draft = fixture.stage(version);

    ParameterMap arguments = Fixture::with(data::argument::kColumn, line.column);
    Fixture::put(arguments, data::argument::kWorksheet, table.worksheet);
    Fixture::put(arguments, data::argument::kDataset, line.dataset);
    Fixture::put(arguments, data::argument::kVersionDraft, draft);
    Fixture::put(arguments, data::argument::kLabel, spec.label);
    Fixture::put(arguments, data::argument::kRole, spec.role);
    Fixture::put(arguments, data::argument::kOrderIndex, spec.order_index);
    if (!spec.unit.empty()) {
        Fixture::put(arguments, data::argument::kUnit, spec.unit);
    }
    if (!spec.display_format.empty()) {
        Fixture::put(arguments, data::argument::kDisplayFormat, spec.display_format);
    }
    (void)fixture.require_run(data::command_id::kColumnCreate, std::move(arguments),
                          fixture.declared_writes({line.column, line.version}));
    table.columns.push_back(line);
    return line;
}

// What each same-domain column's successor version holds: the cells of the kept current
// rows in order, then missing slots for rows the column has no value for yet.
struct SuccessorPlan {
    std::vector<std::size_t> kept;
    std::size_t appended_missing{0};
};

// Moves the worksheet onto a revised row domain of exactly these identities and publishes a
// successor version for every same-domain column in the same revision. The revised domain is
// a new identity, so the superseded one stays readable.
inline void change_rows(Fixture& fixture, SimpleTable& table, const std::vector<RowId>& revised,
                        const SuccessorPlan& plan, const std::string& operation) {
    const RowDomainId revised_id = data::row_domain_id(table.next_domain++);
    auto domain = domain_of(revised_id, revised);
    const DraftId domain_draft = fixture.stage(domain);

    std::vector<std::uint64_t> columns;
    std::vector<std::uint64_t> drafts;
    std::vector<std::pair<std::size_t, std::uint64_t>> successors;
    for (std::size_t position = 0; position < table.columns.size(); ++position) {
        const ColumnLine line = table.columns[position];
        if (!line.same_domain) {
            continue;
        }
        const FieldId column_field = data::field_id(line.field);
        auto current = fixture.version(data::dataset_version_id(line.version));
        const FieldSchema previous_field = *current->schema()->find(column_field);
        std::vector<Cell> cells = version_cells(*current, column_field);
        std::vector<Cell> successor_cells;
        successor_cells.reserve(plan.kept.size() + plan.appended_missing);
        for (const std::size_t index : plan.kept) {
            successor_cells.push_back(cells[static_cast<std::size_t>(index)]);
        }
        for (std::size_t index = 0; index < plan.appended_missing; ++index) {
            successor_cells.push_back(Cell::missing(previous_field.type));
        }
        const std::uint64_t successor_id = table.next_version++;
        auto chunk = chunk_of(column_field, previous_field.type,
                              ChunkId{ObjectId{table.next_chunk++}}, successor_cells);
        auto successor = version_of(
            data::dataset_id(line.dataset), data::dataset_version_id(successor_id), revised_id,
            schema_of({previous_field}), {chunk}, successor_cells.size(),
            provenance_of(data::command_id::kWorksheetChangeRows, {current->id()}));
        drafts.push_back(fixture.stage(successor));
        columns.push_back(line.column);
        successors.emplace_back(position, successor_id);
    }

    ParameterMap arguments = Fixture::with(data::argument::kWorksheet, table.worksheet);
    Fixture::put(arguments, data::argument::kDomainDraft, domain_draft);
    Fixture::put(arguments, data::argument::kOperation, operation);
    Fixture::put(arguments, data::argument::kColumns, Fixture::id_list(columns));
    Fixture::put(arguments, data::argument::kVersionDrafts, Fixture::id_list(drafts));
    std::vector<std::uint64_t> writes{table.worksheet, revised_id.raw()};
    writes.insert(writes.end(), columns.begin(), columns.end());
    for (const auto& entry : successors) {
        writes.push_back(entry.second);
    }
    (void)fixture.require_run(data::command_id::kWorksheetChangeRows, std::move(arguments),
                              fixture.declared_writes(writes));

    for (const auto& entry : successors) {
        table.columns[entry.first].version = entry.second;
    }
    table.rows = revised;
    table.domain = revised_id.raw();
}

inline void append_rows(Fixture& fixture, SimpleTable& table, const std::vector<RowId>& new_rows) {
    SuccessorPlan plan;
    for (std::size_t index = 0; index < table.rows.size(); ++index) {
        plan.kept.push_back(index);
    }
    plan.appended_missing = new_rows.size();
    std::vector<RowId> revised = table.rows;
    revised.insert(revised.end(), new_rows.begin(), new_rows.end());
    change_rows(fixture, table, revised, plan, "append");
}

inline void delete_rows(Fixture& fixture, SimpleTable& table, const std::vector<RowId>& removed) {
    SuccessorPlan plan;
    std::vector<RowId> revised;
    for (std::size_t index = 0; index < table.rows.size(); ++index) {
        if (std::find(removed.begin(), removed.end(), table.rows[index]) == removed.end()) {
            plan.kept.push_back(index);
            revised.push_back(table.rows[index]);
        }
    }
    change_rows(fixture, table, revised, plan, "delete");
}

// Publishes an edited version of one column, keeping its row domain and row count, and moves
// the caller's line and the table's record of it onto the successor version.
inline void edit_column(Fixture& fixture, SimpleTable& table, ColumnLine& line,
                        const std::vector<Cell>& cells) {
    auto current = fixture.version(data::dataset_version_id(line.version));
    const FieldId column_field = data::field_id(line.field);
    const FieldSchema previous_field = *current->schema()->find(column_field);
    const std::uint64_t successor_id = table.next_version++;
    auto chunk = chunk_of(column_field, previous_field.type,
                          ChunkId{ObjectId{table.next_chunk++}}, cells);
    auto successor = version_of(
        data::dataset_id(line.dataset), data::dataset_version_id(successor_id),
        current->row_domain(), schema_of({previous_field}), {chunk}, cells.size(),
        provenance_of(data::command_id::kColumnEdit, {current->id()}));

    ParameterMap arguments = Fixture::with(data::argument::kColumn, line.column);
    Fixture::put(arguments, data::argument::kVersionDraft, fixture.stage(successor));
    (void)fixture.require_run(data::command_id::kColumnEdit, std::move(arguments),
                          fixture.declared_writes({line.column, successor_id}));
    for (auto& stored : table.columns) {
        if (stored.column == line.column) {
            stored.version = successor_id;
        }
    }
    line.version = successor_id;
}

}  // namespace egtest
