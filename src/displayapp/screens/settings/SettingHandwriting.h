#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/fs/FS.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/HandwritingGlyphs.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingHandwriting : public Screen {
      public:
        explicit SettingHandwriting(Pinetime::Controllers::FS& filesystem);
        ~SettingHandwriting() override;

        void OnEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        void DrawPoint(int x, int y);
        void DrawLine(int x0, int y0, int x1, int y1);
        void StampBrush(int cx, int cy);
        void LoadStep();
        void OnNext();
        void OnBack();
        void OnClear();

        Controllers::FS& filesystem;

        static constexpr lv_coord_t canvasX = 98;
        static constexpr lv_coord_t canvasY = 60;

        int step = 0;
        std::array<Widgets::HandwritingGlyphs::GlyphBitmap, Widgets::HandwritingGlyphs::glyphCount> allGlyphs {};

        bool hasLastPoint = false;
        int lastX = 0;
        int lastY = 0;
        uint32_t lastSampleTick = 0;

        lv_obj_t* titleLabel;
        lv_obj_t* canvas;
        lv_obj_t* btnClear;
        lv_obj_t* btnBack;
        lv_obj_t* btnNext;

        uint8_t canvasBuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(Widgets::HandwritingGlyphs::glyphWidth, Widgets::HandwritingGlyphs::glyphHeight)];
      };
    }
  }
}
