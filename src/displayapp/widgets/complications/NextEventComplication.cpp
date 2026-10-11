#include "displayapp/widgets/complications/NextEventComplication.h"
#include <chrono>

using namespace Pinetime::Applications::Widgets;

NextEventComplication::NextEventComplication(const Controllers::CalendarEventService& calendarEventService,
                                             Controllers::DateTime& dateTimeController)
  : calendarEventService {calendarEventService}, dateTimeController {dateTimeController} {
}

void NextEventComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(label, 110);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void NextEventComplication::Update() {
  // CurrentDateTime() is wall-clock time encoded as an epoch, so compare in that domain.
  const auto now = std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();

  int found = -1;
  for (uint8_t i = 0; i < calendarEventService.NbEvents(); i++) {
    const auto& event = calendarEventService.GetEvent(i);
    const uint64_t end = event.end > event.start ? event.end : event.start;
    if (dateTimeController.UtcEpochToLocalEpoch(end) >= now) {
      found = i;
      break;
    }
  }

  // Only touch the label when the shown event actually changes - Update() runs every refresh.
  uint64_t key = 0;
  if (found >= 0) {
    const auto& event = calendarEventService.GetEvent(static_cast<uint8_t>(found));
    key = event.start + 1;
    for (const char* c = event.title.data(); *c != '\0'; c++) {
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
    const auto& event = calendarEventService.GetEvent(static_cast<uint8_t>(found));
    const std::tm start = dateTimeController.ToLocalTime(dateTimeController.UtcEpochToLocalEpoch(event.start));
    lv_label_set_text_fmt(label, "%02d:%02d %s", start.tm_hour, start.tm_min, event.title.data());
  }
  lv_obj_realign(label);
}
