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

      class SettingNightstand : public Screen {
      public:
        SettingNightstand(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingNightstand() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);
        void OnPreviewButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        DisplayApp* app;
        CheckboxList checkboxList;
        lv_obj_t* previewButton;
      };
    }
  }
}
