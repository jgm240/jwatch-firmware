#include "components/ble/HidService.h"
#include "components/ble/NimbleController.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t HidService::hidServiceUuid;
constexpr ble_uuid16_t HidService::hidInformationUuid;
constexpr ble_uuid16_t HidService::reportMapUuid;
constexpr ble_uuid16_t HidService::reportUuid;
constexpr ble_uuid16_t HidService::hidControlPointUuid;
constexpr ble_uuid16_t HidService::protocolModeUuid;
constexpr ble_uuid16_t HidService::reportReferenceUuid;

namespace {
  // Minimal Consumer Control report descriptor: a single 1-byte input report,
  // 6 bits (Play/Pause, Next, Prev, Vol+, Vol-, Mute) + 2 bits of padding.
  // This byte sequence is the widely-reused, known-good minimal HOGP consumer
  // control descriptor -- do not modify without testing against real hardware.
  constexpr uint8_t reportMapData[] = {
    0x05, 0x0C, // Usage Page (Consumer)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x01, //   Report ID (1)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x06, //   Report Count (6)
    0x09, 0xCD, //   Usage (Play/Pause)
    0x09, 0xB5, //   Usage (Scan Next Track)
    0x09, 0xB6, //   Usage (Scan Previous Track)
    0x09, 0xE9, //   Usage (Volume Increment)
    0x09, 0xEA, //   Usage (Volume Decrement)
    0x09, 0xE2, //   Usage (Mute)
    0x81, 0x02, //   Input (Data,Var,Abs)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x02, //   Report Size (2)
    0x81, 0x03, //   Input (Const,Var,Abs)
    0xC0,       // End Collection
  };

  int OnHidInformationRequestedCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnHidInformationRequested(attr_handle, ctxt);
  }

  int OnReportMapRequestedCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnReportMapRequested(attr_handle, ctxt);
  }

  int OnReportRequestedCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnReportRequested(attr_handle, ctxt);
  }

  int OnReportReferenceRequestedCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnReportReferenceRequested(attr_handle, ctxt);
  }

  int OnControlPointWriteCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnControlPointWrite(attr_handle, ctxt);
  }

  int OnProtocolModeAccessCb(uint16_t /*conn_handle*/, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<HidService*>(arg)->OnProtocolModeAccess(attr_handle, ctxt);
  }
}

HidService::HidService(NimbleController& nimble)
  : nimble {nimble},
    reportDescriptors {{.uuid = &reportReferenceUuid.u,
                        .att_flags = BLE_ATT_F_READ,
                        .min_key_size = 0,
                        .access_cb = OnReportReferenceRequestedCb,
                        .arg = this},
                       {0}},
    characteristicDefinition {
      {.uuid = &hidInformationUuid.u,
       .access_cb = OnHidInformationRequestedCb,
       .arg = this,
       .flags = BLE_GATT_CHR_F_READ,
       .val_handle = &hidInformationHandle},
      {.uuid = &reportMapUuid.u, .access_cb = OnReportMapRequestedCb, .arg = this, .flags = BLE_GATT_CHR_F_READ, .val_handle = &reportMapHandle},
      {.uuid = &reportUuid.u,
       .access_cb = OnReportRequestedCb,
       .arg = this,
       .descriptors = reportDescriptors,
       .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
       .val_handle = &reportHandle},
      {.uuid = &hidControlPointUuid.u,
       .access_cb = OnControlPointWriteCb,
       .arg = this,
       .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
       .val_handle = &hidControlPointHandle},
      {.uuid = &protocolModeUuid.u,
       .access_cb = OnProtocolModeAccessCb,
       .arg = this,
       .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
       .val_handle = &protocolModeHandle},
      {0},
    },
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &hidServiceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void HidService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int HidService::OnHidInformationRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle != hidInformationHandle) {
    return 0;
  }
  // bcdHID=0x0111 (little-endian), bCountryCode=0x00, Flags=0x02 (RemoteWake)
  uint8_t buffer[4] = {0x11, 0x01, 0x00, 0x02};
  int res = os_mbuf_append(context->om, buffer, sizeof(buffer));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int HidService::OnReportMapRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle != reportMapHandle) {
    return 0;
  }
  int res = os_mbuf_append(context->om, reportMapData, sizeof(reportMapData));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int HidService::OnReportRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle != reportHandle) {
    return 0;
  }
  int res = os_mbuf_append(context->om, &currentReport, sizeof(currentReport));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int HidService::OnReportReferenceRequested(uint16_t /*attributeHandle*/, ble_gatt_access_ctxt* context) {
  // Report ID = 1, Report Type = 1 (Input)
  uint8_t buffer[2] = {1, 1};
  int res = os_mbuf_append(context->om, buffer, sizeof(buffer));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int HidService::OnControlPointWrite(uint16_t /*attributeHandle*/, ble_gatt_access_ctxt* /*context*/) {
  return 0;
}

int HidService::OnProtocolModeAccess(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle != protocolModeHandle) {
    return 0;
  }
  if (context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    os_mbuf_copydata(context->om, 0, 1, &protocolMode);
    return 0;
  }
  int res = os_mbuf_append(context->om, &protocolMode, sizeof(protocolMode));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

void HidService::SendConsumerReport(uint8_t bitmask) {
  if (!reportNotificationEnabled) {
    return;
  }

  uint16_t connectionHandle = nimble.connHandle();
  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  currentReport = bitmask;
  auto* om = ble_hs_mbuf_from_flat(&currentReport, sizeof(currentReport));
  ble_gattc_notify_custom(connectionHandle, reportHandle, om);
}

void HidService::SubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == reportHandle) {
    reportNotificationEnabled = true;
  }
}

void HidService::UnsubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == reportHandle) {
    reportNotificationEnabled = false;
  }
}
