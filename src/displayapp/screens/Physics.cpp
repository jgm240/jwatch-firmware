#include "displayapp/screens/Physics.h"
#include <cmath>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_color_t beadColors[6] = {
    LV_COLOR_RED, LV_COLOR_YELLOW, Colors::green, Colors::blue, LV_COLOR_MAGENTA, Colors::orange,
  };
  constexpr lv_color_t fluidColor = LV_COLOR_MAKE(0x30, 0x90, 0xff);

  // No hardware calibration possible here - a conservative starting guess, same category
  // of judgment call as the watchface tilt-parallax divisor.
  constexpr float gravityScale = 0.03F;

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    auto* screen = static_cast<Physics*>(obj->user_data);
    screen->OnButtonEvent(obj);
  }
}

Physics::Physics(Controllers::MotionController& motionController, Controllers::Settings& settingsController)
  : motionController {motionController}, settingsController {settingsController} {

  modeButton = Styles::Button(lv_scr_act());
  modeButton->user_data = this;
  lv_obj_set_event_cb(modeButton, event_handler);
  lv_obj_set_size(modeButton, 110, 30);
  lv_obj_align(modeButton, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 2);
  modeLabel = lv_label_create(modeButton, nullptr);
  lv_label_set_text_static(modeLabel, "Beads");

  btnMinus = Styles::Button(lv_scr_act());
  btnMinus->user_data = this;
  lv_obj_set_event_cb(btnMinus, event_handler);
  lv_obj_set_size(btnMinus, 70, 45);
  lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
  lv_label_set_text_static(lblMinus, "-");

  btnPlus = Styles::Button(lv_scr_act());
  btnPlus->user_data = this;
  lv_obj_set_event_cb(btnPlus, event_handler);
  lv_obj_set_size(btnPlus, 70, 45);
  lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
  lv_label_set_text_static(lblPlus, "+");

  countLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(countLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);

  for (int i = 0; i < maxParticles; i++) {
    particles[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(particles[i], particleSize, particleSize);
    lv_obj_set_style_local_radius(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_border_width(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(particles[i], false);

    px[i] = static_cast<float>((i * 37 + 20) % (LV_HOR_RES - particleSize)) + particleSize / 2.0F;
    py[i] = static_cast<float>(simTop + ((i * 23) % (simBottom - simTop - particleSize)));
    vx[i] = 0;
    vy[i] = 0;
    lv_obj_set_pos(particles[i], static_cast<lv_coord_t>(px[i]), static_cast<lv_coord_t>(py[i]));
  }

  ApplyModeColors();
  SetActiveCount(settingsController.GetPhysicsParticleCount());

  taskRefresh = lv_task_create(RefreshTaskCallback, tickPeriodMs, LV_TASK_PRIO_MID, this);
}

Physics::~Physics() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Physics::ApplyModeColors() {
  for (int i = 0; i < maxParticles; i++) {
    lv_color_t color = (mode == Mode::Fluid) ? fluidColor : beadColors[i % 6];
    lv_obj_set_style_local_bg_color(particles[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
  }
}

void Physics::SetActiveCount(uint8_t count) {
  activeCount = count;
  for (int i = 0; i < maxParticles; i++) {
    lv_obj_set_hidden(particles[i], i >= activeCount);
  }
  lv_label_set_text_fmt(countLabel, "%u", activeCount);
  lv_obj_align(countLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
}

void Physics::OnButtonEvent(lv_obj_t* target) {
  if (target == modeButton) {
    mode = (mode == Mode::Beads) ? Mode::Fluid : Mode::Beads;
    lv_label_set_text_static(modeLabel, mode == Mode::Beads ? "Beads" : "Fluid");
    ApplyModeColors();
    for (int i = 0; i < maxParticles; i++) {
      vx[i] = 0;
      vy[i] = 0;
    }
  } else if (target == btnPlus) {
    uint8_t count = settingsController.GetPhysicsParticleCount();
    settingsController.SetPhysicsParticleCount(static_cast<uint8_t>(count + 1));
    settingsController.SaveSettings();
    SetActiveCount(settingsController.GetPhysicsParticleCount());
  } else if (target == btnMinus) {
    uint8_t count = settingsController.GetPhysicsParticleCount();
    settingsController.SetPhysicsParticleCount(static_cast<uint8_t>(count > 0 ? count - 1 : 0));
    settingsController.SaveSettings();
    SetActiveCount(settingsController.GetPhysicsParticleCount());
  }
}

void Physics::Refresh() {
  Step();
}

void Physics::Step() {
  // Negated: a stationary accelerometer reads the reaction force pointing AWAY from
  // gravity's pull (e.g. lying flat screen-up reads +1g on the upward-pointing axis, not
  // downward), so using the raw reading directly made particles accelerate toward "up"
  // instead of "down" - the reported "levitating instead of falling" bug.
  float gx = -static_cast<float>(motionController.X()) * gravityScale;
  float gy = -static_cast<float>(motionController.Y()) * gravityScale;

  float damping = (mode == Mode::Fluid) ? 0.90F : 0.98F;
  float restitution = (mode == Mode::Fluid) ? 0.2F : 0.6F;
  float minDist = static_cast<float>(particleSize);
  float cohesionRange = minDist * 2.2F;

  for (uint8_t i = 0; i < activeCount; i++) {
    vx[i] = (vx[i] + gx) * damping;
    vy[i] = (vy[i] + gy) * damping;
  }

  if (mode == Mode::Fluid) {
    for (uint8_t i = 0; i < activeCount; i++) {
      for (uint8_t j = i + 1; j < activeCount; j++) {
        float dx = px[j] - px[i];
        float dy = py[j] - py[i];
        float distSq = dx * dx + dy * dy;
        if (distSq < 0.01F || distSq > cohesionRange * cohesionRange) {
          continue;
        }
        float dist = std::sqrt(distSq);
        if (dist > minDist) {
          constexpr float pull = 0.02F;
          float nx = dx / dist;
          float ny = dy / dist;
          vx[i] += nx * pull;
          vy[i] += ny * pull;
          vx[j] -= nx * pull;
          vy[j] -= ny * pull;
        }
      }
    }
  }

  for (uint8_t i = 0; i < activeCount; i++) {
    for (uint8_t j = i + 1; j < activeCount; j++) {
      float dx = px[j] - px[i];
      float dy = py[j] - py[i];
      float distSq = dx * dx + dy * dy;
      if (distSq >= minDist * minDist || distSq < 0.0001F) {
        continue;
      }
      float dist = std::sqrt(distSq);
      float overlap = (minDist - dist) * 0.5F;
      float nx = dx / dist;
      float ny = dy / dist;
      px[i] -= nx * overlap;
      py[i] -= ny * overlap;
      px[j] += nx * overlap;
      py[j] += ny * overlap;
    }
  }

  float minX = particleSize / 2.0F;
  float maxX = static_cast<float>(LV_HOR_RES) - particleSize / 2.0F;
  float minY = static_cast<float>(simTop) + particleSize / 2.0F;
  float maxY = static_cast<float>(simBottom) - particleSize / 2.0F;

  for (uint8_t i = 0; i < activeCount; i++) {
    px[i] += vx[i];
    py[i] += vy[i];

    if (px[i] < minX) {
      px[i] = minX;
      vx[i] = -vx[i] * restitution;
    } else if (px[i] > maxX) {
      px[i] = maxX;
      vx[i] = -vx[i] * restitution;
    }
    if (py[i] < minY) {
      py[i] = minY;
      vy[i] = -vy[i] * restitution;
    } else if (py[i] > maxY) {
      py[i] = maxY;
      vy[i] = -vy[i] * restitution;
    }

    lv_obj_set_pos(particles[i],
                    static_cast<lv_coord_t>(px[i] - particleSize / 2.0F),
                    static_cast<lv_coord_t>(py[i] - particleSize / 2.0F));
  }
}
