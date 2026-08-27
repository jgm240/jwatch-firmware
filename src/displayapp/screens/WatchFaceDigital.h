#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/widgets/complications/HeartRateComplication.h"
#include "displayapp/widgets/complications/StepsComplication.h"
#include "displayapp/widgets/complications/WeatherComplication.h"
#include "displayapp/widgets/complications/NotificationIconComplication.h"
#include "displayapp/widgets/SecondsSquare.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class AlarmController;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceDigital : public Screen {
      public:
        WatchFaceDigital(Controllers::DateTime& dateTimeController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         const Controllers::AlarmController& alarmController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController,
                         Controllers::SimpleWeatherService& weather);
        ~WatchFaceDigital() override;

        void Refresh() override;

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        Widgets::SecondsSquare secondsSquare;

        // Opt-in (Settings > Watchface color > Weather effects) rain/snow particle drift
        // behind the time, driven by the current weather condition. Plain lv_obj_t rectangles
        // moved each tick, same idiom as RingsClosed's confetti - not a canvas, to avoid that
        // approach's full-screen-sized RAM buffer.
        enum class WeatherEffect : uint8_t { None, Rain, Snow };
        static constexpr int weatherParticleCount = 10;
        WeatherEffect currentWeatherEffect = WeatherEffect::None;
        std::array<lv_obj_t*, weatherParticleCount> weatherParticles {};
        std::array<lv_coord_t, weatherParticleCount> weatherParticleX {};
        std::array<lv_coord_t, weatherParticleCount> weatherParticleY {};
        std::array<lv_coord_t, weatherParticleCount> weatherParticleSpeed {};

        void UpdateWeatherEffect();

        Widgets::NotificationIconComplication notificationComplication;
        Widgets::WeatherComplication weatherComplication;
        Widgets::HeartRateComplication heartRateComplication;
        Widgets::StepsComplication stepsComplication;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Digital> {
      static constexpr WatchFace watchFace = WatchFace::Digital;
      static constexpr const char* name = "Digital";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceDigital(controllers.dateTimeController,
                                             controllers.batteryController,
                                             controllers.bleController,
                                             controllers.alarmController,
                                             controllers.notificationManager,
                                             controllers.settingsController,
                                             controllers.heartRateController,
                                             controllers.motionController,
                                             *controllers.weatherController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
