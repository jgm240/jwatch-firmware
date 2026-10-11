#pragma once
#include <cstdint>
#include <drivers/include/nrfx_saadc.h>
#include <systemtask/SystemTask.h>

namespace Pinetime {
  namespace Controllers {

    class Battery {
    public:
      Battery();

      void ReadPowerState();
      void MeasureVoltage();
      void Register(System::SystemTask* systemTask);

      uint8_t PercentRemaining() const {
        return percentRemaining;
      }

      uint16_t Voltage() const {
        return voltage;
      }

      bool IsCharging() const {
        // isCharging will go up and down when fully charged
        // isFull makes sure this returns false while fully charged.
        return isCharging && !isFull;
      }

      bool IsPowerPresent() const {
        return isPowerPresent;
      }

      // Estimated minutes of battery left at the recently observed drain rate; 0 = not known yet
      // (needs ~20 min of discharge without charging, and resets whenever power is connected).
      uint32_t MinutesRemaining() const {
        return drainPercentPerHourX100 == 0 ? 0 : (static_cast<uint32_t>(percentRemaining) * 6000) / drainPercentPerHourX100;
      }

    private:
      static Battery* instance;
      nrf_saadc_value_t saadc_value;

      static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
      uint16_t voltage = 0;
      uint8_t percentRemaining = 0;

      bool isFull = false;
      bool isCharging = false;
      bool isPowerPresent = false;
      bool firstMeasurement = true;

      void UpdateDrainEstimate();
      uint8_t drainBaselinePercent = 0xff;
      uint32_t drainBaselineTick = 0;
      uint32_t drainPercentPerHourX100 = 0;

      void SaadcInit();

      void SaadcEventHandler(nrfx_saadc_evt_t const* p_event);
      static void AdcCallbackStatic(nrfx_saadc_evt_t const* event);

      bool isReading = false;

      Pinetime::System::SystemTask* systemTask = nullptr;
    };
  }
}
