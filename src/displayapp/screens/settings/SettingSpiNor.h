#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "components/fs/FS.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // "Erase safe SPI-NOR area" - a direct-from-Settings equivalent of PasscodeLock's
      // "Forgot code? Erase watch" escape hatch, for anyone who wants to wipe the
      // filesystem (settings, BLE pairings, files) without going through that flow. Same
      // FS::Format() call, same "firmware untouched" guarantee - see its comment in
      // FS.h for exactly which flash region this is scoped to.
      class SettingSpiNor : public Screen {
      public:
        SettingSpiNor(DisplayApp* app, Controllers::FS& filesystem);
        ~SettingSpiNor() override;

        void OnEvent(lv_obj_t* target);

      private:
        DisplayApp* app;
        Controllers::FS& filesystem;

        lv_obj_t* infoLabel;
        lv_obj_t* eraseLabel;
        lv_obj_t* eraseBtn;
        lv_obj_t* cancelBtn;
        lv_obj_t* confirmBtn;

        bool confirming = false;

        void ShowMenu();
        void ShowConfirm();
      };
    }
  }
}
