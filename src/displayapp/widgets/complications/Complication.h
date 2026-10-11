#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      enum class ComplicationType : uint8_t {
        None,
        Battery,
        Steps,
        HeartRate,
        Date,
        Weather,
        NotificationIcon,
        Alarm,
        NowPlaying,
        Cube,
        FitnessRings,
        NextEvent,
        Countdown,
        Sun,
        PhoneBattery
      };

      class Complication {
      public:
        virtual ~Complication() = default;

        virtual void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) = 0;
        virtual void Update() = 0;
      };
    }
  }
}
