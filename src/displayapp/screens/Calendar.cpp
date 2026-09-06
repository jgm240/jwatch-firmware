#include "displayapp/screens/Calendar.h"
#include <algorithm>
#include <lvgl/lvgl.h>
#include <cstring>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Calendar::Calendar(Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController)
  : calendarEventService {calendarEventService}, dateTimeController {dateTimeController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Calendar");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  noEventsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(noEventsLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(noEventsLabel, "No upcoming events");
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

Calendar::~Calendar() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Calendar::Refresh() {
  uint8_t count = std::min(calendarEventService.NbEvents(), MaxDisplayedEvents);
  if (count == lastDisplayedCount) {
    return;
  }
  lastDisplayedCount = count;

  lv_obj_set_hidden(noEventsLabel, count != 0);

  for (uint8_t i = 0; i < MaxDisplayedEvents; i++) {
    bool visible = i < count;
    lv_obj_set_hidden(timeLabels[i], !visible);
    lv_obj_set_hidden(titleLabels[i], !visible);
    if (!visible) {
      continue;
    }

    const auto& event = calendarEventService.GetEvent(i);
    std::tm eventTime = dateTimeController.ToLocalTime(static_cast<std::time_t>(event.start));
    lv_label_set_text_fmt(timeLabels[i], "%02d:%02d", eventTime.tm_hour, eventTime.tm_min);
    lv_label_set_text_fmt(titleLabels[i], "%s", event.title.data());
  }
}
