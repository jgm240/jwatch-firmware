#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/motor/MotorController.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Mindfulness : public Screen {
      public:
        explicit Mindfulness(Controllers::MotorController& motorController);
        ~Mindfulness() override;

        void Refresh() override;

      private:
        enum class Phase { In, Out };

        static constexpr lv_coord_t minDiameter = 60;
        static constexpr lv_coord_t maxDiameter = 200;
        static constexpr uint32_t phaseDurationMs = 4000;
        static constexpr uint32_t tickPeriodMs = 50;

        Controllers::MotorController& motorController;

        Phase phase = Phase::In;
        uint32_t elapsedInPhaseMs = 0;

        lv_obj_t* circle;
        lv_obj_t* label;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Mindfulness> {
      static constexpr Apps app = Apps::Mindfulness;
      static constexpr const char* icon = Screens::Symbols::sun;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Mindfulness(controllers.motorController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
