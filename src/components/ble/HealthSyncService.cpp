#include "components/ble/HealthSyncService.h"
#include "nrf_assert.h"
#include "components/health/HealthController.h"
#include <algorithm>

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t HealthSyncService::serviceUuid;
constexpr ble_uuid128_t HealthSyncService::summaryCharUuid;

HealthSyncService::HealthSyncService(HealthController& healthController)
  : healthController {healthController},
    characteristicDefinition {{.uuid = &summaryCharUuid.u,
                                .access_cb = OnAccessCb,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                                .val_handle = &summaryHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &serviceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void HealthSyncService::Init() {
  int res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);
  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int HealthSyncService::OnAccessCb(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<HealthSyncService*>(arg)->OnAccess(ctxt);
}

int HealthSyncService::OnAccess(ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
    return 0;
  }
  auto clamp = [](uint32_t v) {
    return static_cast<uint8_t>(std::min<uint32_t>(v, 255));
  };
  const uint8_t payload[6] = {1,
                              clamp(healthController.GetLastNightLightHalfHours()),
                              clamp(healthController.GetLastNightDeepHalfHours()),
                              clamp(healthController.GetLastNightStillHalfHours()),
                              healthController.GetBedtimeEndHour(),
                              clamp(healthController.GetStandHoursToday())};
  int res = os_mbuf_append(ctxt->om, payload, sizeof(payload));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}
