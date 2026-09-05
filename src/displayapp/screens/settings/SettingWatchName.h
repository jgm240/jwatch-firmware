#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingWatchName : public Screen {
      public:
        explicit SettingWatchName(Pinetime::Controllers::Settings& settingsController);
        ~SettingWatchName() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
