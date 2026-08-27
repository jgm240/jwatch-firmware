#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Snake : public Screen {
      public:
        Snake();
        ~Snake() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        enum class Direction { Up, Down, Left, Right };

        struct Cell {
          int8_t x;
          int8_t y;
        };

        static constexpr int cellSize = 20;
        static constexpr int nCols = 12;
        static constexpr int nRows = 12;
        static constexpr size_t maxLength = nCols * nRows;

        lv_obj_t* CreateSegment(int x, int y);
        void Reset();
        void SpawnFood();
        void EndGame();

        std::array<Cell, maxLength> body;
        std::array<lv_obj_t*, maxLength> bodyObjects {};
        size_t headIndex = 0;
        size_t length = 3;

        Direction direction = Direction::Right;
        Direction pendingDirection = Direction::Right;

        Cell food {0, 0};
        lv_obj_t* foodObject = nullptr;

        lv_obj_t* scoreLabel;
        lv_obj_t* gameOverLabel;

        uint32_t score = 0;
        bool gameOver = false;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Snake> {
      static constexpr Apps app = Apps::Snake;
      static constexpr const char* icon = "S";

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        return new Screens::Snake();
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
