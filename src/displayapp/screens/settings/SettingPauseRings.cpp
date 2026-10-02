#include "displayapp/screens/settings/SettingPauseRings.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingPauseRings* screen = static_cast<SettingPauseRings*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingPauseRings::SettingPauseRings(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Pause hexagons");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::sleep);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* subtitle = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(subtitle, "Days to pause (0=off, max 90)");
  lv_label_set_align(subtitle, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  daysValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(daysValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_align(daysValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(daysValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -35);

  statusLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(statusLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(statusLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;

  btnPlus = Styles::Button(lv_scr_act());
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, btnWidth, btnHeight);
  lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_color(btnPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
  lv_obj_set_style_local_text_font(lblPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblPlus, "+");
  lv_obj_set_event_cb(btnPlus, event_handler);

  btnMinus = Styles::Button(lv_scr_act());
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, btnWidth, btnHeight);
  lv_obj_set_event_cb(btnMinus, event_handler);
  lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
  lv_obj_set_style_local_text_font(lblMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblMinus, "-");

  Redraw();
}

SettingPauseRings::~SettingPauseRings() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingPauseRings::Redraw() {
  uint32_t days = settingsController.GetPausedDaysRemaining();
  lv_label_set_text_fmt(daysValue, "%lu", days);
  lv_obj_realign(daysValue);

  if (days > 0) {
    lv_label_set_text_static(statusLabel, "Hexagons paused");
  } else {
    lv_label_set_text_static(statusLabel, "Not paused");
  }
  lv_obj_realign(statusLabel);
}

void SettingPauseRings::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  int32_t valueChange = 0;
  if (event == LV_EVENT_SHORT_CLICKED) {
    valueChange = 1;
  } else if (event == LV_EVENT_LONG_PRESSED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
    valueChange = 5;
  } else {
    return;
  }

  if (object == btnMinus) {
    valueChange = -valueChange;
  } else if (object != btnPlus) {
    return;
  }

  int32_t newValue = static_cast<int32_t>(settingsController.GetPausedDaysRemaining()) + valueChange;
  if (newValue >= 0 && newValue <= 90) {
    settingsController.SetPausedDaysRemaining(static_cast<uint32_t>(newValue));
    Redraw();
  }
}
