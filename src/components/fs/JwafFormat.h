#pragma once
#include <cstdint>
#include <cstring>

// JWAF ("JWatch App File"): installed onto the watch's external SPI-NOR filesystem over BLE -
// via the existing FSService WRITE command, writing to e.g. "/apps/whatever.jwaf" - without a
// firmware rebuild/flash. Two kinds, picked by the version byte right after the magic:
//
//  - kFormatVersionText (1): a title plus a handful of text lines, rendered as-is by JwafApp.
//    No code, so it's as safe as any other data file - a malformed one just fails to parse.
//
//  - kFormatVersionCode (2): raw Thumb machine code, copied into a fixed 5 KB RAM slot and
//    run from there (see components/sandbox/AppSandbox.h). The nRF52832 has no QSPI/XIP, so
//    code can't execute in place on the external flash - it has to be in RAM - but Cortex-M
//    has no restriction against executing from RAM itself (confirmed: this firmware leaves
//    its MPU library disabled). AppSandbox cages that RAM slot with the chip's real hardware
//    MPU so a broken blob faults instead of corrupting the rest of the watch; see that header
//    for exactly what is and isn't contained.
//
// Wire layout (little-endian, fields not null-terminated unless noted):
//   [0..27]  magic, exactly kJwafMagic (28 bytes) - any file not starting with this is
//            rejected outright, whether it's the wrong file type or just corrupted.
//   [28]     formatVersion - kFormatVersionText or kFormatVersionCode.
//   if formatVersion == kFormatVersionText:
//     [29]     titleLength (0..TitleMaxLength)
//     [30]     lineCount (0..MaxLines)
//     [31..]   titleLength bytes of title text (ASCII), then for each line:
//                1 byte lineLength (0..LineMaxLength) + that many bytes of ASCII text.
//   if formatVersion == kFormatVersionCode:
//     [29..30] codeSize, uint16 LE (0..AppSandbox::CodeAreaSize)
//     [31..]   codeSize bytes of raw Thumb code/data, linked to run at AppSandbox's fixed
//              slot address (see AppSandbox.h) - built by a separate toolchain, not by this
//              firmware.
namespace Pinetime {
  namespace Controllers {
    namespace JwafFormat {
      inline constexpr char kJwafMagic[] = "JWAF-PINETIME-APPFILE-CHKTXT";
      inline constexpr size_t kMagicLength = sizeof(kJwafMagic) - 1; // 28, excludes the implicit '\0'
      inline constexpr uint8_t kFormatVersionText = 1;
      inline constexpr uint8_t kFormatVersionCode = 2;

      inline constexpr uint8_t TitleMaxLength = 24;
      inline constexpr uint8_t LineMaxLength = 40;
      inline constexpr uint8_t MaxLines = 6;

      // Smallest a well-formed file can be: magic + version + 2 more header bytes, no content.
      inline constexpr size_t kHeaderSize = kMagicLength + 1 + 1 + 1;

      inline bool HasValidMagic(const uint8_t* data, size_t size) {
        return size >= kMagicLength && std::memcmp(data, kJwafMagic, kMagicLength) == 0;
      }
    }
  }
}
