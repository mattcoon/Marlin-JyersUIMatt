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
 * DWIN End Stops diagnostic page
 * Author: Miguel A. Risco-Castillo
 * Version: 1.0
 * Date: 2021/11/06
 *
 * Modded for JYERSUI by LCH-77
 */

class ESDiagClass {
public:
  void Draw();
  void Update();
private:
  // void draw_es_label(FSTR_P const flabel=nullptr);
  void draw_es_state(const bool is_hit);
  void draw_es_label(FSTR_P const flabel=nullptr);
};

extern ESDiagClass ESDiag;
