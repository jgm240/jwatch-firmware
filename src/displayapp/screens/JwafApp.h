#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/Controllers.h"
#include "components/fs/FS.h"
#include "components/fs/JwafFormat.h"
#include "components/sandbox/AppSandbox.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Browses "/apps" on the external flash for installed .jwaf files (see JwafFormat.h) and,
      // once one is picked, parses and shows it. Installing one is just writing it to that
      // directory over the existing BLE FSService - nothing JWAF-specific needed on the wire.
      class JwafApp : public Screen {
      public:
        explicit JwafApp(Pinetime::Controllers::FS& filesystem);
        ~JwafApp() override;

        void OnEvent(lv_obj_t* obj, lv_event_t event);

      private:
        static constexpr int MaxFiles = 32;

        struct FileEntry {
          char name[48];
        };

        enum class Mode { List, Viewing, Error };

        void LoadFileList();
        void OpenFile(const char* name);
        void OpenTextFile(const uint8_t* buffer, size_t size);
        void OpenCodeFile(const uint8_t* buffer, size_t size);
        void ShowViewingChrome(const char* title); // "< Apps" back row + title label, shared by both kinds
        void ShowList();
        void ShowError(const char* message);

        // Big enough for either kind of file's largest possible body - the code variant's
        // (AppSandbox::CodeAreaSize) dwarfs the text variant's, but both are computed here
        // rather than just assumed. Too large for the display-app task's own stack (800
        // *words* total - see DisplayApp::Start), hence a class member instead of a local.
        static constexpr size_t TextBodyMaxSize =
          Controllers::JwafFormat::TitleMaxLength + Controllers::JwafFormat::MaxLines * (1 + Controllers::JwafFormat::LineMaxLength);
        static constexpr size_t CodeBodyMaxSize = 2 + Controllers::AppSandbox::CodeAreaSize;
        static constexpr size_t FileBufferSize =
          Controllers::JwafFormat::kHeaderSize + (TextBodyMaxSize > CodeBodyMaxSize ? TextBodyMaxSize : CodeBodyMaxSize);

        Controllers::FS& filesystem;

        Mode mode = Mode::List;

        std::array<FileEntry, MaxFiles> files {};
        int fileCount = 0;
        std::array<lv_obj_t*, MaxFiles> buttonObjects {};
        std::array<uint8_t, FileBufferSize> fileBuffer {};

        lv_obj_t* list = nullptr;
        lv_obj_t* titleLabel = nullptr;
        std::array<lv_obj_t*, Controllers::JwafFormat::MaxLines> lineLabels {};
        lv_obj_t* errorLabel = nullptr;
      };
    }

    template <>
    struct AppTraits<Apps::JwafApp> {
      static constexpr Apps app = Apps::JwafApp;
      static constexpr const char* icon = Screens::Symbols::info;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::JwafApp(controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
