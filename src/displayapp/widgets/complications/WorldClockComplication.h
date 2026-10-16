#pragma once

#include <cstdint>
#include "displayapp/widgets/complications/Complication.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // The time in the first city of the World clock list, e.g. "VIE 14:05".
      class WorldClockComplication : public Complication {
      public:
        WorldClockComplication(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        lv_obj_t* label = nullptr;
        int32_t shownKey = -1;
      };
    }
  }
}
