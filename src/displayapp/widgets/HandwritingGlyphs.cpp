#include "displayapp/widgets/HandwritingGlyphs.h"
#include <array>

using namespace Pinetime::Applications::Widgets;

namespace {
  constexpr std::array<const char*, HandwritingGlyphs::glyphCount> fileNames = {{
    "/glyphs/glyph_0.bin",
    "/glyphs/glyph_1.bin",
    "/glyphs/glyph_2.bin",
    "/glyphs/glyph_3.bin",
    "/glyphs/glyph_4.bin",
    "/glyphs/glyph_5.bin",
    "/glyphs/glyph_6.bin",
    "/glyphs/glyph_7.bin",
    "/glyphs/glyph_8.bin",
    "/glyphs/glyph_9.bin",
    "/glyphs/glyph_colon.bin",
  }};

  // LVGL's built-in image decoder expects each row byte-aligned (ceil(width/8) bytes/row),
  // MSB-first within a byte, bit=1 meaning "opaque/drawn". This must match
  // lv_img_decoder_built_in_line_alpha's alpha1 layout exactly, byte for byte.
  constexpr int bytesPerRow = (HandwritingGlyphs::glyphWidth + 7) / 8;
  constexpr int pixelDataSize = bytesPerRow * HandwritingGlyphs::glyphHeight;
  constexpr uint8_t colorFormatAlpha1Bit = 11; // LV_IMG_CF_ALPHA_1BIT
}

const char* HandwritingGlyphs::GlyphFileName(int index) {
  if (index < 0 || index >= glyphCount) {
    return fileNames[0];
  }
  return fileNames[index];
}

bool HandwritingGlyphs::EnsureGlyphsDir(Pinetime::Controllers::FS& fs) {
  int res = fs.DirCreate("/glyphs");
  return res == 0 || res == LFS_ERR_EXIST;
}

bool HandwritingGlyphs::GlyphExists(Pinetime::Controllers::FS& fs, int index) {
  lfs_file_t file;
  if (fs.FileOpen(&file, GlyphFileName(index), LFS_O_RDONLY) < 0) {
    return false;
  }
  fs.FileClose(&file);
  return true;
}

bool HandwritingGlyphs::SaveGlyph(Pinetime::Controllers::FS& fs, int index, const GlyphBitmap& bitmap) {
  if (index < 0 || index >= glyphCount) {
    return false;
  }
  if (!EnsureGlyphsDir(fs)) {
    return false;
  }

  std::array<uint8_t, 4 + pixelDataSize> buffer {};

  uint32_t header = (static_cast<uint32_t>(colorFormatAlpha1Bit) & 0x1Fu) |
                     ((static_cast<uint32_t>(glyphWidth) & 0x7FFu) << 10) | ((static_cast<uint32_t>(glyphHeight) & 0x7FFu) << 21);
  buffer[0] = static_cast<uint8_t>(header & 0xFFu);
  buffer[1] = static_cast<uint8_t>((header >> 8) & 0xFFu);
  buffer[2] = static_cast<uint8_t>((header >> 16) & 0xFFu);
  buffer[3] = static_cast<uint8_t>((header >> 24) & 0xFFu);

  for (int y = 0; y < glyphHeight; y++) {
    for (int x = 0; x < glyphWidth; x++) {
      if (bitmap[static_cast<size_t>(y) * glyphWidth + x]) {
        size_t byteIndex = 4 + static_cast<size_t>(y) * bytesPerRow + (x / 8);
        uint8_t bitIndex = 7 - (x % 8);
        buffer[byteIndex] |= static_cast<uint8_t>(1u << bitIndex);
      }
    }
  }

  lfs_file_t file;
  if (fs.FileOpen(&file, GlyphFileName(index), LFS_O_WRONLY | LFS_O_CREAT) < 0) {
    return false;
  }
  bool ok = fs.FileWrite(&file, buffer.data(), buffer.size()) == static_cast<int>(buffer.size());
  fs.FileClose(&file);
  return ok;
}
