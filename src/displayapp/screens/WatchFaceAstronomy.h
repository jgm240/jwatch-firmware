#pragma once

#include <lvgl/lvgl.h>
#include <array>
#include <chrono>
#include <cstdint>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/widgets/StatusIcons.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "utility/DirtyValue.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Shows the Moon at its real current phase (a stylized two-overlapping-circles render,
      // not a photo/texture - this hardware has no room for image assets of that kind) behind
      // the time, over a small fixed starfield. Phase is computed from the calendar date via
      // the standard synodic-month approximation - accurate to within about a day, which is
      // plenty for a decorative watchface. Redrawn once per day (the phase doesn't move
      // enough within a day to be worth recomputing more often).
      class WatchFaceAstronomy : public Screen {
      public:
        WatchFaceAstronomy(Controllers::DateTime& dateTimeController,
                           Controllers::Settings& settingsController,
                           const Controllers::Battery& batteryController,
                           const Controllers::Ble& bleController,
                           const Controllers::AlarmController& alarmController);
        ~WatchFaceAstronomy() override;

        void Refresh() override;

      private:
        static constexpr lv_coord_t canvasSize = 80;
        static constexpr int starCount = 8;

        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;

        lv_obj_t* canvas;
        lv_obj_t* label_time;
        lv_obj_t* label_date;
        std::array<lv_obj_t*, starCount> stars {};

        Widgets::StatusIcons statusIcons;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate {};

        uint8_t canvasBuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(canvasSize, canvasSize)];

        lv_task_t* taskRefresh;

        void DrawMoon();
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Astronomy> {
      static constexpr WatchFace watchFace = WatchFace::Astronomy;
      static constexpr const char* name = "Astronomy";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceAstronomy(controllers.dateTimeController,
                                               controllers.settingsController,
                                               controllers.batteryController,
                                               controllers.bleController,
                                               controllers.alarmController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
