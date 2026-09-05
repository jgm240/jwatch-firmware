#include "displayapp/screens/settings/QuickSettings.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/ble/BleController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<QuickSettings*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->OnButtonEvent(obj);
    }
  }

  void lv_update_task(struct _lv_task_t* task) {
    auto* user_data = static_cast<QuickSettings*>(task->user_data);
    user_data->UpdateScreen();
  }

  enum class ButtonState : lv_state_t {
    NotificationsOn = LV_STATE_CHECKED,
    NotificationsOff = LV_STATE_DEFAULT,
    Sleep = 0x40,
  };
}

QuickSettings::QuickSettings(Pinetime::Applications::DisplayApp* app,
                             const Pinetime::Controllers::Battery& batteryController,
                             Controllers::DateTime& dateTimeController,
                             Controllers::BrightnessController& brightness,
                             Controllers::MotorController& motorController,
                             Pinetime::Controllers::Settings& settingsController,
                             const Controllers::Ble& bleController,
                             const Controllers::AlarmController& alarmController,
                             Controllers::NimbleController& nimble)
  : app {app},
    dateTimeController {dateTimeController},
    brightness {brightness},
    motorController {motorController},
    settingsController {settingsController},
    nimble {nimble},
    statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();

  // This is the distance (padding) between all objects on this screen.
  static constexpr uint8_t innerDistance = 10;

  // Time
  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  static constexpr uint8_t barHeight = 20 + innerDistance;
  static constexpr uint8_t buttonHeight = (LV_VER_RES_MAX - barHeight - innerDistance) / 2;
  static constexpr uint8_t buttonWidth = (LV_HOR_RES_MAX - innerDistance) / 2; // wide buttons
  // static constexpr uint8_t buttonWidth = buttonHeight; // square buttons
  static constexpr uint8_t buttonXOffset = (LV_HOR_RES_MAX - buttonWidth * 2 - innerDistance) / 2;

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, LV_STATE_DEFAULT, buttonHeight / 4);
  lv_style_set_bg_color(&btn_style, LV_STATE_DEFAULT, Colors::bgAlt);

  static constexpr lv_align_t corners[4] = {LV_ALIGN_IN_TOP_LEFT, LV_ALIGN_IN_TOP_RIGHT, LV_ALIGN_IN_BOTTOM_LEFT, LV_ALIGN_IN_BOTTOM_RIGHT};
  static constexpr int16_t xOffsets[4] = {buttonXOffset, -buttonXOffset, buttonXOffset, -buttonXOffset};
  static constexpr int16_t yOffsets[4] = {barHeight, barHeight, 0, 0};

  for (size_t i = 0; i < actionButtons.size(); i++) {
    currentActions[i] = settingsController.GetQuickAction(i);

    actionButtons[i] = Styles::Button(lv_scr_act());
    actionButtons[i]->user_data = this;
    lv_obj_set_event_cb(actionButtons[i], ButtonEventHandler);
    lv_obj_add_style(actionButtons[i], LV_BTN_PART_MAIN, &btn_style);
    lv_obj_set_size(actionButtons[i], buttonWidth, buttonHeight);
    lv_obj_align(actionButtons[i], nullptr, corners[i], xOffsets[i], yOffsets[i]);

    actionLabels[i] = lv_label_create(actionButtons[i], nullptr);
    lv_obj_set_style_local_text_font(actionLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);

    CreateActionButton(i, currentActions[i]);
  }

  taskUpdate = lv_task_create(lv_update_task, 5000, LV_TASK_PRIO_MID, this);

  UpdateScreen();
}

void QuickSettings::CreateActionButton(size_t slot, Widgets::QuickAction action) {
  lv_obj_t* button = actionButtons[slot];
  lv_obj_t* label = actionLabels[slot];

  switch (action) {
    case Widgets::QuickAction::Brightness:
      lv_label_set_text_static(label, brightness.GetIcon());
      break;
    case Widgets::QuickAction::Flashlight:
      lv_label_set_text_static(label, Symbols::flashlight);
      break;
    case Widgets::QuickAction::NotificationToggle: {
      lv_obj_set_style_local_bg_color(button, LV_BTN_PART_MAIN, static_cast<lv_state_t>(ButtonState::NotificationsOff), LV_COLOR_RED);
      static constexpr lv_color_t violet = LV_COLOR_MAKE(0x60, 0x00, 0xff);
      lv_obj_set_style_local_bg_color(button, LV_BTN_PART_MAIN, static_cast<lv_state_t>(ButtonState::Sleep), violet);

      if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::On) {
        lv_label_set_text_static(label, Symbols::notificationsOn);
        lv_obj_set_state(button, static_cast<lv_state_t>(ButtonState::NotificationsOn));
      } else if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::Off) {
        lv_label_set_text_static(label, Symbols::notificationsOff);
      } else {
        lv_label_set_text_static(label, Symbols::sleep);
        lv_obj_set_state(button, static_cast<lv_state_t>(ButtonState::Sleep));
      }
      break;
    }
    case Widgets::QuickAction::OpenSettings:
      lv_label_set_text_static(label, Symbols::settings);
      break;
    case Widgets::QuickAction::Activity:
      lv_label_set_text_static(label, Symbols::tachometer);
      break;
    case Widgets::QuickAction::FindMyPhone:
      lv_label_set_text_static(label, Symbols::phone);
      break;
  }
}

QuickSettings::~QuickSettings() {
  lv_style_reset(&btn_style);
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void QuickSettings::UpdateScreen() {
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  statusIcons.Update();
}

void QuickSettings::OnButtonEvent(lv_obj_t* object) {
  for (size_t i = 0; i < actionButtons.size(); i++) {
    if (actionButtons[i] != object) {
      continue;
    }

    switch (currentActions[i]) {
      case Widgets::QuickAction::Brightness:
        brightness.Step();
        lv_label_set_text_static(actionLabels[i], brightness.GetIcon());
        settingsController.SetBrightness(brightness.Level());
        break;

      case Widgets::QuickAction::Flashlight:
        app->StartApp(Apps::FlashLight, DisplayApp::FullRefreshDirections::Up);
        break;

      case Widgets::QuickAction::NotificationToggle:
        if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::On) {
          settingsController.SetNotificationStatus(Controllers::Settings::Notification::Off);
          lv_label_set_text_static(actionLabels[i], Symbols::notificationsOff);
          lv_obj_set_state(object, static_cast<lv_state_t>(ButtonState::NotificationsOff));
        } else if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::Off) {
          settingsController.SetNotificationStatus(Controllers::Settings::Notification::Sleep);
          lv_label_set_text_static(actionLabels[i], Symbols::sleep);
          lv_obj_set_state(object, static_cast<lv_state_t>(ButtonState::Sleep));
        } else {
          settingsController.SetNotificationStatus(Controllers::Settings::Notification::On);
          lv_label_set_text_static(actionLabels[i], Symbols::notificationsOn);
          lv_obj_set_state(object, static_cast<lv_state_t>(ButtonState::NotificationsOn));
          motorController.RunForDuration(35);
        }
        break;

      case Widgets::QuickAction::OpenSettings:
        settingsController.SetSettingsMenu(0);
        app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
        break;

      case Widgets::QuickAction::Activity:
        app->StartApp(Apps::ActivityRings, DisplayApp::FullRefreshDirections::Up);
        break;

      case Widgets::QuickAction::FindMyPhone:
        nimble.findMyPhone().SendTrigger();
        motorController.RunForDuration(35);
        break;
    }
    return;
  }
}
