#include "components/ble/WatchNameService.h"
#include "components/ble/NimbleController.h"
#include "components/settings/Settings.h"
#include <algorithm>
#include <cstring>

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t WatchNameService::serviceUuid;
constexpr ble_uuid128_t WatchNameService::nameCharUuid;

WatchNameService::WatchNameService(NimbleController& nimble)
  : nimble {nimble},
    characteristicDefinition {{.uuid = &nameCharUuid.u,
                                .access_cb = OnAccessCb,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE,
                                .val_handle = &nameHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &serviceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void WatchNameService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int WatchNameService::OnAccessCb(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<WatchNameService*>(arg)->OnAccess(ctxt);
}

int WatchNameService::OnAccess(ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
    return 0;
  }

  size_t receivedSize = OS_MBUF_PKTLEN(ctxt->om);
  size_t copySize = std::min(receivedSize, Settings::watchNameMaxLength);

  char name[Settings::watchNameMaxLength + 1];
  os_mbuf_copydata(ctxt->om, 0, copySize, name);
  name[copySize] = '\0';

  if (copySize == 0) {
    return 0;
  }

  nimble.SetDeviceName(name);
  return 0;
}
