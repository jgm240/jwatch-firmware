#include "displayapp/screens/Camera.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<Camera*>(obj->user_data)->Shutter();
    }
  }
}

Camera::Camera(Controllers::HidService& hidService) : hidService {hidService} {
  lv_obj_t* hint = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(hint, "Open Camera on\nyour phone");
  lv_label_set_align(hint, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(hint, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  button = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(button, 140, 140);
  lv_obj_set_style_local_radius(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_bg_color(button, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_SILVER);
  lv_obj_align(button, lv_scr_act(), LV_ALIGN_CENTER, 0, 25);
  button->user_data = this;
  lv_obj_set_event_cb(button, EventHandler);
}

Camera::~Camera() {
  lv_obj_clean(lv_scr_act());
}

void Camera::Shutter() {
  hidService.SendConsumerReport(Controllers::HidService::VolumeUp);
  hidService.SendConsumerReport(0);
}
