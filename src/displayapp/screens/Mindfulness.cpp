#include "displayapp/screens/Mindfulness.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"
#include "components/motor/VibrationPatterns.h"

using namespace Pinetime::Applications::Screens;

Mindfulness::Mindfulness(Controllers::MotorController& motorController) : motorController {motorController} {
  circle = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(circle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
  lv_obj_set_style_local_radius(circle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_size(circle, minDiameter, minDiameter);
  lv_obj_align(circle, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label, "Breathe In");
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 20);

  taskRefresh = lv_task_create(RefreshTaskCallback, tickPeriodMs, LV_TASK_PRIO_MID, this);
}

Mindfulness::~Mindfulness() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Mindfulness::Refresh() {
  elapsedInPhaseMs += tickPeriodMs;

  if (elapsedInPhaseMs >= phaseDurationMs) {
    elapsedInPhaseMs = 0;
    phase = (phase == Phase::In) ? Phase::Out : Phase::In;
    lv_label_set_text_static(label, phase == Phase::In ? "Breathe In" : "Breathe Out");
    lv_obj_realign(label);
    motorController.RunPattern(Controllers::VibrationPatterns::Single);
  }

  float progress = static_cast<float>(elapsedInPhaseMs) / static_cast<float>(phaseDurationMs);
  if (phase == Phase::Out) {
    progress = 1.0F - progress;
  }

  lv_coord_t diameter = minDiameter + static_cast<lv_coord_t>(progress * static_cast<float>(maxDiameter - minDiameter));
  lv_obj_set_size(circle, diameter, diameter);
  lv_obj_align(circle, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}
