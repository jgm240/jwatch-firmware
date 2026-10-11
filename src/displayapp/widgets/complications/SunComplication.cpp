#include "displayapp/widgets/complications/SunComplication.h"

using namespace Pinetime::Applications::Widgets;

SunComplication::SunComplication(const Controllers::SimpleWeatherService& weatherService, Controllers::DateTime& dateTimeController)
  : weatherService {weatherService}, dateTimeController {dateTimeController} {
}

void SunComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void SunComplication::Update() {
  // sunrise/sunset are minutes into the local day; negative values mean unknown / no sunrise today.
  int32_t key = 0;
  bool isSunrise = false;
  int16_t minutes = -1;

  const auto current = weatherService.Current();
  if (current.has_value() && current->sunrise >= 0 && current->sunset >= 0) {
    const int32_t now = dateTimeController.Hours() * 60 + dateTimeController.Minutes();
    if (now < current->sunrise) {
      isSunrise = true;
      minutes = current->sunrise;
    } else if (now < current->sunset) {
      minutes = current->sunset;
    } else {
      isSunrise = true; // after sunset: the next one is tomorrow's sunrise (approximately today's time)
      minutes = current->sunrise;
    }
    key = (minutes + 1) * 2 + (isSunrise ? 1 : 0);
  }

  if (key == shownKey) {
    return;
  }
  shownKey = key;

  if (key == 0) {
    lv_label_set_text_static(label, "");
  } else {
    lv_label_set_text_fmt(label, "%s %02d:%02d", isSunrise ? "Rise" : "Set", minutes / 60, minutes % 60);
  }
  lv_obj_realign(label);
}
