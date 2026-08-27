#pragma once
#include <array>
#include <cstdint>
#include <bitset>
#include <limits>
#include <optional>
#include "components/brightness/BrightnessController.h"
#include "components/fs/FS.h"
#include "components/motor/VibrationPatterns.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/widgets/complications/Complication.h"
#include "displayapp/widgets/QuickAction.h"
#include <nrf_log.h>

namespace Pinetime {
  namespace Controllers {
    class Settings {
    public:
      enum class ClockType : uint8_t { H24, H12 };
      enum class WeatherFormat : uint8_t { Metric, Imperial };
      enum class Notification : uint8_t { On, Off, Sleep };
      enum class ChimesOption : uint8_t { None, Hours, HalfHours };
      enum class WakeUpMode : uint8_t { SingleTap = 0, DoubleTap = 1, RaiseWrist = 2, Shake = 3, LowerWrist = 4 };
      enum class AppMenuStyle : uint8_t { Grid, Circle, List };
      enum class Colors : uint8_t {
        White,
        Silver,
        Gray,
        Black,
        Red,
        Maroon,
        Yellow,
        Olive,
        Lime,
        Green,
        Cyan,
        Teal,
        Blue,
        Navy,
        Magenta,
        Purple,
        Orange,
        Pink
      };
      enum class PTSGaugeStyle : uint8_t { Full, Half, Numeric };
      enum class PTSWeather : uint8_t { On, Off };
      enum class PrideFlag : uint8_t { Gay, Trans, Bi, Lesbian };
      enum class DfuAndFsMode : uint8_t { Disabled, Enabled, EnabledTillReboot };
      enum class WatchFaceSimpleFont : uint8_t { ExtraBoldCompressed, Light, Mono76 };

      struct PineTimeStyle {
        Colors ColorTime = Colors::Teal;
        Colors ColorBar = Colors::Teal;
        Colors ColorBG = Colors::Black;
        PTSGaugeStyle gaugeStyle = PTSGaugeStyle::Full;
        PTSWeather weatherEnable = PTSWeather::Off;
      };

      struct WatchFaceInfineat {
        bool showSideCover = true;
        int colorIndex = 0;
      };

      // 6 slots: 4 corners plus one flanking each side of the time.
      static constexpr size_t modularWatchFaceSlots = 6;

      struct WatchFaceModular {
        std::array<Pinetime::Applications::Widgets::ComplicationType, modularWatchFaceSlots> slots = {
          Pinetime::Applications::Widgets::ComplicationType::Battery,
          Pinetime::Applications::Widgets::ComplicationType::Steps,
          Pinetime::Applications::Widgets::ComplicationType::Weather,
          Pinetime::Applications::Widgets::ComplicationType::Alarm,
          Pinetime::Applications::Widgets::ComplicationType::HeartRate,
          Pinetime::Applications::Widgets::ComplicationType::Date,
        };
      };

      static constexpr size_t quickActionSlots = 4;

      struct QuickActionsLayout {
        std::array<Pinetime::Applications::Widgets::QuickAction, quickActionSlots> slots = {
          Pinetime::Applications::Widgets::QuickAction::Brightness,
          Pinetime::Applications::Widgets::QuickAction::Flashlight,
          Pinetime::Applications::Widgets::QuickAction::NotificationToggle,
          Pinetime::Applications::Widgets::QuickAction::OpenSettings,
        };
      };

      Settings(Pinetime::Controllers::FS& fs);

      Settings(const Settings&) = delete;
      Settings& operator=(const Settings&) = delete;
      Settings(Settings&&) = delete;
      Settings& operator=(Settings&&) = delete;

      void Init();
      void SaveSettings();

      void SetWatchFace(Pinetime::Applications::WatchFace face) {
        if (face != settings.watchFace) {
          settingsChanged = true;
        }
        settings.watchFace = face;
      };

      Pinetime::Applications::WatchFace GetWatchFace() const {
        return settings.watchFace;
      };

      void SetChimeOption(ChimesOption chimeOption) {
        if (chimeOption != settings.chimesOption) {
          settingsChanged = true;
        }
        settings.chimesOption = chimeOption;
      };

      ChimesOption GetChimeOption() const {
        return settings.chimesOption;
      };

      void SetPTSColorTime(Colors colorTime) {
        if (colorTime != settings.PTS.ColorTime)
          settingsChanged = true;
        settings.PTS.ColorTime = colorTime;
      };

      Colors GetPTSColorTime() const {
        return settings.PTS.ColorTime;
      };

      void SetPTSColorBar(Colors colorBar) {
        if (colorBar != settings.PTS.ColorBar)
          settingsChanged = true;
        settings.PTS.ColorBar = colorBar;
      };

      Colors GetPTSColorBar() const {
        return settings.PTS.ColorBar;
      };

      void SetPTSColorBG(Colors colorBG) {
        if (colorBG != settings.PTS.ColorBG)
          settingsChanged = true;
        settings.PTS.ColorBG = colorBG;
      };

      Colors GetPTSColorBG() const {
        return settings.PTS.ColorBG;
      };

      void SetInfineatShowSideCover(bool show) {
        if (show != settings.watchFaceInfineat.showSideCover) {
          settings.watchFaceInfineat.showSideCover = show;
          settingsChanged = true;
        }
      };

      bool GetInfineatShowSideCover() const {
        return settings.watchFaceInfineat.showSideCover;
      };

      void SetInfineatColorIndex(int index) {
        if (index != settings.watchFaceInfineat.colorIndex) {
          settings.watchFaceInfineat.colorIndex = index;
          settingsChanged = true;
        }
      };

      int GetInfineatColorIndex() const {
        return settings.watchFaceInfineat.colorIndex;
      };

      void SetPTSGaugeStyle(PTSGaugeStyle gaugeStyle) {
        if (gaugeStyle != settings.PTS.gaugeStyle)
          settingsChanged = true;
        settings.PTS.gaugeStyle = gaugeStyle;
      };

      PTSGaugeStyle GetPTSGaugeStyle() const {
        return settings.PTS.gaugeStyle;
      };

      void SetPTSWeather(PTSWeather weatherEnable) {
        if (weatherEnable != settings.PTS.weatherEnable)
          settingsChanged = true;
        settings.PTS.weatherEnable = weatherEnable;
      };

      PTSWeather GetPTSWeather() const {
        return settings.PTS.weatherEnable;
      };

      void SetPrideFlag(PrideFlag prideFlag) {
        if (prideFlag != settings.prideFlag)
          settingsChanged = true;
        settings.prideFlag = prideFlag;
      };

      PrideFlag GetPrideFlag() const {
        return settings.prideFlag;
      };

      void SetAppMenu(uint8_t menu) {
        appMenu = menu;
      };

      uint8_t GetAppMenu() const {
        return appMenu;
      };

      void SetSettingsMenu(uint8_t menu) {
        settingsMenu = menu;
      };

      uint8_t GetSettingsMenu() const {
        return settingsMenu;
      };

      void SetClockType(ClockType clocktype) {
        if (clocktype != settings.clockType) {
          settingsChanged = true;
        }
        settings.clockType = clocktype;
      };

      ClockType GetClockType() const {
        return settings.clockType;
      };

      void SetWeatherFormat(WeatherFormat weatherFormat) {
        if (weatherFormat != settings.weatherFormat) {
          settingsChanged = true;
        }
        settings.weatherFormat = weatherFormat;
      };

      WeatherFormat GetWeatherFormat() const {
        return settings.weatherFormat;
      };

      void SetNotificationStatus(Notification status) {
        if (status != settings.notificationStatus) {
          settingsChanged = true;
        }
        settings.notificationStatus = status;
      };

      Notification GetNotificationStatus() const {
        return settings.notificationStatus;
      };

      void SetScreenTimeOut(uint32_t timeout) {
        if (timeout != settings.screenTimeOut) {
          settingsChanged = true;
        }
        settings.screenTimeOut = timeout;
      };

      uint32_t GetScreenTimeOut() const {
        return settings.screenTimeOut;
      };

      bool GetAlwaysOnDisplay() const {
        return settings.alwaysOnDisplay && GetNotificationStatus() != Notification::Sleep;
      };

      void SetAlwaysOnDisplaySetting(bool state) {
        if (state != settings.alwaysOnDisplay) {
          settingsChanged = true;
        }
        settings.alwaysOnDisplay = state;
      }

      bool GetAlwaysOnDisplaySetting() const {
        return settings.alwaysOnDisplay;
      }

      void SetShakeThreshold(uint16_t thresh) {
        if (settings.shakeWakeThreshold != thresh) {
          settings.shakeWakeThreshold = thresh;
          settingsChanged = true;
        }
      }

      int16_t GetShakeThreshold() const {
        return settings.shakeWakeThreshold;
      }

      void setWakeUpMode(WakeUpMode wakeUp, bool enabled) {
        if (enabled != isWakeUpModeOn(wakeUp)) {
          settingsChanged = true;
        }
        settings.wakeUpMode.set(static_cast<size_t>(wakeUp), enabled);
        // Handle special behavior
        if (enabled) {
          switch (wakeUp) {
            case WakeUpMode::SingleTap:
              settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::DoubleTap), false);
              break;
            case WakeUpMode::DoubleTap:
              settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::SingleTap), false);
              break;
            default:
              break;
          }
        }
      };

      std::bitset<5> getWakeUpModes() const {
        return settings.wakeUpMode;
      }

      bool isWakeUpModeOn(const WakeUpMode mode) const {
        return getWakeUpModes()[static_cast<size_t>(mode)];
      }

      void SetBrightness(Controllers::BrightnessController::Levels level) {
        if (level != settings.brightLevel) {
          settingsChanged = true;
        }
        settings.brightLevel = level;
      };

      Controllers::BrightnessController::Levels GetBrightness() const {
        return settings.brightLevel;
      };

      void SetStepsGoal(uint32_t goal) {
        if (goal != settings.stepsGoal) {
          settingsChanged = true;
        }
        settings.stepsGoal = goal;
      };

      uint32_t GetStepsGoal() const {
        return settings.stepsGoal;
      };

      void SetBleRadioEnabled(bool enabled) {
        bleRadioEnabled = enabled;
      };

      bool GetBleRadioEnabled() const {
        return bleRadioEnabled;
      };

      void SetDfuAndFsMode(DfuAndFsMode mode) {
        if (mode == GetDfuAndFsMode()) {
          return;
        }
        if (mode == DfuAndFsMode::Enabled || GetDfuAndFsMode() == DfuAndFsMode::Enabled) {
          settingsChanged = true;
        }
        settings.dfuAndFsEnabledOnBoot = (mode == DfuAndFsMode::Enabled);
        dfuAndFsEnabledTillReboot = (mode == DfuAndFsMode::EnabledTillReboot);
      };

      DfuAndFsMode GetDfuAndFsMode() {
        if (dfuAndFsEnabledTillReboot) {
          if (settings.dfuAndFsEnabledOnBoot) { // ensure both variables are in consistent state
            settingsChanged = true;
            settings.dfuAndFsEnabledOnBoot = false;
            NRF_LOG_ERROR("Settings: DfuAndFsMode data corrupted");
          }
          return DfuAndFsMode::EnabledTillReboot;
        }
        return (settings.dfuAndFsEnabledOnBoot ? DfuAndFsMode::Enabled : DfuAndFsMode::Disabled);
      };


      void SetVibrationPattern(Controllers::VibrationPattern pattern) {
        if (pattern != settings.selectedVibrationPattern) {
          settingsChanged = true;
        }
        settings.selectedVibrationPattern = pattern;
      };

      Controllers::VibrationPattern GetVibrationPattern() const {
        return settings.selectedVibrationPattern;
      };

      void SetCustomVibrationPattern(const Controllers::PatternData& pattern) {
        settingsChanged = true;
        settings.customVibrationPattern = pattern;
      };

      const Controllers::PatternData& GetCustomVibrationPattern() const {
        return settings.customVibrationPattern;
      };

      void SetModularSlot(size_t index, Pinetime::Applications::Widgets::ComplicationType type) {
        if (index >= modularWatchFaceSlots) {
          return;
        }
        if (type != settings.watchFaceModular.slots[index]) {
          settingsChanged = true;
        }
        settings.watchFaceModular.slots[index] = type;
      };

      Pinetime::Applications::Widgets::ComplicationType GetModularSlot(size_t index) const {
        if (index >= modularWatchFaceSlots) {
          return Pinetime::Applications::Widgets::ComplicationType::None;
        }
        return settings.watchFaceModular.slots[index];
      };

      void SetQuickAction(size_t index, Pinetime::Applications::Widgets::QuickAction action) {
        if (index >= quickActionSlots) {
          return;
        }
        if (action != settings.quickActionsLayout.slots[index]) {
          settingsChanged = true;
        }
        settings.quickActionsLayout.slots[index] = action;
      };

      Pinetime::Applications::Widgets::QuickAction GetQuickAction(size_t index) const {
        if (index >= quickActionSlots) {
          return Pinetime::Applications::Widgets::QuickAction::Brightness;
        }
        return settings.quickActionsLayout.slots[index];
      };

      void SetWatchFaceSimpleFont(WatchFaceSimpleFont font) {
        if (font != settings.watchFaceSimpleFont) {
          settingsChanged = true;
        }
        settings.watchFaceSimpleFont = font;
      };

      WatchFaceSimpleFont GetWatchFaceSimpleFont() const {
        return settings.watchFaceSimpleFont;
      };

      void SetWatchFaceSimpleColor(Colors color) {
        if (color != settings.watchFaceSimpleColor) {
          settingsChanged = true;
        }
        settings.watchFaceSimpleColor = color;
      };

      Colors GetWatchFaceSimpleColor() const {
        return settings.watchFaceSimpleColor;
      };

      void SetAppMenuStyle(AppMenuStyle style) {
        if (style != settings.appMenuStyle) {
          settingsChanged = true;
        }
        settings.appMenuStyle = style;
      };

      AppMenuStyle GetAppMenuStyle() const {
        return settings.appMenuStyle;
      };

      void SetNightstandModeEnabled(bool enabled) {
        if (enabled != settings.nightstandModeEnabled) {
          settingsChanged = true;
        }
        settings.nightstandModeEnabled = enabled;
      };

      bool GetNightstandModeEnabled() const {
        return settings.nightstandModeEnabled;
      };

      void SetWatchFaceAccentColor(Colors color) {
        if (color != settings.watchFaceAccentColor) {
          settingsChanged = true;
        }
        settings.watchFaceAccentColor = color;
      };

      Colors GetWatchFaceAccentColor() const {
        return settings.watchFaceAccentColor;
      };

      void SetButtonCornerRadius(uint8_t radius) {
        if (radius > 40) {
          radius = 40;
        }
        if (radius != settings.buttonCornerRadius) {
          settingsChanged = true;
        }
        settings.buttonCornerRadius = radius;
      };

      uint8_t GetButtonCornerRadius() const {
        return settings.buttonCornerRadius;
      };

      void SetExerciseGoalMinutes(uint32_t minutes) {
        if (minutes != settings.exerciseGoalMinutes) {
          settingsChanged = true;
        }
        settings.exerciseGoalMinutes = minutes;
      };

      uint32_t GetExerciseGoalMinutes() const {
        return settings.exerciseGoalMinutes;
      };

      void SetStandGoalHours(uint32_t hours) {
        if (hours != settings.standGoalHours) {
          settingsChanged = true;
        }
        settings.standGoalHours = hours;
      };

      uint32_t GetStandGoalHours() const {
        return settings.standGoalHours;
      };

      void SetCaloriesGoal(uint32_t kcal) {
        if (kcal != settings.caloriesGoal) {
          settingsChanged = true;
        }
        settings.caloriesGoal = kcal;
      };

      uint32_t GetCaloriesGoal() const {
        return settings.caloriesGoal;
      };

      // Streak bookkeeping - written by HealthController::OnNewDay(), not user-editable.
      uint32_t GetCurrentStreak() const {
        return settings.currentStreak;
      };

      void SetCurrentStreak(uint32_t days) {
        if (days != settings.currentStreak) {
          settingsChanged = true;
        }
        settings.currentStreak = days;
      };

      uint32_t GetLongestStreak() const {
        return settings.longestStreak;
      };

      void SetLongestStreak(uint32_t days) {
        if (days != settings.longestStreak) {
          settingsChanged = true;
        }
        settings.longestStreak = days;
      };

      // Epoch-day number (days since 1970-01-01) of the last day counted into the streak -
      // lets HealthController tell "yesterday" apart from "some earlier day, watch was off
      // in between" without needing its own persistence. std::numeric_limits<uint32_t>::max()
      // means "no streak day recorded yet".
      uint32_t GetStreakLastDayNumber() const {
        return settings.streakLastDayNumber;
      };

      void SetStreakLastDayNumber(uint32_t dayNumber) {
        if (dayNumber != settings.streakLastDayNumber) {
          settingsChanged = true;
        }
        settings.streakLastDayNumber = dayNumber;
      };

      // Apple Watch-style "Pause Rings": while > 0, HealthController skips ring-closed
      // detection/celebration and streak breaking for each day, decrementing by 1 per day.
      void SetPausedDaysRemaining(uint32_t days) {
        if (days > 90) {
          days = 90;
        }
        if (days != settings.pausedDaysRemaining) {
          settingsChanged = true;
        }
        settings.pausedDaysRemaining = days;
      };

      uint32_t GetPausedDaysRemaining() const {
        return settings.pausedDaysRemaining;
      };

      // WatchFaceDigital-only weather particle accent (rain/snow drift based on the current
      // condition) - opt-in since a constantly-animating effect costs more battery/CPU than
      // a static face.
      void SetWeatherEffectsEnabled(bool enabled) {
        if (enabled != settings.weatherEffectsEnabled) {
          settingsChanged = true;
        }
        settings.weatherEffectsEnabled = enabled;
      };

      bool GetWeatherEffectsEnabled() const {
        return settings.weatherEffectsEnabled;
      };

      static constexpr size_t activityHistoryDays = 6;

      struct DailyActivity {
        uint32_t calories = 0;
        uint32_t standHours = 0;
        uint32_t exerciseMinutes = 0;
      };

      // Index 0 = oldest of the last activityHistoryDays completed days, index
      // activityHistoryDays-1 = yesterday. Today's own (in-progress) totals are read live
      // from HealthController instead of being stored here.
      const DailyActivity& GetActivityHistoryDay(size_t index) const {
        static const DailyActivity empty {};
        if (index >= activityHistoryDays) {
          return empty;
        }
        return settings.activityHistory[index];
      };

      void PushActivityHistoryDay(const DailyActivity& day) {
        for (size_t i = 0; i + 1 < activityHistoryDays; i++) {
          settings.activityHistory[i] = settings.activityHistory[i + 1];
        }
        settings.activityHistory[activityHistoryDays - 1] = day;
        settingsChanged = true;
      };

      void SetOpenHaystackEnabled(bool enabled) {
        if (enabled != settings.openHaystackEnabled) {
          settingsChanged = true;
        }
        settings.openHaystackEnabled = enabled;
      };

      bool GetOpenHaystackEnabled() const {
        return settings.openHaystackEnabled;
      };

      void SetBedtimeStartHour(uint8_t hour) {
        if (hour > 23) {
          hour = 23;
        }
        if (hour != settings.bedtimeStartHour) {
          settingsChanged = true;
        }
        settings.bedtimeStartHour = hour;
      };

      uint8_t GetBedtimeStartHour() const {
        return settings.bedtimeStartHour;
      };

      void SetBedtimeEndHour(uint8_t hour) {
        if (hour > 23) {
          hour = 23;
        }
        if (hour != settings.bedtimeEndHour) {
          settingsChanged = true;
        }
        settings.bedtimeEndHour = hour;
      };

      uint8_t GetBedtimeEndHour() const {
        return settings.bedtimeEndHour;
      };

      void SetNightModeEnabled(bool enabled) {
        if (enabled != settings.nightModeEnabled) {
          settingsChanged = true;
        }
        settings.nightModeEnabled = enabled;
      };

      bool GetNightModeEnabled() const {
        return settings.nightModeEnabled;
      };

      void SetGlassUIEnabled(bool enabled) {
        if (enabled != settings.glassUIEnabled) {
          settingsChanged = true;
        }
        settings.glassUIEnabled = enabled;
      };

      bool GetGlassUIEnabled() const {
        return settings.glassUIEnabled;
      };

      // Casual screen-lock deterrence only, same posture as the OpenHaystack private key
      // and custom vibration pattern already stored in plaintext elsewhere in this struct -
      // not a defense against someone pulling the external flash chip and reading it directly.
      void SetPasscodeEnabled(bool enabled) {
        if (enabled != settings.passcodeEnabled) {
          settingsChanged = true;
        }
        settings.passcodeEnabled = enabled;
      };

      bool GetPasscodeEnabled() const {
        return settings.passcodeEnabled;
      };

      void SetPasscode(uint16_t code) {
        if (code > 9999) {
          code = 9999;
        }
        if (code != settings.passcode) {
          settingsChanged = true;
        }
        settings.passcode = code;
      };

      uint16_t GetPasscode() const {
        return settings.passcode;
      };

      void SetTiltParallaxStrength(uint8_t strength) {
        if (strength > 140) {
          strength = 140;
        }
        if (strength != settings.tiltParallaxStrength) {
          settingsChanged = true;
        }
        settings.tiltParallaxStrength = strength;
      };

      uint8_t GetTiltParallaxStrength() const {
        return settings.tiltParallaxStrength;
      };

      std::optional<uint16_t> GetHeartRateBackgroundMeasurementInterval() const {
        if (settings.heartRateBackgroundPeriod == std::numeric_limits<uint16_t>::max()) {
          return std::nullopt;
        }
        return settings.heartRateBackgroundPeriod;
      }

      void SetHeartRateBackgroundMeasurementInterval(std::optional<uint16_t> newIntervalInSeconds) {
        newIntervalInSeconds = newIntervalInSeconds.value_or(std::numeric_limits<uint16_t>::max());
        if (newIntervalInSeconds != settings.heartRateBackgroundPeriod) {
          settingsChanged = true;
        }
        settings.heartRateBackgroundPeriod = newIntervalInSeconds.value();
      }

    private:
      Pinetime::Controllers::FS& fs;

      static constexpr uint32_t settingsVersion = 0x0019;

      struct SettingsData {
        uint32_t version = settingsVersion;
        uint32_t stepsGoal = 10000;
        uint32_t screenTimeOut = 15000;

        bool alwaysOnDisplay = false;

        ClockType clockType = ClockType::H24;
        WeatherFormat weatherFormat = WeatherFormat::Metric;
        Notification notificationStatus = Notification::On;

        Pinetime::Applications::WatchFace watchFace = Pinetime::Applications::WatchFace::Digital;
        ChimesOption chimesOption = ChimesOption::None;

        PineTimeStyle PTS;

        PrideFlag prideFlag = PrideFlag::Gay;

        WatchFaceInfineat watchFaceInfineat;
        WatchFaceModular watchFaceModular;
        QuickActionsLayout quickActionsLayout;

        WatchFaceSimpleFont watchFaceSimpleFont = WatchFaceSimpleFont::ExtraBoldCompressed;
        Colors watchFaceSimpleColor = Colors::White;
        AppMenuStyle appMenuStyle = AppMenuStyle::Grid;
        bool nightstandModeEnabled = false;

        Colors watchFaceAccentColor = Colors::White;
        uint8_t buttonCornerRadius = 20;
        uint32_t exerciseGoalMinutes = 30;
        uint32_t standGoalHours = 12;
        bool openHaystackEnabled = false;

        uint8_t bedtimeStartHour = 22;
        uint8_t bedtimeEndHour = 8;
        bool nightModeEnabled = false;
        bool glassUIEnabled = false;
        uint8_t tiltParallaxStrength = 5;

        bool passcodeEnabled = false;
        uint16_t passcode = 0;

        uint32_t caloriesGoal = 500;
        uint32_t currentStreak = 0;
        uint32_t longestStreak = 0;
        uint32_t streakLastDayNumber = std::numeric_limits<uint32_t>::max();
        uint32_t pausedDaysRemaining = 0;
        std::array<DailyActivity, activityHistoryDays> activityHistory {};
        bool weatherEffectsEnabled = false;

        Controllers::VibrationPattern selectedVibrationPattern = Controllers::VibrationPattern::Single;
        Controllers::PatternData customVibrationPattern;

        std::bitset<5> wakeUpMode {0};
        uint16_t shakeWakeThreshold = 150;

        Controllers::BrightnessController::Levels brightLevel = Controllers::BrightnessController::Levels::Medium;

        bool dfuAndFsEnabledOnBoot = false;
        uint16_t heartRateBackgroundPeriod = std::numeric_limits<uint16_t>::max(); // Disabled by default
      };

      SettingsData settings;
      bool settingsChanged = false;

      uint8_t appMenu = 0;
      uint8_t settingsMenu = 0;
      uint8_t watchFacesMenu = 0;
      /* ble state is intentionally not saved with the other watch settings and initialized
       * to off (false) on every boot because we always want ble to be enabled on startup
       */
      bool bleRadioEnabled = true;
      bool dfuAndFsEnabledTillReboot = false;

      void LoadSettingsFromFile();
      void SaveSettingsToFile();
    };
  }
}
