#pragma once
#include <cstddef>

// Carries a short crash message across a reset, so the NEXT boot can show the user what went
// wrong instead of silently landing back on the watch face. Works because ordinary SRAM
// content survives a warm reset (pin reset excepted) - this just needs the record to sit in a
// RAM section the startup code never zero-initializes, which gcc_nrf52-mcuboot.ld already
// provides (".noinit"), originally for other callers.
namespace Pinetime {
  namespace System {
    namespace CrashLog {
      inline constexpr size_t MessageMaxLength = 80;

      // Call only from a fault handler (or anywhere else about to deliberately reset): records
      // 'message' (truncated to fit) into the surviving RAM record, then resets the MCU. Never
      // returns - a fault handler's registers/stack may already be in a bad state, so this
      // doesn't try to unwind back into whatever was running.
      [[noreturn]] void RecordAndReset(const char* message);

      // Call once at boot, before anything else touches the record. If a valid record from the
      // previous boot is found, fills 'outMessage' (NUL-terminated) and returns true. Either
      // way, clears the record so a stale message can't resurface after the next ordinary
      // reset.
      bool TakePending(char* outMessage, size_t outCap);
    }
  }
}
