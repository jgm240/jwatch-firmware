#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      void SetRadioButtonStyle(lv_obj_t* checkbox);

      namespace Styles {
        // Live-mutable button corner radius, set once at boot from Settings and again whenever
        // the user changes it in SettingButtonRadius - applies to freshly-constructed screens,
        // same "changeable, takes effect on next screen entry" model as Theme (dark/light).
        void SetButtonRadius(uint8_t radius);
        uint8_t GetButtonRadius();

        // Live-mutable "glass" look: translucent fill, a bright top-to-bottom gloss gradient,
        // and a thin light border - no real blur (this LVGL version/hardware can't do backdrop
        // blur at all), just the opacity/border/gradient cues that read as a glass surface.
        void SetGlassEnabled(bool enabled);
        bool IsGlassEnabled();

        // lv_btn_create(parent, nullptr) + the current button radius (and glass look, if
        // enabled) applied, in one call - the single place every interactive button in the
        // app should be created from.
        lv_obj_t* Button(lv_obj_t* parent);
      }
    }
  }
}
