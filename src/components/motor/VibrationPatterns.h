#pragma once

#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    enum class VibrationPattern : uint8_t { Single, Double, Triple, Long, Ramp, Custom };

    static constexpr size_t maxPatternSteps = 8;

    struct PatternData {
      uint8_t stepCount = 0;
      uint16_t onMs[maxPatternSteps] = {};
      uint16_t offMs[maxPatternSteps] = {};
    };

    namespace VibrationPatterns {
      constexpr PatternData Single = {1, {50}, {0}};
      constexpr PatternData Double = {2, {50, 50}, {100, 0}};
      constexpr PatternData Triple = {3, {40, 40, 40}, {80, 80, 0}};
      constexpr PatternData Long = {1, {400}, {0}};
      constexpr PatternData Ramp = {4, {30, 50, 70, 100}, {60, 60, 60, 0}};

      const PatternData& Get(VibrationPattern pattern, const PatternData& custom);
    }
  }
}
