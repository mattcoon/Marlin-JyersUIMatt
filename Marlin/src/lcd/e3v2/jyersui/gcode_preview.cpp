/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2022 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

/**
 * DWIN G-code thumbnail preview
 * Author: Miguel A. Risco-Castillo
 * version: 2.1
 * Date: 2021/06/19
 *
 * Modded for JYERSUI by LCH-77
 */

#include "../../../inc/MarlinConfigPre.h"
#if ENABLED(DWIN_CREALITY_LCD_JYERSUI)

#include "dwin_defines.h"

#if HAS_GCODE_PREVIEW

#include "../../../core/types.h"
#include "../../marlinui.h"
#include "../../../sd/cardreader.h"
#include "../../../MarlinCore.h" // for wait_for_user
#include "dwin_lcd.h"
#include "dwinui.h"
#include "dwin.h"
#include "base64.hpp"
#include "gcode_preview.h"


void GcodePreviewClass::Get_Value(char *buf, const char * const key, float &value) {
}

bool GcodePreviewClass::Has_Preview() {
  return false;
}

void GcodePreviewClass::Preview_DrawFromSD() {
}

bool GcodePreviewClass::Preview_Valid() {
  return false; 
}

void GcodePreviewClass::Preview_Reset() {
}

GcodePreviewClass GcodePreview;

#endif // HAS_GCODE_PREVIEW
#endif // DWIN_LCD_PROUI
