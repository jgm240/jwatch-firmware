#include "displayapp/screens/settings/SettingAppMenuStyle.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::AppMenuStyle style;
    const char* name;
  };

  constexpr std::array<Option, 3> options = {{
    {Pinetime::Controllers::Settings::AppMenuStyle::Grid, "Grid"},
    {Pinetime::Controllers::Settings::AppMenuStyle::Circle, "Circles"},
    {Pinetime::Controllers::Settings::AppMenuStyle::List, "List"},
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

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::AppMenuStyle style) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].style == style) {
        return i;
      }
    }
    return 0;
  }
}

SettingAppMenuStyle::SettingAppMenuStyle(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "App menu",
      Symbols::list,
      GetDefaultOption(settingsController.GetAppMenuStyle()),
      [app, &settings = settingsController](uint32_t index) {
        settings.SetAppMenuStyle(options[index].style);
        settings.SaveSettings();
        app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
      },
      CreateOptionArray()) {
}

SettingAppMenuStyle::~SettingAppMenuStyle() {
  lv_obj_clean(lv_scr_act());
}
