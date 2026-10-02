#include "displayapp/widgets/complications/FitnessRingsComplication.h"
#include <algorithm>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  uint8_t Percent(uint32_t value, uint32_t goal) {
    if (goal == 0) {
      return 0;
    }
    uint32_t percent = (value * 100) / goal;
    return static_cast<uint8_t>(std::min<uint32_t>(percent, 100));
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

  moveGauge.Create(container, 25, 3, Colors::deepOrange);
  exerciseGauge.Create(container, 18, 3, Colors::green);
  standGauge.Create(container, 11, 3, Colors::blue);
}

void FitnessRingsComplication::Update() {
  uint32_t calories = healthController.GetCaloriesToday();
  uint32_t caloriesGoal = settingsController.GetCaloriesGoal();
  uint32_t activeMinutes = healthController.GetActiveMinutesToday();
  uint32_t exerciseGoalMinutes = settingsController.GetExerciseGoalMinutes();
  uint32_t standHours = healthController.GetStandHoursToday();
  uint32_t standGoalHours = settingsController.GetStandGoalHours();

  moveGauge.SetValue(Percent(calories, caloriesGoal));
  exerciseGauge.SetValue(Percent(activeMinutes, exerciseGoalMinutes));
  standGauge.SetValue(Percent(standHours, standGoalHours));
}
