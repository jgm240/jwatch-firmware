#include "displayapp/screens/settings/SettingAssist.h"
#include <lvgl/lvgl.h>
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingAssist*>(obj->user_data);
    if (event == LV_EVENT_VALUE_CHANGED) {
      screen->Toggled(obj);
    }
  }

  const char* const names[] = {"Smart alarm", "Stand reminders", "Haptic nav", "Shake to silence", "Phone lost alert"};
}

SettingAssist::SettingAssist(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container, 10, 35);
  lv_obj_set_width(container, LV_HOR_RES - 20);
  lv_obj_set_height(container, LV_VER_RES - 20);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Assists");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::bell);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  const bool values[optionCount] = {settingsController.GetSmartAlarmEnabled(),
                                    settingsController.GetStandReminders(),
                                    settingsController.GetHapticNavigation(),
                                    settingsController.GetShakeToSilence(),
                                    settingsController.GetPhoneLostAlert()};
  for (uint8_t i = 0; i < optionCount; i++) {
    checkboxes[i] = lv_checkbox_create(container, nullptr);
    lv_checkbox_set_text_static(checkboxes[i], names[i]);
    lv_checkbox_set_checked(checkboxes[i], values[i]);
    checkboxes[i]->user_data = this;
    lv_obj_set_event_cb(checkboxes[i], EventHandler);
  }
}

SettingAssist::~SettingAssist() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingAssist::Toggled(lv_obj_t* object) {
  for (uint8_t i = 0; i < optionCount; i++) {
    if (checkboxes[i] != object) {
      continue;
    }
    const bool on = lv_checkbox_is_checked(object);
    switch (i) {
      case 0:
        settingsController.SetSmartAlarmEnabled(on);
        break;
      case 1:
        settingsController.SetStandReminders(on);
        break;
      case 2:
        settingsController.SetHapticNavigation(on);
        break;
      case 3:
        settingsController.SetShakeToSilence(on);
        break;
      case 4:
        settingsController.SetPhoneLostAlert(on);
        break;
    }
    return;
  }
}
