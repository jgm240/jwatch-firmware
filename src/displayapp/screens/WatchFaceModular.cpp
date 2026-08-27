#include "displayapp/screens/WatchFaceModular.h"

#include <lvgl/lvgl.h>
#include <algorithm>

#include "displayapp/widgets/complications/ComplicationFactory.h"
#include "displayapp/Colors.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr std::array<lv_align_t, Pinetime::Controllers::Settings::modularWatchFaceSlots> slotAlign =
    {LV_ALIGN_IN_TOP_LEFT, LV_ALIGN_IN_TOP_RIGHT, LV_ALIGN_IN_BOTTOM_LEFT, LV_ALIGN_IN_BOTTOM_RIGHT};

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

WatchFaceModular::WatchFaceModular(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController, AppControllers& controllers)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, motionController {controllers.motionController} {

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_set_style_local_text_color(
    label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Applications::Convert(settingsController.GetWatchFaceAccentColor()));
  lv_label_set_text_static(label_time, "");
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_date, "");
  lv_obj_align(label_date, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  for (size_t i = 0; i < complications.size(); i++) {
    auto type = settingsController.GetModularSlot(i);
    complications[i] = Widgets::CreateComplication(type, controllers);
    if (complications[i] != nullptr) {
      complications[i]->Create(lv_scr_act(), slotAlign[i], 0, 0);
    }
  }

  secondsSquare.Create(lv_scr_act(), Applications::Convert(settingsController.GetWatchFaceAccentColor()));

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceModular::~WatchFaceModular() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceModular::Refresh() {
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

    lv_label_set_text_fmt(label_date, "%s %d %s", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day(), dateTimeController.MonthShortToString());
  }

  for (auto& complication : complications) {
    if (complication != nullptr) {
      complication->Update();
    }
  }
}
