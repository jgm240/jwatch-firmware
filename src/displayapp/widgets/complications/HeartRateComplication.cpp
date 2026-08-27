#include "displayapp/widgets/complications/HeartRateComplication.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

HeartRateComplication::HeartRateComplication(const Controllers::HeartRateController& heartRateController)
  : heartRateController {heartRateController} {
}

void HeartRateComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void HeartRateComplication::Update() {
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(label, "%s %d", Screens::Symbols::heartBeat, heartbeat.Get());
    } else {
      lv_label_set_text_fmt(label, "%s --", Screens::Symbols::heartBeat);
    }
    lv_obj_realign(label);
  }
}
