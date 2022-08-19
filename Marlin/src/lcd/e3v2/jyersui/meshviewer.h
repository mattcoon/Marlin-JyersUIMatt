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

#define PROBE_X_MIN _MAX(0 + temp_val.corner_pos, X_MIN_POS + probe.offset.x, X_MIN_POS + PROBING_MARGIN) - probe.offset.x
#define PROBE_X_MAX _MIN((X_BED_SIZE + X_MIN_POS) - temp_val.corner_pos, X_MAX_POS + probe.offset.x, X_MAX_POS - PROBING_MARGIN) - probe.offset.x
#define PROBE_Y_MIN _MAX(0 + temp_val.corner_pos, Y_MIN_POS + probe.offset.y, Y_MIN_POS + PROBING_MARGIN) - probe.offset.y
#define PROBE_Y_MAX _MIN((Y_BED_SIZE + Y_MIN_POS) - temp_val.corner_pos, Y_MAX_POS + probe.offset.y, Y_MAX_POS - PROBING_MARGIN) - probe.offset.y

#define PROBEFL 0
#define PROBERL 1
#define PROBERR 2
#define PROBEFR 3

/**
 * Mesh Viewer for PRO UI
 * Author: Miguel A. Risco-Castillo (MRISCOC)
 * version: 3.14.1
 * Date: 2022/04/11
 */

class MeshViewerClass {
  public:
    void DrawMesh(bed_mesh_t zval, const uint8_t sizex, const uint8_t sizey);
    void Update();
    inline float CornerTolerance() { return ABS(max - min); }
    bed_mesh_t Zval;
  private:
    float max, min;
};

extern MeshViewerClass MeshViewer;

void TrammingWizard();
