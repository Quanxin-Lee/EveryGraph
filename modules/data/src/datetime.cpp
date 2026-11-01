#include "everygraph/data/datetime.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace everygraph::data {
namespace {

constexpr std::int32_t kMinYear = -9999;
constexpr std::int32_t kMaxYear = 9999;

using Definition = TimeZoneRules::Definition;
using RuleTime = TimeZoneRules::RuleTime;
using Transition = TimeZoneRules::Transition;

constexpr std::int64_t seconds_to_micros(std::int64_t seconds) noexcept {
    return seconds * kMicrosPerSecond;
}

// Floor division by a positive divisor, so a pre-epoch instant names the day its UTC clock
// reads rather than the one truncation toward zero gives. Written without negating the
// dividend, because the most negative int64 has no positive counterpart.
constexpr std::int64_t floor_div(std::int64_t value, std::int64_t divisor) noexcept {
    const std::int64_t truncated = value / divisor;
    const std::int64_t remainder = value - truncated * divisor;
    if (value < 0 && remainder != 0) {
        return truncated - 1;
    }
    return truncated;
}

// Proleptic Gregorian civil-to-ordinal conversion, valid across the supported year range
// and for dates before the epoch.
std::int64_t days_from_civil(std::int32_t year, std::uint32_t month, std::uint32_t day) noexcept {
    std::int32_t adjusted = year;
    adjusted -= static_cast<std::int32_t>(month <= 2U);
    const std::int32_t era = (adjusted >= 0 ? adjusted : adjusted - 399) / 400;
    const auto year_of_era = static_cast<std::uint32_t>(adjusted - era * 400);
    const auto month_shift = static_cast<std::uint32_t>(month > 2U ? month - 3U : month + 9U);
    const auto day_of_year = (153U * month_shift + 2U) / 5U + day - 1U;
    const auto day_of_era =
        year_of_era * 365U + year_of_era / 4U - year_of_era / 100U + day_of_year;
    return static_cast<std::int64_t>(era) * 146097LL + static_cast<std::int64_t>(day_of_era) -
           719468LL;
}

void civil_from_days(std::int64_t days, CivilDate& out) noexcept {
    const std::int64_t shifted = days + 719468LL;
    const std::int64_t era = (shifted >= 0 ? shifted : shifted - 146096LL) / 146097LL;
    const auto day_of_era = static_cast<std::uint32_t>(shifted - era * 146097LL);
    const auto year_of_era =
        (day_of_era - day_of_era / 1460U + day_of_era / 36524U - day_of_era / 146096U) / 365U;
    auto year = static_cast<std::int32_t>(static_cast<std::int64_t>(year_of_era) + era * 400LL);
    const auto day_of_year =
        day_of_era - (365U * year_of_era + year_of_era / 4U - year_of_era / 100U);
    const auto month_shift = (5U * day_of_year + 2U) / 153U;
    const auto day = day_of_year - (153U * month_shift + 2U) / 5U + 1U;
    const auto month = month_shift + (month_shift < 10U ? 3U : -9U);
    year += static_cast<std::int32_t>(month <= 2U);
    out.year = year;
    out.month = month;
    out.day = day;
}

// Weekday index with 0 = Sunday, so a rule reads the same way its source notation does.
std::uint32_t weekday_of_ordinal(std::int64_t days) noexcept {
    const std::int64_t modulo = (days + 4LL) % 7LL;
    return static_cast<std::uint32_t>((modulo + 7LL) % 7LL);
}

Diagnostic date_diagnostic(std::string message) {
    return Diagnostic::reject(DetailCode::date_format, std::move(message));
}

// The nth weekday of a month as a rule names it, or nothing when that occurrence does not
// exist. A missing "5th Monday" is not clamped onto the last day of the month: the caller
// skips the transition instead of inventing a date the rule never declared.
std::optional<CivilDate> nth_weekday(std::int32_t year, const RuleTime& rule) {
    const auto month_length = days_in_month(year, rule.month);
    if (!month_length.ok()) {
        return std::nullopt;
    }
    const std::uint32_t length = month_length.value();
    const std::uint32_t first_weekday = weekday_of_ordinal(days_from_civil(year, rule.month, 1));
    const std::uint32_t first_matching = 1U + (rule.day_of_week + 7U - first_weekday) % 7U;
    std::uint32_t day = first_matching;
    if (rule.week > 0) {
        day = first_matching + static_cast<std::uint32_t>(rule.week - 1) * 7U;
        if (day > length) {
            return std::nullopt;
        }
    } else {
        day = first_matching + ((length - first_matching) / 7U) * 7U;
    }
    return CivilDate{year, rule.month, day};
}

std::optional<std::int64_t> transition_instant(const Definition& definition, const RuleTime& rule,
                                               std::int32_t year) {
    const auto date = nth_weekday(year, rule);
    if (!date.has_value()) {
        return std::nullopt;
    }
    const std::int64_t offset =
        rule.in_daylight_time ? definition.standard_offset_micros + definition.savings_micros
                              : definition.standard_offset_micros;
    return days_from_civil(date->year, date->month, date->day) * kMicrosPerDay +
           static_cast<std::int64_t>(rule.hour) * 3600LL * kMicrosPerSecond +
           static_cast<std::int64_t>(rule.minute) * 60LL * kMicrosPerSecond - offset;
}

std::int64_t hours_micros(std::int32_t hours) noexcept {
    return seconds_to_micros(static_cast<std::int64_t>(hours) * 3600LL);
}

std::int64_t minutes_micros(std::int32_t minutes) noexcept {
    return seconds_to_micros(static_cast<std::int64_t>(minutes) * 60LL);
}

Definition make_definition(std::string id, std::int64_t standard_offset_micros,
                           std::int64_t savings_micros, std::int32_t dst_from_year, bool southern,
                           const std::optional<RuleTime>& dst_start,
                           const std::optional<RuleTime>& dst_end) {
    Definition definition;
    definition.id = std::move(id);
    definition.standard_offset_micros = standard_offset_micros;
    definition.savings_micros = savings_micros;
    definition.dst_from_year = dst_from_year;
    definition.southern = southern;
    definition.dst_start = dst_start;
    definition.dst_end = dst_end;
    return definition;
}

const std::vector<Definition>& builtin_definitions() {
    static const std::vector<Definition> definitions = {
        make_definition("UTC", 0, 0, kMaxYear, false, std::nullopt, std::nullopt),
        make_definition("Asia/Shanghai", hours_micros(8), 0, kMaxYear, false, std::nullopt,
                        std::nullopt),
        make_definition("Asia/Kolkata", hours_micros(5) + minutes_micros(30), 0, kMaxYear, false,
                        std::nullopt, std::nullopt),
        make_definition("America/New_York", hours_micros(-5), 3600LL * kMicrosPerSecond, 2007,
                        false, RuleTime{3, 2, 0, 2, 0, false}, RuleTime{11, 1, 0, 2, 0, true}),
        make_definition("Europe/Berlin", hours_micros(1), 3600LL * kMicrosPerSecond, 1996, false,
                        RuleTime{3, -1, 0, 2, 0, false}, RuleTime{10, -1, 0, 3, 0, true}),
        make_definition("Australia/Sydney", hours_micros(10), 3600LL * kMicrosPerSecond, 2000, true,
                        RuleTime{10, 1, 0, 2, 0, false}, RuleTime{4, 1, 0, 3, 0, true}),
    };
    return definitions;
}

Result<ZonedInstant> zone_diagnostic(std::string_view zone_id, DetailCode code,
                                     std::string message) {
    Location location;
    location.description = std::string{zone_id};
    return Result<ZonedInstant>::failure(
        Diagnostic::reject(code, std::move(message), std::move(location)));
}

}  // namespace

TimeZoneRules::TimeZoneRules(std::string version, std::vector<Definition> definitions)
    : version_(std::move(version)), definitions_(std::move(definitions)) {}

const TimeZoneRules& TimeZoneRules::builtin() {
    static const TimeZoneRules rules{"EG-TZ-RULES-1", builtin_definitions()};
    return rules;
}

const TimeZoneRules::Definition* TimeZoneRules::find(std::string_view zone_id) const noexcept {
    for (const auto& definition : definitions_) {
        if (definition.id == zone_id) {
            return &definition;
        }
    }
    return nullptr;
}

bool TimeZoneRules::contains(std::string_view zone_id) const {
    return find(zone_id) != nullptr;
}

std::vector<std::string> TimeZoneRules::zone_ids() const {
    std::vector<std::string> ids;
    ids.reserve(definitions_.size());
    for (const auto& definition : definitions_) {
        ids.push_back(definition.id);
    }
    return ids;
}

std::optional<std::int64_t> TimeZoneRules::standard_offset(std::string_view zone_id) const noexcept {
    const auto* definition = find(zone_id);
    if (definition == nullptr) {
        return std::nullopt;
    }
    return definition->standard_offset_micros;
}

std::vector<TimeZoneRules::Transition> TimeZoneRules::transitions_around(
    const Definition& definition, std::int32_t year) const {
    std::vector<Transition> events;
    if (!definition.observes_daylight_time()) {
        return events;
    }
    for (std::int32_t candidate = year - 1; candidate <= year + 1; ++candidate) {
        if (candidate < definition.dst_from_year) {
            continue;
        }
        // A rule whose occurrence does not exist in a given year contributes no transition
        // to that year, so the offset simply carries across it.
        if (const auto start = transition_instant(definition, *definition.dst_start, candidate)) {
            events.push_back(Transition{
                *start, definition.standard_offset_micros,
                definition.standard_offset_micros + definition.savings_micros});
        }
        if (const auto end = transition_instant(definition, *definition.dst_end, candidate)) {
            events.push_back(Transition{
                *end, definition.standard_offset_micros + definition.savings_micros,
                definition.standard_offset_micros});
        }
    }
    std::sort(events.begin(), events.end(), [](const Transition& left, const Transition& right) {
        return left.instant_micros < right.instant_micros;
    });
    return events;
}

std::int64_t TimeZoneRules::offset_at(const Definition& definition,
                                      std::int64_t instant_micros) const {
    CivilDate reference;
    civil_from_days(floor_div(instant_micros, kMicrosPerDay), reference);
    const auto events = transitions_around(definition, reference.year);
    if (reference.year < definition.dst_from_year) {
        return definition.standard_offset_micros;
    }
    std::int64_t offset = definition.standard_offset_micros;
    if (definition.southern && definition.observes_daylight_time()) {
        // A southern zone already reads daylight time at the start of January.
        offset = definition.standard_offset_micros + definition.savings_micros;
    }
    for (const auto& event : events) {
        if (event.instant_micros > instant_micros) {
            break;
        }
        offset = event.offset_after_micros;
    }
    return offset;
}

Result<LocalObservation> TimeZoneRules::observe(std::int64_t instant_micros,
                                                std::string_view zone_id) const {
    const auto* definition = find(zone_id);
    if (definition == nullptr) {
        Location location;
        location.description = std::string{zone_id};
        return Result<LocalObservation>::failure(Diagnostic::reject(
            DetailCode::unknown_zone, "zone identifier is not in the declared rule table",
            std::move(location)));
    }
    const std::int64_t offset = offset_at(*definition, instant_micros);
    auto civil = civil_from_instant(instant_micros, offset);
    if (!civil.ok()) {
        return Result<LocalObservation>::failure(civil.diagnostic_copy());
    }
    LocalObservation observation;
    observation.local = civil.take();
    observation.offset_micros = offset;
    observation.in_daylight_time = offset != definition->standard_offset_micros;
    return observation;
}

Result<ZonedInstant> TimeZoneRules::resolve(CivilDateTime local, std::string_view zone_id,
                                            LocalTimeResolution resolution) const {
    const auto* definition = find(zone_id);
    if (definition == nullptr) {
        return zone_diagnostic(zone_id, DetailCode::unknown_zone,
                               "zone identifier is not in the declared rule table");
    }
    auto naive = naive_micros(local);
    if (!naive.ok()) {
        return Result<ZonedInstant>::failure(naive.diagnostic_copy());
    }
    const std::int64_t local_micros = naive.value();

    std::vector<std::int64_t> offsets{definition->standard_offset_micros};
    if (definition->observes_daylight_time()) {
        offsets.push_back(definition->standard_offset_micros + definition->savings_micros);
    }
    std::vector<std::int64_t> valid;
    for (const std::int64_t offset : offsets) {
        if (offset_at(*definition, local_micros - offset) == offset) {
            valid.push_back(offset);
        }
    }

    const std::int64_t low = *std::min_element(offsets.begin(), offsets.end());
    const std::int64_t high = *std::max_element(offsets.begin(), offsets.end());
    std::int64_t chosen = 0;
    if (valid.size() == 1) {
        chosen = valid.front();
    } else if (valid.empty()) {
        if (resolution == LocalTimeResolution::reject) {
            return zone_diagnostic(zone_id, DetailCode::date_nonexistent,
                                   "local time does not exist because the clocks jump forward");
        }
        chosen = resolution == LocalTimeResolution::pre_transition ? low : high;
    } else {
        if (resolution == LocalTimeResolution::reject) {
            return zone_diagnostic(zone_id, DetailCode::date_ambiguous,
                                   "local time occurs twice because the clocks fall back");
        }
        chosen = resolution == LocalTimeResolution::pre_transition ? high : low;
    }

    ZonedInstant instant;
    instant.instant_micros = local_micros - chosen;
    instant.offset_micros = chosen;
    instant.zone_id = std::string{zone_id};
    instant.in_daylight_time = chosen != definition->standard_offset_micros;
    return instant;
}

bool is_leap_year(std::int32_t year) noexcept {
    if (year % 4 != 0) {
        return false;
    }
    return year % 100 != 0 || year % 400 == 0;
}

Result<std::uint32_t> days_in_month(std::int32_t year, std::uint32_t month) noexcept {
    if (month < 1U || month > 12U) {
        return Result<std::uint32_t>::failure(
            date_diagnostic("month must be between 1 and 12"));
    }
    static constexpr std::uint32_t kLengths[12] = {31, 28, 31, 30, 31, 30,
                                                   31, 31, 30, 31, 30, 31};
    if (month == 2U && is_leap_year(year)) {
        return std::uint32_t{29};
    }
    return kLengths[month - 1U];
}

Result<std::int64_t> ordinal_days(CivilDate date) noexcept {
    if (date.year < kMinYear || date.year > kMaxYear) {
        return Result<std::int64_t>::failure(
            date_diagnostic("year is outside the supported proleptic Gregorian range"));
    }
    auto length = days_in_month(date.year, date.month);
    if (!length.ok()) {
        return Result<std::int64_t>::failure(length.diagnostic_copy());
    }
    if (date.day < 1U || date.day > length.value()) {
        Location location;
        location.description = "day " + std::to_string(date.day) + " of month " +
                               std::to_string(date.month) + " in year " +
                               std::to_string(date.year);
        return Result<std::int64_t>::failure(Diagnostic::reject(
            DetailCode::date_format, "day is outside the month, including the leap-day rule",
            std::move(location)));
    }
    return days_from_civil(date.year, date.month, date.day);
}

Result<CivilDate> date_from_ordinal(std::int64_t days) noexcept {
    CivilDate date;
    civil_from_days(days, date);
    if (date.year < kMinYear || date.year > kMaxYear) {
        return Result<CivilDate>::failure(
            date_diagnostic("day count is outside the supported proleptic Gregorian range"));
    }
    auto round_trip = days_from_civil(date.year, date.month, date.day);
    if (round_trip != days) {
        return Result<CivilDate>::failure(
            date_diagnostic("day count does not map back to a civil date"));
    }
    return date;
}

Result<std::int64_t> naive_micros(CivilDateTime local) noexcept {
    auto days = ordinal_days(local.date);
    if (!days.ok()) {
        return Result<std::int64_t>::failure(days.diagnostic_copy());
    }
    if (local.time.hour > 23U || local.time.minute > 59U || local.time.second > 59U ||
        local.time.microsecond > 999999U) {
        return Result<std::int64_t>::failure(
            date_diagnostic("time of day fields are outside their ranges"));
    }
    const std::int64_t seconds_of_day = static_cast<std::int64_t>(local.time.hour) * 3600LL +
                                        static_cast<std::int64_t>(local.time.minute) * 60LL +
                                        static_cast<std::int64_t>(local.time.second);
    return days.value() * kMicrosPerDay + seconds_of_day * kMicrosPerSecond +
           static_cast<std::int64_t>(local.time.microsecond);
}

Result<CivilDateTime> civil_from_naive(std::int64_t micros) noexcept {
    const std::int64_t floor_days = floor_div(micros, kMicrosPerDay);
    const std::int64_t rest = micros - floor_days * kMicrosPerDay;
    auto date = date_from_ordinal(floor_days);
    if (!date.ok()) {
        return Result<CivilDateTime>::failure(date.diagnostic_copy());
    }
    CivilDateTime local;
    local.date = date.take();
    local.time.hour = static_cast<std::uint32_t>(rest / (3600LL * kMicrosPerSecond));
    local.time.minute = static_cast<std::uint32_t>((rest / (60LL * kMicrosPerSecond)) % 60LL);
    local.time.second = static_cast<std::uint32_t>((rest / kMicrosPerSecond) % 1000000LL);
    local.time.microsecond = static_cast<std::uint32_t>(rest % kMicrosPerSecond);
    return local;
}

Result<CivilDateTime> civil_from_instant(std::int64_t instant_micros,
                                         std::int64_t offset_micros) noexcept {
    return civil_from_naive(instant_micros + offset_micros);
}

}  // namespace everygraph::data
