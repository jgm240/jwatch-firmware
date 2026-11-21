#pragma once

#include "displayapp/screens/Screen.h"
#include "BootErrors.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Error : public Screen {
      public:
        // 'crashMessage' is only used when error == BootErrors::FirmwareCrash; may be nullptr.
        Error(System::BootErrors error, const char* crashMessage = nullptr);
        ~Error() override;

        void ButtonEventHandler();

      private:
        lv_obj_t* btnOk;
      };
    }
  }
}
