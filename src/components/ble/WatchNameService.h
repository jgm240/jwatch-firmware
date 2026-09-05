#pragma once
#include <cstdint>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

// A custom service (no standard BLE-SIG service exists for this) - a phone app writes the
// desired Bluetooth device name here. The standard GAP Device Name characteristic (0x2A00)
// exists too, but its write path is compiled out in this build's NimBLE config, and even
// enabled it wouldn't give us a hook to persist the new name or know when to re-advertise.

// 94f1fed1-1b8e-4cac-a3de-4a15c12ef036
#define WATCH_NAME_SERVICE_UUID_BASE                                                                                                        \
  { 0x36, 0xf0, 0x2e, 0xc1, 0x15, 0x4a, 0xde, 0xa3, 0xac, 0x4c, 0x8e, 0x1b, 0xd1, 0xfe, 0xf1, 0x94 }

// 36680df1-1e57-4dde-a2bd-b1e65eb24552
#define WATCH_NAME_CHAR_UUID_BASE                                                                                                           \
  { 0x52, 0x45, 0xb2, 0x5e, 0xe6, 0xb1, 0xbd, 0xa2, 0xde, 0x4d, 0x57, 0x1e, 0xf1, 0x0d, 0x68, 0x36 }

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class WatchNameService {
    public:
      explicit WatchNameService(NimbleController& nimble);
      void Init();

    private:
      static int OnAccessCb(uint16_t connHandle, uint16_t attrHandle, ble_gatt_access_ctxt* ctxt, void* arg);
      int OnAccess(ble_gatt_access_ctxt* ctxt);

      static constexpr ble_uuid128_t serviceUuid {.u {.type = BLE_UUID_TYPE_128}, .value = WATCH_NAME_SERVICE_UUID_BASE};
      static constexpr ble_uuid128_t nameCharUuid {.u {.type = BLE_UUID_TYPE_128}, .value = WATCH_NAME_CHAR_UUID_BASE};

      NimbleController& nimble;
      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];
      uint16_t nameHandle {0};
    };
  }
}
