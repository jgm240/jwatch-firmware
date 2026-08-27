#include "displayapp/widgets/complications/FitnessRingsComplication.h"
#include <algorithm>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  lv_obj_t* CreateRing(lv_obj_t* parent, lv_coord_t size, lv_color_t color) {
    lv_obj_t* arc = lv_arc_create(parent, nullptr);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 0);
    lv_arc_set_adjustable(arc, false);
    lv_obj_set_click(arc, false);
    lv_obj_set_size(arc, size, size);
    lv_obj_align(arc, parent, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, color);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
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

FitnessRingsComplication::FitnessRingsComplication(const Controllers::HealthController& healthController,
                                                    const Controllers::Settings& settingsController)
  : healthController {healthController}, settingsController {settingsController} {
}

void FitnessRingsComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  container = lv_obj_create(parent, nullptr);
  lv_obj_set_size(container, 54, 54);
  lv_obj_set_style_local_bg_opa(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_border_width(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_click(container, false);
  lv_obj_align(container, parent, align, xOfs, yOfs);

  moveArc = CreateRing(container, 54, Colors::deepOrange);
  exerciseArc = CreateRing(container, 40, Colors::green);
  standArc = CreateRing(container, 26, Colors::blue);
}

void FitnessRingsComplication::Update() {
  uint32_t calories = healthController.GetCaloriesToday();
  uint32_t caloriesGoal = settingsController.GetCaloriesGoal();
  uint32_t activeMinutes = healthController.GetActiveMinutesToday();
  uint32_t exerciseGoalMinutes = settingsController.GetExerciseGoalMinutes();
  uint32_t standHours = healthController.GetStandHoursToday();
  uint32_t standGoalHours = settingsController.GetStandGoalHours();

  lv_arc_set_value(moveArc, Percent(calories, caloriesGoal));
  lv_arc_set_value(exerciseArc, Percent(activeMinutes, exerciseGoalMinutes));
  lv_arc_set_value(standArc, Percent(standHours, standGoalHours));
}
