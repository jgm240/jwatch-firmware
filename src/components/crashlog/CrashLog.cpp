#include "components/crashlog/CrashLog.h"
#include <cstring>
#include <mdk/nrf.h>

namespace {
  constexpr uint32_t kMagic = 0x43524153u; // "CRAS", arbitrary but distinctive

  struct Record {
    uint32_t magic;
    char message[Pinetime::System::CrashLog::MessageMaxLength];
  };

  // Deliberately NOT zero-initialized at boot (see the .noinit section in gcc_nrf52-mcuboot.ld)
  // so its content survives the very reset this module triggers.
  __attribute__((section(".noinit"))) Record record;
}

namespace Pinetime {
  namespace System {
    namespace CrashLog {
      void RecordAndReset(const char* message) {
        record.magic = kMagic;
        size_t len = 0;
        if (message != nullptr) {
          while (message[len] != '\0' && len < MessageMaxLength - 1) {
            record.message[len] = message[len];
            len++;
          }
        }
        record.message[len] = '\0';

        NVIC_SystemReset();
        while (true) {
        }
      }

      bool TakePending(char* outMessage, size_t outCap) {
        bool valid = record.magic == kMagic;
        if (valid && outMessage != nullptr && outCap > 0) {
          std::strncpy(outMessage, record.message, outCap - 1);
          outMessage[outCap - 1] = '\0';
        }
        record.magic = 0;
        return valid;
      }
    }
  }
}
