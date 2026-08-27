#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/widgets/HandwritingGlyphs.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {
    class Battery;
    class Ble;
    class AlarmController;
    class FS;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceHandwriting : public Screen {
      public:
        WatchFaceHandwriting(Controllers::DateTime& dateTimeController,
                             const Controllers::Battery& batteryController,
                             const Controllers::Ble& bleController,
                             const Controllers::AlarmController& alarmController,
                             Controllers::Settings& settingsController);
        ~WatchFaceHandwriting() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        void SetDigit(lv_obj_t* imgObj, uint8_t digit);

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint8_t> hourTens {};
        Utility::DirtyValue<uint8_t> hourOnes {};
        Utility::DirtyValue<uint8_t> minuteTens {};
        Utility::DirtyValue<uint8_t> minuteOnes {};

        lv_obj_t* imgHourTens;
        lv_obj_t* imgHourOnes;
        lv_obj_t* imgColon;
        lv_obj_t* imgMinuteTens;
        lv_obj_t* imgMinuteOnes;
        lv_obj_t* label_date;

        Controllers::DateTime& dateTimeController;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Handwriting> {
      static constexpr WatchFace watchFace = WatchFace::Handwriting;
      static constexpr const char* name = "Handwriting";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceHandwriting(controllers.dateTimeController,
                                                  controllers.batteryController,
                                                  controllers.bleController,
                                                  controllers.alarmController,
                                                  controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
        return Screens::WatchFaceHandwriting::IsAvailable(filesystem);
      }
    };
  }
}
