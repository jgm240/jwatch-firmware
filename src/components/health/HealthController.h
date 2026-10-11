#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class MotionController;
    class DateTime;
    class Settings;

    // Background activity/sleep tracker. Driven purely by SystemTask's existing
    // OnNewHalfHour/OnNewHour/OnNewDay wall-clock messages (DateTimeController.cpp),
    // so it never needs a FreeRTOS timer of its own. All state is RAM-only and
    // resets at day/night rollover - there is no historical persistence. The sleep
    // window itself is user-configurable via Settings (the Bedtime screen), shared
    // with Night Mode's own schedule.
    class HealthController {
    public:
      HealthController(MotionController& motionController, DateTime& dateTimeController, Settings& settingsController);

      void OnHalfHourTick();
      void OnHourTick();
      void OnNewDay();

      uint32_t GetActiveMinutesToday() const {
        return activeMinutesToday;
      }

      // Estimated from steps via a fixed average kcal-per-step constant - there is no way
      // to compute real energy expenditure on this hardware (no age/weight/sex profile,
      // and the PPG heart rate sensor isn't sampled continuously in the background).
      uint32_t GetCaloriesToday() const;

      uint32_t GetStandHoursToday() const {
        return standHoursToday;
      }

      uint8_t GetBedtimeEndHour() const;

      // True at most once per half hour: awake hours, no real movement in the first half of this
      // hour, and the day's stand goal not reached yet. Clears itself on read.
      bool ConsumeStandNudge() {
        bool result = standNudgePending;
        standNudgePending = false;
        return result;
      }

      uint32_t GetLastNightStillHalfHours() const {
        return lastNightStillHalfHours;
      }

      // Coarse Light/Deep split of the still half-hours above, by stillness depth. No REM -
      // nothing on this hardware (no HRV/SpO2 sensor) can support detecting it.
      uint32_t GetLastNightLightHalfHours() const {
        return lastNightLightHalfHours;
      }

      uint32_t GetLastNightDeepHalfHours() const {
        return lastNightDeepHalfHours;
      }

      // True at most once per day, the first tick after Move/Exercise/Stand all reach
      // their goals - clears itself on read so the caller (SystemTask) only sees it once.
      // Checked on the existing half-hour/hour ticks, so closing can lag up to ~30-60
      // minutes behind the actual step/minute that closed the last ring.
      bool ConsumeRingsJustClosed() {
        bool result = ringsJustClosed;
        ringsJustClosed = false;
        return result;
      }

    private:
      static constexpr uint32_t stepsPerActiveHalfHour = 200;
      static constexpr int32_t stillShakeSpeedThreshold = 30;
      static constexpr int32_t deepStillShakeSpeedThreshold = 15;

      MotionController& motionController;
      DateTime& dateTimeController;
      Settings& settingsController;

      uint32_t lastStepSnapshot = 0;
      uint32_t activeMinutesToday = 0;
      uint32_t standHoursToday = 0;
      bool movedThisHour = false;

      uint32_t stillHalfHoursTonight = 0;
      uint32_t lastNightStillHalfHours = 0;

      uint32_t lightHalfHoursTonight = 0;
      uint32_t deepHalfHoursTonight = 0;
      uint32_t lastNightLightHalfHours = 0;
      uint32_t lastNightDeepHalfHours = 0;

      bool ringsClosedToday = false;
      bool ringsJustClosed = false;
      bool standNudgePending = false;
      bool wasInSleepWindow = false;

      static uint32_t CurrentDayNumber(DateTime& dateTimeController);

      bool IsWithinSleepWindow() const;
      void CheckRingsClosed();
      void UpdateStreak(bool ringsClosedYesterday);
    };
  }
}
