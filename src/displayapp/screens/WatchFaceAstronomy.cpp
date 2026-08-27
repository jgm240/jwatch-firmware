#include "displayapp/screens/WatchFaceAstronomy.h"
#include <cmath>
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/alarm/AlarmController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Synodic month (new moon to new moon) in days, and a known new moon reference date
  // (2000-01-06, expressed as days since the Unix epoch) - the standard, widely-used
  // approximation for a simple phase calculation. Day-level granularity only.
  constexpr double synodicMonthDays = 29.53058867;
  constexpr int32_t referenceNewMoonEpochDay = 10958;

  // 0.0 = new moon, 0.5 = full moon, approaching 1.0 = new moon again.
  double MoonPhase(int32_t epochDay) {
    double daysSinceReference = static_cast<double>(epochDay - referenceNewMoonEpochDay);
    double phase = std::fmod(daysSinceReference, synodicMonthDays) / synodicMonthDays;
    if (phase < 0.0) {
      phase += 1.0;
    }
    return phase;
  }
}

WatchFaceAstronomy::WatchFaceAstronomy(Controllers::DateTime& dateTimeController,
                                       Controllers::Settings& settingsController,
                                       const Controllers::Battery& batteryController,
                                       const Controllers::Ble& bleController,
                                       const Controllers::AlarmController& alarmController)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();
  lv_obj_align(statusIcons.GetObject(), lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -8, 0);

  // Small fixed starfield, drawn once - purely decorative, never redrawn.
  for (int i = 0; i < starCount; i++) {
    stars[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(stars[i], 2, 2);
    lv_obj_set_style_local_bg_color(stars[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_opa(stars[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
    lv_obj_set_style_local_border_width(stars[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(stars[i], false);
    lv_coord_t x = static_cast<lv_coord_t>((i * 47 + 13) % LV_HOR_RES);
    lv_coord_t y = static_cast<lv_coord_t>((i * 29 + 7) % LV_VER_RES);
    lv_obj_set_pos(stars[i], x, y);
  }

  canvas = lv_canvas_create(lv_scr_act(), nullptr);
  lv_canvas_set_buffer(canvas, canvasBuf, canvasSize, canvasSize, LV_IMG_CF_TRUE_COLOR);
  lv_obj_align(canvas, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_static(label_time, "");
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(label_date, "");
  lv_obj_align(label_date, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceAstronomy::~WatchFaceAstronomy() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceAstronomy::DrawMoon() {
  lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, LV_OPA_COVER);

  auto epochDay =
    static_cast<int32_t>(std::chrono::time_point_cast<std::chrono::days>(dateTimeController.CurrentDateTime()).time_since_epoch().count());
  double phase = MoonPhase(epochDay);

  constexpr lv_coord_t radius = 28;
  constexpr lv_coord_t cx = canvasSize / 2;
  constexpr lv_coord_t cy = canvasSize / 2;

  lv_draw_rect_dsc_t moonDsc;
  lv_draw_rect_dsc_init(&moonDsc);
  moonDsc.radius = LV_RADIUS_CIRCLE;
  moonDsc.bg_color = lv_color_hex(0xd8d8d0);
  moonDsc.bg_opa = LV_OPA_COVER;
  lv_canvas_draw_rect(canvas, cx - radius, cy - radius, radius * 2, radius * 2, &moonDsc);

  // Shadow disc, same radius, shifted horizontally: 0 offset fully covers the moon (new),
  // 2*radius offset doesn't overlap it at all (full). Waning mirrors the same offset back
  // down to 0 - the crescent shape isn't astronomically exact, but reads correctly at a
  // glance, which is what a decorative watchface needs.
  double offsetFraction = (phase <= 0.5) ? (phase * 2.0) : ((1.0 - phase) * 2.0);
  auto shadowOffsetX = static_cast<lv_coord_t>(offsetFraction * radius * 2.0);

  lv_draw_rect_dsc_t shadowDsc;
  lv_draw_rect_dsc_init(&shadowDsc);
  shadowDsc.radius = LV_RADIUS_CIRCLE;
  shadowDsc.bg_color = LV_COLOR_BLACK;
  shadowDsc.bg_opa = LV_OPA_COVER;
  lv_canvas_draw_rect(canvas, cx - radius + shadowOffsetX, cy - radius, radius * 2, radius * 2, &shadowDsc);
}

void WatchFaceAstronomy::Refresh() {
  statusIcons.Update();

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text_fmt(label_time, "%2d:%02d %s", hour, minute, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_realign(label_time);
  }

  currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
  if (currentDate.IsUpdated()) {
    lv_label_set_text_fmt(
      label_date, "%s %d %s", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day(), dateTimeController.MonthShortToString());
    lv_obj_realign(label_date);
    DrawMoon();
  }
}
