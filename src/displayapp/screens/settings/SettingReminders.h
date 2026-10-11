#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingReminders : public Screen {
      public:
        explicit SettingReminders(Pinetime::Controllers::Settings& settingsController);
        ~SettingReminders() override;

        void Toggled(lv_obj_t* object);

      private:
        static constexpr uint8_t optionCount = 3;

        Controllers::Settings& settingsController;
        lv_obj_t* checkboxes[optionCount];
      };
    }
  }
}
