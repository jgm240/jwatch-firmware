#include "components/datetime/WorldTime.h"

using namespace Pinetime::Controllers;

const std::array<WorldCity, worldCityCount> Pinetime::Controllers::worldCities = {{
  {"Honolulu", "HNL", -600, DstRule::None},
  {"Anchorage", "ANC", -540, DstRule::Us},
  {"Los Angeles", "LAX", -480, DstRule::Us},
  {"Denver", "DEN", -420, DstRule::Us},
  {"Chicago", "CHI", -360, DstRule::Us},
  {"Mexico City", "MEX", -360, DstRule::None},
  {"New York", "NYC", -300, DstRule::Us},
  {"Sao Paulo", "SAO", -180, DstRule::None},
  {"Reykjavik", "REK", 0, DstRule::None},
  {"London", "LON", 0, DstRule::Eu},
  {"Lisbon", "LIS", 0, DstRule::Eu},
  {"Paris", "PAR", 60, DstRule::Eu},
  {"Berlin", "BER", 60, DstRule::Eu},
  {"Vienna", "VIE", 60, DstRule::Eu},
  {"Athens", "ATH", 120, DstRule::Eu},
  {"Joburg", "JNB", 120, DstRule::None},
  {"Istanbul", "IST", 180, DstRule::None},
  {"Moscow", "MOW", 180, DstRule::None},
  {"Dubai", "DXB", 240, DstRule::None},
  {"Delhi", "DEL", 330, DstRule::None},
  {"Bangkok", "BKK", 420, DstRule::None},
  {"Singapore", "SIN", 480, DstRule::None},
  {"Hong Kong", "HKG", 480, DstRule::None},
  {"Tokyo", "TYO", 540, DstRule::None},
  {"Seoul", "SEL", 540, DstRule::None},
  {"Sydney", "SYD", 600, DstRule::Au},
  {"Auckland", "AKL", 720, DstRule::Nz},
}};

namespace {
  // Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm).
  int64_t DaysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2 ? 1 : 0;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
  }

  int YearOfDay(int64_t days) {
    const int64_t z = days + 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const int64_t doe = z - era * 146097;
    const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int64_t mp = (5 * doy + 2) / 153;
    const int64_t month = mp < 10 ? mp + 3 : mp - 9;
    return static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0));
  }

  // 0 = Sunday.
  int WeekdayOfDay(int64_t days) {
    int weekday = static_cast<int>((days + 4) % 7);
    return weekday < 0 ? weekday + 7 : weekday;
  }

  int64_t NthSunday(int year, unsigned month, int n) {
    const int64_t first = DaysFromCivil(year, month, 1);
    return first + (7 - WeekdayOfDay(first)) % 7 + 7 * (n - 1);
  }

  int64_t LastSunday(int year, unsigned month) {
    const int64_t next = month == 12 ? DaysFromCivil(year + 1, 1, 1) : DaysFromCivil(year, month + 1, 1);
    const int64_t last = next - 1;
    return last - WeekdayOfDay(last);
  }
}

int Pinetime::Controllers::WorldOffsetMinutes(const WorldCity& city, int64_t utcSeconds) {
  if (city.rule == DstRule::None) {
    return city.stdMinutes;
  }
  const int64_t std = static_cast<int64_t>(city.stdMinutes) * 60;
  const int64_t dst = std + 3600;
  const int year = YearOfDay(DayNumber(utcSeconds));
  bool inDst = false;
  switch (city.rule) {
    case DstRule::Us: // 2nd Sunday in March 02:00 local standard -> 1st Sunday in November 02:00 local daylight
      inDst = utcSeconds >= NthSunday(year, 3, 2) * 86400 + 2 * 3600 - std && utcSeconds < NthSunday(year, 11, 1) * 86400 + 2 * 3600 - dst;
      break;
    case DstRule::Eu: // last Sunday in March -> last Sunday in October, both at 01:00 UTC
      inDst = utcSeconds >= LastSunday(year, 3) * 86400 + 3600 && utcSeconds < LastSunday(year, 10) * 86400 + 3600;
      break;
    case DstRule::Au: // southern hemisphere: 1st Sunday in October 02:00 standard -> 1st Sunday in April 03:00 daylight
      inDst = utcSeconds >= NthSunday(year, 10, 1) * 86400 + 2 * 3600 - std || utcSeconds < NthSunday(year, 4, 1) * 86400 + 3 * 3600 - dst;
      break;
    case DstRule::Nz: // last Sunday in September 02:00 standard -> 1st Sunday in April 03:00 daylight
      inDst = utcSeconds >= LastSunday(year, 9) * 86400 + 2 * 3600 - std || utcSeconds < NthSunday(year, 4, 1) * 86400 + 3 * 3600 - dst;
      break;
    default:
      break;
  }
  return city.stdMinutes + (inDst ? 60 : 0);
}
