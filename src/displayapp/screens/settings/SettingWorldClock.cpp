#include "displayapp/screens/settings/SettingWorldClock.h"
#include <cstring>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void Handler(lv_obj_t* object, lv_event_t event) {
    static_cast<SettingWorldClock*>(object->user_data)->OnEvent(object, event);
  }
}

SettingWorldClock::SettingWorldClock(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  options[0] = '\0';
  for (size_t i = 0; i < Pinetime::Controllers::worldCityCount; i++) {
    if (i > 0) {
      std::strcat(options, "\n");
    }
    std::strcat(options, Pinetime::Controllers::worldCities[i].name);
  }

  prevButton = lv_btn_create(lv_scr_act(), nullptr);
  prevButton->user_data = this;
  lv_obj_set_event_cb(prevButton, Handler);
  lv_obj_set_size(prevButton, 54, 40);
  lv_obj_set_pos(prevButton, 0, 0);
  lv_label_set_text_static(lv_label_create(prevButton, nullptr), "<");

  nextButton = lv_btn_create(lv_scr_act(), nullptr);
  nextButton->user_data = this;
  lv_obj_set_event_cb(nextButton, Handler);
  lv_obj_set_size(nextButton, 54, 40);
  lv_obj_set_pos(nextButton, 186, 0);
  lv_label_set_text_static(lv_label_create(nextButton, nullptr), ">");

  slotLabel = lv_label_create(lv_scr_act(), nullptr);

  roller = lv_roller_create(lv_scr_act(), nullptr);
  roller->user_data = this;
  lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller, 5);
  lv_obj_set_width(roller, 220);
  lv_obj_align(roller, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -6);
  lv_obj_set_event_cb(roller, Handler);

  ShowSlot();
}

SettingWorldClock::~SettingWorldClock() {
  lv_obj_clean(lv_scr_act());
}

void SettingWorldClock::ShowSlot() {
  lv_label_set_text_fmt(slotLabel, "City %d", slot + 1);
  lv_obj_align(slotLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
  lv_roller_set_selected(roller, settingsController.GetWorldClockCity(slot) % Pinetime::Controllers::worldCityCount, LV_ANIM_OFF);
}

void SettingWorldClock::OnEvent(lv_obj_t* object, lv_event_t event) {
  if (object == roller && event == LV_EVENT_VALUE_CHANGED) {
    settingsController.SetWorldClockCity(slot, static_cast<uint8_t>(lv_roller_get_selected(roller)));
    settingsController.SaveSettings();
  } else if (event == LV_EVENT_CLICKED && object == prevButton) {
    slot = (slot + Pinetime::Controllers::worldClockSlots - 1) % Pinetime::Controllers::worldClockSlots;
    ShowSlot();
  } else if (event == LV_EVENT_CLICKED && object == nextButton) {
    slot = (slot + 1) % Pinetime::Controllers::worldClockSlots;
    ShowSlot();
  }
}
