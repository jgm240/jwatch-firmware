#include "displayapp/screens/settings/SettingWatchName.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
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
    std::array<CheckboxList::Item, CheckboxList::MaxItems> optionArray;
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

// The name itself is set from the phone app (there's no on-watch keyboard) - this screen
// just shows the current value and toggles whether it's displayed on the watchface.
SettingWatchName::SettingWatchName(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Show on face",
      Symbols::info,
      GetDefaultOption(settingsController.GetShowWatchNameOnFace()),
      [&settings = settingsController](uint32_t index) {
        settings.SetShowWatchNameOnFace(options[index].enabled);
        settings.SaveSettings();
      },
      CreateOptionArray()) {

  lv_obj_t* nameLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(nameLabel, "Name: %s", settingsController.GetWatchName());
  lv_obj_align(nameLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
}

SettingWatchName::~SettingWatchName() {
  lv_obj_clean(lv_scr_act());
}
