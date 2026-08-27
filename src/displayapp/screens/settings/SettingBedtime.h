#pragma once

#include <cstdint>
#include <memory>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // Shared start/end hours governing both Sleep Tracking's overnight window and
      // Night Mode's schedule, plus Night Mode's own on/off toggle.
      class SettingBedtime : public Screen {
      public:
        SettingBedtime(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingBedtime() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int nScreens = 3;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        DisplayApp* app;
        Controllers::Settings& settingsController;

        ScreenList<nScreens> screens;
      };
    }
  }
}
