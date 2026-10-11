#pragma once
#include <cstdint>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

// Custom read-only service: a small summary of last night's sleep that the phone app writes
// into Apple Health. Payload (6 bytes): [0] version (1), [1] light half-hours, [2] deep
// half-hours, [3] still half-hours, [4] bedtime end hour, [5] stand hours today.

// 4d8a2f10-6b1e-4c3a-9a57-2f1d8c6e0b11
#define HEALTH_SYNC_SERVICE_UUID_BASE \
  { 0x11, 0x0b, 0x6e, 0x8c, 0x1d, 0x2f, 0x57, 0x9a, 0x3a, 0x4c, 0x1e, 0x6b, 0x10, 0x2f, 0x8a, 0x4d }

// 4d8a2f11-6b1e-4c3a-9a57-2f1d8c6e0b11
#define HEALTH_SYNC_CHAR_UUID_BASE \
  { 0x11, 0x0b, 0x6e, 0x8c, 0x1d, 0x2f, 0x57, 0x9a, 0x3a, 0x4c, 0x1e, 0x6b, 0x11, 0x2f, 0x8a, 0x4d }

namespace Pinetime {
  namespace Controllers {
    class HealthController;

    class HealthSyncService {
    public:
      explicit HealthSyncService(HealthController& healthController);
      void Init();

    private:
      static int OnAccessCb(uint16_t connHandle, uint16_t attrHandle, ble_gatt_access_ctxt* ctxt, void* arg);
      int OnAccess(ble_gatt_access_ctxt* ctxt);

      static constexpr ble_uuid128_t serviceUuid {.u {.type = BLE_UUID_TYPE_128}, .value = HEALTH_SYNC_SERVICE_UUID_BASE};
      static constexpr ble_uuid128_t summaryCharUuid {.u {.type = BLE_UUID_TYPE_128}, .value = HEALTH_SYNC_CHAR_UUID_BASE};

      HealthController& healthController;
      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];
      uint16_t summaryHandle {0};
    };
  }
}
