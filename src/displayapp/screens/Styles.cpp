#include "Styles.h"
#include "displayapp/InfiniTimeTheme.h"

void Pinetime::Applications::Screens::SetRadioButtonStyle(lv_obj_t* checkbox) {
  lv_obj_set_style_local_radius(checkbox, LV_CHECKBOX_PART_BULLET, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(checkbox, LV_CHECKBOX_PART_BULLET, LV_STATE_CHECKED, 9);
  lv_obj_set_style_local_border_color(checkbox, LV_CHECKBOX_PART_BULLET, LV_STATE_CHECKED, Colors::highlight);
  lv_obj_set_style_local_bg_color(checkbox, LV_CHECKBOX_PART_BULLET, LV_STATE_CHECKED, LV_COLOR_WHITE);
}

namespace {
  uint8_t buttonRadius = 20;
  bool glassEnabled = false;
}

void Pinetime::Applications::Screens::Styles::SetButtonRadius(uint8_t radius) {
  buttonRadius = radius;
}

uint8_t Pinetime::Applications::Screens::Styles::GetButtonRadius() {
  return buttonRadius;
}

void Pinetime::Applications::Screens::Styles::SetGlassEnabled(bool enabled) {
  glassEnabled = enabled;
}

bool Pinetime::Applications::Screens::Styles::IsGlassEnabled() {
  return glassEnabled;
}

lv_obj_t* Pinetime::Applications::Screens::Styles::Button(lv_obj_t* parent) {
  lv_obj_t* btn = lv_btn_create(parent, nullptr);
  lv_obj_set_style_local_radius(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, buttonRadius);

  if (glassEnabled) {
    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
    lv_obj_set_style_local_bg_grad_dir(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_obj_set_style_local_bg_grad_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_grad_stop(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 90);
    lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
  }

  return btn;
}
