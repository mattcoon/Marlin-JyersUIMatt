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

/**
 * Mesh Viewer for PRO UI
 * Author: Miguel A. Risco-Castillo (MRISCOC)
 * version: 3.14.1
 * Date: 2022/04/11
 */

#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_CREALITY_LCD_JYERSUI,HAS_MESH)

#include "../../../core/types.h"
#include "../../marlinui.h"
#include "dwin_lcd.h"
#include "dwinui.h"
#include "dwin.h"
// #include "dwin_popup.h"
#include "../../../feature/bedlevel/bedlevel.h"
#include "../../../module/probe.h"
#include "meshviewer.h"

#if ENABLED(AUTO_BED_LEVELING_UBL)
  // #include "bedlevel_tools.h"
#endif

MeshViewerClass MeshViewer;

void MeshViewerClass::DrawMesh(bed_mesh_t zval, const uint8_t sizex, const uint8_t sizey) {

  const int8_t mx = 50, my = 50;  // Margins
  const int16_t stx = (DWIN_WIDTH - 2 * mx) / (sizex - 1),  // Steps
                sty = (DWIN_WIDTH - 2 * my) / (sizey - 1);
  const int8_t rmax = _MIN(mx - 2, stx / 2);
  const int8_t rmin = 7;
  int16_t zmesh[sizex][sizey];
  #define px(xp) (mx + (xp) * stx)
  #define py(yp) (30 + DWIN_WIDTH - my - (yp) * sty)
  #define rm(z) ((z - minz) * (rmax - rmin) / _MAX(1, (maxz - minz)) + rmin)
  #define DrawMeshValue(xp, yp, zv) DWINUI::Draw_Signed_Float(font6x12, 1, 2, px(xp) - 18, py(yp) - 6, zv)
  #define DrawMeshHLine(yp) DWIN_Draw_HLine(Line_Color, px(0), py(yp), DWIN_WIDTH - 2 * mx)
  #define DrawMeshVLine(xp) DWIN_Draw_VLine(Line_Color, px(xp), py(sizey - 1), DWIN_WIDTH - 2 * my)
  int16_t maxz =-32000; int16_t minz = 32000;
  LOOP_L_N(y, sizey) LOOP_L_N(x, sizex) {
    const float v = isnan(zval[x][y]) ? 0 : round(zval[x][y] * 100);
    zmesh[x][y] = v;
    NOLESS(maxz, v);
    NOMORE(minz, v);
  }
  max = (float)maxz / 100;
  min = (float)minz / 100;
  CrealityDWINClass::Clear_Screen(1);
  CrealityDWINClass::Draw_Title(F("Tramming"));
  DWINUI::ClearMainArea();
  // DWIN_Draw_Rectangle(0, Color_White, 14, 60, 258, 330);
  // DWINUI::Draw_Button(BTN_Continue, 86, 280);
  DWIN_Draw_Rectangle(0, Line_Color, px(0), py(0), px(sizex - 1), py(sizey - 1));
  LOOP_S_L_N(x, 1, sizex - 1) DrawMeshVLine(x);
  LOOP_S_L_N(y, 1, sizey - 1) DrawMeshHLine(y);
  LOOP_L_N(y, sizey) {
    hal.watchdog_refresh();
    LOOP_L_N(x, sizex) {
      uint16_t color = DWINUI::RainbowInt(zmesh[x][y], _MIN(-5, minz), _MAX(5, maxz));
      uint8_t radius = rm(zmesh[x][y]);
      DWINUI::Draw_FillCircle(color, px(x), py(y), radius);
      if (sizex < 9) {
        if (zmesh[x][y] == 0) DWINUI::Draw_Float(font6x12, 1, 2, px(x) - 12, py(y) - 6, 0);
        else DWINUI::Draw_Signed_Float(font6x12, 1, 2, px(x) - 18, py(y) - 6, zval[x][y]);
      }
      else {
        char str_1[9];
        str_1[0] = 0;
        switch (zmesh[x][y]) {
          case -999 ... -100:
            DWINUI::Draw_Signed_Float(font6x12, 1, 1, px(x) - 18, py(y) - 6, zval[x][y]);
            break;
          case -99 ... -1:
            sprintf_P(str_1, PSTR("-.%02i"), -zmesh[x][y]);
            break;
          case 0:
            DWIN_Draw_String(false, font6x12, DWINUI::textcolor, DWINUI::backcolor, px(x) - 4, py(y) - 6, "0");
            break;
          case 1 ... 99:
            sprintf_P(str_1, PSTR(".%02i"), zmesh[x][y]);
            break;
          case 100 ... 999:
            DWINUI::Draw_Signed_Float(font6x12, 1, 1, px(x) - 18, py(y) - 6, zval[x][y]);
            break;
        }
        if (str_1[0])
          DWIN_Draw_String(false, font6x12, DWINUI::textcolor, DWINUI::backcolor, px(x) - 12, py(y) - 6, str_1);
      }
    }
  }
  Update();
}

void MeshViewerClass::Update() {
  DWIN_UpdateLCD();
}

void TrammingWizard() {
  if (!eeprom_settings.FullManualTramming) {
    CrealityDWIN.Popup_Handler(Level);
    // gcode.home_all_axes(true);
    do_z_clearance(Z_HOMING_HEIGHT);
    temp_val.corner_avg = 0;

    MeshViewer.Zval[0][0] = probe.probe_at_point(PROBE_X_MIN, PROBE_Y_MIN, PROBE_PT_RAISE, 0, false);
    const char * MSG_UNREACHABLE = GET_TEXT(MSG_ZPROBE_UNREACHABLE);
    if (isnan(MeshViewer.Zval[0][0])) {
      CrealityDWIN.Update_Status(MSG_UNREACHABLE);
      CrealityDWIN.Redraw_Menu();
    }
    MeshViewer.DrawMesh(MeshViewer.Zval, 2, 2);
    temp_val.corner_avg += MeshViewer.Zval[0][0];

    MeshViewer.Zval[0][1] = probe.probe_at_point(PROBE_X_MIN, PROBE_Y_MAX, PROBE_PT_RAISE, 0, false);
    if (isnan(MeshViewer.Zval[0][1])) {
      CrealityDWIN.Update_Status(MSG_UNREACHABLE);
      CrealityDWIN.Redraw_Menu();
    }
    MeshViewer.DrawMesh(MeshViewer.Zval, 2, 2);
    temp_val.corner_avg += MeshViewer.Zval[0][1];

    MeshViewer.Zval[1][1] = probe.probe_at_point(PROBE_X_MAX, PROBE_Y_MAX, PROBE_PT_RAISE, 0, false);
    if (isnan(MeshViewer.Zval[1][1])) {
      CrealityDWIN.Update_Status(MSG_UNREACHABLE);
      CrealityDWIN.Redraw_Menu();
    }
    temp_val.corner_avg += MeshViewer.Zval[1][1];
    MeshViewer.DrawMesh(MeshViewer.Zval, 2, 2);

    MeshViewer.Zval[1][0] = probe.probe_at_point(PROBE_X_MAX, PROBE_Y_MIN, PROBE_PT_STOW, 0, false);
    if (isnan(MeshViewer.Zval[1][0])) {
      CrealityDWIN.Update_Status(MSG_UNREACHABLE);
      CrealityDWIN.Redraw_Menu();
    }
    MeshViewer.DrawMesh(MeshViewer.Zval, 2, 2);
    temp_val.corner_avg += MeshViewer.Zval[1][0];
    temp_val.corner_avg /= 4;
    // update grid with corner offsets to average height
    LOOP_L_N(x, 2) LOOP_L_N(y, 2) MeshViewer.Zval[x][y] -= temp_val.corner_avg;
    MeshViewer.DrawMesh(MeshViewer.Zval, 2, 2);
    if (MeshViewer.CornerTolerance() < 0.05) {
      DWINUI::Draw_CenteredString(140,F("Corners leveled"));
      DWINUI::Draw_CenteredString(160,F("Tolerance achieved!"));
    }
    else {
      const float threads_factor[] = { 0.5, 0.7, 0.8 };
      const uint8_t screw_thread = TRAMMING_SCREW_THREAD;
      uint8_t p = 0;
      float max = 0;
      FSTR_P plabel;
      bool s = true;
      float adjust = 0;
      LOOP_L_N(x, 2) LOOP_L_N(y, 2) {
        const float d = ABS(MeshViewer.Zval[x][y]);
        if (max < d) {
          s = (MeshViewer.Zval[x][y] >= 0);
          max = d;
          p = x + 2 * y;
          adjust = ABS(d) < 0.001f ? 0 : d / threads_factor[(screw_thread - 30) / 10];
        }
      }
      const int full_turns = trunc(adjust);
      const float decimal_part = adjust - float(full_turns);
      const int minutes = trunc(decimal_part * 60.0f);
      switch (p) {
        case 0b00 : plabel = GET_TEXT_F(MSG_LEVBED_FL); break;
        case 0b01 : plabel = GET_TEXT_F(MSG_LEVBED_FR); break;
        case 0b10 : plabel = GET_TEXT_F(MSG_LEVBED_BL); break;
        case 0b11 : plabel = GET_TEXT_F(MSG_LEVBED_BR); break;
        default   : plabel = F(""); break;
      }

      DWINUI::Draw_CenteredString(120, F("Corners not leveled. Turn"));
      DWINUI::Draw_CenteredString(140,plabel);
      DWINUI::Draw_Float(1,3,DWIN_WIDTH/2,160,adjust);
      DWINUI::Draw_CenteredString(180, (s != (screw_thread&1)) ? F("Turns CCW") : F("Turns CW")); 
    }
    wait_for_user = false;
    CrealityDWIN.Popup_Handler(MeshviewPopup);
  }

}

#endif // DWIN_LCD_PROUI && HAS_MESH
