#include "components/health/HealthController.h"
#include <chrono>
#include <limits>
#include "components/motion/MotionController.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Controllers;

namespace {
  // Average kcal burned per step for an adult - the closest this hardware can get to a
  // calorie estimate without an age/weight/sex profile or continuous heart-rate sampling.
  constexpr double kcalPerStep = 0.04;

  uint32_t EstimateCalories(uint32_t steps) {
    return static_cast<uint32_t>(static_cast<double>(steps) * kcalPerStep);
  }
}

HealthController::HealthController(MotionController& motionController, DateTime& dateTimeController, Settings& settingsController)
  : motionController {motionController}, dateTimeController {dateTimeController}, settingsController {settingsController} {
}

uint32_t HealthController::GetCaloriesToday() const {
  return EstimateCalories(motionController.NbSteps());
}

uint32_t HealthController::CurrentDayNumber(DateTime& dateTimeController) {
  auto days = std::chrono::time_point_cast<std::chrono::days>(dateTimeController.CurrentDateTime());
  return static_cast<uint32_t>(days.time_since_epoch().count());
}

bool HealthController::IsWithinSleepWindow() const {
  uint8_t hour = dateTimeController.Hours();
  uint8_t start = settingsController.GetBedtimeStartHour();
  uint8_t end = settingsController.GetBedtimeEndHour();
  return hour >= start || hour < end;
}

void HealthController::OnHalfHourTick() {
  uint32_t currentSteps = motionController.NbSteps();
  uint32_t delta = currentSteps >= lastStepSnapshot ? currentSteps - lastStepSnapshot : currentSteps;
  lastStepSnapshot = currentSteps;

  if (delta >= stepsPerActiveHalfHour) {
    activeMinutesToday += 10;
    movedThisHour = true;
  }

  if (IsWithinSleepWindow()) {
    int32_t shakeSpeed = motionController.CurrentShakeSpeed();
    if (shakeSpeed < deepStillShakeSpeedThreshold) {
      stillHalfHoursTonight++;
      deepHalfHoursTonight++;
    } else if (shakeSpeed < stillShakeSpeedThreshold) {
      stillHalfHoursTonight++;
      lightHalfHoursTonight++;
    }
  }

  CheckRingsClosed();
}

void HealthController::OnHourTick() {
  if (movedThisHour) {
    standHoursToday++;
  }
  movedThisHour = false;

  CheckRingsClosed();
}

void HealthController::OnNewDay() {
  // motionController.AdvanceDay() has already run by this point (SystemTask.cpp), so
  // NbSteps(Today) is the new day's (zero); NbSteps(Yesterday) is the day that just ended.
  uint32_t yesterdaySteps = motionController.NbSteps(MotionController::Days::Yesterday);
  Settings::DailyActivity finishedDay {EstimateCalories(yesterdaySteps), standHoursToday, activeMinutesToday};
  settingsController.PushActivityHistoryDay(finishedDay);

  uint32_t pausedRemaining = settingsController.GetPausedDaysRemaining();
  if (pausedRemaining > 0) {
    // Paused day: neutral for the streak - neither extends nor breaks it.
    settingsController.SetPausedDaysRemaining(pausedRemaining - 1);
  } else {
    UpdateStreak(ringsClosedToday);
  }

  activeMinutesToday = 0;
  standHoursToday = 0;
  movedThisHour = false;
  lastStepSnapshot = 0;
  ringsClosedToday = false;
  ringsJustClosed = false;

  lastNightStillHalfHours = stillHalfHoursTonight;
  stillHalfHoursTonight = 0;

  lastNightLightHalfHours = lightHalfHoursTonight;
  lightHalfHoursTonight = 0;
  lastNightDeepHalfHours = deepHalfHoursTonight;
  deepHalfHoursTonight = 0;

  // Deliberately not saving here. This runs from SystemTask at midnight, when the watch is
  // almost always asleep and the SPI-NOR flash is in deep power-down - a flash write then
  // never completes (SpiNorFlash waits on a status bit that never comes), SystemTask stops
  // feeding the watchdog, and the watch reboots with its clock reset to 1 Jan. The changed
  // flag is already set by the setters above; SystemTask flushes it right after this
  // returns if the watch is awake, and otherwise the next save-timer tick / GoToSleep does.
}

void HealthController::UpdateStreak(bool ringsClosedYesterday) {
  if (!ringsClosedYesterday) {
    settingsController.SetCurrentStreak(0);
    return;
  }

  uint32_t dayThatEnded = CurrentDayNumber(dateTimeController) - 1;
  uint32_t lastStreakDay = settingsController.GetStreakLastDayNumber();

  uint32_t newStreak =
    (lastStreakDay != std::numeric_limits<uint32_t>::max() && dayThatEnded == lastStreakDay + 1)
      ? settingsController.GetCurrentStreak() + 1
      : 1;

  settingsController.SetCurrentStreak(newStreak);
  settingsController.SetStreakLastDayNumber(dayThatEnded);
  if (newStreak > settingsController.GetLongestStreak()) {
    settingsController.SetLongestStreak(newStreak);
  }
}

void HealthController::CheckRingsClosed() {
  // Paused hexagons never celebrate or count toward the streak - activity is still tracked (for the history graphs), it's just
  // not judged against goals while paused.
  if (ringsClosedToday || settingsController.GetPausedDaysRemaining() > 0) {
    return;
  }

  uint32_t calories = GetCaloriesToday();
  uint32_t caloriesGoal = settingsController.GetCaloriesGoal();
  uint32_t exerciseGoal = settingsController.GetExerciseGoalMinutes();
  uint32_t standGoal = settingsController.GetStandGoalHours();

  if (calories >= caloriesGoal && activeMinutesToday >= exerciseGoal && standHoursToday >= standGoal) {
    ringsClosedToday = true;
    ringsJustClosed = true;
  }
}
