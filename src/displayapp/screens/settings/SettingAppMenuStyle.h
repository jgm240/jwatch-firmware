#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class SettingAppMenuStyle : public Screen {
      public:
        SettingAppMenuStyle(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingAppMenuStyle() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
