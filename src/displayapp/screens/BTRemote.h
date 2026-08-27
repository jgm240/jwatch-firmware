#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Controllers {
    class HidService;
  }

  namespace Applications {
    namespace Screens {

      class BTRemote : public Screen {
      public:
        explicit BTRemote(Pinetime::Controllers::HidService& hidService);
        ~BTRemote() override;

        void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

      private:
        bool OnTouchEvent(TouchEvents event) override;

        Pinetime::Controllers::HidService& hidService;

        lv_style_t btn_style;

        lv_obj_t* btnPrev;
        lv_obj_t* btnPlayPause;
        lv_obj_t* btnNext;
        lv_obj_t* btnVolDown;
        lv_obj_t* btnVolUp;

        Widgets::PageIndicator pageIndicator = Widgets::PageIndicator(0, 2);
      };
    }

    template <>
    struct AppTraits<Apps::BTRemote> {
      static constexpr Apps app = Apps::BTRemote;
      static constexpr const char* icon = Screens::Symbols::bluetooth;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::BTRemote(*controllers.hidService);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
