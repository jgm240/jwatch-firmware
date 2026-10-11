#pragma once

#include <cstdint>
#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/CalendarEventService.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // The phone's battery level, as last pushed by the companion app.
      class PhoneBatteryComplication : public Complication {
      public:
        explicit PhoneBatteryComplication(const Controllers::CalendarEventService& calendarEventService);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::CalendarEventService& calendarEventService;
        lv_obj_t* label = nullptr;
        int32_t shownKey = -1;
      };
    }
  }
}
