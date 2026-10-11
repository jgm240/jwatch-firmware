#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/Controllers.h"
#include "components/health/HealthController.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "systemtask/WakeLock.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Start/stop workout with continuous heart rate. The watch stays awake while it runs; leaving
      // the screen ends (and saves) the session. JWLink picks the summary up and records it in Health.
      class Workout : public Screen {
      public:
        Workout(Controllers::HealthController& healthController,
                Controllers::HeartRateController& heartRateController,
                Controllers::MotionController& motionController,
                System::SystemTask& systemTask);
        ~Workout() override;

        void Refresh() override;
        void ToggleRunning();

      private:
        void Stop();

        Controllers::HealthController& healthController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        System::WakeLock wakeLock;

        bool hrWasRunning = false;
        TickType_t startTick = 0;
        uint32_t startSteps = 0;

        lv_obj_t* timeLabel;
        lv_obj_t* heartRateLabel;
        lv_obj_t* stepsLabel;
        lv_obj_t* hintLabel;
        lv_obj_t* button;
        lv_obj_t* buttonLabel;
        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Workout> {
      static constexpr Apps app = Apps::Workout;
      static constexpr const char* icon = Screens::Symbols::heartBeat;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Workout(controllers.healthController,
                                    controllers.heartRateController,
                                    controllers.motionController,
                                    *controllers.systemTask);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
