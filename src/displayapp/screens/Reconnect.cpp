#include "displayapp/screens/Reconnect.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<Reconnect*>(obj->user_data)->Announce();
    }
  }
}

Reconnect::Reconnect(Controllers::NimbleController& nimble, const Controllers::Ble& bleController, System::SystemTask& systemTask)
  : nimble {nimble}, bleController {bleController}, wakeLock(systemTask) {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Reconnect");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 12);

  statusLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(statusLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(statusLabel, "Announcing...");
  lv_obj_align(statusLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  hintLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(hintLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_align(hintLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(hintLabel, "Connect from your\nphone now");
  lv_obj_align(hintLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 5);

  button = lv_btn_create(lv_scr_act(), nullptr);
  button->user_data = this;
  lv_obj_set_size(button, 200, 60);
  lv_obj_align(button, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_event_cb(button, ButtonHandler);
  buttonLabel = lv_label_create(button, nullptr);
  lv_label_set_text_static(buttonLabel, "Announce again");

  // Stay awake and visible while the screen is open.
  wakeLock.Lock();
  nimble.HoldFastAdvertising(true);
  nimble.MakeDiscoverable();

  taskRefresh = lv_task_create(RefreshTaskCallback, 500, LV_TASK_PRIO_MID, this);
}

Reconnect::~Reconnect() {
  lv_task_del(taskRefresh);
  nimble.HoldFastAdvertising(false);
  wakeLock.Release();
  lv_obj_clean(lv_scr_act());
}

void Reconnect::Announce() {
  nimble.MakeDiscoverable();
}

void Reconnect::Refresh() {
  if (bleController.IsConnected()) {
    lv_obj_set_style_local_text_color(statusLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::green);
    lv_label_set_text_static(statusLabel, "Connected");
    lv_label_set_text_static(hintLabel, "Phone is linked");
  } else {
    lv_obj_set_style_local_text_color(statusLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
    lv_label_set_text_static(statusLabel, "Announcing...");
    lv_label_set_text_static(hintLabel, "Connect from your\nphone now");
  }
  lv_obj_realign(statusLabel);
  lv_obj_realign(hintLabel);
}
