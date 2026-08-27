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

      // Passcode lock is intentionally always reachable to unset even while enabled -
      // PasscodeLock.cpp's own "Forgot code?" erase escape hatch means a lost PIN is
      // recoverable, but changing it here first is obviously less destructive.
      class SettingPasscode : public Screen {
      public:
        SettingPasscode(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingPasscode() override;

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
