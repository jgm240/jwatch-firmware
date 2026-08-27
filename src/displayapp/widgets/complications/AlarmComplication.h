#pragma once

#include "displayapp/widgets/complications/Complication.h"
#include "components/alarm/AlarmController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class AlarmComplication : public Complication {
      public:
        explicit AlarmComplication(const Controllers::AlarmController& alarmController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::AlarmController& alarmController;
        Utility::DirtyValue<bool> alarmEnabled {};
        Utility::DirtyValue<uint8_t> alarmHours {};
        Utility::DirtyValue<uint8_t> alarmMinutes {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
