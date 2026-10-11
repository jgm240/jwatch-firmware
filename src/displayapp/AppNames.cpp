#include "displayapp/AppNames.h"

const char* Pinetime::Applications::AppNameFor(Apps app) {
  switch (app) {
    case Apps::StopWatch:
      return "Stopwatch";
    case Apps::Alarm:
      return "Alarm";
    case Apps::Timer:
      return "Timer";
    case Apps::Steps:
      return "Steps";
    case Apps::HeartRate:
      return "Heart Rate";
    case Apps::Music:
      return "Music";
    case Apps::Paint:
      return "Paint";
    case Apps::Paddle:
      return "Paddle";
    case Apps::Twos:
      return "2048";
    case Apps::Snake:
      return "Snake";
    case Apps::BTRemote:
      return "BT Remote";
    case Apps::FileBrowser:
      return "Files";
    case Apps::Dice:
      return "Dice";
    case Apps::Metronome:
      return "Metronome";
    case Apps::Navigation:
      return "Navigation";
    case Apps::Calculator:
      return "Calculator";
    case Apps::Weather:
      return "Weather";
    case Apps::WorldClock:
      return "World Clock";
    case Apps::Calendar:
      return "Calendar";
    case Apps::Camera:
      return "Camera";
    case Apps::Workout:
      return "Workout";
    case Apps::Mindfulness:
      return "Mindfulness";
    case Apps::ActivityRings:
      return "Activity";
    case Apps::SleepTracking:
      return "Sleep";
    default:
      return "App";
  }
}
