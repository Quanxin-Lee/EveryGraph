#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/command_registry.hpp"
#include "everygraph/data/cell.hpp"
#include "everygraph/data/commands.hpp"
#include "everygraph/data/dataset.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/logical_type.hpp"

namespace everygraph::data {

// Command names follow the `namespace.verb` convention of the application contract. The
// create-and-publish family is in `everygraph/data/commands.hpp`; the names here are the
// editing writes that build their own successor domain, chunks and versions.
namespace command_id {
inline constexpr const char* kWorksheetRowInsert = "data.worksheet_row_insert";
inline constexpr const char* kWorksheetRowDelete = "data.worksheet_row_delete";
inline constexpr const char* kWorksheetRowMove = "data.worksheet_row_move";
inline constexpr const char* kWorksheetResize = "data.worksheet_resize";
inline constexpr const char* kWorksheetColumnInsert = "data.worksheet_column_insert";
inline constexpr const char* kWorksheetColumnOrderSet = "data.worksheet_column_order_set";
inline constexpr const char* kWorksheetCellSet = "data.worksheet_cell_set";
inline constexpr const char* kWorksheetCellPaste = "data.worksheet_cell_paste";
inline constexpr const char* kWorksheetColumnTypePreview = "data.worksheet_column_type_preview";
inline constexpr const char* kWorksheetColumnTypeSet = "data.worksheet_column_type_set";
}  // namespace command_id

namespace argument {
inline constexpr const char* kRowIndex = "row_index";
inline constexpr const char* kTargetRowIndex = "target_row_index";
inline constexpr const char* kRow = "row_id";
inline constexpr const char* kLength = "length";
inline constexpr const char* kTargetRowCount = "target_row_count";
inline constexpr const char* kRevisedDomain = "revised_row_domain_id";
inline constexpr const char* kSuccessorVersions = "successor_version_ids";
inline constexpr const char* kSuccessorChunks = "successor_chunk_ids";
inline constexpr const char* kValue = "value";
inline constexpr const char* kValues = "values";
inline constexpr const char* kTargetType = "target_type";
inline constexpr const char* kConversion = "conversion";
inline constexpr const char* kDictionary = "dictionary_labels";
inline constexpr const char* kZone = "zone_id";
inline constexpr const char* kSourceUnit = "source_unit";
}  // namespace argument

// A typed value travels through the kernel's parameter map, which holds scalars, so a cell and
// a block of cells are declared as text. The form is self-delimiting: one kind character, the
// decimal byte length of the payload, a colon and exactly that many bytes. Nothing inside a
// payload needs an escape, which is what lets a category label or a text value carry a comma,
// a semicolon or a digit.
//
//   `i` signed ticks or integer    `u` unsigned integer       `f` decimal floating point
//   `b` exact stored bits         `n` missing (length zero)  `t` text code points
//   `c` category label            `q` category code
//
// The kind names the *storage form*, and the target field's declared logical type decides
// whether that form is accepted: `i` fills a signed integer or a temporal tick, `u` an unsigned
// integer, `f` a float of either width, `b` any fixed-width type through the bit round-trip,
// `c` and `q` only a categorical field, `t` only a text field and `n` a nullable field.
struct ValueForm {
    char kind{'n'};
    std::string payload;

    [[nodiscard]] std::string encode() const;

    friend bool operator==(const ValueForm&, const ValueForm&) noexcept = default;
};

[[nodiscard]] ValueForm missing_form();
[[nodiscard]] ValueForm signed_form(std::int64_t value);
[[nodiscard]] ValueForm unsigned_form(std::uint64_t value);
[[nodiscard]] ValueForm float_form(double value);
[[nodiscard]] ValueForm bits_form(std::uint64_t bits);
[[nodiscard]] ValueForm text_form(std::string value);
[[nodiscard]] ValueForm category_form(std::string label);
[[nodiscard]] ValueForm code_form(std::int64_t code);

// One cell, written in its own form.
[[nodiscard]] std::string encode_value(const ValueForm& value);

// A row-major block: `<rows>x<cols>;<cells>` with `rows * cols` self-delimiting forms
// concatenated. A zero row or column count is a declaration error, not an empty block.
[[nodiscard]] std::string encode_grid(std::uint64_t rows, std::uint64_t columns,
                                      std::vector<ValueForm> values);

[[nodiscard]] Result<ValueForm> parse_value(std::string_view text);
[[nodiscard]] Result<std::vector<ValueForm>> parse_grid(std::string_view text);

// The declared target type of a column type change, spelled the way `to_string(LogicalType)`
// reports it. A name the type table does not have is rejected rather than guessed, so a caller
// can never ask for a type this module stores under a different one.
[[nodiscard]] Result<LogicalType> logical_type_from_text(std::string_view text);

// How a column type change treats a value the target type cannot hold exactly. There is no
// default that quietly narrows or demotes: `reject` keeps only value-preserving moves, and a
// fraction or an inexact float64-to-float32 needs the caller to name `round`. An out-of-range
// value has no policy that accepts it, so no declared route makes it a wrap.
enum class ConversionPolicy {
    reject,
    round,
};

[[nodiscard]] const char* to_string(ConversionPolicy policy) noexcept;
[[nodiscard]] Result<ConversionPolicy> conversion_policy_from_text(std::string_view text);

// Registers the editing family on the same registry and store as the create-and-publish family.
// Every command declares its effect class, schema version, argument shape and repeatability, so
// an editor, a table view, a script and an AI client all reach the store through this one path.
//
// Unlike the create-and-publish family, these commands build their own successor row domain,
// chunks and versions inside the transaction, so the caller names only the *identities* those
// successors will hold: `revised_row_domain_id`, then one `successor_version_ids` and one
// `successor_chunk_ids` entry per edited column, in the worksheet's display order. The only
// draft this family consumes is the candidate version a new column publishes. A command that
// computes no change writes nothing at all, so a repeat of an idempotent edit is a no-op rather
// than a clash with the identities it names.
[[nodiscard]] std::optional<Diagnostic> register_worksheet_edit_commands(
    application::CommandRegistry& registry, DataStore& store, const DraftRegistry& drafts);

}  // namespace everygraph::data
