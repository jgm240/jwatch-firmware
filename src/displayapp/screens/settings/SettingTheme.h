#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class SettingTheme : public Screen {
      public:
        SettingTheme(DisplayApp* app, Pinetime::Controllers::Settings& settingsController, Pinetime::Components::LittleVgl& lvgl);
        ~SettingTheme() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
