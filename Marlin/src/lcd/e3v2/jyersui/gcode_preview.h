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
 */

#pragma once


class GcodePreviewClass {
  public:
    void Preview_Reset();
    bool find_and_decode_gcode_preview(char *name, uint8_t preview_type, uint16_t *address, bool onlyCachedFileIcon = false);
    bool find_and_decode_gcode_header(char *name, uint8_t header_type);
    bool file_preview;
    uint16_t file_preview_image_address;
  private:
    uint16_t next_available_address;
};

extern GcodePreviewClass GcodePreview;