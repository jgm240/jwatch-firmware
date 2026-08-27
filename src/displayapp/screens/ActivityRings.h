#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/health/HealthController.h"
#include "components/settings/Settings.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class ActivityRings : public Screen {
      public:
        ActivityRings(DisplayApp* app, Controllers::HealthController& healthController, Controllers::Settings& settingsController);
        ~ActivityRings() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        Controllers::HealthController& healthController;
        Controllers::Settings& settingsController;

        lv_obj_t* moveArc;
        lv_obj_t* exerciseArc;
        lv_obj_t* standArc;
        lv_obj_t* label;
        lv_obj_t* streakLabel;
        lv_obj_t* hintLabel;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::ActivityRings> {
      static constexpr Apps app = Apps::ActivityRings;
      static constexpr const char* icon = Screens::Symbols::tachometer;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ActivityRings(controllers.displayApp, controllers.healthController, controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
