#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Controllers {
    class MotorController;
    class Settings;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // Full-screen celebration shown once per day, the first tick after Move/Exercise/
      // Stand all reach their goals (see HealthController::ConsumeRingsJustClosed - the
      // check happens on the existing half-hour/hour background ticks, so this can lag up
      // to ~30-60 minutes behind the exact step/minute that closed the last ring). Confetti
      // is a handful of falling lv_obj_t squares moved each tick, this codebase's established
      // animation idiom (see Paddle/Nightstand) rather than LVGL's unused lv_anim system.
      // Plays one non-looping vibration pulse on entry. Auto-dismisses after a few seconds,
      // or immediately on touch/button, same self-contained exit pattern as Nightstand.
      class RingsClosed : public Screen {
      public:
        RingsClosed(DisplayApp* app, Controllers::MotorController& motorController, Controllers::Settings& settingsController);
        ~RingsClosed() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        static constexpr int particleCount = 14;
        static constexpr uint32_t tickPeriodMs = 60;
        static constexpr uint32_t displayDurationMs = 3500;
        static constexpr lv_coord_t particleSize = 6;

        DisplayApp* app;

        std::array<lv_obj_t*, particleCount> particles {};
        std::array<lv_coord_t, particleCount> particleX {};
        std::array<lv_coord_t, particleCount> particleY {};
        std::array<lv_coord_t, particleCount> particleSpeed {};

        uint32_t elapsedMs = 0;

        lv_task_t* taskRefresh;

        void Exit();
      };
    }
  }
}
