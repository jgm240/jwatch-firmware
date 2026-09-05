#pragma once
#include <cstdint>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

// A custom service (no standard BLE-SIG service exists for this) - a phone app subscribes
// to the trigger characteristic and reacts however it likes (local notification, sound,
// etc.) when the watch calls SendTrigger(). Modeled directly on AlertNotificationService's
// notificationEventUuid + ble_gattc_notify_custom pattern.

// a4fb5869-b23f-428d-9d52-d602d8c0f163
#define FIND_MY_PHONE_SERVICE_UUID_BASE                                                                                                      \
  { 0x63, 0xf1, 0xc0, 0xd8, 0x02, 0xd6, 0x52, 0x9d, 0x8d, 0x42, 0x3f, 0xb2, 0x69, 0x58, 0xfb, 0xa4 }

// 03e09113-9668-43bb-95a5-3e08e4ab6859
#define FIND_MY_PHONE_TRIGGER_CHAR_UUID_BASE                                                                                                 \
  { 0x59, 0x68, 0xab, 0xe4, 0x08, 0x3e, 0xa5, 0x95, 0xbb, 0x43, 0x68, 0x96, 0x13, 0x91, 0xe0, 0x03 }

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class FindMyPhoneService {
    public:
      explicit FindMyPhoneService(NimbleController& nimble);
      void Init();

      // Content doesn't matter - this is a pure signal, not a data payload.
      void SendTrigger();

      void SubscribeNotification(uint16_t attributeHandle);
      void UnsubscribeNotification(uint16_t attributeHandle);

    private:
      static constexpr ble_uuid128_t serviceUuid {.u {.type = BLE_UUID_TYPE_128}, .value = FIND_MY_PHONE_SERVICE_UUID_BASE};
      static constexpr ble_uuid128_t triggerCharUuid {.u {.type = BLE_UUID_TYPE_128}, .value = FIND_MY_PHONE_TRIGGER_CHAR_UUID_BASE};

      NimbleController& nimble;

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t triggerHandle {0};
      bool triggerNotificationEnabled {false};
      uint8_t triggerValue {0};
    };
  }
}
