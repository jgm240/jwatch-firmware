#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    enum class DstRule : uint8_t { None, Us, Eu, Au, Nz };

    struct WorldCity {
      const char* name;
      const char* code;
      int16_t stdMinutes; // offset from UTC in standard time
      DstRule rule;
    };

    constexpr size_t worldCityCount = 27;
    constexpr size_t worldClockSlots = 5;
    extern const std::array<WorldCity, worldCityCount> worldCities;

    // Total UTC offset in minutes for `city` at the given UTC instant, including daylight saving time.
    int WorldOffsetMinutes(const WorldCity& city, int64_t utcSeconds);

    // Floor division by a day, so instants before 1970 and negative local times behave.
    inline int64_t DayNumber(int64_t seconds) {
      return seconds >= 0 ? seconds / 86400 : -((-seconds + 86399) / 86400);
    }
  }
}
