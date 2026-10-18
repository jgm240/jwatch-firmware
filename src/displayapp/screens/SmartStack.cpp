#include "displayapp/screens/SmartStack.h"
#include <algorithm>
#include <cstdio>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

extern lv_font_t jetbrains_mono_bold_20;
#include "components/battery/BatteryController.h"
#include "components/ble/CalendarEventService.h"
#include "components/ble/MusicService.h"
#include "components/ble/NotificationManager.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/datetime/DateTimeController.h"
#include "components/datetime/WorldTime.h"
#include "components/health/HealthController.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/alarm/AlarmController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // One line of label text, big enough to read at a glance.
  struct CardText {
    const char* icon;
    char value[32];
    char sub[40];
  };

  // Everything a card needs to know about its own content, computed fresh each Refresh(). Centralizing
  // this (rather than spreading it across per-type classes) is what lets SmartStack itself re-use the
  // exact same logic to score relevance - the score and the displayed text come from the same place.
  CardText DescribeCard(SmartStackCardType type, const Pinetime::Applications::AppControllers& c, bool& isRelevant) {
    using namespace Pinetime::Applications::Screens::Symbols;
    CardText t {"", "", ""};
    isRelevant = false;

    switch (type) {
      case SmartStackCardType::Alarm: {
        t.icon = bell;
        if (!c.alarmController.IsEnabled()) {
          std::snprintf(t.value, sizeof(t.value), "No alarm set");
          return t;
        }
        std::snprintf(t.value, sizeof(t.value), "%02d:%02d", c.alarmController.Hours(), c.alarmController.Minutes());
        const uint32_t secs = c.alarmController.SecondsToAlarm();
        if (c.alarmController.IsAlerting()) {
          std::snprintf(t.sub, sizeof(t.sub), "Ringing now");
          isRelevant = true;
        } else if (secs <= 20 * 60) {
          std::snprintf(t.sub, sizeof(t.sub), "in %lu min", static_cast<unsigned long>((secs + 59) / 60));
          isRelevant = true;
        } else {
          std::snprintf(t.sub, sizeof(t.sub), "Alarm");
        }
        return t;
      }
      case SmartStackCardType::Workout: {
        t.icon = heartBeat;
        if (!c.healthController.IsWorkoutRunning()) {
          std::snprintf(t.value, sizeof(t.value), "No workout");
          return t;
        }
        const uint8_t bpm = c.heartRateController.HeartRate();
        if (bpm > 0 && c.heartRateController.State() == Pinetime::Controllers::HeartRateController::States::Running) {
          std::snprintf(t.value, sizeof(t.value), "%d bpm", bpm);
        } else {
          std::snprintf(t.value, sizeof(t.value), "Active");
        }
        std::snprintf(t.sub, sizeof(t.sub), "Workout in progress");
        isRelevant = true;
        return t;
      }
      case SmartStackCardType::NowPlaying: {
        t.icon = music;
        if (c.musicService == nullptr || !c.musicService->isPlaying()) {
          std::snprintf(t.value, sizeof(t.value), "Not playing");
          return t;
        }
        const auto track = c.musicService->getTrack();
        std::snprintf(t.value, sizeof(t.value), "%.31s", track.empty() ? "Playing" : track.c_str());
        const auto artist = c.musicService->getArtist();
        std::snprintf(t.sub, sizeof(t.sub), "%.39s", artist.c_str());
        isRelevant = true;
        return t;
      }
      case SmartStackCardType::NextEvent: {
        t.icon = Pinetime::Applications::Screens::Symbols::list;
        if (c.calendarService == nullptr) {
          std::snprintf(t.value, sizeof(t.value), "No events");
          return t;
        }
        const int64_t nowLocal =
          std::chrono::duration_cast<std::chrono::seconds>(c.dateTimeController.CurrentDateTime().time_since_epoch()).count();
        for (uint8_t i = 0; i < c.calendarService->NbEvents(); i++) {
          const auto& event = c.calendarService->GetEvent(i);
          if (event.title[0] == '>' || event.title[0] == '*') {
            continue; // all-day / reminder items have their own card
          }
          const int64_t startLocal = static_cast<int64_t>(c.dateTimeController.UtcEpochToLocalEpoch(event.start));
          const int64_t endLocal = static_cast<int64_t>(c.dateTimeController.UtcEpochToLocalEpoch(event.end));
          if (std::max(startLocal, endLocal) < nowLocal) {
            continue;
          }
          const std::tm start = c.dateTimeController.ToLocalTime(static_cast<std::time_t>(startLocal));
          std::snprintf(t.value, sizeof(t.value), "%02d:%02d", start.tm_hour, start.tm_min);
          std::snprintf(t.sub, sizeof(t.sub), "%.39s", event.title.data());
          isRelevant = startLocal - nowLocal <= 60 * 60;
          return t;
        }
        std::snprintf(t.value, sizeof(t.value), "No events");
        return t;
      }
      case SmartStackCardType::Countdown: {
        t.icon = Pinetime::Applications::Screens::Symbols::clock;
        if (c.calendarService != nullptr) {
          const int64_t today =
            std::chrono::duration_cast<std::chrono::seconds>(c.dateTimeController.CurrentDateTime().time_since_epoch()).count() / 86400;
          for (uint8_t i = 0; i < c.calendarService->NbEvents(); i++) {
            const auto& event = c.calendarService->GetEvent(i);
            if (event.title[0] != '>' || event.title[1] != ' ') {
              continue;
            }
            const int64_t eventDay = static_cast<int64_t>(c.dateTimeController.UtcEpochToLocalEpoch(event.start)) / 86400;
            if (eventDay < today) {
              continue;
            }
            const int64_t days = eventDay - today;
            if (days == 0) {
              std::snprintf(t.value, sizeof(t.value), "Today");
              isRelevant = true;
            } else {
              std::snprintf(t.value, sizeof(t.value), "%lld day%s", static_cast<long long>(days), days == 1 ? "" : "s");
            }
            std::snprintf(t.sub, sizeof(t.sub), "%.39s", event.title.data() + 2);
            return t;
          }
        }
        std::snprintf(t.value, sizeof(t.value), "Nothing upcoming");
        return t;
      }
      case SmartStackCardType::Notifications: {
        t.icon = notificationsOn;
        const size_t n = c.notificationManager.NbNotifications();
        if (n == 0) {
          t.icon = notificationsOff;
          std::snprintf(t.value, sizeof(t.value), "No notifications");
          return t;
        }
        std::snprintf(t.value, sizeof(t.value), "%zu unread", n);
        std::snprintf(t.sub, sizeof(t.sub), "Notifications");
        isRelevant = true;
        return t;
      }
      case SmartStackCardType::WatchBattery: {
        t.icon = batteryHalf;
        const uint8_t percent = c.batteryController.PercentRemaining();
        std::snprintf(t.value, sizeof(t.value), "%d%%", percent);
        std::snprintf(t.sub, sizeof(t.sub), "%s", c.batteryController.IsCharging() ? "Watch battery - charging" : "Watch battery");
        isRelevant = percent <= 20 && !c.batteryController.IsCharging();
        return t;
      }
      case SmartStackCardType::PhoneBattery: {
        t.icon = batteryHalf;
        if (c.calendarService == nullptr ||
            c.calendarService->PhoneBatteryPercent() == Pinetime::Controllers::CalendarEventService::PhoneBatteryUnknown) {
          std::snprintf(t.value, sizeof(t.value), "Unknown");
          std::snprintf(t.sub, sizeof(t.sub), "Phone battery");
          return t;
        }
        const uint8_t percent = c.calendarService->PhoneBatteryPercent();
        std::snprintf(t.value, sizeof(t.value), "%d%%", percent);
        std::snprintf(t.sub, sizeof(t.sub), "%s", c.calendarService->PhoneCharging() ? "Phone battery - charging" : "Phone battery");
        isRelevant = percent <= 15 && !c.calendarService->PhoneCharging();
        return t;
      }
      case SmartStackCardType::Steps: {
        t.icon = shoe;
        const uint32_t steps = c.motionController.NbSteps();
        const uint32_t goal = c.settingsController.GetStepsGoal();
        std::snprintf(t.value, sizeof(t.value), "%lu", static_cast<unsigned long>(steps));
        std::snprintf(t.sub, sizeof(t.sub), "of %lu steps", static_cast<unsigned long>(goal));
        isRelevant = goal > 0 && steps * 100 / goal >= 80 && steps < goal && c.dateTimeController.Hours() >= 17;
        return t;
      }
      case SmartStackCardType::HeartRate: {
        t.icon = heartBeat;
        if (c.heartRateController.State() == Pinetime::Controllers::HeartRateController::States::Running) {
          std::snprintf(t.value, sizeof(t.value), "%d bpm", c.heartRateController.HeartRate());
        } else {
          std::snprintf(t.value, sizeof(t.value), "-- bpm");
        }
        std::snprintf(t.sub, sizeof(t.sub), "Heart rate");
        return t;
      }
      case SmartStackCardType::Weather: {
        t.icon = cloudSunRain;
        if (c.weatherController == nullptr) {
          std::snprintf(t.value, sizeof(t.value), "No data");
          std::snprintf(t.sub, sizeof(t.sub), "Weather");
          return t;
        }
        const auto current = c.weatherController->Current();
        if (!current.has_value()) {
          std::snprintf(t.value, sizeof(t.value), "No data");
          std::snprintf(t.sub, sizeof(t.sub), "Weather");
          return t;
        }
        const bool imperial = c.settingsController.GetWeatherFormat() == Pinetime::Controllers::Settings::WeatherFormat::Imperial;
        const int16_t temp = imperial ? current->temperature.Fahrenheit() : current->temperature.Celsius();
        std::snprintf(t.value, sizeof(t.value), "%d\xC2\xB0%c", temp, imperial ? 'F' : 'C');
        std::snprintf(t.sub, sizeof(t.sub), "Weather");
        return t;
      }
      case SmartStackCardType::WorldClock: {
        t.icon = Symbols::clock;
        const auto& city = Pinetime::Controllers::worldCities[c.settingsController.GetWorldClockCity(0) %
                                                              Pinetime::Controllers::worldCityCount];
        const int64_t utc =
          std::chrono::duration_cast<std::chrono::seconds>(c.dateTimeController.UTCDateTime().time_since_epoch()).count();
        const int64_t local = utc + static_cast<int64_t>(Pinetime::Controllers::WorldOffsetMinutes(city, utc)) * 60;
        const int64_t secondsToday = local - Pinetime::Controllers::DayNumber(local) * 86400;
        std::snprintf(t.value, sizeof(t.value), "%02d:%02d", static_cast<int>(secondsToday / 3600), static_cast<int>((secondsToday % 3600) / 60));
        std::snprintf(t.sub, sizeof(t.sub), "%.39s", city.name);
        return t;
      }
      case SmartStackCardType::Sun: {
        t.icon = sun;
        if (c.weatherController != nullptr) {
          const auto current = c.weatherController->Current();
          if (current.has_value() && current->sunrise >= 0 && current->sunset >= 0) {
            const int32_t now = c.dateTimeController.Hours() * 60 + c.dateTimeController.Minutes();
            const bool isSunrise = now < current->sunrise || now >= current->sunset;
            const int32_t minutes = isSunrise ? current->sunrise : current->sunset;
            std::snprintf(t.value, sizeof(t.value), "%02d:%02d", static_cast<int>(minutes / 60), static_cast<int>(minutes % 60));
            std::snprintf(t.sub, sizeof(t.sub), "%s", isSunrise ? "Sunrise" : "Sunset");
            return t;
          }
        }
        std::snprintf(t.value, sizeof(t.value), "No data");
        std::snprintf(t.sub, sizeof(t.sub), "Sun");
        return t;
      }
      case SmartStackCardType::Date: {
        t.icon = Symbols::clock;
        std::snprintf(t.value, sizeof(t.value), "%s %d", c.dateTimeController.MonthShortToString(), c.dateTimeController.Day());
        std::snprintf(t.sub, sizeof(t.sub), "%s", c.dateTimeController.DayOfWeekToString());
        isRelevant = true; // always a safe, always-available fallback card
        return t;
      }
      case SmartStackCardType::FitnessRings: {
        t.icon = Symbols::tachometer;
        const uint32_t calories = c.healthController.GetCaloriesToday();
        const uint32_t activeMinutes = c.healthController.GetActiveMinutesToday();
        const uint32_t exerciseGoal = c.settingsController.GetExerciseGoalMinutes();
        const uint32_t standHours = c.healthController.GetStandHoursToday();
        std::snprintf(t.value, sizeof(t.value), "%lu/%lu/%lu", static_cast<unsigned long>(activeMinutes),
                      static_cast<unsigned long>(standHours), static_cast<unsigned long>(calories));
        std::snprintf(t.sub, sizeof(t.sub), "min / hrs stand / kcal");
        isRelevant = exerciseGoal > 0 && activeMinutes * 100 / exerciseGoal >= 80 && activeMinutes < exerciseGoal;
        return t;
      }
      case SmartStackCardType::Timer: {
        t.icon = Symbols::hourGlass;
        const auto state = c.timer.GetTimerState();
        if (!state.has_value() || state->expired) {
          std::snprintf(t.value, sizeof(t.value), "No timer");
          return t;
        }
        const auto totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(state->distanceToExpiry).count();
        std::snprintf(t.value, sizeof(t.value), "%02lld:%02lld", static_cast<long long>(totalSeconds / 60),
                      static_cast<long long>(totalSeconds % 60));
        std::snprintf(t.sub, sizeof(t.sub), "Timer");
        isRelevant = totalSeconds <= 60;
        return t;
      }
    }
    return t;
  }
}

const std::array<const char*, Pinetime::Applications::Screens::smartStackCardCount>
  Pinetime::Applications::Screens::smartStackCardNames = {
    "Alarm",      "Workout",       "Now playing", "Next event", "Countdown",     "Notifications", "Watch battery",
    "Phone battery", "Steps",      "Heart rate",  "Weather",    "World clock",   "Sun",           "Date",
    "Fitness hexagons", "Timer",
};

SmartStackCard::SmartStackCard(SmartStackCardType type, const AppControllers& controllers) : type {type}, controllers {controllers} {
  iconLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(iconLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(iconLabel, "");
  lv_obj_align(iconLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 16);

  valueLabel = lv_label_create(lv_scr_act(), nullptr);
  // jetbrains_mono_42 only has a handful of glyphs (digits, ": . + - % ° C F K-M f i k m t") - fine
  // for a bare clock face, but most cards here show real words, so use a font with the full alphabet.
  lv_obj_set_style_local_text_font(valueLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_DOT);
  lv_obj_set_width(valueLabel, LV_HOR_RES - 20);
  lv_label_set_align(valueLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(valueLabel, "");
  lv_obj_align(valueLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);

  subLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(subLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(subLabel, LV_LABEL_LONG_DOT);
  lv_obj_set_width(subLabel, LV_HOR_RES - 20);
  lv_label_set_align(subLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(subLabel, "");
  lv_obj_align(subLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);

  taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
  Refresh();
}

SmartStackCard::~SmartStackCard() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void SmartStackCard::Refresh() {
  bool relevant = false;
  const CardText text = DescribeCard(type, controllers, relevant);
  lv_label_set_text(iconLabel, text.icon);
  lv_label_set_text(valueLabel, text.value);
  lv_obj_realign(valueLabel);
  lv_label_set_text(subLabel, text.sub);
  lv_obj_realign(subLabel);
  lastState = relevant;
}

namespace {
  // Most time-sensitive card type first; boosted further (see the constructor) when it actually has
  // something relevant to show right now.
  constexpr std::array<int, smartStackCardCount> basePriority = {
    90, // Alarm
    85, // Workout
    70, // NowPlaying
    65, // NextEvent
    40, // Countdown
    55, // Notifications
    30, // WatchBattery
    28, // PhoneBattery
    20, // Steps
    10, // HeartRate
    15, // Weather
    5,  // WorldClock
    8,  // Sun
    1,  // Date
    35, // FitnessRings
    45, // Timer
  };
}

SmartStack::SmartStack(const AppControllers& controllers) : controllers {controllers}, app {controllers.displayApp} {
  std::array<int, smartStackCardCount> score {};
  std::array<size_t, smartStackCardCount> indices {};
  size_t enabledCount = 0;
  for (size_t i = 0; i < smartStackCardCount; i++) {
    if (!controllers.settingsController.GetSmartStackCardEnabled(i)) {
      continue;
    }
    bool relevant = false;
    DescribeCard(static_cast<SmartStackCardType>(i), controllers, relevant);
    score[i] = basePriority[i] + (relevant ? 100 : 0);
    indices[enabledCount++] = i;
  }
  // A disabled-everything settings file shouldn't leave the stack with nothing to show.
  if (enabledCount == 0) {
    indices[0] = static_cast<size_t>(SmartStackCardType::Date);
    enabledCount = 1;
  }

  std::sort(indices.begin(), indices.begin() + enabledCount, [&score](size_t a, size_t b) { return score[a] > score[b]; });
  activeCount = static_cast<uint8_t>(enabledCount);
  for (size_t i = 0; i < enabledCount; i++) {
    order[i] = static_cast<SmartStackCardType>(indices[i]);
  }

  Show(0);
}

SmartStack::~SmartStack() {
  current.reset();
  lv_obj_clean(lv_scr_act());
}

void SmartStack::Show(uint8_t newIndex) {
  index = newIndex;
  current.reset(); // destroy the old card's LVGL objects before the new one creates its own
  current = std::make_unique<SmartStackCard>(order[index], controllers);
}

bool SmartStack::OnTouchEvent(TouchEvents event) {
  if (current->OnTouchEvent(event)) {
    return true;
  }
  if (event == TouchEvents::SwipeLeft && index + 1 < activeCount) {
    app->SetFullRefresh(DisplayApp::FullRefreshDirections::None);
    Show(index + 1);
    return true;
  }
  if (event == TouchEvents::SwipeRight && index > 0) {
    app->SetFullRefresh(DisplayApp::FullRefreshDirections::None);
    Show(index - 1);
    return true;
  }
  return false;
}
