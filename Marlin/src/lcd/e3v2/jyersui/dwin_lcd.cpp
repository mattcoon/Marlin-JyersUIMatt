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

/********************************************************************************
 * @file     lcd/e3v2/jyersui/dwin_lcd.cpp
 * @brief    DWIN screen control functions
 ********************************************************************************/

#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_CREALITY_LCD_JYERSUI)

#include "dwin_lcd.h"

/*-------------------------------------- System variable function --------------------------------------*/

void DWIN_Startup() {}

/*---------------------------------------- Drawing functions ----------------------------------------*/

// Draw the degree (°) symbol
// Color: color
//  x/y: Upper-left coordinate of the first pixel
void DWIN_Draw_DegreeSymbol(uint16_t Color, uint16_t x, uint16_t y) {
  DWIN_Draw_Point(Color, 1, 1, x + 1, y);
  DWIN_Draw_Point(Color, 1, 1, x + 2, y);
  DWIN_Draw_Point(Color, 1, 1, x, y + 1);
  DWIN_Draw_Point(Color, 1, 1, x + 3, y + 1);
  DWIN_Draw_Point(Color, 1, 1, x, y + 2);
  DWIN_Draw_Point(Color, 1, 1, x + 3, y + 2);
  DWIN_Draw_Point(Color, 1, 1, x + 1, y + 3);
  DWIN_Draw_Point(Color, 1, 1, x + 2, y + 3);
}

/*---------------------------------------- Picture related functions ----------------------------------------*/

// Draw an Icon
//  libID: Icon library ID
//  picID: Icon ID
//  x/y: Upper-left point
void DWIN_ICON_Show(uint8_t libID, uint8_t picID, uint16_t x, uint16_t y) {
  DWIN_ICON_Show(true, false, false, libID, picID, x, y);
}

void DWIN_Save_JPEG_in_SRAM(uint8_t *data, uint16_t size, uint16_t dest_addr) {
  const uint8_t max_data_size = 128;
  uint16_t pending_data = size;

  uint8_t iter = 0;

  while (pending_data > 0) {
    uint16_t data_to_send = max_data_size;
    if (pending_data - max_data_size <= 0)
        data_to_send = pending_data;

    uint16_t from_i = iter * max_data_size;
    uint16_t to_i = from_i + data_to_send - 1;

    size_t i = 0;
    DWIN_Byte(i, 0x31);
    DWIN_Byte(i, 0x5A);
    DWIN_Word(i, dest_addr+(iter * max_data_size));
    ++i;
    LOOP_L_N(n, i) { LCD_SERIAL.write(DWIN_SendBuf[n]); delayMicroseconds(1); }
    for (uint16_t n=from_i; n<=to_i; n++) { LCD_SERIAL.write(*(data + n)); delayMicroseconds(1);}
    LOOP_L_N(n, 4) { LCD_SERIAL.write(DWIN_BufTail[n]); delayMicroseconds(1); }
    pending_data -= data_to_send;
    iter++;
  } 
}

void DWIN_SRAM_Memory_Icon_Display(uint16_t x, uint16_t y, uint16_t source_addr) {
  size_t i = 0;
  DWIN_Byte(i, 0x24);
  NOMORE(x, DWIN_WIDTH - 1);
  NOMORE(y, DWIN_HEIGHT - 1); // -- ozy -- srl
  DWIN_Word(i, x);
  DWIN_Word(i, y);
  DWIN_Byte(i, 0x80);
  DWIN_Word(i, source_addr);
  DWIN_Send(i);
}

/*---------------------------------------- Memory functions ----------------------------------------*/
// The LCD has an additional 32KB SRAM and 16KB Flash

// Data can be written to the sram and save to one of the jpeg page files

// Write Data Memory
//  command 0x31
//  Type: Write memory selection; 0x5A=SRAM; 0xA5=Flash
//  Address: Write data memory address; 0x000-0x7FFF for SRAM; 0x000-0x3FFF for Flash
//  Data: data
//
//  Flash writing returns 0xA5 0x4F 0x4B

// Read Data Memory
//  command 0x32
//  Type: Read memory selection; 0x5A=SRAM; 0xA5=Flash
//  Address: Read data memory address; 0x000-0x7FFF for SRAM; 0x000-0x3FFF for Flash
//  Length: leangth of data to read; 0x01-0xF0
//
//  Response:
//    Type, Address, Length, Data

// Write Picture Memory
//  Write the contents of the 32KB SRAM data memory into the designated image memory space
//  Issued: 0x5A, 0xA5, PIC_ID
//  Response: 0xA5 0x4F 0x4B
//
//  command 0x33
//  0x5A, 0xA5
//  PicId: Picture Memory location, 0x00-0x0F
//
//  Flash writing returns 0xA5 0x4F 0x4B


#endif // DWIN_CREALITY_LCD_JYERSUI
