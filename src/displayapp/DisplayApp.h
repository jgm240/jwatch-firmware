#pragma once
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <memory>
#include <systemtask/Messages.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/TouchEvents.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "components/timer/Timer.h"
#include "components/stopwatch/StopWatchController.h"
#include "components/alarm/AlarmController.h"
#include "touchhandler/TouchHandler.h"

#include "displayapp/Messages.h"
#include "BootErrors.h"

#include "utility/StaticStack.h"
#include "displayapp/Controllers.h"

namespace Pinetime {

  namespace Drivers {
    class St7789;
    class Cst816S;
    class Watchdog;
  }

  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class TouchHandler;
    class SimpleWeatherService;
  }

  namespace System {
    class SystemTask;
  };

  namespace Applications {
    class DisplayApp {
    public:
      enum class States { Idle, Running, AOD };
      enum class FullRefreshDirections { None, Up, Down, Left, Right, LeftAnim, RightAnim };

      DisplayApp(Drivers::St7789& lcd,
                 const Drivers::Cst816S&,
                 const Controllers::Battery& batteryController,
                 const Controllers::Ble& bleController,
                 Controllers::DateTime& dateTimeController,
                 const Drivers::Watchdog& watchdog,
                 Pinetime::Controllers::NotificationManager& notificationManager,
                 Pinetime::Controllers::HeartRateController& heartRateController,
                 Controllers::Settings& settingsController,
                 Pinetime::Controllers::MotorController& motorController,
                 Pinetime::Controllers::MotionController& motionController,
                 Pinetime::Controllers::HealthController& healthController,
                 Pinetime::Controllers::StopWatchController& stopWatchController,
                 Pinetime::Controllers::AlarmController& alarmController,
                 Pinetime::Controllers::BrightnessController& brightnessController,
                 Pinetime::Controllers::TouchHandler& touchHandler,
                 Pinetime::Controllers::FS& filesystem,
                 Pinetime::Drivers::SpiNorFlash& spiNorFlash);
      void Start(System::BootErrors error);
      void PushMessage(Display::Messages msg);

      void StartApp(Apps app, DisplayApp::FullRefreshDirections direction);

      void SetFullRefresh(FullRefreshDirections direction);

      void Register(Pinetime::System::SystemTask* systemTask);
      void Register(Pinetime::Controllers::SimpleWeatherService* weatherService);
      void Register(Pinetime::Controllers::MusicService* musicService);
      void Register(Pinetime::Controllers::NavigationService* NavigationService);
      void Register(Pinetime::Controllers::HidService* hidService);

      // Called by PasscodeLock once the correct code is entered. Until then, GoToRunning
      // reloads PasscodeLock instead of leaving whatever screen was showing when it slept.
      void Unlock();

    private:
      Pinetime::Drivers::St7789& lcd;
      const Pinetime::Drivers::Cst816S& touchPanel;
      const Pinetime::Controllers::Battery& batteryController;
      const Pinetime::Controllers::Ble& bleController;
      Pinetime::Controllers::DateTime& dateTimeController;
      const Pinetime::Drivers::Watchdog& watchdog;
      Pinetime::System::SystemTask* systemTask = nullptr;
      Pinetime::Controllers::NotificationManager& notificationManager;
      Pinetime::Controllers::HeartRateController& heartRateController;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Controllers::MotionController& motionController;
      Pinetime::Controllers::HealthController& healthController;
      Pinetime::Controllers::StopWatchController& stopWatchController;
      Pinetime::Controllers::AlarmController& alarmController;
      Pinetime::Controllers::BrightnessController& brightnessController;
      Pinetime::Controllers::TouchHandler& touchHandler;
      Pinetime::Controllers::FS& filesystem;
      Pinetime::Drivers::SpiNorFlash& spiNorFlash;

      Pinetime::Controllers::FirmwareValidator validator;
      Pinetime::Components::LittleVgl lvgl;
      Pinetime::Controllers::Timer timer;

      AppControllers controllers;
      TaskHandle_t taskHandle;

      States state = States::Running;
      // Only meaningful while settingsController.GetPasscodeEnabled() is true - starts
      // true so a passcode-enabled watch boots straight into PasscodeLock.
      bool locked = true;
      QueueHandle_t msgQueue;

      static constexpr uint8_t queueSize = 10;
      static constexpr uint8_t itemSize = 1;

      std::unique_ptr<Screens::Screen> currentScreen;

      Apps currentApp = Apps::None;
      Apps returnToApp = Apps::None;
      FullRefreshDirections returnDirection = FullRefreshDirections::None;
      TouchEvents returnTouchEvent = TouchEvents::None;

      TouchEvents GetGesture();
      static void Process(void* instance);
      void Init();
      void Refresh();
      void LoadNewScreen(Apps app, DisplayApp::FullRefreshDirections direction);
      void LoadScreen(Apps app, DisplayApp::FullRefreshDirections direction);
      void PushMessageToSystemTask(Pinetime::System::Messages message);

      Apps nextApp = Apps::None;
      DisplayApp::FullRefreshDirections nextDirection;
      System::BootErrors bootError;
      void ApplyBrightness();

      // Warm-tint overlay on lv_layer_top(), shown/hidden based on Settings' Night Mode
      // toggle and bedtime hours - persists across screen changes since layer_top isn't
      // part of any individual screen's object tree.
      lv_obj_t* nightModeOverlay = nullptr;
      void ApplyNightMode();

      // UI Parallax: nudges the whole active screen object (lv_scr_act(), which every
      // screen's widgets are children of) opposite to the accelerometer's tilt, so the
      // entire UI shifts together rather than just the watchface's time label the way
      // the original per-watchface tilt effect worked. Reuses the same Settings > Tilt
      // Parallax strength setting. Suspended during the app's own custom scroll-transition
      // (see LittleVgl::GetFullRefresh()) since that already repositions pixels at the
      // flush level - stacking this on top would visually fight it.
      void ApplyUiParallax();

      // Water Lock: triple-click the button to ignore all touchscreen input (so stray
      // touches from water don't trigger swipes/taps); long-press the button to dismiss.
      // The badge lives on lv_layer_top() for the same reason as nightModeOverlay - it
      // needs to stay visible across screen changes without every screen knowing about it.
      bool waterLockActive = false;
      lv_obj_t* waterLockOverlay = nullptr;

      // Watchface gesture cycling: a long-tap on the clock face arms a short window
      // (see watchFaceGestureTimeout below) during which a horizontal swipe changes
      // watchface and a vertical swipe changes its accent color, instead of the normal
      // Clock-screen swipe navigation (Launcher/Notifications/QuickSettings). Armed one
      // long-tap at a time rather than combined into a single continuous gesture,
      // because the touch controller's own gesture engine reports LongPress and Slide
      // as separate, mutually exclusive gestures - it can't detect a held-then-dragged
      // touch as one event.
      bool watchFaceGestureArmed = false;
      TickType_t watchFaceGestureArmedTick = 0;
      static constexpr TickType_t watchFaceGestureTimeout = pdMS_TO_TICKS(5000);
      bool IsWatchFaceGestureArmed() const;
      void ArmWatchFaceGesture();
      void CycleWatchFace(bool forward);
      void CycleWatchFaceColor(bool forward);

      static constexpr size_t returnAppStackSize = 10;
      Utility::StaticStack<Apps, returnAppStackSize> returnAppStack;
      Utility::StaticStack<FullRefreshDirections, returnAppStackSize> appStackDirections;

      bool isDimmed = false;

      TickType_t CalculateSleepTime();
      TickType_t alwaysOnFrameCount;
      TickType_t alwaysOnStartTime;
      // If this is to be changed, make sure the actual always on refresh rate is changed
      // by configuring the LCD refresh timings
      static constexpr uint32_t alwaysOnRefreshPeriod = 500;
    };
  }
}
