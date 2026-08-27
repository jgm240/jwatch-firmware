#include "displayapp/screens/settings/SettingTheme.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::Theme theme;
    const char* name;
  };

  constexpr std::array<Option, 2> options = {{
    {Pinetime::Controllers::Settings::Theme::Dark, "Dark"},
    {Pinetime::Controllers::Settings::Theme::Light, "Light"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateOptionArray() {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= options.size()) {
        optionArray[i].name = "";
        optionArray[i].enabled = false;
      } else {
        optionArray[i].name = options[i].name;
        optionArray[i].enabled = true;
      }
    }
    return optionArray;
  }

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::Theme currentTheme) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].theme == currentTheme) {
        return i;
      }
    }
    return 0;
  }
}

SettingTheme::SettingTheme(DisplayApp* app, Pinetime::Controllers::Settings& settingsController, Pinetime::Components::LittleVgl& lvgl)
  : checkboxList(
      0,
      1,
      "Theme",
      Symbols::moon,
      GetDefaultOption(settingsController.GetTheme()),
      [app, &settings = settingsController, &lvgl](uint32_t index) {
        auto theme = options[index].theme;
        settings.SetTheme(theme);
        settings.SaveSettings();
        lvgl.SetTheme(theme == Pinetime::Controllers::Settings::Theme::Light);
        app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
      },
      CreateOptionArray()) {
}

SettingTheme::~SettingTheme() {
  lv_obj_clean(lv_scr_act());
}
