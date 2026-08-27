#pragma once

#include <string>
#include "displayapp/widgets/complications/Complication.h"
#include "components/ble/MusicService.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      class NowPlayingComplication : public Complication {
      public:
        explicit NowPlayingComplication(const Controllers::MusicService& musicService);

        void Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) override;
        void Update() override;

      private:
        const Controllers::MusicService& musicService;
        Utility::DirtyValue<bool> isPlaying {};
        Utility::DirtyValue<std::string> track {};
        lv_obj_t* label = nullptr;
      };
    }
  }
}
