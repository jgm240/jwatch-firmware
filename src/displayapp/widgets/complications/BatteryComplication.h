#pragma once

#include "displayapp/widgets/complications/Complication.h"
#include "components/battery/BatteryController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class BatteryComplication : public Complication {
      public:
        explicit BatteryComplication(const Controllers::Battery& batteryController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::Battery& batteryController;
        Utility::DirtyValue<uint8_t> percentRemaining {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
