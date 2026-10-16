#include "displayapp/widgets/complications/WorldClockComplication.h"
#include <chrono>
#include "components/datetime/WorldTime.h"

using namespace Pinetime::Applications::Widgets;

WorldClockComplication::WorldClockComplication(Controllers::DateTime& dateTimeController, Controllers::Settings& settingsController)
  : dateTimeController {dateTimeController}, settingsController {settingsController} {
}

void WorldClockComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void WorldClockComplication::Update() {
  const uint8_t index = settingsController.GetWorldClockCity(0) % Controllers::worldCityCount;
  const auto& city = Controllers::worldCities[index];
  const int64_t utc = std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.UTCDateTime().time_since_epoch()).count();
  const int64_t local = utc + static_cast<int64_t>(Controllers::WorldOffsetMinutes(city, utc)) * 60;
  const int64_t secondsToday = local - Controllers::DayNumber(local) * 86400;
  const int hour = static_cast<int>(secondsToday / 3600);
  const int minute = static_cast<int>((secondsToday % 3600) / 60);

  const int32_t key = (index * 24 + hour) * 60 + minute;
  if (key == shownKey) {
    return;
  }
  shownKey = key;

  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    lv_label_set_text_fmt(label, "%s %d:%02d%c", city.code, hour % 12 == 0 ? 12 : hour % 12, minute, hour < 12 ? 'a' : 'p');
  } else {
    lv_label_set_text_fmt(label, "%s %02d:%02d", city.code, hour, minute);
  }
  lv_obj_realign(label);
}
