#include "displayapp/screens/ListApplicationList.h"
#include "displayapp/screens/AppList.h"
#include <lvgl/lvgl.h>
#include <functional>
#include <algorithm>
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

auto ListApplicationList::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

ListApplicationList::ListApplicationList(DisplayApp* app,
                                         Pinetime::Controllers::Settings& settingsController,
                                         const Pinetime::Controllers::Battery& batteryController,
                                         const Pinetime::Controllers::Ble& bleController,
                                         const Pinetime::Controllers::AlarmController& alarmController,
                                         Controllers::DateTime& dateTimeController,
                                         Pinetime::Controllers::FS& filesystem,
                                         std::array<Tile::Applications, UserAppTypes::Count>&& apps)
  : app {app},
    settingsController {settingsController},
    batteryController {batteryController},
    bleController {bleController},
    alarmController {alarmController},
    dateTimeController {dateTimeController},
    filesystem {filesystem},
    apps {std::move(apps)},
    screens {app, settingsController.GetAppMenu(), CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

ListApplicationList::~ListApplicationList() {
  lv_obj_clean(lv_scr_act());
}

bool ListApplicationList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> ListApplicationList::CreateScreen(unsigned int screenNum) const {
  std::array<Tile::Applications, appsPerScreen> pageApps;

  for (int i = 0; i < appsPerScreen; i++) {
    if (i + (screenNum * appsPerScreen) >= apps.size()) {
      pageApps[i] = {"", Pinetime::Applications::Apps::None, false};
    } else {
      pageApps[i] = apps[i + (screenNum * appsPerScreen)];
    }
  }

  return std::make_unique<Screens::AppList>(screenNum,
                                            nScreens,
                                            app,
                                            settingsController,
                                            batteryController,
                                            bleController,
                                            alarmController,
                                            dateTimeController,
                                            pageApps);
}
