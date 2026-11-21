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

      // Shows the next few days of synced events (see CalendarEventService): school lessons, exams
      // ("Exam: ..." titles) and homework due dates ("HW: ..." titles) all arrive over the same
      // channel, prefixed by whatever pushed them (e.g. SSC's `ssc.py watch sync`); this screen just
      // tells them apart by that prefix to color-code and to show a day label once it's not today.
      class School : public Screen {
      public:
        School(Controllers::CalendarEventService& calendarEventService, Controllers::DateTime& dateTimeController);
        ~School() override;

        void Refresh() override;

      private:
        Controllers::CalendarEventService& calendarEventService;
        Controllers::DateTime& dateTimeController;

        static constexpr uint8_t MaxDisplayedEvents = Controllers::CalendarEventService::MaxEvents;

        lv_obj_t* noEventsLabel = nullptr;
        std::array<lv_obj_t*, MaxDisplayedEvents> timeLabels {};
        std::array<lv_obj_t*, MaxDisplayedEvents> titleLabels {};

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::School> {
      static constexpr Apps app = Apps::School;
      static constexpr const char* icon = Screens::Symbols::calendar;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::School(*controllers.calendarService, controllers.dateTimeController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
