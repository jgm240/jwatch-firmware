#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // A toy, not a real physics engine: no gyro on this hardware, so "gravity" is just
      // the accelerometer's X/Y tilt reading (same signal as the watchface tilt-parallax
      // and Cube complication) reused as a 2D gravity vector. Both modes are the same
      // O(n^2) circle-particle simulation (cheap enough for <=28 particles at a ~50ms tick)
      // with different damping/cohesion constants - Fluid is not a real incompressible-flow
      // solver, just particles with a short-range attract-when-near, repel-when-overlapping
      // rule that makes them clump and slosh instead of bouncing apart like Beads.
      class Physics : public Screen {
      public:
        Physics(Controllers::MotionController& motionController, Controllers::Settings& settingsController);
        ~Physics() override;

        void Refresh() override;

      private:
        enum class Mode : uint8_t { Beads, Fluid };

        static constexpr int maxParticles = 28;
        static constexpr lv_coord_t particleSize = 8;
        static constexpr lv_coord_t simTop = 34;
        static constexpr lv_coord_t simBottom = 200;
        static constexpr uint32_t tickPeriodMs = 50;

        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;

        Mode mode = Mode::Beads;

        std::array<lv_obj_t*, maxParticles> particles {};
        std::array<float, maxParticles> px {};
        std::array<float, maxParticles> py {};
        std::array<float, maxParticles> vx {};
        std::array<float, maxParticles> vy {};

        uint8_t activeCount = 0;

        lv_obj_t* modeButton;
        lv_obj_t* modeLabel;
        lv_obj_t* btnPlus;
        lv_obj_t* btnMinus;
        lv_obj_t* countLabel;

        lv_task_t* taskRefresh;

        void ApplyModeColors();
        void SetActiveCount(uint8_t count);
        void Step();

      public:
        void OnButtonEvent(lv_obj_t* target);
      };
    }

    template <>
    struct AppTraits<Apps::Physics> {
      static constexpr Apps app = Apps::Physics;
      static constexpr const char* icon = Screens::Symbols::dice;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Physics(controllers.motionController, controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
