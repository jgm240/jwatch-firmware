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

      // One-tap "make me connectable": drops any stale connection and advertises at the fastest
      // interval for as long as this screen is open, so the phone can find the watch without
      // switching Bluetooth off and on.
      class Reconnect : public Screen {
      public:
        Reconnect(Controllers::NimbleController& nimble, const Controllers::Ble& bleController, System::SystemTask& systemTask);
        ~Reconnect() override;

        void Refresh() override;
        void Announce();

      private:
        Controllers::NimbleController& nimble;
        const Controllers::Ble& bleController;
        System::WakeLock wakeLock;

        lv_obj_t* statusLabel;
        lv_obj_t* hintLabel;
        lv_obj_t* button;
        lv_obj_t* buttonLabel;
        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Reconnect> {
      static constexpr Apps app = Apps::Reconnect;
      static constexpr const char* icon = Screens::Symbols::bluetooth;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Reconnect(controllers.systemTask->nimble(), controllers.bleController, *controllers.systemTask);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
