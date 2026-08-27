#pragma once

#include <array>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/List.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class Settings : public Screen {
      public:
        Settings(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~Settings() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        DisplayApp* app;
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;

        static constexpr int entriesPerScreen = 4;

        // Increment this when more space is needed
        static constexpr int nScreens = 8;

        static constexpr std::array<List::Applications, entriesPerScreen * nScreens> entries {{
          {Symbols::sun, "Display", Apps::SettingDisplay},
          {Symbols::eye, "Wake Up", Apps::SettingWakeUp},
          {Symbols::clock, "Time format", Apps::SettingTimeFormat},
          {Symbols::home, "Watch face", Apps::SettingWatchFace},

          {Symbols::shoe, "Steps", Apps::SettingSteps},
          {Symbols::heartBeat, "Heartrate", Apps::SettingHeartRate},
          {Symbols::clock, "Date & Time", Apps::SettingSetDateTime},
          {Symbols::cloudSunRain, "Weather", Apps::SettingWeatherFormat},

          {Symbols::batteryHalf, "Battery", Apps::BatteryInfo},
          {Symbols::clock, "Chimes", Apps::SettingChimes},
          {Symbols::tachometer, "Shake Calib.", Apps::SettingShakeThreshold},
          {Symbols::check, "Firmware", Apps::FirmwareValidation},

          {Symbols::shieldAlt, "Over-the-air", Apps::SettingOTA},
          {Symbols::bluetooth, "Bluetooth", Apps::SettingBluetooth},
          {Symbols::list, "About", Apps::SysInfo},
          {Symbols::bell, "Vibration", Apps::SettingVibration},

          {Symbols::none, "", Apps::None},
          {Symbols::paintbrush, "Modular face", Apps::SettingModularWatchFace},
          {Symbols::list, "Quick Actions", Apps::SettingQuickActions},
          {Symbols::paintbrushLg, "Handwriting", Apps::SettingHandwriting},

          {Symbols::paintbrush, "Watchface style", Apps::SettingWatchFaceSimpleStyle},
          {Symbols::list, "App menu", Apps::SettingAppMenuStyle},
          {Symbols::clock, "Nightstand", Apps::SettingNightstand},
          {Symbols::paintbrushLg, "Watchface color", Apps::SettingWatchFaceColor},

          {Symbols::list, "Button radius", Apps::SettingButtonRadius},
          {Symbols::tachometer, "Activity goals", Apps::SettingActivityGoals},
          {Symbols::bluetooth, "Find My", Apps::SettingOpenHaystack},
          {Symbols::moon, "Bedtime", Apps::SettingBedtime},

          {Symbols::tachometer, "Tilt parallax", Apps::SettingTiltParallax},
          {Symbols::check, "Passcode lock", Apps::SettingPasscode},
          {Symbols::sleep, "Pause rings", Apps::SettingPauseRings},
          {Symbols::list, "SPI-NOR", Apps::SettingSpiNor},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
