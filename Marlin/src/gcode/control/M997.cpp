/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../gcode.h"

#if ENABLED(PLATFORM_M997_SUPPORT)

#if ENABLED(DWIN_CREALITY_LCD_ENHANCED)
  #include "../../lcd/e3v2/proui/dwin.h"
#elif ENABLED(DWIN_CREALITY_LCD_JYERSUI)
  #include "../../lcd/e3v2/jyersui/dwin.h"
#endif

/**
 * M997: Perform in-application firmware update
 */
void GcodeSuite::M997() {

  TERN_(DWIN_CREALITY_LCD_ENHANCED, DWIN_RebootScreen());
  TERN_(DWIN_CREALITY_LCD_JYERSUI, CrealityDWIN.DWIN_RebootScreen());

  flashFirmware(parser.intval('S'));

}

#endif
