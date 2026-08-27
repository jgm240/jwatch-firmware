#pragma once

#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Applications {

    // Regular grid apps only ever carried an icon glyph (AppTraits<>::icon) - Grid/Circle
    // menu styles never needed a display name. The List style does, so this is a single
    // centralized lookup rather than adding a name to every app's own AppTraits
    // specialization across ~25 files.
    const char* AppNameFor(Apps app);
  }
}
