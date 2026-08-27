#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class SettingWatchFaceSimpleStyle : public Screen {
      public:
        SettingWatchFaceSimpleStyle(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWatchFaceSimpleStyle() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int optionsPerScreen = 4;
        static constexpr int fontOptionsCount = 3;
        static constexpr int colorOptionsCount = 18;
        static constexpr int fontPages = (fontOptionsCount - 1) / optionsPerScreen + 1;
        static constexpr int colorPages = (colorOptionsCount - 1) / optionsPerScreen + 1;
        static constexpr int nScreens = fontPages + colorPages;

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
