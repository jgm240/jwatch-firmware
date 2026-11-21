#pragma once
#include <cstddef>
#include <cstdint>

// Runs a JWAF "code" blob (JwafFormat::kFormatVersionCode) from a fixed 5 KB RAM slot, caged
// by the nRF52832's real ARMv7-M MPU (confirmed present: __MPU_PRESENT in the SDK's nrf52.h)
// so a broken blob faults instead of corrupting the rest of the watch.
//
// Design, and what it does and doesn't contain:
//  - The slot is "pure compute, no callbacks": the app can't call back into LVGL or any other
//    firmware function (it has no linkable addresses for them - only its own code, called at
//    a fixed offset). It writes its result as a NUL-terminated ASCII string into its own
//    output area within the slot; AppSandbox reads that back out once the app returns. This
//    sidesteps needing a safe way to cross the MPU boundary mid-call, which is the genuinely
//    hard part of a sandbox like this.
//  - While the app runs, PSP (this firmware's FreeRTOS port keeps tasks on PSP, confirmed from
//    vPortSVCHandler's "msr psp, r0" - see port.c) is redirected to a stack *inside* the slot,
//    and the MPU is configured to allow only: the slot itself (RWX), the firmware's own
//    internal flash (RX, so the trampoline's own code stays executable), and the fixed MSP
//    exception stack at the top of RAM (RW - __StackTop - __STACK_SIZE from the linker script).
//    Everything else - the rest of RAM, peripherals - is denied. Interrupts are masked for the
//    same window (both because nothing else may touch the now-off-limits firmware RAM, and
//    because FreeRTOS must not preempt a task whose PSP it doesn't recognize).
//  - A bad memory access (wild pointer, overrun) raises a CPU fault instead of corrupting
//    anything. The fault handler (see components/crashlog/FaultHandlers.cpp) doesn't try to
//    resume - it records what happened and resets the watch, same as any other firmware fault.
//    That's deliberate: trying to unwind and continue from an arbitrary fault is the one part
//    of this design that's genuinely hard to get right without hardware to iterate on, and
//    getting *that* wrong would be worse than a clean reset.
//  - What this does NOT contain: an infinite loop. A fault needs a bad *access*; a hang in a
//    tight loop produces neither, so recovery still falls back to the existing hardware
//    watchdog, same as a hang anywhere else in this firmware. Keep blobs short and non-looping.
//  - Interrupts (BLE radio timing included) are paused for the call's duration, so blobs
//    should be small and fast - this isn't a general coprocessor, it's a few hundred bytes of
//    compute that writes one string and returns.
namespace Pinetime {
  namespace Controllers {
    class AppSandbox {
    public:
      // 8 KiB aligned base so the MPU can address it as one region; only the first 5 KiB
      // (CodeAreaSize + OutputAreaSize + StackAreaSize) are enabled - the ARMv7-M MPU's 8-way
      // subregion-disable is used to cut an 8 KiB region down to exactly 5 KiB (disable the
      // last 3 of 8 1 KiB subregions). The trailing 3 KiB is never touched by anything.
      static constexpr size_t SlotAlignedSize = 8192;
      static constexpr size_t UsableSize = 5120;

      static constexpr size_t CodeAreaSize = 3072;   // code + its own globals, loaded at slot start
      static constexpr size_t OutputAreaSize = 1024;  // NUL-terminated result string, written by the app
      static constexpr size_t StackAreaSize = 1024;   // the app's own stack, growing down from the end

      enum class Result : uint8_t {
        Ok,
        CodeTooLarge,
        Faulted, // only reachable if a fault's reset is ever skipped; today a fault always resets.
      };

      // Copies 'code' (must be <= CodeAreaSize) into the slot, runs it with PSP/MPU confined as
      // described above, and on a normal return copies its output string (if any) into
      // 'output' (NUL-terminated, truncated to outputCap). A fault during the call resets the
      // watch instead of returning here - see FaultHandlers.cpp.
      static Result Run(const uint8_t* code, size_t codeSize, char* output, size_t outputCap);

      // True only while a Run() call's app code is actually executing - checked by the fault
      // handlers so a crash's recorded message can say "app" instead of just "firmware".
      static bool IsRunning() {
        return sRunning;
      }

    private:
      static volatile bool sRunning;
    };
  }
}
