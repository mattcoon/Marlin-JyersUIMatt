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
#pragma once

/**
 * DWIN Single var plot
 * Author: Miguel A. Risco-Castillo
 * Version: 1.0
 * Date: 2022/01/30
 *
 * Modded for JYERSUI by LCH-77
 */

#include "dwinui.h"

class PlotClass {
public:
  void Draw(frame_rect_t frame, float max, float ref = 0);
  void Update(float value);
private:
  frame_rect_t grphframe;
  float scale;
  uint16_t grphpoints, r, x2, y2;
};

extern PlotClass Plot;
