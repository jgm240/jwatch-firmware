#include "displayapp/screens/settings/SettingVibration.h"
#include <lvgl/lvgl.h>
#include <array>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/motor/VibrationPatterns.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::VibrationPattern pattern;
    const char* name;
  };

  constexpr std::array<Option, SettingVibration::optionsCount> options = {{
    {Pinetime::Controllers::VibrationPattern::Single, "Single"},
    {Pinetime::Controllers::VibrationPattern::Double, "Double"},
    {Pinetime::Controllers::VibrationPattern::Triple, "Triple"},
    {Pinetime::Controllers::VibrationPattern::Long, "Long"},
    {Pinetime::Controllers::VibrationPattern::Ramp, "Ramp"},
    {Pinetime::Controllers::VibrationPattern::Custom, "Custom..."},
  }};

  uint32_t IndexOf(Pinetime::Controllers::VibrationPattern pattern) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].pattern == pattern) {
        return i;
      }
    }
    return 0;
  }

  static constexpr const char* title = "Vibration";
  static constexpr const char* symbol = Symbols::bell;
}

auto SettingVibration::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingVibration::SettingVibration(DisplayApp* app,
                                    Pinetime::Controllers::Settings& settingsController,
                                    Pinetime::Controllers::MotorController& motorController)
  : app {app}, settingsController {settingsController}, motorController {motorController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingVibration::~SettingVibration() {
  lv_obj_clean(lv_scr_act());
}

bool SettingVibration::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingVibration::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < optionsPerScreen; i++) {
    if (i + (screenNum * optionsPerScreen) >= static_cast<int>(options.size())) {
      optionsOnThisScreen[i] = {"", false};
    } else {
      auto& item = options[i + (screenNum * optionsPerScreen)];
      optionsOnThisScreen[i] = Screens::CheckboxList::Item {item.name, true};
    }
  }

  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    title,
    symbol,
    IndexOf(settingsController.GetVibrationPattern()),
    [this, &settings = settingsController, &motor = motorController](uint32_t index) {
      auto pattern = options[index].pattern;
      settings.SetVibrationPattern(pattern);
      settings.SaveSettings();
      if (pattern == Pinetime::Controllers::VibrationPattern::Custom) {
        app->StartApp(Apps::SettingVibrationRecord, DisplayApp::FullRefreshDirections::Up);
      } else {
        motor.RunPattern(Pinetime::Controllers::VibrationPatterns::Get(pattern, settings.GetCustomVibrationPattern()));
      }
    },
    optionsOnThisScreen);
}
