/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
#pragma once

/********************************************************************************
 * @file     lcd/e3v2/jyersui/dwin_lcd.h
 * @brief    DWIN screen control functions
 ********************************************************************************/

#include "../common/dwin_api.h"

// Draw a numeric value
//  bShow: true=display background color; false=don't display background color
//  zeroFill: true=zero fill; false=no zero fill
//  signedMode: 1=signed; 0=unsigned
//  zeroMode: 1=leading 0 displayed as 0; 0=leading 0 displayed as a space
//  size: Font size
//  color: Character color
//  bColor: Background color
//  iNum: Number of digits
//  fNum: Number of decimal digits
//  x/y: Upper-left coordinate
//  value: Integer value
void DWIN_Draw_Value(uint8_t bShow, bool signedMode, bool zeroFill, uint8_t zeroMode, uint8_t size, uint16_t color,
                          uint16_t bColor, uint8_t iNum, uint8_t fNum, uint16_t x, uint16_t y, int32_t value);
//  value: positive unscaled float value
void DWIN_Draw_Value(uint8_t bShow, bool signedMode, bool zeroFill, uint8_t zeroMode, uint8_t size, uint16_t color,
                          uint16_t bColor, uint8_t iNum, uint8_t fNum, uint16_t x, uint16_t y, float value);

// Draw a positive integer
//  bShow: true=display background color; false=don't display background color
//  zeroFill: true=zero fill; false=no zero fill
//  zeroMode: 1=leading 0 displayed as 0; 0=leading 0 displayed as a space
//  size: Font size
//  color: Character color
//  bColor: Background color
//  iNum: Number of digits
//  x/y: Upper-left coordinate
//  value: Integer value
inline void Draw_Int(uint8_t bShow, bool zeroFill, uint8_t zeroMode, uint8_t size, uint16_t color, uint16_t bColor, uint8_t iNum, uint16_t x, uint16_t y, long value) {
  DWIN_Draw_Value(bShow, 0, zeroFill, zeroMode, size, color, bColor, iNum, 0, x, y, value);
}

// Draw the degree (°) symbol
// Color: color
//  x/y: Upper-left coordinate of the first pixel
void DWIN_Draw_DegreeSymbol(uint16_t Color, uint16_t x, uint16_t y);

void DWIN_Save_JPEG_in_SRAM(uint8_t mem, uint8_t *data, uint16_t size, uint16_t dest_addr);

void DWIN_SRAM_Memory_Icon_Display(uint16_t x, uint16_t y, uint16_t source_addr);

// Write the contents of the 32KB SRAM data memory into the designated image memory space.
//  picID: Picture memory space location, 0x00-0x0F, each space is 32Kbytes
void DWIN_SRAMToPic(uint8_t picID);

// Draw an Icon With Background
//  icon: Icon library ID
//  ICON: Icon ID
//  x/y: Upper-left point
inline void DRAW_IconWB(uint8_t libicon, uint8_t icon, uint16_t x, uint16_t y) {
  DWIN_ICON_Show(true, false, false, libicon, icon, x, y);
}

// Draw an Icon With Transparent Background
//  icon: Icon library ID
//  ICON: Icon ID
//  x/y: Upper-left point
inline void DRAW_IconWTB(uint8_t libicon, uint8_t icon, uint16_t x, uint16_t y) {
  DWIN_ICON_Show(false, false, true, libicon, icon, x, y);
}

inline void DRAW_IconTH(uint16_t x, uint16_t y, uint16_t source_addr) {
  DWIN_ICON_Show(true, false, false, x, y, source_addr);
}
