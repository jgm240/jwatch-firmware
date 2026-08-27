#include "displayapp/widgets/complications/BatteryComplication.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

BatteryComplication::BatteryComplication(const Controllers::Battery& batteryController) : batteryController {batteryController} {
}

void BatteryComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void BatteryComplication::Update() {
  percentRemaining = batteryController.PercentRemaining();
  if (percentRemaining.IsUpdated()) {
    lv_label_set_text_fmt(label, "%s %d%%", Screens::Symbols::batteryHalf, percentRemaining.Get());
    lv_obj_realign(label);
  }
}
