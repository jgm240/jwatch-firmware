#include "components/ble/CalendarEventService.h"
#include <algorithm>
#include <cstring>

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t CalendarEventService::serviceUuid;
constexpr ble_uuid128_t CalendarEventService::eventCharUuid;
constexpr ble_uuid128_t CalendarEventService::phoneBatteryCharUuid;
constexpr ble_uuid128_t CalendarEventService::commandCharUuid;

namespace {
  uint64_t ToUInt64(const uint8_t* data) {
    return data[0] + (static_cast<uint64_t>(data[1]) << 8) + (static_cast<uint64_t>(data[2]) << 16) +
           (static_cast<uint64_t>(data[3]) << 24) + (static_cast<uint64_t>(data[4]) << 32) + (static_cast<uint64_t>(data[5]) << 40) +
           (static_cast<uint64_t>(data[6]) << 48) + (static_cast<uint64_t>(data[7]) << 56);
  }
}

CalendarEventService::CalendarEventService()
  : characteristicDefinition {{.uuid = &eventCharUuid.u,
                                .access_cb = OnAccessCb,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE,
                                .val_handle = &eventHandle},
                              {.uuid = &phoneBatteryCharUuid.u,
                                .access_cb = OnAccessCb,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE,
                                .val_handle = &phoneBatteryHandle},
                              {.uuid = &commandCharUuid.u,
                                .access_cb = OnAccessCb,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE,
                                .val_handle = &commandHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &serviceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void CalendarEventService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}

int CalendarEventService::OnAccessCb(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<CalendarEventService*>(arg)->OnAccess(ctxt);
}

int CalendarEventService::OnAccess(ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
    return 0;
  }

  const size_t receivedSize = OS_MBUF_PKTLEN(ctxt->om);

  if (ble_uuid_cmp(ctxt->chr->uuid, &phoneBatteryCharUuid.u) == 0) {
    if (receivedSize >= 2) {
      uint8_t data[2];
      os_mbuf_copydata(ctxt->om, 0, 2, data);
      phoneBatteryPercent = std::min<uint8_t>(data[0], 100);
      phoneCharging = data[1] != 0;
    }
    return 0;
  }

  if (ble_uuid_cmp(ctxt->chr->uuid, &commandCharUuid.u) == 0) {
    if (receivedSize >= 1) {
      os_mbuf_copydata(ctxt->om, 0, 1, &pendingCommand);
    }
    return 0;
  }

  // Event slot wire format:
  // [0] index, [1] total event count for this sync, [2..9] start epoch (u64 LE),
  // [10..17] end epoch (u64 LE), [18] title length, [19..] title bytes.
  static constexpr size_t headerSize = 19;
  if (receivedSize < headerSize) {
    return 0;
  }

  uint8_t buffer[headerSize + TitleMaxLength];
  size_t copySize = std::min(receivedSize, sizeof(buffer));
  os_mbuf_copydata(ctxt->om, 0, copySize, buffer);

  uint8_t index = buffer[0];
  if (index >= MaxEvents) {
    return 0;
  }

  if (index == 0) {
    eventCount = std::min(buffer[1], MaxEvents);
  }

  Event& event = events[index];
  event.start = ToUInt64(&buffer[2]);
  event.end = ToUInt64(&buffer[10]);

  uint8_t titleLen = std::min(buffer[18], static_cast<uint8_t>(std::min(copySize - headerSize, static_cast<size_t>(TitleMaxLength))));
  std::memcpy(event.title.data(), &buffer[headerSize], titleLen);
  event.title[titleLen] = '\0';

  return 0;
}
