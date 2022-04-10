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

/**
 *
 * Based on the original code provided by Creality under GPL
 */

#include "../../../core/types.h"
#include "../common/encoder.h"

#include <stdint.h>

class ShortcutsClass {
private:
  static int16_t rel_value;
  static int16_t control_value;
public:
  static bool quitmenu;
  static bool signedMode;
  static void initZ();
  static void onEncoderZ(EncoderState encoder_diffState);
  static void draw_moveZ();
  static bool isQuitedZ() { return quitmenu; }
};

extern ShortcutsClass shortcuts;
