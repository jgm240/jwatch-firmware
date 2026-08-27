#include "displayapp/screens/AppList.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/AppNames.h"
#include "components/ble/BleController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_coord_t rowHeight = 32;
  constexpr lv_coord_t rowGap = 4;
  constexpr lv_coord_t firstRowY = 38;

  void lv_update_task(struct _lv_task_t* task) {
    auto* userData = static_cast<AppList*>(task->user_data);
    userData->UpdateScreen();
  }

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    static_cast<AppList*>(obj->user_data)->OnRowClicked(obj);
  }
}

AppList::AppList(uint8_t screenID,
                 uint8_t numScreens,
                 DisplayApp* app,
                 Controllers::Settings& settingsController,
                 const Controllers::Battery& batteryController,
                 const Controllers::Ble& bleController,
                 const Controllers::AlarmController& alarmController,
                 Controllers::DateTime& dateTimeController,
                 std::array<Tile::Applications, 6>& applications)
  : app {app}, dateTimeController {dateTimeController}, pageIndicator(screenID, numScreens), statusIcons(batteryController, bleController, alarmController) {

  settingsController.SetAppMenu(screenID);

  statusIcons.Create();
  lv_obj_align(statusIcons.GetObject(), lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -8, 0);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  pageIndicator.Create();

  for (uint8_t i = 0; i < 6; i++) {
    apps[i] = applications[i].application;

    lv_obj_t* row = lv_obj_create(lv_scr_act(), nullptr);
    rows[i] = row;
    lv_obj_set_size(row, LV_HOR_RES - 16, rowHeight);
    lv_obj_set_pos(row, 8, firstRowY + i * (rowHeight + rowGap));
    lv_obj_set_style_local_radius(row, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Styles::GetButtonRadius());
    lv_obj_set_style_local_bg_color(row, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_border_width(row, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    bool enabled = applications[i].application != Apps::None && applications[i].enabled;
    if (!enabled) {
      lv_obj_set_style_local_bg_opa(row, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
      lv_obj_set_click(row, false);
    } else {
      row->user_data = this;
      lv_obj_set_event_cb(row, event_handler);
    }

    if (applications[i].application != Apps::None) {
      lv_obj_t* icon = lv_label_create(row, nullptr);
      lv_label_set_text_static(icon, applications[i].icon);
      lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
      lv_obj_align(icon, row, LV_ALIGN_IN_LEFT_MID, 12, 0);

      lv_obj_t* name = lv_label_create(row, nullptr);
      lv_label_set_text_static(name, Pinetime::Applications::AppNameFor(applications[i].application));
      lv_obj_align(name, row, LV_ALIGN_IN_LEFT_MID, 44, 0);
    }
  }

  taskUpdate = lv_task_create(lv_update_task, 5000, LV_TASK_PRIO_MID, this);

  UpdateScreen();
}

AppList::~AppList() {
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
}

void AppList::UpdateScreen() {
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  statusIcons.Update();
}

void AppList::OnRowClicked(lv_obj_t* obj) {
  for (uint8_t i = 0; i < rows.size(); i++) {
    if (rows[i] == obj) {
      app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
      running = false;
      return;
    }
  }
}
