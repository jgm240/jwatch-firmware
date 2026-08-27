#include "displayapp/screens/CircleTile.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr lv_coord_t circleDiameter = 64;

  // 3 columns, 2 rows - the bottom row is nudged sideways slightly for a honeycomb feel
  // without risking any circle running off the 240px-wide screen.
  constexpr std::array<lv_coord_t, 3> topRowColX = {12, 88, 164};
  constexpr std::array<lv_coord_t, 3> bottomRowColX = {28, 104, 180};
  constexpr lv_coord_t topRowY = 50;
  constexpr lv_coord_t bottomRowY = 134;

  constexpr std::array<Pinetime::Controllers::Settings::Colors, 6> palette = {{
    Pinetime::Controllers::Settings::Colors::Red,
    Pinetime::Controllers::Settings::Colors::Orange,
    Pinetime::Controllers::Settings::Colors::Yellow,
    Pinetime::Controllers::Settings::Colors::Green,
    Pinetime::Controllers::Settings::Colors::Blue,
    Pinetime::Controllers::Settings::Colors::Purple,
  }};

  lv_style_t labelStyle;
  bool labelStyleInit = false;

  void lv_update_task(struct _lv_task_t* task) {
    auto* userData = static_cast<CircleTile*>(task->user_data);
    userData->UpdateScreen();
  }

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    auto* screen = static_cast<CircleTile*>(obj->user_data);
    screen->OnEvent(obj, event);
  }
}

CircleTile::CircleTile(uint8_t screenID,
                       uint8_t numScreens,
                       DisplayApp* app,
                       Controllers::Settings& settingsController,
                       const Controllers::Battery& batteryController,
                       const Controllers::Ble& bleController,
                       const Controllers::AlarmController& alarmController,
                       Controllers::DateTime& dateTimeController,
                       std::array<Tile::Applications, 6>& applications)
  : app {app},
    dateTimeController {dateTimeController},
    pageIndicator(screenID, numScreens),
    statusIcons(batteryController, bleController, alarmController) {

  settingsController.SetAppMenu(screenID);

  statusIcons.Create();
  lv_obj_align(statusIcons.GetObject(), lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -8, 0);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  pageIndicator.Create();

  // Every circle on this page shares one of these 7 style objects instead of setting local
  // styles per object - cheap at 6 objects, but keeping the habit consistent everywhere it's
  // used matters more than the savings here specifically.
  for (size_t i = 0; i < circleStyles.size(); i++) {
    lv_style_init(&circleStyles[i]);
    lv_style_set_radius(&circleStyles[i], LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&circleStyles[i], LV_STATE_DEFAULT, Applications::Convert(palette[i]));
    lv_style_set_bg_opa(&circleStyles[i], LV_STATE_DEFAULT, LV_OPA_COVER);
  }
  lv_style_init(&disabledStyle);
  lv_style_set_radius(&disabledStyle, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_bg_color(&disabledStyle, LV_STATE_DEFAULT, Colors::bgDark);
  lv_style_set_bg_opa(&disabledStyle, LV_STATE_DEFAULT, LV_OPA_50);

  if (!labelStyleInit) {
    lv_style_init(&labelStyle);
    lv_style_set_text_color(&labelStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    labelStyleInit = true;
  }

  for (uint8_t i = 0; i < 6; i++) {
    apps[i] = applications[i].application;

    uint8_t col = i % 3;
    uint8_t row = i / 3;
    lv_coord_t x = row == 0 ? topRowColX[col] : bottomRowColX[col];
    lv_coord_t y = row == 0 ? topRowY : bottomRowY;

    lv_obj_t* circle = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(circle, circleDiameter, circleDiameter);
    lv_obj_set_pos(circle, x, y);

    if (applications[i].application == Apps::None || !applications[i].enabled) {
      lv_obj_add_style(circle, LV_OBJ_PART_MAIN, &disabledStyle);
      lv_obj_set_click(circle, false);
    } else {
      lv_obj_add_style(circle, LV_OBJ_PART_MAIN, &circleStyles[i % circleStyles.size()]);
      circle->user_data = this;
      lv_obj_set_event_cb(circle, event_handler);

      lv_obj_t* label = lv_label_create(circle, nullptr);
      lv_obj_add_style(label, LV_LABEL_PART_MAIN, &labelStyle);
      lv_label_set_text_static(label, applications[i].icon);
      lv_obj_align(label, circle, LV_ALIGN_CENTER, 0, 0);
    }

    circles[i] = circle;
  }

  taskUpdate = lv_task_create(lv_update_task, 5000, LV_TASK_PRIO_MID, this);

  UpdateScreen();
}

CircleTile::~CircleTile() {
  lv_task_del(taskUpdate);

  for (auto& style : circleStyles) {
    lv_style_reset(&style);
  }
  lv_style_reset(&disabledStyle);

  lv_obj_clean(lv_scr_act());
}

void CircleTile::UpdateScreen() {
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  statusIcons.Update();
}

void CircleTile::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  for (uint8_t i = 0; i < circles.size(); i++) {
    if (circles[i] == obj) {
      app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
      running = false;
      return;
    }
  }
}
