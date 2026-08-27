#include "displayapp/screens/settings/SettingTiltParallax.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingTiltParallax* screen = static_cast<SettingTiltParallax*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

  void SetStrengthLabel(lv_obj_t* label, uint8_t strength) {
    if (strength == 0) {
      lv_label_set_text_static(label, "Off");
    } else {
      lv_label_set_text_fmt(label, "%u", strength);
    }
  }
}

SettingTiltParallax::SettingTiltParallax(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Tilt parallax");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::tachometer);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* subtitle = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(subtitle, "Strength (0=off, 140=max)");
  lv_label_set_align(subtitle, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  strengthValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(strengthValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  SetStrengthLabel(strengthValue, settingsController.GetTiltParallaxStrength());
  lv_label_set_align(strengthValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(strengthValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

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
}

SettingTiltParallax::~SettingTiltParallax() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingTiltParallax::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  int value = settingsController.GetTiltParallaxStrength();

  int valueChange = 0;
  if (event == LV_EVENT_SHORT_CLICKED) {
    valueChange = 1;
  } else if (event == LV_EVENT_LONG_PRESSED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
    valueChange = 5;
  } else {
    return;
  }

  if (object == btnPlus) {
    value += valueChange;
  } else if (object == btnMinus) {
    value -= valueChange;
  }

  if (value >= 0 && value <= 140) {
    settingsController.SetTiltParallaxStrength(static_cast<uint8_t>(value));
    SetStrengthLabel(strengthValue, settingsController.GetTiltParallaxStrength());
    lv_obj_realign(strengthValue);
  }
}
