#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/fs/FS.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class FileBrowser : public Screen {
      public:
        explicit FileBrowser(Pinetime::Controllers::FS& filesystem);
        ~FileBrowser() override;

        void OnEvent(lv_obj_t* obj, lv_event_t event);

      private:
        struct Entry {
          char name[51];
          bool isDir;
          uint32_t size;
        };

        void LoadDirectory();
        void NavigateInto(const Entry& entry);
        void NavigateUp();

        Controllers::FS& filesystem;

        static constexpr int maxEntries = 64;
        std::array<Entry, maxEntries> entries {};
        int entryCount = 0;

        std::array<lv_obj_t*, maxEntries + 1> buttonObjects {};
        int totalRows = 0;
        bool hasUpRow = false;

        char currentPath[256] = "/";

        lv_obj_t* pathLabel;
        lv_obj_t* list;
      };
    }

    template <>
    struct AppTraits<Apps::FileBrowser> {
      static constexpr Apps app = Apps::FileBrowser;
      static constexpr const char* icon = Screens::Symbols::list;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::FileBrowser(controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
