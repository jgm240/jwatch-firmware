#pragma once

#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/NotificationManager.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class NotificationIconComplication : public Complication {
      public:
        explicit NotificationIconComplication(const Controllers::NotificationManager& notificationManager);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::NotificationManager& notificationManager;
        Utility::DirtyValue<bool> notificationState {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
