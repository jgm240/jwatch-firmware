#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/motor/VibrationPatterns.h"

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController() = default;

      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      void StopRinging();
      bool IsRinging();

      void MotorOn();
      void MotorOff();
      void RunPattern(const PatternData& pattern, bool loop = false);
      void StopPattern();
      bool IsPatternRunning() const;

    private:
      static void Ring(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      static void PatternTick(TimerHandle_t xTimer);
      void AdvancePattern();

      TimerHandle_t shortVib;
      TimerHandle_t longVib;
      TimerHandle_t patternVib;

      PatternData currentPattern;
      uint8_t currentStep = 0;
      bool patternMotorOn = false;
      bool patternLoop = false;
    };
  }
}
