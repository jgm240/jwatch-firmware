#include "displayapp/widgets/complications/NowPlayingComplication.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

NowPlayingComplication::NowPlayingComplication(const Controllers::MusicService& musicService) : musicService {musicService} {
}

void NowPlayingComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, "");
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void NowPlayingComplication::Update() {
  isPlaying = musicService.isPlaying();
  track = musicService.getTrack();
  if (isPlaying.IsUpdated() || track.IsUpdated()) {
    if (isPlaying.Get()) {
      lv_label_set_text_fmt(label, "%s %s", Screens::Symbols::music, track.Get().c_str());
    } else {
      lv_label_set_text_static(label, "");
    }
    lv_obj_realign(label);
  }
}
