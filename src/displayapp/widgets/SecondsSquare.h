#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // Apple Watch-style square seconds track around the screen edge, replacing the
      // circular arc: a dim square outline with a bright segment that sweeps clockwise
      // around its 4 sides over 60 seconds (each side = one quarter, 15s). Plain lv_obj_t
      // rectangles, not a canvas - a border-only shape drawn via canvas would still need a
      // full-screen-sized pixel buffer (~115KB at 240x240), far past this hardware's 64KB
      // RAM, whereas 8 small rectangles cost nothing.
      class SecondsSquare {
      public:
        void Create(lv_obj_t* parent, lv_color_t color);
        void SetValue(uint8_t seconds);

      private:
        lv_obj_t* litTop = nullptr;
        lv_obj_t* litRight = nullptr;
        lv_obj_t* litBottom = nullptr;
        lv_obj_t* litLeft = nullptr;
      };
    }
  }
}
