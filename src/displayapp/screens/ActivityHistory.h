#pragma once

#include <cstdint>
#include <memory>
#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "components/health/HealthController.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // Three swipeable pages (Calories / Exercise / Stand), each a 7-bar lv_chart: the
      // last activityHistoryDays completed days (Settings::GetActivityHistoryDay, persisted
      // through reboots) plus today's own in-progress total read live from HealthController.
      class ActivityHistory : public Screen {
      public:
        ActivityHistory(DisplayApp* app, Controllers::HealthController& healthController, Controllers::Settings& settingsController);
        ~ActivityHistory() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int nScreens = 3;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        DisplayApp* app;
        Controllers::HealthController& healthController;
        Controllers::Settings& settingsController;

        ScreenList<nScreens> screens;
      };
    }
  }
}
