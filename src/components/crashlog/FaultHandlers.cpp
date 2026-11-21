#include "components/crashlog/CrashLog.h"
#include "components/sandbox/AppSandbox.h"
#include <cstdio>
#include <mdk/nrf.h>

// Overrides the weak default handlers from gcc_startup_nrf52.S (they just spin forever) so a
// CPU fault anywhere in the firmware - not only inside a sandboxed JWAF app - records what
// happened and resets cleanly instead of hanging silently. These all run on MSP (exception
// entry always does, regardless of what task code was doing on PSP), so they're ordinary,
// safe C functions with no special handling needed - see AppSandbox.h for why that's true
// even while a sandboxed app's PSP points somewhere unusual.
//
// Deliberately NOT attempting to resume whatever was running: safely rewinding from an
// arbitrary fault is the one part of a setup like this that's genuinely hard to get right
// without real hardware to iterate against, and getting that part wrong risks being worse
// than just resetting. CrashLog::RecordAndReset() both captures the message and resets.

namespace {
  // SCB->CFSR's MMARVALID/BFARVALID bits aren't given names in this SDK's CMSIS header (only
  // the three whole-byte sub-status fields are), so these are spelled out directly: bit 7 of
  // the MemManage status byte, bit 7 of the BusFault status byte (i.e. bit 15 of CFSR).
  constexpr uint32_t kMemManageAddrValid = 1UL << 7;
  constexpr uint32_t kBusFaultAddrValid = 1UL << 15;
}

extern "C" {

void HardFault_Handler() {
  Pinetime::System::CrashLog::RecordAndReset("HardFault");
}

void MemoryManagement_Handler() {
  char buf[64];
  const char* who = Pinetime::Controllers::AppSandbox::IsRunning() ? "App" : "Firmware";
  if (SCB->CFSR & kMemManageAddrValid) {
    snprintf(buf, sizeof(buf), "%s memory fault at 0x%08lX", who, static_cast<unsigned long>(SCB->MMFAR));
  } else {
    snprintf(buf, sizeof(buf), "%s memory fault", who);
  }
  Pinetime::System::CrashLog::RecordAndReset(buf);
}

void BusFault_Handler() {
  char buf[64];
  if (SCB->CFSR & kBusFaultAddrValid) {
    snprintf(buf, sizeof(buf), "Bus fault at 0x%08lX", static_cast<unsigned long>(SCB->BFAR));
  } else {
    snprintf(buf, sizeof(buf), "Bus fault");
  }
  Pinetime::System::CrashLog::RecordAndReset(buf);
}

void UsageFault_Handler() {
  Pinetime::System::CrashLog::RecordAndReset("Usage fault");
}

// The watchdog's own early-warning interrupt (fires ~61 us before its forced reset on an
// actual hang) is handled in main.cpp's pre-existing WDT_IRQHandler, not here - that symbol
// already existed for an unrelated reason and a handler can only be defined once.
}
