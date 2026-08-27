#include "displayapp/screens/settings/SettingSpiNor.h"
#include <hal/nrf_rtc.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    static_cast<SettingSpiNor*>(obj->user_data)->OnEvent(obj);
  }
}

SettingSpiNor::SettingSpiNor(DisplayApp* app, Controllers::FS& filesystem) : app {app}, filesystem {filesystem} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "SPI-NOR");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 15);

  infoLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(infoLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(infoLabel, 200);
  lv_label_set_align(infoLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_style_local_text_color(infoLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(infoLabel, "External flash storage (settings, BLE pairings, files)");
  lv_obj_align(infoLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 45);

  eraseBtn = Styles::Button(lv_scr_act());
  eraseBtn->user_data = this;
  lv_obj_set_event_cb(eraseBtn, eventHandler);
  lv_obj_set_size(eraseBtn, 200, 50);
  lv_obj_align(eraseBtn, lv_scr_act(), LV_ALIGN_CENTER, 0, 30);
  lv_obj_t* eraseBtnLabel = lv_label_create(eraseBtn, nullptr);
  lv_label_set_text_static(eraseBtnLabel, "Erase safe SPI-NOR area");

  eraseLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(eraseLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(eraseLabel, 200);
  lv_label_set_align(eraseLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(eraseLabel,
                           "Erase all data?\nSettings, pairings and files "
                           "are wiped. Firmware is kept.\nThis cannot be undone.");
  lv_obj_align(eraseLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);

  cancelBtn = Styles::Button(lv_scr_act());
  cancelBtn->user_data = this;
  lv_obj_set_event_cb(cancelBtn, eventHandler);
  lv_obj_set_size(cancelBtn, 100, 50);
  lv_obj_align(cancelBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -10);
  lv_obj_t* cancelLabel = lv_label_create(cancelBtn, nullptr);
  lv_label_set_text_static(cancelLabel, "Cancel");

  confirmBtn = Styles::Button(lv_scr_act());
  confirmBtn->user_data = this;
  lv_obj_set_event_cb(confirmBtn, eventHandler);
  lv_obj_set_size(confirmBtn, 100, 50);
  lv_obj_align(confirmBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -10);
  lv_obj_set_style_local_bg_color(confirmBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_t* confirmLabel = lv_label_create(confirmBtn, nullptr);
  lv_label_set_text_static(confirmLabel, "Erase");

  ShowMenu();
}

SettingSpiNor::~SettingSpiNor() {
  lv_obj_clean(lv_scr_act());
}

void SettingSpiNor::ShowMenu() {
  confirming = false;
  lv_obj_set_hidden(infoLabel, false);
  lv_obj_set_hidden(eraseBtn, false);

  lv_obj_set_hidden(eraseLabel, true);
  lv_obj_set_hidden(cancelBtn, true);
  lv_obj_set_hidden(confirmBtn, true);
}

void SettingSpiNor::ShowConfirm() {
  confirming = true;
  lv_obj_set_hidden(infoLabel, true);
  lv_obj_set_hidden(eraseBtn, true);

  lv_obj_set_hidden(eraseLabel, false);
  lv_obj_set_hidden(cancelBtn, false);
  lv_obj_set_hidden(confirmBtn, false);
}

void SettingSpiNor::OnEvent(lv_obj_t* target) {
  if (target == eraseBtn) {
    ShowConfirm();
  } else if (target == cancelBtn) {
    ShowMenu();
  } else if (target == confirmBtn) {
    filesystem.Format();
    NVIC_SystemReset();
  }
}
