#include "displayapp/screens/WatchFaceHandwriting.h"

#include <lvgl/lvgl.h>
#include <cstdio>

#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"

using namespace Pinetime::Applications::Screens;
namespace HandwritingGlyphs = Pinetime::Applications::Widgets::HandwritingGlyphs;

namespace {
  constexpr lv_coord_t slotWidth = HandwritingGlyphs::glyphWidth;
  constexpr lv_coord_t slotHeight = HandwritingGlyphs::glyphHeight;
  constexpr lv_coord_t rowY = 75;
  constexpr lv_coord_t startX = (240 - slotWidth * 5) / 2;

  lv_obj_t* CreateGlyphImage(int slotIndex, lv_color_t inkColor) {
    lv_obj_t* img = lv_img_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(img, startX + slotIndex * slotWidth, rowY);
    lv_obj_set_style_local_image_recolor(img, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, inkColor);
    lv_obj_set_style_local_image_recolor_opa(img, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    return img;
  }
}

WatchFaceHandwriting::WatchFaceHandwriting(Controllers::DateTime& dateTimeController,
                                           const Controllers::Battery& batteryController,
                                           const Controllers::Ble& bleController,
                                           const Controllers::AlarmController& alarmController,
                                           Controllers::Settings& settingsController)
  : dateTimeController {dateTimeController}, statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();

  lv_color_t inkColor = Applications::Convert(settingsController.GetWatchFaceAccentColor());

  imgHourTens = CreateGlyphImage(0, inkColor);
  imgHourOnes = CreateGlyphImage(1, inkColor);
  imgColon = CreateGlyphImage(2, inkColor);
  imgMinuteTens = CreateGlyphImage(3, inkColor);
  imgMinuteOnes = CreateGlyphImage(4, inkColor);

  lv_img_set_src(imgColon, "F:/glyphs/glyph_colon.bin");

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, rowY + slotHeight + 10);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceHandwriting::~WatchFaceHandwriting() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool WatchFaceHandwriting::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  return HandwritingGlyphs::GlyphExists(filesystem, HandwritingGlyphs::colonIndex);
}

void WatchFaceHandwriting::SetDigit(lv_obj_t* imgObj, uint8_t digit) {
  char path[24];
  snprintf(path, sizeof(path), "F:/glyphs/glyph_%u.bin", static_cast<unsigned>(digit));
  lv_img_set_src(imgObj, path);
}

void WatchFaceHandwriting::Refresh() {
  statusIcons.Update();

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    hourTens = static_cast<uint8_t>(hour / 10);
    hourOnes = static_cast<uint8_t>(hour % 10);
    minuteTens = static_cast<uint8_t>(minute / 10);
    minuteOnes = static_cast<uint8_t>(minute % 10);

    if (hourTens.IsUpdated()) {
      SetDigit(imgHourTens, hourTens.Get());
    }
    if (hourOnes.IsUpdated()) {
      SetDigit(imgHourOnes, hourOnes.Get());
    }
    if (minuteTens.IsUpdated()) {
      SetDigit(imgMinuteTens, minuteTens.Get());
    }
    if (minuteOnes.IsUpdated()) {
      SetDigit(imgMinuteOnes, minuteOnes.Get());
    }

    lv_label_set_text_fmt(label_date,
                          "%s %d %s %d",
                          dateTimeController.DayOfWeekShortToString(),
                          dateTimeController.Day(),
                          dateTimeController.MonthShortToString(),
                          dateTimeController.Year());
    lv_obj_realign(label_date);
  }
}
