#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
  patternVib = xTimerCreate("patternVib", 1, pdFALSE, this, PatternTick);
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

void MotorController::StartRinging() {
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

bool MotorController::IsRinging() {
  return (xTimerIsTimerActive(longVib) == pdTRUE);
}

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::MotorOn() {
  nrf_gpio_pin_clear(PinMap::Motor);
}

void MotorController::MotorOff() {
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::RunPattern(const PatternData& pattern, bool loop) {
  if (pattern.stepCount == 0) {
    return;
  }
  xTimerStop(patternVib, 0);
  currentPattern = pattern;
  currentStep = 0;
  patternMotorOn = false;
  patternLoop = loop;
  AdvancePattern();
}

void MotorController::StopPattern() {
  xTimerStop(patternVib, 0);
  patternLoop = false;
  MotorOff();
}

bool MotorController::IsPatternRunning() const {
  return xTimerIsTimerActive(patternVib) == pdTRUE;
}

void MotorController::PatternTick(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->AdvancePattern();
}

void MotorController::AdvancePattern() {
  if (patternMotorOn) {
    MotorOff();
    patternMotorOn = false;
    uint16_t gapMs = currentPattern.offMs[currentStep];
    currentStep++;
    if (currentStep >= currentPattern.stepCount) {
      if (!patternLoop) {
        return;
      }
      currentStep = 0;
    }
    if (gapMs == 0) {
      AdvancePattern();
      return;
    }
    xTimerChangePeriod(patternVib, pdMS_TO_TICKS(gapMs), 0);
    xTimerStart(patternVib, 0);
  } else {
    uint16_t onMs = currentPattern.onMs[currentStep];
    if (onMs == 0) {
      patternMotorOn = true;
      AdvancePattern();
      return;
    }
    MotorOn();
    patternMotorOn = true;
    xTimerChangePeriod(patternVib, pdMS_TO_TICKS(onMs), 0);
    xTimerStart(patternVib, 0);
  }
}
