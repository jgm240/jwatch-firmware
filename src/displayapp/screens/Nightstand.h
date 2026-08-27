#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/brightness/BrightnessController.h"
#include "systemtask/WakeLock.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // Large always-visible time display shown while charging (if enabled in Settings),
      // with the backlight dimmed purely for nighttime comfort - PineTime's LCD has no
      // OLED-style burn-in risk, so this isn't a burn-in mitigation. Holds a WakeLock so
      // DisplayApp's normal idle-dim-then-sleep sequence never kicks in while showing.
      // A small shape screensaver animates behind the time - one shape at a time shrinks
      // to nothing, then the next shape in the cycle grows in its place.
      class Nightstand : public Screen {
      public:
        Nightstand(DisplayApp* app,
                   System::SystemTask& systemTask,
                   Controllers::DateTime& dateTimeController,
                   const Controllers::Battery& batteryController,
                   Controllers::BrightnessController& brightnessController);
        ~Nightstand() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        enum class Shape : uint8_t { Circle, Square, Triangle };
        enum class Phase : uint8_t { Growing, Shrinking };

        static constexpr lv_coord_t canvasSize = 64;
        static constexpr lv_coord_t minShapeSize = 16;
        static constexpr uint32_t phaseDurationMs = 4000;
        static constexpr uint32_t tickPeriodMs = 100;

        DisplayApp* app;
        System::WakeLock wakeLock;
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        Controllers::BrightnessController& brightnessController;

        Controllers::BrightnessController::Levels previousBrightness;

        lv_obj_t* label_time;
        lv_obj_t* canvas;

        Shape currentShape = Shape::Circle;
        Phase phase = Phase::Growing;
        uint32_t elapsedInPhaseMs = 0;

        lv_task_t* taskRefresh;

        uint8_t canvasBuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(canvasSize, canvasSize)];

        void UpdateShape();
        void Exit();
      };
    }
  }
}
