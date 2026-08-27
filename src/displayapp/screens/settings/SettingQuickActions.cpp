#include "displayapp/screens/settings/SettingQuickActions.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Applications::Widgets::QuickAction action;
    const char* name;
  };

  constexpr std::array<Option, SettingQuickActions::optionsCount> options = {{
    {Pinetime::Applications::Widgets::QuickAction::Brightness, "Brightness"},
    {Pinetime::Applications::Widgets::QuickAction::Flashlight, "Flashlight"},
    {Pinetime::Applications::Widgets::QuickAction::NotificationToggle, "Notifications"},
    {Pinetime::Applications::Widgets::QuickAction::OpenSettings, "Settings"},
  }};

  constexpr std::array<const char*, SettingQuickActions::nSlots> slotTitles = {"Top left", "Top right", "Bottom left", "Bottom right"};

  uint32_t IndexOf(Pinetime::Applications::Widgets::QuickAction action) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].action == action) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingQuickActions::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingQuickActions::SettingQuickActions(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingQuickActions::~SettingQuickActions() {
  lv_obj_clean(lv_scr_act());
}

bool SettingQuickActions::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingQuickActions::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, optionsCount> optionsOnThisScreen;
  for (int i = 0; i < optionsCount; i++) {
    optionsOnThisScreen[i] = Screens::CheckboxList::Item {options[i].name, true};
  }

  return std::make_unique<Screens::CheckboxList>(
    0,
    1,
    slotTitles[screenNum],
    Symbols::list,
    IndexOf(settingsController.GetQuickAction(screenNum)),
    [this, screenNum, &settings = settingsController](uint32_t index) {
      settings.SetQuickAction(screenNum, options[index].action);
      settings.SaveSettings();
    },
    optionsOnThisScreen);
}
