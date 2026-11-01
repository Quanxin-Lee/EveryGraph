#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/data/diagnostics.hpp"

namespace everygraph::data {

constexpr std::int64_t kMicrosPerSecond = 1000000;
constexpr std::int64_t kSecondsPerDay = 86400;
constexpr std::int64_t kMicrosPerDay = kMicrosPerSecond * kSecondsPerDay;

struct CivilDate {
    std::int32_t year{1970};
    std::uint32_t month{1};
    std::uint32_t day{1};
};

struct CivilTime {
    std::uint32_t hour{0};
    std::uint32_t minute{0};
    std::uint32_t second{0};
    std::uint32_t microsecond{0};
};

struct CivilDateTime {
    CivilDate date;
    CivilTime time;
};

struct LocalObservation {
    CivilDateTime local;
    std::int64_t offset_micros{0};
    bool in_daylight_time{false};
};

struct ZonedInstant {
    std::int64_t instant_micros{0};
    std::int64_t offset_micros{0};
    std::string zone_id;
    bool in_daylight_time{false};
};

// How a caller resolves a local time that a zone does not name uniquely. `pre_transition`
// and `post_transition` name the UTC offset in effect before, respectively after, the
// transition that creates the gap or the overlap; they are not a claim about which
// resulting instant is earlier. A rejected local time never becomes an instant.
enum class LocalTimeResolution {
    reject,
    pre_transition,
    post_transition,
};

// A named-zone rule set. The built-in instance is versioned, holds the zones listed by
// `zone_ids()` and uses non-negative daylight savings only, which is what makes the
// pre/post offset rule above decidable. It is a declared rule table, not a copy of a
// pinned IANA distribution: the production timezone database source stays an open
// decision recorded in the owning task.
class TimeZoneRules {
public:
    struct RuleTime {
        std::uint32_t month{1};
        std::int8_t week{1};  // 1..5, or -1 for the last occurrence in the month
        std::uint32_t day_of_week{0};
        std::uint32_t hour{0};
        std::uint32_t minute{0};
        bool in_daylight_time{false};
    };

    struct Definition {
        std::string id;
        std::int64_t standard_offset_micros{0};
        std::int64_t savings_micros{0};
        std::int32_t dst_from_year{9999};
        bool southern{false};
        std::optional<RuleTime> dst_start;
        std::optional<RuleTime> dst_end;

        [[nodiscard]] bool observes_daylight_time() const noexcept {
            return dst_start.has_value() && dst_end.has_value() && savings_micros > 0;
        }
    };

    struct Transition {
        std::int64_t instant_micros{0};
        std::int64_t offset_before_micros{0};
        std::int64_t offset_after_micros{0};
    };

    [[nodiscard]] static const TimeZoneRules& builtin();

    [[nodiscard]] const std::string& version() const noexcept { return version_; }
    [[nodiscard]] bool contains(std::string_view zone_id) const;
    [[nodiscard]] std::vector<std::string> zone_ids() const;
    [[nodiscard]] std::optional<std::int64_t> standard_offset(std::string_view zone_id) const noexcept;

    [[nodiscard]] Result<LocalObservation> observe(std::int64_t instant_micros,
                                                   std::string_view zone_id) const;
    [[nodiscard]] Result<ZonedInstant> resolve(CivilDateTime local, std::string_view zone_id,
                                               LocalTimeResolution resolution =
                                                   LocalTimeResolution::reject) const;

private:
    TimeZoneRules(std::string version, std::vector<Definition> definitions);

    [[nodiscard]] const Definition* find(std::string_view zone_id) const noexcept;
    [[nodiscard]] std::vector<Transition> transitions_around(const Definition& definition,
                                                             std::int32_t year) const;
    [[nodiscard]] std::int64_t offset_at(const Definition& definition,
                                         std::int64_t instant_micros) const;

    std::string version_;
    std::vector<Definition> definitions_;
};

// Civil calendar arithmetic on the proleptic Gregorian calendar. A date-only value stays a
// day count and is never widened into an instant.
[[nodiscard]] Result<std::int64_t> ordinal_days(CivilDate date) noexcept;
[[nodiscard]] Result<CivilDate> date_from_ordinal(std::int64_t days) noexcept;
[[nodiscard]] bool is_leap_year(std::int32_t year) noexcept;
[[nodiscard]] Result<std::uint32_t> days_in_month(std::int32_t year, std::uint32_t month) noexcept;

// Local civil time to microseconds since the epoch without a zone. This is the naive
// timestamp form; it is distinct from a zoned instant.
[[nodiscard]] Result<std::int64_t> naive_micros(CivilDateTime local) noexcept;
[[nodiscard]] Result<CivilDateTime> civil_from_naive(std::int64_t micros) noexcept;
[[nodiscard]] Result<CivilDateTime> civil_from_instant(std::int64_t instant_micros,
                                                       std::int64_t offset_micros) noexcept;

}  // namespace everygraph::data
