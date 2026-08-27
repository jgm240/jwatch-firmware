#include "displayapp/screens/settings/SettingBedtime.h"
#include <lvgl/lvgl.h>
#include <functional>
#include <utility>
#include <array>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Same generic +/- hour stepper shape as SettingActivityGoals.cpp's GoalStepper.
  class HourStepper : public Screen {
  public:
    HourStepper(uint8_t screenID, uint8_t numScreens, const char* title, uint8_t initialHour, std::function<void(uint8_t)> onValueChanged)
      : onValueChanged {std::move(onValueChanged)}, value {initialHour}, pageIndicator(screenID, numScreens) {

      pageIndicator.Create();

      lv_obj_t* titleLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_text_static(titleLabel, title);
      lv_label_set_align(titleLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(titleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

      lv_obj_t* iconLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(iconLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
      lv_label_set_text_static(iconLabel, Symbols::clock);
      lv_label_set_align(iconLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(iconLabel, titleLabel, LV_ALIGN_OUT_LEFT_MID, -10, 0);

      valueLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_font(valueLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_fmt(valueLabel, "%02u:00", value);
      lv_label_set_align(valueLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(valueLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

      static constexpr uint8_t btnWidth = 115;
      static constexpr uint8_t btnHeight = 80;

      btnPlus = Styles::Button(lv_scr_act());
      btnPlus->user_data = this;
      lv_obj_set_size(btnPlus, btnWidth, btnHeight);
      lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
      lv_obj_set_style_local_bg_color(btnPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
      lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
      lv_obj_set_style_local_text_font(lblPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_static(lblPlus, "+");
      lv_obj_set_event_cb(btnPlus, EventHandler);

      btnMinus = Styles::Button(lv_scr_act());
      btnMinus->user_data = this;
      lv_obj_set_size(btnMinus, btnWidth, btnHeight);
      lv_obj_set_event_cb(btnMinus, EventHandler);
      lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
      lv_obj_set_style_local_bg_color(btnMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
      lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
      lv_obj_set_style_local_text_font(lblMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_static(lblMinus, "-");
    }

    ~HourStepper() override {
      lv_obj_clean(lv_scr_act());
    }

    void UpdateSelected(lv_obj_t* object, lv_event_t event) {
      if (event != LV_EVENT_SHORT_CLICKED && event != LV_EVENT_LONG_PRESSED && event != LV_EVENT_LONG_PRESSED_REPEAT) {
        return;
      }

      if (object == btnPlus) {
        value = static_cast<uint8_t>((value + 1) % 24);
      } else if (object == btnMinus) {
        value = static_cast<uint8_t>((value + 23) % 24);
      } else {
        return;
      }

      lv_label_set_text_fmt(valueLabel, "%02u:00", value);
      lv_obj_realign(valueLabel);
      onValueChanged(value);
    }

  private:
    static void EventHandler(lv_obj_t* obj, lv_event_t event) {
      static_cast<HourStepper*>(obj->user_data)->UpdateSelected(obj, event);
    }

    std::function<void(uint8_t)> onValueChanged;
    uint8_t value;

    lv_obj_t* valueLabel;
    lv_obj_t* btnPlus;
    lv_obj_t* btnMinus;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };

  struct ToggleOption {
    bool enabled;
    const char* name;
  };

  constexpr std::array<ToggleOption, 2> nightModeOptions = {{
    {false, "Off"},
    {true, "On"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateNightModeOptionArray() {
    std::array<CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= nightModeOptions.size()) {
        optionArray[i] = {"", false};
      } else {
        optionArray[i] = {nightModeOptions[i].name, true};
      }
    }
    return optionArray;
  }

  uint32_t IndexOfNightModeOption(bool enabled) {
    for (size_t i = 0; i < nightModeOptions.size(); i++) {
      if (nightModeOptions[i].enabled == enabled) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingBedtime::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingBedtime::SettingBedtime(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingBedtime::~SettingBedtime() {
  lv_obj_clean(lv_scr_act());
}

bool SettingBedtime::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingBedtime::CreateScreen(unsigned int screenNum) const {
  if (screenNum == 0) {
    return std::make_unique<HourStepper>(0, nScreens, "Bedtime start", settingsController.GetBedtimeStartHour(), [&settings = settingsController](uint8_t hour) {
      settings.SetBedtimeStartHour(hour);
      settings.SaveSettings();
    });
  }
  if (screenNum == 1) {
    return std::make_unique<HourStepper>(1, nScreens, "Bedtime end", settingsController.GetBedtimeEndHour(), [&settings = settingsController](uint8_t hour) {
      settings.SetBedtimeEndHour(hour);
      settings.SaveSettings();
    });
  }

  // screenID=0, not this page's index within SettingBedtime's own ScreenList - same fix
  // as SettingButtonRadius's Glass UI page. This is a standalone single-page 2-option
  // list; passing the outer page index corrupted the checked-item index math and could
  // read nightModeOptions out of bounds in OnValueChanged.
  return std::make_unique<CheckboxList>(
    0,
    nScreens,
    "Night mode",
    Symbols::moon,
    IndexOfNightModeOption(settingsController.GetNightModeEnabled()),
    [app = app, &settings = settingsController](uint32_t index) {
      settings.SetNightModeEnabled(nightModeOptions[index].enabled);
      settings.SaveSettings();
      app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
    },
    CreateNightModeOptionArray());
}
