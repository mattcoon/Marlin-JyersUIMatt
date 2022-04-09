/**
 * JyersUI extensions
 * Author: tititopher68-dev (Christophe LEVEQUE LChristophe68)
 * Version: 1.0
 * Date: 2022/02/12
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

#include "../../../inc/MarlinConfigPre.h"
#include "../../../core/types.h"


#if ENABLED(EXTJYERSUI)

#include "dwin.h"
#include "../../../gcode/gcode.h"

#include "extjyersui.h"


#if ENABLED(NOZZLE_PARK_FEATURE)
    void ExtJyersuiClass::C125() {
        if (!parser.seen_any()) return CrealityDWIN.DWIN_CError();

        xyz_int_t park_value = { HMI_datas.Park_point.x, HMI_datas.Park_point.y, HMI_datas.Park_point.z };
        if (parser.seenval('X')) {
            park_value.x = static_cast<int>(parser.value_linear_units());
            if (!WITHIN(park_value.x, 0, X_MAX_POS)) return CrealityDWIN.DWIN_CError();
            }
        if (parser.seenval('Y')) {
            park_value.y = static_cast<int>(parser.value_linear_units());
            if (!WITHIN(park_value.y, 0, Y_MAX_POS)) return CrealityDWIN.DWIN_CError();
            }
        if (parser.seenval('Z')) {
            park_value.z = static_cast<int>(parser.value_linear_units());
            if (!WITHIN(park_value.z, MIN_PARK_POINT_Z, Z_MAX_POS)) return CrealityDWIN.DWIN_CError();
            }
        
        HMI_datas.Park_point = park_value;
        }    
#endif  // Set park position

void ExtJyersuiClass::C562() {
    if (!parser.seen_any()) return CrealityDWIN.DWIN_CError();
        if (parser.seenval('E')) {
            HMI_datas.invert_dir_extruder = parser.value_bool() ? !HMI_datas.invert_dir_extruder : HMI_datas.invert_dir_extruder;
            CrealityDWIN.DWIN_Invert_Extruder();
        }
    }      // Invert Extruder

#if HAS_BED_PROBE
    void ExtJyersuiClass::C851() {
        if (!parser.seen_any()) return CrealityDWIN.DWIN_CError();
        float margin_parser = HMI_datas.probing_margin;
        uint16_t z_fast_feedrate_parser = HMI_datas.zprobefeedfast ;
        uint16_t z_slow_feedrate_parser = HMI_datas.zprobefeedslow;

        if (parser.seenval('M')) {
            margin_parser = parser.value_linear_units();
            if (!WITHIN(margin_parser, MIN_PROBE_MARGIN, MAX_PROBE_MARGIN)) return CrealityDWIN.DWIN_CError();
            }
        if (parser.seenval('F')) {
            z_fast_feedrate_parser = static_cast<int>(parser.value_linear_units());
            if (!WITHIN(z_fast_feedrate_parser, (MIN_Z_PROBE_FEEDRATE * 2), MAX_Z_PROBE_FEEDRATE)) return CrealityDWIN.DWIN_CError();
            }
        if (parser.seenval('S')) {
            z_slow_feedrate_parser = static_cast<int>(parser.value_linear_units());
            if  (!WITHIN(z_slow_feedrate_parser, MIN_Z_PROBE_FEEDRATE, MAX_Z_PROBE_FEEDRATE)) return CrealityDWIN.DWIN_CError();
            }
        HMI_datas.probing_margin = margin_parser;
        HMI_datas.zprobefeedfast = z_fast_feedrate_parser;
        HMI_datas.zprobefeedslow = z_slow_feedrate_parser;
    }      
#endif  // Set probing margin and z feed rate of the probe mesh leveling

#endif
