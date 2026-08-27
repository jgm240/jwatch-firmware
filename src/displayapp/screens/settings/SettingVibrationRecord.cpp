#include "displayapp/screens/settings/SettingVibrationRecord.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::PatternData;
using Pinetime::Controllers::VibrationPattern;
using Pinetime::Controllers::maxPatternSteps;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingVibrationRecord*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  uint16_t TicksToMs(TickType_t ticks) {
    uint32_t ms = ticks * 1000u / configTICK_RATE_HZ;
    if (ms > UINT16_MAX) {
      return UINT16_MAX;
    }
    return static_cast<uint16_t>(ms);
  }
}

SettingVibrationRecord::SettingVibrationRecord(Pinetime::Controllers::Settings& settingsController,
                                                 Pinetime::Controllers::MotorController& motorController)
  : settingsController {settingsController}, motorController {motorController} {

  stepLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(stepLabel, "Step 0/8");
  lv_obj_align(stepLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  tapZone = Styles::Button(lv_scr_act());
  tapZone->user_data = this;
  lv_obj_set_event_cb(tapZone, eventHandler);
  lv_obj_set_size(tapZone, 200, 120);
  lv_obj_align(tapZone, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);
  lv_obj_t* tapLabel = lv_label_create(tapZone, nullptr);
  lv_label_set_text(tapLabel, "Tap out\nyour pattern");
  lv_label_set_align(tapLabel, LV_LABEL_ALIGN_CENTER);

  btnPreview = Styles::Button(lv_scr_act());
  btnPreview->user_data = this;
  lv_obj_set_event_cb(btnPreview, eventHandler);
  lv_obj_set_size(btnPreview, 76, 50);
  lv_obj_align(btnPreview, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_t* previewLabel = lv_label_create(btnPreview, nullptr);
  lv_label_set_text_static(previewLabel, Symbols::play);

  btnSave = Styles::Button(lv_scr_act());
  btnSave->user_data = this;
  lv_obj_set_event_cb(btnSave, eventHandler);
  lv_obj_set_size(btnSave, 76, 50);
  lv_obj_align(btnSave, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_t* saveLabel = lv_label_create(btnSave, nullptr);
  lv_label_set_text_static(saveLabel, Symbols::check);

  btnCancel = Styles::Button(lv_scr_act());
  btnCancel->user_data = this;
  lv_obj_set_event_cb(btnCancel, eventHandler);
  lv_obj_set_size(btnCancel, 76, 50);
  lv_obj_align(btnCancel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t* cancelLabel = lv_label_create(btnCancel, nullptr);
  lv_label_set_text_static(cancelLabel, Symbols::ban);
}

SettingVibrationRecord::~SettingVibrationRecord() {
  motorController.StopPattern();
  lv_obj_clean(lv_scr_act());
}

Pinetime::Controllers::PatternData SettingVibrationRecord::Finalized() const {
  PatternData result = recording;
  if (hasPendingStep && result.stepCount < maxPatternSteps) {
    result.offMs[result.stepCount] = 0;
    result.stepCount++;
  }
  return result;
}

void SettingVibrationRecord::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (obj == tapZone) {
    switch (event) {
      case LV_EVENT_PRESSED: {
        if (recording.stepCount >= maxPatternSteps) {
          break;
        }
        TickType_t now = xTaskGetTickCount();
        if (hasPendingStep) {
          recording.offMs[recording.stepCount] = TicksToMs(now - releaseStartTick);
          recording.stepCount++;
          hasPendingStep = false;
        }
        if (recording.stepCount >= maxPatternSteps) {
          break;
        }
        isPressed = true;
        pressStartTick = now;
        motorController.MotorOn();
        break;
      }
      case LV_EVENT_RELEASED:
      case LV_EVENT_PRESS_LOST: {
        if (!isPressed) {
          break;
        }
        isPressed = false;
        motorController.MotorOff();
        TickType_t now = xTaskGetTickCount();
        recording.onMs[recording.stepCount] = TicksToMs(now - pressStartTick);
        releaseStartTick = now;
        hasPendingStep = true;
        lv_label_set_text_fmt(stepLabel, "Step %d/%d", recording.stepCount + 1, static_cast<int>(maxPatternSteps));
        break;
      }
      default:
        break;
    }
  } else if (obj == btnPreview && event == LV_EVENT_CLICKED) {
    motorController.RunPattern(Finalized());
  } else if (obj == btnSave && event == LV_EVENT_CLICKED) {
    auto finalized = Finalized();
    settingsController.SetCustomVibrationPattern(finalized);
    settingsController.SetVibrationPattern(VibrationPattern::Custom);
    settingsController.SaveSettings();
    running = false;
  } else if (obj == btnCancel && event == LV_EVENT_CLICKED) {
    running = false;
  }
}

bool SettingVibrationRecord::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeDown && isPressed) {
    return true;
  }
  return false;
}
