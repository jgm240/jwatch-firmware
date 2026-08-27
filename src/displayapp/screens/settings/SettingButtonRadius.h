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

      // Two pages: the corner-radius stepper (with a live preview button), and the Glass UI
      // on/off toggle - grouped together since both control how buttons look app-wide.
      class SettingButtonRadius : public Screen {
      public:
        SettingButtonRadius(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingButtonRadius() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int nScreens = 2;

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
