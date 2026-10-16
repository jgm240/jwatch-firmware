#include "displayapp/screens/settings/FaceEditor.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Colors.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::Settings;

namespace {
  void EventHandler(lv_obj_t* object, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<FaceEditor*>(object->user_data)->OnClick(object);
    }
  }

  lv_obj_t* MakeButton(lv_obj_t* parent, FaceEditor* owner, const char* text, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_obj_t** labelOut = nullptr) {
    lv_obj_t* button = lv_btn_create(parent, nullptr);
    button->user_data = owner;
    lv_obj_set_event_cb(button, EventHandler);
    lv_obj_set_size(button, w, h);
    lv_obj_set_pos(button, x, y);
    lv_obj_t* label = lv_label_create(button, nullptr);
    lv_label_set_text(label, text);
    if (labelOut != nullptr) {
      *labelOut = label;
    }
    return button;
  }
}

FaceEditor::FaceEditor(DisplayApp* app, std::array<Item, UserWatchFaceTypes::Count>&& faces, Pinetime::Controllers::Settings& settingsController)
  : app {app}, faces {std::move(faces)}, settingsController {settingsController} {

  lv_obj_t* screen = lv_scr_act();
  prevButton = MakeButton(screen, this, "<", 0, 0, 54, 40);
  nextButton = MakeButton(screen, this, ">", 186, 0, 54, 40);
  faceLabel = lv_label_create(screen, nullptr);
  lv_label_set_align(faceLabel, LV_LABEL_ALIGN_CENTER);

  targetButton = MakeButton(screen, this, "", 0, 44, 118, 34, &targetLabel);
  slotsButton = MakeButton(screen, this, "Slots >", 122, 44, 118, 34);

  for (uint8_t i = 0; i < colorCount; i++) {
    lv_obj_t* swatch = lv_obj_create(screen, nullptr);
    swatch->user_data = this;
    lv_obj_set_event_cb(swatch, EventHandler);
    lv_obj_set_size(swatch, 32, 32);
    lv_obj_set_pos(swatch, 4 + (i % 6) * 40, 86 + (i / 6) * 38);
    lv_obj_set_style_local_radius(swatch, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
    lv_obj_set_style_local_bg_color(swatch,
                                    LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    Pinetime::Applications::Convert(static_cast<Settings::Colors>(i)));
    lv_obj_set_style_local_border_color(swatch, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    swatches[i] = swatch;
  }

  doneButton = MakeButton(screen, this, "Done", 20, 202, 200, 36);

  Refresh();
}

FaceEditor::~FaceEditor() {
  lv_obj_clean(lv_scr_act());
}

uint8_t FaceEditor::TargetCount() const {
  switch (settingsController.GetWatchFace()) {
    case WatchFace::Digital:
    case WatchFace::Analog:
    case WatchFace::Modular:
    case WatchFace::Simple:
      return 1;
    case WatchFace::PineTimeStyle:
      return 3;
    default:
      return 0;
  }
}

FaceEditor::Target FaceEditor::CurrentTarget() const {
  switch (settingsController.GetWatchFace()) {
    case WatchFace::PineTimeStyle:
      return targetIndex == 0 ? Target::Time : (targetIndex == 1 ? Target::Bar : Target::Background);
    case WatchFace::Simple:
      return Target::Simple;
    default:
      return Target::Accent;
  }
}

Settings::Colors FaceEditor::GetColor(Target target) const {
  switch (target) {
    case Target::Time:
      return settingsController.GetPTSColorTime();
    case Target::Bar:
      return settingsController.GetPTSColorBar();
    case Target::Background:
      return settingsController.GetPTSColorBG();
    case Target::Simple:
      return settingsController.GetWatchFaceSimpleColor();
    default:
      return settingsController.GetWatchFaceAccentColor();
  }
}

void FaceEditor::SetColor(Target target, Settings::Colors color) {
  switch (target) {
    case Target::Time:
      settingsController.SetPTSColorTime(color);
      break;
    case Target::Bar:
      settingsController.SetPTSColorBar(color);
      break;
    case Target::Background:
      settingsController.SetPTSColorBG(color);
      break;
    case Target::Simple:
      settingsController.SetWatchFaceSimpleColor(color);
      break;
    default:
      settingsController.SetWatchFaceAccentColor(color);
      break;
  }
  settingsController.SaveSettings();
}

void FaceEditor::ChangeFace(int step) {
  const int count = static_cast<int>(faces.size());
  int index = 0;
  for (int i = 0; i < count; i++) {
    if (faces[i].watchface == settingsController.GetWatchFace()) {
      index = i;
    }
  }
  // Skip faces that can't run here (missing resources).
  for (int tries = 0; tries < count; tries++) {
    index = (index + step + count) % count;
    if (faces[index].enabled) {
      break;
    }
  }
  settingsController.SetWatchFace(faces[index].watchface);
  settingsController.SaveSettings();
  targetIndex = 0;
  Refresh();
}

void FaceEditor::CycleTarget() {
  const uint8_t count = TargetCount();
  if (count > 1) {
    targetIndex = (targetIndex + 1) % count;
    Refresh();
  }
}

void FaceEditor::Refresh() {
  const WatchFace current = settingsController.GetWatchFace();
  for (const auto& face : faces) {
    if (face.watchface == current) {
      lv_label_set_text(faceLabel, face.name);
    }
  }
  lv_obj_set_width(faceLabel, 120);
  lv_obj_align(faceLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  const uint8_t targets = TargetCount();
  const bool hasColors = targets > 0;
  lv_obj_set_hidden(targetButton, !hasColors);
  lv_obj_set_hidden(slotsButton, current != WatchFace::Modular);
  if (hasColors) {
    switch (CurrentTarget()) {
      case Target::Time:
        lv_label_set_text(targetLabel, "Time >");
        break;
      case Target::Bar:
        lv_label_set_text(targetLabel, "Bar >");
        break;
      case Target::Background:
        lv_label_set_text(targetLabel, "Background >");
        break;
      case Target::Simple:
        lv_label_set_text(targetLabel, "Color");
        break;
      default:
        lv_label_set_text(targetLabel, "Accent");
        break;
    }
  }

  const uint8_t selected = static_cast<uint8_t>(GetColor(CurrentTarget()));
  for (uint8_t i = 0; i < colorCount; i++) {
    lv_obj_set_hidden(swatches[i], !hasColors);
    lv_obj_set_style_local_border_width(swatches[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, i == selected ? 4 : 0);
  }
}

void FaceEditor::OnClick(lv_obj_t* object) {
  if (object == prevButton) {
    ChangeFace(-1);
  } else if (object == nextButton) {
    ChangeFace(1);
  } else if (object == targetButton) {
    CycleTarget();
  } else if (object == slotsButton) {
    app->StartApp(Apps::SettingModularWatchFace, DisplayApp::FullRefreshDirections::Up);
  } else if (object == doneButton) {
    app->StartApp(Apps::Clock, DisplayApp::FullRefreshDirections::Down);
  } else {
    for (uint8_t i = 0; i < colorCount; i++) {
      if (swatches[i] == object) {
        SetColor(CurrentTarget(), static_cast<Settings::Colors>(i));
        Refresh();
        return;
      }
    }
  }
}
