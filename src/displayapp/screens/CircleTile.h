#pragma once

#include <lvgl/lvgl.h>
#include <array>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Tile.h"
#include "displayapp/apps/Apps.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/widgets/StatusIcons.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Apple Watch-style colored circle grid, one page of up to 6 apps at a time (paged via
      // swipe, exactly like Tile). A first version tried rendering every app as one continuous
      // free-pan canvas; that put 40+ LVGL objects alive simultaneously and reliably crashed
      // the watch. This keeps the per-page object count identical to Tile/ApplicationList's
      // already-proven-stable footprint (6 circles + 6 labels max), trading continuous panning
      // for reliability - real hardware verification isn't available here, so the object count
      // that's already survived a full session of use is the safer bet over a further guess.
      class CircleTile : public Screen {
      public:
        static constexpr int paletteSize = 6;

        explicit CircleTile(uint8_t screenID,
                            uint8_t numScreens,
                            DisplayApp* app,
                            Controllers::Settings& settingsController,
                            const Controllers::Battery& batteryController,
                            const Controllers::Ble& bleController,
                            const Controllers::AlarmController& alarmController,
                            Controllers::DateTime& dateTimeController,
                            std::array<Tile::Applications, 6>& applications);
        ~CircleTile() override;

        void UpdateScreen();
        void OnEvent(lv_obj_t* obj, lv_event_t event);

      private:
        DisplayApp* app;
        Controllers::DateTime& dateTimeController;

        lv_task_t* taskUpdate;

        lv_obj_t* label_time;

        Widgets::PageIndicator pageIndicator;
        Widgets::StatusIcons statusIcons;

        std::array<lv_style_t, paletteSize> circleStyles;
        lv_style_t disabledStyle;

        std::array<lv_obj_t*, 6> circles {};
        std::array<Apps, 6> apps {};
      };
    }
  }
}
