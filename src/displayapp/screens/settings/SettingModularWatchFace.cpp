#include "displayapp/screens/settings/SettingModularWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Applications::Widgets::ComplicationType type;
    const char* name;
  };

  constexpr std::array<Option, SettingModularWatchFace::optionsCount> options = {{
    {Pinetime::Applications::Widgets::ComplicationType::Battery, "Battery"},
    {Pinetime::Applications::Widgets::ComplicationType::Steps, "Steps"},
    {Pinetime::Applications::Widgets::ComplicationType::HeartRate, "Heart rate"},
    {Pinetime::Applications::Widgets::ComplicationType::Date, "Date"},
    {Pinetime::Applications::Widgets::ComplicationType::Weather, "Weather"},
    {Pinetime::Applications::Widgets::ComplicationType::NotificationIcon, "Notifications"},
    {Pinetime::Applications::Widgets::ComplicationType::Alarm, "Alarm"},
    {Pinetime::Applications::Widgets::ComplicationType::NowPlaying, "Now Playing"},
    {Pinetime::Applications::Widgets::ComplicationType::Cube, "Cube"},
    {Pinetime::Applications::Widgets::ComplicationType::FitnessRings, "Fitness Rings"},
  }};

  constexpr std::array<const char*, SettingModularWatchFace::nSlots> slotTitles = {"Slot 1", "Slot 2", "Slot 3", "Slot 4"};

  uint32_t IndexOf(Pinetime::Applications::Widgets::ComplicationType type) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].type == type) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingModularWatchFace::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

SettingModularWatchFace::SettingModularWatchFace(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingModularWatchFace::~SettingModularWatchFace() {
  lv_obj_clean(lv_scr_act());
}

bool SettingModularWatchFace::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingModularWatchFace::CreateScreen(unsigned int screenNum) const {
  unsigned int slotIndex = screenNum / pagesPerSlot;
  unsigned int pageInSlot = screenNum % pagesPerSlot;

  std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < optionsPerScreen; i++) {
    unsigned int optionIndex = i + (pageInSlot * optionsPerScreen);
    if (optionIndex >= options.size()) {
      optionsOnThisScreen[i] = {"", false};
    } else {
      optionsOnThisScreen[i] = Screens::CheckboxList::Item {options[optionIndex].name, true};
    }
  }

  return std::make_unique<Screens::CheckboxList>(
    pageInSlot,
    pagesPerSlot,
    slotTitles[slotIndex],
    Symbols::paintbrush,
    IndexOf(settingsController.GetModularSlot(slotIndex)),
    [this, slotIndex, &settings = settingsController](uint32_t index) {
      settings.SetModularSlot(slotIndex, options[index].type);
      settings.SaveSettings();
    },
    optionsOnThisScreen);
}
