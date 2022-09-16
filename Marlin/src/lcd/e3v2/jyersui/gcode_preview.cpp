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
#include "../../../gcode/gcode.h"
#include "dwin_lcd.h"
#include "dwinui.h"
#include "dwin.h"
#include "base64.hpp"
#include "gcode_preview.h"

#include <map>
#include <string>
using namespace std;

std::map<string, int> image_cache;

bool GcodePreviewClass::find_and_decode_gcode_preview(char *name, uint8_t preview_type, uint16_t *address, bool onlyCachedFileIcon/*=false*/) {
  // Won't work if we don't copy the name
  // for (char *c = &name[0]; *c; c++) *c = tolower(*c);

  char file_name[strlen(name) + 1]; // Room for filename and null
  sprintf_P(file_name, PSTR("%s"), name);
  char file_path[strlen(name) + 1 + MAXPATHNAMELENGTH]; // Room for path, filename and null
  sprintf_P(file_path, PSTR("%s/%s"), card.getWorkDirName(), file_name);

  // Check if cached
  bool use_cache = preview_type == Thumnail_Icon;
  if (use_cache) {
    auto it = image_cache.find(file_path+to_string(Thumnail_Icon));
    if (it != image_cache.end()) { // already cached
      if (it->second == 0) return false; // no image available
      *address = it->second;
      return true;
    } else if (onlyCachedFileIcon) return false;
  }

  const uint16_t buff_size = 256;
  char public_buf[buff_size+1];
  //uint8_t output_buffer[6144];
  uint8_t output_buffer[8192];
  uint32_t position_in_file = 0;
  char *encoded_image = NULL;

  card.openFileRead(file_name);
  uint8_t n_reads = 0;
  int16_t data_read = card.read(public_buf, buff_size);
  card.setIndex(card.getIndex()+data_read);
  char key[31] = "";
  switch (preview_type) {
    case Thumnail_Icon: strcpy_P(key, PSTR("; jpeg thumbnail begin 50x50")); break;
    case Thumnail_Preview: strcpy_P(key, PSTR("; jpeg thumbnail begin 180x180")); break;
  }
  while(n_reads < 16 && data_read) { // Max 16 passes so we don't loop forever
  if (Encoder_ReceiveAnalyze() != ENCODER_DIFF_NO) return false;
    encoded_image = strstr(public_buf, key);
    if (encoded_image) {
      uint32_t index_bw = &public_buf[buff_size] - encoded_image;
      position_in_file = card.getIndex() - index_bw;
      break;
    }

    card.setIndex(card.getIndex()-32);
    data_read = card.read(public_buf, buff_size);
    card.setIndex(card.getIndex()+data_read);

    n_reads++;
  }

  // If we found the image, decode it
  if (encoded_image) {
  memset(public_buf, 0, sizeof(public_buf));
  card.setIndex(position_in_file+23); // ; jpeg thumbnail begin <move here>180x180 99999
  while (card.get() != ' '); // ; jpeg thumbnail begin 180x180 <move here>180x180

  char size_buf[10];
  for (size_t i = 0; i < sizeof(size_buf); i++)
  {
    uint8_t c = card.get();
    if (ISEOL(c)) {
      size_buf[i] = 0;
      break;
    }
    else
      size_buf[i] = c;
  }
  uint16_t image_size = atoi(size_buf);
  uint16_t stored_in_buffer = 0;
  uint8_t encoded_image_data[image_size+1];
  while (stored_in_buffer < image_size) {
    char c = card.get();
    if (ISEOL(c) || c == ';' || c == ' ') {
      continue;
    }
    else {
      encoded_image_data[stored_in_buffer] = c;
      stored_in_buffer++;
    }
  }

  encoded_image_data[stored_in_buffer] = 0;
  unsigned int output_size = decode_base64(encoded_image_data, output_buffer);
  if (next_available_address + output_size >= 0x7530) { // cache is full, invalidate it
    next_available_address = 0;
    image_cache.clear();
    SERIAL_ECHOLNPGM("Preview cache full, cleaning up...");
  }
  DWIN_Save_JPEG_in_SRAM(0x5a, (uint8_t *)output_buffer, output_size, next_available_address);
  *address = next_available_address;
  if(use_cache) {
    image_cache[file_path+to_string(preview_type)] = next_available_address;
    next_available_address += output_size + 1;
  }
  } else if (use_cache)  // If we didn't find the image, but we are using the cache, mark it as image not available
  {
    image_cache[file_path+to_string(preview_type)] = 0;
  }

  card.closefile(); 
  gcode.process_subcommands_now(F("M117")); // Clear the message sent by the card API
  return encoded_image;
}

bool GcodePreviewClass::find_and_decode_gcode_header(char *name, uint8_t header_type) {
  char file_name[strlen(name) + 1]; // Room for filename and null
  sprintf_P(file_name, PSTR("%s"), name);
  char file_path[strlen(name) + 1 + MAXPATHNAMELENGTH]; // Room for path, filename and null
  sprintf_P(file_path, PSTR("%s/%s"), card.getWorkDirName(), file_name);

  const uint16_t buff_size = 256;
  char public_buf[buff_size+1];
  uint32_t position_in_file = 0;
  char *encoded_header = NULL;

  card.openFileRead(file_name);
  uint8_t n_reads = 0;
  int16_t data_read = card.read(public_buf, buff_size);
  card.setIndex(card.getIndex()+data_read);
  char key[16] = "";
  switch (header_type) {
    case Header_Time: strcpy_P(key, PSTR(";TIME")); break;
    case Header_Filament: strcpy_P(key, PSTR(";Filament used")); break;
    case Header_Layer: strcpy_P(key, PSTR(";Layer height")); break;
  }
  while(n_reads < 16 && data_read) { // Max 16 passes so we don't loop forever
  if (Encoder_ReceiveAnalyze() != ENCODER_DIFF_NO) return false;
    encoded_header = strstr(public_buf, key);
    if (encoded_header) {
      uint32_t index_bw = &public_buf[buff_size] - encoded_header;
      position_in_file = card.getIndex() - index_bw;
      break;
    }

    card.setIndex(card.getIndex()-17);
    data_read = card.read(public_buf, buff_size);
    card.setIndex(card.getIndex()+data_read);

    n_reads++;
  }

  if (encoded_header) {
    switch (header_type) {
      case Header_Time: card.setIndex(position_in_file+5); break;
      case Header_Filament: card.setIndex(position_in_file+14); break;
      case Header_Layer: card.setIndex(position_in_file+13); break;
    }
    while (card.get() != ':'); // ; jpeg thumbnail begin 180x180 <move here>180x180
      
    char out_buf[12];
    uint8_t stored_in_buffer = 0;
    for (int i = 0; i < 12; i++)
    {
      char c = card.get();
      if (ISEOL(c) || c == ';') {
        break;
      }
      else {
        out_buf[stored_in_buffer] = c;
        stored_in_buffer++;
      }
    }
    sprintf_P(buf, PSTR("%s"), out_buf);
  }
  card.closefile();
return encoded_header;
}

void GcodePreviewClass::Preview_Reset() {
      image_cache.clear();
}

GcodePreviewClass GcodePreview;

#endif // HAS_GCODE_PREVIEW
#endif // DWIN_LCD_PROUI
