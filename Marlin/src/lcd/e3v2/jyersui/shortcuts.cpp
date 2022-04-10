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


#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_CREALITY_LCD_JYERSUI)

#include "../../../core/types.h"
#include "dwin_lcd.h"
#include "jyersui.h"
#include "dwin.h"
#include "shortcuts.h"
#include "../../../module/planner.h"

ShortcutsClass shortcuts;

bool ShortcutsClass::quitmenu = false;
bool ShortcutsClass::signedMode = false;
int16_t ShortcutsClass::rel_value = 0;
int16_t ShortcutsClass::control_value = 0;

void ShortcutsClass::initZ() {
  quitmenu = false;
  rel_value = 0;
  draw_moveZ();
}

void ShortcutsClass::draw_moveZ() {
  CrealityDWINClass::Clear_Screen(1);
  CrealityDWINClass::Draw_Title(GET_TEXT(MSG_MOVE_Z));
  JYERSUI::ClearMenuArea();
  DWIN_Draw_Rectangle(0, CrealityDWINClass::GetColor(HMI_datas.highlight_box, Color_White), 13, 59, 259, 351);
  DRAW_IconWB(ICON ,ICON_LOGO, 71, 120);  // CREALITY logo
  DWIN_Draw_Rectangle(1, Confirm_Color, 87, 280, 186, 317);
  DWIN_Draw_Rectangle(0, Color_White, 86, 279, 187, 318);
  DWIN_Draw_Rectangle(0, Color_White, 85, 278, 188, 319);
  char str[20];
  sprintf_P(str, PSTR(" %s "), GET_TEXT(MSG_BUTTON_CANCEL));
  DWIN_Draw_String(false, DWIN_FONT_STAT, Color_White, JYERSUI::backcolor, 96, 290, F(str));
  const uint8_t LM = 40;
  JYERSUI::cursor.x = LM;
  JYERSUI::cursor.y = 200;
  char nstr[6];
  sprintf_P(str, PSTR(" %s%s "), (rel_value >= 0)? "+" : "-" ,dtostrf(abs(rel_value), 4, 0, nstr));
  if WITHIN(current_position.z, 0, Z_MAX_POS) planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
  DWIN_Draw_String(false, DWIN_FONT_STAT, Color_White, JYERSUI::backcolor, LM, JYERSUI::cursor.y, GET_TEXT_F(MSG_LIVEMOVE_Z));
  DWIN_Draw_String(true, DWIN_FONT_STAT, Color_White, JYERSUI::backcolor, LM + (STAT_CHR_W * strlen(GET_TEXT(MSG_LIVEMOVE_Z))), JYERSUI::cursor.y, F(str));
  
  DWIN_UpdateLCD();
}

void ShortcutsClass::onEncoderZ(EncoderState encoder_diffState) {
  switch (encoder_diffState) {
    case ENCODER_DIFF_CW:
      control_value = current_position.z + EncoderRate.encoderMoveValue;    
      if (control_value <= (Z_MAX_POS - 5)) {
        rel_value += EncoderRate.encoderMoveValue;
        current_position.z += EncoderRate.encoderMoveValue; 
      } 
      break;
    case ENCODER_DIFF_CCW:
      control_value = current_position.z - EncoderRate.encoderMoveValue;
      if (control_value >= 0) {
        rel_value -= EncoderRate.encoderMoveValue;
        current_position.z -= EncoderRate.encoderMoveValue;
      } 
      break;
    case ENCODER_DIFF_ENTER: quitmenu = true; break;
    default: break;
  }
  const uint8_t LM = 40;
  JYERSUI::cursor.x = LM;
  JYERSUI::cursor.y = 200;
  char nstr[7];
  char str[10];
  sprintf_P(str, PSTR(" %s%s "), (rel_value >= 0)? "+" : "-" ,dtostrf(abs(rel_value), 4, 0, nstr));
  DWIN_Draw_String(true, DWIN_FONT_HEAD, Color_White, JYERSUI::backcolor, LM + (STAT_CHR_W * strlen(GET_TEXT(MSG_LIVEMOVE_Z))), JYERSUI::cursor.y, str);
  planner.synchronize();
  planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
  planner.synchronize();
  
  DWIN_UpdateLCD();
}

#endif
