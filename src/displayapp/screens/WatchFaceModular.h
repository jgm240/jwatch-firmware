#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <lvgl/src/lv_core/lv_obj.h>

#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/complications/Complication.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "utility/DirtyValue.h"
#include "displayapp/widgets/SecondsSquare.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class WatchFaceModular : public Screen {
      public:
        WatchFaceModular(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController, AppControllers& controllers);
        ~WatchFaceModular() override;

        void Refresh() override;

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};

        lv_obj_t* label_time;
        lv_obj_t* label_date;
        Widgets::SecondsSquare secondsSquare;

        std::array<std::unique_ptr<Widgets::Complication>, Controllers::Settings::modularWatchFaceSlots> complications;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Modular> {
      static constexpr WatchFace watchFace = WatchFace::Modular;
      static constexpr const char* name = "Modular";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceModular(controllers.dateTimeController, controllers.settingsController, controllers);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
