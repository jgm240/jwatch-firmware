#include "components/ble/OpenHaystack.h"

using namespace Pinetime::Controllers;

// PLACEHOLDER - all zero. This key is broadcast in cleartext over BLE and uniquely
// identifies this specific watch to anyone who logs it, so it must never be a real key
// committed to a public repo (a previous, personally-generated key lived here before this
// was open-sourced). Generate your own EC P-224 keypair offline, keep the private half out
// of version control, and put only the public X-coordinate's 28 bytes here before enabling
// OpenHaystack - otherwise it advertises this all-zero placeholder, which is not usable for
// tracking.
const std::array<uint8_t, OpenHaystack::keySize> OpenHaystack::advertisementKey = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void OpenHaystack::BuildAdvertisingAddress(std::array<uint8_t, 6>& addr) {
  // NimBLE's ble_hs_id_set_rnd validates the address-type bits in addr[5], i.e. index 5 is
  // the most-significant byte - the reverse of the reference C firmware's addr[0] convention,
  // which was written against SDKs using the opposite byte order.
  for (size_t i = 0; i < 6; i++) {
    addr[5 - i] = advertisementKey[i];
  }
  addr[5] |= 0xC0; // Static random address: top two bits of the MSB must be 1,1.
}

void OpenHaystack::BuildAdvertisingPayload(std::array<uint8_t, advDataSize>& payload) {
  payload[0] = 0x1E; // AD structure length (following bytes: type + 29 data bytes)
  payload[1] = 0xFF; // AD type: Manufacturer Specific Data
  payload[2] = 0x4C; // Company ID (Apple), little-endian
  payload[3] = 0x00;
  payload[4] = 0x12; // Offline Finding type
  payload[5] = 0x19; // Offline Finding payload length (25 bytes follow)
  payload[6] = 0x00; // Status byte
  for (size_t i = 0; i < 22; i++) {
    payload[7 + i] = advertisementKey[6 + i];
  }
  payload[29] = advertisementKey[0] >> 6; // top 2 bits of the first key byte
  payload[30] = 0x00;                     // hint byte
}
