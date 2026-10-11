#pragma once
#include <cstdint>

namespace Pinetime {
  namespace Applications {
    namespace Display {
      enum class Messages : uint8_t {
        GoToSleep,
        GoToAOD,
        GoToRunning,
        UpdateBleConnection,
        TouchEvent,
        ButtonPushed,
        ButtonLongPressed,
        ButtonLongerPressed,
        ButtonDoubleClicked,
        ButtonTripleClicked,
        NewNotification,
        TimerDone,
        BleFirmwareUpdateStarted,
        // Resets the screen timeout timer when awake
        // Does nothing when asleep
        NotifyDeviceActivity,
        ShowPairingKey,
        AlarmTriggered,
        Chime,
        BleRadioEnableToggle,
        GoToNightstand,
        UpdateNightMode,
        RingsClosed,
        HapticNavLeft,
        HapticNavRight,
        HapticNavArrive,
        StandNudge,
        ShakeSilence,
        FindWatch,
      };
    }
  }
}
