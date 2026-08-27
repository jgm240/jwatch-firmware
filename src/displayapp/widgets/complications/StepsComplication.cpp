#include "displayapp/widgets/complications/StepsComplication.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

StepsComplication::StepsComplication(const Controllers::MotionController& motionController) : motionController {motionController} {
}

void StepsComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void StepsComplication::Update() {
  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(label, "%s %lu", Screens::Symbols::shoe, stepCount.Get());
    lv_obj_realign(label);
  }
}
