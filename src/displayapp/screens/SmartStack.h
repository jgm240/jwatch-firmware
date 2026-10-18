#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <lvgl/lvgl.h>

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // One full-screen "card": an icon, a big value and a subtitle, backed directly by whichever
      // controller that card type needs. Shared across all card types rather than one class each,
      // since the layout is identical - only Refresh()'s content differs.
      enum class SmartStackCardType : uint8_t {
        Alarm,
        Workout,
        NowPlaying,
        NextEvent,
        Countdown,
        Notifications,
        WatchBattery,
        PhoneBattery,
        Steps,
        HeartRate,
        Weather,
        WorldClock,
        Sun,
        Date,
        FitnessRings,
        Timer,
      };
      static constexpr size_t smartStackCardCount = static_cast<size_t>(SmartStackCardType::Timer) + 1;
      // Short name shown in Settings > Smart Stack, in the same order as SmartStackCardType.
      extern const std::array<const char*, smartStackCardCount> smartStackCardNames;

      class SmartStackCard : public Screen {
      public:
        SmartStackCard(SmartStackCardType type, const AppControllers& controllers);
        ~SmartStackCard() override;

        void Refresh() override;

      private:
        SmartStackCardType type;
        const AppControllers& controllers;

        lv_obj_t* iconLabel;
        lv_obj_t* valueLabel;
        lv_obj_t* subLabel;
        lv_task_t* taskRefresh;

        // Only used by the Alarm/Workout/NowPlaying cards, whose relevance can change while the card
        // is already open (e.g. a workout started elsewhere) - everything else is set once since its
        // relevance was already decided before this card was shown.
        bool lastState = false;
      };

      // Swipeable stack of SmartStackCard pages, ordered by relevance (most useful card first) at
      // the moment the stack is opened - it doesn't reshuffle while you're browsing it. Cards turned
      // off in Settings > Smart Stack are left out entirely, so this paginates itself at a size
      // decided at runtime rather than using the (compile-time-sized) ScreenList other apps use.
      class SmartStack : public Screen {
      public:
        explicit SmartStack(const AppControllers& controllers);
        ~SmartStack() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        void Show(uint8_t index);

        const AppControllers& controllers;
        DisplayApp* app;
        std::array<SmartStackCardType, smartStackCardCount> order {};
        uint8_t activeCount = 0;
        uint8_t index = 0;
        std::unique_ptr<Screen> current;
      };
    }

    template <>
    struct AppTraits<Apps::SmartStack> {
      static constexpr Apps app = Apps::SmartStack;
      static constexpr const char* icon = Screens::Symbols::list;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SmartStack(controllers);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
