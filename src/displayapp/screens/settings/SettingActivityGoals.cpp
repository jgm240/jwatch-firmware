#include "displayapp/screens/settings/SettingActivityGoals.h"
#include "displayapp/screens/Styles.h"
#include <lvgl/lvgl.h>
#include <functional>
#include <utility>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Generic +/- stepper page, structurally identical to SettingSteps.cpp's own stepper UI,
  // parameterized so it can be reused for both the exercise-minutes and stand-hours goals.
  class GoalStepper : public Screen {
  public:
    GoalStepper(uint8_t screenID,
                uint8_t numScreens,
                const char* title,
                const char* icon,
                uint32_t initialValue,
                uint32_t stepSmall,
                uint32_t stepLarge,
                uint32_t minValue,
                uint32_t maxValue,
                std::function<void(uint32_t)> onValueChanged)
      : onValueChanged {std::move(onValueChanged)},
        value {initialValue},
        stepSmall {stepSmall},
        stepLarge {stepLarge},
        minValue {minValue},
        maxValue {maxValue},
        pageIndicator(screenID, numScreens) {

      if (numScreens > 1) {
        pageIndicator.Create();
      }

      lv_obj_t* titleLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_text_static(titleLabel, title);
      lv_label_set_align(titleLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(titleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

      lv_obj_t* iconLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(iconLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
      lv_label_set_text_static(iconLabel, icon);
      lv_label_set_align(iconLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(iconLabel, titleLabel, LV_ALIGN_OUT_LEFT_MID, -10, 0);

      valueLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_font(valueLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text_fmt(valueLabel, "%lu", value);
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

    ~GoalStepper() override {
      lv_obj_clean(lv_scr_act());
    }

    void UpdateSelected(lv_obj_t* object, lv_event_t event) {
      int32_t valueChange = 0;
      if (event == LV_EVENT_SHORT_CLICKED) {
        valueChange = static_cast<int32_t>(stepSmall);
      } else if (event == LV_EVENT_LONG_PRESSED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
        valueChange = static_cast<int32_t>(stepLarge);
      } else {
        return;
      }

      if (object == btnMinus) {
        valueChange = -valueChange;
      } else if (object != btnPlus) {
        return;
      }

      int64_t newValue = static_cast<int64_t>(value) + valueChange;
      if (newValue >= static_cast<int64_t>(minValue) && newValue <= static_cast<int64_t>(maxValue)) {
        value = static_cast<uint32_t>(newValue);
        lv_label_set_text_fmt(valueLabel, "%lu", value);
        lv_obj_realign(valueLabel);
        onValueChanged(value);
      }
    }

  private:
    static void EventHandler(lv_obj_t* obj, lv_event_t event) {
      static_cast<GoalStepper*>(obj->user_data)->UpdateSelected(obj, event);
    }

    std::function<void(uint32_t)> onValueChanged;
    uint32_t value;
    uint32_t stepSmall;
    uint32_t stepLarge;
    uint32_t minValue;
    uint32_t maxValue;

    lv_obj_t* valueLabel;
    lv_obj_t* btnPlus;
    lv_obj_t* btnMinus;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };
}

auto SettingActivityGoals::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingActivityGoals::SettingActivityGoals(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    settingsController {settingsController},
    screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingActivityGoals::~SettingActivityGoals() {
  lv_obj_clean(lv_scr_act());
}

bool SettingActivityGoals::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingActivityGoals::CreateScreen(unsigned int screenNum) const {
  if (screenNum == 0) {
    return std::make_unique<GoalStepper>(0,
                                         nScreens,
                                         "Calories goal",
                                         Symbols::tachometer,
                                         settingsController.GetCaloriesGoal(),
                                         50,
                                         100,
                                         50,
                                         5000,
                                         [&settings = settingsController](uint32_t v) {
                                           settings.SetCaloriesGoal(v);
                                           settings.SaveSettings();
                                         });
  }
  if (screenNum == 1) {
    return std::make_unique<GoalStepper>(1,
                                         nScreens,
                                         "Exercise goal (min)",
                                         Symbols::heartBeat,
                                         settingsController.GetExerciseGoalMinutes(),
                                         5,
                                         10,
                                         5,
                                         240,
                                         [&settings = settingsController](uint32_t v) {
                                           settings.SetExerciseGoalMinutes(v);
                                           settings.SaveSettings();
                                         });
  }
  return std::make_unique<GoalStepper>(2,
                                       nScreens,
                                       "Stand goal (hr)",
                                       Symbols::tachometer,
                                       settingsController.GetStandGoalHours(),
                                       1,
                                       2,
                                       1,
                                       24,
                                       [&settings = settingsController](uint32_t v) {
                                         settings.SetStandGoalHours(v);
                                         settings.SaveSettings();
                                       });
}
