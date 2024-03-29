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

/*---------------------------------------- Numeric related functions ----------------------------------------*/

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
                          uint16_t bColor, uint8_t iNum, uint8_t fNum, uint16_t x, uint16_t y, int32_t value) {
  size_t i = 0;
  DWIN_Byte(i, 0x14);
  // Bit 7: bshow
  // Bit 6: 1 = signed; 0 = unsigned number;
  // Bit 5: zeroFill
  // Bit 4: zeroMode
  // Bit 3-0: size
  DWIN_Byte(i, (bShow * 0x80) | (signedMode * 0x40) | (zeroFill * 0x20) | (zeroMode * 0x10) | size);
  DWIN_Word(i, color);
  DWIN_Word(i, bColor);
  DWIN_Byte(i, signedMode && (value >= 0) ? iNum + 1 : iNum);
  DWIN_Byte(i, fNum);
  DWIN_Word(i, x);
  DWIN_Word(i, y);
  // Write a big-endian 64 bit integer
  const size_t p = i + 1;
  for (char count = 8; count--;) { // 7..0
    ++i;
    DWIN_SendBuf[p + count] = value;
    value >>= 8;
  }
  DWIN_Send(i);
}

// Draw a numeric value
//  value: positive unscaled float value
void DWIN_Draw_Value(uint8_t bShow, bool signedMode, bool zeroFill, uint8_t zeroMode, uint8_t size, uint16_t color,
                          uint16_t bColor, uint8_t iNum, uint8_t fNum, uint16_t x, uint16_t y, float value) {
  const int32_t val = round(value * POW(10, fNum));
  DWIN_Draw_Value(bShow, signedMode, zeroFill, zeroMode, size, color, bColor, iNum, fNum, x, y, val);
}


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

// Draw an Icon with transparent background
//  libID: Icon library ID
//  picID: Icon ID
//  x/y: Upper-left point
void DWIN_ICON_Show(uint8_t libID, uint8_t picID, uint16_t x, uint16_t y) {
  DWIN_ICON_Show(true, false, false, libID, picID, x, y);
}

// From DWIN Enhanced implementation for PRO UI v3.10.1
// Write buffer data to the SRAM or Flash
//  mem: 0x5A=32KB SRAM, 0xA5=16KB Flash
//  addr: start address
//  length: Bytes to write
//  data: address of the buffer with data
void DWIN_WriteToMem(uint8_t mem, uint16_t addr, uint16_t length, uint8_t *data) {
  const uint8_t max_size = 128;
  uint16_t pending = length;
  uint16_t to_send;
  uint16_t indx;
  uint8_t block = 0;

  while (pending > 0) {
    indx = block * max_size;
    to_send = _MIN(pending, max_size);
    size_t i = 0;
    DWIN_Byte(i, 0x31);
    DWIN_Byte(i, mem);
    DWIN_Word(i, addr + indx); // start address of the data block
    ++i;
    LOOP_L_N(j, i) { LCD_SERIAL.write(DWIN_SendBuf[j]); delayMicroseconds(1); }  // Buf header
    for (uint16_t j = indx; j <= indx + to_send - 1; j++) LCD_SERIAL.write(*(data + j)); delayMicroseconds(1);  // write block of data
    LOOP_L_N(j, 4) { LCD_SERIAL.write(DWIN_BufTail[j]); delayMicroseconds(1); }
    block++;
    pending -= to_send;
  }
}

// Write buffer data to the SRAM or Flash
//  mem: 0x5A=32KB SRAM, 0xA5=16KB Flash -> to be fixed for ender3 S1
//  dest_addr: start address
//  size: Bytes to write
//  data: address of the buffer with data
void DWIN_Save_JPEG_in_SRAM(uint8_t mem, uint8_t *data, uint16_t size, uint16_t dest_addr) {
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
    DWIN_Byte(i, mem);
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

// Write the contents of the 32KB SRAM data memory into the designated image memory space.
//  picID: Picture memory space location, 0x00-0x0F, each space is 32Kbytes
void DWIN_SRAMToPic(uint8_t picID) {
  size_t i = 0;
  DWIN_Byte(i, 0x33);
  DWIN_Byte(i, 0x5A);
  DWIN_Byte(i, 0xA5);
  DWIN_Byte(i, picID);
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
