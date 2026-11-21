#pragma once
#include <cstdint>
#include <cstring>

// JWAF ("JWatch App File"): a small declarative app a phone can install onto the watch's
// external SPI-NOR filesystem over BLE - via the existing FSService WRITE command, writing
// to e.g. "/apps/whatever.jwaf" - without a firmware rebuild/flash. There's no QSPI/XIP on
// this MCU and only 64 KB of RAM, so a JWAF can't carry executable code; it's read and
// rendered by JwafApp (see displayapp/screens/JwafApp.h) as a title plus a handful of text
// lines, the same shape as a static info card.
//
// Wire layout (little-endian, fields not null-terminated unless noted):
//   [0..28]  magic, exactly kJwafMagic (29 bytes) - any file not starting with this is
//            rejected outright, whether it's the wrong file type or just corrupted.
//   [29]     formatVersion - must equal kJwafFormatVersion for now.
//   [30]     titleLength (0..TitleMaxLength)
//   [31]     lineCount (0..MaxLines)
//   [32..]   titleLength bytes of title text (ASCII), then for each line:
//              1 byte lineLength (0..LineMaxLength) + that many bytes of ASCII text.
namespace Pinetime {
  namespace Controllers {
    namespace JwafFormat {
      inline constexpr char kJwafMagic[] = "JWAF-PINETIME-APPFILE-CHKTXT";
      inline constexpr size_t kMagicLength = sizeof(kJwafMagic) - 1; // 29, excludes the implicit '\0'
      inline constexpr uint8_t kFormatVersion = 1;

      inline constexpr uint8_t TitleMaxLength = 24;
      inline constexpr uint8_t LineMaxLength = 40;
      inline constexpr uint8_t MaxLines = 6;

      // Smallest a well-formed file can be: magic + version + titleLength + lineCount, all zero-content.
      inline constexpr size_t kHeaderSize = kMagicLength + 1 + 1 + 1;

      inline bool HasValidMagic(const uint8_t* data, size_t size) {
        return size >= kMagicLength && std::memcmp(data, kJwafMagic, kMagicLength) == 0;
      }
    }
  }
}
