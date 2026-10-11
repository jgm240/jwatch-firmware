#include "displayapp/widgets/complications/CountdownComplication.h"
#include <chrono>

using namespace Pinetime::Applications::Widgets;

CountdownComplication::CountdownComplication(const Controllers::CalendarEventService& calendarEventService,
                                             Controllers::DateTime& dateTimeController)
  : calendarEventService {calendarEventService}, dateTimeController {dateTimeController} {
}

void CountdownComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void CountdownComplication::Update() {
  const int64_t nowLocal =
    std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();
  const int64_t today = nowLocal / 86400;

  int found = -1;
  int64_t days = 0;
  for (uint8_t i = 0; i < calendarEventService.NbEvents(); i++) {
    const auto& event = calendarEventService.GetEvent(i);
    if (event.title[0] != '>' || event.title[1] != ' ') {
      continue;
    }
    const int64_t eventDay = static_cast<int64_t>(dateTimeController.UtcEpochToLocalEpoch(event.start)) / 86400;
    if (eventDay >= today) {
      found = i;
      days = eventDay - today;
      break;
    }
  }

  uint64_t key = 0;
  if (found >= 0) {
    key = static_cast<uint64_t>(days) + 1;
    for (const char* c = calendarEventService.GetEvent(static_cast<uint8_t>(found)).title.data(); *c != '\0'; c++) {
      key = key * 31 + static_cast<uint8_t>(*c);
    }
  }
  if (key == shownKey) {
    return;
  }
  shownKey = key;

  if (found < 0) {
    lv_label_set_text_static(label, "");
  } else {
    const char* name = calendarEventService.GetEvent(static_cast<uint8_t>(found)).title.data() + 2;
    if (days == 0) {
      lv_label_set_text_fmt(label, "%.8s today", name);
    } else {
      lv_label_set_text_fmt(label, "%.8s %dd", name, static_cast<int>(days));
    }
  }
  lv_obj_realign(label);
}
