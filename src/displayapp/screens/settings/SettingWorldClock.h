#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/datetime/WorldTime.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Pick the cities of the World clock: one roller, a slot selector on top.
      class SettingWorldClock : public Screen {
      public:
        explicit SettingWorldClock(Pinetime::Controllers::Settings& settingsController);
        ~SettingWorldClock() override;

        void OnEvent(lv_obj_t* object, lv_event_t event);

      private:
        void ShowSlot();

        Pinetime::Controllers::Settings& settingsController;
        uint8_t slot = 0;
        char options[Pinetime::Controllers::worldCityCount * 13];

        lv_obj_t* slotLabel;
        lv_obj_t* prevButton;
        lv_obj_t* nextButton;
        lv_obj_t* roller;
      };
    }
  }
}
