#pragma once

#include <memory>
#include "displayapp/widgets/complications/Complication.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      std::unique_ptr<Complication> CreateComplication(ComplicationType type, AppControllers& controllers);
    }
  }
}
