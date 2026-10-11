#pragma once

#include <cstdint>
#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // The next sunrise or sunset time, from the phone's weather data.
      class SunComplication : public Complication {
      public:
        SunComplication(const Controllers::SimpleWeatherService& weatherService, Controllers::DateTime& dateTimeController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::SimpleWeatherService& weatherService;
        Controllers::DateTime& dateTimeController;
        lv_obj_t* label = nullptr;
        int32_t shownKey = -1;
      };
    }
  }
}
