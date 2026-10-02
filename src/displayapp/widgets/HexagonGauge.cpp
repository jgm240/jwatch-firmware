#include "displayapp/widgets/HexagonGauge.h"
#include <algorithm>

using namespace Pinetime::Applications::Widgets;

namespace {
  lv_obj_t* CreateLine(lv_obj_t* parent, lv_coord_t lineWidth, lv_color_t color, lv_opa_t opa) {
    lv_obj_t* line = lv_line_create(parent, nullptr);
    lv_obj_set_click(line, false);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lineWidth);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, opa);
    lv_obj_set_style_local_line_rounded(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    return line;
  }

  lv_point_t Lerp(lv_point_t from, lv_point_t to, int32_t numerator, int32_t denominator) {
    return {static_cast<lv_coord_t>(from.x + (to.x - from.x) * numerator / denominator),
            static_cast<lv_coord_t>(from.y + (to.y - from.y) * numerator / denominator)};
  }
}

void HexagonGauge::Create(lv_obj_t* parent, lv_coord_t circumradius, lv_coord_t lineWidth, lv_color_t color) {
  radius = circumradius;
  const lv_coord_t half = radius / 2;
  const lv_coord_t h = static_cast<lv_coord_t>(radius * 866 / 1000); // radius * sin(60 deg)

  // Clockwise from the middle of the top edge; the last point closes the loop.
  outline = {{{radius, 0},
              {static_cast<lv_coord_t>(radius + half), 0},
              {static_cast<lv_coord_t>(2 * radius), h},
              {static_cast<lv_coord_t>(radius + half), static_cast<lv_coord_t>(2 * h)},
              {static_cast<lv_coord_t>(radius - half), static_cast<lv_coord_t>(2 * h)},
              {0, h},
              {static_cast<lv_coord_t>(radius - half), 0},
              {radius, 0}}};

  const lv_coord_t offsetX = (lv_obj_get_width(parent) - 2 * radius) / 2;
  const lv_coord_t offsetY = (lv_obj_get_height(parent) - 2 * h) / 2;

  track = CreateLine(parent, lineWidth, color, LV_OPA_20);
  trackPoints = outline;
  lv_line_set_points(track, trackPoints.data(), maxPoints);
  lv_obj_set_pos(track, offsetX, offsetY);

  progress = CreateLine(parent, lineWidth, color, LV_OPA_COVER);
  lv_obj_set_pos(progress, offsetX, offsetY);
  lv_obj_set_hidden(progress, true);
}

void HexagonGauge::SetValue(uint8_t percent) {
  percent = std::min<uint8_t>(percent, 100);
  if (percent == lastPercent) {
    return;
  }
  lastPercent = percent;

  if (percent == 0) {
    lv_obj_set_hidden(progress, true);
    return;
  }

  // The outline is 7 segments: half a side, five full sides, half a side. Measure the
  // distance covered in hundredths of half-a-side so everything stays in integers.
  int32_t remaining = percent * 12;
  uint8_t count = 0;
  progressPoints[count++] = outline[0];
  for (uint8_t segment = 0; segment < 7 && remaining > 0; segment++) {
    const int32_t length = (segment == 0 || segment == 6) ? 100 : 200;
    if (remaining >= length) {
      progressPoints[count++] = outline[segment + 1];
      remaining -= length;
    } else {
      progressPoints[count++] = Lerp(outline[segment], outline[segment + 1], remaining, length);
      remaining = 0;
    }
  }

  lv_line_set_points(progress, progressPoints.data(), count);
  lv_obj_set_hidden(progress, false);
}
