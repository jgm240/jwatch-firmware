#include "displayapp/widgets/complications/WeatherComplication.h"

using namespace Pinetime::Applications::Widgets;

WeatherComplication::WeatherComplication(const Controllers::SimpleWeatherService& weatherService, const Controllers::Settings& settingsController)
  : weatherService {weatherService}, settingsController {settingsController} {
}

void WeatherComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void WeatherComplication::Update() {
  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature.Celsius();
      char tempUnit = 'C';
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = optCurrentWeather->temperature.Fahrenheit();
        tempUnit = 'F';
      }
      lv_label_set_text_fmt(label, "%d°%c", temp, tempUnit);
    } else {
      lv_label_set_text_static(label, "");
    }
    lv_obj_realign(label);
  }
}
