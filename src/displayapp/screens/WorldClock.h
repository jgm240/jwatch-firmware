#pragma once

#include <array>
#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class WorldClock : public Screen {
      public:
        explicit WorldClock(Controllers::DateTime& dateTimeController);
        ~WorldClock() override;

        void Refresh() override;

      private:
        struct City {
          const char* name;
          int8_t utcOffsetHours;
        };

        static constexpr std::array<City, 4> cities = {{
          {"New York", -5},
          {"London", 0},
          {"Tokyo", 9},
          {"Sydney", 10},
        }};

        Controllers::DateTime& dateTimeController;
        std::array<lv_obj_t*, cities.size()> valueLabels {};

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::WorldClock> {
      static constexpr Apps app = Apps::WorldClock;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WorldClock(controllers.dateTimeController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
