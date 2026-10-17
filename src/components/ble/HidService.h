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

      // Kindle Click (see KindleClick app): a keyboard report (PageUp/PageDown) and a second consumer report
      // (Power, Brightness+/-). These reports are only described in the report map while Kindle mode is on, so
      // the media-key remote keeps exactly the descriptor it always had.
      static constexpr uint8_t keyPageUp = 0x4B;
      static constexpr uint8_t keyPageDown = 0x4E;
      static constexpr uint8_t kindlePower = 1 << 0;
      static constexpr uint8_t kindleBrightnessUp = 1 << 1;
      static constexpr uint8_t kindleBrightnessDown = 1 << 2;

      void SendKeyboardReport(uint8_t keycode); // 0 releases
      void SendKindleConsumerReport(uint8_t bitmask);
      int OnKeyboardReportRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      int OnKindleReportRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);

      static void SetKindleMode(bool enabled) {
        kindleMode = enabled;
      }
      static bool KindleMode() {
        return kindleMode;
      }

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

      static inline bool kindleMode = false;

      struct ble_gatt_dsc_def reportDescriptors[2];
      struct ble_gatt_dsc_def keyboardDescriptors[2];
      struct ble_gatt_dsc_def kindleDescriptors[2];
      struct ble_gatt_chr_def characteristicDefinition[8];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t hidInformationHandle {0};
      uint16_t reportMapHandle {0};
      uint16_t reportHandle {0};
      uint16_t hidControlPointHandle {0};
      uint16_t protocolModeHandle {0};
      uint16_t keyboardHandle {0};
      uint16_t kindleHandle {0};

      uint8_t currentReport {0};
      uint8_t protocolMode {1};
      bool reportNotificationEnabled {false};
      bool keyboardNotificationEnabled {false};
      bool kindleNotificationEnabled {false};
      uint8_t keyboardReport[8] {0};
      uint8_t kindleReport {0};
    };
  }
}
