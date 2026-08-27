#include "displayapp/screens/ActivityHistory.h"
#include <lvgl/lvgl.h>
#include <algorithm>
#include <functional>
#include <utility>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Generic 7-bar history page: activityHistoryDays persisted completed days plus today's
  // own in-progress total, refreshed live. Parameterized so it covers all three metrics.
  class HistoryChart : public Screen {
  public:
    HistoryChart(uint8_t screenID,
                 uint8_t numScreens,
                 const char* title,
                 const char* unit,
                 std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> historyValues,
                 uint32_t goal,
                 std::function<uint32_t()> getLiveValue)
      : unit {unit}, goal {goal}, getLiveValue {std::move(getLiveValue)}, pageIndicator(screenID, numScreens) {
      pageIndicator.Create();

      lv_obj_t* titleLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_text_static(titleLabel, title);
      lv_label_set_align(titleLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(titleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

      chart = lv_chart_create(lv_scr_act(), nullptr);
      lv_obj_set_size(chart, 200, 140);
      lv_obj_align(chart, lv_scr_act(), LV_ALIGN_CENTER, 0, 5);
      lv_chart_set_type(chart, LV_CHART_TYPE_COLUMN);
      lv_chart_set_point_count(chart, Pinetime::Controllers::Settings::activityHistoryDays + 1);
      lv_chart_set_div_line_count(chart, 3, 0);
      lv_obj_set_style_local_bg_opa(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
      lv_obj_set_style_local_border_width(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, 0);

      series = lv_chart_add_series(chart, Colors::blue);

      uint32_t maxValue = goal;
      lv_coord_t points[Pinetime::Controllers::Settings::activityHistoryDays + 1];
      for (size_t i = 0; i < historyValues.size(); i++) {
        points[i] = static_cast<lv_coord_t>(historyValues[i]);
        maxValue = std::max(maxValue, historyValues[i]);
      }
      points[historyValues.size()] = 0; // today, filled in by Refresh()
      lv_chart_set_points(chart, series, points);
      lv_chart_set_y_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, static_cast<lv_coord_t>(maxValue + maxValue / 5 + 1));

      valueLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_label_set_align(valueLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(valueLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -5);

      taskRefresh = lv_task_create(RefreshTaskCallback, 5000, LV_TASK_PRIO_MID, this);
      Refresh();
    }

    ~HistoryChart() override {
      lv_task_del(taskRefresh);
      lv_obj_clean(lv_scr_act());
    }

    void Refresh() override {
      uint32_t live = getLiveValue();
      lv_chart_set_point_id(chart, series, static_cast<lv_coord_t>(live), Pinetime::Controllers::Settings::activityHistoryDays);
      lv_chart_refresh(chart);

      lv_label_set_text_fmt(valueLabel, "Today: %lu%s / %lu%s", live, unit, goal, unit);
      lv_obj_align(valueLabel, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }

  private:
    const char* unit;
    uint32_t goal;
    std::function<uint32_t()> getLiveValue;

    lv_obj_t* chart;
    lv_chart_series_t* series;
    lv_obj_t* valueLabel;

    lv_task_t* taskRefresh;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };

  std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> CaloriesHistory(Pinetime::Controllers::Settings& s) {
    std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> values {};
    for (size_t i = 0; i < values.size(); i++) {
      values[i] = s.GetActivityHistoryDay(i).calories;
    }
    return values;
  }

  std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> ExerciseHistory(Pinetime::Controllers::Settings& s) {
    std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> values {};
    for (size_t i = 0; i < values.size(); i++) {
      values[i] = s.GetActivityHistoryDay(i).exerciseMinutes;
    }
    return values;
  }

  std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> StandHistory(Pinetime::Controllers::Settings& s) {
    std::array<uint32_t, Pinetime::Controllers::Settings::activityHistoryDays> values {};
    for (size_t i = 0; i < values.size(); i++) {
      values[i] = s.GetActivityHistoryDay(i).standHours;
    }
    return values;
  }
}

auto ActivityHistory::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

ActivityHistory::ActivityHistory(DisplayApp* app, Controllers::HealthController& healthController, Controllers::Settings& settingsController)
  : app {app},
    healthController {healthController},
    settingsController {settingsController},
    screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

ActivityHistory::~ActivityHistory() {
  lv_obj_clean(lv_scr_act());
}

bool ActivityHistory::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> ActivityHistory::CreateScreen(unsigned int screenNum) const {
  if (screenNum == 0) {
    return std::make_unique<HistoryChart>(0,
                                          nScreens,
                                          "Calories (week)",
                                          "cal",
                                          CaloriesHistory(settingsController),
                                          settingsController.GetCaloriesGoal(),
                                          [&health = healthController]() { return health.GetCaloriesToday(); });
  }
  if (screenNum == 1) {
    return std::make_unique<HistoryChart>(1,
                                          nScreens,
                                          "Exercise (week)",
                                          "m",
                                          ExerciseHistory(settingsController),
                                          settingsController.GetExerciseGoalMinutes(),
                                          [&health = healthController]() { return health.GetActiveMinutesToday(); });
  }
  return std::make_unique<HistoryChart>(2,
                                        nScreens,
                                        "Stand (week)",
                                        "h",
                                        StandHistory(settingsController),
                                        settingsController.GetStandGoalHours(),
                                        [&health = healthController]() { return health.GetStandHoursToday(); });
}
