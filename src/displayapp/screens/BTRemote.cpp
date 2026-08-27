#include "displayapp/screens/BTRemote.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/ble/HidService.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<BTRemote*>(obj->user_data);
    screen->OnObjectEvent(obj, event);
  }

  void SendTap(Pinetime::Controllers::HidService& hidService, uint8_t bit) {
    hidService.SendConsumerReport(bit);
    hidService.SendConsumerReport(0);
  }
}

BTRemote::BTRemote(Pinetime::Controllers::HidService& hidService) : hidService {hidService} {
  lv_obj_t* label;

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, LV_STATE_DEFAULT, 20);
  lv_style_set_bg_color(&btn_style, LV_STATE_DEFAULT, Colors::bgAlt);

  btnVolDown = Styles::Button(lv_scr_act());
  btnVolDown->user_data = this;
  lv_obj_set_event_cb(btnVolDown, event_handler);
  lv_obj_set_size(btnVolDown, 76, 76);
  lv_obj_align(btnVolDown, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 3, 0);
  lv_obj_add_style(btnVolDown, LV_STATE_DEFAULT, &btn_style);
  label = lv_label_create(btnVolDown, nullptr);
  lv_label_set_text_static(label, Symbols::volumDown);
  lv_obj_set_hidden(btnVolDown, true);

  btnVolUp = Styles::Button(lv_scr_act());
  btnVolUp->user_data = this;
  lv_obj_set_event_cb(btnVolUp, event_handler);
  lv_obj_set_size(btnVolUp, 76, 76);
  lv_obj_align(btnVolUp, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_obj_add_style(btnVolUp, LV_STATE_DEFAULT, &btn_style);
  label = lv_label_create(btnVolUp, nullptr);
  lv_label_set_text_static(label, Symbols::volumUp);
  lv_obj_set_hidden(btnVolUp, true);

  btnPrev = Styles::Button(lv_scr_act());
  btnPrev->user_data = this;
  lv_obj_set_event_cb(btnPrev, event_handler);
  lv_obj_set_size(btnPrev, 76, 76);
  lv_obj_align(btnPrev, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 3, 0);
  lv_obj_add_style(btnPrev, LV_STATE_DEFAULT, &btn_style);
  label = lv_label_create(btnPrev, nullptr);
  lv_label_set_text_static(label, Symbols::stepBackward);

  btnNext = Styles::Button(lv_scr_act());
  btnNext->user_data = this;
  lv_obj_set_event_cb(btnNext, event_handler);
  lv_obj_set_size(btnNext, 76, 76);
  lv_obj_align(btnNext, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_obj_add_style(btnNext, LV_STATE_DEFAULT, &btn_style);
  label = lv_label_create(btnNext, nullptr);
  lv_label_set_text_static(label, Symbols::stepForward);

  btnPlayPause = Styles::Button(lv_scr_act());
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, event_handler);
  lv_obj_set_size(btnPlayPause, 76, 76);
  lv_obj_align(btnPlayPause, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(btnPlayPause, LV_STATE_DEFAULT, &btn_style);
  label = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text_static(label, Symbols::play);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "BT Remote");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  pageIndicator.Create();
}

BTRemote::~BTRemote() {
  lv_style_reset(&btn_style);
  lv_obj_clean(lv_scr_act());
}

void BTRemote::OnObjectEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (obj == btnVolDown) {
    SendTap(hidService, Controllers::HidService::VolumeDown);
  } else if (obj == btnVolUp) {
    SendTap(hidService, Controllers::HidService::VolumeUp);
  } else if (obj == btnPrev) {
    SendTap(hidService, Controllers::HidService::PrevTrack);
  } else if (obj == btnPlayPause) {
    SendTap(hidService, Controllers::HidService::PlayPause);
  } else if (obj == btnNext) {
    SendTap(hidService, Controllers::HidService::NextTrack);
  }
}

bool BTRemote::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeUp:
      lv_obj_set_hidden(btnVolDown, false);
      lv_obj_set_hidden(btnVolUp, false);
      lv_obj_set_hidden(btnNext, true);
      lv_obj_set_hidden(btnPrev, true);
      pageIndicator.SetPageIndicatorPosition(1);
      return true;
    case TouchEvents::SwipeDown:
      if (lv_obj_get_hidden(btnNext)) {
        lv_obj_set_hidden(btnNext, false);
        lv_obj_set_hidden(btnPrev, false);
        lv_obj_set_hidden(btnVolDown, true);
        lv_obj_set_hidden(btnVolUp, true);
        pageIndicator.SetPageIndicatorPosition(0);
        return true;
      }
      return false;
    default:
      return false;
  }
}
