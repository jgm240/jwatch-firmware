#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/widgets/complications/Complication.h"
#include "components/health/HealthController.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/HexagonGauge.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // Miniature version of ActivityRings (Move/Exercise/Stand), same 3-hexagon idea scaled
      // down to fit a modular watchface slot.
      class FitnessRingsComplication : public Complication {
      public:
        FitnessRingsComplication(const Controllers::HealthController& healthController, const Controllers::Settings& settingsController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::HealthController& healthController;
        const Controllers::Settings& settingsController;

        lv_obj_t* container = nullptr;
        HexagonGauge moveGauge;
        HexagonGauge exerciseGauge;
        HexagonGauge standGauge;
      };
    }
  }
}
