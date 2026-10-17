#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/Controllers.h"
#include "components/ble/BleController.h"
#include "components/ble/NimbleController.h"
#include "systemtask/WakeLock.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Turns the watch into a Kindle Click page-turner remote. While this app is open the phone link is dropped and the
      // watch pairs with a Kindle (Settings > Bluetooth on the Kindle); closing the app brings the phone back.
      class KindleClick : public Screen {
      public:
        enum class Key : uint8_t { Back, Forward, BrightnessDown, Sleep, BrightnessUp };

        KindleClick(Controllers::NimbleController& nimble, const Controllers::Ble& bleController, System::SystemTask& systemTask);
        ~KindleClick() override;

        void Refresh() override;
        void Press(lv_obj_t* button);

      private:
        void Release();

        Controllers::NimbleController& nimble;
        const Controllers::Ble& bleController;
        System::WakeLock wakeLock;

        bool releasePending = false;
        bool pressedConsumer = false;
        uint8_t ticks = 0;
        uint8_t announcingTicks = 0;

        lv_obj_t* statusLabel;
        lv_obj_t* hintLabel;
        lv_obj_t* buttons[5];
        lv_obj_t* announceButton;
        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::KindleClick> {
      static constexpr Apps app = Apps::KindleClick;
      static constexpr const char* icon = Screens::Symbols::list;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::KindleClick(controllers.systemTask->nimble(), controllers.bleController, *controllers.systemTask);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
