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

      class SettingActivityGoals : public Screen {
      public:
        SettingActivityGoals(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingActivityGoals() override;

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
