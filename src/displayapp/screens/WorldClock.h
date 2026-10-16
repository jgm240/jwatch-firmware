#pragma once

#include <array>
#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "components/datetime/WorldTime.h"
#include "components/settings/Settings.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Local time in up to five cities (picked in Settings > World clock), daylight saving included.
      class WorldClock : public Screen {
      public:
        WorldClock(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController);
        ~WorldClock() override;

        void Refresh() override;

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        std::array<lv_obj_t*, Controllers::worldClockSlots> nameLabels {};
        std::array<lv_obj_t*, Controllers::worldClockSlots> timeLabels {};
        std::array<lv_obj_t*, Controllers::worldClockSlots> detailLabels {};

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::WorldClock> {
      static constexpr Apps app = Apps::WorldClock;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WorldClock(controllers.dateTimeController, controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
