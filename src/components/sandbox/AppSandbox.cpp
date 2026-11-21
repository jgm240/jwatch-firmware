#include "components/sandbox/AppSandbox.h"
#include <cstring>
#include <mdk/nrf.h>

using namespace Pinetime::Controllers;

extern "C" {
// AppSandboxTrampoline.S. Switches PSP into 'newSp', branches into 'entry' (Thumb bit forced
// on there, not here), then restores PSP and returns here normally - MPU setup/teardown and
// interrupt masking happen around it in EnableMpu/DisableMpu below, called from the same .S
// file so the whole window (mask -> MPU on -> call -> MPU off -> unmask) is one unbroken
// sequence with no C-compiler-generated stack use in between.
void AppSandbox_Invoke(void* entry, void* newSp);
}

namespace {
  // Placed in its own linker-script section (gcc_nrf52-mcuboot.ld), 8 KiB-aligned there, so
  // the MPU can address it as a single region - see that script for why this isn't just an
  // `alignas(8192)` on the object (see AppSandbox.h for why only the first 5 KiB of it are
  // ever actually enabled).
  __attribute__((section(".app_sandbox_slot"))) uint8_t sSlot[AppSandbox::SlotAlignedSize];

  constexpr uint32_t kFlashBase = 0x00000000;
  // Covers the whole 512 KiB internal flash (nRF52832, NRF52832_XXAA) as one region so the
  // trampoline's own code - and the fault/reset path after it - stays executable no matter
  // where in flash it ends up. 512 KiB = 2^19, RASR.SIZE = 19-1 = 18.
  constexpr uint32_t kFlashRegionSizeField = 18;

  // __StackTop - __STACK_SIZE from gcc_nrf52-mcuboot.ld / nrf_common.ld: the fixed MSP
  // exception stack at the very top of RAM. RAM is 0x20000000 + 64 KiB = 0x20010000;
  // __STACK_SIZE is 1024 (build flag), and 0x20010000 - 1024 = 0x2000FC00, which is itself
  // 1024-byte aligned - confirmed by inspection, not assumed, since this has to be exactly
  // right for fault/interrupt entry during a sandboxed call to land somewhere valid.
  constexpr uint32_t kMspStackBase = 0x2000FC00;
  constexpr uint32_t kMspStackSizeField = 9; // 2^(9+1) = 1024

  constexpr uint32_t kSlotRegionSizeField = 12; // 2^(12+1) = 8192
  // Disable the last 3 of 8 1 KiB subregions of the 8 KiB slot region, leaving exactly the
  // first 5 KiB (AppSandbox::UsableSize) enabled.
  constexpr uint32_t kSlotSubregionDisable = 0xE0;

  constexpr uint32_t kApFullAccess = 0b011;
  constexpr uint32_t kApReadOnly = 0b110;

  void ConfigureRegion(uint32_t regionNumber, uint32_t base, uint32_t sizeField, uint32_t ap, bool executable, uint32_t srd = 0) {
    MPU->RBAR = (base & ~0x1FUL) | MPU_RBAR_VALID_Msk | (regionNumber & MPU_RBAR_REGION_Msk);
    uint32_t rasr = (ap << MPU_RASR_AP_Pos) | (sizeField << MPU_RASR_SIZE_Pos) | (srd << MPU_RASR_SRD_Pos) | MPU_RASR_ENABLE_Msk;
    if (!executable) {
      rasr |= MPU_RASR_XN_Msk;
    }
    MPU->RASR = rasr;
  }
}

extern "C" void AppSandbox_EnableMpu() {
  __DMB();
  MPU->CTRL = 0;
  ConfigureRegion(0, kFlashBase, kFlashRegionSizeField, kApReadOnly, /*executable=*/true);
  ConfigureRegion(1, kMspStackBase, kMspStackSizeField, kApFullAccess, /*executable=*/false);
  ConfigureRegion(2, reinterpret_cast<uint32_t>(sSlot), kSlotRegionSizeField, kApFullAccess, /*executable=*/true, kSlotSubregionDisable);
  // PRIVDEFENA deliberately left clear: everything not covered by a region above - the rest
  // of RAM, peripherals - is denied for the duration of the call.
  MPU->CTRL = MPU_CTRL_ENABLE_Msk;
  __DSB();
  __ISB();
}

extern "C" void AppSandbox_DisableMpu() {
  MPU->CTRL = 0;
  __DSB();
  __ISB();
}

volatile bool AppSandbox::sRunning = false;

AppSandbox::Result AppSandbox::Run(const uint8_t* code, size_t codeSize, char* output, size_t outputCap) {
  if (codeSize > CodeAreaSize) {
    return Result::CodeTooLarge;
  }

  std::memset(sSlot, 0, UsableSize);
  std::memcpy(sSlot, code, codeSize);

  uint8_t* outputArea = sSlot + CodeAreaSize;
  uint8_t* stackArea = sSlot + CodeAreaSize + OutputAreaSize;
  void* entry = sSlot;                   // the blob's own first instruction, by convention.
  void* newSp = stackArea + StackAreaSize; // stack grows down from the top of the stack area.

  sRunning = true;
  AppSandbox_Invoke(entry, newSp); // never returns here if the app faults - see FaultHandlers.cpp.
  sRunning = false;

  outputArea[OutputAreaSize - 1] = '\0'; // guarantee termination regardless of what the app wrote.
  std::strncpy(output, reinterpret_cast<const char*>(outputArea), outputCap - 1);
  output[outputCap - 1] = '\0';
  return Result::Ok;
}
