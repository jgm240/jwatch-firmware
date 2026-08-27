#include "components/motor/VibrationPatterns.h"

using namespace Pinetime::Controllers;

const PatternData& VibrationPatterns::Get(VibrationPattern pattern, const PatternData& custom) {
  switch (pattern) {
    case VibrationPattern::Single:
      return Single;
    case VibrationPattern::Double:
      return Double;
    case VibrationPattern::Triple:
      return Triple;
    case VibrationPattern::Long:
      return Long;
    case VibrationPattern::Ramp:
      return Ramp;
    case VibrationPattern::Custom:
    default:
      return custom;
  }
}
