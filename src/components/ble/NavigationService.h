/*  Copyright (C) 2021  Adam Pigg

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <cstdint>
#include <string>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

namespace Pinetime {
  namespace Controllers {

    class NavigationService {
    public:
      NavigationService();

      void Init();

      int OnCommand(struct ble_gatt_access_ctxt* ctxt);

      std::string getFlag();

      std::string getNarrative();

      std::string getManDist();

      int getProgress();

      // 0 none, 1 left, 2 right, 3 arrive. Set when the phone sends a new maneuver flag; clears on read.
      uint8_t ConsumeTurn() {
        uint8_t turn = pendingTurn;
        pendingTurn = 0;
        return turn;
      }

      // 1 = a route just started, 2 = it ended (flag went back to empty/"flag"); clears on read.
      uint8_t ConsumeLifecycle() {
        uint8_t event = pendingLifecycle;
        pendingLifecycle = 0;
        return event;
      }

    private:
      struct ble_gatt_chr_def characteristicDefinition[5];
      struct ble_gatt_svc_def serviceDefinition[2];

      std::string m_flag;
      std::string m_narrative;
      std::string m_manDist;
      int m_progress;
      uint8_t pendingTurn = 0;
      uint8_t pendingLifecycle = 0;
      bool active = false;
    };
  }
}
