#include "displayapp/screens/WorldClock.h"
#include <lvgl/lvgl.h>
#include <chrono>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void GetHourMinute(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> cityTime, int& hour, int& minute) {
    auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(cityTime.time_since_epoch()).count();
    long secondsToday = secondsSinceEpoch % 86400;
    if (secondsToday < 0) {
      secondsToday += 86400;
    }
    hour = static_cast<int>(secondsToday / 3600);
    minute = static_cast<int>((secondsToday % 3600) / 60);
  }
}

WorldClock::WorldClock(Controllers::DateTime& dateTimeController) : dateTimeController {dateTimeController} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "World Clock");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  lv_coord_t rowY = 50;
  for (size_t i = 0; i < cities.size(); i++) {
    lv_obj_t* nameLabel = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(nameLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(nameLabel, cities[i].name);
    lv_obj_align(nameLabel, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, rowY);

    valueLabels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(valueLabels[i], "--:--");
    lv_obj_align(valueLabels[i], lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -10, rowY);

    rowY += 40;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
  Refresh();
}

WorldClock::~WorldClock() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WorldClock::Refresh() {
  for (size_t i = 0; i < cities.size(); i++) {
    auto cityTime = dateTimeController.UTCDateTime() + std::chrono::hours(cities[i].utcOffsetHours);
    int hour = 0;
    int minute = 0;
    GetHourMinute(cityTime, hour, minute);
    lv_label_set_text_fmt(valueLabels[i], "%02d:%02d", hour, minute);
  }
}
