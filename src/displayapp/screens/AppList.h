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

      // Apple Watch-style vertical list: one row per app, icon + name, same 6-per-page
      // pagination as Tile (Grid style) - reuses Tile::Applications as the per-app data
      // shape, just rendered as stacked clickable rows instead of a button matrix.
      class AppList : public Screen {
      public:
        explicit AppList(uint8_t screenID,
                         uint8_t numScreens,
                         DisplayApp* app,
                         Controllers::Settings& settingsController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         const Controllers::AlarmController& alarmController,
                         Controllers::DateTime& dateTimeController,
                         std::array<Tile::Applications, 6>& applications);

        ~AppList() override;

        void UpdateScreen();
        void OnRowClicked(lv_obj_t* obj);

      private:
        DisplayApp* app;
        Controllers::DateTime& dateTimeController;

        lv_task_t* taskUpdate;

        lv_obj_t* label_time;
        std::array<lv_obj_t*, 6> rows {};
        std::array<Pinetime::Applications::Apps, 6> apps {};

        Widgets::PageIndicator pageIndicator;
        Widgets::StatusIcons statusIcons;
      };
    }
  }
}
