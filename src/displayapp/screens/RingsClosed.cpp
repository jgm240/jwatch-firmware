#include "displayapp/screens/RingsClosed.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/motor/MotorController.h"
#include "components/motor/VibrationPatterns.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_color_t confettiColors[6] = {
    LV_COLOR_RED, LV_COLOR_YELLOW, Colors::green, Colors::blue, LV_COLOR_MAGENTA, Colors::orange,
  };
}

RingsClosed::RingsClosed(DisplayApp* app, Controllers::MotorController& motorController, Controllers::Settings& settingsController)
  : app {app} {

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label, "Rings closed!\nWell done");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  for (int i = 0; i < particleCount; i++) {
    particles[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(particles[i], particleSize, particleSize);
    lv_obj_set_style_local_bg_color(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, confettiColors[i % 6]);
    lv_obj_set_style_local_border_width(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (i % 2 == 0) ? 0 : LV_RADIUS_CIRCLE);
    lv_obj_set_click(particles[i], false);

    particleX[i] = static_cast<lv_coord_t>((i * (LV_HOR_RES / particleCount)) + (i % 3) * 4);
    particleY[i] = static_cast<lv_coord_t>(-10 - (i * 14));
    particleSpeed[i] = static_cast<lv_coord_t>(5 + (i % 3) * 2);

    lv_obj_set_pos(particles[i], particleX[i], particleY[i]);
  }

  motorController.RunPattern(Controllers::VibrationPatterns::Get(settingsController.GetVibrationPattern(), settingsController.GetCustomVibrationPattern()));

  taskRefresh = lv_task_create(RefreshTaskCallback, tickPeriodMs, LV_TASK_PRIO_MID, this);
}

RingsClosed::~RingsClosed() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void RingsClosed::Refresh() {
  elapsedMs += tickPeriodMs;
  if (elapsedMs >= displayDurationMs) {
    Exit();
    return;
  }

  for (int i = 0; i < particleCount; i++) {
    particleY[i] = static_cast<lv_coord_t>(particleY[i] + particleSpeed[i]);
    if (particleY[i] > LV_VER_RES + particleSize) {
      particleY[i] = static_cast<lv_coord_t>(-10 - (i % 5) * 10);
    }
    lv_obj_set_pos(particles[i], particleX[i], particleY[i]);
  }
}

bool RingsClosed::OnTouchEvent(TouchEvents /*event*/) {
  Exit();
  return true;
}

bool RingsClosed::OnButtonPushed() {
  Exit();
  return true;
}

void RingsClosed::Exit() {
  app->StartApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);
  running = false;
}
