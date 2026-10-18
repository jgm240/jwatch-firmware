#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/SmartStack.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // One page of up to 4 independent on/off toggles, covering smartStackCardNames[firstCard..+3].
      // Unlike CheckboxList (used for the watch face/Modular slot pickers), nothing here is mutually
      // exclusive - any number of cards can be on.
      class SmartStackTogglePage : public Screen {
      public:
        static constexpr uint8_t perPage = 4;

        SmartStackTogglePage(uint8_t firstCard, Controllers::Settings& settingsController);
        ~SmartStackTogglePage() override;

        void Toggled(lv_obj_t* object);

      private:
        uint8_t firstCard;
        Controllers::Settings& settingsController;
        std::array<lv_obj_t*, perPage> checkboxes {};
        uint8_t count = 0;
      };

      class SettingSmartStack : public Screen {
      public:
        SettingSmartStack(DisplayApp* app, Controllers::Settings& settingsController);
        ~SettingSmartStack() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        static constexpr uint8_t nScreens = (smartStackCardCount - 1) / SmartStackTogglePage::perPage + 1;

        auto CreateScreenList() const;

        Controllers::Settings& settingsController;
        ScreenList<nScreens> screens;
      };
    }

    template <>
    struct AppTraits<Apps::SettingSmartStack> {
      static constexpr Apps app = Apps::SettingSmartStack;
      static constexpr const char* icon = Screens::Symbols::list;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SettingSmartStack(controllers.displayApp, controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
