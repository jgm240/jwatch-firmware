#include "displayapp/screens/School.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // event.title already carries its own type prefix - see watch_ble.py (SSC) / the equivalent on
  // phone companions - so no extra byte of wire format is needed to tell lessons, exams and
  // homework apart; this just picks a color from the prefix text itself.
  lv_color_t TitleColor(const char* title) {
    if (std::strncmp(title, "Exam:", 5) == 0) {
      return Colors::orange;
    }
    if (std::strncmp(title, "HW:", 3) == 0) {
      return Colors::blue;
    }
    return LV_COLOR_WHITE;
  }
}

School::School(Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController)
  : calendarEventService {calendarEventService}, dateTimeController {dateTimeController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "School");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  noEventsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(noEventsLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(noEventsLabel, "Nothing coming up");
  lv_obj_align(noEventsLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(noEventsLabel, true);

  lv_coord_t rowY = 45;
  for (size_t i = 0; i < MaxDisplayedEvents; i++) {
    timeLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(timeLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(timeLabels[i], "");
    lv_obj_align(timeLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, rowY);

    titleLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_long_mode(titleLabels[i], LV_LABEL_LONG_DOT);
    lv_obj_set_width(titleLabels[i], 165);
    lv_label_set_text_static(titleLabels[i], "");
    lv_obj_align(titleLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 55, rowY);

    lv_obj_set_hidden(timeLabels[i], true);
    lv_obj_set_hidden(titleLabels[i], true);

    rowY += 35;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, 2000, LV_TASK_PRIO_MID, this);
  Refresh();
}

School::~School() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void School::Refresh() {
  uint8_t count = std::min(calendarEventService.NbEvents(), MaxDisplayedEvents);

  lv_obj_set_hidden(noEventsLabel, count != 0);

  std::tm nowTime = dateTimeController.ToLocalTime(std::chrono::system_clock::to_time_t(dateTimeController.CurrentDateTime()));

  for (uint8_t i = 0; i < MaxDisplayedEvents; i++) {
    bool visible = i < count;
    lv_obj_set_hidden(timeLabels[i], !visible);
    lv_obj_set_hidden(titleLabels[i], !visible);
    if (!visible) {
      continue;
    }

    const auto& event = calendarEventService.GetEvent(i);
    std::tm eventTime = dateTimeController.ToLocalTime(dateTimeController.UtcEpochToLocalEpoch(event.start));
    lv_label_set_text_fmt(timeLabels[i], "%02d:%02d", eventTime.tm_hour, eventTime.tm_min);

    // Events span up to a few days (see CalendarEventService::MaxEvents and watch_ble.py's window),
    // so "today" alone isn't enough context - add a day word once the event isn't today.
    int dayDelta = eventTime.tm_yday - nowTime.tm_yday;
    if (dayDelta < 0) {
      dayDelta += 365; // year wrap: close enough for a cosmetic label
    }
    if (dayDelta == 0) {
      lv_label_set_text_fmt(titleLabels[i], "%s", event.title.data());
    } else if (dayDelta == 1) {
      lv_label_set_text_fmt(titleLabels[i], "Tmrw %s", event.title.data());
    } else {
      auto weekday = static_cast<Controllers::DateTime::Days>(eventTime.tm_wday == 0 ? 7 : eventTime.tm_wday);
      lv_label_set_text_fmt(titleLabels[i], "%s %s", Controllers::DateTime::DayOfWeekShortToStringLow(weekday), event.title.data());
    }
    lv_obj_set_style_local_text_color(titleLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, TitleColor(event.title.data()));
  }
}
