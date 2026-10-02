#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // A flat-top hexagon outline used as a progress gauge: a dim full-hexagon track plus a
      // brighter outline that traces the perimeter clockwise, starting at the middle of the
      // top edge, covering `percent` of the total length. Centered in its parent.
      //
      // LVGL's lv_line keeps a pointer to its point array, so the gauge (not just the lv
      // objects it creates) has to outlive the screen/complication that owns it.
      class HexagonGauge {
      public:
        // circumradius is the distance from the center to each vertex (the hexagon is
        // 2 * circumradius wide). Keep it at least lineWidth / 2 smaller than half of the
        // parent's width, or the outer edge of the line gets clipped by the parent.
        void Create(lv_obj_t* parent, lv_coord_t circumradius, lv_coord_t lineWidth, lv_color_t color);

        // 0..100; values above 100 are clamped.
        void SetValue(uint8_t percent);

      private:
        static constexpr uint8_t maxPoints = 8; // mid-top, 6 vertices, back to mid-top

        lv_obj_t* track = nullptr;
        lv_obj_t* progress = nullptr;

        lv_coord_t radius = 0;
        std::array<lv_point_t, maxPoints> outline {}; // the full closed hexagon
        std::array<lv_point_t, maxPoints> trackPoints {};
        std::array<lv_point_t, maxPoints> progressPoints {};

        uint8_t lastPercent = 255;
      };
    }
  }
}
