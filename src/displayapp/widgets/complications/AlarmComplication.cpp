#include "displayapp/widgets/complications/AlarmComplication.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

AlarmComplication::AlarmComplication(const Controllers::AlarmController& alarmController) : alarmController {alarmController} {
}

void AlarmComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void AlarmComplication::Update() {
  alarmEnabled = alarmController.IsEnabled();
  alarmHours = alarmController.Hours();
  alarmMinutes = alarmController.Minutes();
  if (alarmEnabled.IsUpdated() || alarmHours.IsUpdated() || alarmMinutes.IsUpdated()) {
    if (alarmEnabled.Get()) {
      lv_label_set_text_fmt(label, "%s %02d:%02d", Screens::Symbols::bell, alarmHours.Get(), alarmMinutes.Get());
    } else {
      lv_label_set_text_static(label, "");
    }
    lv_obj_realign(label);
  }
}
