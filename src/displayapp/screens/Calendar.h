#pragma once

#include <array>
#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/Controllers.h"
#include "components/ble/CalendarEventService.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Calendar : public Screen {
      public:
        Calendar(Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController);
        ~Calendar() override;

        void Refresh() override;

      private:
        Controllers::CalendarEventService& calendarEventService;
        Controllers::DateTime& dateTimeController;

        static constexpr uint8_t MaxDisplayedEvents = Controllers::CalendarEventService::MaxEvents;

        lv_obj_t* noEventsLabel = nullptr;
        std::array<lv_obj_t*, MaxDisplayedEvents> timeLabels {};
        std::array<lv_obj_t*, MaxDisplayedEvents> titleLabels {};

        uint8_t lastDisplayedCount = 0xff;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Calendar> {
      static constexpr Apps app = Apps::Calendar;
      static constexpr const char* icon = Screens::Symbols::calendar;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Calendar(*controllers.calendarService, controllers.dateTimeController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
