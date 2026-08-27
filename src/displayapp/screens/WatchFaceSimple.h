#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/widgets/complications/StepsComplication.h"
#include "displayapp/widgets/SecondsSquare.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class AlarmController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceSimple : public Screen {
      public:
        WatchFaceSimple(Controllers::DateTime& dateTimeController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        const Controllers::AlarmController& alarmController,
                        Controllers::Settings& settingsController,
                        Controllers::MotionController& motionController);
        ~WatchFaceSimple() override;

        void Refresh() override;

      private:
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate {};

        lv_obj_t* label_time;
        lv_obj_t* label_date;
        Widgets::SecondsSquare secondsSquare;

        Widgets::StepsComplication stepsComplication;

        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Simple> {
      static constexpr WatchFace watchFace = WatchFace::Simple;
      static constexpr const char* name = "Simple";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceSimple(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.alarmController,
                                            controllers.settingsController,
                                            controllers.motionController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
