#pragma once

#include <lvgl/lvgl.h>

namespace Colors {
  static constexpr lv_color_t deepOrange = LV_COLOR_MAKE(0xff, 0x40, 0x0);
  static constexpr lv_color_t orange = LV_COLOR_MAKE(0xff, 0xb0, 0x0);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x0, 0xb0, 0x0);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x0, 0x50, 0xff);
  static constexpr lv_color_t lightGray = LV_COLOR_MAKE(0xb0, 0xb0, 0xb0);
  static constexpr lv_color_t gray = LV_COLOR_MAKE(0x50, 0x50, 0x50);

  static constexpr lv_color_t bg = LV_COLOR_MAKE(0x5d, 0x69, 0x7e);
  static constexpr lv_color_t bgAlt = LV_COLOR_MAKE(0x38, 0x38, 0x38);
  static constexpr lv_color_t bgDark = LV_COLOR_MAKE(0x18, 0x18, 0x18);
  static constexpr lv_color_t highlight = green;

  // Light theme palette. Only used by InfiniTimeTheme.cpp (and any watchface that
  // explicitly opts in, e.g. the retrofitted WatchFaceDigital) -- the constants
  // above keep meaning "dark" so every screen that already references them
  // directly is unaffected by light mode.
  namespace Light {
    static constexpr lv_color_t background = LV_COLOR_MAKE(0xf2, 0xf2, 0xf2);
    static constexpr lv_color_t text = LV_COLOR_MAKE(0x10, 0x10, 0x10);
    static constexpr lv_color_t button = LV_COLOR_MAKE(0xcf, 0xd4, 0xdc);
    static constexpr lv_color_t buttonDisabled = LV_COLOR_MAKE(0xb8, 0xb8, 0xb8);
    static constexpr lv_color_t icon = LV_COLOR_MAKE(0x20, 0x20, 0x20);
  };
};

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init(bool isLight);
