#include "displayapp/screens/WorldClock.h"
#include <lvgl/lvgl.h>
#include <chrono>
#include <cstdio>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

WorldClock::WorldClock(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController)
  : dateTimeController {dateTimeController}, settingsController {settingsController} {
  lv_coord_t rowY = 6;
  for (size_t i = 0; i < Controllers::worldClockSlots; i++) {
    nameLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(nameLabels[i], "");
    lv_obj_align(nameLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, rowY);

    timeLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(timeLabels[i], "--:--");
    lv_obj_align(timeLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -10, rowY);

    detailLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(detailLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(detailLabels[i], "");
    lv_obj_align(detailLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, rowY + 22);

    rowY += 46;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
  Refresh();
}

WorldClock::~WorldClock() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WorldClock::Refresh() {
  const int64_t utc = std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.UTCDateTime().time_since_epoch()).count();
  const int64_t watchLocal = std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();
  const bool h12 = settingsController.GetClockType() == Controllers::Settings::ClockType::H12;

  for (size_t i = 0; i < Controllers::worldClockSlots; i++) {
    const auto& city = Controllers::worldCities[settingsController.GetWorldClockCity(i) % Controllers::worldCityCount];
    const int offset = Controllers::WorldOffsetMinutes(city, utc);
    const int64_t cityLocal = utc + static_cast<int64_t>(offset) * 60;
    const int64_t secondsToday = cityLocal - Controllers::DayNumber(cityLocal) * 86400;
    const int hour = static_cast<int>(secondsToday / 3600);
    const int minute = static_cast<int>((secondsToday % 3600) / 60);
    const int dayDelta = static_cast<int>(Controllers::DayNumber(cityLocal) - Controllers::DayNumber(watchLocal));

    lv_label_set_text(nameLabels[i], city.name);
    if (h12) {
      lv_label_set_text_fmt(timeLabels[i], "%d:%02d%c", hour % 12 == 0 ? 12 : hour % 12, minute, hour < 12 ? 'a' : 'p');
    } else {
      lv_label_set_text_fmt(timeLabels[i], "%02d:%02d", hour, minute);
    }
    lv_obj_align(timeLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -10, 6 + static_cast<lv_coord_t>(i) * 46);

    const int absOffset = offset < 0 ? -offset : offset;
    char zone[12];
    if (absOffset % 60 == 0) {
      std::snprintf(zone, sizeof(zone), "UTC%c%d", offset < 0 ? '-' : '+', absOffset / 60);
    } else {
      std::snprintf(zone, sizeof(zone), "UTC%c%d:%02d", offset < 0 ? '-' : '+', absOffset / 60, absOffset % 60);
    }
    lv_label_set_text_fmt(detailLabels[i], "%s%s", zone, dayDelta > 0 ? " tomorrow" : (dayDelta < 0 ? " yesterday" : ""));
  }
}
