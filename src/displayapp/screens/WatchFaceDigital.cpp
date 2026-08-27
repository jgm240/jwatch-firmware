#include "displayapp/screens/WatchFaceDigital.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include <algorithm>

#include "displayapp/InfiniTimeTheme.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"
#include "displayapp/Colors.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Cheap 2D parallax, not real 3D tilt (this hardware/LVGL version has no perspective
  // transform at all) - just nudges the time label a few px opposite to the accelerometer's
  // tilt, reusing readings SystemTask already samples continuously. Strength is user-adjustable
  // (Settings > Tilt Parallax, 0=off..140=max); the earlier fixed 200/6 combo produced
  // sub-pixel, imperceptible offsets for ordinary hand tilts, so the divisor here is lower.
  // tiltMaxOffsetCap keeps the top of that 140-wide range from pushing the label off-screen
  // entirely - strength still scales the effect linearly up to that cap.
  constexpr int16_t tiltDivisor = 90;
  constexpr lv_coord_t tiltMaxOffsetPerStrength = 3;
  constexpr lv_coord_t tiltMaxOffsetCap = 100;

  lv_coord_t TiltOffset(int16_t raw, uint8_t strength) {
    lv_coord_t maxOffset = std::min(static_cast<lv_coord_t>(static_cast<lv_coord_t>(strength) * tiltMaxOffsetPerStrength), tiltMaxOffsetCap);
    if (maxOffset == 0) {
      return 0;
    }
    int32_t offset = raw / tiltDivisor;
    if (offset > maxOffset) {
      offset = maxOffset;
    } else if (offset < -maxOffset) {
      offset = -maxOffset;
    }
    return static_cast<lv_coord_t>(offset);
  }
}

WatchFaceDigital::WatchFaceDigital(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   const Controllers::AlarmController& alarmController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    notificationComplication {notificationManager},
    weatherComplication {weatherService, settingsController},
    heartRateComplication {heartRateController},
    stepsComplication {motionController},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService},
    statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();

  notificationComplication.Create(lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  weatherComplication.Create(lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 50);

  bool isLight = settingsController.GetTheme() == Controllers::Settings::Theme::Light;
  lv_color_t secondaryTextColor = isLight ? Colors::Light::text : lv_color_hex(0x999999);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryTextColor);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_set_style_local_text_color(
    label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Applications::Convert(settingsController.GetWatchFaceAccentColor()));

  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

  heartRateComplication.Create(lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  stepsComplication.Create(lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  // Square seconds track around the screen edge (Apple Watch-style), sweeping clockwise
  // around its 4 sides over 60 seconds.
  secondsSquare.Create(lv_scr_act(), Applications::Convert(settingsController.GetWatchFaceAccentColor()));

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceDigital::Refresh() {
  statusIcons.Update();

  notificationComplication.Update();

  secondsSquare.SetValue(dateTimeController.Seconds());

  uint8_t tiltStrength = settingsController.GetTiltParallaxStrength();
  lv_coord_t tiltX = TiltOffset(motionController.X(), tiltStrength);
  lv_coord_t tiltY = TiltOffset(motionController.Y(), tiltStrength);
  lv_align_t timeAlign =
    settingsController.GetClockType() == Controllers::Settings::ClockType::H12 ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_CENTER;
  lv_obj_align(label_time, lv_scr_act(), timeAlign, tiltX, tiltY);

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
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date,
                              "%s %d %s %d",
                              dateTimeController.DayOfWeekShortToString(),
                              day,
                              dateTimeController.MonthShortToString(),
                              year);
      } else {
        lv_label_set_text_fmt(label_date,
                              "%s %s %d %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day,
                              year);
      }
      lv_obj_realign(label_date);
    }
  }

  heartRateComplication.Update();
  stepsComplication.Update();
  weatherComplication.Update();
}
