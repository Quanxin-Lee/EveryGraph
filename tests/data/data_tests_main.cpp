#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "data_test_support.hpp"

#ifndef EG_FIXTURE_MANIFEST
#define EG_FIXTURE_MANIFEST ""
#endif

namespace {

// The fixture-index gate the conformance plan asks the harness to hold: read this task's own
// fragment, reject a duplicated fixture id, and require every executed case to name a fixture
// that the fragment declares and vice versa. A missing fragment is a failure, not a skip.
[[nodiscard]] std::size_t report_manifest_problems(const std::vector<std::string>& case_names) {
    const std::string path{EG_FIXTURE_MANIFEST};
    if (path.empty()) {
        std::cerr << "[FAIL] fixture manifest path was not compiled in\n";
        return 1;
    }
    std::ifstream stream{path};
    if (!stream) {
        std::cerr << "[FAIL] fixture manifest fragment is unreadable: " << path << '\n';
        return 1;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    const std::string text = buffer.str();

    const std::regex id_pattern{"\"fixture_id\"\\s*:\\s*\"(EG-FX-[A-Z]+-[0-9]{3})\""};
    std::set<std::string> declared;
    std::size_t duplicates = 0;
    for (std::sregex_iterator iterator{text.begin(), text.end(), id_pattern};
         iterator != std::sregex_iterator(); ++iterator) {
        const std::string id = (*iterator)[1].str();
        if (!declared.insert(id).second) {
            ++duplicates;
            std::cerr << "[FAIL] fixture id is declared twice in the fragment: " << id << '\n';
        }
    }
    if (declared.empty()) {
        std::cerr << "[FAIL] fixture manifest fragment declares no fixture\n";
        return duplicates + 1;
    }

    std::set<std::string> executed;
    const std::regex case_pattern{"EG-FX-[A-Z]+-[0-9]{3}"};
    std::size_t untracked = 0;
    for (const auto& name : case_names) {
        std::smatch match;
        if (!std::regex_search(name, match, case_pattern)) {
            std::cerr << "[FAIL] a case names no fixture id: " << name << '\n';
            ++untracked;
            continue;
        }
        executed.insert(match.str());
    }
    for (const auto& id : declared) {
        if (!executed.contains(id)) {
            std::cerr << "[FAIL] the fragment declares a fixture no case exercised: " << id << '\n';
            ++untracked;
        }
    }
    for (const auto& id : executed) {
        if (!declared.contains(id)) {
            std::cerr << "[FAIL] a case exercised a fixture the fragment does not declare: " << id
                      << '\n';
            ++untracked;
        }
    }
    std::cout << "fixture fragment: " << declared.size() << " fixtures, " << executed.size()
              << " exercised\n";
    return duplicates + untracked;
}

}  // namespace

int main() {
    const auto& cases = egtest::test_cases();
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
              << " data-model tests passed\n";
    return failures == 0 && manifest_problems == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
