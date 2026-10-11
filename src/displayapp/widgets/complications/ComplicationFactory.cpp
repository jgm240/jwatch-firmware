#include "displayapp/widgets/complications/ComplicationFactory.h"
#include "displayapp/widgets/complications/BatteryComplication.h"
#include "displayapp/widgets/complications/StepsComplication.h"
#include "displayapp/widgets/complications/HeartRateComplication.h"
#include "displayapp/widgets/complications/DateComplication.h"
#include "displayapp/widgets/complications/WeatherComplication.h"
#include "displayapp/widgets/complications/NotificationIconComplication.h"
#include "displayapp/widgets/complications/AlarmComplication.h"
#include "displayapp/widgets/complications/NowPlayingComplication.h"
#include "displayapp/widgets/complications/CubeComplication.h"
#include "displayapp/widgets/complications/FitnessRingsComplication.h"
#include "displayapp/widgets/complications/NextEventComplication.h"
#include "displayapp/widgets/complications/CountdownComplication.h"
#include "displayapp/widgets/complications/SunComplication.h"
#include "displayapp/widgets/complications/PhoneBatteryComplication.h"

using namespace Pinetime::Applications::Widgets;

std::unique_ptr<Complication> Pinetime::Applications::Widgets::CreateComplication(ComplicationType type, AppControllers& controllers) {
  switch (type) {
    case ComplicationType::Battery:
      return std::make_unique<BatteryComplication>(controllers.batteryController);
    case ComplicationType::Steps:
      return std::make_unique<StepsComplication>(controllers.motionController);
    case ComplicationType::HeartRate:
      return std::make_unique<HeartRateComplication>(controllers.heartRateController);
    case ComplicationType::Date:
      return std::make_unique<DateComplication>(controllers.dateTimeController);
    case ComplicationType::Weather:
      return std::make_unique<WeatherComplication>(*controllers.weatherController, controllers.settingsController);
    case ComplicationType::NotificationIcon:
      return std::make_unique<NotificationIconComplication>(controllers.notificationManager);
    case ComplicationType::Alarm:
      return std::make_unique<AlarmComplication>(controllers.alarmController);
    case ComplicationType::NowPlaying:
      return std::make_unique<NowPlayingComplication>(*controllers.musicService);
    case ComplicationType::Cube:
      return std::make_unique<CubeComplication>(controllers.motionController);
    case ComplicationType::FitnessRings:
      return std::make_unique<FitnessRingsComplication>(controllers.healthController, controllers.settingsController);
    case ComplicationType::NextEvent:
      return std::make_unique<NextEventComplication>(*controllers.calendarService, controllers.dateTimeController);
    case ComplicationType::Countdown:
      return std::make_unique<CountdownComplication>(*controllers.calendarService, controllers.dateTimeController);
    case ComplicationType::Sun:
      return std::make_unique<SunComplication>(*controllers.weatherController, controllers.dateTimeController);
    case ComplicationType::PhoneBattery:
      return std::make_unique<PhoneBatteryComplication>(*controllers.calendarService);
    case ComplicationType::None:
    default:
      return nullptr;
  }
}
