#pragma once

#include <chrono>
#include "displayapp/widgets/complications/Complication.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class DateComplication : public Complication {
      public:
        explicit DateComplication(Controllers::DateTime& dateTimeController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        Controllers::DateTime& dateTimeController;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
