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

      // Accent-color picker shared by WatchFaceDigital, WatchFaceAnalog, WatchFaceModular,
      // and WatchFaceHandwriting. Separate from watchFaceSimpleColor, which stays scoped to
      // WatchFaceSimple's own already-shipped settings screen.
      class SettingWatchFaceColor : public Screen {
      public:
        SettingWatchFaceColor(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWatchFaceColor() override;

        bool OnTouchEvent(TouchEvents event) override;

        static constexpr int optionsPerScreen = 4;
        static constexpr int colorOptionsCount = 18;
        // +1 for a trailing "Weather effects" toggle page - not part of the color picker's
        // own pagination, just tacked on the end so it doesn't need its own grid entry.
        static constexpr int nScreens = (colorOptionsCount - 1) / optionsPerScreen + 1 + 1;

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
