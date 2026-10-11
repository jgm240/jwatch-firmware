#pragma once

#include <cstdint>
#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/CalendarEventService.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      // "HH:MM Title" of the first synced calendar event that hasn't ended yet.
      class NextEventComplication : public Complication {
      public:
        NextEventComplication(const Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::CalendarEventService& calendarEventService;
        Controllers::DateTime& dateTimeController;
        lv_obj_t* label = nullptr;
        uint64_t shownKey = 0;
      };
    }
  }
}
