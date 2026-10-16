/*  Copyright (C) 2021  Adam Pigg

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "displayapp/screens/Navigation.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "components/ble/NavigationService.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_42;

namespace {
  constexpr lv_coord_t boxSize = 80;

  void StyleLine(lv_obj_t* line) {
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_obj_set_style_local_line_rounded(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
  }

  void StyleBlob(lv_obj_t* obj, lv_coord_t border, bool filled) {
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, border);
    lv_obj_set_style_local_border_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, filled ? LV_OPA_COVER : LV_OPA_TRANSP);
  }
}

/**
 * Navigation watchapp. The maneuver arrows are drawn with lines instead of the icon files of upstream
 * InfiniTime, so the app works without the external resources installed.
 */
Navigation::Navigation(Pinetime::Controllers::NavigationService& nav) : navService(nav) {
  arrowBox = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(arrowBox, boxSize, boxSize);
  lv_obj_set_style_local_bg_opa(arrowBox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_border_width(arrowBox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(arrowBox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_align(arrowBox, nullptr, LV_ALIGN_CENTER, 0, -60);

  ring = lv_obj_create(arrowBox, nullptr);
  StyleBlob(ring, 6, false);
  dot = lv_obj_create(arrowBox, nullptr);
  StyleBlob(dot, 0, true);
  arrowPath = lv_line_create(arrowBox, nullptr);
  StyleLine(arrowPath);
  arrowHead = lv_line_create(arrowBox, nullptr);
  StyleLine(arrowHead);
  DrawFlag("flag");

  txtNarrative = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtNarrative, LV_LABEL_LONG_DOT);
  lv_obj_set_width(txtNarrative, LV_HOR_RES);
  lv_obj_set_height(txtNarrative, 80);
  lv_label_set_text_static(txtNarrative, "Navigation");
  lv_label_set_align(txtNarrative, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(txtNarrative, nullptr, LV_ALIGN_CENTER, 0, 30);

  txtManDist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtManDist, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_color(txtManDist, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_text_font(txtManDist, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_width(txtManDist, LV_HOR_RES);
  lv_label_set_text_static(txtManDist, "--M");
  lv_label_set_align(txtManDist, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(txtManDist, nullptr, LV_ALIGN_CENTER, 0, 90);

  // Route Progress
  barProgress = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(barProgress, 200, 20);
  lv_obj_align(barProgress, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_bar_set_anim_time(barProgress, 500);
  lv_bar_set_range(barProgress, 0, 100);
  lv_bar_set_value(barProgress, 0, LV_ANIM_OFF);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Navigation::~Navigation() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Navigation::DrawFlag(const std::string& f) {
  auto has = [&f](const char* word) {
    return f.find(word) != std::string::npos;
  };

  lv_obj_set_hidden(ring, true);
  lv_obj_set_hidden(dot, true);
  lv_obj_set_hidden(arrowPath, true);
  lv_obj_set_hidden(arrowHead, true);

  if (f.empty() || has("arrive") || has("flag") || has("ferry")) {
    // Destination pin: a ring with a dot.
    lv_obj_set_size(ring, 56, 56);
    lv_obj_set_pos(ring, 12, 12);
    lv_obj_set_size(dot, 20, 20);
    lv_obj_set_pos(dot, 30, 30);
    lv_obj_set_hidden(ring, false);
    lv_obj_set_hidden(dot, false);
    return;
  }

  // Arrow shaft as up to 4 points in the 80x80 box (the tip is the last one); the head is derived below.
  const bool left = has("left");
  const bool right = has("right");
  const bool sharp = has("sharp");
  const bool slight = has("slight");
  uint8_t count = 0;
  auto add = [this, &count](lv_coord_t x, lv_coord_t y) {
    pathPoints[count++] = {x, y};
  };
  // Mirror x for right-hand variants so each shape is only written once (for "left").
  auto mx = [right](lv_coord_t x) -> lv_coord_t {
    return right ? static_cast<lv_coord_t>(boxSize - x) : x;
  };

  if (has("uturn")) {
    add(mx(56), 76);
    add(mx(56), 30);
    add(mx(24), 30);
    add(mx(24), 56);
  } else if (has("roundabout") || has("rotary")) {
    lv_obj_set_size(ring, 40, 40);
    lv_obj_set_pos(ring, 20, 38);
    lv_obj_set_hidden(ring, false);
    if (left) {
      add(26, 42);
      add(8, 22);
    } else if (right) {
      add(54, 42);
      add(72, 22);
    } else {
      add(40, 38);
      add(40, 6);
    }
  } else if (left || right) {
    if (sharp) {
      add(mx(56), 70);
      add(mx(56), 36);
      add(mx(24), 60);
    } else if (slight) {
      add(mx(50), 76);
      add(mx(50), 50);
      add(mx(26), 22);
    } else {
      add(mx(52), 76);
      add(mx(52), 44);
      add(mx(16), 44);
    }
  } else {
    add(40, 76);
    add(40, 12);
  }

  // Arrow head: a chevron at the tip, oriented along the last segment.
  const lv_point_t from = pathPoints[count - 2];
  const lv_point_t tip = pathPoints[count - 1];
  float dx = static_cast<float>(tip.x - from.x);
  float dy = static_cast<float>(tip.y - from.y);
  const float length = std::sqrt(dx * dx + dy * dy);
  if (length > 0) {
    dx /= length;
    dy /= length;
  }
  const float backX = tip.x - 16 * dx;
  const float backY = tip.y - 16 * dy;
  headPoints[0] = {static_cast<lv_coord_t>(backX - 12 * dy), static_cast<lv_coord_t>(backY + 12 * dx)};
  headPoints[1] = tip;
  headPoints[2] = {static_cast<lv_coord_t>(backX + 12 * dy), static_cast<lv_coord_t>(backY - 12 * dx)};

  lv_line_set_points(arrowPath, pathPoints, count);
  lv_line_set_points(arrowHead, headPoints, 3);
  lv_obj_set_pos(arrowPath, 0, 0);
  lv_obj_set_pos(arrowHead, 0, 0);
  lv_obj_set_hidden(arrowPath, false);
  lv_obj_set_hidden(arrowHead, false);
}

void Navigation::Refresh() {
  if (flag != navService.getFlag()) {
    flag = navService.getFlag();
    DrawFlag(flag);
  }

  if (narrative != navService.getNarrative()) {
    narrative = navService.getNarrative();
    lv_label_set_text(txtNarrative, narrative.data());
  }

  if (manDist != navService.getManDist()) {
    manDist = navService.getManDist();
    lv_label_set_text(txtManDist, manDist.data());
  }

  if (progress != navService.getProgress()) {
    progress = navService.getProgress();
    lv_bar_set_value(barProgress, progress, LV_ANIM_OFF);
    if (progress > 90) {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
    } else {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::orange);
    }
  }
}

bool Navigation::IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
  return true;
}
