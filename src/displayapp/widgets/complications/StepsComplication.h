#pragma once

#include "displayapp/widgets/complications/Complication.h"
#include "components/motion/MotionController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class StepsComplication : public Complication {
      public:
        explicit StepsComplication(const Controllers::MotionController& motionController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::MotionController& motionController;
        Utility::DirtyValue<uint32_t> stepCount {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
