#include "displayapp/screens/SleepTracking.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

SleepTracking::SleepTracking(Controllers::HealthController& healthController) : healthController {healthController} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Sleep");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, -25);

  phaseLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(phaseLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_align(phaseLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 30);

  caveat = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(caveat, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(caveat, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(caveat, LV_HOR_RES - 20);
  lv_label_set_align(caveat, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(caveat, "Stillness-depth estimate, 22:00-08:00. No REM - not real sleep-stage tracking");
  lv_obj_align(caveat, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);

  taskRefresh = lv_task_create(RefreshTaskCallback, 60000, LV_TASK_PRIO_MID, this);
  Refresh();
}

SleepTracking::~SleepTracking() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void SleepTracking::Refresh() {
  uint32_t minutes = healthController.GetLastNightStillHalfHours() * 30;
  lv_label_set_text_fmt(label, "%luh%02lum", minutes / 60, minutes % 60);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, -25);

  uint32_t lightMinutes = healthController.GetLastNightLightHalfHours() * 30;
  uint32_t deepMinutes = healthController.GetLastNightDeepHalfHours() * 30;
  lv_label_set_text_fmt(phaseLabel, "Light %luh%02lum / Deep %luh%02lum", lightMinutes / 60, lightMinutes % 60, deepMinutes / 60, deepMinutes % 60);
  lv_obj_align(phaseLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 30);
}
