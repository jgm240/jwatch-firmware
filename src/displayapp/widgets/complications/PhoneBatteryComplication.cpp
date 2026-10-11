#include "displayapp/widgets/complications/PhoneBatteryComplication.h"

using namespace Pinetime::Applications::Widgets;

PhoneBatteryComplication::PhoneBatteryComplication(const Controllers::CalendarEventService& calendarEventService)
  : calendarEventService {calendarEventService} {
}

void PhoneBatteryComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void PhoneBatteryComplication::Update() {
  const uint8_t percent = calendarEventService.PhoneBatteryPercent();
  const bool charging = calendarEventService.PhoneCharging();
  const int32_t key = percent == Controllers::CalendarEventService::PhoneBatteryUnknown ? 0 : (percent + 1) * 2 + (charging ? 1 : 0);
  if (key == shownKey) {
    return;
  }
  shownKey = key;

  if (key == 0) {
    lv_label_set_text_static(label, "");
  } else {
    lv_label_set_text_fmt(label, "Ph %d%%%s", percent, charging ? "+" : "");
  }
  lv_obj_realign(label);
}
