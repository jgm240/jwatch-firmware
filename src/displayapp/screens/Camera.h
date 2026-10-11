#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/Controllers.h"
#include "components/ble/HidService.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // One big button that taps the HID "volume up" key. With the phone's Camera app open,
      // that triggers the shutter - the same trick cheap Bluetooth camera remotes use.
      class Camera : public Screen {
      public:
        explicit Camera(Controllers::HidService& hidService);
        ~Camera() override;

        void Shutter();

      private:
        Controllers::HidService& hidService;
        lv_obj_t* button = nullptr;
      };
    }

    template <>
    struct AppTraits<Apps::Camera> {
      static constexpr Apps app = Apps::Camera;
      static constexpr const char* icon = Screens::Symbols::eye;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Camera(*controllers.hidService);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
