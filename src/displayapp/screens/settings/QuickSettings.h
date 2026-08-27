#pragma once

#include <array>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/widgets/QuickAction.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class QuickSettings : public Screen {
      public:
        QuickSettings(DisplayApp* app,
                      const Pinetime::Controllers::Battery& batteryController,
                      Controllers::DateTime& dateTimeController,
                      Controllers::BrightnessController& brightness,
                      Controllers::MotorController& motorController,
                      Pinetime::Controllers::Settings& settingsController,
                      const Controllers::Ble& bleController,
                      const Controllers::AlarmController& alarmController);

        ~QuickSettings() override;

        void OnButtonEvent(lv_obj_t* object);

        void UpdateScreen();

      private:
        void CreateActionButton(size_t slot, Widgets::QuickAction action);

        DisplayApp* app;
        Controllers::DateTime& dateTimeController;
        Controllers::BrightnessController& brightness;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;

        lv_task_t* taskUpdate;
        lv_obj_t* label_time;

        lv_style_t btn_style;

        std::array<lv_obj_t*, Controllers::Settings::quickActionSlots> actionButtons;
        std::array<lv_obj_t*, Controllers::Settings::quickActionSlots> actionLabels;
        std::array<Widgets::QuickAction, Controllers::Settings::quickActionSlots> currentActions;

        Widgets::StatusIcons statusIcons;
      };
    }
  }
}
