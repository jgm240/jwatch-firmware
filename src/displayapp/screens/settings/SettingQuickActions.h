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

      class SettingQuickActions : public Screen {
      public:
        SettingQuickActions(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingQuickActions() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int optionsPerScreen = 4;
        static constexpr int optionsCount = 5;
        static constexpr int pagesPerSlot = (optionsCount - 1) / optionsPerScreen + 1;
        static constexpr int nSlots = Controllers::Settings::quickActionSlots;
        static constexpr int nScreens = nSlots * pagesPerSlot;

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
