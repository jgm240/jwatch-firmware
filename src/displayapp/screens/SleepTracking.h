#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/health/HealthController.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SleepTracking : public Screen {
      public:
        explicit SleepTracking(Controllers::HealthController& healthController);
        ~SleepTracking() override;

        void Refresh() override;

      private:
        Controllers::HealthController& healthController;

        lv_obj_t* label;
        lv_obj_t* phaseLabel;
        lv_obj_t* caveat;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::SleepTracking> {
      static constexpr Apps app = Apps::SleepTracking;
      static constexpr const char* icon = Screens::Symbols::sleep;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SleepTracking(controllers.healthController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
