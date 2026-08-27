#pragma once

#include <cstdint>
#include <FreeRTOS.h>
#include <task.h>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "components/motor/VibrationPatterns.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingVibrationRecord : public Screen {
      public:
        SettingVibrationRecord(Pinetime::Controllers::Settings& settingsController, Pinetime::Controllers::MotorController& motorController);
        ~SettingVibrationRecord() override;

        void OnEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(TouchEvents event) override;

      private:
        Controllers::PatternData Finalized() const;

        Controllers::Settings& settingsController;
        Controllers::MotorController& motorController;

        Controllers::PatternData recording {};
        bool isPressed = false;
        bool hasPendingStep = false;
        TickType_t pressStartTick = 0;
        TickType_t releaseStartTick = 0;

        lv_obj_t* tapZone;
        lv_obj_t* stepLabel;
        lv_obj_t* btnPreview;
        lv_obj_t* btnSave;
        lv_obj_t* btnCancel;
      };
    }
  }
}
