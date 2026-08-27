#pragma once

#include <optional>
#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class WeatherComplication : public Complication {
      public:
        WeatherComplication(const Controllers::SimpleWeatherService& weatherService, const Controllers::Settings& settingsController);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::SimpleWeatherService& weatherService;
        const Controllers::Settings& settingsController;
        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
