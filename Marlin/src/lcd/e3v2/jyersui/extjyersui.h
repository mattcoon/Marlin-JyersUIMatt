/**
 * ExtJyersUI extenions
 * Author: LChristophe68 (tititopher68-dev)
 * Version: 1.0
 * Date: 2022/02/03
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For commercial applications additional licences can be requested
 */
#pragma once

#include "../../../inc/MarlinConfigPre.h"
#include "../../../core/types.h"


#ifndef LOW
  #define LOW  0x0
#endif
#ifndef HIGH
  #define HIGH 0x1
#endif

#if EXTJYERSUI

  #define DEF_NOZZLE_PARK_POINT {240, 220, 20}
  #define MIN_PARK_POINT_Z 10

  #if HAS_BED_PROBE
    constexpr int16_t DEF_PROBING_MARGIN = PROBING_MARGIN;
    #define MIN_PROBE_MARGIN 5
    #define MAX_PROBE_MARGIN 60
    #define MIN_Z_PROBE_FEEDRATE 60
    #define MAX_Z_PROBE_FEEDRATE 1200
    constexpr int16_t DEF_Z_PROBE_FEEDRATE_FAST = Z_PROBE_FEEDRATE_FAST;
    constexpr int16_t DEF_Z_PROBE_FEEDRATE_SLOW = (Z_PROBE_FEEDRATE_FAST / 2);
  #endif

  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    #define MIN_FIL_CHANGE_FEEDRATE 1
    #define MAX_FIL_CHANGE_FEEDRATE 60
    constexpr int8_t DEF_FILAMENT_CHANGE_UNLOAD_FEEDRATE = FILAMENT_CHANGE_UNLOAD_FEEDRATE;
    constexpr int8_t DEF_FILAMENT_CHANGE_FAST_LOAD_FEEDRATE = FILAMENT_CHANGE_FAST_LOAD_FEEDRATE;
  #endif

  #define DEF_INVERT_E0_DIR false
  //#define DEF_FIL_MOTION_SENSOR false

  // Class
  class ExtJyersuiClass {
  public:
  
    #if ENABLED(NOZZLE_PARK_FEATURE)
      static void C125();
    #endif
      static void C562();
    #if HAS_BED_PROBE
      static void C851();
    #endif
 
  };

  extern ExtJyersuiClass ExtJyersui;

#endif


