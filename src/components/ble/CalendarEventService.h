#pragma once
#include <array>
#include <cstdint>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

// A custom service (no standard BLE-SIG service exists for this) - a phone app writes the
// user's upcoming calendar events here, one event per write, so the watch can show an agenda
// screen without needing its own calendar/account integration.

// 9214f6b1-01ea-4c6b-8f1e-6a3a3e5f6c10
#define CALENDAR_EVENT_SERVICE_UUID_BASE                                                                                                   \
  { 0x10, 0x6c, 0x5f, 0x3e, 0x3a, 0x6a, 0x1e, 0x8f, 0x6b, 0x4c, 0xea, 0x01, 0xb1, 0xf6, 0x14, 0x92 }

// 9214f6b2-01ea-4c6b-8f1e-6a3a3e5f6c10
#define CALENDAR_EVENT_CHAR_UUID_BASE                                                                                                      \
  { 0x10, 0x6c, 0x5f, 0x3e, 0x3a, 0x6a, 0x1e, 0x8f, 0x6b, 0x4c, 0xea, 0x01, 0xb2, 0xf6, 0x14, 0x92 }

namespace Pinetime {
  namespace Controllers {

    class CalendarEventService {
    public:
      static constexpr uint8_t MaxEvents = 5;
      static constexpr uint8_t TitleMaxLength = 40;

      struct Event {
        uint64_t start = 0;
        uint64_t end = 0;
        std::array<char, TitleMaxLength + 1> title {};
      };

      CalendarEventService();

      void Init();

      int OnAccess(ble_gatt_access_ctxt* ctxt);

      uint8_t NbEvents() const {
        return eventCount;
      }

      const Event& GetEvent(uint8_t idx) const {
        return events[idx];
      }

    private:
      static int OnAccessCb(uint16_t connHandle, uint16_t attrHandle, ble_gatt_access_ctxt* ctxt, void* arg);

      static constexpr ble_uuid128_t serviceUuid {.u {.type = BLE_UUID_TYPE_128}, .value = CALENDAR_EVENT_SERVICE_UUID_BASE};
      static constexpr ble_uuid128_t eventCharUuid {.u {.type = BLE_UUID_TYPE_128}, .value = CALENDAR_EVENT_CHAR_UUID_BASE};

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];
      uint16_t eventHandle {0};

      std::array<Event, MaxEvents> events;
      uint8_t eventCount = 0;
    };
  }
}
