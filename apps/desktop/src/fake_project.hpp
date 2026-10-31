#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::desktop {

namespace fake_commands {

inline constexpr std::string_view kCreateObject = "project.create_object";
inline constexpr std::string_view kMoveObject = "project.move_object";
inline constexpr std::string_view kFindObject = "project.find_object";

}  // namespace fake_commands

namespace fake_ids {

inline constexpr foundation::ObjectId kRootFolder{1};
inline constexpr foundation::ObjectId kExperimentsFolder{2};
inline constexpr foundation::ObjectId kScratchFolder{3};
inline constexpr foundation::ObjectId kSpectrumDataset{4};
inline constexpr foundation::ObjectId kSpectrumGraph{5};
inline constexpr foundation::ObjectId kReadmeNote{6};

}  // namespace fake_ids

[[nodiscard]] application::DocumentSnapshot make_fake_document();
[[nodiscard]] application::CommandRegistry make_fake_registry();

// Owns the only kernel instance of the prototype. The gateway takes its
// registry and seed snapshot by value, so the fake project cannot be copied.
class FakeProject {
public:
    FakeProject();
    FakeProject(const FakeProject&) = delete;
    FakeProject& operator=(const FakeProject&) = delete;

    [[nodiscard]] application::CommandGateway& gateway() noexcept;
    [[nodiscard]] const application::CommandGateway& gateway() const noexcept;

private:
    application::CommandGateway gateway_;
};

using FakeCommandDispatch = std::function<application::CommandOutcome(
    std::string_view command_id, application::ParameterMap arguments,
    application::ExpectedRevisions expected)>;

// Turns UI intent into registered-command requests with explicit expected
// revisions. The dispatch callback is the shell's only command boundary.
class FakeProjectController {
public:
    FakeProjectController(application::CommandGateway& gateway, FakeCommandDispatch dispatch);

    [[nodiscard]] application::CommandOutcome create_folder(foundation::ObjectId parent);
    [[nodiscard]] application::CommandOutcome create_dataset(foundation::ObjectId parent);
    [[nodiscard]] application::CommandOutcome create_object(foundation::ObjectId parent,
                                                            std::string_view type);
    [[nodiscard]] application::CommandOutcome move_object(foundation::ObjectId object);
    [[nodiscard]] application::CommandOutcome find_object(foundation::ObjectId object);

private:
    [[nodiscard]] std::string next_name(std::string_view type,
                                        foundation::ObjectId id) const;

    application::CommandGateway& gateway_;
    FakeCommandDispatch dispatch_;
    std::uint64_t next_object_id_{1};
};

}  // namespace everygraph::desktop
