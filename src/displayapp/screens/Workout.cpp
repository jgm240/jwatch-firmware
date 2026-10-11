#include "displayapp/screens/Workout.h"
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_42;

namespace {
  void ButtonHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<Workout*>(obj->user_data)->ToggleRunning();
    }
  }
}

Workout::Workout(Controllers::HealthController& healthController,
                 Controllers::HeartRateController& heartRateController,
                 Controllers::MotionController& motionController,
                 System::SystemTask& systemTask)
  : healthController {healthController},
    heartRateController {heartRateController},
    motionController {motionController},
    wakeLock(systemTask) {

  timeLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(timeLabel, "00:00");
  lv_obj_align(timeLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 15);

  heartRateLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartRateLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::deepOrange);
  lv_label_set_text_static(heartRateLabel, "-- bpm");
  lv_obj_align(heartRateLabel, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 20, 85);

  stepsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(stepsLabel, "0 steps");
  lv_obj_align(stepsLabel, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -20, 85);

  hintLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(hintLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(hintLabel, "Stay on this screen");
  lv_obj_align(hintLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 10);

  button = lv_btn_create(lv_scr_act(), nullptr);
  button->user_data = this;
  lv_obj_set_size(button, 200, 60);
  lv_obj_align(button, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_event_cb(button, ButtonHandler);
  buttonLabel = lv_label_create(button, nullptr);
  lv_label_set_text_static(buttonLabel, "Start");

  taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
}

Workout::~Workout() {
  lv_task_del(taskRefresh);
  if (healthController.IsWorkoutRunning()) {
    Stop();
  }
  lv_obj_clean(lv_scr_act());
}

void Workout::ToggleRunning() {
  if (!healthController.IsWorkoutRunning()) {
    hrWasRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
    if (!hrWasRunning) {
      heartRateController.Enable();
    }
    healthController.StartWorkout();
    startTick = xTaskGetTickCount();
    startSteps = motionController.NbSteps();
    wakeLock.Lock();
    lv_label_set_text_static(buttonLabel, "Stop");
    lv_label_set_text_static(hintLabel, "Stay on this screen");
  } else {
    Stop();
    lv_label_set_text_static(buttonLabel, "Start");
    lv_label_set_text_static(hintLabel, "Saved - sync in JWLink");
    lv_obj_realign(hintLabel);
  }
}

void Workout::Stop() {
  const uint32_t elapsed = (xTaskGetTickCount() - startTick) / configTICK_RATE_HZ;
  healthController.StopWorkout(elapsed);
  if (!hrWasRunning) {
    heartRateController.Disable();
  }
  wakeLock.Release();
}

void Workout::Refresh() {
  if (!healthController.IsWorkoutRunning()) {
    return;
  }
  const uint32_t elapsed = (xTaskGetTickCount() - startTick) / configTICK_RATE_HZ;
  lv_label_set_text_fmt(timeLabel, "%02lu:%02lu", elapsed / 60, elapsed % 60);

  const uint8_t bpm = heartRateController.HeartRate();
  if (heartRateController.State() == Controllers::HeartRateController::States::Running && bpm > 0) {
    healthController.RecordWorkoutHeartRate(bpm);
    lv_label_set_text_fmt(heartRateLabel, "%03d bpm", bpm);
  } else {
    lv_label_set_text_static(heartRateLabel, "-- bpm");
  }

  const uint32_t nowSteps = motionController.NbSteps();
  lv_label_set_text_fmt(stepsLabel, "%lu steps", nowSteps >= startSteps ? nowSteps - startSteps : 0);
}
