#pragma once

#include "displayapp/widgets/complications/Complication.h"
#include "components/heartrate/HeartRateController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class HeartRateComplication : public Complication {
      public:
        explicit HeartRateComplication(const Controllers::HeartRateController& heartRateController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::HeartRateController& heartRateController;
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
