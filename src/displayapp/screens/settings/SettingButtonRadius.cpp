#include "displayapp/screens/settings/SettingButtonRadius.h"
#include <lvgl/lvgl.h>
#include <array>
#include <functional>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr uint8_t minRadius = 0;
  constexpr uint8_t maxRadius = 40;

  class RadiusPage : public Screen {
  public:
    RadiusPage(uint8_t screenID, uint8_t numScreens, Pinetime::Controllers::Settings& settingsController)
      : settingsController {settingsController}, pageIndicator(screenID, numScreens) {
      pageIndicator.Create();

      lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_text_static(title, "Button radius");
      lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

      lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
      lv_label_set_text_static(icon, Symbols::list);
      lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

      valueLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_font(valueLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_fmt(valueLabel, "%u", settingsController.GetButtonCornerRadius());
      lv_label_set_align(valueLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(valueLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 55);

      previewButton = Styles::Button(lv_scr_act());
      lv_obj_set_size(previewButton, 140, 70);
      lv_obj_align(previewButton, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
      lv_obj_t* previewLabel = lv_label_create(previewButton, nullptr);
      lv_label_set_text_static(previewLabel, "Preview");

      static constexpr uint8_t btnWidth = 115;
      static constexpr uint8_t btnHeight = 80;

      btnPlus = lv_btn_create(lv_scr_act(), nullptr);
      btnPlus->user_data = this;
      lv_obj_set_size(btnPlus, btnWidth, btnHeight);
      lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
      lv_obj_set_style_local_bg_color(btnPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
      lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
      lv_obj_set_style_local_text_font(lblPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_static(lblPlus, "+");
      lv_obj_set_event_cb(btnPlus, EventHandler);

      btnMinus = lv_btn_create(lv_scr_act(), nullptr);
      btnMinus->user_data = this;
      lv_obj_set_size(btnMinus, btnWidth, btnHeight);
      lv_obj_set_event_cb(btnMinus, EventHandler);
      lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
      lv_obj_set_style_local_bg_color(btnMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
      lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
      lv_obj_set_style_local_text_font(lblMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_static(lblMinus, "-");
    }

    ~RadiusPage() override {
      lv_obj_clean(lv_scr_act());
      settingsController.SaveSettings();
    }

    void UpdateSelected(lv_obj_t* object, lv_event_t event) {
      int valueChange = 0;
      if (event == LV_EVENT_SHORT_CLICKED) {
        valueChange = 2;
      } else if (event == LV_EVENT_LONG_PRESSED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
        valueChange = 5;
      } else {
        return;
      }

      if (object == btnMinus) {
        valueChange = -valueChange;
      } else if (object != btnPlus) {
        return;
      }

      int newValue = static_cast<int>(settingsController.GetButtonCornerRadius()) + valueChange;
      if (newValue >= minRadius && newValue <= maxRadius) {
        auto radius = static_cast<uint8_t>(newValue);
        settingsController.SetButtonCornerRadius(radius);
        Styles::SetButtonRadius(radius);
        lv_label_set_text_fmt(valueLabel, "%u", radius);
        lv_obj_realign(valueLabel);
        lv_obj_set_style_local_radius(previewButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, radius);
      }
    }

  private:
    static void EventHandler(lv_obj_t* obj, lv_event_t event) {
      static_cast<RadiusPage*>(obj->user_data)->UpdateSelected(obj, event);
    }

    Pinetime::Controllers::Settings& settingsController;

    lv_obj_t* valueLabel;
    lv_obj_t* previewButton;
    lv_obj_t* btnPlus;
    lv_obj_t* btnMinus;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };

  struct GlassOption {
    bool enabled;
    const char* name;
  };

  constexpr std::array<GlassOption, 2> glassOptions = {{
    {false, "Off"},
    {true, "On"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateGlassOptionArray() {
    std::array<CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= glassOptions.size()) {
        optionArray[i] = {"", false};
      } else {
        optionArray[i] = {glassOptions[i].name, true};
      }
    }
    return optionArray;
  }

  uint32_t IndexOfGlassOption(bool enabled) {
    for (size_t i = 0; i < glassOptions.size(); i++) {
      if (glassOptions[i].enabled == enabled) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingButtonRadius::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingButtonRadius::SettingButtonRadius(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingButtonRadius::~SettingButtonRadius() {
  lv_obj_clean(lv_scr_act());
}

bool SettingButtonRadius::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingButtonRadius::CreateScreen(unsigned int screenNum) const {
  if (screenNum == 0) {
    return std::make_unique<RadiusPage>(0, nScreens, settingsController);
  }

  // screenID=0, not this page's index within SettingButtonRadius's own ScreenList: this
  // CheckboxList holds its own complete, single-page 2-option list (Off/On), so its
  // internal checked-item math (MaxItems * screenID + i) must index from 0. Passing the
  // outer page index here previously pushed that math out of range, silently corrupting
  // both the initial checked state and the index handed to OnValueChanged - the actual
  // cause of "Glass UI can't be turned off".
  return std::make_unique<CheckboxList>(
    0,
    nScreens,
    "Glass UI",
    Symbols::eye,
    IndexOfGlassOption(settingsController.GetGlassUIEnabled()),
    [&settings = settingsController](uint32_t index) {
      settings.SetGlassUIEnabled(glassOptions[index].enabled);
      settings.SaveSettings();
      Styles::SetGlassEnabled(glassOptions[index].enabled);
    },
    CreateGlassOptionArray());
}
