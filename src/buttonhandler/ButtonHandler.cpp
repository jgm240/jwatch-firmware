#include "buttonhandler/ButtonHandler.h"

using namespace Pinetime::Controllers;

void ButtonTimerCallback(TimerHandle_t xTimer) {
  auto* sysTask = static_cast<Pinetime::System::SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->PushMessage(Pinetime::System::Messages::HandleButtonTimerEvent);
}

void ButtonHandler::Init(Pinetime::System::SystemTask* systemTask) {
  buttonTimer = xTimerCreate("buttonTimer", pdMS_TO_TICKS(200), pdFALSE, systemTask, ButtonTimerCallback);
}

ButtonActions ButtonHandler::HandleEvent(Events event) {
  static constexpr TickType_t doubleClickTime = pdMS_TO_TICKS(200);
  static constexpr TickType_t longPressTime = pdMS_TO_TICKS(400);
  static constexpr TickType_t longerPressTime = pdMS_TO_TICKS(2000);

  if (event == Events::Press) {
    buttonPressed = true;
  } else if (event == Events::Release) {
    releaseTime = xTaskGetTickCount();
    buttonPressed = false;
  }

  switch (state) {
    case States::Idle:
      if (event == Events::Press) {
        clickCount = 1;
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        state = States::Pressed;
      }
      break;
    case States::Pressed:
      if (event == Events::Press) {
        if (xTaskGetTickCount() - releaseTime < doubleClickTime) {
          clickCount++;
          if (clickCount >= 3) {
            xTimerStop(buttonTimer, 0);
            state = States::Idle;
            clickCount = 0;
            return ButtonActions::TripleClick;
          }
          // clickCount == 2: stay in Pressed and keep waiting for a possible third
          // press; the Release branch below restarts the click window as usual.
        }
      } else if (event == Events::Release) {
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
      } else if (event == Events::Timer) {
        if (buttonPressed) {
          xTimerChangePeriod(buttonTimer, longPressTime - doubleClickTime, 0);
          xTimerStart(buttonTimer, 0);
          state = States::Holding;
        } else {
          state = States::Idle;
          bool wasDoubleClick = clickCount >= 2;
          clickCount = 0;
          return wasDoubleClick ? ButtonActions::DoubleClick : ButtonActions::Click;
        }
      }
      break;
    case States::Holding:
      if (event == Events::Release) {
        xTimerStop(buttonTimer, 0);
        state = States::Idle;
        clickCount = 0;
        return ButtonActions::Click;
      } else if (event == Events::Timer) {
        xTimerChangePeriod(buttonTimer, longerPressTime - longPressTime - doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        state = States::LongHeld;
        clickCount = 0;
        return ButtonActions::LongPress;
      }
      break;
    case States::LongHeld:
      if (event == Events::Release) {
        xTimerStop(buttonTimer, 0);
        state = States::Idle;
      } else if (event == Events::Timer) {
        state = States::Idle;
        return ButtonActions::LongerPress;
      }
      break;
  }
  return ButtonActions::None;
}
