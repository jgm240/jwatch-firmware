#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class SettingVibration : public Screen {
      public:
        SettingVibration(DisplayApp* app, Pinetime::Controllers::Settings& settingsController, Pinetime::Controllers::MotorController& motorController);
        ~SettingVibration() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int optionsPerScreen = 4;
        static constexpr int optionsCount = 6;
        static constexpr int nScreens = (optionsCount - 1) / optionsPerScreen + 1;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        DisplayApp* app;
        Controllers::Settings& settingsController;
        Controllers::MotorController& motorController;

        ScreenList<nScreens> screens;
      };
    }
  }
}
