#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      // Apple Watch-style "Pause Rings": stepping the value up and it takes effect
      // immediately (same live-apply convention as every other stepper in this codebase -
      // SettingSteps, SettingTiltParallax, etc.). 0 = not paused / resumed.
      class SettingPauseRings : public Screen {
      public:
        SettingPauseRings(Pinetime::Controllers::Settings& settingsController);
        ~SettingPauseRings() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;

        lv_obj_t* daysValue;
        lv_obj_t* statusLabel;
        lv_obj_t* btnPlus;
        lv_obj_t* btnMinus;

        void Redraw();
      };
    }
  }
}
