#include "displayapp/screens/settings/SettingPasscode.h"
#include <lvgl/lvgl.h>
#include <functional>
#include <array>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr const char* const keypadMap[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", "", "0", "<", ""};

  // Enter a new 4-digit code; saves as soon as the 4th digit is entered, then resets so
  // you can immediately re-enter to double check or pick a different code.
  class SetCodeScreen : public Screen {
  public:
    SetCodeScreen(uint8_t screenID, uint8_t numScreens, Pinetime::Controllers::Settings& settingsController)
      : settingsController {settingsController}, pageIndicator(screenID, numScreens) {

      pageIndicator.Create();

      lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_text_static(title, "Set code");
      lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 15);

      for (int i = 0; i < 4; i++) {
        dots[i] = lv_obj_create(lv_scr_act(), nullptr);
        lv_obj_set_size(dots[i], 14, 14);
        lv_obj_set_style_local_radius(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
        lv_obj_set_style_local_border_width(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
        lv_obj_set_style_local_border_color(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
        lv_obj_set_style_local_bg_opa(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_click(dots[i], false);
        lv_obj_align(dots[i], lv_scr_act(), LV_ALIGN_IN_TOP_MID, (i - 2) * 24 + 12, 50);
      }

      savedLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(savedLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
      lv_label_set_text_static(savedLabel, "");
      lv_obj_align(savedLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 70);

      keypad = lv_btnmatrix_create(lv_scr_act(), nullptr);
      keypad->user_data = this;
      lv_obj_set_event_cb(keypad, EventHandler);
      lv_btnmatrix_set_map(keypad, const_cast<const char**>(keypadMap));
      lv_obj_set_size(keypad, 200, 150);
      lv_obj_align(keypad, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -4);

      UpdateDots();
    }

    ~SetCodeScreen() override {
      lv_obj_clean(lv_scr_act());
    }

    void OnKeypadEvent(lv_event_t event) {
      if (event != LV_EVENT_PRESSED) {
        return;
      }
      const char* buttonText = lv_btnmatrix_get_active_btn_text(keypad);
      if (buttonText == nullptr || buttonText[0] == '\0') {
        return;
      }

      lv_label_set_text_static(savedLabel, "");

      if (buttonText[0] == '<') {
        if (enteredCount > 0) {
          enteredCount--;
          UpdateDots();
        }
        return;
      }

      if (buttonText[0] < '0' || buttonText[0] > '9' || enteredCount >= 4) {
        return;
      }

      enteredDigits[enteredCount] = static_cast<uint8_t>(buttonText[0] - '0');
      enteredCount++;
      UpdateDots();

      if (enteredCount == 4) {
        uint16_t code =
          static_cast<uint16_t>(enteredDigits[0] * 1000 + enteredDigits[1] * 100 + enteredDigits[2] * 10 + enteredDigits[3]);
        settingsController.SetPasscode(code);
        settingsController.SaveSettings();
        lv_label_set_text_static(savedLabel, "Saved");
        enteredCount = 0;
        UpdateDots();
      }
    }

  private:
    static void EventHandler(lv_obj_t* obj, lv_event_t event) {
      static_cast<SetCodeScreen*>(obj->user_data)->OnKeypadEvent(event);
    }

    void UpdateDots() {
      for (int i = 0; i < 4; i++) {
        bool filled = i < enteredCount;
        lv_obj_set_style_local_bg_opa(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, filled ? LV_OPA_COVER : LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_color(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
      }
    }

    Pinetime::Controllers::Settings& settingsController;

    std::array<uint8_t, 4> enteredDigits {};
    uint8_t enteredCount = 0;

    std::array<lv_obj_t*, 4> dots {};
    lv_obj_t* savedLabel;
    lv_obj_t* keypad;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };

  struct ToggleOption {
    bool enabled;
    const char* name;
  };

  constexpr std::array<ToggleOption, 2> passcodeOptions = {{
    {false, "Off"},
    {true, "On"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreatePasscodeOptionArray() {
    std::array<CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= passcodeOptions.size()) {
        optionArray[i] = {"", false};
      } else {
        optionArray[i] = {passcodeOptions[i].name, true};
      }
    }
    return optionArray;
  }

  uint32_t IndexOfPasscodeOption(bool enabled) {
    for (size_t i = 0; i < passcodeOptions.size(); i++) {
      if (passcodeOptions[i].enabled == enabled) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingPasscode::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingPasscode::SettingPasscode(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingPasscode::~SettingPasscode() {
  lv_obj_clean(lv_scr_act());
}

bool SettingPasscode::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingPasscode::CreateScreen(unsigned int screenNum) const {
  if (screenNum == 1) {
    return std::make_unique<SetCodeScreen>(1, nScreens, settingsController);
  }

  // No app->StartApp(Apps::Settings, ...) here (unlike some other single-page toggles) -
  // this CheckboxList is page 0 of 2, with the code-entry keypad as page 1. Since
  // ScreenList destroys/reconstructs each page's Screen on every swipe, this callback
  // (called from CheckboxList's destructor) fires just as often when swiping FORWARD to
  // page 1 as it does when actually leaving SettingPasscode - a forced jump back to
  // Settings here made it impossible to ever reach the code-entry page at all, which was
  // the actual cause of "no passcode can be set".
  return std::make_unique<CheckboxList>(
    0,
    nScreens,
    "Passcode lock",
    Symbols::check,
    IndexOfPasscodeOption(settingsController.GetPasscodeEnabled()),
    [&settings = settingsController](uint32_t index) {
      settings.SetPasscodeEnabled(passcodeOptions[index].enabled);
      settings.SaveSettings();
    },
    CreatePasscodeOptionArray());
}
