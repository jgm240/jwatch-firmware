#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/widgets/complications/Complication.h"
#include "components/motion/MotionController.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // This hardware has no gyroscope, only an accelerometer - this reuses the same
      // tilt signal as the watchface tilt-parallax feature to fake a cube leaning in
      // 3D as the watch is tilted. It's a wireframe drawn with a fixed front face and
      // a "depth" face offset by tilt, not a real 3D transform.
      class CubeComplication : public Complication {
      public:
        explicit CubeComplication(const Controllers::MotionController& motionController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        static constexpr lv_coord_t canvasSize = 32;

        const Controllers::MotionController& motionController;
        lv_obj_t* canvas = nullptr;
        uint8_t canvasBuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(canvasSize, canvasSize)];

        void Draw(lv_coord_t dx, lv_coord_t dy);
      };
    }
  }
}
