#include "displayapp/screens/settings/SettingOpenHaystack.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    bool enabled;
    const char* name;
  };

  constexpr std::array<Option, 2> options = {{
    {false, "Off"},
    {true, "On"},
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

  uint32_t GetDefaultOption(bool enabled) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].enabled == enabled) {
        return i;
      }
    }
    return 0;
  }
}

SettingOpenHaystack::SettingOpenHaystack(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Find My",
      Symbols::bluetooth,
      GetDefaultOption(settingsController.GetOpenHaystackEnabled()),
      [app, &settings = settingsController](uint32_t index) {
        settings.SetOpenHaystackEnabled(options[index].enabled);
        settings.SaveSettings();
        app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
      },
      CreateOptionArray()) {

  caveat = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(caveat, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(caveat, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(caveat, LV_HOR_RES - 20);
  lv_label_set_align(caveat, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(caveat, "Broadcasts a passive location beacon via Apple's Find My network after 5min disconnected");
  lv_obj_align(caveat, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
}

SettingOpenHaystack::~SettingOpenHaystack() {
  lv_obj_clean(lv_scr_act());
}
