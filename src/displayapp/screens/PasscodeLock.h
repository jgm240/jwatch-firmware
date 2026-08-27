#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Controllers {
    class MotorController;
    class Settings;
    class FS;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      // Full-screen kiosk lock shown at boot and on every wake while
      // Settings::GetPasscodeEnabled() is true (see DisplayApp's locked flag). A 4-digit
      // PIN is stored in plaintext in Settings - a casual screen-lock deterrent, not a
      // defense against someone pulling the external flash chip and reading it directly.
      // "Forgot code?" is a deliberate, always-reachable escape hatch: without it, a
      // forgotten PIN would brick the watch forever, since there is no companion account
      // or remote-unlock path for this firmware. It erases the littlefs filesystem
      // (FS::Format) and reboots - see FS::Format's own comment for exactly what that
      // does and does not touch.
      class PasscodeLock : public Screen {
      public:
        PasscodeLock(DisplayApp* app,
                     Controllers::MotorController& motorController,
                     Controllers::Settings& settingsController,
                     Controllers::FS& filesystem);
        ~PasscodeLock() override;

        void OnKeypadEvent(lv_obj_t* target, lv_event_t event);
        void OnLinkEvent(lv_obj_t* target);

        // Swallow every gesture/button press unconditionally - DisplayApp's default
        // unhandled-touch/button behavior (swipe-to-return, long-press-to-Clock, etc.)
        // would otherwise bypass the lock entirely. See DisplayApp.cpp's own guards on
        // ButtonLongPressed/ButtonLongerPressed/ButtonDoubleClicked/NewNotification for
        // the other bypass paths this screen alone can't intercept.
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        enum class ViewState : uint8_t { Entry, ConfirmErase };

        DisplayApp* app;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;
        Controllers::FS& filesystem;

        ViewState viewState = ViewState::Entry;

        std::array<uint8_t, 4> enteredDigits {};
        uint8_t enteredCount = 0;

        lv_obj_t* title;
        std::array<lv_obj_t*, 4> dots {};
        lv_obj_t* keypad;
        lv_obj_t* forgotLink;

        lv_obj_t* eraseLabel;
        lv_obj_t* eraseCancelBtn;
        lv_obj_t* eraseConfirmBtn;

        void UpdateDots();
        void ResetEntry();
        void CheckPasscode();
        void ShowConfirmErase();
        void ShowEntry();
      };
    }
  }
}
