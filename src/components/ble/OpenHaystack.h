#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    // Builds the BLE advertisement the Find My network (and OpenHaystack, its open-source
    // reimplementation) uses to let any nearby iPhone report this device's location without
    // needing to pair or connect to it. Only the 28-byte public "advertisement key" (the
    // X-coordinate of an EC P-224 keypair generated offline) lives in firmware - the matching
    // private key never does, since it's only needed later to decrypt the location reports
    // Apple's servers collect, not to broadcast them.
    class OpenHaystack {
    public:
      static constexpr size_t keySize = 28;
      static constexpr size_t advDataSize = 31;

      static const std::array<uint8_t, keySize> advertisementKey;

      // Fills a NimBLE-order (index 5 = most-significant byte) random static address derived
      // from the key, per the reverse-engineered Find My beacon protocol.
      static void BuildAdvertisingAddress(std::array<uint8_t, 6>& addr);

      // Fills the exact 31-byte legacy-advertisement payload (one manufacturer-specific AD
      // structure, no room left for a Flags AD - matches real Find My beacons).
      static void BuildAdvertisingPayload(std::array<uint8_t, advDataSize>& payload);
    };
  }
}
