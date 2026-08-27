#include "displayapp/widgets/SecondsSquare.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  constexpr lv_coord_t margin = 2;
  constexpr lv_coord_t thickness = 3;

  lv_coord_t TrackLen() {
    return LV_HOR_RES - 2 * margin;
  }

  lv_obj_t* CreateBar(lv_obj_t* parent, lv_color_t dimColor) {
    lv_obj_t* bar = lv_obj_create(parent, nullptr);
    lv_obj_set_style_local_bg_color(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, dimColor);
    lv_obj_set_style_local_bg_opa(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_border_width(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(bar, false);
    return bar;
  }

  // Dim static track bars, drawn once and never moved - the "unlit" background of each side.
  void CreateDimTrack(lv_obj_t* parent) {
    lv_color_t dim = lv_color_hex(0x404040);
    lv_coord_t len = TrackLen();

    lv_obj_t* top = CreateBar(parent, dim);
    lv_obj_set_size(top, len, thickness);
    lv_obj_set_pos(top, margin, margin);

    lv_obj_t* right = CreateBar(parent, dim);
    lv_obj_set_size(right, thickness, len);
    lv_obj_set_pos(right, LV_HOR_RES - margin - thickness, margin);

    lv_obj_t* bottom = CreateBar(parent, dim);
    lv_obj_set_size(bottom, len, thickness);
    lv_obj_set_pos(bottom, margin, LV_VER_RES - margin - thickness);

    lv_obj_t* left = CreateBar(parent, dim);
    lv_obj_set_size(left, thickness, len);
    lv_obj_set_pos(left, margin, margin);
  }
}

void SecondsSquare::Create(lv_obj_t* parent, lv_color_t color) {
  CreateDimTrack(parent);

  litTop = CreateBar(parent, color);
  litRight = CreateBar(parent, color);
  litBottom = CreateBar(parent, color);
  litLeft = CreateBar(parent, color);

  SetValue(0);
}

void SecondsSquare::SetValue(uint8_t seconds) {
  lv_coord_t len = TrackLen();

  float legProgress = (static_cast<float>(seconds) / 60.0F) * 4.0F;
  int leg = static_cast<int>(legProgress);
  float fracInLeg = legProgress - static_cast<float>(leg);

  float progressTop = (leg > 0) ? 1.0F : fracInLeg;
  float progressRight = (leg > 1) ? 1.0F : ((leg == 1) ? fracInLeg : 0.0F);
  float progressBottom = (leg > 2) ? 1.0F : ((leg == 2) ? fracInLeg : 0.0F);
  float progressLeft = (leg >= 3) ? fracInLeg : 0.0F;

  // Top: grows rightward from the top-left corner - fixed position, growing width.
  auto topWidth = static_cast<lv_coord_t>(len * progressTop);
  lv_obj_set_size(litTop, topWidth, thickness);
  lv_obj_set_pos(litTop, margin, margin);

  // Right: grows downward from the top-right corner - fixed position, growing height.
  auto rightHeight = static_cast<lv_coord_t>(len * progressRight);
  lv_obj_set_size(litRight, thickness, rightHeight);
  lv_obj_set_pos(litRight, LV_HOR_RES - margin - thickness, margin);

  // Bottom: grows leftward from the bottom-right corner - right edge fixed, so position
  // shifts left as the (growing) width increases.
  auto bottomWidth = static_cast<lv_coord_t>(len * progressBottom);
  lv_obj_set_size(litBottom, bottomWidth, thickness);
  lv_obj_set_pos(litBottom, LV_HOR_RES - margin - bottomWidth, LV_VER_RES - margin - thickness);

  // Left: grows upward from the bottom-left corner - bottom edge fixed, so position
  // shifts up as the (growing) height increases.
  auto leftHeight = static_cast<lv_coord_t>(len * progressLeft);
  lv_obj_set_size(litLeft, thickness, leftHeight);
  lv_obj_set_pos(litLeft, margin, LV_VER_RES - margin - leftHeight);
}
