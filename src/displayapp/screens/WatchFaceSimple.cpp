#include "displayapp/screens/WatchFaceSimple.h"

#include <lvgl/lvgl.h>
#include <algorithm>

#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const lv_font_t* ConvertFont(Pinetime::Controllers::Settings::WatchFaceSimpleFont font) {
    switch (font) {
      case Pinetime::Controllers::Settings::WatchFaceSimpleFont::Light:
        return &open_sans_light;
      case Pinetime::Controllers::Settings::WatchFaceSimpleFont::Mono76:
        return &jetbrains_mono_76;
      case Pinetime::Controllers::Settings::WatchFaceSimpleFont::ExtraBoldCompressed:
      default:
        return &jetbrains_mono_extrabold_compressed;
    }
  }

  // Cheap 2D parallax, not real 3D tilt - see WatchFaceDigital.cpp for the full rationale.
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

WatchFaceSimple::WatchFaceSimple(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 const Controllers::AlarmController& alarmController,
                                 Controllers::Settings& settingsController,
                                 Controllers::MotionController& motionController)
  : stepsComplication {motionController}, dateTimeController {dateTimeController}, settingsController {settingsController},
    motionController {motionController}, statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();

  lv_color_t secondaryTextColor = Colors::lightGray;

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, ConvertFont(settingsController.GetWatchFaceSimpleFont()));
  lv_obj_set_style_local_text_color(
    label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Applications::Convert(settingsController.GetWatchFaceSimpleColor()));
  lv_label_set_text_static(label_time, "");
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryTextColor);
  lv_label_set_text_static(label_date, "");
  lv_obj_align(label_date, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  stepsComplication.Create(lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  secondsSquare.Create(lv_scr_act(), Applications::Convert(settingsController.GetWatchFaceSimpleColor()));

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceSimple::~WatchFaceSimple() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceSimple::Refresh() {
  statusIcons.Update();

  secondsSquare.SetValue(dateTimeController.Seconds());

  uint8_t tiltStrength = settingsController.GetTiltParallaxStrength();
  lv_coord_t tiltX = TiltOffset(motionController.X(), tiltStrength);
  lv_coord_t tiltY = TiltOffset(motionController.Y(), tiltStrength);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, tiltX, tiltY);
  lv_obj_realign(label_date);

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

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      lv_label_set_text_fmt(label_date, "%s %d %s", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day(), dateTimeController.MonthShortToString());
    }
  }

  stepsComplication.Update();
}
