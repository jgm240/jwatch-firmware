#include "displayapp/screens/KindleClick.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<KindleClick*>(obj->user_data)->Press(obj);
    }
  }
}

KindleClick::KindleClick(Controllers::NimbleController& nimble, const Controllers::Ble& bleController, System::SystemTask& systemTask)
  : nimble {nimble}, bleController {bleController}, wakeLock(systemTask) {

  statusLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(statusLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(statusLabel, "Kindle Click");
  lv_obj_align(statusLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 6);

  hintLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(hintLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_align(hintLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(hintLabel, "Pair from the Kindle");
  lv_obj_align(hintLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);

  struct Spec {
    const char* text;
    lv_coord_t x, y, w, h;
  };
  const Spec specs[5] = {
    {"< Back", 0, 58, 118, 126},
    {"Next >", 122, 58, 118, 126},
    {"Dim", 0, 192, 76, 46},
    {"Sleep", 82, 192, 76, 46},
    {"Bright", 164, 192, 76, 46},
  };
  for (uint8_t i = 0; i < 5; i++) {
    buttons[i] = lv_btn_create(lv_scr_act(), nullptr);
    buttons[i]->user_data = this;
    lv_obj_set_event_cb(buttons[i], ButtonHandler);
    lv_obj_set_size(buttons[i], specs[i].w, specs[i].h);
    lv_obj_set_pos(buttons[i], specs[i].x, specs[i].y);
    lv_label_set_text_static(lv_label_create(buttons[i], nullptr), specs[i].text);
  }
  lv_obj_set_style_local_bg_color(buttons[1], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);

  // Stay awake while the app is open: closing it (screen timeout included) hands the link back to the phone.
  wakeLock.Lock();
  nimble.EnterKindleMode();

  taskRefresh = lv_task_create(RefreshTaskCallback, 50, LV_TASK_PRIO_MID, this);
}

KindleClick::~KindleClick() {
  lv_task_del(taskRefresh);
  nimble.ExitKindleMode();
  wakeLock.Release();
  lv_obj_clean(lv_scr_act());
}

void KindleClick::Press(lv_obj_t* button) {
  Release(); // never leave a previous key down
  auto& hid = nimble.hid();
  if (button == buttons[static_cast<uint8_t>(Key::Back)]) {
    hid.SendKeyboardReport(Controllers::HidService::keyPageUp);
    pressedConsumer = false;
  } else if (button == buttons[static_cast<uint8_t>(Key::Forward)]) {
    hid.SendKeyboardReport(Controllers::HidService::keyPageDown);
    pressedConsumer = false;
  } else if (button == buttons[static_cast<uint8_t>(Key::BrightnessDown)]) {
    hid.SendKindleConsumerReport(Controllers::HidService::kindleBrightnessDown);
    pressedConsumer = true;
  } else if (button == buttons[static_cast<uint8_t>(Key::BrightnessUp)]) {
    hid.SendKindleConsumerReport(Controllers::HidService::kindleBrightnessUp);
    pressedConsumer = true;
  } else if (button == buttons[static_cast<uint8_t>(Key::Sleep)]) {
    hid.SendKindleConsumerReport(Controllers::HidService::kindlePower);
    pressedConsumer = true;
  } else {
    return;
  }
  releasePending = true;
}

void KindleClick::Release() {
  if (!releasePending) {
    return;
  }
  releasePending = false;
  if (pressedConsumer) {
    nimble.hid().SendKindleConsumerReport(0);
  } else {
    nimble.hid().SendKeyboardReport(0);
  }
}

void KindleClick::Refresh() {
  Release(); // key-up one tick (50 ms) after key-down
  if (++ticks < 10) {
    return;
  }
  ticks = 0;
  if (bleController.IsConnected()) {
    lv_label_set_text_static(statusLabel, "Connected");
    lv_label_set_text_static(hintLabel, "Tap to turn pages");
  } else {
    lv_label_set_text_static(statusLabel, "Waiting for Kindle");
    lv_label_set_text_static(hintLabel, "Pair from the Kindle");
  }
  lv_obj_realign(statusLabel);
  lv_obj_realign(hintLabel);
}
