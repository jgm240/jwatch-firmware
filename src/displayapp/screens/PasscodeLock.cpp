#include "displayapp/screens/PasscodeLock.h"
#include <hal/nrf_rtc.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Styles.h"
#include "components/motor/MotorController.h"
#include "components/settings/Settings.h"
#include "components/fs/FS.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr const char* const keypadMap[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", "", "0", "<", ""};

  void keypadEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<PasscodeLock*>(obj->user_data);
    screen->OnKeypadEvent(obj, event);
  }

  void linkEventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    auto* screen = static_cast<PasscodeLock*>(obj->user_data);
    screen->OnLinkEvent(obj);
  }
}

PasscodeLock::PasscodeLock(DisplayApp* app,
                           Controllers::MotorController& motorController,
                           Controllers::Settings& settingsController,
                           Controllers::FS& filesystem)
  : app {app}, motorController {motorController}, settingsController {settingsController}, filesystem {filesystem} {

  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Enter Passcode");
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

  keypad = lv_btnmatrix_create(lv_scr_act(), nullptr);
  keypad->user_data = this;
  lv_obj_set_event_cb(keypad, keypadEventHandler);
  lv_btnmatrix_set_map(keypad, const_cast<const char**>(keypadMap));
  lv_obj_set_size(keypad, 200, 150);
  lv_obj_align(keypad, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -26);

  forgotLink = lv_label_create(lv_scr_act(), nullptr);
  forgotLink->user_data = this;
  lv_obj_set_click(forgotLink, true);
  lv_obj_set_event_cb(forgotLink, linkEventHandler);
  lv_obj_set_style_local_text_color(forgotLink, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(forgotLink, "Forgot code? Erase watch");
  lv_obj_align(forgotLink, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -4);

  eraseLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(eraseLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(eraseLabel, 200);
  lv_label_set_align(eraseLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(eraseLabel,
                           "Erase all data?\nSettings, pairings and files "
                           "are wiped. Firmware is kept.\nThis cannot be undone.");
  lv_obj_align(eraseLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);

  eraseCancelBtn = Styles::Button(lv_scr_act());
  eraseCancelBtn->user_data = this;
  lv_obj_set_event_cb(eraseCancelBtn, linkEventHandler);
  lv_obj_set_size(eraseCancelBtn, 100, 50);
  lv_obj_align(eraseCancelBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -10);
  lv_obj_t* cancelLabel = lv_label_create(eraseCancelBtn, nullptr);
  lv_label_set_text_static(cancelLabel, "Cancel");

  eraseConfirmBtn = Styles::Button(lv_scr_act());
  eraseConfirmBtn->user_data = this;
  lv_obj_set_event_cb(eraseConfirmBtn, linkEventHandler);
  lv_obj_set_size(eraseConfirmBtn, 100, 50);
  lv_obj_align(eraseConfirmBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -10);
  lv_obj_set_style_local_bg_color(eraseConfirmBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_t* confirmLabel = lv_label_create(eraseConfirmBtn, nullptr);
  lv_label_set_text_static(confirmLabel, "Erase");

  ShowEntry();
}

PasscodeLock::~PasscodeLock() {
  lv_obj_clean(lv_scr_act());
}

void PasscodeLock::ShowEntry() {
  viewState = ViewState::Entry;
  lv_obj_set_hidden(title, false);
  for (auto* dot : dots) {
    lv_obj_set_hidden(dot, false);
  }
  lv_obj_set_hidden(keypad, false);
  lv_obj_set_hidden(forgotLink, false);

  lv_obj_set_hidden(eraseLabel, true);
  lv_obj_set_hidden(eraseCancelBtn, true);
  lv_obj_set_hidden(eraseConfirmBtn, true);

  ResetEntry();
}

void PasscodeLock::ShowConfirmErase() {
  viewState = ViewState::ConfirmErase;
  lv_obj_set_hidden(title, true);
  for (auto* dot : dots) {
    lv_obj_set_hidden(dot, true);
  }
  lv_obj_set_hidden(keypad, true);
  lv_obj_set_hidden(forgotLink, true);

  lv_obj_set_hidden(eraseLabel, false);
  lv_obj_set_hidden(eraseCancelBtn, false);
  lv_obj_set_hidden(eraseConfirmBtn, false);
}

void PasscodeLock::UpdateDots() {
  for (int i = 0; i < 4; i++) {
    bool filled = i < enteredCount;
    lv_obj_set_style_local_bg_opa(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, filled ? LV_OPA_COVER : LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_color(dots[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  }
}

void PasscodeLock::ResetEntry() {
  enteredCount = 0;
  UpdateDots();
}

void PasscodeLock::OnKeypadEvent(lv_obj_t* /*target*/, lv_event_t event) {
  if (viewState != ViewState::Entry || event != LV_EVENT_PRESSED) {
    return;
  }

  const char* buttonText = lv_btnmatrix_get_active_btn_text(keypad);
  if (buttonText == nullptr || buttonText[0] == '\0') {
    return;
  }

  if (buttonText[0] == '<') {
    if (enteredCount > 0) {
      enteredCount--;
      UpdateDots();
    }
    return;
  }

  if (buttonText[0] < '0' || buttonText[0] > '9') {
    return;
  }

  if (enteredCount >= 4) {
    return;
  }

  enteredDigits[enteredCount] = static_cast<uint8_t>(buttonText[0] - '0');
  enteredCount++;
  UpdateDots();

  if (enteredCount == 4) {
    CheckPasscode();
  }
}

void PasscodeLock::CheckPasscode() {
  uint16_t entered = static_cast<uint16_t>(enteredDigits[0] * 1000 + enteredDigits[1] * 100 + enteredDigits[2] * 10 + enteredDigits[3]);

  if (entered == settingsController.GetPasscode()) {
    app->Unlock();
    app->StartApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);
    running = false;
  } else {
    motorController.RunForDuration(200);
    ResetEntry();
  }
}

bool PasscodeLock::OnTouchEvent(TouchEvents /*event*/) {
  return true;
}

bool PasscodeLock::OnButtonPushed() {
  return true;
}

void PasscodeLock::OnLinkEvent(lv_obj_t* target) {
  if (target == forgotLink) {
    ShowConfirmErase();
  } else if (target == eraseCancelBtn) {
    ShowEntry();
  } else if (target == eraseConfirmBtn) {
    filesystem.Format();
    NVIC_SystemReset();
  }
}
