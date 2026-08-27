#include "displayapp/screens/ActivityRings.h"
#include <lvgl/lvgl.h>
#include <algorithm>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  lv_obj_t* CreateRing(lv_coord_t size, lv_color_t color) {
    lv_obj_t* arc = lv_arc_create(lv_scr_act(), nullptr);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 0);
    lv_arc_set_adjustable(arc, false);
    lv_obj_set_size(arc, size, size);
    lv_obj_align(arc, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, color);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 12);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 12);
    lv_obj_set_style_local_line_opa(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_20);
    return arc;
  }

  int16_t Percent(uint32_t value, uint32_t goal) {
    if (goal == 0) {
      return 0;
    }
    uint32_t percent = (value * 100) / goal;
    return static_cast<int16_t>(std::min<uint32_t>(percent, 100));
  }
}

ActivityRings::ActivityRings(DisplayApp* app, Controllers::HealthController& healthController, Controllers::Settings& settingsController)
  : app {app}, healthController {healthController}, settingsController {settingsController} {

  moveArc = CreateRing(220, Colors::deepOrange);
  exerciseArc = CreateRing(170, Colors::green);
  standArc = CreateRing(120, Colors::blue);

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  streakLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(streakLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::deepOrange);
  lv_obj_align(streakLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  hintLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(hintLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(hintLabel, "Tap for history");
  lv_obj_align(hintLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -8);

  taskRefresh = lv_task_create(RefreshTaskCallback, 5000, LV_TASK_PRIO_MID, this);
  Refresh();
}

ActivityRings::~ActivityRings() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool ActivityRings::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::Tap) {
    app->StartApp(Apps::ActivityHistory, DisplayApp::FullRefreshDirections::Up);
    return true;
  }
  return false;
}

void ActivityRings::Refresh() {
  uint32_t calories = healthController.GetCaloriesToday();
  uint32_t caloriesGoal = settingsController.GetCaloriesGoal();
  uint32_t activeMinutes = healthController.GetActiveMinutesToday();
  uint32_t standHours = healthController.GetStandHoursToday();
  uint32_t exerciseGoalMinutes = settingsController.GetExerciseGoalMinutes();
  uint32_t standGoalHours = settingsController.GetStandGoalHours();

  lv_arc_set_value(moveArc, Percent(calories, caloriesGoal));
  lv_arc_set_value(exerciseArc, Percent(activeMinutes, exerciseGoalMinutes));
  lv_arc_set_value(standArc, Percent(standHours, standGoalHours));

  lv_label_set_text_fmt(
    label, "%lucal/%lu\n%lu/%lum\n%lu/%luh", calories, caloriesGoal, activeMinutes, exerciseGoalMinutes, standHours, standGoalHours);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  uint32_t pausedDays = settingsController.GetPausedDaysRemaining();
  if (pausedDays > 0) {
    lv_label_set_text_fmt(streakLabel, "Paused - %lud left", pausedDays);
  } else {
    lv_label_set_text_fmt(streakLabel, "%lu day streak", settingsController.GetCurrentStreak());
  }
  lv_obj_align(streakLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);
}
