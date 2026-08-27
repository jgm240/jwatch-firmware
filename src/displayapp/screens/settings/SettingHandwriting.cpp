#include "displayapp/screens/settings/SettingHandwriting.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include <cstdlib>
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;
namespace HandwritingGlyphs = Pinetime::Applications::Widgets::HandwritingGlyphs;

namespace {
  constexpr lv_color_t bgColor = LV_COLOR_BLACK;
  constexpr lv_color_t inkColor = LV_COLOR_WHITE;
  // OnTouchEvent(x,y) is only ever called while genuinely touching (DisplayApp.cpp gates it on
  // IsTouching()), so it never fires during a real pen lift - a gap this long between samples is
  // the only signal available that the finger actually lifted and came back down elsewhere.
  constexpr uint32_t liftGapMs = 150;

  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingHandwriting*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  char StepChar(int step) {
    if (step == HandwritingGlyphs::colonIndex) {
      return ':';
    }
    return static_cast<char>('0' + step);
  }
}

SettingHandwriting::SettingHandwriting(Pinetime::Controllers::FS& filesystem) : filesystem {filesystem} {
  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(titleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 15);

  canvas = lv_canvas_create(lv_scr_act(), nullptr);
  lv_canvas_set_buffer(canvas, canvasBuf, HandwritingGlyphs::glyphWidth, HandwritingGlyphs::glyphHeight, LV_IMG_CF_TRUE_COLOR);
  lv_obj_set_pos(canvas, canvasX, canvasY);

  btnClear = Styles::Button(lv_scr_act());
  btnClear->user_data = this;
  lv_obj_set_event_cb(btnClear, eventHandler);
  lv_obj_set_size(btnClear, 76, 50);
  lv_obj_align(btnClear, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_t* clearLabel = lv_label_create(btnClear, nullptr);
  lv_label_set_text_static(clearLabel, Symbols::backspace);

  btnBack = Styles::Button(lv_scr_act());
  btnBack->user_data = this;
  lv_obj_set_event_cb(btnBack, eventHandler);
  lv_obj_set_size(btnBack, 76, 50);
  lv_obj_align(btnBack, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_t* backLabel = lv_label_create(btnBack, nullptr);
  lv_label_set_text_static(backLabel, Symbols::stepBackward);

  btnNext = Styles::Button(lv_scr_act());
  btnNext->user_data = this;
  lv_obj_set_event_cb(btnNext, eventHandler);
  lv_obj_set_size(btnNext, 76, 50);
  lv_obj_align(btnNext, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t* nextLabel = lv_label_create(btnNext, nullptr);
  lv_label_set_text_static(nextLabel, Symbols::check);

  LoadStep();
}

SettingHandwriting::~SettingHandwriting() {
  lv_obj_clean(lv_scr_act());
}

void SettingHandwriting::LoadStep() {
  lv_canvas_fill_bg(canvas, bgColor, LV_OPA_COVER);
  const auto& bitmap = allGlyphs[step];
  for (int y = 0; y < HandwritingGlyphs::glyphHeight; y++) {
    for (int x = 0; x < HandwritingGlyphs::glyphWidth; x++) {
      if (bitmap[static_cast<size_t>(y) * HandwritingGlyphs::glyphWidth + x]) {
        lv_canvas_set_px(canvas, x, y, inkColor);
      }
    }
  }
  hasLastPoint = false;
  lv_label_set_text_fmt(titleLabel, "Draw: %c", StepChar(step));
  lv_obj_realign(titleLabel);
}

void SettingHandwriting::StampBrush(int cx, int cy) {
  for (int oy = -1; oy <= 1; oy++) {
    for (int ox = -1; ox <= 1; ox++) {
      int px = cx + ox;
      int py = cy + oy;
      if (px < 0 || px >= HandwritingGlyphs::glyphWidth || py < 0 || py >= HandwritingGlyphs::glyphHeight) {
        continue;
      }
      allGlyphs[step][static_cast<size_t>(py) * HandwritingGlyphs::glyphWidth + px] = true;
      lv_canvas_set_px(canvas, px, py, inkColor);
    }
  }
}

void SettingHandwriting::DrawLine(int x0, int y0, int x1, int y1) {
  int dx = std::abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int x = x0;
  int y = y0;
  while (true) {
    StampBrush(x, y);
    if (x == x1 && y == y1) {
      break;
    }
    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y += sy;
    }
  }
}

void SettingHandwriting::DrawPoint(int x, int y) {
  if (hasLastPoint) {
    DrawLine(lastX, lastY, x, y);
  } else {
    StampBrush(x, y);
  }
  lastX = x;
  lastY = y;
  hasLastPoint = true;
}

bool SettingHandwriting::OnTouchEvent(uint16_t x, uint16_t y) {
  uint32_t now = lv_tick_get();
  if (hasLastPoint && lv_tick_elaps(lastSampleTick) > liftGapMs) {
    hasLastPoint = false;
  }
  lastSampleTick = now;

  if (x < canvasX || x >= canvasX + HandwritingGlyphs::glyphWidth || y < canvasY || y >= canvasY + HandwritingGlyphs::glyphHeight) {
    hasLastPoint = false;
    return true;
  }
  DrawPoint(x - canvasX, y - canvasY);
  return true;
}

void SettingHandwriting::OnClear() {
  allGlyphs[step].reset();
  lv_canvas_fill_bg(canvas, bgColor, LV_OPA_COVER);
  hasLastPoint = false;
}

void SettingHandwriting::OnNext() {
  HandwritingGlyphs::SaveGlyph(filesystem, step, allGlyphs[step]);
  if (step >= HandwritingGlyphs::glyphCount - 1) {
    running = false;
    return;
  }
  step++;
  LoadStep();
}

void SettingHandwriting::OnBack() {
  if (step == 0) {
    return;
  }
  step--;
  LoadStep();
}

void SettingHandwriting::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (obj == btnClear) {
    OnClear();
  } else if (obj == btnBack) {
    OnBack();
  } else if (obj == btnNext) {
    OnNext();
  }
}
