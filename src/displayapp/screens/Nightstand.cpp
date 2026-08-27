#include "displayapp/screens/Nightstand.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_color_t bgColor = LV_COLOR_BLACK;
}

Nightstand::Nightstand(DisplayApp* app,
                       System::SystemTask& systemTask,
                       Controllers::DateTime& dateTimeController,
                       const Controllers::Battery& batteryController,
                       Controllers::BrightnessController& brightnessController)
  : app {app},
    wakeLock(systemTask),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    brightnessController {brightnessController} {

  wakeLock.Lock();
  previousBrightness = brightnessController.Level();
  brightnessController.Set(Controllers::BrightnessController::Levels::Low);

  canvas = lv_canvas_create(lv_scr_act(), nullptr);
  lv_canvas_set_buffer(canvas, canvasBuf, canvasSize, canvasSize, LV_IMG_CF_TRUE_COLOR);
  lv_obj_align(canvas, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(label_time, "");
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -30);

  taskRefresh = lv_task_create(RefreshTaskCallback, tickPeriodMs, LV_TASK_PRIO_MID, this);
  Refresh();
}

Nightstand::~Nightstand() {
  lv_task_del(taskRefresh);
  brightnessController.Set(previousBrightness);
  lv_obj_clean(lv_scr_act());
}

void Nightstand::Refresh() {
  if (!batteryController.IsCharging()) {
    Exit();
    return;
  }
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -30);
  UpdateShape();
}

void Nightstand::UpdateShape() {
  elapsedInPhaseMs += tickPeriodMs;
  if (elapsedInPhaseMs >= phaseDurationMs) {
    elapsedInPhaseMs = 0;
    if (phase == Phase::Shrinking) {
      // Advance to the next shape in the cycle and start growing it in.
      switch (currentShape) {
        case Shape::Circle:
          currentShape = Shape::Square;
          break;
        case Shape::Square:
          currentShape = Shape::Triangle;
          break;
        case Shape::Triangle:
          currentShape = Shape::Circle;
          break;
      }
      phase = Phase::Growing;
    } else {
      phase = Phase::Shrinking;
    }
  }

  float progress = static_cast<float>(elapsedInPhaseMs) / static_cast<float>(phaseDurationMs);
  if (phase == Phase::Shrinking) {
    progress = 1.0F - progress;
  }

  lv_coord_t size = minShapeSize + static_cast<lv_coord_t>(progress * static_cast<float>(canvasSize - minShapeSize));
  lv_coord_t offset = (canvasSize - size) / 2;

  lv_canvas_fill_bg(canvas, bgColor, LV_OPA_COVER);

  lv_draw_rect_dsc_t dsc;
  lv_draw_rect_dsc_init(&dsc);
  dsc.bg_color = Colors::blue;
  dsc.bg_opa = LV_OPA_COVER;

  switch (currentShape) {
    case Shape::Circle:
      dsc.radius = LV_RADIUS_CIRCLE;
      lv_canvas_draw_rect(canvas, offset, offset, size, size, &dsc);
      break;
    case Shape::Square:
      dsc.radius = 0;
      lv_canvas_draw_rect(canvas, offset, offset, size, size, &dsc);
      break;
    case Shape::Triangle: {
      lv_point_t points[3] = {
        {static_cast<lv_coord_t>(canvasSize / 2), offset},
        {offset, static_cast<lv_coord_t>(offset + size)},
        {static_cast<lv_coord_t>(offset + size), static_cast<lv_coord_t>(offset + size)},
      };
      lv_canvas_draw_polygon(canvas, points, 3, &dsc);
      break;
    }
  }
}

bool Nightstand::OnTouchEvent(TouchEvents /*event*/) {
  Exit();
  return true;
}

bool Nightstand::OnButtonPushed() {
  Exit();
  return true;
}

void Nightstand::Exit() {
  app->StartApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);
  running = false;
}
