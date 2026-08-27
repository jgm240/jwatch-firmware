#include "displayapp/screens/settings/SettingWatchFaceColor.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct ColorOption {
    Pinetime::Controllers::Settings::Colors color;
    const char* name;
  };

  constexpr std::array<ColorOption, SettingWatchFaceColor::colorOptionsCount> colorOptions = {{
    {Pinetime::Controllers::Settings::Colors::White, "White"},
    {Pinetime::Controllers::Settings::Colors::Silver, "Silver"},
    {Pinetime::Controllers::Settings::Colors::Gray, "Gray"},
    {Pinetime::Controllers::Settings::Colors::Black, "Black"},
    {Pinetime::Controllers::Settings::Colors::Red, "Red"},
    {Pinetime::Controllers::Settings::Colors::Maroon, "Maroon"},
    {Pinetime::Controllers::Settings::Colors::Yellow, "Yellow"},
    {Pinetime::Controllers::Settings::Colors::Olive, "Olive"},
    {Pinetime::Controllers::Settings::Colors::Lime, "Lime"},
    {Pinetime::Controllers::Settings::Colors::Green, "Green"},
    {Pinetime::Controllers::Settings::Colors::Cyan, "Cyan"},
    {Pinetime::Controllers::Settings::Colors::Teal, "Teal"},
    {Pinetime::Controllers::Settings::Colors::Blue, "Blue"},
    {Pinetime::Controllers::Settings::Colors::Navy, "Navy"},
    {Pinetime::Controllers::Settings::Colors::Magenta, "Magenta"},
    {Pinetime::Controllers::Settings::Colors::Purple, "Purple"},
    {Pinetime::Controllers::Settings::Colors::Orange, "Orange"},
    {Pinetime::Controllers::Settings::Colors::Pink, "Pink"},
  }};

  uint32_t IndexOfColor(Pinetime::Controllers::Settings::Colors color) {
    for (size_t i = 0; i < colorOptions.size(); i++) {
      if (colorOptions[i].color == color) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingWatchFaceColor::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingWatchFaceColor::SettingWatchFaceColor(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingWatchFaceColor::~SettingWatchFaceColor() {
  lv_obj_clean(lv_scr_act());
}

bool SettingWatchFaceColor::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingWatchFaceColor::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < optionsPerScreen; i++) {
    unsigned int optionIndex = i + (screenNum * optionsPerScreen);
    if (optionIndex >= colorOptions.size()) {
      optionsOnThisScreen[i] = {"", false};
    } else {
      optionsOnThisScreen[i] = Screens::CheckboxList::Item {colorOptions[optionIndex].name, true};
    }
  }
  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    "Watchface color",
    Symbols::paintbrushLg,
    IndexOfColor(settingsController.GetWatchFaceAccentColor()),
    [this, &settings = settingsController](uint32_t index) {
      settings.SetWatchFaceAccentColor(colorOptions[index].color);
      settings.SaveSettings();
    },
    optionsOnThisScreen);
}
