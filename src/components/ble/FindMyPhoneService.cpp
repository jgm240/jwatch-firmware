#include "components/ble/FindMyPhoneService.h"
#include "components/ble/NimbleController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t FindMyPhoneService::serviceUuid;
constexpr ble_uuid128_t FindMyPhoneService::triggerCharUuid;

namespace {
  int OnAccessCb(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, ble_gatt_access_ctxt* /*ctxt*/, void* /*arg*/) {
    // Notify-only characteristic - nothing for a central to read/write, but NimBLE still
    // requires an access_cb function pointer on every ble_gatt_chr_def.
    return 0;
  }
}

FindMyPhoneService::FindMyPhoneService(NimbleController& nimble)
  : nimble {nimble},
    characteristicDefinition {{.uuid = &triggerCharUuid.u, .access_cb = OnAccessCb, .flags = BLE_GATT_CHR_F_NOTIFY, .val_handle = &triggerHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &serviceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void FindMyPhoneService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

void FindMyPhoneService::SendTrigger() {
  if (!triggerNotificationEnabled) {
    return;
  }

  uint16_t connectionHandle = nimble.connHandle();
  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  triggerValue++;
  auto* om = ble_hs_mbuf_from_flat(&triggerValue, sizeof(triggerValue));
  ble_gattc_notify_custom(connectionHandle, triggerHandle, om);
}

void FindMyPhoneService::SubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == triggerHandle) {
    triggerNotificationEnabled = true;
  }
}

void FindMyPhoneService::UnsubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == triggerHandle) {
    triggerNotificationEnabled = false;
  }
}
