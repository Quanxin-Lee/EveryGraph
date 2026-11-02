#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "edit_fixture.hpp"

#ifndef EG_FIXTURE_MANIFEST
#define EG_FIXTURE_MANIFEST ""
#endif

// The other task fragments the harness cross-checks against. This task mints fixture ids the
// conformance plan's tables do not list, so it proves its own numbering claims no id another
// fragment already declares, which is what the plan asks of a fragment reader.
#ifndef EG_FIXTURE_OTHER_MANIFESTS
#define EG_FIXTURE_OTHER_MANIFESTS ""
#endif

namespace {

using egtest::test_cases;

struct Fragment {
    std::string path;
    std::set<std::string> ids;
    std::size_t duplicates{0};
};

[[nodiscard]] std::vector<std::string> split_paths(const std::string& text) {
    std::vector<std::string> paths;
    std::string entry;
    std::istringstream stream{text};
    while (std::getline(stream, entry, ';')) {
        if (!entry.empty()) {
            paths.push_back(entry);
        }
    }
    return paths;
}

[[nodiscard]] bool read_fragment(Fragment& fragment) {
    std::ifstream stream{fragment.path};
    if (!stream) {
        std::cerr << "[FAIL] fixture manifest fragment is unreadable: " << fragment.path << '\n';
        return false;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    const std::string text = buffer.str();
    const std::regex id_pattern{"\"fixture_id\"\\s*:\\s*\"(EG-FX-[A-Z]+-[0-9]{3})\""};
    for (std::sregex_iterator iterator{text.begin(), text.end(), id_pattern};
         iterator != std::sregex_iterator(); ++iterator) {
        const std::string id = (*iterator)[1].str();
        if (!fragment.ids.insert(id).second) {
            ++fragment.duplicates;
            std::cerr << "[FAIL] fixture id is declared twice in one fragment: " << id << " in "
                      << fragment.path << '\n';
        }
    }
    return true;
}

// The fixture-index gate the conformance plan asks the harness to hold: every executed case names
// a fixture this fragment declares and every declared fixture is exercised, no id is declared
// twice, and no id this task minted collides with another task's fragment. A missing fragment is
// a failure, not a skip.
[[nodiscard]] std::size_t report_manifest_problems(const std::vector<std::string>& case_names) {
    Fragment own{std::string{EG_FIXTURE_MANIFEST}, {}, 0};
    if (own.path.empty()) {
        std::cerr << "[FAIL] fixture manifest path was not compiled in\n";
        return 1;
    }
    if (!read_fragment(own)) {
        return 1;
    }
    if (own.ids.empty()) {
        std::cerr << "[FAIL] fixture manifest fragment declares no fixture\n";
        return own.duplicates + 1;
    }

    std::size_t problems = own.duplicates;
    for (const std::string& path : split_paths(std::string{EG_FIXTURE_OTHER_MANIFESTS})) {
        Fragment other{path, {}, 0};
        if (!read_fragment(other)) {
            ++problems;
            continue;
        }
        for (const auto& id : own.ids) {
            if (other.ids.contains(id)) {
                ++problems;
                std::cerr << "[FAIL] fixture id is declared by this fragment and by " << path
                          << ": " << id << '\n';
            }
        }
    }

    std::set<std::string> executed;
    const std::regex case_pattern{"EG-FX-[A-Z]+-[0-9]{3}"};
    for (const auto& name : case_names) {
        std::smatch match;
        if (!std::regex_search(name, match, case_pattern)) {
            std::cerr << "[FAIL] a case names no fixture id: " << name << '\n';
            ++problems;
            continue;
        }
        executed.insert(match.str());
    }
    for (const auto& id : own.ids) {
        if (!executed.contains(id)) {
            std::cerr << "[FAIL] the fragment declares a fixture no case exercised: " << id << '\n';
            ++problems;
        }
    }
    for (const auto& id : executed) {
        if (!own.ids.contains(id)) {
            std::cerr << "[FAIL] a case exercised a fixture the fragment does not declare: " << id
                      << '\n';
            ++problems;
        }
    }
    std::cout << "fixture fragment: " << own.ids.size() << " fixtures, " << executed.size()
              << " exercised, " << split_paths(std::string{EG_FIXTURE_OTHER_MANIFESTS}).size()
              << " sibling fragments cross-checked\n";
    return problems;
}

}  // namespace

int main() {
    const auto& cases = test_cases();
    std::size_t failures = 0;
    std::vector<std::string> names;
    names.reserve(cases.size());
    for (const auto& test_case : cases) {
        names.push_back(test_case.name);
        try {
            test_case.function();
            std::cout << "[PASS] " << test_case.name << '\n';
        } catch (const std::exception& exception) {
            ++failures;
            std::cerr << "[FAIL] " << test_case.name << ": " << exception.what() << '\n';
        } catch (...) {
            std::cerr << "[FAIL] " << test_case.name << ": unknown exception\n";
            ++failures;
        }
    }
    const std::size_t manifest_problems = report_manifest_problems(names);
    std::cout << (cases.size() - failures) << '/' << cases.size()
              << " worksheet-edit command tests passed\n";
    return failures == 0 && manifest_problems == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
