#pragma once

#include <bitset>
#include <cstdint>
#include "components/fs/FS.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      namespace HandwritingGlyphs {

        static constexpr int glyphWidth = 45;
        static constexpr int glyphHeight = 90;
        // Indices 0-9 are the digits '0'-'9', index 10 is the ':' symbol.
        static constexpr int glyphCount = 11;
        static constexpr int colonIndex = 10;

        using GlyphBitmap = std::bitset<glyphWidth * glyphHeight>;

        // Returns the littlefs path (e.g. "/glyphs/glyph_7.bin") for a glyph index (0-9, colonIndex).
        const char* GlyphFileName(int index);

        // Creates /glyphs if it doesn't already exist. Returns false only on a real error.
        bool EnsureGlyphsDir(Pinetime::Controllers::FS& fs);

        // Packs and writes one glyph bitmap to flash in the exact format LVGL's built-in
        // image decoder expects for LV_IMG_CF_ALPHA_1BIT files loaded via lv_img_set_src("F:/...").
        bool SaveGlyph(Pinetime::Controllers::FS& fs, int index, const GlyphBitmap& bitmap);

        // True if a probe-open of the glyph file at `index` succeeds.
        bool GlyphExists(Pinetime::Controllers::FS& fs, int index);
      }
    }
  }
}
