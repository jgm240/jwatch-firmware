#include "displayapp/screens/settings/SettingNightstand.h"
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

  void previewEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingNightstand*>(obj->user_data);
    screen->OnPreviewButtonEvent(obj, event);
  }
}

SettingNightstand::SettingNightstand(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    checkboxList(
      0,
      1,
      "Nightstand",
      Symbols::clock,
      GetDefaultOption(settingsController.GetNightstandModeEnabled()),
      [app, &settings = settingsController](uint32_t index) {
        settings.SetNightstandModeEnabled(options[index].enabled);
        settings.SaveSettings();
        app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
      },
      CreateOptionArray()) {

  previewButton = Styles::Button(lv_scr_act());
  previewButton->user_data = this;
  lv_obj_set_event_cb(previewButton, previewEventHandler);
  lv_obj_set_size(previewButton, 120, 50);
  lv_obj_align(previewButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);

  lv_obj_t* label = lv_label_create(previewButton, nullptr);
  lv_label_set_text_static(label, "Preview");
}

SettingNightstand::~SettingNightstand() {
  lv_obj_clean(lv_scr_act());
}

void SettingNightstand::OnPreviewButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (object == previewButton && event == LV_EVENT_CLICKED) {
    app->StartApp(Apps::Nightstand, DisplayApp::FullRefreshDirections::Up);
  }
}
