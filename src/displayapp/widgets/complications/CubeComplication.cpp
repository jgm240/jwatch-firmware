#include "displayapp/widgets/complications/CubeComplication.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  constexpr int16_t tiltDivisor = 150;
  constexpr lv_coord_t maxDepth = 6;

  lv_coord_t DepthOffset(int16_t raw) {
    int32_t offset = raw / tiltDivisor;
    if (offset > maxDepth) {
      offset = maxDepth;
    } else if (offset < -maxDepth) {
      offset = -maxDepth;
    }
    return static_cast<lv_coord_t>(offset);
  }
}

CubeComplication::CubeComplication(const Controllers::MotionController& motionController) : motionController {motionController} {
}

void CubeComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  canvas = lv_canvas_create(parent, nullptr);
  lv_canvas_set_buffer(canvas, canvasBuf, canvasSize, canvasSize, LV_IMG_CF_TRUE_COLOR_ALPHA);
  lv_obj_align(canvas, parent, align, xOfs, yOfs);
  Draw(0, 0);
}

void CubeComplication::Update() {
  lv_coord_t dx = DepthOffset(motionController.X());
  lv_coord_t dy = DepthOffset(motionController.Y());
  Draw(dx, dy);
}

void CubeComplication::Draw(lv_coord_t dx, lv_coord_t dy) {
  lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, LV_OPA_TRANSP);

  constexpr lv_coord_t half = canvasSize / 2 - maxDepth - 2;
  constexpr lv_coord_t cx = canvasSize / 2;
  constexpr lv_coord_t cy = canvasSize / 2;

  lv_point_t front[5] = {
    {static_cast<lv_coord_t>(cx - half), static_cast<lv_coord_t>(cy - half)},
    {static_cast<lv_coord_t>(cx + half), static_cast<lv_coord_t>(cy - half)},
    {static_cast<lv_coord_t>(cx + half), static_cast<lv_coord_t>(cy + half)},
    {static_cast<lv_coord_t>(cx - half), static_cast<lv_coord_t>(cy + half)},
    {static_cast<lv_coord_t>(cx - half), static_cast<lv_coord_t>(cy - half)},
  };
  lv_point_t back[5];
  for (int i = 0; i < 5; i++) {
    back[i].x = static_cast<lv_coord_t>(front[i].x + dx);
    back[i].y = static_cast<lv_coord_t>(front[i].y + dy);
  }

  lv_draw_line_dsc_t lineDsc;
  lv_draw_line_dsc_init(&lineDsc);
  lineDsc.color = LV_COLOR_WHITE;
  lineDsc.width = 1;
  lineDsc.opa = LV_OPA_COVER;

  lv_canvas_draw_line(canvas, front, 5, &lineDsc);
  lv_canvas_draw_line(canvas, back, 5, &lineDsc);

  for (int i = 0; i < 4; i++) {
    lv_point_t edge[2] = {front[i], back[i]};
    lv_canvas_draw_line(canvas, edge, 2, &lineDsc);
  }
}
