#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class HidService {
    public:
      // Consumer Control usage bits (see the Report Map descriptor for the exact layout)
      enum ConsumerControl : uint8_t {
        PlayPause = 1 << 0,
        NextTrack = 1 << 1,
        PrevTrack = 1 << 2,
        VolumeUp = 1 << 3,
        VolumeDown = 1 << 4,
        Mute = 1 << 5,
      };

      explicit HidService(NimbleController& nimble);
      void Init();

      int OnHidInformationRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnReportMapRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnReportRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnReportReferenceRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnControlPointWrite(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnProtocolModeAccess(uint16_t attributeHandle, ble_gatt_access_ctxt* context);

      void SendConsumerReport(uint8_t bitmask);

      void SubscribeNotification(uint16_t attributeHandle);
      void UnsubscribeNotification(uint16_t attributeHandle);

      static constexpr uint16_t hidServiceId {0x1812};
      static constexpr ble_uuid16_t hidServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = hidServiceId};

    private:
      NimbleController& nimble;

      static constexpr uint16_t hidInformationId {0x2A4A};
      static constexpr uint16_t reportMapId {0x2A4B};
      static constexpr uint16_t reportId {0x2A4D};
      static constexpr uint16_t hidControlPointId {0x2A4C};
      static constexpr uint16_t protocolModeId {0x2A4E};
      static constexpr uint16_t reportReferenceId {0x2908};

      static constexpr ble_uuid16_t hidInformationUuid {.u {.type = BLE_UUID_TYPE_16}, .value = hidInformationId};
      static constexpr ble_uuid16_t reportMapUuid {.u {.type = BLE_UUID_TYPE_16}, .value = reportMapId};
      static constexpr ble_uuid16_t reportUuid {.u {.type = BLE_UUID_TYPE_16}, .value = reportId};
      static constexpr ble_uuid16_t hidControlPointUuid {.u {.type = BLE_UUID_TYPE_16}, .value = hidControlPointId};
      static constexpr ble_uuid16_t protocolModeUuid {.u {.type = BLE_UUID_TYPE_16}, .value = protocolModeId};
      static constexpr ble_uuid16_t reportReferenceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = reportReferenceId};

      struct ble_gatt_dsc_def reportDescriptors[2];
      struct ble_gatt_chr_def characteristicDefinition[6];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t hidInformationHandle {0};
      uint16_t reportMapHandle {0};
      uint16_t reportHandle {0};
      uint16_t hidControlPointHandle {0};
      uint16_t protocolModeHandle {0};

      uint8_t currentReport {0};
      uint8_t protocolMode {1};
      bool reportNotificationEnabled {false};
    };
  }
}
