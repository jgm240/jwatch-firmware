#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // One screen for the whole watch face: cycle the face, pick its color(s) from a swatch grid and jump to the
      // complication slots (Modular face). Every change is applied and saved straight away.
      class FaceEditor : public Screen {
      public:
        struct Item {
          const char* name;
          WatchFace watchface;
          bool enabled;
        };

        FaceEditor(DisplayApp* app,
                   std::array<Item, UserWatchFaceTypes::Count>&& faces,
                   Pinetime::Controllers::Settings& settingsController);
        ~FaceEditor() override;

        void OnClick(lv_obj_t* object);

      private:
        enum class Target : uint8_t { Accent, Time, Bar, Background, Simple };

        void ChangeFace(int step);
        void CycleTarget();
        void Refresh();
        Target CurrentTarget() const;
        uint8_t TargetCount() const;
        Pinetime::Controllers::Settings::Colors GetColor(Target target) const;
        void SetColor(Target target, Pinetime::Controllers::Settings::Colors color);

        static constexpr uint8_t colorCount = 18;

        DisplayApp* app;
        std::array<Item, UserWatchFaceTypes::Count> faces;
        Pinetime::Controllers::Settings& settingsController;
        uint8_t targetIndex = 0;

        lv_obj_t* faceLabel;
        lv_obj_t* prevButton;
        lv_obj_t* nextButton;
        lv_obj_t* targetButton;
        lv_obj_t* targetLabel;
        lv_obj_t* slotsButton;
        lv_obj_t* doneButton;
        lv_obj_t* swatches[colorCount];
      };
    }
  }
}
