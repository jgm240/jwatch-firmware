#include "displayapp/screens/settings/SettingWatchFaceSimpleStyle.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct FontOption {
    Pinetime::Controllers::Settings::WatchFaceSimpleFont font;
    const char* name;
  };

  constexpr std::array<FontOption, SettingWatchFaceSimpleStyle::fontOptionsCount> fontOptions = {{
    {Pinetime::Controllers::Settings::WatchFaceSimpleFont::ExtraBoldCompressed, "Bold"},
    {Pinetime::Controllers::Settings::WatchFaceSimpleFont::Light, "Light"},
    {Pinetime::Controllers::Settings::WatchFaceSimpleFont::Mono76, "Mono"},
  }};

  struct ColorOption {
    Pinetime::Controllers::Settings::Colors color;
    const char* name;
  };

  constexpr std::array<ColorOption, SettingWatchFaceSimpleStyle::colorOptionsCount> colorOptions = {{
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

  uint32_t IndexOfFont(Pinetime::Controllers::Settings::WatchFaceSimpleFont font) {
    for (size_t i = 0; i < fontOptions.size(); i++) {
      if (fontOptions[i].font == font) {
        return i;
      }
    }
    return 0;
  }

  uint32_t IndexOfColor(Pinetime::Controllers::Settings::Colors color) {
    for (size_t i = 0; i < colorOptions.size(); i++) {
      if (colorOptions[i].color == color) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingWatchFaceSimpleStyle::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingWatchFaceSimpleStyle::SettingWatchFaceSimpleStyle(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingWatchFaceSimpleStyle::~SettingWatchFaceSimpleStyle() {
  lv_obj_clean(lv_scr_act());
}

bool SettingWatchFaceSimpleStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingWatchFaceSimpleStyle::CreateScreen(unsigned int screenNum) const {
  if (screenNum < fontPages) {
    std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
    for (int i = 0; i < optionsPerScreen; i++) {
      if (static_cast<size_t>(i) >= fontOptions.size()) {
        optionsOnThisScreen[i] = {"", false};
      } else {
        optionsOnThisScreen[i] = Screens::CheckboxList::Item {fontOptions[i].name, true};
      }
    }
    return std::make_unique<Screens::CheckboxList>(
      screenNum,
      fontPages,
      "Font",
      Symbols::paintbrushLg,
      IndexOfFont(settingsController.GetWatchFaceSimpleFont()),
      [this, &settings = settingsController](uint32_t index) {
        settings.SetWatchFaceSimpleFont(fontOptions[index].font);
        settings.SaveSettings();
      },
      optionsOnThisScreen);
  }

  unsigned int colorPage = screenNum - fontPages;
  std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < optionsPerScreen; i++) {
    unsigned int optionIndex = i + (colorPage * optionsPerScreen);
    if (optionIndex >= colorOptions.size()) {
      optionsOnThisScreen[i] = {"", false};
    } else {
      optionsOnThisScreen[i] = Screens::CheckboxList::Item {colorOptions[optionIndex].name, true};
    }
  }
  return std::make_unique<Screens::CheckboxList>(
    colorPage,
    colorPages,
    "Color",
    Symbols::paintbrushLg,
    IndexOfColor(settingsController.GetWatchFaceSimpleColor()),
    [this, &settings = settingsController](uint32_t index) {
      settings.SetWatchFaceSimpleColor(colorOptions[index].color);
      settings.SaveSettings();
    },
    optionsOnThisScreen);
}
