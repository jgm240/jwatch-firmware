#include "displayapp/screens/settings/SettingSmartStack.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
      static_cast<SmartStackTogglePage*>(obj->user_data)->Toggled(obj);
    }
  }
}

SmartStackTogglePage::SmartStackTogglePage(uint8_t firstCard, Controllers::Settings& settingsController)
  : firstCard {firstCard}, settingsController {settingsController} {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container, 10, 35);
  lv_obj_set_width(container, LV_HOR_RES - 20);
  lv_obj_set_height(container, LV_VER_RES - 45);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Smart Stack");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 8);

  count = 0;
  for (uint8_t i = 0; i < perPage && (firstCard + i) < smartStackCardCount; i++) {
    checkboxes[i] = lv_checkbox_create(container, nullptr);
    lv_checkbox_set_text(checkboxes[i], smartStackCardNames[firstCard + i]);
    lv_checkbox_set_checked(checkboxes[i], settingsController.GetSmartStackCardEnabled(firstCard + i));
    checkboxes[i]->user_data = this;
    lv_obj_set_event_cb(checkboxes[i], EventHandler);
    count++;
  }
}

SmartStackTogglePage::~SmartStackTogglePage() {
  lv_obj_clean(lv_scr_act());
}

void SmartStackTogglePage::Toggled(lv_obj_t* object) {
  for (uint8_t i = 0; i < count; i++) {
    if (checkboxes[i] == object) {
      settingsController.SetSmartStackCardEnabled(firstCard + i, lv_checkbox_is_checked(object));
      return;
    }
  }
}

auto SettingSmartStack::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return std::make_unique<SmartStackTogglePage>(static_cast<uint8_t>(i * SmartStackTogglePage::perPage), settingsController);
    };
  }
  return screenList;
}

SettingSmartStack::SettingSmartStack(DisplayApp* app, Controllers::Settings& settingsController)
  : settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingSmartStack::~SettingSmartStack() {
  settingsController.SaveSettings();
  lv_obj_clean(lv_scr_act());
}

bool SettingSmartStack::OnTouchEvent(TouchEvents event) {
  return screens.OnTouchEvent(event);
}
