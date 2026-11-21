#include "displayapp/screens/School.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_coord_t CardMargin = 8;
  constexpr lv_coord_t CardHeight = 38;
  constexpr lv_coord_t CardGap = 4;
  constexpr lv_coord_t FirstCardY = 28;

  // event.title already carries its own type prefix - see watch_ble.py (SSC) / the equivalent on
  // phone companions - so no extra byte of wire format is needed to tell lessons, exams and
  // homework apart. Rather than show that prefix as text (the original design), it now picks
  // both an accent color and the text to display from it - closer to how SSC's own desktop
  // dashboard (parent.html) tells entries apart: by a colored badge/border, not inline text.
  lv_color_t AccentColor(const char* title) {
    if (std::strncmp(title, "Exam:", 5) == 0) {
      return Colors::deepOrange;
    }
    if (std::strncmp(title, "HW:", 3) == 0) {
      return Colors::orange;
    }
    return Colors::blue;
  }

  // Returns a pointer into 'title' past its "Exam: "/"HW: " prefix and any following space, or
  // 'title' itself if it has neither - the color/border already says what kind of event this
  // is, so the label text doesn't need to repeat it.
  const char* StrippedTitle(const char* title) {
    const char* rest = nullptr;
    if (std::strncmp(title, "Exam:", 5) == 0) {
      rest = title + 5;
    } else if (std::strncmp(title, "HW:", 3) == 0) {
      rest = title + 3;
    } else {
      return title;
    }
    while (*rest == ' ') {
      rest++;
    }
    return rest;
  }
}

School::School(Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController)
  : calendarEventService {calendarEventService}, dateTimeController {dateTimeController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "School");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 6);

  noEventsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(noEventsLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(noEventsLabel, "Nothing coming up");
  lv_obj_align(noEventsLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(noEventsLabel, true);

  for (size_t i = 0; i < MaxDisplayedEvents; i++) {
    lv_coord_t cardY = FirstCardY + static_cast<lv_coord_t>(i) * (CardHeight + CardGap);

    cards[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(cards[i], LV_HOR_RES - 2 * CardMargin, CardHeight);
    lv_obj_align(cards[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, CardMargin, cardY);
    lv_obj_set_style_local_radius(cards[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_bg_color(cards[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_border_side(cards[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT);
    lv_obj_set_style_local_border_width(cards[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 4);

    titleLabels[i] = lv_label_create(cards[i], nullptr);
    lv_label_set_long_mode(titleLabels[i], LV_LABEL_LONG_DOT);
    lv_obj_set_width(titleLabels[i], LV_HOR_RES - 2 * CardMargin - 18);
    lv_label_set_text_static(titleLabels[i], "");
    lv_obj_align(titleLabels[i], cards[i], LV_ALIGN_IN_TOP_LEFT, 10, 3);

    timeLabels[i] = lv_label_create(cards[i], nullptr);
    lv_obj_set_style_local_text_color(timeLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(timeLabels[i], "");
    lv_obj_align(timeLabels[i], cards[i], LV_ALIGN_IN_BOTTOM_LEFT, 10, -3);

    lv_obj_set_hidden(cards[i], true);
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
    lv_obj_set_hidden(cards[i], !visible);
    if (!visible) {
      continue;
    }

    const auto& event = calendarEventService.GetEvent(i);
    lv_label_set_text_fmt(titleLabels[i], "%s", StrippedTitle(event.title.data()));
    lv_obj_set_style_local_border_color(cards[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, AccentColor(event.title.data()));

    std::tm eventTime = dateTimeController.ToLocalTime(dateTimeController.UtcEpochToLocalEpoch(event.start));

    // Events span up to a few days (see CalendarEventService::MaxEvents and watch_ble.py's window),
    // so "today" alone isn't enough context - add a day word once the event isn't today.
    int dayDelta = eventTime.tm_yday - nowTime.tm_yday;
    if (dayDelta < 0) {
      dayDelta += 365; // year wrap: close enough for a cosmetic label
    }
    if (dayDelta == 0) {
      lv_label_set_text_fmt(timeLabels[i], "%02d:%02d", eventTime.tm_hour, eventTime.tm_min);
    } else if (dayDelta == 1) {
      lv_label_set_text_fmt(timeLabels[i], "Tmrw %02d:%02d", eventTime.tm_hour, eventTime.tm_min);
    } else {
      auto weekday = static_cast<Controllers::DateTime::Days>(eventTime.tm_wday == 0 ? 7 : eventTime.tm_wday);
      lv_label_set_text_fmt(
        timeLabels[i], "%s %02d:%02d", Controllers::DateTime::DayOfWeekShortToStringLow(weekday), eventTime.tm_hour, eventTime.tm_min);
    }
  }
}
