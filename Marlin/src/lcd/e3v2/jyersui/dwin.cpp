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
 * lcd/e3v2/jyersui/dwin.cpp
 */

#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_CREALITY_LCD_JYERSUI)

  #include "dwin.h"

  #if EXTJYERSUI
    #include "extjyersui.h"
  #endif

  #include "screenlock.h"

  #include "../../marlinui.h"
  #include "../../../MarlinCore.h"

  #include "../../../gcode/gcode.h"
  #include "../../../module/temperature.h"
  #include "../../../module/planner.h"
  #include "../../../module/settings.h"
  #include "../../../libs/buzzer.h"
  #include "../../../inc/Conditionals_post.h"

  //#define DEBUG_OUT 1
  #include "../../../core/debug_out.h"

  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    #include "../../../feature/pause.h"
  #endif

  #if HAS_FILAMENT_SENSOR
    #include "../../../feature/runout.h"
    float editable_distance;
  #endif

  #if ENABLED(HOST_ACTION_COMMANDS)
    #include "../../../feature/host_actions.h"
  #endif

  #include "screenlock.h"

  #if ANY(BABYSTEPPING, HAS_BED_PROBE, HAS_WORKSPACE_OFFSET)
    #define HAS_ZOFFSET_ITEM 1
  #endif

  #ifndef strcasecmp_P
    #define strcasecmp_P(a, b) strcasecmp((a), (b))
  #endif

  #ifdef BLTOUCH_HS_MODE
    #include "../../../feature/bltouch.h"
  #endif

  #if HAS_LEVELING
    #include "../../../feature/bedlevel/bedlevel.h"
  #endif

  #if ENABLED(AUTO_BED_LEVELING_UBL)
    #include "../../../libs/least_squares_fit.h"
    #include "../../../libs/vector_3.h"
  #endif

  #if HAS_BED_PROBE
    #include "../../../module/probe.h"
  #endif

  #if ENABLED(POWER_LOSS_RECOVERY)
    #include "../../../feature/powerloss.h"
  #endif

  #include "../../../module/stepper.h"

  #if HAS_ES_DIAG
    #include "diag_endstops.h"
  #endif

  #if HAS_SHORTCUTS
    #include "shortcuts.h"
  #endif

  #include <stdio.h>
  
  bool sd_item_flag = false;
  
  #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
  #include "../../../libs/base64.hpp"
  #include <map>
  #endif
  #include <string>
  using namespace std;

  #define MACHINE_SIZE STRINGIFY(X_BED_SIZE) "x" STRINGIFY(Y_BED_SIZE) "x" STRINGIFY(Z_MAX_POS)

  #ifndef CORP_WEBSITE_E
    #define CORP_WEBSITE1 "Github.com/"
    #if ENABLED(BOOTPERSO)
      #define CORP_WEBSITE2 "tititopher68-dev/Marlin/"
    #else
      #define CORP_WEBSITE2 "Jyers/Marlin/"
    #endif
  #endif

  #if ENABLED(BOOTPERSO)
    #define BUILD_NUMBER "Build nb: v3.0.0a"
  #else
    #define BUILD_NUMBER "Build nb: v2.1.0a"
  #endif

  #define MENU_CHAR_LIMIT  24
  #define STATUS_CHAR_LIMIT  30

  #define MAX_PRINT_SPEED   500
  #define MIN_PRINT_SPEED   10

  #if HAS_FAN
    #define MAX_FAN_SPEED     255
    #define MIN_FAN_SPEED     0
  #endif

  #define MAX_XY_OFFSET 100

  #if HAS_ZOFFSET_ITEM
    #define MAX_Z_OFFSET 9.99
    #if HAS_BED_PROBE
      #define MIN_Z_OFFSET -9.99
    #else
      #define MIN_Z_OFFSET -1
    #endif
  #endif

  #if HAS_HOTEND
    #define MAX_FLOW_RATE   200
    #define MIN_FLOW_RATE   10

    #define MAX_E_TEMP    (HEATER_0_MAXTEMP - HOTEND_OVERSHOOT)
    #define MIN_E_TEMP    0
  #endif

  #if HAS_HEATED_BED
    #define MAX_BED_TEMP  (BED_MAXTEMP - BED_OVERSHOOT)
    #define MIN_BED_TEMP  0
  #endif

  #define KEY_WIDTH 26
  #define KEY_HEIGHT 30
  #define KEY_INSET 5
  #define KEY_PADDING 3
  #define KEY_Y_START DWIN_HEIGHT-(KEY_HEIGHT*4+2*(KEY_INSET+1))

  #define MBASE(L) (49 + MLINE * (L))

  constexpr float default_max_feedrate[]        = DEFAULT_MAX_FEEDRATE;
  constexpr float default_max_acceleration[]    = DEFAULT_MAX_ACCELERATION;
  constexpr float default_steps[]               = DEFAULT_AXIS_STEPS_PER_UNIT;
  #if HAS_CLASSIC_JERK
    constexpr float default_max_jerk[]            = { DEFAULT_XJERK, DEFAULT_YJERK, DEFAULT_ZJERK, DEFAULT_EJERK };
  #endif

  #if HAS_JUNCTION_DEVIATION
    #define MIN_JD_MM  0.01
    #define MAX_JD_MM  0.3
  #endif


  enum SelectItem : uint8_t {
    PAGE_PRINT = 0,
    PAGE_PREPARE,
    PAGE_CONTROL,
    PAGE_INFO_LEVELING,
    PAGE_SHORTCUT0,
    PAGE_SHORTCUT1,
    PAGE_COUNT,

    PRINT_SETUP = 0,
    PRINT_PAUSE_RESUME,
    PRINT_STOP,
    PRINT_COUNT
  };

  uint8_t shortcut0 = 0;
  uint8_t shortcut1 = 0;

  uint8_t scrollpos = 0;
  uint8_t active_menu = MainMenu, last_menu = MainMenu;
  uint8_t selection = 0, last_selection = 0, last_pos_selection = 0;
  uint8_t process = Main, last_process = Main;
  PopupID popup, last_popup;
  bool keyboard_restrict, reset_keyboard, numeric_keyboard = false;
  uint8_t maxstringlen;
  char *stringpointer = nullptr;

  bool flag_tune = false;
  bool flag_chg_fil = false;
  bool flag_viewmesh = false;
  bool flag_leveling_m = false;
  bool flag_shortcut = false;

  void (*funcpointer)() = nullptr;
  void *valuepointer = nullptr;
  float tempvalue;
  float valuemin;
  float valuemax;
  uint8_t valueunit;
  uint8_t valuetype;

  char cmd[MAX_CMD_SIZE+32], str_1[16], str_2[16], str_3[16];
  char statusmsg[128];
  char filename[LONG_FILENAME_LENGTH];
  char Hostfilename[LONG_FILENAME_LENGTH];
  char reprint_filename[13];
  char STM_cpu[5];
  
  bool paused = false;
  bool sdprint = false;

  int16_t pausetemp, pausebed, pausefan;

  bool livemove = false;
  bool liveadjust = false;

  uint8_t preheatmode = 0;
  uint8_t zoffsetmode = 0;
  
  float zoffsetvalue = 0;

  #if HAS_FILAMENT_SENSOR
    bool State_runoutenable = false;
    uint8_t rsensormode = 0;
  #endif
  
  uint8_t gridpoint;
  float corner_avg;
  float corner_pos;
  float zval;

  #if ENABLED(HOST_ACTION_COMMANDS)
    char action1[9];
    char action2[9];
    char action3[9];
  #endif

  #if ENABLED(BAUD_RATE_GCODE)
    uint8_t BAUD_PORT = 0;
  #endif

  bool probe_deployed = false;

  #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
    std::map<string, int> image_cache;
    uint16_t next_available_address = 1;
    static millis_t thumbtime = 0;
    static millis_t name_scroll_time = 0;
    #define SCROLL_WAIT 1000
    uint16_t file_preview_image_address = 1;
    bool file_preview = false;
    uint16_t header_time_s = 0;
    char header1[40], header2[40], header3[40];
  #endif

  #if HAS_LEVELING
    static bool level_state;
  #endif

  #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
    uint16_t NPrinted = 0;
  #endif

  //struct
  HMI_flags_t HMI_flags;
  HMI_datas_t HMI_datas;
  CrealityDWINClass CrealityDWIN;

  bool CrealityDWINClass::printing = false;

  

  

  #if HAS_MESH

    struct Mesh_Settings {
      bool viewer_asymmetric_range = false;
      bool viewer_print_value = false;
      bool last_viewer_asymmetric_range = false;
      bool last_viewer_print_value = false;
      bool goto_mesh_value = false;
      bool drawing_mesh = false;
      uint8_t mesh_x = 0;
      uint8_t mesh_y = 0;

      #if ENABLED(AUTO_BED_LEVELING_UBL)
        uint8_t tilt_grid = 1;

        void manual_value_update(bool undefined=false) {
          sprintf_P(cmd, PSTR("M421 I%i J%i Z%s %s"), mesh_x, mesh_y, dtostrf(current_position.z, 1, 3, str_1), undefined ? "N" : "");
          gcode.process_subcommands_now(cmd);
          planner.synchronize();
        }

        bool create_plane_from_mesh() {
          struct linear_fit_data lsf_results;
          incremental_LSF_reset(&lsf_results);
          GRID_LOOP(x, y) {
            if (!isnan(Z_VALUES_ARR[x][y])) {
              xy_pos_t rpos;
              rpos.x = ubl.mesh_index_to_xpos(x);
              rpos.y = ubl.mesh_index_to_ypos(y);
              incremental_LSF(&lsf_results, rpos, Z_VALUES_ARR[x][y]);
            }
          }

          if (finish_incremental_LSF(&lsf_results)) {
            SERIAL_ECHOPGM("Could not complete LSF!");
            return true;
          }

          ubl.set_all_mesh_points_to_value(0);

          matrix_3x3 rotation = matrix_3x3::create_look_at(vector_3(lsf_results.A, lsf_results.B, 1));
          GRID_LOOP(i, j) {
            float mx = ubl.mesh_index_to_xpos(i),
                  my = ubl.mesh_index_to_ypos(j),
                  mz = Z_VALUES_ARR[i][j];

            if (DEBUGGING(LEVELING)) {
              DEBUG_ECHOPAIR_F("before rotation = [", mx, 7);
              DEBUG_CHAR(',');
              DEBUG_ECHO_F(my, 7);
              DEBUG_CHAR(',');
              DEBUG_ECHO_F(mz, 7);
              DEBUG_ECHOPGM("]   ---> ");
              DEBUG_DELAY(20);
            }

            rotation.apply_rotation_xyz(mx, my, mz);

            if (DEBUGGING(LEVELING)) {
              DEBUG_ECHOPAIR_F("after rotation = [", mx, 7);
              DEBUG_CHAR(',');
              DEBUG_ECHO_F(my, 7);
              DEBUG_CHAR(',');
              DEBUG_ECHO_F(mz, 7);
              DEBUG_ECHOLNPGM("]");
              DEBUG_DELAY(20);
            }

            Z_VALUES_ARR[i][j] = mz - lsf_results.D;
          }
          return false;
        }

      #else

        void manual_value_update() {
          sprintf_P(cmd, PSTR("G29 I%i J%i Z%s"), mesh_x, mesh_y, dtostrf(current_position.z, 1, 3, str_1));
          gcode.process_subcommands_now(cmd);
          planner.synchronize();
        }

      #endif

      void manual_move(bool zmove=false) {
        if (zmove) {
          planner.synchronize();
          current_position.z = goto_mesh_value ? Z_VALUES_ARR[mesh_x][mesh_y] : Z_CLEARANCE_BETWEEN_PROBES;
          planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
          planner.synchronize();
        }
        else {
          CrealityDWIN.Popup_Handler(MoveWait);
          sprintf_P(cmd, PSTR("G0 F300 Z%s"), dtostrf(Z_CLEARANCE_BETWEEN_PROBES, 1, 3, str_1));
          gcode.process_subcommands_now(cmd);
          sprintf_P(cmd, PSTR("G42 F4000 I%i J%i"), mesh_x, mesh_y);
          gcode.process_subcommands_now(cmd);
          planner.synchronize();
          current_position.z = goto_mesh_value ? Z_VALUES_ARR[mesh_x][mesh_y] : Z_CLEARANCE_BETWEEN_PROBES;
          planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
          planner.synchronize();
          CrealityDWIN.Redraw_Menu();
        }
      }

      float get_max_value() {
        float max = __FLT_MIN__;
        GRID_LOOP(x, y) {
          if (!isnan(Z_VALUES_ARR[x][y]) && Z_VALUES_ARR[x][y] > max)
            max = Z_VALUES_ARR[x][y];
        }
        return max;
      }

      float get_min_value() {
        float min = __FLT_MAX__;
        GRID_LOOP(x, y) {
          if (!isnan(Z_VALUES_ARR[x][y]) && Z_VALUES_ARR[x][y] < min)
            min = Z_VALUES_ARR[x][y];
        }
        return min;
      }

      void Draw_Bed_Mesh(int16_t selected = -1, uint8_t gridline_width = 1, uint16_t padding_x = 8, uint16_t padding_y_top = 40 + 53 - 7) {
        drawing_mesh = true;
        const uint16_t total_width_px = DWIN_WIDTH - padding_x - padding_x;
        const uint16_t cell_width_px  = total_width_px / GRID_MAX_POINTS_X;
        const uint16_t cell_height_px = total_width_px / GRID_MAX_POINTS_Y;
        const float v_max = abs(get_max_value()), v_min = abs(get_min_value()), range = _MAX(v_min, v_max);

        // Clear background from previous selection and select new square
        DWIN_Draw_Rectangle(1, Color_Bg_Black, _MAX(0, padding_x - gridline_width), _MAX(0, padding_y_top - gridline_width), padding_x + total_width_px, padding_y_top + total_width_px);
        if (selected >= 0) {
          const auto selected_y = selected / GRID_MAX_POINTS_X;
          const auto selected_x = selected - (GRID_MAX_POINTS_X * selected_y);
          const auto start_y_px = padding_y_top + selected_y * cell_height_px;
          const auto start_x_px = padding_x + selected_x * cell_width_px;
          DWIN_Draw_Rectangle(1, Color_White, _MAX(0, start_x_px - gridline_width), _MAX(0, start_y_px - gridline_width), start_x_px + cell_width_px, start_y_px + cell_height_px);
        }

        // Draw value square grid
        char buf[8];
        GRID_LOOP(x, y) {
          const auto start_x_px = padding_x + x * cell_width_px;
          const auto end_x_px   = start_x_px + cell_width_px - 1 - gridline_width;
          const auto start_y_px = padding_y_top + (GRID_MAX_POINTS_Y - y - 1) * cell_height_px;
          const auto end_y_px   = start_y_px + cell_height_px - 1 - gridline_width;
          DWIN_Draw_Rectangle(1,                                                                                 // RGB565 colors: http://www.barth-dev.de/online/rgb565-color-picker/
            isnan(Z_VALUES_ARR[x][y]) ? Color_Grey : (                                                           // gray if undefined
              (Z_VALUES_ARR[x][y] < 0 ?
                (uint16_t)round(0x1F * -Z_VALUES_ARR[x][y] / (!viewer_asymmetric_range ? range : v_min)) << 11 : // red if mesh point value is negative
                (uint16_t)round(0x3F *  Z_VALUES_ARR[x][y] / (!viewer_asymmetric_range ? range : v_max)) << 5) | // green if mesh point value is positive
                  _MIN(0x1F, (((uint8_t)abs(Z_VALUES_ARR[x][y]) / 10) * 4))),                                    // + blue stepping for every mm
            start_x_px, start_y_px, end_x_px, end_y_px
          );

          safe_delay(10);
          LCD_SERIAL.flushTX();

          // Draw value text on
          if (viewer_print_value) {
            int8_t offset_x, offset_y = cell_height_px / 2 - 6;
            if (isnan(Z_VALUES_ARR[x][y])) {  // undefined
              DWIN_Draw_String(false, font6x12, Color_White, Color_Bg_Blue, start_x_px + cell_width_px / 2 - 5, start_y_px + offset_y, F("X"));
            }
            else {                          // has value
              if (GRID_MAX_POINTS_X < 10)
                sprintf_P(buf, PSTR("%s"), dtostrf(abs(Z_VALUES_ARR[x][y]), 1, 2, str_1));
              else
                sprintf_P(buf, PSTR("%02i"), (uint16_t)(abs(Z_VALUES_ARR[x][y] - (int16_t)Z_VALUES_ARR[x][y]) * 100));
              offset_x = cell_width_px / 2 - 3 * (strlen(buf)) - 2;
              if (!(GRID_MAX_POINTS_X < 10))
                DWIN_Draw_String(false, font6x12, Color_White, Color_Bg_Blue, start_x_px - 2 + offset_x, start_y_px + offset_y /*+ square / 2 - 6*/, F("."));
              DWIN_Draw_String(false, font6x12, Color_White, Color_Bg_Blue, start_x_px + 1 + offset_x, start_y_px + offset_y /*+ square / 2 - 6*/, buf);
            }
            safe_delay(10);
            LCD_SERIAL.flushTX();
          }
        }
      }

      void Set_Mesh_Viewer_Status() { // TODO: draw gradient with values as a legend instead
        float v_max = abs(get_max_value()), v_min = abs(get_min_value()), range = _MAX(v_min, v_max);
        if (v_min > 3e+10F) v_min = 0.0000001;
        if (v_max > 3e+10F) v_max = 0.0000001;
        if (range > 3e+10F) range = 0.0000001;
        char msg[46];
        if (viewer_asymmetric_range) {
          dtostrf(-v_min, 1, 3, str_1);
          dtostrf( v_max, 1, 3, str_2);
        }
        else {
          dtostrf(-range, 1, 3, str_1);
          dtostrf( range, 1, 3, str_2);
        }
        sprintf_P(msg, PSTR("%s %s..0..%s %s"), GET_TEXT(MSG_COLORS_RED), str_1, str_2, GET_TEXT(MSG_COLORS_GREEN));
        CrealityDWIN.Update_Status(msg);
        drawing_mesh = false;
      }

    };
    Mesh_Settings mesh_conf;

  #endif // HAS_MESH

  /* General Display Functions */

  //struct CrealityDWINClass::eeprom_settings CrealityDWINClass::eeprom_settings{0};
  constexpr const char * const CrealityDWINClass::color_names[16];
  constexpr const char * const CrealityDWINClass::preheat_modes[3];
  constexpr const char * const CrealityDWINClass::zoffset_modes[3];
  #if HAS_FILAMENT_SENSOR
    constexpr const char * const CrealityDWINClass::runoutsensor_modes[4];
  #endif
  #if ENABLED(DWIN_ICON_SET)
    // constexpr const char * const CrealityDWINClass::icon_set[2];
    // constexpr const uint8_t CrealityDWINClass::icon_set_num[2];
    uint8_t CrealityDWINClass::iconset_current = DWIN_ICON_DEF;
  #endif
  constexpr const char * const CrealityDWINClass::shortcut_list[NB_Shortcuts + 1];
  constexpr const char * const CrealityDWINClass::_shortcut_list[NB_Shortcuts + 1];

  // Clear a part of the screen
  //  4=Entire screen
  //  3=Title bar and Menu area (default)
  //  2=Menu area
  //  1=Title bar
  void CrealityDWINClass::Clear_Screen(uint8_t e/*=3*/) {
    if (e == 1 || e == 3 || e == 4) DWIN_Draw_Rectangle(1, GetColor(HMI_datas.menu_top_bg, Color_Bg_Blue, false), 0, 0, DWIN_WIDTH, TITLE_HEIGHT); // Clear Title Bar
    if (e == 2 || e == 3) DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, STATUS_Y); // Clear Menu Area
    if (e == 4) DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, DWIN_HEIGHT); // Clear Popup Area
  }


  void CrealityDWINClass::Draw_Float(float value, uint8_t row, bool selected/*=false*/, uint8_t minunit/*=10*/) {
    const uint8_t digits = (uint8_t)floor(log10(abs(value))) + log10(minunit) + (minunit > 1);
    const uint16_t bColor = (selected) ? GetColor(HMI_datas.select_bg, Select_Color) : GetColor(HMI_datas.background, Color_Bg_Black);
    const uint16_t xpos = 240 - (digits * 8);
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 192, MBASE(row), 232 - (digits * 8), MBASE(row) + 16);
    if (isnan(value))
      DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), bColor, xpos - 8, MBASE(row), F(" NaN"));
    else {
      DWIN_Draw_FloatValue(true, true, 0, DWIN_FONT_MENU, Color_White, bColor, digits - log10(minunit) + 1, log10(minunit), xpos, MBASE(row), (value < 0 ? -value : value));
      DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.select_txt, Color_White), bColor, xpos - 8, MBASE(row), value < 0 ? F("-") : F(" "));
    }
  }

  void CrealityDWINClass::Draw_Option(uint8_t value, const char * const * options, uint8_t row, bool selected/*=false*/, bool color/*=false*/) {
    uint16_t sColor = GetColor(HMI_datas.select_txt, Color_White);
    uint16_t bColor = (selected) ? GetColor(HMI_datas.select_bg, Select_Color): GetColor(HMI_datas.background, Color_Bg_Black);
    uint16_t tColor = (color) ? GetColor(value, sColor, false) : GetColor(HMI_datas.items_menu_text, Color_White);
    DWIN_Draw_Rectangle(1, bColor, 202, MBASE(row) + 14, 258, MBASE(row) - 2);
    DWIN_Draw_String(false, DWIN_FONT_MENU, ((tColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((tColor == Color_Black) && (HMI_datas.background == 0))) ? GetColor(HMI_datas.items_menu_text, Color_White) : tColor, bColor, 202, MBASE(row) - 1, options[value]);
  }

  void CrealityDWINClass::Draw_String(char * string, uint8_t row, bool selected/*=false*/, bool below/*=false*/) {
    if (!string) string[0] = '\0';
    const uint8_t offset_x = DWIN_WIDTH-strlen(string)*8 - 20;
    const uint8_t offset_y = (below) ? MENU_CHR_H * 3 / 5 : 0;
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), offset_x - 10, MBASE(row)+offset_y-1, offset_x, MBASE(row)+16+offset_y);
    DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), (selected) ? Select_Color : GetColor(HMI_datas.background, Color_Bg_Black), offset_x, MBASE(row)-1+offset_y, string);
  }

  const uint64_t CrealityDWINClass::Encode_String(const char * string) {
    const char table[65] = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    uint64_t output = 0;
    LOOP_L_N(i, strlen(string)) {
      uint8_t upper_bound = 63, lower_bound = 0;
      uint8_t midpoint;
      LOOP_L_N(x, 6) {
        midpoint = (uint8_t)(0.5*(upper_bound+lower_bound));
        if (string[i] > table[midpoint]) lower_bound = midpoint;
        else if (string[i] < table[midpoint]) upper_bound = midpoint;
        else break;
      }
      output += midpoint*pow(64,i);
    }
    return output;
  }

  void CrealityDWINClass::Decode_String(uint64_t num, char * string) {
    const char table[65] = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    LOOP_L_N(i, 30) {
      string[i] = table[num%64];
      num /= 64;
      if (num==0) {
        string[i+1] = '\0';
        break;
      }
    }
  }

  void CrealityDWINClass::Apply_shortcut(uint8_t shortcut) {
    switch (shortcut){
      case Preheat_menu: flag_shortcut = true; Draw_Menu(Preheat); break;
      case Cooldown: thermalManager.cooldown(); break;
      case Disable_stepper: queue.inject(F("M84")); break;
      case Autohome: Popup_Handler(Home); gcode.home_all_axes(true); Draw_Main_Menu(1); break;
      case ZOffsetmenu: 
        #if HAS_LEVELING
          level_state = planner.leveling_active;
          #if NONE(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN, USE_PROBE_FOR_Z_HOMING)
            set_bed_leveling_enabled(true);
          #else
            set_bed_leveling_enabled(false);
          #endif
        #endif
        #if !HAS_BED_PROBE
          gcode.process_subcommands_now(F("M211 S0"));
        #endif
        flag_shortcut = true;
        Draw_Menu(ZOffset);
        break;
      case M_Tramming_menu:
        if (axes_should_home()) {
          Popup_Handler(Home);
          gcode.home_all_axes(true);
        }
        #if HAS_LEVELING
          level_state = planner.leveling_active;
          set_bed_leveling_enabled(false);
          #endif
          flag_shortcut = true;
          Draw_Menu(ManualLevel);
          break;
      #if ENABLED(ADVANCED_PAUSE_FEATURE)
        case Change_Fil:
          #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
            flag_shortcut = true;
            Draw_Menu(ChangeFilament);
          #else
            Draw_Menu(Prepare, PREPARE_CHANGEFIL);
          #endif
        break;
      #endif
      #if HAS_SHORTCUTS
        case Move_rel_Z:
          DWIN_Move_Z();
          break;
      #endif
      case ScreenL:
        DWIN_ScreenLock();
        break;
      default : break;
    }
  }

  uint16_t CrealityDWINClass::GetColor(uint8_t color, uint16_t original, bool light/*=false*/) {
    switch (color){
      case Default:
        return original;
        break;
      case White:
        return (light) ? Color_Light_White : Color_White;
        break;
      case Light_White:
        return Color_Light_White;
        break;
      case Green:
        return (light) ? Color_Light_Green : Color_Green;
        break;
      case Light_Green:
        return Color_Light_Green;
        break;
      case Blue:
        return (light) ? Color_Light_Blue : Color_Blue;
        break;
      case Magenta:
        return (light) ? Color_Light_Magenta : Color_Magenta;
        break;
      case Light_Magenta:
        return Color_Light_Magenta;
        break;
      case Red:
        return (light) ? Color_Light_Red : Color_Red;
        break;
      case Light_Red:
        return Color_Light_Red;
        break;
      case Orange:
        return (light) ? Color_Light_Orange : Color_Orange;
        break;
      case Yellow:
        return (light) ? Color_Light_Yellow : Color_Yellow;
        break;
      case Brown:
        return (light) ? Color_Light_Brown : Color_Brown;
        break;
      case Cyan:
        return (light) ? Color_Light_Cyan : Color_Cyan;
        break;
      case Light_Cyan:
        return Color_Light_Cyan;
        break;
      case Black:
        return Color_Black;
        break;           
    }
    return Color_White;
  }

  void CrealityDWINClass::Draw_Title(const char * ctitle) {
    DWIN_Draw_String(false, DWIN_FONT_HEAD, GetColor(HMI_datas.menu_top_txt, Color_White, false), Color_Bg_Blue, (DWIN_WIDTH - strlen(ctitle) * STAT_CHR_W) / 2, 5, ctitle);
  }
  void CrealityDWINClass::Draw_Title(FSTR_P const ftitle) {
    DWIN_Draw_String(false, DWIN_FONT_HEAD, GetColor(HMI_datas.menu_top_txt, Color_White, false), Color_Bg_Blue, (DWIN_WIDTH - strlen_P(FTOP(ftitle)) * STAT_CHR_W) / 2, 5, ftitle);
  }

  void _Decorate_Menu_Item(uint8_t row, uint8_t icon, bool more) {
    if (icon) DRAW_IconWTB(ICON, icon, 26, MBASE(row) - 3);   //Draw Menu Icon
    if (more) DRAW_IconWTB(ICON, ICON_More, 226, MBASE(row) - 3); // Draw More Arrow
    DWIN_Draw_Line(CrealityDWIN.GetColor(HMI_datas.menu_split_line, Line_Color, true), 16, MBASE(row) + 33, 256, MBASE(row) + 33); // Draw Menu Line
  }

  uint16_t image_address;
  void CrealityDWINClass::Draw_Menu_Item(uint16_t row, uint8_t icon/*=0*/, const char * label1, const char * label2, bool more/*=false*/, bool centered/*=false*/, bool onlyCachedFileIcon/*=false*/) {
    const uint8_t label_offset_y = (label1 && label2) ? MENU_CHR_H * 3 / 5 : 0,
                  label1_offset_x = !centered ? LBLX : LBLX * 4/5 + _MAX(LBLX * 1U/5, (DWIN_WIDTH - LBLX - (label1 ? strlen(label1) : 0) * MENU_CHR_W) / 2),
                  label2_offset_x = !centered ? LBLX : LBLX * 4/5 + _MAX(LBLX * 1U/5, (DWIN_WIDTH - LBLX - (label2 ? strlen(label2) : 0) * MENU_CHR_W) / 2);
    if (label1) DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), label1_offset_x, MBASE(row) - 1 - label_offset_y, label1); // Draw Label
    if (label2) DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), label2_offset_x, MBASE(row) - 1 + label_offset_y, label2); // Draw Label
    //_Decorate_Menu_Item(row, icon, more);
    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
      if ((HMI_datas.show_gcode_thumbnails) && (sd_item_flag) && (icon == ICON_File) && find_and_decode_gcode_preview(card.filename, Thumnail_Icon, &image_address, onlyCachedFileIcon))
        DWIN_SRAM_Memory_Icon_Display(9, MBASE(row) - 18, image_address);
      else 
        if (icon) DRAW_IconWTB(ICON, icon, 26, MBASE(row) - 3);   //Draw Menu Icon
    #else
      if (icon) DRAW_IconWTB(ICON, icon, 26, MBASE(row) - 3);   //Draw Menu Icon
    #endif
    if (more) DRAW_IconWTB(ICON, ICON_More, 226, MBASE(row) - 3); // Draw More Arrow
    DWIN_Draw_Line(GetColor(HMI_datas.menu_split_line, Line_Color, true), 16, MBASE(row) + 33, 256, MBASE(row) + 33); // Draw Menu Line
    }

  void CrealityDWINClass::Draw_Menu_Item(uint8_t row, uint8_t icon/*=0*/, FSTR_P const flabel1, FSTR_P const flabel2, bool more/*=false*/, bool centered/*=false*/, bool onlyCachedFileIcon/*=false*/) {
    const uint8_t label_offset_y = (flabel1 && flabel2) ? MENU_CHR_H * 3 / 5 : 0,
                  label1_offset_x = !centered ? LBLX : LBLX * 4/5 + _MAX(LBLX * 1U/5, (DWIN_WIDTH - LBLX - (flabel1 ? strlen_P(FTOP(flabel1)) : 0) * MENU_CHR_W) / 2),
                  label2_offset_x = !centered ? LBLX : LBLX * 4/5 + _MAX(LBLX * 1U/5, (DWIN_WIDTH - LBLX - (flabel2 ? strlen_P(FTOP(flabel2)) : 0) * MENU_CHR_W) / 2);
    if (flabel1) DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), label1_offset_x, MBASE(row) - 1 - label_offset_y, flabel1); // Draw Label
    if (flabel2) DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.items_menu_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), label2_offset_x, MBASE(row) - 1 + label_offset_y, flabel2); // Draw Label
    //_Decorate_Menu_Item(row, icon, more);
    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
      if ((HMI_datas.show_gcode_thumbnails) && (sd_item_flag) && (icon == ICON_File) && find_and_decode_gcode_preview(card.filename, Thumnail_Icon, &image_address, onlyCachedFileIcon))
      DWIN_SRAM_Memory_Icon_Display(9, MBASE(row) - 18, image_address);
    else 
      if (icon) DRAW_IconWTB(ICON, icon, 26, MBASE(row) - 3);   //Draw Menu Icon
    #else
      if (icon) DRAW_IconWTB(ICON, icon, 26, MBASE(row) - 3);   //Draw Menu Icon
    #endif
    if (more) DRAW_IconWTB(ICON, ICON_More, 226, MBASE(row) - 3); // Draw More Arrow
    DWIN_Draw_Line(GetColor(HMI_datas.menu_split_line, Line_Color, true), 16, MBASE(row) + 33, 256, MBASE(row) + 33); // Draw Menu Line
  }

  void CrealityDWINClass::Draw_Checkbox(uint8_t row, bool value) {
    #if ENABLED(DWIN_CREALITY_LCD_CUSTOM_ICONS) // Draw appropriate checkbox icon
      // Only for my personal Display icon pack
      // DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 226, MBASE(row) - 3, 226 + 40, MBASE(row) - 3 + 20);
      // DRAW_IconWTB(ICON, (value ? ICON_Checkbox_T : ICON_Checkbox_F), 226, MBASE(row) - 3);
      DRAW_IconWB(ICON, (value ? ICON_Checkbox_T : ICON_Checkbox_F), 226, MBASE(row) - 3);
    #else                                         // Draw a basic checkbox using rectangles and lines
      DWIN_Draw_Rectangle(1, Color_Bg_Black, 226, MBASE(row) - 3, 226 + 20, MBASE(row) - 3 + 20);
      DWIN_Draw_Rectangle(0, Color_White, 226, MBASE(row) - 3, 226 + 20, MBASE(row) - 3 + 20);
      if (value) {
        DWIN_Draw_Line(Check_Color, 227, MBASE(row) - 3 + 11, 226 + 8, MBASE(row) - 3 + 17);
        DWIN_Draw_Line(Check_Color, 227 + 8, MBASE(row) - 3 + 17, 226 + 19, MBASE(row) - 3 + 1);
        DWIN_Draw_Line(Check_Color, 227, MBASE(row) - 3 + 12, 226 + 8, MBASE(row) - 3 + 18);
        DWIN_Draw_Line(Check_Color, 227 + 8, MBASE(row) - 3 + 18, 226 + 19, MBASE(row) - 3 + 2);
        DWIN_Draw_Line(Check_Color, 227, MBASE(row) - 3 + 13, 226 + 8, MBASE(row) - 3 + 19);
        DWIN_Draw_Line(Check_Color, 227 + 8, MBASE(row) - 3 + 19, 226 + 19, MBASE(row) - 3 + 3);
      }
    #endif
  }

  void CrealityDWINClass::Draw_Menu(uint8_t menu, uint8_t select/*=0*/, uint8_t scroll/*=0*/) {
    uint16_t cColor = GetColor(HMI_datas.cursor_color, Rectangle_Color);
    if (active_menu != menu) {
      last_menu = active_menu;
      if (process == Menu) last_selection = selection;
    }
    selection = _MIN(select, Get_Menu_Size(menu));
    scrollpos = scroll;
    if (selection - scrollpos > MROWS)
      scrollpos = selection - MROWS;
    process = Menu;
    active_menu = menu;
    Clear_Screen();
    Draw_Title(Get_Menu_Title(menu));
    LOOP_L_N(i, TROWS) Menu_Item_Handler(menu, i + scrollpos);
    if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0)))
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 31);
    else
        DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 31);
  }

  void CrealityDWINClass::Redraw_Menu(bool lastprocess/*=true*/, bool lastselection/*=false*/, bool lastmenu/*=false*/, bool flag_scroll/*=false*/) {
    switch ((lastprocess) ? last_process : process) {
      case Menu:
        if (flag_tune) { last_selection = last_pos_selection; flag_tune = false; }
        Draw_Menu((lastmenu) ? last_menu : active_menu, (lastselection) ? last_selection : selection, (flag_scroll) ? 0 : scrollpos);
        break;
      case Main:  Draw_Main_Menu((lastselection) ? last_selection : selection); break;
      case Print: Draw_Print_Screen(); break;
      case File:  Draw_SD_List(false, (lastselection) ? last_selection : 0, (lastselection) ? scrollpos : 0, true); break;
      default: break;
    }
  }

  void CrealityDWINClass::Redraw_Screen() {
    Redraw_Menu(false);
    Draw_Status_Area(true);
    Update_Status_Bar(true);
  }

  /* Primary Menus and Screen Elements */

  void CrealityDWINClass::Main_Menu_Icons() {
    if (selection == 0) {
      DRAW_IconWB(ICON, ICON_Print_1, 17, 68);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 17, 68, 126, 167);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 52, 138, GET_TEXT_F(MSG_BUTTON_PRINT));
    }
    else {
      DRAW_IconWB(ICON, ICON_Print_0, 17, 68);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 52, 138, GET_TEXT_F(MSG_BUTTON_PRINT));
    }
    if (selection == 1) {
      DRAW_IconWB(ICON, ICON_Prepare_1, 145, 68);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 145, 68, 254, 167);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 170, 138, GET_TEXT_F(MSG_PREPARE));
    }
    else {
      DRAW_IconWB(ICON, ICON_Prepare_0, 145, 68);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 170, 138, GET_TEXT_F(MSG_PREPARE));
    }
    if (selection == 2) {
      DRAW_IconWB(ICON, ICON_Control_1, 17, 184);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 17, 184, 126, 283);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 43, 255, GET_TEXT_F(MSG_CONTROL));
    }
    else {
      DRAW_IconWB(ICON, ICON_Control_0, 17, 184);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 43, 255, GET_TEXT_F(MSG_CONTROL));
    }
    #if HAS_ABL_OR_UBL
      if (selection == 3) {
        DRAW_IconWB(ICON, ICON_Leveling_1, 145, 184);
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 145, 184, 254, 283);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 179, 255, GET_TEXT_F(MSG_BUTTON_LEVEL));
      }
      else {
        DRAW_IconWB(ICON, ICON_Leveling_0, 145, 184);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 179, 255, GET_TEXT_F(MSG_BUTTON_LEVEL));
      }
    #else
      if (selection == 3) {
        DRAW_IconWB(ICON, ICON_Info_1, 145, 184);
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 145, 184, 254, 283);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 181, 255, GET_TEXT_F(MSG_BUTTON_INFO));
      }
      else {
        DRAW_IconWB(ICON, ICON_Info_0, 145, 184);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 181, 255, GET_TEXT_F(MSG_BUTTON_INFO));
      }
    #endif
    if (selection == 4) {
      DWIN_Draw_Rectangle(1, Color_Shortcut_1, 17, 300, 126, 347);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 17, 300, 126, 347);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 17 + ((109 - strlen(_shortcut_list[shortcut0]) * MENU_CHR_W) / 2), 316, F(_shortcut_list[shortcut0]));
    }
    else {
      DWIN_Draw_Rectangle(1, Color_Shortcut_0, 17, 300, 126, 347);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 17 + ((109 - strlen(_shortcut_list[shortcut0]) * MENU_CHR_W) / 2), 316, F(_shortcut_list[shortcut0]));
    }
    if (selection == 5) {
      DWIN_Draw_Rectangle(1, Color_Shortcut_1, 145, 300, 254, 347);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 145, 300, 254, 347);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 145 + ((109 - strlen(_shortcut_list[shortcut1]) * MENU_CHR_W) / 2), 316, F(_shortcut_list[shortcut1]));
    }
    else {
      DWIN_Draw_Rectangle(1, Color_Shortcut_0, 145, 300, 254, 347);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 145 + ((109 - strlen(_shortcut_list[shortcut1]) * MENU_CHR_W) / 2), 316, F(_shortcut_list[shortcut1]));
    }
  }

  void CrealityDWINClass::Draw_Main_Menu(uint8_t select/*=0*/) {
    process = Main;
    active_menu = MainMenu;
    selection = select;
    Clear_Screen();
    Draw_Title(Get_Menu_Title(MainMenu));
    SERIAL_ECHOPGM("\nDWIN handshake ");
    DRAW_IconWTB(ICON, ICON_LOGO, 71, 41);
    Main_Menu_Icons();
  }

  void CrealityDWINClass::Print_Screen_Icons() {
    if (selection == 0) {
      DRAW_IconWB(ICON, ICON_Setup_1, 8, 252);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 8, 252, 87, 351);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 30, 322, GET_TEXT_F(MSG_TUNE));
    }
    else {
      DRAW_IconWB(ICON, ICON_Setup_0, 8, 252);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 30, 322, GET_TEXT_F(MSG_TUNE));
    }
    if (selection == 2) {
      DRAW_IconWB(ICON, ICON_Stop_1, 184, 252);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 184, 252, 263, 351);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 205, 322, GET_TEXT_F(MSG_BUTTON_STOP));
    }
    else {
      DRAW_IconWB(ICON, ICON_Stop_0, 184, 252);
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 205, 322, GET_TEXT_F(MSG_BUTTON_STOP));
    }
    if (paused) {
      if (selection == 1) {
        DRAW_IconWB(ICON, ICON_Continue_1, 96, 252);
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 96, 252, 175, 351);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 114, 322, GET_TEXT_F(MSG_BUTTON_PRINT));
      }
      else {
        DRAW_IconWB(ICON, ICON_Continue_0, 96, 252);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 114, 322, GET_TEXT_F(MSG_BUTTON_PRINT));
      }
    }
    else {
      if (selection == 1) {
        DRAW_IconWB(ICON, ICON_Pause_1, 96, 252);
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 96, 252, 175, 351);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 114, 322, GET_TEXT_F(MSG_BUTTON_PAUSE));
      }
      else {
        DRAW_IconWB(ICON, ICON_Pause_0, 96, 252);
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.icons_menu_text, Color_White), Color_Bg_Blue, 114, 322, GET_TEXT_F(MSG_BUTTON_PAUSE));
      }
    }
  }

  void CrealityDWINClass::Draw_Print_Screen() {
    process = Print;
    selection = 0;
    Clear_Screen();
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 352, DWIN_WIDTH - 8, 376);
    Draw_Title(GET_TEXT(MSG_PRINTING));
    Print_Screen_Icons();
    DRAW_IconWTB(ICON, ICON_PrintTime, 14, 171);
    DRAW_IconWTB(ICON, ICON_RemainTime, 147, 169);
    DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.print_screen_txt, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 41, 163, GET_TEXT_F(MSG_ELAPSED_TIME));
    DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.print_screen_txt, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 176, 163, GET_TEXT_F(MSG_REMAINING_TIME));
    Update_Status_Bar(true);
    Draw_Print_ProgressBar();
    Draw_Print_ProgressElapsed();
    TERN_(USE_M73_REMAINING_TIME, Draw_Print_ProgressRemain());
    Draw_Print_Filename(true);
  }


  void CrealityDWINClass::Draw_Print_Filename(const bool reset/*=false*/) {
    static uint8_t namescrl = 0;
    if (reset) namescrl = 0;
    if (process == Print) {
      size_t len = strlen(filename);
      int8_t pos = len;
      if (pos > STATUS_CHAR_LIMIT) {
        pos -= namescrl;
        len = _MIN((size_t)pos, (size_t)STATUS_CHAR_LIMIT);
        char dispname[len + 1];
        if (pos >= 0) {
          LOOP_L_N(i, len) dispname[i] = filename[i + namescrl];
        }
        else {
          LOOP_L_N(i, STATUS_CHAR_LIMIT + pos) dispname[i] = ' ';
          LOOP_S_L_N(i, STATUS_CHAR_LIMIT + pos, STATUS_CHAR_LIMIT) dispname[i] = filename[i - (STATUS_CHAR_LIMIT + pos)];
        }
        dispname[len] = '\0';
        DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 50, DWIN_WIDTH - 8, 80);
        const int8_t npos = (DWIN_WIDTH - STATUS_CHAR_LIMIT * MENU_CHR_W) / 2;
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.print_filename, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 60, dispname);
        if (-pos >= STATUS_CHAR_LIMIT) namescrl = 0;
        namescrl++;
      }
      else {
          DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 50, DWIN_WIDTH - 8, 80);
          const int8_t npos = (DWIN_WIDTH - strlen(filename) * MENU_CHR_W) / 2;
          DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.print_filename, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 60, filename);
      }
    }
  }

  void CrealityDWINClass::Draw_Print_ProgressBar() {
    uint8_t printpercent = sdprint ? card.percentDone() : (ui._get_progress() / 100);
    DRAW_IconWB(ICON, ICON_Bar, 15, 93);
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.progress_bar, BarFill_Color), 16 + printpercent * 240 / 100, 93, 256, 113);
    DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_MENU, GetColor(HMI_datas.progress_percent, Percent_Color), GetColor(HMI_datas.background, Color_Bg_Black), 3, 109, 133, printpercent);
    DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.progress_percent, Percent_Color), GetColor(HMI_datas.background, Color_Bg_Black), 133, 133, F("%"));
  }

  #if ENABLED(USE_M73_REMAINING_TIME)

    void CrealityDWINClass::Draw_Print_ProgressRemain() {
      uint16_t remainingtime = ui.get_remaining_time();
      DWIN_Draw_IntValue(true, true, 1, DWIN_FONT_MENU, GetColor(HMI_datas.remain_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 2, 176, 187, remainingtime / 3600);
      DWIN_Draw_IntValue(true, true, 1, DWIN_FONT_MENU, GetColor(HMI_datas.remain_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 2, 200, 187, (remainingtime % 3600) / 60);
      if (HMI_datas.time_format_textual) {
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.remain_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 192, 187, GET_TEXT_F(MSG_SHORT_HOUR));
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.remain_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 216, 187, GET_TEXT_F(MSG_SHORT_MINUTE));
      }
      else
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.remain_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 192, 187, F(":"));
    }

  #endif

  void CrealityDWINClass::Draw_Print_ProgressElapsed() {
    duration_t elapsed = print_job_timer.duration();
    DWIN_Draw_IntValue(true, true, 1, DWIN_FONT_MENU, GetColor(HMI_datas.elapsed_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 2, 42, 187, elapsed.value / 3600);
    DWIN_Draw_IntValue(true, true, 1, DWIN_FONT_MENU, GetColor(HMI_datas.elapsed_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 2, 66, 187, (elapsed.value % 3600) / 60);
    if (HMI_datas.time_format_textual) {
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.elapsed_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 58, 187, GET_TEXT_F(MSG_SHORT_HOUR));
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.elapsed_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 82, 187, GET_TEXT_F(MSG_SHORT_MINUTE));
    }
    else
      DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.elapsed_time, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 58, 187, F(":"));
  }

  void CrealityDWINClass::Draw_Print_confirm() {
    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
      if (!file_preview) Draw_Print_Screen(); 
      else {Clear_Screen(); Draw_Title(GET_TEXT(MSG_PRINTING));}
    #else
      Draw_Print_Screen();
    #endif
    process = Confirm;
    popup = Complete;
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 252, 263, 351);
    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
          if (file_preview) {
            //Clear_Screen();
            DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 45, 75, 231, 261);
            DWIN_Draw_Rectangle(0, GetColor(HMI_datas.highlight_box, Color_White), 45, 75, 231, 261);
            TERN(DACAI_DISPLAY, DRAW_IconTH(48 ,78 , file_preview_image_address), DWIN_SRAM_Memory_Icon_Display(48 ,78 , file_preview_image_address));
            Update_Status(cmd);
          }
    #endif
    //DRAW_IconWTB(ICON, ICON_Confirm_E, 87, 283);
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.ico_confirm_bg , Confirm_Color), 87, 288, 186, 335);
    DWIN_Draw_String(false, DWIN_FONT_HEAD, GetColor(HMI_datas.ico_confirm_txt, Color_White), GetColor(HMI_datas.ico_confirm_bg, Confirm_Color), 87 + ((99 - 7 * STAT_CHR_W) / 2), 304, GET_TEXT_F(MSG_BUTTON_CONFIRM));
    
    DWIN_Draw_Rectangle(0, GetColor(HMI_datas.popup_highlight, Color_White), 86, 287, 187, 336);
    DWIN_Draw_Rectangle(0, GetColor(HMI_datas.popup_highlight, Color_White), 85, 286, 188, 337);
  }

  void CrealityDWINClass::Draw_SD_Item(uint8_t item, uint8_t row, bool onlyCachedFileIcon/*=false*/) {
    if (item == 0)
      Draw_Menu_Item(0, ICON_Back, card.flag.workDirIsRoot ? GET_TEXT_F(MSG_BACK) : F(".."));
    else {
      card.getfilename_sorted(SD_ORDER(item - 1, card.get_num_Files()));
      char * const filename = card.longest_filename();
      size_t max = MENU_CHAR_LIMIT;
      size_t pos = strlen(filename), len = pos;
      if (!card.flag.filenameIsDir)
        while (pos && filename[pos] != '.') pos--;
      len = pos;
      if (len > max) len = max;
      char name[len + 1];
      LOOP_L_N(i, len) name[i] = filename[i];
      if (pos > max)
        LOOP_S_L_N(i, len - 3, len) name[i] = '.';
      name[len] = '\0';
      Draw_Menu_Item(row, card.flag.filenameIsDir ? ICON_More : ICON_File, name, NULL, NULL, false, onlyCachedFileIcon);
    }
  }

  void CrealityDWINClass::Draw_SD_List(bool removed/*=false*/, uint8_t select/*=0*/, uint8_t scroll/*=0*/, bool onlyCachedFileIcon/*=false*/) {
    uint16_t cColor = GetColor(HMI_datas.cursor_color, Rectangle_Color);
    Clear_Screen();
    Draw_Title(GET_TEXT(MSG_FILE_SELECTION));
    // selection = 0;
    // scrollpos = 0;
    // process = File;
    selection = min((int)select, card.get_num_Files() + 1);
    scrollpos = scroll;
    if (selection - scrollpos > MROWS)
      scrollpos = selection - MROWS;
    process = File;
    if (card.isMounted() && !removed) {
      sd_item_flag = true;
      LOOP_L_N(i, _MIN(card.get_num_Files() + 1, TROWS))
        Draw_SD_Item(i + scrollpos, i, onlyCachedFileIcon);
    }
    else {
      Draw_Menu_Item(0, ICON_Back, GET_TEXT_F(MSG_BACK));
      sd_item_flag = false;
      DWIN_Draw_Rectangle(1, Color_Bg_Red, 10, MBASE(3) - 10, DWIN_WIDTH - 10, MBASE(4));
      DWIN_Draw_String(false, font16x32, Color_Yellow, Color_Bg_Red, ((DWIN_WIDTH) - 8 * 16) / 2, MBASE(3), GET_TEXT_F(MSG_NO_MEDIA));
    }
    if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0)))
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection-scrollpos) - 18, 8, MBASE(selection-scrollpos) + 31);
    else
        DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection-scrollpos) - 18, 8, MBASE(selection-scrollpos) + 31);
  }

  void CrealityDWINClass::Draw_Status_Area(bool icons/*=false*/) {

    if (icons) DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, STATUS_Y, DWIN_WIDTH, DWIN_HEIGHT - 1);

    #if HAS_HOTEND
      static float hotend = -1;
      static int16_t hotendtarget = -1, flow = -1;
      if (icons) {
        hotend = -1;
        hotendtarget = -1;
        DRAW_IconWTB(ICON, ICON_HotendTemp, 10, 383);
        DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 25 + 3 * STAT_CHR_W + 5, 384, F("/"));
      }
      if (thermalManager.temp_hotend[0].celsius != hotend) {
        hotend = thermalManager.temp_hotend[0].celsius;
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 28, 384, thermalManager.temp_hotend[0].celsius);
        DWIN_Draw_DegreeSymbol(GetColor(HMI_datas.status_area_text, Color_White), 25 + 3 * STAT_CHR_W + 5, 386);
      }
      if (thermalManager.temp_hotend[0].target != hotendtarget) {
        hotendtarget = thermalManager.temp_hotend[0].target;
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 25 + 4 * STAT_CHR_W + 6, 384, thermalManager.temp_hotend[0].target);
        DWIN_Draw_DegreeSymbol(GetColor(HMI_datas.status_area_percent, Color_White), 25 + 4 * STAT_CHR_W + 39, 386);
      }
      if (icons) {
        flow = -1;
        DRAW_IconWTB(ICON, ICON_StepE, 112, 417);
        DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 116 + 5 * STAT_CHR_W + 2, 417, F("%"));
      }
      if (planner.flow_percentage[0] != flow) {
        flow = planner.flow_percentage[0];
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 116 + 2 * STAT_CHR_W, 417, planner.flow_percentage[0]);
      }
    #endif

    #if HAS_HEATED_BED
      static float bed = -1;
      static int16_t bedtarget = -1;
      if (icons) {
        bed = -1;
        bedtarget = -1;
        DRAW_IconWTB(ICON, ICON_BedTemp, 10, 416);
        DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 25 + 3 * STAT_CHR_W + 5, 417, F("/"));
      }
      if (thermalManager.temp_bed.celsius != bed) {
        bed = thermalManager.temp_bed.celsius;
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 28, 417, thermalManager.temp_bed.celsius);
        DWIN_Draw_DegreeSymbol(GetColor(HMI_datas.status_area_text, Color_White), 25 + 3 * STAT_CHR_W + 5, 419);
      }
      if (thermalManager.temp_bed.target != bedtarget) {
        bedtarget = thermalManager.temp_bed.target;
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 25 + 4 * STAT_CHR_W + 6, 417, thermalManager.temp_bed.target);
        DWIN_Draw_DegreeSymbol(GetColor(HMI_datas.status_area_percent, Color_White), 25 + 4 * STAT_CHR_W + 39, 419);
      }
    #endif

    #if HAS_FAN
      static uint8_t fan = -1;
      if (icons) {
        fan = -1;
        DRAW_IconWTB(ICON, ICON_FanSpeed, 187, 383);
        DWIN_Draw_String((HMI_datas.fan_percent) ? false : true, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 195 + 5 * STAT_CHR_W + 2, 384, (HMI_datas.fan_percent) ? F("%") : F(" "));
      }
      if (thermalManager.fan_speed[0] != fan) {
        fan = thermalManager.fan_speed[0];
        DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 195 + 2 * STAT_CHR_W, 384, (HMI_datas.fan_percent) ? (uint32_t)floor((thermalManager.fan_speed[0]) * 100 / 255) : thermalManager.fan_speed[0]) ;
      
      }
    #endif

    #if HAS_ZOFFSET_ITEM
      static float offset = -1;

      if (icons) {
        offset = -1;
        DRAW_IconWTB(ICON, ICON_Zoffset, 187, 416);
      }
      if (zoffsetvalue != offset) {
        offset = zoffsetvalue;
        DWIN_Draw_FloatValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 2, 2, 207, 417, (zoffsetvalue < 0 ? -zoffsetvalue : zoffsetvalue));
        DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.status_area_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 205, 419, zoffsetvalue < 0 ? F("-") : F(" "));
      }
      #if HAS_MESH
        static bool _leveling_active = false;
        static bool _printing_leveling_active = false;
        if (printingIsActive()) {
          _printing_leveling_active = ((planner.leveling_active && planner.leveling_active_at_z(destination.z)) || _leveling_active);   
          if ((_printing_leveling_active = (planner.leveling_active && planner.leveling_active_at_z(destination.z)) && ui.get_blink()))
            DWIN_Draw_Rectangle(0, GetColor(HMI_datas.status_area_text, Color_White), 187, 415, 204, 435);
          else 
            DWIN_Draw_Rectangle(0, GetColor(HMI_datas.background, Color_Bg_Black), 187, 415, 204, 435);
        }
        else {
          _leveling_active = (planner.leveling_active || _leveling_active);
          if ((_leveling_active = planner.leveling_active && ui.get_blink()))
            DWIN_Draw_Rectangle(0, GetColor(HMI_datas.status_area_text, Color_White), 187, 415, 204, 435);
          else 
            DWIN_Draw_Rectangle(0, GetColor(HMI_datas.background, Color_Bg_Black), 187, 415, 204, 435);
        }  
      #endif
    #endif

    static int16_t feedrate = -1;
    if (icons) {
      feedrate = -1;
      DRAW_IconWTB(ICON, ICON_Speed, 113, 383);
      DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 116 + 5 * STAT_CHR_W + 2, 384, F("%"));
    }
    if (feedrate_percentage != feedrate) {
      feedrate = feedrate_percentage;
      DWIN_Draw_IntValue(true, true, 0, DWIN_FONT_STAT, GetColor(HMI_datas.status_area_percent, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 116 + 2 * STAT_CHR_W, 384, feedrate_percentage);
    }

    static float x = -1, y = -1, z = -1;
    static bool update_x = false, update_y = false, update_z = false;
    update_x = (current_position.x != x || axis_should_home(X_AXIS) || update_x);
    update_y = (current_position.y != y || axis_should_home(Y_AXIS) || update_y);
    update_z = (current_position.z != z || axis_should_home(Z_AXIS) || update_z);
    if (icons) {
      x = y = z = -1;
      DWIN_Draw_Line(GetColor(HMI_datas.coordinates_split_line, Line_Color, true), 16, 450, 256, 450);
      DRAW_IconWTB(ICON, ICON_MaxSpeedX,  10, 456);
      DRAW_IconWTB(ICON, ICON_MaxSpeedY,  95, 456);
      DRAW_IconWTB(ICON, ICON_MaxSpeedZ, 180, 456);
    }
    if (update_x) {
      x = current_position.x;
      if ((update_x = axis_should_home(X_AXIS) && ui.get_blink()))
        DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 35, 459, F("  -?-  "));
      else
        DWIN_Draw_FloatValue(true, true, 0, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 1, 35, 459, current_position.x);
    }
    if (update_y) {
      y = current_position.y;
      if ((update_y = axis_should_home(Y_AXIS) && ui.get_blink()))
        DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 120, 459, F("  -?-  "));
      else
        DWIN_Draw_FloatValue(true, true, 0, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 1, 120, 459, current_position.y);
    }
    if (update_z) {
      z = current_position.z;
      if ((update_z = axis_should_home(Z_AXIS) && ui.get_blink()))
        DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 205, 459, F("  -?-  "));
      else
        DWIN_Draw_FloatValue(true, true, 0, DWIN_FONT_MENU, GetColor(HMI_datas.coordinates_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), 3, 2, 205, 459, (current_position.z>=0) ? ((DISABLED(HAS_BED_PROBE) && printing) ? (current_position.z - zoffsetvalue) : current_position.z) : 0);
    }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Draw_Popup(FSTR_P const line1, FSTR_P const line2, FSTR_P const line3, uint8_t mode, uint8_t icon/*=0*/) {
    if (process != Confirm && process != Popup && process != Wait) last_process = process;
    if ((process == Menu || process == Wait || process == File) && mode == Popup) last_selection = selection;
    process = mode;
    Clear_Screen();
    const uint16_t color_bg = GetColor(HMI_datas.popup_bg, Color_Bg_Window);
    DWIN_Draw_Rectangle(0, GetColor(HMI_datas.popup_highlight, Color_White), 13, 59, 259, 351);
    DWIN_Draw_Rectangle(1, GetColor(HMI_datas.popup_bg, Color_Bg_Window), 14, 60, 258, 350);
    const uint8_t ypos = (mode == Popup || mode == Confirm) ? 150 : 230;
    const uint8_t ypos_icon = (mode == Popup || mode == Confirm) ? 74 : 105;
    if (icon > 0) DRAW_IconWTB(ICON, icon, 101, ypos_icon);
    DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.popup_txt, Popup_Text_Color), color_bg, (272 - 8 * strlen_P(FTOP(line1))) / 2, ypos, line1);
    DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.popup_txt, Popup_Text_Color), color_bg, (272 - 8 * strlen_P(FTOP(line2))) / 2, ypos + 30, line2);
    DWIN_Draw_String(true, DWIN_FONT_MENU, GetColor(HMI_datas.popup_txt, Popup_Text_Color), color_bg, (272 - 8 * strlen_P(FTOP(line3))) / 2, ypos + 60, line3);
    if (mode == Popup) {
      selection = 0;
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.ico_confirm_bg, Confirm_Color), 26, 280, 125, 317);
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.ico_cancel_bg , Cancel_Color), 146, 280, 245, 317);
      DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.ico_confirm_txt, Color_White), GetColor(HMI_datas.ico_confirm_bg, Confirm_Color), 39, 290, GET_TEXT_F(MSG_BUTTON_CONFIRM));
      DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.ico_cancel_txt, Color_White), GetColor(HMI_datas.ico_cancel_bg , Cancel_Color), 165, 290, GET_TEXT_F(MSG_BUTTON_CANCEL));
      Popup_Select();
    }
    else if (mode == Confirm) {
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.ico_continue_bg, Confirm_Color), 87, 280, 186, 317);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.popup_highlight, Color_White), 86, 279, 187, 318);
      DWIN_Draw_Rectangle(0, GetColor(HMI_datas.popup_highlight, Color_White), 85, 278, 188, 319);
      DWIN_Draw_String(false, DWIN_FONT_STAT, GetColor(HMI_datas.ico_continue_txt, Color_White), GetColor(HMI_datas.ico_continue_bg, Confirm_Color), 
          #if EXTJYERSUI
            (popup == Level2) ? 104 : 96, 
            290,
            (popup == Level2) ? GET_TEXT_F(MSG_BUTTON_CANCEL) : GET_TEXT_F(MSG_BUTTON_CONTINUE)
          #else
            GET_TEXT_F(MSG_BUTTON_CONTINUE)
          #endif
          );
    }
  }

  void CrealityDWINClass::DWIN_ScreenLock() {
    process = Locked;
    if  (!screenLock.unlocked) {
      screenLock.init();
    }
  }

  void CrealityDWINClass::DWIN_ScreenUnLock() {
    if (screenLock.unlocked) {
      screenLock.unlocked = false;
      if (!printing) Draw_Main_Menu();
      else Draw_Print_Screen();
    }
  }

  void CrealityDWINClass::HMI_ScreenLock() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    screenLock.onEncoder(encoder_diffState);
    if (screenLock.isUnlocked()) DWIN_ScreenUnLock();
  }

  #if HAS_SHORTCUTS
    void CrealityDWINClass::DWIN_Move_Z() {
      process = Short_cuts;
      if  (!shortcuts.quitmenu) {
        shortcuts.initZ();
      }
    }

    void CrealityDWINClass::DWIN_QuitMove_Z() {
      if (shortcuts.quitmenu) {
        shortcuts.quitmenu = false;
        queue.inject(F("M84"));
        Draw_Main_Menu();
      }
    }

    void CrealityDWINClass::HMI_Move_Z() {
      EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
      if (encoder_diffState == ENCODER_DIFF_NO) return;
      shortcuts.onEncoderZ(encoder_diffState);
      if (shortcuts.isQuitedZ()) DWIN_QuitMove_Z();
    }
  #endif

  void CrealityDWINClass::Viewmesh() {
    Clear_Screen(4);
    Draw_Title(GET_TEXT_F(MSG_MESH_VIEW));
    flag_viewmesh = true;
    last_process = process;
    last_selection = selection;
    process = Confirm;
    popup = viewmesh;
  #if HAS_MESH
    if (planner.leveling_active) {
          mesh_conf.last_viewer_asymmetric_range = mesh_conf.viewer_asymmetric_range;
          mesh_conf.last_viewer_print_value = mesh_conf.viewer_print_value;
          mesh_conf.viewer_asymmetric_range = true ;
          mesh_conf.viewer_print_value = true ;
          DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, KEY_Y_START, DWIN_WIDTH-2, DWIN_HEIGHT-2);
          mesh_conf.Draw_Bed_Mesh();
          mesh_conf.Set_Mesh_Viewer_Status();
    }
    else Confirm_Handler(LevelError);
  #endif
    DWIN_Draw_Rectangle(1, Confirm_Color, 87, 406, 186, 443);
    DWIN_Draw_Rectangle(0, Color_White, 86, 405, 187, 444);
    DWIN_Draw_Rectangle(0, Color_White, 85, 404, 188, 445);
    DWIN_Draw_String(false, DWIN_FONT_STAT, Color_White, Color_Bg_Window, 96, 416, GET_TEXT_F(MSG_BUTTON_CONTINUE));
  }

  void MarlinUI::kill_screen(FSTR_P const error, FSTR_P const component) {
    CrealityDWIN.Draw_Popup(GET_TEXT_F(MSG_KILLED), error, GET_TEXT_F(MSG_SWITCH_PS_OFF), Wait, ICON_BLTouch);
  }

  void CrealityDWINClass::Popup_Select(bool stflag/*=false*/) {
    const uint16_t c1 = (selection == 0) ? GetColor(HMI_datas.popup_highlight, Color_White) : GetColor(HMI_datas.popup_bg, Color_Bg_Window),
                  c2 = (selection == 0) ? GetColor(HMI_datas.popup_bg, Color_Bg_Window) : GetColor(HMI_datas.popup_highlight, Color_White);
    DWIN_Draw_Rectangle(0, c1, 25, (stflag) ? 425 : 279, 126, (stflag) ? 464 : 318);
    DWIN_Draw_Rectangle(0, c1, 24, (stflag) ? 424 : 278, 127, (stflag) ? 465 : 319);
    DWIN_Draw_Rectangle(0, c2, 145, (stflag) ? 425 : 279, 246, (stflag) ? 464 : 318);
    DWIN_Draw_Rectangle(0, c2, 144, (stflag) ? 424 : 278, 247, (stflag) ? 465 : 319);
  }

  void CrealityDWINClass::Update_Status_Bar(bool refresh/*=false*/) {
    static bool new_msg;
    static uint8_t msgscrl = 0;
    static char lastmsg[128];
    if (strcmp(lastmsg, statusmsg) != 0 || refresh) {
      strcpy(lastmsg, statusmsg);
      msgscrl = 0;
      new_msg = true;
    }
    size_t len = strlen(statusmsg);
    int8_t pos = len;
    if (pos > STATUS_CHAR_LIMIT) {
      pos -= msgscrl;
      len = _MIN((size_t)pos, (size_t)STATUS_CHAR_LIMIT);
      char dispmsg[len + 1];
      if (pos >= 0) {
        LOOP_L_N(i, len) dispmsg[i] = statusmsg[i + msgscrl];
      }
      else {
        LOOP_L_N(i, STATUS_CHAR_LIMIT + pos) dispmsg[i] = ' ';
        LOOP_S_L_N(i, STATUS_CHAR_LIMIT + pos, STATUS_CHAR_LIMIT) dispmsg[i] = statusmsg[i - (STATUS_CHAR_LIMIT + pos)];
      }
      dispmsg[len] = '\0';
      if (process == Print) {
        DWIN_Draw_Rectangle(1, Color_Grey, 8, 214, DWIN_WIDTH - 8, 238);
        const int8_t npos = (DWIN_WIDTH - STATUS_CHAR_LIMIT * MENU_CHR_W) / 2;
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.status_bar_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 219, dispmsg);
      }
      else {
        DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 352, DWIN_WIDTH - 8, 376);
        const int8_t npos = (DWIN_WIDTH - STATUS_CHAR_LIMIT * MENU_CHR_W) / 2;
        DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.status_bar_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 357, dispmsg);
      }
      if (-pos >= STATUS_CHAR_LIMIT) msgscrl = 0;
      msgscrl++;
    }
    else {
      if (new_msg) {
        new_msg = false;
        if (process == Print) {
          DWIN_Draw_Rectangle(1, Color_Grey, 8, 214, DWIN_WIDTH - 8, 238);
          const int8_t npos = (DWIN_WIDTH - strlen(statusmsg) * MENU_CHR_W) / 2;
          DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.status_bar_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 219, statusmsg);
        }
        else {
          DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 8, 352, DWIN_WIDTH - 8, 376);
          const int8_t npos = (DWIN_WIDTH - strlen(statusmsg) * MENU_CHR_W) / 2;
          DWIN_Draw_String(false, DWIN_FONT_MENU, GetColor(HMI_datas.status_bar_text, Color_White), GetColor(HMI_datas.background, Color_Bg_Black), npos, 357, statusmsg);
        }
      }
    }
  }

  void CrealityDWINClass::Draw_Keyboard(bool restrict, bool numeric, uint8_t selected, bool uppercase/*=false*/, bool lock/*=false*/) {
    process = Keyboard;
    keyboard_restrict = restrict;
    numeric_keyboard = numeric;
    DWIN_Draw_Rectangle(0, Color_White, 0, KEY_Y_START, DWIN_WIDTH-2, DWIN_HEIGHT-2);
    DWIN_Draw_Rectangle(1, Color_Bg_Black, 1, KEY_Y_START+1, DWIN_WIDTH-3, DWIN_HEIGHT-3);
    LOOP_L_N(i, 36) Draw_Keys(i, (i == selected), uppercase, lock);
  }

  void CrealityDWINClass::Draw_Keys(uint8_t index, bool selected, bool uppercase/*=false*/, bool lock/*=false*/) {
    const char *keys;
    if (numeric_keyboard) keys = "1234567890&<>(){}[]*\"\':;!?";
    else keys = (uppercase) ? "QWERTYUIOPASDFGHJKLZXCVBNM" : "qwertyuiopasdfghjklzxcvbnm";
    #define KEY_X1(x) x*KEY_WIDTH+KEY_INSET+KEY_PADDING
    #define KEY_X2(x) (x+1)*KEY_WIDTH+KEY_INSET-KEY_PADDING
    #define KEY_Y1(y) KEY_Y_START+KEY_INSET+KEY_PADDING+y*KEY_HEIGHT
    #define KEY_Y2(y) KEY_Y_START+KEY_INSET-KEY_PADDING+(y+1)*KEY_HEIGHT

    const uint8_t rowCount[3] = {10, 9, 7};
    const float xOffset[3] = {0, 0.5f*KEY_WIDTH, 1.5f*KEY_WIDTH};

    if (index < 28) {
      if (index == 19) {
        DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(0), KEY_Y1(2), KEY_X2(0)+xOffset[1], KEY_Y2(2));
        DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(0)+1, KEY_Y1(2)+1, KEY_X2(0)+xOffset[1]-1, KEY_Y2(2)-1);
        if (!numeric_keyboard) {
          if (lock) {
            DWIN_Draw_Line(Select_Color, KEY_X1(0)+17, KEY_Y1(2)+16, KEY_X1(0)+25, KEY_Y1(2)+8);
            DWIN_Draw_Line(Select_Color, KEY_X1(0)+17, KEY_Y1(2)+16, KEY_X1(0)+9, KEY_Y1(2)+8);
          }
          else {
            DWIN_Draw_Line((uppercase) ? Select_Color : Color_White, KEY_X1(0)+17, KEY_Y1(2)+8, KEY_X1(0)+25, KEY_Y1(2)+16);
            DWIN_Draw_Line((uppercase) ? Select_Color : Color_White, KEY_X1(0)+17, KEY_Y1(2)+8, KEY_X1(0)+9, KEY_Y1(2)+16);
          }
        }
      }
      else if (index == 27) {
        DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(7)+xOffset[2], KEY_Y1(2), KEY_X2(9), KEY_Y2(2));
        DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(7)+xOffset[2]+1, KEY_Y1(2)+1, KEY_X2(9)-1, KEY_Y2(2)-1);
        DWIN_Draw_String(true, DWIN_FONT_MENU, Color_Red, Color_Bg_Black, KEY_X1(7)+xOffset[2]+3, KEY_Y1(2)+5, F("<--"));
      }
      else {
        if (index > 19) index--;
        if (index > 27) index--;
        uint8_t y, x;
        if (index < rowCount[0]) y = 0, x = index;
        else if (index < (rowCount[0]+rowCount[1])) y = 1, x = index-rowCount[0];
        else y = 2, x = index-(rowCount[0]+rowCount[1]);
        const char keyStr[2] = {keys[(y>0)*rowCount[0]+(y>1)*rowCount[1]+x], '\0'};
        DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(x)+xOffset[y], KEY_Y1(y), KEY_X2(x)+xOffset[y], KEY_Y2(y));
        DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(x)+xOffset[y]+1, KEY_Y1(y)+1, KEY_X2(x)+xOffset[y]-1, KEY_Y2(y)-1);
        DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(x)+xOffset[y]+5, KEY_Y1(y)+5, keyStr);
        if (keyboard_restrict && numeric_keyboard && index > 9) {
          DWIN_Draw_Line(Color_Light_Red, KEY_X1(x)+xOffset[y]+1, KEY_Y1(y)+1, KEY_X2(x)+xOffset[y]-1, KEY_Y2(y)-1);
          DWIN_Draw_Line(Color_Light_Red, KEY_X1(x)+xOffset[y]+1, KEY_Y2(y)-1, KEY_X2(x)+xOffset[y]-1, KEY_Y1(y)+1);
        }
      }
    }
    else {
      switch (index) {
        case 28:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(0), KEY_Y1(3), KEY_X2(0)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(0)+1, KEY_Y1(3)+1, KEY_X2(0)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(0)-1, KEY_Y1(3)+5, F("?123"));
          break;
        case 29:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(1)+xOffset[1], KEY_Y1(3), KEY_X2(1)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(1)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(1)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(1)+xOffset[1]+5, KEY_Y1(3)+5, F("-"));
          break;
        case 30:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(2)+xOffset[1], KEY_Y1(3), KEY_X2(2)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(2)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(2)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(2)+xOffset[1]+5, KEY_Y1(3)+5, F("_"));
          break;
        case 31:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(3)+xOffset[1], KEY_Y1(3), KEY_X2(5)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(3)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(5)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(3)+xOffset[1]+14, KEY_Y1(3)+5, F("Space"));
          if (keyboard_restrict) {
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(3)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(5)+xOffset[1]-1, KEY_Y2(3)-1);
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(3)+xOffset[1]+1, KEY_Y2(3)-1, KEY_X2(5)+xOffset[1]-1, KEY_Y1(3)+1);
          }
          break;
        case 32:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(6)+xOffset[1], KEY_Y1(3), KEY_X2(6)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(6)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(6)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(6)+xOffset[1]+7, KEY_Y1(3)+5, F("."));
          if (keyboard_restrict) {
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(6)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(6)+xOffset[1]-1, KEY_Y2(3)-1);
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(6)+xOffset[1]+1, KEY_Y2(3)-1, KEY_X2(6)+xOffset[1]-1, KEY_Y1(3)+1);
          }
          break;
        case 33:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(7)+xOffset[1], KEY_Y1(3), KEY_X2(7)+xOffset[1], KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(7)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(7)+xOffset[1]-1, KEY_Y2(3)-1);
          DWIN_Draw_String(false, DWIN_FONT_MENU, Color_White, Color_Bg_Black, KEY_X1(7)+xOffset[1]+4, KEY_Y1(3)+5, F("/"));
          if (keyboard_restrict) {
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(7)+xOffset[1]+1, KEY_Y1(3)+1, KEY_X2(7)+xOffset[1]-1, KEY_Y2(3)-1);
            DWIN_Draw_Line(Color_Light_Red, KEY_X1(7)+xOffset[1]+1, KEY_Y2(3)-1, KEY_X2(7)+xOffset[1]-1, KEY_Y1(3)+1);
          }
          break;
        case 34:
          DWIN_Draw_Rectangle(0, Color_Light_Blue, KEY_X1(7)+xOffset[2], KEY_Y1(3), KEY_X2(9), KEY_Y2(3));
          DWIN_Draw_Rectangle(0, (selected) ? Select_Color : Color_Bg_Black, KEY_X1(7)+xOffset[2]+1, KEY_Y1(3)+1, KEY_X2(9)-1, KEY_Y2(3)-1);
          DWIN_Draw_String(true, DWIN_FONT_MENU, Color_Cyan, Color_Bg_Black, KEY_X1(7)+xOffset[2]+3, KEY_Y1(3)+5, F("-->"));
          break;
      }
    }
  }


  /* Menu Item Config */

  void CrealityDWINClass::Menu_Item_Handler(uint8_t menu, uint8_t item, bool draw/*=true*/) {
    uint8_t row = item - scrollpos;
    string buf;

    // #if HAS_LEVELING
    //   static bool level_state;
    // #endif
    switch (menu) {
      case Prepare:

        #define PREPARE_BACK 0
        #define PREPARE_MOVE (PREPARE_BACK + 1)
        #define PREPARE_DISABLE (PREPARE_MOVE + 1)
        #define PREPARE_HOME (PREPARE_DISABLE + 1)
        #define PREPARE_MANUALLEVEL (PREPARE_HOME + 1)
        #define PREPARE_ZOFFSET (PREPARE_MANUALLEVEL + ENABLED(HAS_ZOFFSET_ITEM))
        #define PREPARE_PREHEAT (PREPARE_ZOFFSET + ENABLED(HAS_PREHEAT))
        #define PREPARE_COOLDOWN (PREPARE_PREHEAT + EITHER(HAS_HOTEND, HAS_HEATED_BED))
        #define PREPARE_CHANGEFIL (PREPARE_COOLDOWN + ENABLED(ADVANCED_PAUSE_FEATURE))
        #define PREPARE_LASERMODE (PREPARE_CHANGEFIL + 1) // mmm
        #define PREPARE_ACTIONCOMMANDS (PREPARE_LASERMODE + 1)
        #define PREPARE_TOTAL PREPARE_ACTIONCOMMANDS

        switch (item) {
          case PREPARE_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Main_Menu(1);
            break;
          case PREPARE_MOVE:
            if (draw)
              Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_MOVE_AXIS), nullptr, true);
            else
              Draw_Menu(Move);
            break;
          case PREPARE_DISABLE:
            if (draw)
              Draw_Menu_Item(row, ICON_CloseMotor, GET_TEXT_F(MSG_DISABLE_STEPPERS));
            else
              queue.inject(F("M84"));
            break;
          case PREPARE_HOME:
            if (draw)
              Draw_Menu_Item(row, ICON_SetHome, GET_TEXT_F(MSG_HOMING), nullptr, true);
            else
              Draw_Menu(HomeMenu);
            break;
          case PREPARE_MANUALLEVEL:
            if (draw)
              Draw_Menu_Item(row, ICON_PrintSize, GET_TEXT_F(MSG_BED_TRAMMING_MANUAL), nullptr, true);
            else {
              if (axes_should_home()) {
                Popup_Handler(Home);
                gcode.home_all_axes(true);
              }
              #if HAS_LEVELING
                level_state = planner.leveling_active;
                set_bed_leveling_enabled(false);
              #endif
              Draw_Menu(ManualLevel);
            }
            break;

          #if HAS_ZOFFSET_ITEM
            case PREPARE_ZOFFSET:
              if (draw)
                Draw_Menu_Item(row, ICON_Zoffset, GET_TEXT_F(MSG_OFFSET_Z), nullptr, true);
              else {
                #if HAS_LEVELING
                  level_state = planner.leveling_active;
                  #if NONE(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN, USE_PROBE_FOR_Z_HOMING)
                    set_bed_leveling_enabled(true);
                  #else
                    set_bed_leveling_enabled(false);
                  #endif
                #endif
                #if !HAS_BED_PROBE
                  gcode.process_subcommands_now(F("M211 S0"));
                #endif
                Draw_Menu(ZOffset);
              }
              break;
          #endif

          #if HAS_PREHEAT
            case PREPARE_PREHEAT:
              if (draw)
                Draw_Menu_Item(row, ICON_Temperature, GET_TEXT_F(MSG_PREHEAT), nullptr, true);
              else
                Draw_Menu(Preheat);
              break;
          #endif

          #if HAS_HOTEND || HAS_HEATED_BED
            case PREPARE_COOLDOWN:
              if (draw)
                Draw_Menu_Item(row, ICON_Cool, GET_TEXT_F(MSG_COOLDOWN));
              else {
                thermalManager.cooldown();
                Update_Status(GET_TEXT(MSG_COOLDOWN));
              }
              break;
          #endif
          case PREPARE_LASERMODE:
            if (draw)
              Draw_Menu_Item(row, ICON_StockConfiguration, F("Laser Mode"));
            else
              planner.laserMode = !planner.laserMode;
            Draw_Checkbox(row, planner.laserMode);
            break;
          #if ENABLED(HOST_ACTION_COMMANDS)
            case PREPARE_ACTIONCOMMANDS:
            if (draw)
              Draw_Menu_Item(row, ICON_SetHome, GET_TEXT_F(MSG_HOST_ACTIONS), nullptr, true);
            else 
              Draw_Menu(HostActions);
            break;
          #endif

          #if ENABLED(ADVANCED_PAUSE_FEATURE)
            case PREPARE_CHANGEFIL:
              if (draw) {
                Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_FILAMENTCHANGE)
                  #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
                    , nullptr, true
                  #endif
                );
              }
              else {
                #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
                  Draw_Menu(ChangeFilament);
                #else
                  LCD_MESSAGE(MSG_FILAMENT_PARK_ENABLED);
                  #if ENABLED(NOZZLE_PARK_FEATURE)
                    queue.inject(F("G28O\nG27 P2"));
                  #else
                    sprintf_P(cmd, PSTR("G28O\nG0 F4000 X%i Y%i\nG0 F3000 Z%i"), TERN(EXTJYERSUI, HMI_datas.Park_point.x, 240) , TERN(EXTJYERSUI, HMI_datas.Park_point.y, 220), TERN(EXTJYERSUI, HMI_datas.Park_point.z, 20));
                    queue.inject(cmd);
                  #endif
                  if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp)
                    Popup_Handler(ETemp);
                  else {
                    flag_chg_fil = true;
                    if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                      Popup_Handler(Heating);
                      Update_Status(GET_TEXT(MSG_HEATING));
                      thermalManager.wait_for_hotend(0);
                    }
                    Popup_Handler(FilChange);
                    Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_INIT));
                    sprintf_P(cmd, PSTR("M600 B1 R%i"), thermalManager.temp_hotend[0].target);
                    gcode.process_subcommands_now(cmd);
                    flag_chg_fil = false;
                    Draw_Menu(Prepare, PREPARE_CHANGEFIL);
                  }
                #endif
              }
              break;
          #endif
        }
        break;

      case HomeMenu:

        #define HOME_BACK  0
        #define HOME_ALL   (HOME_BACK + 1)
        #define HOME_X     (HOME_ALL + 1)
        #define HOME_Y     (HOME_X + 1)
        #define HOME_Z     (HOME_Y + 1)
        #define HOME_SET   (HOME_Z + 1)
        #define HOME_TOTAL HOME_SET

        switch (item) {
          case HOME_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Prepare, PREPARE_HOME);
            break;
          case HOME_ALL:
            if (draw)
              Draw_Menu_Item(row, ICON_Homing, GET_TEXT_F(MSG_AUTO_HOME));
            else {
              Popup_Handler(Home);
              gcode.home_all_axes(true);
              Redraw_Menu();
            }
            break;
          case HOME_X:
            if (draw)
              Draw_Menu_Item(row, ICON_MoveX, GET_TEXT_F(MSG_AUTO_HOME_X));
            else {
              Popup_Handler(Home);
              gcode.process_subcommands_now(F("G28 X"));
              planner.synchronize();
              Redraw_Menu();
            }
            break;
          case HOME_Y:
            if (draw)
              Draw_Menu_Item(row, ICON_MoveY, GET_TEXT_F(MSG_AUTO_HOME_Y));
            else {
              Popup_Handler(Home);
              gcode.process_subcommands_now(F("G28 Y"));
              planner.synchronize();
              Redraw_Menu();
            }
            break;
          case HOME_Z:
            if (draw)
              Draw_Menu_Item(row, ICON_MoveZ, GET_TEXT_F(MSG_AUTO_HOME_Z));
            else {
              Popup_Handler(Home);
              gcode.process_subcommands_now(F("G28 Z"));
              planner.synchronize();
              Redraw_Menu();
            }
            break;
          case HOME_SET:
            if (draw)
              Draw_Menu_Item(row, ICON_SetHome, GET_TEXT_F(MSG_SET_HOME_OFFSETS));
            else {
              gcode.process_subcommands_now(F("G92 X0 Y0 Z0"));
              AudioFeedback();
            }
            break;
        }
        break;

      case Move:

        #define MOVE_BACK 0
        #define MOVE_X (MOVE_BACK + 1)
        #define MOVE_Y (MOVE_X + 1)
        #define MOVE_Z (MOVE_Y + 1)
        #define MOVE_E (MOVE_Z + ENABLED(HAS_HOTEND))
        #define MOVE_P (MOVE_E + ENABLED(HAS_BED_PROBE))
        #define MOVE_LIVE (MOVE_P + 1)
        #define MOVE_TOTAL MOVE_LIVE

        switch (item) {
          case MOVE_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else {
              #if HAS_BED_PROBE
                probe_deployed = false;
                probe.set_deployed(probe_deployed);
              #endif
              Draw_Menu(Prepare, PREPARE_MOVE);
            }
            break;
          case MOVE_X:
            if (draw) {
              Draw_Menu_Item(row, ICON_MoveX, GET_TEXT_F(MSG_MOVE_X));
              Draw_Float(current_position.x, row, false);
            }
            else
              Modify_Value(current_position.x, X_MIN_POS, X_MAX_POS, 10);
            break;
          case MOVE_Y:
            if (draw) {
              Draw_Menu_Item(row, ICON_MoveY, GET_TEXT_F(MSG_MOVE_Y));
              Draw_Float(current_position.y, row);
            }
            else
              Modify_Value(current_position.y, Y_MIN_POS, Y_MAX_POS, 10);
            break;
          case MOVE_Z:
            if (draw) {
              Draw_Menu_Item(row, ICON_MoveZ, GET_TEXT_F(MSG_MOVE_Z));
              Draw_Float(current_position.z, row);
            }
            else
              Modify_Value(current_position.z, Z_MIN_POS, Z_MAX_POS, 10);
            break;

          #if HAS_HOTEND
            case MOVE_E:
              if (draw) {
                Draw_Menu_Item(row, ICON_Extruder, GET_TEXT_F(MSG_MOVE_E));
                current_position.e = 0;
                sync_plan_position();
                Draw_Float(current_position.e, row);
              }
              else {
                if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp) {
                  Popup_Handler(ETemp);
                }
                else {
                  if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                    Popup_Handler(Heating);
                    Update_Status(GET_TEXT(MSG_HEATING));
                    thermalManager.wait_for_hotend(0);
                    Update_Status("");
                    Redraw_Menu();
                  }
                  current_position.e = 0;
                  sync_plan_position();
                  Modify_Value(current_position.e, -1000, 1000, 10);
                }
              }
            break;
          #endif // HAS_HOTEND

          #if HAS_BED_PROBE
            case MOVE_P:
              if (draw) {
                Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_MANUAL_DEPLOY));
                Draw_Checkbox(row, probe_deployed);
              }
              else {
                probe_deployed = !probe_deployed;
                probe.set_deployed(probe_deployed);
                Draw_Checkbox(row, probe_deployed);
              }
              break;
          #endif

          case MOVE_LIVE:
            if (draw) {
              Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_LIVE_ADJUSTMENT));
              Draw_Checkbox(row, livemove);
            }
            else {
              livemove = !livemove;
              Draw_Checkbox(row, livemove);
            }
            break;
        }
        break;
      case ManualLevel:

        #define MLEVEL_BACK 0
        #define MLEVEL_PROBE (MLEVEL_BACK + ENABLED(HAS_BED_PROBE))
        #define MLEVEL_BL (MLEVEL_PROBE + 1)
        #define MLEVEL_TL (MLEVEL_BL + 1)
        #define MLEVEL_TR (MLEVEL_TL + 1)
        #define MLEVEL_BR (MLEVEL_TR + 1)
        #define MLEVEL_C (MLEVEL_BR + 1)
        #define MLEVEL_ZPOS (MLEVEL_C + 1)
        #define MLEVEL_TOTAL MLEVEL_ZPOS

        static float mlev_z_pos = 0;
        static bool use_probe = false;

        switch (item) {
          case MLEVEL_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else {
              TERN_(HAS_LEVELING, set_bed_leveling_enabled(level_state));
              if (flag_shortcut) { flag_shortcut = false; Draw_Main_Menu(1); }
              else Draw_Menu(Prepare, PREPARE_MANUALLEVEL);
            }
            break;
          #if HAS_BED_PROBE
            case MLEVEL_PROBE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Zoffset, GET_TEXT_F(MSG_ZPROBE_ENABLE));
                Draw_Checkbox(row, use_probe);
              }
              else {
                use_probe = !use_probe;
                Draw_Checkbox(row, use_probe);
                if (use_probe) {
                  Popup_Handler(Level);
                  do_z_clearance(Z_HOMING_HEIGHT);
                  corner_avg = 0;
                  #define PROBE_X_MIN _MAX(0 + corner_pos, X_MIN_POS + probe.offset.x, X_MIN_POS + PROBING_MARGIN) - probe.offset.x
                  #define PROBE_X_MAX _MIN((X_BED_SIZE + X_MIN_POS) - corner_pos, X_MAX_POS + probe.offset.x, X_MAX_POS - PROBING_MARGIN) - probe.offset.x
                  #define PROBE_Y_MIN _MAX(0 + corner_pos, Y_MIN_POS + probe.offset.y, Y_MIN_POS + PROBING_MARGIN) - probe.offset.y
                  #define PROBE_Y_MAX _MIN((Y_BED_SIZE + Y_MIN_POS) - corner_pos, Y_MAX_POS + probe.offset.y, Y_MAX_POS - PROBING_MARGIN) - probe.offset.y
                  zval = probe.probe_at_point(PROBE_X_MIN, PROBE_Y_MIN, PROBE_PT_RAISE, 0, false);
                  if (isnan(zval)) {
                      Update_Status(GET_TEXT(MSG_ZPROBE_UNREACHABLE));
                      Redraw_Menu();
                  }
                  corner_avg += zval;
                  zval = probe.probe_at_point(PROBE_X_MIN, PROBE_Y_MAX, PROBE_PT_RAISE, 0, false);
                  if (isnan(zval)) {
                      Update_Status(GET_TEXT(MSG_ZPROBE_UNREACHABLE));
                      Redraw_Menu();
                  }
                  corner_avg += zval;
                  zval = probe.probe_at_point(PROBE_X_MAX, PROBE_Y_MAX, PROBE_PT_RAISE, 0, false);
                  if (isnan(zval)) {
                      Update_Status(GET_TEXT(MSG_ZPROBE_UNREACHABLE));
                      Redraw_Menu();
                  }
                  corner_avg += zval;
                  zval = probe.probe_at_point(PROBE_X_MAX, PROBE_Y_MIN, PROBE_PT_STOW, 0, false);
                  if (isnan(zval)) {
                      Update_Status(GET_TEXT(MSG_ZPROBE_UNREACHABLE));
                      Redraw_Menu();
                  }
                  corner_avg += zval; 
                  corner_avg /= 4;
                  Redraw_Menu();
                }
              }
              break;
          #endif
          case MLEVEL_BL:
            if (draw)
              Draw_Menu_Item(row, ICON_AxisBL, GET_TEXT_F(MSG_LEVBED_FL));
            else {
              Popup_Handler(MoveWait);
              if (use_probe) {
                #if HAS_BED_PROBE
                  sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s"), dtostrf(PROBE_X_MIN, 1, 3, str_1), dtostrf(PROBE_Y_MIN, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                  planner.synchronize();
                  Popup_Handler(ManualProbing);
                #endif
              }
              else {
                sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s\nG0 F300 Z%s"), dtostrf(corner_pos, 1, 3, str_1), dtostrf(corner_pos, 1, 3, str_2), dtostrf(mlev_z_pos, 1, 3, str_3));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                Redraw_Menu();
              }
            }
            break;
          case MLEVEL_TL:
            if (draw)
              Draw_Menu_Item(row, ICON_AxisTL, GET_TEXT_F(MSG_LEVBED_BL));
            else {
              Popup_Handler(MoveWait);
              if (use_probe) {
                #if HAS_BED_PROBE
                  sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s"), dtostrf(PROBE_X_MIN, 1, 3, str_1), dtostrf(PROBE_Y_MAX, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                  planner.synchronize();
                  Popup_Handler(ManualProbing);
                #endif
              }
              else {
                sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s\nG0 F300 Z%s"), dtostrf(corner_pos, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) - corner_pos, 1, 3, str_2), dtostrf(mlev_z_pos, 1, 3, str_3));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                Redraw_Menu();
              }
            }
            break;
          case MLEVEL_TR:
            if (draw)
              Draw_Menu_Item(row, ICON_AxisTR, GET_TEXT_F(MSG_LEVBED_BR));
            else {
              Popup_Handler(MoveWait);
              if (use_probe) {
                #if HAS_BED_PROBE
                  sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s"), dtostrf(PROBE_X_MAX, 1, 3, str_1), dtostrf(PROBE_Y_MAX, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                  planner.synchronize();
                  Popup_Handler(ManualProbing);
                #endif
              }
              else {
                sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s\nG0 F300 Z%s"), dtostrf((X_BED_SIZE + X_MIN_POS) - corner_pos, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) - corner_pos, 1, 3, str_2), dtostrf(mlev_z_pos, 1, 3, str_3));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                Redraw_Menu();
              }
            }
            break;
          case MLEVEL_BR:
            if (draw)
              Draw_Menu_Item(row, ICON_AxisBR, GET_TEXT_F(MSG_LEVBED_FR));
            else {
              Popup_Handler(MoveWait);
              if (use_probe) {
                #if HAS_BED_PROBE
                  sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s"), dtostrf(PROBE_X_MAX, 1, 3, str_1), dtostrf(PROBE_Y_MIN, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                  planner.synchronize();
                  Popup_Handler(ManualProbing);
                #endif
              }
              else {
                sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s\nG0 F300 Z%s"), dtostrf((X_BED_SIZE + X_MIN_POS) - corner_pos, 1, 3, str_1), dtostrf(corner_pos, 1, 3, str_2), dtostrf(mlev_z_pos, 1, 3, str_3));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                Redraw_Menu();
              }
            }
            break;
          case MLEVEL_C:
            if (draw)
              Draw_Menu_Item(row, ICON_AxisC, GET_TEXT_F(MSG_LEVBED_C));
            else {
              Popup_Handler(MoveWait);
              if (use_probe) {
                #if HAS_BED_PROBE
                  sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s"), dtostrf((X_BED_SIZE + X_MIN_POS) / 2.0f - probe.offset.x, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) / 2.0f - probe.offset.y, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                  planner.synchronize();
                  Popup_Handler(ManualProbing);
                #endif
              }
              else {
                sprintf_P(cmd, PSTR("G0 F4000\nG0 Z10\nG0 X%s Y%s\nG0 F300 Z%s"), dtostrf((X_BED_SIZE + X_MIN_POS) / 2.0f, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) / 2.0f, 1, 3, str_2), dtostrf(mlev_z_pos, 1, 3, str_3));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                Redraw_Menu();
              }
            }
            break;
          case MLEVEL_ZPOS:
            if (draw) {
              Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_MOVE_Z));
              Draw_Float(mlev_z_pos, row, false, 100);
            }
            else
              Modify_Value(mlev_z_pos, 0, MAX_Z_OFFSET, 100);
            break;
        }
        break;
      #if HAS_ZOFFSET_ITEM
        case ZOffset:

          #define ZOFFSET_BACK 0
          #define ZOFFSET_HOME (ZOFFSET_BACK + 1)
          #define ZOFFSET_MODE (ZOFFSET_HOME + 1)
          #define ZOFFSET_OFFSET (ZOFFSET_MODE + 1)
          #define ZOFFSET_UP (ZOFFSET_OFFSET + 1)
          #define ZOFFSET_DOWN (ZOFFSET_UP + 1)
          #define ZOFFSET_SAVE (ZOFFSET_DOWN + ENABLED(EEPROM_SETTINGS))
          #define ZOFFSET_TOTAL ZOFFSET_SAVE

          switch (item) {
            case ZOFFSET_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else {
                zoffsetmode = 0;
                #if !HAS_BED_PROBE
                  gcode.process_subcommands_now(F("M211 S1"));
                #endif
                //liveadjust = false;
                //adjustonclick = false;
                TERN_(HAS_LEVELING, set_bed_leveling_enabled(level_state));
                if (flag_shortcut) { flag_shortcut = false; Draw_Main_Menu(1); }
                else Draw_Menu(Prepare, PREPARE_ZOFFSET);
              }
              break;
            case ZOFFSET_HOME:
              if (draw)
                Draw_Menu_Item(row, ICON_Homing, GET_TEXT_F(MSG_AUTO_HOME_Z));
              else {
                Popup_Handler(Home);
                gcode.process_subcommands_now(F("G28 Z"));
                Popup_Handler(MoveWait);
                #if ENABLED(Z_SAFE_HOMING)
                  planner.synchronize();
                  sprintf_P(cmd, PSTR("G0 F4000 X%s Y%s"), dtostrf(Z_SAFE_HOMING_X_POINT, 1, 3, str_1), dtostrf(Z_SAFE_HOMING_Y_POINT, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                #else
                  sprintf_P(cmd, PSTR("G0 F4000 X%s Y%s"), dtostrf((X_BED_SIZE + X_MIN_POS) / 2.0f, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) / 2.0f, 1, 3, str_2));
                  gcode.process_subcommands_now(cmd);
                #endif
                gcode.process_subcommands_now(F("G0 F300 Z0"));
                planner.synchronize();
                Redraw_Menu();
              }
              break;
              case ZOFFSET_MODE:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Zoffset, GET_TEXT_F(MSG_LIVE_ADJUSTMENT));
                  Draw_Option(zoffsetmode, zoffset_modes, row);
                }
                else 
                  Modify_Option(zoffsetmode, zoffset_modes, 2);
                break;
            case ZOFFSET_OFFSET:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_OFFSET_Z));
                Draw_Float(zoffsetvalue, row, false, 100);
              }
              else {
                Modify_Value(zoffsetvalue, MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
              }
              break;
            case ZOFFSET_UP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_UP));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
                }
               else {
                if (zoffsetvalue < MAX_Z_OFFSET) {
                  //if (liveadjust || adjustonclick) {
                  if (zoffsetmode != 0) {
                    gcode.process_subcommands_now(F("M290 Z0.01"));
                    planner.synchronize();
                  }
                  zoffsetvalue += 0.01;
                  Draw_Float(zoffsetvalue, row - 1, false, 100);
                }
              }
              break;
            case ZOFFSET_DOWN:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_DOWN));
                Draw_Menu_Item(row, ICON_AxisD, F(cmd));
                }
               else {
                if (zoffsetvalue > MIN_Z_OFFSET) {
                  //if (liveadjust || adjustonclick) {
                  if (zoffsetmode != 0) {
                    gcode.process_subcommands_now(F("M290 Z-0.01"));
                    planner.synchronize();
                  }
                  zoffsetvalue -= 0.01;
                  Draw_Float(zoffsetvalue, row - 2, false, 100);
                }
              }
              break;  
            #if ENABLED(EEPROM_SETTINGS)
              case ZOFFSET_SAVE:
                if (draw)
                  Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_BUTTON_SAVE));
                else
                  AudioFeedback(settings.save());
                break;
            #endif
          }
          break;
      #endif

      #if HAS_PREHEAT
        case Preheat: {
        
          #define PREHEAT_BACK 0
          #define PREHEAT_MODE (PREHEAT_BACK + 1)
          #define PREHEAT_1 (PREHEAT_MODE + 1)
          #define PREHEAT_2 (PREHEAT_1 + (PREHEAT_COUNT >= 2))
          #define PREHEAT_3 (PREHEAT_2 + (PREHEAT_COUNT >= 3))
          #define PREHEAT_4 (PREHEAT_3 + (PREHEAT_COUNT >= 4))
          #define PREHEAT_5 (PREHEAT_4 + (PREHEAT_COUNT >= 5))
          #define PREHEAT_TOTAL PREHEAT_5

          auto do_preheat = [](const uint8_t m) {
            thermalManager.cooldown();
            if (preheatmode == 0 || preheatmode == 1) { ui.preheat_hotend_and_fan(m); }
            if (preheatmode == 0 || preheatmode == 2) ui.preheat_bed(m);
          };

          switch (item) {
            case PREHEAT_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else {
                if (flag_shortcut) { flag_shortcut = false; Draw_Main_Menu(1); }
                   else Draw_Menu(Prepare, PREPARE_PREHEAT); }
              break;
            case PREHEAT_MODE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Homing, GET_TEXT_F(MSG_CONFIGURATION));
                Draw_Option(preheatmode, preheat_modes, row);
              }
              else
                Modify_Option(preheatmode, preheat_modes, 2);
              break;

            #if PREHEAT_COUNT >= 1
              case PREHEAT_1:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_1_LABEL));
                  }
                else 
                  do_preheat(0);
                break;
            #endif

            #if PREHEAT_COUNT >= 2
              case PREHEAT_2:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_2_LABEL));
                  }
                else 
                  do_preheat(1);
                break;
            #endif

            #if PREHEAT_COUNT >= 3
              case PREHEAT_3:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_3_LABEL));
                }              
                else 
                  do_preheat(3);
                break;
            #endif

            #if PREHEAT_COUNT >= 4
              case PREHEAT_4:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_4_LABEL));
                }
                else 
                  do_preheat(3);
                break;
            #endif

            #if PREHEAT_COUNT >= 5
              case PREHEAT_5:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_5_LABEL));
                }
                else 
                  do_preheat(4);
                break;
            #endif
          }
        } break;
      #endif // HAS_PREHEAT

      #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
        case ChangeFilament:

          #define CHANGEFIL_BACK 0
          #define CHANGEFIL_PARKHEAD (CHANGEFIL_BACK + 1)
          #define CHANGEFIL_LOAD (CHANGEFIL_PARKHEAD + 1)
          #define CHANGEFIL_UNLOAD (CHANGEFIL_LOAD + 1)
          #define CHANGEFIL_CHANGE (CHANGEFIL_UNLOAD + 1)
          #define CHANGEFIL_TOTAL CHANGEFIL_CHANGE

          switch (item) {
            case CHANGEFIL_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else {
                if (flag_shortcut) { flag_shortcut = false; Draw_Main_Menu(1); }
                else Draw_Menu(Prepare, PREPARE_CHANGEFIL);
              }
              break;
            case CHANGEFIL_PARKHEAD:
              if (draw)
                Draw_Menu_Item(row, ICON_ParkPos, GET_TEXT_F(MSG_FILAMENT_PARK_ENABLED));
              else {
                #if ENABLED(NOZZLE_PARK_FEATURE)
                    queue.inject(F("G28O\nG27 P2"));
                  #else
                    sprintf_P(cmd, PSTR("G28O\nG0 F4000 X%i Y%i\nG0 F3000 Z%i"), HMI_datas.Park_point.x, HMI_datas.Park_point.y, HMI_datas.Park_point.z);
                    queue.inject(cmd);
                  #endif
              }
              break;
            case CHANGEFIL_LOAD:
              if (draw)
                Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_FILAMENTLOAD));
              else {
                if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp)
                  Popup_Handler(ETemp);
                else {
                  flag_chg_fil = true;
                  if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                    Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING));
                    Popup_Handler(Heating);
                    thermalManager.wait_for_hotend(0);
                  }
                  Popup_Handler(FilLoad);
                  Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD));
                  gcode.process_subcommands_now(F("M701"));
                  planner.synchronize();
                  flag_chg_fil = false;
                  Draw_Menu(ChangeFilament, CHANGEFIL_LOAD);
                  //Redraw_Menu(true, true);
                }
              }
              break;
            case CHANGEFIL_UNLOAD:
              if (draw)
                Draw_Menu_Item(row, ICON_ReadEEPROM, GET_TEXT_F(MSG_FILAMENTUNLOAD));
              else {
                if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp)
                  Popup_Handler(ETemp);
                else {
                  if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                    Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING));
                    Popup_Handler(Heating);
                    thermalManager.wait_for_hotend(0);
                  }
                  Popup_Handler(FilLoad, true);
                  Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD));
                  gcode.process_subcommands_now(F("M702"));
                  planner.synchronize();
                  Draw_Menu(ChangeFilament, CHANGEFIL_UNLOAD);
                  //Redraw_Menu(true, true);
                }
              }
              break;
            case CHANGEFIL_CHANGE:
              if (draw)
                Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_FILAMENTCHANGE));
              else {
                if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp) {
                  Popup_Handler(ETemp);
                }
                else {
                  flag_chg_fil = true;
                  if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                    Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING));
                    Popup_Handler(Heating);
                    thermalManager.wait_for_hotend(0);
                  }
                  Popup_Handler(FilChange);
                  Update_Status(GET_TEXT(MSG_FILAMENT_CHANGE_HEADER));
                  sprintf_P(cmd, PSTR("M600 B1 R%i"), thermalManager.temp_hotend[0].target);
                  gcode.process_subcommands_now(cmd);
                  flag_chg_fil = false;
                  Draw_Menu(ChangeFilament, CHANGEFIL_CHANGE);
                }
              }
              break;
          }
          break;
      #endif // FILAMENT_LOAD_UNLOAD_GCODES

      #if ENABLED(HOST_ACTION_COMMANDS)
        case HostActions:

          #define HOSTACTIONS_BACK 0
          #define HOSTACTIONS_1 (HOSTACTIONS_BACK + 1)
          #define HOSTACTIONS_2 (HOSTACTIONS_1 + 1)
          #define HOSTACTIONS_3 (HOSTACTIONS_2 + 1)
          #define HOSTACTIONS_4 (HOSTACTIONS_3 + ENABLED(HOST_SHUTDOWN_MENU_ITEM))
          #define HOSTACTIONS_TOTAL HOSTACTIONS_4

          switch(item) {
            case HOSTACTIONS_BACK:
              if (draw) 
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Prepare, PREPARE_ACTIONCOMMANDS);
              break;
            case HOSTACTIONS_1:
              if (draw) {
                sd_item_flag = false;
                Draw_Menu_Item(row, ICON_File, action1);
              }
              else {
                if (!strcmp(action1, "-") == 0) hostui.action(F(action1));
              }
              break;
            case HOSTACTIONS_2:
              if (draw) {
                sd_item_flag = false;
                Draw_Menu_Item(row, ICON_File, action2);
              }
              else {
                if (!strcmp(action2, "-") == 0) hostui.action(F(action2));
              }
              break;
            case HOSTACTIONS_3:
              if (draw) {
                sd_item_flag = false;
                Draw_Menu_Item(row, ICON_File, action3);
              }
              else {
                if (!strcmp(action3, "-") == 0) hostui.action(F(action3));
              }
              break;
          #if ENABLED(HOST_SHUTDOWN_MENU_ITEM)
          case HOSTACTIONS_4:
            if (draw) {
			  sd_item_flag = false;
              Draw_Menu_Item(row, ICON_File, "Shutdown");
            }
            else {
              hostui.shutdown(); 
            }
            break;
          #endif
          }
          break;
      #endif

      case Control:

        #define CONTROL_BACK 0
        #define CONTROL_TEMP (CONTROL_BACK + 1)
        #define CONTROL_MOTION (CONTROL_TEMP + 1)
        #define CONTROL_FWRETRACT (CONTROL_MOTION + ENABLED(FWRETRACT))
        #define CONTROL_PARKMENU (CONTROL_FWRETRACT + ENABLED(NOZZLE_PARK_FEATURE))
        #define CONTROL_VISUAL (CONTROL_PARKMENU + 1)
        #define CONTROL_HOSTSETTINGS (CONTROL_VISUAL + 1)
        #define CONTROL_ADVANCED (CONTROL_HOSTSETTINGS + 1)
        #define CONTROL_SAVE (CONTROL_ADVANCED + ENABLED(EEPROM_SETTINGS))
        #define CONTROL_RESTORE (CONTROL_SAVE + ENABLED(EEPROM_SETTINGS))
        #define CONTROL_RESET (CONTROL_RESTORE + ENABLED(EEPROM_SETTINGS))
        #define CONTROL_REBOOT (CONTROL_RESET + 1)
        #define CONTROL_INFO (CONTROL_REBOOT + 1)
        #define CONTROL_TOTAL CONTROL_INFO

        switch (item) {
          case CONTROL_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Main_Menu(2);
            break;
          case CONTROL_TEMP:
            if (draw)
              Draw_Menu_Item(row, ICON_Temperature, GET_TEXT_F(MSG_TEMPERATURE), nullptr, true);
            else
              Draw_Menu(TempMenu);
            break;
          case CONTROL_MOTION:
            if (draw)
              Draw_Menu_Item(row, ICON_Motion, GET_TEXT_F(MSG_MOTION), nullptr, true);
            else
              Draw_Menu(Motion);
            break;
          #if ENABLED(FWRETRACT)
            case CONTROL_FWRETRACT:
            if (draw)
              Draw_Menu_Item(row, ICON_StepE, GET_TEXT_F(MSG_AUTORETRACT), nullptr, true);
            else
              Draw_Menu(FwRetraction);
            break;
          #endif
          #if ENABLED(NOZZLE_PARK_FEATURE)
            case CONTROL_PARKMENU:
            if (draw)
              Draw_Menu_Item(row, ICON_ParkPos, GET_TEXT_F(MSG_FILAMENT_PARK_ENABLED), nullptr, true);
            else
              Draw_Menu(Parkmenu);
            break;
          #endif
          case CONTROL_VISUAL:
            if (draw)
              Draw_Menu_Item(row, ICON_PrintSize, GET_TEXT_F(MSG_VISUAL_SETTINGS), nullptr, true);
            else
              Draw_Menu(Visual);
            break;
          case CONTROL_HOSTSETTINGS:
            if (draw)
              Draw_Menu_Item(row, ICON_Contact, GET_TEXT_F(MSG_HOST_SETTINGS), nullptr, true);
            else 
              Draw_Menu(HostSettings);
            break;
          case CONTROL_ADVANCED:
            if (draw)
              Draw_Menu_Item(row, ICON_Version, GET_TEXT_F(MSG_ADVANCED_SETTINGS), nullptr, true);
            else
              Draw_Menu(Advanced);
            break;
          #if ENABLED(EEPROM_SETTINGS)
            case CONTROL_SAVE:
              if (draw)
                Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_STORE_EEPROM));
              else
                AudioFeedback(settings.save());
              break;
            case CONTROL_RESTORE:
              if (draw)
                Draw_Menu_Item(row, ICON_ReadEEPROM, GET_TEXT_F(MSG_LOAD_EEPROM));
              else
                AudioFeedback(settings.load());
              break;
            case CONTROL_RESET:
              if (draw)
                Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_RESTORE_DEFAULTS));
              else {
                settings.reset();
                AudioFeedback();
              }
              break;
          #endif
          case CONTROL_REBOOT:
            if (draw)
              Draw_Menu_Item(row, ICON_Reboot, GET_TEXT_F(MSG_RESET_PRINTER));
            else {
              RebootPrinter();
            }
            break;
          case CONTROL_INFO:
            if (draw)
              Draw_Menu_Item(row, ICON_Info, GET_TEXT_F(MSG_INFO_SCREEN));
            else
              Draw_Menu(Info);
            break;
        }
        break;

      case TempMenu:

        #define TEMP_BACK 0
        #define TEMP_HOTEND (TEMP_BACK + ENABLED(HAS_HOTEND))
        #define TEMP_BED (TEMP_HOTEND + ENABLED(HAS_HEATED_BED))
        #define TEMP_FAN (TEMP_BED + ENABLED(HAS_FAN))
        #define TEMP_PID (TEMP_FAN + (ANY(HAS_HOTEND, HAS_HEATED_BED) && ANY(PIDTEMP, PIDTEMPBED)))
        #define TEMP_PREHEAT1 (TEMP_PID + (PREHEAT_COUNT >= 1))
        #define TEMP_PREHEAT2 (TEMP_PREHEAT1 + (PREHEAT_COUNT >= 2))
        #define TEMP_PREHEAT3 (TEMP_PREHEAT2 + (PREHEAT_COUNT >= 3))
        #define TEMP_PREHEAT4 (TEMP_PREHEAT3 + (PREHEAT_COUNT >= 4))
        #define TEMP_PREHEAT5 (TEMP_PREHEAT4 + (PREHEAT_COUNT >= 5))
        #define TEMP_TOTAL TEMP_PREHEAT5

        switch (item) {
          case TEMP_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Control, CONTROL_TEMP);
            break;
          #if HAS_HOTEND
            case TEMP_HOTEND:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                Draw_Float(thermalManager.temp_hotend[0].target, row, false, 1);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].target, MIN_E_TEMP, MAX_E_TEMP, 1);
              break;
          #endif
          #if HAS_HEATED_BED
            case TEMP_BED:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                Draw_Float(thermalManager.temp_bed.target, row, false, 1);
              }
              else
                Modify_Value(thermalManager.temp_bed.target, MIN_BED_TEMP, MAX_BED_TEMP, 1);
              break;
          #endif
          #if HAS_FAN
            case TEMP_FAN:
              if (draw) {
                Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                Draw_Float(thermalManager.fan_speed[0], row, false, 1);
              }
              else
                Modify_Value(thermalManager.fan_speed[0], MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
              break;
          #endif
          #if (HAS_HOTEND || HAS_HEATED_BED) && ANY(PIDTEMP, PIDTEMPBED)
            case TEMP_PID:
              if (draw) {
                Draw_Menu_Item(row, ICON_Step, GET_TEXT_F(MSG_PID), nullptr, true);
               }
              else
                Draw_Menu(PID);
              break;
          #endif
          #if PREHEAT_COUNT >= 1
            case TEMP_PREHEAT1:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, F(PREHEAT_1_LABEL), nullptr, true);
              else
                Draw_Menu(Preheat1);
              break;
          #endif
          #if PREHEAT_COUNT >= 2
            case TEMP_PREHEAT2:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, F(PREHEAT_2_LABEL), nullptr, true);
              else
                Draw_Menu(Preheat2);
              break;
          #endif
          #if PREHEAT_COUNT >= 3
            case TEMP_PREHEAT3:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, F(PREHEAT_3_LABEL), nullptr, true);
              else
                Draw_Menu(Preheat3);
              break;
          #endif
          #if PREHEAT_COUNT >= 4
            case TEMP_PREHEAT4:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, F(PREHEAT_4_LABEL), nullptr, true);
              else
                Draw_Menu(Preheat4);
              break;
          #endif
          #if PREHEAT_COUNT >= 5
            case TEMP_PREHEAT5:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, F(PREHEAT_5_LABEL), nullptr, true);
              else
                Draw_Menu(Preheat5);
              break;
          #endif
        }
        break;

      #if (HAS_HOTEND || HAS_HEATED_BED) && ANY(PIDTEMP, PIDTEMPBED)
        case PID:

          #define PID_BACK 0
          #define PID_HOTEND (PID_BACK + (HAS_HOTEND && ENABLED(PIDTEMP)))
          #define PID_BED (PID_HOTEND + (HAS_HEATED_BED && ENABLED(PIDTEMPBED)))
          #define PID_CYCLES (PID_BED + 1)
          #define PID_SAVE (PID_CYCLES +1)
          #define PID_TOTAL PID_SAVE

          static uint8_t PID_cycles = 5;

          switch (item) {
            case PID_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PID);
              break;
            #if HAS_HOTEND && ENABLED(PIDTEMP)
              case PID_HOTEND:
                if (draw)
                  Draw_Menu_Item(row, ICON_HotendTemp, GET_TEXT_F(MSG_HOTEND_PID_AUTOTUNE), nullptr, true);
                else
                  Draw_Menu(HotendPID);
                break;
            #endif
            #if HAS_HEATED_BED && ENABLED(PIDTEMPBED)
              case PID_BED:
                if (draw)
                  Draw_Menu_Item(row, ICON_BedTemp, GET_TEXT_F(MSG_BED_PID_AUTOTUNE), nullptr, true);
                else
                  Draw_Menu(BedPID);
                break;
            #endif
            case PID_CYCLES:
              if (draw) {
                Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_PID_CYCLE));
                Draw_Float(PID_cycles, row, false, 1);
              }
              else
                Modify_Value(PID_cycles, 3, 50, 1);
              break;
            case PID_SAVE:
              if(draw) {
                Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_PID_AUTOTUNE_SAVE));
              }
              else {
                AudioFeedback(settings.save());
              }
              break;
          }
          break;
      #endif // HAS_HOTEND || HAS_HEATED_BED

      #if HAS_HOTEND && ENABLED(PIDTEMP)
        case HotendPID:

          #define HOTENDPID_BACK 0
          #define HOTENDPID_TUNE (HOTENDPID_BACK + 1)
          #define HOTENDPID_TEMP (HOTENDPID_TUNE + 1)
          #define HOTENDPID_FAN (HOTENDPID_TEMP + 1)
          #define HOTENDPID_KP (HOTENDPID_FAN + 1)
          #define HOTENDPID_KI (HOTENDPID_KP + 1)
          #define HOTENDPID_KD (HOTENDPID_KI + 1)
          #define HOTENDPID_TOTAL HOTENDPID_KD

          static uint16_t PID_e_temp = 180;

          switch (item) {
            case HOTENDPID_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(PID, PID_HOTEND);
              break;
            case HOTENDPID_TUNE:
              if (draw)
                Draw_Menu_Item(row, ICON_HotendTemp, GET_TEXT_F(MSG_PID_AUTOTUNE));
              else {
                Popup_Handler(PIDWait);
                sprintf_P(cmd, PSTR("M303 E0 C%i S%i U1"), PID_cycles, PID_e_temp);
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                //Redraw_Menu();
              }
              break;
            case HOTENDPID_TEMP:
              if (draw) {
                Draw_Menu_Item(row, ICON_Temperature, GET_TEXT_F(MSG_TEMPERATURE));
                Draw_Float(PID_e_temp, row, false, 1);
              }
              else
                Modify_Value(PID_e_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
              break;
            #if HAS_FAN
            case HOTENDPID_FAN:
              if (draw) {
                Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                Draw_Float(thermalManager.fan_speed[0], row, false, 1);
              }
              else {
                Modify_Value(thermalManager.fan_speed[0], MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
              }
              break;
            #endif
            case HOTENDPID_KP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_P), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(thermalManager.temp_hotend[0].pid.Kp, row, false, 100);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].pid.Kp, 0, 5000, 100, thermalManager.updatePID);
              break;
            case HOTENDPID_KI:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_I), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(unscalePID_i(thermalManager.temp_hotend[0].pid.Ki), row, false, 100);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].pid.Ki, 0, 5000, 100, thermalManager.updatePID);
              break;
            case HOTENDPID_KD:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_D), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(unscalePID_d(thermalManager.temp_hotend[0].pid.Kd), row, false, 100);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].pid.Kd, 0, 5000, 100, thermalManager.updatePID);
              break;
          }
          break;
      #endif // HAS_HOTEND

      #if HAS_HEATED_BED && ENABLED(PIDTEMPBED)
        case BedPID:

          #define BEDPID_BACK 0
          #define BEDPID_TUNE (BEDPID_BACK + 1)
          #define BEDPID_TEMP (BEDPID_TUNE + 1)
          #define BEDPID_KP (BEDPID_TEMP + 1)
          #define BEDPID_KI (BEDPID_KP + 1)
          #define BEDPID_KD (BEDPID_KI + 1)
          #define BEDPID_TOTAL BEDPID_KD

          static uint16_t PID_bed_temp = 60;

          switch (item) {
            case BEDPID_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(PID, PID_BED);
              break;
            case BEDPID_TUNE:
              if (draw)
                Draw_Menu_Item(row, ICON_HotendTemp, GET_TEXT_F(MSG_PID_AUTOTUNE));
              else {
                Popup_Handler(PIDWait, true);
                sprintf_P(cmd, PSTR("M303 E-1 C%i S%i U1"), PID_cycles, PID_bed_temp);
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
                //Redraw_Menu();
              }
              break;
            case BEDPID_TEMP:
              if (draw) {
                Draw_Menu_Item(row, ICON_Temperature, GET_TEXT_F(MSG_TEMPERATURE));
                Draw_Float(PID_bed_temp, row, false, 1);
              }
              else
                Modify_Value(PID_bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
              break;
            case BEDPID_KP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_P), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(thermalManager.temp_bed.pid.Kp, row, false, 100);
              }
              else {
                Modify_Value(thermalManager.temp_bed.pid.Kp, 0, 5000, 100, thermalManager.updatePID);
              }
              break;
            case BEDPID_KI:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_I), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(unscalePID_i(thermalManager.temp_bed.pid.Ki), row, false, 100);
              }
              else
                Modify_Value(thermalManager.temp_bed.pid.Ki, 0, 5000, 100, thermalManager.updatePID);
              break;
            case BEDPID_KD:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s: "), GET_TEXT(MSG_PID_D), GET_TEXT(MSG_PID_VALUE));
                Draw_Menu_Item(row, ICON_Version, F(cmd));
                Draw_Float(unscalePID_d(thermalManager.temp_bed.pid.Kd), row, false, 100);
              }
              else
                Modify_Value(thermalManager.temp_bed.pid.Kd, 0, 5000, 100, thermalManager.updatePID);
              break;
          }
          break;
      #endif // HAS_HEATED_BED

      #if PREHEAT_COUNT >= 1
        case Preheat1:

          #define PREHEAT1_BACK 0
          #define PREHEAT1_HOTEND (PREHEAT1_BACK + ENABLED(HAS_HOTEND))
          #define PREHEAT1_BED (PREHEAT1_HOTEND + ENABLED(HAS_HEATED_BED))
          #define PREHEAT1_FAN (PREHEAT1_BED + ENABLED(HAS_FAN))
          #define PREHEAT1_TOTAL PREHEAT1_FAN

          switch (item) {
            case PREHEAT1_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PREHEAT1);
              break;
            #if HAS_HOTEND
              case PREHEAT1_HOTEND:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(ui.material_preset[0].hotend_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[0].hotend_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
            #endif
            #if HAS_HEATED_BED
              case PREHEAT1_BED:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Float(ui.material_preset[0].bed_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[0].bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
                break;
            #endif
            #if HAS_FAN
              case PREHEAT1_FAN:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                  Draw_Float(ui.material_preset[0].fan_speed, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[0].fan_speed, MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
                break;
            #endif
          }
          break;
      #endif // PREHEAT_COUNT >= 1

      #if PREHEAT_COUNT >= 2
        case Preheat2:

          #define PREHEAT2_BACK 0
          #define PREHEAT2_HOTEND (PREHEAT2_BACK + ENABLED(HAS_HOTEND))
          #define PREHEAT2_BED (PREHEAT2_HOTEND + ENABLED(HAS_HEATED_BED))
          #define PREHEAT2_FAN (PREHEAT2_BED + ENABLED(HAS_FAN))
          #define PREHEAT2_TOTAL PREHEAT2_FAN

          switch (item) {
            case PREHEAT2_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PREHEAT2);
              break;
            #if HAS_HOTEND
              case PREHEAT2_HOTEND:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(ui.material_preset[1].hotend_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[1].hotend_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
            #endif
            #if HAS_HEATED_BED
              case PREHEAT2_BED:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Float(ui.material_preset[1].bed_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[1].bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
                break;
            #endif
            #if HAS_FAN
              case PREHEAT2_FAN:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                  Draw_Float(ui.material_preset[1].fan_speed, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[1].fan_speed, MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
                break;
            #endif
          }
          break;
      #endif // PREHEAT_COUNT >= 2

      #if PREHEAT_COUNT >= 3
        case Preheat3:

          #define PREHEAT3_BACK 0
          #define PREHEAT3_HOTEND (PREHEAT3_BACK + ENABLED(HAS_HOTEND))
          #define PREHEAT3_BED (PREHEAT3_HOTEND + ENABLED(HAS_HEATED_BED))
          #define PREHEAT3_FAN (PREHEAT3_BED + ENABLED(HAS_FAN))
          #define PREHEAT3_TOTAL PREHEAT3_FAN

          switch (item) {
            case PREHEAT3_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PREHEAT3);
              break;
            #if HAS_HOTEND
              case PREHEAT3_HOTEND:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(ui.material_preset[2].hotend_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[2].hotend_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
            #endif
            #if HAS_HEATED_BED
              case PREHEAT3_BED:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Float(ui.material_preset[2].bed_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[2].bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
                break;
            #endif
            #if HAS_FAN
              case PREHEAT3_FAN:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                  Draw_Float(ui.material_preset[2].fan_speed, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[2].fan_speed, MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
                break;
            #endif
          }
          break;
      #endif // PREHEAT_COUNT >= 3

      #if PREHEAT_COUNT >= 4
        case Preheat4:

          #define PREHEAT4_BACK 0
          #define PREHEAT4_HOTEND (PREHEAT4_BACK + ENABLED(HAS_HOTEND))
          #define PREHEAT4_BED (PREHEAT4_HOTEND + ENABLED(HAS_HEATED_BED))
          #define PREHEAT4_FAN (PREHEAT4_BED + ENABLED(HAS_FAN))
          #define PREHEAT4_TOTAL PREHEAT4_FAN

          switch (item) {
            case PREHEAT4_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PREHEAT4);
              break;
            #if HAS_HOTEND
              case PREHEAT4_HOTEND:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(ui.material_preset[3].hotend_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[3].hotend_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
            #endif
            #if HAS_HEATED_BED
              case PREHEAT4_BED:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Float(ui.material_preset[3].bed_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[3].bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
                break;
            #endif
            #if HAS_FAN
              case PREHEAT4_FAN:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                  Draw_Float(ui.material_preset[3].fan_speed, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[3].fan_speed, MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
                break;
            #endif
          }
          break;
      #endif // PREHEAT_COUNT >= 4

      #if PREHEAT_COUNT >= 5
        case Preheat5:

          #define PREHEAT5_BACK 0
          #define PREHEAT5_HOTEND (PREHEAT5_BACK + ENABLED(HAS_HOTEND))
          #define PREHEAT5_BED (PREHEAT5_HOTEND + ENABLED(HAS_HEATED_BED))
          #define PREHEAT5_FAN (PREHEAT5_BED + ENABLED(HAS_FAN))
          #define PREHEAT5_TOTAL PREHEAT5_FAN

          switch (item) {
            case PREHEAT5_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(TempMenu, TEMP_PREHEAT5);
              break;
            #if HAS_HOTEND
              case PREHEAT5_HOTEND:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(ui.material_preset[4].hotend_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[4].hotend_temp, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
            #endif
            #if HAS_HEATED_BED
              case PREHEAT5_BED:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Float(ui.material_preset[4].bed_temp, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[4].bed_temp, MIN_BED_TEMP, MAX_BED_TEMP, 1);
                break;
            #endif
            #if HAS_FAN
              case PREHEAT5_FAN:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                  Draw_Float(ui.material_preset[4].fan_speed, row, false, 1);
                }
                else
                  Modify_Value(ui.material_preset[4].fan_speed, MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
                break;
            #endif
          }
          break;
      #endif // PREHEAT_COUNT >= 5

      case Motion:

        #define MOTION_BACK 0
        #define MOTION_HOMEOFFSETS (MOTION_BACK + 1)
        #define MOTION_SPEED (MOTION_HOMEOFFSETS + 1)
        #define MOTION_ACCEL (MOTION_SPEED + 1)
        #define MOTION_JERK (MOTION_ACCEL + ENABLED(HAS_CLASSIC_JERK))
        #define MOTION_JD (MOTION_JERK + ENABLED(HAS_JUNCTION_DEVIATION))
        #define MOTION_STEPS (MOTION_JD + 1)
        #define MOTION_INVERT_DIR_EXTR (MOTION_STEPS + (ENABLED(HAS_HOTEND) && EXTJYERSUI))
        #define MOTION_FLOW (MOTION_INVERT_DIR_EXTR + ENABLED(HAS_HOTEND))
        #define MOTION_TOTAL MOTION_FLOW

        switch (item) {
          case MOTION_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Control, CONTROL_MOTION);
            break;
          case MOTION_HOMEOFFSETS:
            if (draw)
              Draw_Menu_Item(row, ICON_SetHome, GET_TEXT_F(MSG_SET_HOME_OFFSETS), nullptr, true);
            else
              Draw_Menu(HomeOffsets);
            break;
          case MOTION_SPEED:
            if (draw)
              Draw_Menu_Item(row, ICON_MaxSpeed, GET_TEXT_F(MSG_MAXSPEED), nullptr, true);
            else
              Draw_Menu(MaxSpeed);
            break;
          case MOTION_ACCEL:
            if (draw) 
              Draw_Menu_Item(row, ICON_MaxAccelerated, GET_TEXT_F(MSG_AMAX_EN), nullptr, true);
            
            else
              Draw_Menu(MaxAcceleration);
            break;
          #if HAS_CLASSIC_JERK
            case MOTION_JERK:
              if (draw)
                Draw_Menu_Item(row, ICON_MaxJerk, GET_TEXT_F(MSG_VEN_JERK), nullptr, true);
              else
                Draw_Menu(MaxJerk);
              break;
          #endif
          #if HAS_JUNCTION_DEVIATION
            case MOTION_JD:
              if (draw)
                Draw_Menu_Item(row, ICON_MaxJerk, GET_TEXT_F(MSG_JUNCTION_DEVIATION_MENU), nullptr, true);
              else
                Draw_Menu(JDmenu);
              break;
          #endif
          case MOTION_STEPS:
            if (draw)
              Draw_Menu_Item(row, ICON_Step, GET_TEXT_F(MSG_STEPS_PER_MM), nullptr, true);
            else
              Draw_Menu(Steps);
            break;
          #if HAS_HOTEND
            #if EXTJYERSUI
              case MOTION_INVERT_DIR_EXTR:
                if (draw) {
                Draw_Menu_Item(row, ICON_Motion, GET_TEXT_F(MSG_EXTRUDER_INVERT));
                Draw_Checkbox(row, HMI_datas.invert_dir_extruder);
              }
              else {
                HMI_datas.invert_dir_extruder = !HMI_datas.invert_dir_extruder;
                DWIN_Invert_Extruder();
                Draw_Checkbox(row, HMI_datas.invert_dir_extruder);
              }
              break;
            #endif
            case MOTION_FLOW:
              if (draw) {
                Draw_Menu_Item(row, ICON_Speed, GET_TEXT_F(MSG_FLOW));
                Draw_Float(planner.flow_percentage[0], row, false, 1);
              }
              else
                Modify_Value(planner.flow_percentage[0], MIN_FLOW_RATE, MAX_FLOW_RATE, 1);
              break;
          #endif
        }
        break;
      #if ENABLED(FWRETRACT)
        case FwRetraction:

          #define FWR_BACK 0
          #define FWR_RET_LENGTH (FWR_BACK + 1)
          #define FWR_RET_SPEED (FWR_RET_LENGTH + 1)
          #define FWR_ZLIFT (FWR_RET_SPEED + 1)
          #define FWR_REC_EXT_LENGTH (FWR_ZLIFT + 1)
          #define FWR_REC_SPEED (FWR_REC_EXT_LENGTH + 1)
          #define FWR_RESET (FWR_REC_SPEED + 1)
          #define FWR_TOTAL FWR_RESET

          switch (item) {
          
            case FWR_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else {
              if (flag_tune) {
                flag_tune = false;
                Redraw_Menu(false, true, true);
              }
              else
                Draw_Menu(Control, CONTROL_FWRETRACT);
            }
            break;
          case FWR_RET_LENGTH:
            if (draw) {
              Draw_Menu_Item(row, ICON_FWRetLength, GET_TEXT_F(MSG_CONTROL_RETRACT));
              Draw_Float(fwretract.settings.retract_length, row, false, 10);
            }
            else
              Modify_Value(fwretract.settings.retract_length, 0, 10, 10);
            break;
          case FWR_RET_SPEED:
            if (draw) {
              Draw_Menu_Item(row, ICON_FWRetSpeed, GET_TEXT_F(MSG_SINGLENOZZLE_RETRACT_SPEED));
              Draw_Float(fwretract.settings.retract_feedrate_mm_s, row, false, 10);
            }
            else
              Modify_Value(fwretract.settings.retract_feedrate_mm_s, 1, 90, 10);
            break;
          case FWR_ZLIFT:
            if (draw) {
              Draw_Menu_Item(row, ICON_FWRetZRaise, GET_TEXT_F(MSG_CONTROL_RETRACT_ZHOP));
              Draw_Float(fwretract.settings.retract_zraise, row, false, 100);
            }
            else
              Modify_Value(fwretract.settings.retract_zraise, 0, 10, 100);
            break;
          case FWR_REC_EXT_LENGTH:
            if (draw) {
              Draw_Menu_Item(row, ICON_FWRecExtLength, GET_TEXT_F(MSG_CONTROL_RETRACT_RECOVER));
              Draw_Float(fwretract.settings.retract_recover_extra, row, false, 10);
            }
            else
              Modify_Value(fwretract.settings.retract_recover_extra, -10, 10, 10);
            break;
          case FWR_REC_SPEED:
            if (draw) {
              Draw_Menu_Item(row, ICON_FWRecSpeed, GET_TEXT_F(MSG_SINGLENOZZLE_UNRETRACT_SPEED));
              Draw_Float(fwretract.settings.retract_recover_feedrate_mm_s, row, false, 10);
            }
            else
              Modify_Value(fwretract.settings.retract_recover_feedrate_mm_s, 1, 90, 10);
            break;
          case FWR_RESET:
            if (draw)
              Draw_Menu_Item(row, ICON_StepE, GET_TEXT_F(MSG_BUTTON_RESET));
            else {
              fwretract.reset();
              Draw_Menu(FwRetraction);
            }
            break;
          }
          break;
      #endif

      #if ENABLED(NOZZLE_PARK_FEATURE)
        case Parkmenu:

          #define PARKMENU_BACK 0
          #define PARKMENU_POSX (PARKMENU_BACK + 1)
          #define PARKMENU_POSY (PARKMENU_POSX + 1)
          #define PARKMENU_POSZ (PARKMENU_POSY + 1)
          #define PARKMENU_TOTAL PARKMENU_POSZ

          switch (item) {
            case PARKMENU_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Control, CONTROL_PARKMENU);
              break;
            case PARKMENU_POSX:
              if (draw) {
                Draw_Menu_Item(row, ICON_ParkPosX, GET_TEXT_F(MSG_FILAMENT_PARK_X));
                Draw_Float(HMI_datas.Park_point.x, row, false, 1);
              }
              else
                Modify_Value(HMI_datas.Park_point.x, 0, X_MAX_POS, 1);
              break;
            case PARKMENU_POSY:
              if (draw) {
                Draw_Menu_Item(row, ICON_ParkPosY, GET_TEXT_F(MSG_FILAMENT_PARK_Y));
                Draw_Float(HMI_datas.Park_point.y, row, false, 1);
              }
              else
                Modify_Value(HMI_datas.Park_point.y, 0, Y_MAX_POS, 1);
              break;
            case PARKMENU_POSZ:
              if (draw) {
                Draw_Menu_Item(row, ICON_ParkPosZ, GET_TEXT_F(MSG_FILAMENT_PARK_Z));
                Draw_Float(HMI_datas.Park_point.z, row, false, 1);
              }
              else
                Modify_Value(HMI_datas.Park_point.z, MIN_PARK_POINT_Z, Z_MAX_POS, 1);
              break;
          }
        break;
      #endif

      case HomeOffsets:

        #define HOMEOFFSETS_BACK 0
        #define HOMEOFFSETS_XOFFSET (HOMEOFFSETS_BACK + 1)
        #define HOMEOFFSETS_YOFFSET (HOMEOFFSETS_XOFFSET + 1)
        #define HOMEOFFSETS_TOTAL HOMEOFFSETS_YOFFSET

        switch (item) {
          case HOMEOFFSETS_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Motion, MOTION_HOMEOFFSETS);
            break;
          case HOMEOFFSETS_XOFFSET:
            if (draw) {
              Draw_Menu_Item(row, ICON_StepX, GET_TEXT_F(MSG_HOME_OFFSET_X));
              Draw_Float(home_offset.x, row, false, 100);
            }
            else
              Modify_Value(home_offset.x, -MAX_XY_OFFSET, MAX_XY_OFFSET, 100);
            break;
          case HOMEOFFSETS_YOFFSET:
            if (draw) {
              Draw_Menu_Item(row, ICON_StepY, GET_TEXT_F(MSG_HOME_OFFSET_Y));
              Draw_Float(home_offset.y, row, false, 100);
            }
            else
              Modify_Value(home_offset.y, -MAX_XY_OFFSET, MAX_XY_OFFSET, 100);
            break;
        }
        break;
      case MaxSpeed:

        #define SPEED_BACK 0
        #define SPEED_X (SPEED_BACK + 1)
        #define SPEED_Y (SPEED_X + 1)
        #define SPEED_Z (SPEED_Y + 1)
        #define SPEED_E (SPEED_Z + ENABLED(HAS_HOTEND))
        #define SPEED_TOTAL SPEED_E

        switch (item) {
          case SPEED_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Motion, MOTION_SPEED);
            break;
          case SPEED_X:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxSpeedX, GET_TEXT_F(MSG_MAXSPEED_X));
              Draw_Float(planner.settings.max_feedrate_mm_s[X_AXIS], row, false, 1);
            }
            else
              Modify_Value(planner.settings.max_feedrate_mm_s[X_AXIS], 0, default_max_feedrate[X_AXIS] * 2, 1);
            break;

          #if HAS_Y_AXIS
            case SPEED_Y:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedY, GET_TEXT_F(MSG_MAXSPEED_Y));
                Draw_Float(planner.settings.max_feedrate_mm_s[Y_AXIS], row, false, 1);
              }
              else
                Modify_Value(planner.settings.max_feedrate_mm_s[Y_AXIS], 0, default_max_feedrate[Y_AXIS] * 2, 1);
              break;
          #endif

          #if HAS_Z_AXIS
            case SPEED_Z:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedZ, GET_TEXT_F(MSG_MAXSPEED_Z));
                Draw_Float(planner.settings.max_feedrate_mm_s[Z_AXIS], row, false, 1);
              }
              else
                Modify_Value(planner.settings.max_feedrate_mm_s[Z_AXIS], 0, default_max_feedrate[Z_AXIS] * 2, 1);
              break;
          #endif

          #if HAS_HOTEND
            case SPEED_E:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedE, GET_TEXT_F(MSG_MAXSPEED_E));
                Draw_Float(planner.settings.max_feedrate_mm_s[E_AXIS], row, false, 1);
              }
              else
                Modify_Value(planner.settings.max_feedrate_mm_s[E_AXIS], 0, default_max_feedrate[E_AXIS] * 2, 1);
              break;
          #endif
        }
        break;

      case MaxAcceleration:

        #define ACCEL_BACK 0
        #define ACCEL_X (ACCEL_BACK + 1)
        #define ACCEL_Y (ACCEL_X + 1)
        #define ACCEL_Z (ACCEL_Y + 1)
        #define ACCEL_E (ACCEL_Z + ENABLED(HAS_HOTEND))
        #define ACCEL_TOTAL ACCEL_E

        switch (item) {
          case ACCEL_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Motion, MOTION_ACCEL);
            break;
          case ACCEL_X:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxAccX, GET_TEXT_F(MSG_AMAX_A));
              Draw_Float(planner.settings.max_acceleration_mm_per_s2[X_AXIS], row, false, 1);
            }
            else
              Modify_Value(planner.settings.max_acceleration_mm_per_s2[X_AXIS], 0, default_max_acceleration[X_AXIS] * 2, 1);
            break;
          case ACCEL_Y:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxAccY, GET_TEXT_F(MSG_AMAX_B));
              Draw_Float(planner.settings.max_acceleration_mm_per_s2[Y_AXIS], row, false, 1);
            }
            else
              Modify_Value(planner.settings.max_acceleration_mm_per_s2[Y_AXIS], 0, default_max_acceleration[Y_AXIS] * 2, 1);
            break;
          case ACCEL_Z:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxAccZ, GET_TEXT_F(MSG_AMAX_C));
              Draw_Float(planner.settings.max_acceleration_mm_per_s2[Z_AXIS], row, false, 1);
            }
            else
              Modify_Value(planner.settings.max_acceleration_mm_per_s2[Z_AXIS], 0, default_max_acceleration[Z_AXIS] * 2, 1);
            break;
          #if HAS_HOTEND
            case ACCEL_E:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxAccE, GET_TEXT_F(MSG_AMAX_E));
                Draw_Float(planner.settings.max_acceleration_mm_per_s2[E_AXIS], row, false, 1);
              }
              else
                Modify_Value(planner.settings.max_acceleration_mm_per_s2[E_AXIS], 0, default_max_acceleration[E_AXIS] * 2, 1);
              break;
          #endif
        }
        break;
      #if HAS_CLASSIC_JERK
        case MaxJerk:

          #define JERK_BACK 0
          #define JERK_X (JERK_BACK + 1)
          #define JERK_Y (JERK_X + 1)
          #define JERK_Z (JERK_Y + 1)
          #define JERK_E (JERK_Z + ENABLED(HAS_HOTEND))
          #define JERK_TOTAL JERK_E

          switch (item) {
            case JERK_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Motion, MOTION_JERK);
              break;
            case JERK_X:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedJerkX, GET_TEXT_F(MSG_VA_JERK));
                Draw_Float(planner.max_jerk[X_AXIS], row, false, 10);
              }
              else
                Modify_Value(planner.max_jerk[X_AXIS], 0, default_max_jerk[X_AXIS] * 2, 10);
              break;
            case JERK_Y:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedJerkY, GET_TEXT_F(MSG_VB_JERK));
                Draw_Float(planner.max_jerk[Y_AXIS], row, false, 10);
              }
              else
                Modify_Value(planner.max_jerk[Y_AXIS], 0, default_max_jerk[Y_AXIS] * 2, 10);
              break;
            case JERK_Z:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeedJerkZ, GET_TEXT_F(MSG_VC_JERK));
                Draw_Float(planner.max_jerk[Z_AXIS], row, false, 10);
              }
              else
                Modify_Value(planner.max_jerk[Z_AXIS], 0, default_max_jerk[Z_AXIS] * 2, 10);
              break;
            #if HAS_HOTEND
              case JERK_E:
                if (draw) {
                  Draw_Menu_Item(row, ICON_MaxSpeedJerkE, GET_TEXT_F(MSG_VE_JERK));
                  Draw_Float(planner.max_jerk[E_AXIS], row, false, 10);
                }
                else
                  Modify_Value(planner.max_jerk[E_AXIS], 0, default_max_jerk[E_AXIS] * 2, 10);
                break;
            #endif
          }
          break;
      #endif
      #if HAS_JUNCTION_DEVIATION
        case JDmenu:

          #define JD_BACK 0
          #define JD_SETTING_JD_MM (JD_BACK + ENABLED(HAS_HOTEND))
          #define JD_TOTAL JD_SETTING_JD_MM

          switch (item) {
            case JD_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Motion, MOTION_JD);
              break;
            #if HAS_HOTEND
              case JD_SETTING_JD_MM:
                if (draw) {
                  Draw_Menu_Item(row, ICON_MaxJerk, GET_TEXT_F(MSG_JUNCTION_DEVIATION));
                  Draw_Float(planner.junction_deviation_mm, row, false, 100);
                }
                else
                  Modify_Value(planner.junction_deviation_mm, MIN_JD_MM, MAX_JD_MM, 100);
                break;
              #endif
          }
          break;
      #endif
      case Steps:

        #define STEPS_BACK 0
        #define STEPS_X (STEPS_BACK + 1)
        #define STEPS_Y (STEPS_X + 1)
        #define STEPS_Z (STEPS_Y + 1)
        #define STEPS_E (STEPS_Z + ENABLED(HAS_HOTEND))
        #define STEPS_TOTAL STEPS_E

        switch (item) {
          case STEPS_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Motion, MOTION_STEPS);
            break;
          case STEPS_X:
            if (draw) {
              Draw_Menu_Item(row, ICON_StepX, GET_TEXT_F(MSG_A_STEPS));
              Draw_Float(planner.settings.axis_steps_per_mm[X_AXIS], row, false, 100);
            }
            else
              Modify_Value(planner.settings.axis_steps_per_mm[X_AXIS], 0, default_steps[X_AXIS] * 2, 100);
            break;
          case STEPS_Y:
            if (draw) {
              Draw_Menu_Item(row, ICON_StepY, GET_TEXT_F(MSG_B_STEPS));
              Draw_Float(planner.settings.axis_steps_per_mm[Y_AXIS], row, false, 100);
            }
            else
              Modify_Value(planner.settings.axis_steps_per_mm[Y_AXIS], 0, default_steps[Y_AXIS] * 2, 100);
            break;
          case STEPS_Z:
            if (draw) {
              Draw_Menu_Item(row, ICON_StepZ, GET_TEXT_F(MSG_C_STEPS));
              Draw_Float(planner.settings.axis_steps_per_mm[Z_AXIS], row, false, 100);
            }
            else
              Modify_Value(planner.settings.axis_steps_per_mm[Z_AXIS], 0, default_steps[Z_AXIS] * 2, 100);
            break;
          #if HAS_HOTEND
            case STEPS_E:
              if (draw) {
                Draw_Menu_Item(row, ICON_StepE, GET_TEXT_F(MSG_E_STEPS));
                Draw_Float(planner.settings.axis_steps_per_mm[E_AXIS], row, false, 100);
              }
              else
                Modify_Value(planner.settings.axis_steps_per_mm[E_AXIS], 0, 2000, 100);
              break;
          #endif
        }
        break;

      case Visual:

        #define VISUAL_BACK 0
        #define VISUAL_BACKLIGHT (VISUAL_BACK + 1)
        #define VISUAL_BRIGHTNESS (VISUAL_BACKLIGHT + 1)
        #define VISUAL_FAN_PERCENT (VISUAL_BRIGHTNESS + HAS_FAN)
        #define VISUAL_TIME_FORMAT (VISUAL_FAN_PERCENT + 1)
        #define VISUAL_ICON_SET (VISUAL_TIME_FORMAT + ENABLED(DWIN_ICON_SET))
        #define VISUAL_COLOR_THEMES (VISUAL_ICON_SET + 1)
        #define VISUAL_SHORTCUT0 (VISUAL_COLOR_THEMES + 1)
        #define VISUAL_SHORTCUT1 (VISUAL_SHORTCUT0 + 1)
        #define VISUAL_FILE_TUMBNAILS (VISUAL_SHORTCUT1 + (ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)))
        #define VISUAL_TOTAL VISUAL_FILE_TUMBNAILS

        switch (item) {
          case VISUAL_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Control, CONTROL_VISUAL);
            break;
          case VISUAL_BACKLIGHT:
            if (draw)
              Draw_Menu_Item(row, ICON_Brightness, GET_TEXT_F(MSG_BRIGHTNESS_OFF));
            else
              ui.set_brightness(0);
            break;
          case VISUAL_BRIGHTNESS:
            if (draw) {
              Draw_Menu_Item(row, ICON_Brightness, GET_TEXT_F(MSG_BRIGHTNESS));
              Draw_Float(ui.brightness, row, false, 1);
            }
            else
              Modify_Value(ui.brightness, LCD_BRIGHTNESS_MIN, LCD_BRIGHTNESS_MAX, 1, ui.refresh_brightness);
            break;
          #if HAS_FAN
            case VISUAL_FAN_PERCENT:
              if (draw) {
                Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED_PERCENT));
                Draw_Checkbox(row, HMI_datas.fan_percent);
              }
              else {
                HMI_datas.fan_percent = !HMI_datas.fan_percent;
                Draw_Checkbox(row, HMI_datas.fan_percent);
                Redraw_Screen();
              }
              break;
          #endif
          case VISUAL_TIME_FORMAT:
            if (draw) {
              Draw_Menu_Item(row, ICON_PrintTime, GET_TEXT_F(MSG_PROGRESS_IN_HHMM));
              Draw_Checkbox(row, HMI_datas.time_format_textual);
            }
            else {
              HMI_datas.time_format_textual = !HMI_datas.time_format_textual;
              Draw_Checkbox(row, HMI_datas.time_format_textual);
            }
            break;
            #if ENABLED(DWIN_ICON_SET) // mmm - ICON setting
              case VISUAL_ICON_SET:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Binary, GET_TEXT_F(MSG_ICON_SET));
                  // Draw_Option(HMI_datas.iconset_index, icon_set, row);
                  Draw_Float(iconset_current, row, false, 1);

                }
                else {
                  // Modify_Option(HMI_datas.iconset_index, icon_set, 2);
                  Modify_Value(iconset_current, 0, 9, 1, MarlinUI::init_lcd);

                }
                break;
            #endif // mmm end ICON setting
          case VISUAL_COLOR_THEMES:
            if (draw)
              Draw_Menu_Item(row, ICON_MaxSpeed, GET_TEXT_F(MSG_COLORS_SELECT), nullptr, true);
            else
              Draw_Menu(ColorSettings);
          break;
          case VISUAL_SHORTCUT0:
            if (draw) {
              sprintf_P(cmd, PSTR("%s #1"), GET_TEXT(MSG_MAIN_SHORTCUT));
              Draw_Menu_Item(row, ICON_Shortcut, F(cmd));
              Draw_Option(shortcut0, shortcut_list, row);
            }
            else {
              flag_shortcut = false;
              Modify_Option(shortcut0, shortcut_list, NB_Shortcuts);
            }
            break;
          case VISUAL_SHORTCUT1:
            if (draw) {
              sprintf_P(cmd, PSTR("%s #2"), GET_TEXT(MSG_MAIN_SHORTCUT));
              Draw_Menu_Item(row, ICON_Shortcut, F(cmd));
              Draw_Option(shortcut1, shortcut_list, row);
            }
            else {
              flag_shortcut = true;
              Modify_Option(shortcut1, shortcut_list, NB_Shortcuts);
            }
            break;
          #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
            case VISUAL_FILE_TUMBNAILS:
              if (draw) {
                sd_item_flag = false;
                Draw_Menu_Item(row, ICON_File, GET_TEXT_F(MSG_GCODE_THUMBNAILS));
                Draw_Checkbox(row, HMI_datas.show_gcode_thumbnails);
              }
              else {
                HMI_datas.show_gcode_thumbnails = !HMI_datas.show_gcode_thumbnails;
                Draw_Checkbox(row, HMI_datas.show_gcode_thumbnails);
              }
              break;
          #endif
        }
        break;

      case ColorSettings:

        #define COLORSETTINGS_BACK 0
        #define COLORSETTINGS_CURSOR (COLORSETTINGS_BACK + 1)
        #define COLORSETTINGS_SPLIT_LINE (COLORSETTINGS_CURSOR + 1)
        #define COLORSETTINGS_ITEMS_MENU_TEXT (COLORSETTINGS_SPLIT_LINE + 1)
        #define COLORSETTINGS_ICONS_MENU_TEXT (COLORSETTINGS_ITEMS_MENU_TEXT + 1)
        #define COLORSETTINGS_BACKGROUND (COLORSETTINGS_ICONS_MENU_TEXT + 1)
        #define COLORSETTINGS_MENU_TOP_TXT (COLORSETTINGS_BACKGROUND + 1)
        #define COLORSETTINGS_MENU_TOP_BG (COLORSETTINGS_MENU_TOP_TXT + 1)
        #define COLORSETTINGS_SELECT_TXT (COLORSETTINGS_MENU_TOP_BG + 1)
        #define COLORSETTINGS_SELECT_BG (COLORSETTINGS_SELECT_TXT + 1)
        #define COLORSETTINGS_HIGHLIGHT_BORDER (COLORSETTINGS_SELECT_BG + 1)
        #define COLORSETTINGS_POPUP_HIGHLIGHT (COLORSETTINGS_HIGHLIGHT_BORDER + 1)
        #define COLORSETTINGS_POPUP_TXT (COLORSETTINGS_POPUP_HIGHLIGHT + 1)
        #define COLORSETTINGS_POPUP_BG (COLORSETTINGS_POPUP_TXT + 1)
        #define COLORSETTINGS_ICON_CONFIRM_TXT (COLORSETTINGS_POPUP_BG + 1)
        #define COLORSETTINGS_ICON_CONFIRM_BG (COLORSETTINGS_ICON_CONFIRM_TXT + 1)
        #define COLORSETTINGS_ICON_CANCEL_TXT (COLORSETTINGS_ICON_CONFIRM_BG + 1)
        #define COLORSETTINGS_ICON_CANCEL_BG (COLORSETTINGS_ICON_CANCEL_TXT + 1)
        #define COLORSETTINGS_ICON_CONTINUE_TXT (COLORSETTINGS_ICON_CANCEL_BG + 1)
        #define COLORSETTINGS_ICON_CONTINUE_BG (COLORSETTINGS_ICON_CONTINUE_TXT + 1)
        #define COLORSETTINGS_PRINT_SCREEN_TXT (COLORSETTINGS_ICON_CONTINUE_BG + 1)
        #define COLORSETTINGS_PRINT_FILENAME (COLORSETTINGS_PRINT_SCREEN_TXT + 1)
        #define COLORSETTINGS_PROGRESS_BAR (COLORSETTINGS_PRINT_FILENAME + 1)
        #define COLORSETTINGS_PROGRESS_PERCENT (COLORSETTINGS_PROGRESS_BAR + 1)
        #define COLORSETTINGS_REMAIN_TIME (COLORSETTINGS_PROGRESS_PERCENT + 1)
        #define COLORSETTINGS_ELAPSED_TIME (COLORSETTINGS_REMAIN_TIME + 1)
        #define COLORSETTINGS_PROGRESS_STATUS_BAR (COLORSETTINGS_ELAPSED_TIME + 1)
        #define COLORSETTINGS_PROGRESS_STATUS_AREA (COLORSETTINGS_PROGRESS_STATUS_BAR + 1)
        #define COLORSETTINGS_PROGRESS_STATUS_PERCENT (COLORSETTINGS_PROGRESS_STATUS_AREA + 1)
        #define COLORSETTINGS_PROGRESS_COORDINATES (COLORSETTINGS_PROGRESS_STATUS_PERCENT + 1)
        #define COLORSETTINGS_PROGRESS_COORDINATES_LINE (COLORSETTINGS_PROGRESS_COORDINATES + 1)
        #define COLORSETTINGS_TOTAL COLORSETTINGS_PROGRESS_COORDINATES_LINE

        switch (item) {
          case COLORSETTINGS_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Visual, VISUAL_COLOR_THEMES);
            break;
          case COLORSETTINGS_CURSOR:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxSpeed, F("Cursor"));
              Draw_Option(HMI_datas.cursor_color, color_names, row, false, true);
            }
            else
              Modify_Option(HMI_datas.cursor_color, color_names, Custom_Colors);
            break;
          case COLORSETTINGS_SPLIT_LINE:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxSpeed, F("Menu Split Line"));
              Draw_Option(HMI_datas.menu_split_line, color_names, row, false, true);
            }
            else
              Modify_Option(HMI_datas.menu_split_line, color_names, Custom_Colors);
            break;
          case COLORSETTINGS_ITEMS_MENU_TEXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Item Menu Text"));
                Draw_Option(HMI_datas.items_menu_text, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.items_menu_text, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICONS_MENU_TEXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Icon Menu Text"));
                Draw_Option(HMI_datas.icons_menu_text, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.icons_menu_text, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_BACKGROUND:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Background"));
                Draw_Option(HMI_datas.background, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.background, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_MENU_TOP_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Menu Header Text"));
                Draw_Option(HMI_datas.menu_top_txt, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.menu_top_txt, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_MENU_TOP_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Menu Header Bg"));
                Draw_Option(HMI_datas.menu_top_bg, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.menu_top_bg, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_SELECT_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Select Text"));
                Draw_Option(HMI_datas.select_txt, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.select_txt, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_SELECT_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Select Bg"));
                Draw_Option(HMI_datas.select_bg, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.select_bg, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_HIGHLIGHT_BORDER:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Highlight Box"));
                Draw_Option(HMI_datas.highlight_box, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.highlight_box, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_POPUP_HIGHLIGHT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Popup Highlight"));
                Draw_Option(HMI_datas.popup_highlight, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.popup_highlight, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_POPUP_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Popup Text"));
                Draw_Option(HMI_datas.popup_txt, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.popup_txt, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_POPUP_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Popup Bg"));
                Draw_Option(HMI_datas.popup_bg, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.popup_bg, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CONFIRM_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Confirm Icon Txt"));
                Draw_Option(HMI_datas.ico_confirm_txt, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_confirm_txt, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CONFIRM_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Confirm Icon Bg"));
                Draw_Option(HMI_datas.ico_confirm_bg, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_confirm_bg, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CANCEL_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Cancel Icon Text"));
                Draw_Option(HMI_datas.ico_cancel_txt, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_cancel_txt, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CANCEL_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Cancel Icon Bg"));
                Draw_Option(HMI_datas.ico_cancel_bg, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_cancel_bg, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CONTINUE_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Continue Ico Txt"));
                Draw_Option(HMI_datas.ico_continue_txt, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_continue_txt, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_ICON_CONTINUE_BG:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Continue Ico Bg"));
                Draw_Option(HMI_datas.ico_continue_bg, color_names, row, false, true);
              }
              else {
              Modify_Option(HMI_datas.ico_continue_bg, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_PRINT_SCREEN_TXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Print Screen Txt"));
                Draw_Option(HMI_datas.print_screen_txt, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.print_screen_txt, color_names, Custom_Colors_no_Black);
              }
              break;
            case COLORSETTINGS_PRINT_FILENAME:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Print Filename"));
                Draw_Option(HMI_datas.print_filename, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.print_filename, color_names, Custom_Colors_no_Black);
              }
              break;
              case COLORSETTINGS_PROGRESS_BAR:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Progress Bar"));
                Draw_Option(HMI_datas.progress_bar, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.progress_bar, color_names, Custom_Colors);
              }
              break;
            case COLORSETTINGS_PROGRESS_PERCENT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Progress %"));
                Draw_Option(HMI_datas.progress_percent, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.progress_percent, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_REMAIN_TIME:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Remaining Time"));
                Draw_Option(HMI_datas.remain_time, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.remain_time, color_names, Custom_Colors);
              break;
              case COLORSETTINGS_ELAPSED_TIME:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Elapsed Time"));
                Draw_Option(HMI_datas.elapsed_time, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.elapsed_time, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_PROGRESS_STATUS_BAR:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Status Bar Text"));
                Draw_Option(HMI_datas.status_bar_text, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.status_bar_text, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_PROGRESS_STATUS_AREA:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Status Area Text"));
                Draw_Option(HMI_datas.status_area_text, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.status_area_text, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_PROGRESS_STATUS_PERCENT:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Status Area %"));
                Draw_Option(HMI_datas.status_area_percent, color_names, row, false, true);
              }
              else {
                Modify_Option(HMI_datas.status_area_percent, color_names, Custom_Colors_no_Black);
              }
              break;  
            case COLORSETTINGS_PROGRESS_COORDINATES:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Coordinates Text"));
                Draw_Option(HMI_datas.coordinates_text, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.coordinates_text, color_names, Custom_Colors);
              break;
            case COLORSETTINGS_PROGRESS_COORDINATES_LINE:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxSpeed, F("Coordinates Line"));
                Draw_Option(HMI_datas.coordinates_split_line, color_names, row, false, true);
              }
              else
                Modify_Option(HMI_datas.coordinates_split_line, color_names, Custom_Colors);
              break;
          } // switch (item)
        break;

      case HostSettings:

        #define HOSTSETTINGS_BACK 0
        #define HOSTSETTINGS_ACTIONCOMMANDS (HOSTSETTINGS_BACK + ENABLED(HOST_ACTION_COMMANDS))
        #define HOSTSETTINGS_TOTAL HOSTSETTINGS_ACTIONCOMMANDS

        switch (item) {
          case HOSTSETTINGS_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Control, CONTROL_HOSTSETTINGS);
            break;
          #if ENABLED(HOST_ACTION_COMMANDS)
            case HOSTSETTINGS_ACTIONCOMMANDS:
              if (draw) {
                sd_item_flag = false;
                Draw_Menu_Item(row, ICON_File, GET_TEXT_F(MSG_HOST_ACTIONS));
              }
              else {
                Draw_Menu(ActionCommands);
              }
              break;
          #endif
        }
        break;

      #if ENABLED(HOST_ACTION_COMMANDS)
      case ActionCommands:

        #define ACTIONCOMMANDS_BACK 0
        #define ACTIONCOMMANDS_1 (ACTIONCOMMANDS_BACK + 1)
        #define ACTIONCOMMANDS_2 (ACTIONCOMMANDS_1 + 1)
        #define ACTIONCOMMANDS_3 (ACTIONCOMMANDS_2 + 1)
        #define ACTIONCOMMANDS_TOTAL ACTIONCOMMANDS_3

        switch (item) {
          case ACTIONCOMMANDS_BACK:
            if (draw) {
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            }
            else {
              Draw_Menu(HostSettings, HOSTSETTINGS_ACTIONCOMMANDS);
            }
            break;
          case ACTIONCOMMANDS_1:
            if (draw) {
              sd_item_flag = false;
              Draw_Menu_Item(row, ICON_File, F("Action #1"));
              Draw_String(action1, row);
            }
            else {
              Modify_String(action1, 8, true);
            }
            break;
          case ACTIONCOMMANDS_2:
            if (draw) {
              sd_item_flag = false;
              Draw_Menu_Item(row, ICON_File, F("Action #2"));
              Draw_String(action2, row);
            }
            else {
              Modify_String(action2, 8, true);
            }
            break;
          case ACTIONCOMMANDS_3:
            if (draw) {
              sd_item_flag = false;
              Draw_Menu_Item(row, ICON_File, F("Action #3"));
              Draw_String(action3, row);
            }
            else {
              Modify_String(action3, 8, true);
            }
            break;
        }
        break;
      #endif

      case Advanced:

        #define ADVANCED_BACK 0
        #define ADVANCED_BEEPER (ADVANCED_BACK + ENABLED(SOUND_MENU_ITEM))
        #define ADVANCED_PROBE (ADVANCED_BEEPER + ENABLED(HAS_BED_PROBE))
        #define ADVANCED_CORNER (ADVANCED_PROBE + 1)
        #define ADVANCED_LA (ADVANCED_CORNER + ENABLED(LIN_ADVANCE))
        #define ADVANCED_FILMENU (ADVANCED_LA + 1)
        #define ADVANCED_POWER_LOSS (ADVANCED_FILMENU + ENABLED(POWER_LOSS_RECOVERY))
        #define ADVANCED_ENDSDIAG (ADVANCED_POWER_LOSS + HAS_ES_DIAG)
        #define ADVANCED_BAUDRATE_MODE (ADVANCED_ENDSDIAG + ENABLED(BAUD_RATE_GCODE))
        #define ADVANCED_SCREENLOCK (ADVANCED_BAUDRATE_MODE + 1)
        #define ADVANCED_TOTAL ADVANCED_SCREENLOCK

        switch (item) {
          case ADVANCED_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Control, CONTROL_ADVANCED);
            break;

          #if ENABLED(SOUND_MENU_ITEM)
            case ADVANCED_BEEPER:
              if (draw) {
                Draw_Menu_Item(row, ICON_Version, GET_TEXT_F(MSG_SOUND_ENABLE));
                Draw_Checkbox(row, ui.buzzer_enabled);
              }
              else {
                ui.buzzer_enabled = !ui.buzzer_enabled;
                Draw_Checkbox(row, ui.buzzer_enabled);
              }
              break;
          #endif

          #if HAS_BED_PROBE
            case ADVANCED_PROBE:
              if (draw)
                Draw_Menu_Item(row, ICON_StepX, GET_TEXT_F(MSG_ZPROBE_SETTINGS), nullptr, true);
              else
                Draw_Menu(ProbeMenu);
              break;
          #endif

          case ADVANCED_CORNER:
            if (draw) {
              Draw_Menu_Item(row, ICON_MaxAccelerated, GET_TEXT_F(MSG_BED_SCREW_INSET));
              Draw_Float(corner_pos, row, false, 10);
            }
            else
              Modify_Value(corner_pos, 1, 100, 10);
            break;

          #if ENABLED(LIN_ADVANCE)
            case ADVANCED_LA:
              if (draw) {
                Draw_Menu_Item(row, ICON_MaxAccelerated, GET_TEXT_F(MSG_ADVANCE_K_E));
                Draw_Float(planner.extruder_advance_K[0], row, false, 100);
              }
              else
                Modify_Value(planner.extruder_advance_K[0], 0, 10, 100);
              break;
          #endif

          case ADVANCED_FILMENU:
              if (draw)
                Draw_Menu_Item(row, ICON_FilSet, GET_TEXT_F(MSG_FILAMENT_SET), nullptr, true);
              else
                Draw_Menu(Filmenu);
              break;
          #if ENABLED(POWER_LOSS_RECOVERY)
            case ADVANCED_POWER_LOSS:
              if (draw) {
                Draw_Menu_Item(row, ICON_Motion, GET_TEXT_F(MSG_OUTAGE_RECOVERY));
                Draw_Checkbox(row, recovery.enabled);
              }
              else {
                recovery.enable(!recovery.enabled);
                Draw_Checkbox(row, recovery.enabled);
              }
              break;
          #endif
          #if HAS_ES_DIAG
            case ADVANCED_ENDSDIAG:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_LCD_ENDSTOPS), GET_TEXT(MSG_DEBUG_MENU));
                Draw_Menu_Item(row, ICON_ESDiag, F(cmd));
              }
              else 
                EndSDiag.Draw_ends_diag();
              break;
          #endif
          #if ENABLED(BAUD_RATE_GCODE)
            case ADVANCED_BAUDRATE_MODE:
              if (draw) {
                sprintf_P(cmd, PSTR("115K %s"), GET_TEXT(MSG_INFO_BAUDRATE));
                Draw_Menu_Item(row, ICON_Setspeed, F(cmd));
                Draw_Checkbox(row, HMI_datas.baudratemode);
                }
                else {
                  HMI_datas.baudratemode = !HMI_datas.baudratemode;
                  sprintf_P(cmd, PSTR("M575 P%i B%i"), BAUD_PORT, HMI_datas.baudratemode ? 115 : 250);
                  gcode.process_subcommands_now(cmd);
                  Draw_Checkbox(row, HMI_datas.baudratemode);
                  sprintf_P(cmd, GET_TEXT(MSG_INFO_BAUDRATE_CHANGED), HMI_datas.baudratemode ? 115200 : 250000);
                  Update_Status(cmd);
                  }
              break;
          #endif
          case ADVANCED_SCREENLOCK:
            if (draw) 
                Draw_Menu_Item(row, ICON_Lock, GET_TEXT_F(MSG_LOCKSCREEN));
            else 
                DWIN_ScreenLock();
            break;
        }
        break;

      #if HAS_BED_PROBE
        case ProbeMenu:

          #define PROBE_BACK 0
          #define PROBE_XOFFSET (PROBE_BACK + 1)
          #define PROBE_YOFFSET (PROBE_XOFFSET + 1)
          #define PROBE_ZOFFSET (PROBE_YOFFSET + 1)
          #define PROBE_PMARGIN (PROBE_ZOFFSET + EXTJYERSUI)
          #define PROBE_Z_FEEDR_FAST (PROBE_PMARGIN + EXTJYERSUI)
          #define PROBE_Z_FEEDR_SLOW (PROBE_Z_FEEDR_FAST + EXTJYERSUI)
          #define PROBE_HSMODE (PROBE_Z_FEEDR_SLOW + ENABLED(BLTOUCH))
          #define PROBE_ALARMR (PROBE_HSMODE + ENABLED(BLTOUCH))
          #define PROBE_SELFTEST (PROBE_ALARMR + ENABLED(BLTOUCH))
          #define PROBE_MOVEP (PROBE_SELFTEST + ENABLED(BLTOUCH))
          #define PROBE_TEST (PROBE_MOVEP + 1)
          #define PROBE_TEST_COUNT (PROBE_TEST + 1)
          #define PROBE_TOTAL PROBE_TEST_COUNT

          static uint8_t testcount = 4;

          switch (item) {
              case PROBE_BACK:
                if (draw)
                  Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
                else
                  Draw_Menu(Advanced, ADVANCED_PROBE);
                break;
              case PROBE_XOFFSET:
                if (draw) {
                  Draw_Menu_Item(row, ICON_StepX, GET_TEXT_F(MSG_ZPROBE_XOFFSET));
                  Draw_Float(probe.offset.x, row, false, 10);
                }
                else
                  Modify_Value(probe.offset.x, -MAX_XY_OFFSET, MAX_XY_OFFSET, 10);
                break;
              case PROBE_YOFFSET:
                if (draw) {
                  Draw_Menu_Item(row, ICON_StepY, GET_TEXT_F(MSG_ZPROBE_YOFFSET));
                  Draw_Float(probe.offset.y, row, false, 10);
                }
                else
                  Modify_Value(probe.offset.y, -MAX_XY_OFFSET, MAX_XY_OFFSET, 10);
                break;
              case PROBE_ZOFFSET:
                if (draw) {
                  Draw_Menu_Item(row, ICON_StepY, GET_TEXT_F(MSG_ZPROBE_ZOFFSET));
                  Draw_Float(probe.offset.z, row, false, 100);
                }
                else
                  Modify_Value(probe.offset.z, MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
                break;
              #if EXTJYERSUI
                case PROBE_PMARGIN:
                  if (draw) {
                    Draw_Menu_Item(row, ICON_ProbeMargin, GET_TEXT_F(MSG_ZPROBE_MARGIN));
                    Draw_Float(HMI_datas.probing_margin, row, false, 10);
                    }
                  else
                    Modify_Value(HMI_datas.probing_margin, MIN_PROBE_MARGIN, MAX_PROBE_MARGIN, 10);
                  break;
                case PROBE_Z_FEEDR_FAST:
                  if (draw) {
                    Draw_Menu_Item(row, ICON_ProbeZSpeed, GET_TEXT_F(MSG_ZPROBEF_FAST));
                    Draw_Float(HMI_datas.zprobefeedfast, row, false, 1);
                  }
                  else
                    Modify_Value(HMI_datas.zprobefeedfast, MIN_Z_PROBE_FEEDRATE * 2, MAX_Z_PROBE_FEEDRATE, 1);
                  break;
                case PROBE_Z_FEEDR_SLOW:
                  if (draw) {
                    Draw_Menu_Item(row, ICON_ProbeZSpeed, GET_TEXT_F(MSG_ZPROBEF_SLOW));
                    Draw_Float(HMI_datas.zprobefeedslow, row, false, 1);
                  }
                  else
                    Modify_Value(HMI_datas.zprobefeedslow, MIN_Z_PROBE_FEEDRATE, MAX_Z_PROBE_FEEDRATE, 1);
                  break;
              #endif
              #if ENABLED(BLTOUCH)
                case PROBE_HSMODE:
                  if (draw) {
                    sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BLTOUCH), GET_TEXT(MSG_BLTOUCH_SPEED_MODE));
                    Draw_Menu_Item(row, ICON_StockConfiguration, F(cmd));
                    Draw_Checkbox(row, bltouch.high_speed_mode);
                  }
                  else {
                    bltouch.high_speed_mode = !bltouch.high_speed_mode;
                    Draw_Checkbox(row, bltouch.high_speed_mode);
                  }
                  break;
                case PROBE_ALARMR:
                  if (draw) {
                  Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_BLTOUCH_RESET));
                  }
                  else {
                    gcode.process_subcommands_now(F("M280 P0 S160"));
                    AudioFeedback();
                  }
                  break;
                case PROBE_SELFTEST:
                  if (draw) {
                    Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_BLTOUCH_SELFTEST));
                  }
                  else {
                    gcode.process_subcommands_now(F("M280 P0 S120\nG4 P1000\nM280 P0 S160"));
                    planner.synchronize();
                    AudioFeedback();
                  }
                  break;
                case PROBE_MOVEP:
                  if (draw) {
                    Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_BLTOUCH_DEPLOY));
                    Draw_Checkbox(row, probe_deployed);
                  }
                  else {
                    probe_deployed = !probe_deployed;
                    if (probe_deployed == true)  gcode.process_subcommands_now(F("M280 P0 S10"));
                    else  gcode.process_subcommands_now(F("M280 P0 S90"));
                    Draw_Checkbox(row, probe_deployed);
                  }
                  break;
              #endif
              case PROBE_TEST:
                if (draw)
                  Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_M48_TEST));
                else {
                  sprintf_P(cmd, PSTR("G28O\nM48 X%s Y%s P%i"), dtostrf((X_BED_SIZE + X_MIN_POS) / 2.0f, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) / 2.0f, 1, 3, str_2), testcount);
                  gcode.process_subcommands_now(cmd);
                }
                break;
              case PROBE_TEST_COUNT:
                if (draw) {
                  Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_M48_COUNT));
                  Draw_Float(testcount, row, false, 1);
                }
                else
                  Modify_Value(testcount, 4, 50, 1);
                break;
          }
          break;
      #endif

      case Filmenu:

        #define FIL_BACK 0
        #define FIL_SENSORENABLED (FIL_BACK + HAS_FILAMENT_SENSOR)
        #define FIL_RUNOUTACTIVE (FIL_SENSORENABLED + (BOTH(HAS_FILAMENT_SENSOR, EXTJYERSUI)))
        #define FIL_SENSORDISTANCE (FIL_RUNOUTACTIVE + 1)
        #define FIL_LOAD (FIL_SENSORDISTANCE + ENABLED(ADVANCED_PAUSE_FEATURE))
        #define FIL_UNLOAD (FIL_LOAD + ENABLED(ADVANCED_PAUSE_FEATURE))
        #define FIL_UNLOAD_FEEDRATE (FIL_UNLOAD + (BOTH(ADVANCED_PAUSE_FEATURE, EXTJYERSUI)))
        #define FIL_FAST_LOAD_FEEDRATE (FIL_UNLOAD_FEEDRATE + (BOTH(ADVANCED_PAUSE_FEATURE, EXTJYERSUI)))
        #define FIL_COLD_EXTRUDE  (FIL_FAST_LOAD_FEEDRATE + ENABLED(PREVENT_COLD_EXTRUSION))
        #define FIL_TOTAL FIL_COLD_EXTRUDE

        switch (item) {
          case FIL_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu(Advanced, ADVANCED_FILMENU);
            break;

          #if HAS_FILAMENT_SENSOR
            case FIL_SENSORENABLED:
              if (draw) {
                Draw_Menu_Item(row, ICON_Extruder, GET_TEXT_F(MSG_RUNOUT_SENSOR));
                if (runout.mode[0] == RM_NONE) runout.enabled[0] = false;
                Draw_Checkbox(row, runout.enabled[0]);
              }
              else {
                if (runout.mode[0] == RM_NONE) runout.enabled[0] = false;
                else runout.enabled[0] = !runout.enabled[0];
                Draw_Checkbox(row, runout.enabled[0]);
              }
              break;
            #if EXTJYERSUI 
              case FIL_RUNOUTACTIVE:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FilSet, GET_TEXT_F(MSG_RUNOUT_ENABLE));
                  Draw_Option(rsensormode, runoutsensor_modes, row);
                }
                else {
                  runout.reset();
                  State_runoutenable = runout.enabled[0];
                  runout.enabled[0] = false;
                  Modify_Option(rsensormode, runoutsensor_modes, 3);
                }
                break;
            #endif
            //#if ENABLED(HAS_FILAMENT_RUNOUT_DISTANCE)
              case FIL_SENSORDISTANCE:
                if (draw) {
                  editable_distance = runout.runout_distance();
                  Draw_Menu_Item(row, ICON_MaxAccE, GET_TEXT_F(MSG_RUNOUT_DISTANCE_MM));
                  Draw_Float(editable_distance, row, false, 10);
                }
                else
                  Modify_Value(editable_distance, 0, 999, 10);
                break;
            //#endif
          #endif

          #if ENABLED(ADVANCED_PAUSE_FEATURE)
            case FIL_LOAD:
              if (draw) {
                Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_FILAMENT_LOAD));
                Draw_Float(fc_settings[0].load_length, row, false, 1);
              }
              else
                Modify_Value(fc_settings[0].load_length, 0, EXTRUDE_MAXLENGTH, 1);
              break;
            case FIL_UNLOAD:
              if (draw) {
                Draw_Menu_Item(row, ICON_ReadEEPROM, GET_TEXT_F(MSG_FILAMENT_UNLOAD));
                Draw_Float(fc_settings[0].unload_length, row, false, 1);
              }
              else
                Modify_Value(fc_settings[0].unload_length, 0, EXTRUDE_MAXLENGTH, 1);
              break;
            #if EXTJYERSUI
              case FIL_UNLOAD_FEEDRATE:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FilUnload, GET_TEXT_F(MSG_FILAMENT_UNLOAD_RATE));
                  Draw_Float(HMI_datas.fil_unload_feedrate, row, false, 1);
                }
                else
                  Modify_Value(HMI_datas.fil_unload_feedrate, MIN_FIL_CHANGE_FEEDRATE, MAX_FIL_CHANGE_FEEDRATE, 1);
                break;
              case FIL_FAST_LOAD_FEEDRATE:
                if (draw) {
                  Draw_Menu_Item(row, ICON_FilLoad, GET_TEXT_F(MSG_FILAMENT_LOAD_RATE));
                  Draw_Float(HMI_datas.fil_fast_load_feedrate, row, false, 1);
                }
                else
                  Modify_Value(HMI_datas.fil_fast_load_feedrate, MIN_FIL_CHANGE_FEEDRATE, MAX_FIL_CHANGE_FEEDRATE, 1);
                break;
            #endif
          #endif // ADVANCED_PAUSE_FEATURE

          #if ENABLED(PREVENT_COLD_EXTRUSION)
            case FIL_COLD_EXTRUDE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Cool, GET_TEXT_F(MSG_INFO_MIN_TEMP));
                Draw_Float(thermalManager.extrude_min_temp, row, false, 1);
              }
              else {
                Modify_Value(thermalManager.extrude_min_temp, 0, MAX_E_TEMP, 1);
                thermalManager.allow_cold_extrude = (thermalManager.extrude_min_temp == 0);
              }
              break;
          #endif
        }
      break;

      case InfoMain:
      case Info:

        #define INFO_BACK 0
        #define INFO_PRINTCOUNT (INFO_BACK + ENABLED(PRINTCOUNTER))
        #define INFO_PRINTTIME (INFO_PRINTCOUNT + ENABLED(PRINTCOUNTER))
        #define INFO_SIZE (INFO_PRINTTIME + 1)
        #define INFO_VERSION (INFO_SIZE + 1)
        #define INFO_CONTACT (INFO_VERSION + 1)

        #if ENABLED(PRINTCOUNTER)
          #define INFO_RESET_PRINTCOUNTER (INFO_CONTACT + 1)
          #define INFO_TOTAL INFO_RESET_PRINTCOUNTER
        #else
          #define INFO_TOTAL INFO_BACK
        #endif

        switch (item) {
          case INFO_BACK:
            if (draw) {
              Update_Status(CUSTOM_MACHINE_NAME);
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));

              #if ENABLED(PRINTCOUNTER)
                char row1[50], row2[50], buf[32];
                printStatistics ps = print_job_timer.getStats();

                sprintf_P(row1, PSTR("%s: %i, %i %s"), GET_TEXT(MSG_INFO_PRINT_COUNT), ps.totalPrints, ps.finishedPrints, GET_TEXT(MSG_INFO_COMPLETED_PRINTS));
                sprintf_P(row2, PSTR("%sm %s"), dtostrf(ps.filamentUsed / 1000, 1, 2, str_1), GET_TEXT(MSG_INFO_PRINT_FILAMENT));
                Draw_Menu_Item(INFO_PRINTCOUNT, ICON_HotendTemp, row1, row2, false, true);

                duration_t(print_job_timer.getStats().printTime).toString(buf);
                sprintf_P(row1, PSTR("%s: %s"), GET_TEXT(MSG_INFO_PRINT_TIME), buf);
                duration_t(print_job_timer.getStats().longestPrint).toString(buf);
                sprintf_P(row2, PSTR("%s: %s"), GET_TEXT(MSG_INFO_PRINT_LONGEST), buf);
                Draw_Menu_Item(INFO_PRINTTIME, ICON_PrintTime, row1, row2, false, true);
              #endif
              sprintf_P(cmd, PSTR("%s (%s)"), BUILD_NUMBER, STM_cpu);
              Draw_Menu_Item(INFO_SIZE, ICON_PrintSize, F(MACHINE_SIZE), nullptr, false, true);
              Draw_Menu_Item(INFO_VERSION, ICON_Version, F(SHORT_BUILD_VERSION), F(cmd), false, true);
              Draw_Menu_Item(INFO_CONTACT, ICON_Contact, F(CORP_WEBSITE1), F(CORP_WEBSITE2), false, true);
            }
            else {
              Update_Status("");
              if (menu == Info)
                Draw_Menu(Control, CONTROL_INFO);
              else
                Draw_Main_Menu(3);
            }
            break;
          #if ENABLED(PRINTCOUNTER)
            case INFO_RESET_PRINTCOUNTER:
              if (draw) {
                Draw_Menu_Item(row, ICON_HotendTemp, GET_TEXT_F(MSG_INFO_PRINT_COUNT_RESET));
              }
              else {
                print_job_timer.initStats();
                ui.reset_status();
                Draw_Menu(Info);
                AudioFeedback();
              }
              break;
          #endif
        }
        break;

      #if HAS_MESH
        case Leveling:

          #define LEVELING_BACK 0
          #define LEVELING_ACTIVE (LEVELING_BACK + 1)
          #define LEVELING_GET_TILT (LEVELING_ACTIVE + BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL))
          #define LEVELING_GET_MESH (LEVELING_GET_TILT + 1)
          #define LEVELING_MANUAL (LEVELING_GET_MESH + 1)
          #define LEVELING_VIEW (LEVELING_MANUAL + 1)
          #define LEVELING_SETTINGS (LEVELING_VIEW + 1)
          #define LEVELING_SLOT (LEVELING_SETTINGS + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_LOAD (LEVELING_SLOT + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SAVE (LEVELING_LOAD + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_TOTAL LEVELING_SAVE

          switch (item) {
            case LEVELING_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Main_Menu(3);
              break;
            case LEVELING_ACTIVE:
              if (draw) {
                Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_MESH_LEVELING));
                Draw_Checkbox(row, planner.leveling_active);
              }
              else {
                if (!planner.leveling_active) {
                  set_bed_leveling_enabled(!planner.leveling_active);
                  if (!planner.leveling_active) {
                    Confirm_Handler(LevelError);
                    break;
                  }
                }
                else  set_bed_leveling_enabled(!planner.leveling_active);
                  
                Draw_Checkbox(row, planner.leveling_active);
                HMI_datas.leveling_active = planner.leveling_active;
              }
              break;
            #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
              case LEVELING_GET_TILT:
                if (draw)
                  Draw_Menu_Item(row, ICON_Tilt, GET_TEXT_F(MSG_UBL_AUTOTILT));
                else {
                  if (ubl.storage_slot < 0) {
                    Popup_Handler(MeshSlot);
                    break;
                  }
                  #if EITHER(PREHEAT_BEFORE_LEVELING, PREHEAT_BEFORE_LEVELING_PROBE_MANUALLY)
                    HeatBeforeLeveling();
                  #endif
                  Update_Status("");
                  Popup_Handler(Home);
                  gcode.home_all_axes(true);
                  Popup_Handler(Level);
                  if (mesh_conf.tilt_grid > 1) {
                    sprintf_P(cmd, PSTR("G29 J%i"), mesh_conf.tilt_grid);
                    gcode.process_subcommands_now(cmd);
                  }
                  else
                    gcode.process_subcommands_now(F("G29 J"));
                  planner.synchronize();
                  Redraw_Menu();
                }
                break;
            #endif
            case LEVELING_GET_MESH:
              if (draw)
                Draw_Menu_Item(row, ICON_Mesh, GET_TEXT_F(MSG_UBL_BUILD_MESH_MENU));
              else {
                #if ENABLED(AUTO_BED_LEVELING_UBL)
                    if (ubl.storage_slot <0) {
                      Popup_Handler(MeshSlot, true);
                      break;
                    }
                #endif             
                #if EITHER(PREHEAT_BEFORE_LEVELING, PREHEAT_BEFORE_LEVELING_PROBE_MANUALLY)
                  HeatBeforeLeveling();
                #endif
                Update_Status("");
                Popup_Handler(Home);
                gcode.home_all_axes(true);
                #if ENABLED(AUTO_BED_LEVELING_UBL) 
                  #if HAS_BED_PROBE
                    TERN_(EXTJYERSUI, HMI_flags.cancel_ubl = 0);
                    //Popup_Handler(Level);
                    TERN(EXTJYERSUI, Confirm_Handler(Level2), Popup_Handler(Level));
                    gcode.process_subcommands_now(F("G29 P1"));
                    gcode.process_subcommands_now(F("G29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nG29 P3\nM420 S1"));
                    planner.synchronize();
                    Update_Status(GET_TEXT(MSG_MESH_DONE));
                    //Popup_Handler(SaveLevel);
                    #if EXTJYERSUI
                      if (!HMI_flags.cancel_ubl) Viewmesh();
                    #else
                      Viewmesh();
                    #endif
                  #else
                    level_state = planner.leveling_active;
                    set_bed_leveling_enabled(false);
                    mesh_conf.goto_mesh_value = true;
                    mesh_conf.mesh_x = mesh_conf.mesh_y = 0;
                    Popup_Handler(MoveWait);
                    mesh_conf.manual_move();
                    gcode.process_subcommands_now(F("M211 S0"));
                    Draw_Menu(UBLMesh);
                  #endif
                #elif HAS_BED_PROBE
                  TERN_(EXTJYERSUI,HMI_flags.cancel_abl = 0);
                  //Popup_Handler(Level);
                  TERN(EXTJYERSUI, Confirm_Handler(Level2), Popup_Handler(Level));
                  gcode.process_subcommands_now(F("G29"));
                  planner.synchronize();
                  //Popup_Handler(SaveLevel);
                  #if EXTJYERSUI
                   if (!HMI_flags.cancel_abl) Viewmesh();
                  #else
                   Viewmesh();
                  #endif
                #else
                  level_state = planner.leveling_active;
                  set_bed_leveling_enabled(false);
                  gridpoint = 1;
                  Popup_Handler(MoveWait);
                  gcode.process_subcommands_now(F("M211 S0\nG29"));
                  planner.synchronize();
                  Draw_Menu(ManualMesh);
                #endif
              }
              break;
            case LEVELING_MANUAL:
              if (draw)
                Draw_Menu_Item(row, ICON_Mesh, GET_TEXT_F(MSG_UBL_MESH_EDIT), nullptr, true);
              else {
                #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
                  if (!leveling_is_valid()) {
                    Confirm_Handler(InvalidMesh);
                    break;
                  }
                #endif
                #if ENABLED(AUTO_BED_LEVELING_UBL)
                  if (ubl.storage_slot < 0) {
                    Popup_Handler(MeshSlot);
                    break;
                  }
                #endif
                #if EITHER(PREHEAT_BEFORE_LEVELING, PREHEAT_BEFORE_LEVELING_PROBE_MANUALLY)
                  HeatBeforeLeveling();
                #endif
                Update_Status("");
                if (axes_should_home()) {
                  Popup_Handler(Home);
                  gcode.home_all_axes(true);
                }
                level_state = planner.leveling_active;
                set_bed_leveling_enabled(false);
                mesh_conf.goto_mesh_value = false;
                Popup_Handler(MoveWait);
                mesh_conf.manual_move();
                gcode.process_subcommands_now(F("M211 S0"));
                Draw_Menu(LevelManual);
              }
              break;
            case LEVELING_VIEW:
              if (draw)
                Draw_Menu_Item(row, ICON_Mesh, GET_TEXT_F(MSG_MESH_VIEW), nullptr, true);
              else {
                #if ENABLED(AUTO_BED_LEVELING_UBL)
                  if (ubl.storage_slot < 0) {
                    Popup_Handler(MeshSlot);
                    break;
                  }
                #endif
                Draw_Menu(LevelView);
              }
              break;
            case LEVELING_SETTINGS:
              if (draw)
                Draw_Menu_Item(row, ICON_Step, GET_TEXT_F(MSG_ADVANCED_SETTINGS), nullptr, true);
              else
                Draw_Menu(LevelSettings);
              break;
            #if ENABLED(AUTO_BED_LEVELING_UBL)
            case LEVELING_SLOT:
              if (draw) {
                Draw_Menu_Item(row, ICON_PrintSize, GET_TEXT_F(MSG_UBL_STORAGE_SLOT));
                Draw_Float(ubl.storage_slot, row, false, 1);
              }
              else
                Modify_Value(ubl.storage_slot, 0, settings.calc_num_meshes() - 1, 1);
              break;
            case LEVELING_LOAD:
              if (draw)
                Draw_Menu_Item(row, ICON_ReadEEPROM, GET_TEXT_F(MSG_UBL_LOAD_MESH));
              else {
                if (ubl.storage_slot < 0) {
                  Popup_Handler(MeshSlot);
                  break;
                }
                gcode.process_subcommands_now(F("G29 L"));
                planner.synchronize();
                AudioFeedback(true);
              }
              break;
            case LEVELING_SAVE:
              if (draw)
                Draw_Menu_Item(row, ICON_WriteEEPROM, GET_TEXT_F(MSG_UBL_SAVE_MESH));
              else {
                if (ubl.storage_slot < 0) {
                  Popup_Handler(MeshSlot, true);
                  break;
                }
                gcode.process_subcommands_now(F("G29 S"));
                planner.synchronize();
                AudioFeedback(true);
              }
              break;
            #endif
          }
          break;

        case LevelView:

          #define LEVELING_VIEW_BACK 0
          #define LEVELING_VIEW_MESH (LEVELING_VIEW_BACK + 1)
          #define LEVELING_VIEW_TEXT (LEVELING_VIEW_MESH + 1)
          #define LEVELING_VIEW_ASYMMETRIC (LEVELING_VIEW_TEXT + 1)
          #define LEVELING_VIEW_TOTAL LEVELING_VIEW_ASYMMETRIC

          switch (item) {
            case LEVELING_VIEW_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Leveling, LEVELING_VIEW);
              break;
            case LEVELING_VIEW_MESH:
              if (draw)
                Draw_Menu_Item(row, ICON_PrintSize, GET_TEXT_F(MSG_MESH_VIEW), nullptr, true);
              else
                Draw_Menu(MeshViewer);
              break;
            case LEVELING_VIEW_TEXT:
              if (draw) {
                Draw_Menu_Item(row, ICON_Contact, GET_TEXT_F(MSG_MESH_VIEW_TEXT));
                Draw_Checkbox(row, mesh_conf.viewer_print_value);
              }
              else {
                mesh_conf.viewer_print_value = !mesh_conf.viewer_print_value;
                Draw_Checkbox(row, mesh_conf.viewer_print_value);
              }
              break;
            case LEVELING_VIEW_ASYMMETRIC:
              if (draw) {
                Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_MESH_VIEW_ASYMMETRIC));
                Draw_Checkbox(row, mesh_conf.viewer_asymmetric_range);
              }
              else {
                mesh_conf.viewer_asymmetric_range = !mesh_conf.viewer_asymmetric_range;
                Draw_Checkbox(row, mesh_conf.viewer_asymmetric_range);
              }
              break;
          }
          break;

        case LevelSettings:

          #define LEVELING_SETTINGS_BACK 0
          #define LEVELING_SETTINGS_HOTENDTEMP_ENA (LEVELING_SETTINGS_BACK + ENABLED(HAS_LEVELING_HEAT))
          #define LEVELING_SETTINGS_HOTENDTEMP (LEVELING_SETTINGS_HOTENDTEMP_ENA + ENABLED(HAS_LEVELING_HEAT))
          #define LEVELING_SETTINGS_BEDTEMP_ENA (LEVELING_SETTINGS_HOTENDTEMP  + ENABLED(HAS_LEVELING_HEAT))
          #define LEVELING_SETTINGS_BEDTEMP (LEVELING_SETTINGS_BEDTEMP_ENA + ENABLED(HAS_LEVELING_HEAT))
          #define LEVELING_SETTINGS_FADE (LEVELING_SETTINGS_BEDTEMP + 1)
          #define LEVELING_SETTINGS_TILT (LEVELING_SETTINGS_FADE + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SETTINGS_TILT_AFTER_N_PRINTS (LEVELING_SETTINGS_TILT + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SETTINGS_PLANE (LEVELING_SETTINGS_TILT_AFTER_N_PRINTS + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SETTINGS_ZERO (LEVELING_SETTINGS_PLANE + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SETTINGS_UNDEF (LEVELING_SETTINGS_ZERO + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_SETTINGS_TOTAL LEVELING_SETTINGS_UNDEF

          switch (item) {
            case LEVELING_SETTINGS_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else
                Draw_Menu(Leveling, LEVELING_SETTINGS);
              break;
            #if HAS_LEVELING_HEAT
              case LEVELING_SETTINGS_HOTENDTEMP_ENA:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Checkbox(row, HMI_datas.ena_LevelingTemp_hotend);
                }
                else {
                  HMI_datas.ena_LevelingTemp_hotend = !HMI_datas.ena_LevelingTemp_hotend;
                  Draw_Checkbox(row, HMI_datas.ena_LevelingTemp_hotend);
                }
                break;
              case LEVELING_SETTINGS_HOTENDTEMP:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                  Draw_Float(HMI_datas.LevelingTemp_hotend, row, false, 1);
                }
                else
                  Modify_Value(HMI_datas.LevelingTemp_hotend, MIN_E_TEMP, MAX_E_TEMP, 1);
                break;
              case LEVELING_SETTINGS_BEDTEMP_ENA:
                if (draw) {
                  Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                  Draw_Checkbox(row, HMI_datas.ena_LevelingTemp_bed);
                }
                else {
                  HMI_datas.ena_LevelingTemp_bed = !HMI_datas.ena_LevelingTemp_bed;
                  Draw_Checkbox(row, HMI_datas.ena_LevelingTemp_bed);
                }
                break;
              case LEVELING_SETTINGS_BEDTEMP:
                if (draw) {
                Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                Draw_Float(HMI_datas.LevelingTemp_bed, row, false, 1);
              }
              else
                Modify_Value(HMI_datas.LevelingTemp_bed, MIN_BED_TEMP, MAX_BED_TEMP, 1);
              break;
            #endif
            case LEVELING_SETTINGS_FADE:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Fade, GET_TEXT_F(MSG_Z_FADE_HEIGHT));
                  Draw_Float(planner.z_fade_height, row, false, 1);
                }
                else {
                  Modify_Value(planner.z_fade_height, 0, Z_MAX_POS, 1);
                  planner.z_fade_height = -1;
                  set_z_fade_height(planner.z_fade_height);
                }
                break;

            #if ENABLED(AUTO_BED_LEVELING_UBL)
              case LEVELING_SETTINGS_TILT:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Tilt, GET_TEXT_F(MSG_LCD_TILTING_GRID_SIZE));
                  Draw_Float(mesh_conf.tilt_grid, row, false, 1);
                }
                else
                  Modify_Value(mesh_conf.tilt_grid, 1, 8, 1);
                break;
              case LEVELING_SETTINGS_TILT_AFTER_N_PRINTS:
                if (draw) {
                  Draw_Menu_Item(row, ICON_Tilt, GET_TEXT_F(MSG_UBL_AUTOTILT_AFTER_N_PRINTS));
                  Draw_Float(NPrinted, row, false, 1);
                }
                else 
                  Modify_Value(NPrinted, 0, 200, 1);
                break;
              case LEVELING_SETTINGS_PLANE:
                if (draw)
                  Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_MESH_TO_PLANE));
                else {
                  if (mesh_conf.create_plane_from_mesh()) {
                    Confirm_Handler(NocreatePlane);
                  break;
                }
                  gcode.process_subcommands_now(F("M420 S1"));
                  planner.synchronize();
                  AudioFeedback(true);
                }
                break;
              case LEVELING_SETTINGS_ZERO:
                if (draw)
                  Draw_Menu_Item(row, ICON_Mesh, GET_TEXT_F(MSG_MESH_ZERO));
                else
                  ZERO(Z_VALUES_ARR);
                break;
              case LEVELING_SETTINGS_UNDEF:
                if (draw)
                  Draw_Menu_Item(row, ICON_Mesh, GET_TEXT_F(MSG_MESH_CLEAR));
                else
                  ubl.invalidate();
                break;
            #endif // AUTO_BED_LEVELING_UBL
          }
          break;

        case MeshViewer:
          #define MESHVIEW_BACK 0
          #define MESHVIEW_TOTAL MESHVIEW_BACK

          if (item == MESHVIEW_BACK) {
            if (draw) {
              Draw_Menu_Item(0, ICON_Back, GET_TEXT_F(MSG_BACK));
              mesh_conf.Draw_Bed_Mesh();
              mesh_conf.Set_Mesh_Viewer_Status();
            }
            else if (!mesh_conf.drawing_mesh) {           
              Draw_Menu(LevelView, LEVELING_VIEW_MESH);
              Update_Status("");  
            }
          }
          break;

        case LevelManual:

          #define LEVELING_M_BACK 0
          #define LEVELING_M_MODELIVE (LEVELING_M_BACK + 1)
          #define LEVELING_M_X (LEVELING_M_MODELIVE + 1)
          #define LEVELING_M_Y (LEVELING_M_X + 1)
          #define LEVELING_M_NEXT (LEVELING_M_Y + 1)
          #define LEVELING_M_PREV (LEVELING_M_NEXT + 1)
          #define LEVELING_M_OFFSET (LEVELING_M_PREV + 1)
          #define LEVELING_M_UP (LEVELING_M_OFFSET + 1)
          #define LEVELING_M_DOWN (LEVELING_M_UP + 1)
          #define LEVELING_M_GOTO_VALUE (LEVELING_M_DOWN + 1)
          #define LEVELING_M_UNDEF (LEVELING_M_GOTO_VALUE + ENABLED(AUTO_BED_LEVELING_UBL))
          #define LEVELING_M_TOTAL LEVELING_M_UNDEF

          switch (item) {
            case LEVELING_M_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else {
                liveadjust = false;
                flag_leveling_m = true;
                gcode.process_subcommands_now(F("M211 S1"));
                set_bed_leveling_enabled(level_state);
                TERN_(AUTO_BED_LEVELING_BILINEAR, bbl.refresh_bed_level());
                Popup_Handler(SaveLevel, true);
              }
              break;
            case LEVELING_M_MODELIVE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_LIVE_ADJUSTMENT));
                Draw_Checkbox(row, liveadjust);
              }
              else {
                liveadjust = !liveadjust;
                Draw_Checkbox(row, liveadjust);
              }
              break;
            case LEVELING_M_X:
              if (draw) {
                Draw_Menu_Item(row, ICON_MoveX, GET_TEXT_F(MSG_MESH_X));
                Draw_Float(mesh_conf.mesh_x, row, 0, 1);
              }
              else
                Modify_Value(mesh_conf.mesh_x, 0, GRID_MAX_POINTS_X - 1, 1);
              break;
            case LEVELING_M_Y:
              if (draw) {
                Draw_Menu_Item(row, ICON_MoveY, GET_TEXT_F(MSG_MESH_Y));
                Draw_Float(mesh_conf.mesh_y, row, 0, 1);
              }
              else
                Modify_Value(mesh_conf.mesh_y, 0, GRID_MAX_POINTS_Y - 1, 1);
              break;
            case LEVELING_M_NEXT:
              if (draw) {
                if (mesh_conf.mesh_x != (GRID_MAX_POINTS_X-1) || mesh_conf.mesh_y != (GRID_MAX_POINTS_Y-1))
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_LEVEL_BED_NEXT_POINT));
                else
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_UBL_SAVE_MESH));
              }
              else {
                if (mesh_conf.mesh_x != (GRID_MAX_POINTS_X - 1) || mesh_conf.mesh_y != (GRID_MAX_POINTS_Y - 1)) {
                  if ((mesh_conf.mesh_x == (GRID_MAX_POINTS_X - 1) && mesh_conf.mesh_y % 2 == 0) || (mesh_conf.mesh_x == 0 && mesh_conf.mesh_y % 2 == 1))
                    mesh_conf.mesh_y++;
                  else if (mesh_conf.mesh_y % 2 == 0)
                    mesh_conf.mesh_x++;
                  else
                    mesh_conf.mesh_x--;
                  mesh_conf.manual_move();
                }
                else {
                    gcode.process_subcommands_now(F("M211 S1"));
                    #if ENABLED(EEPROM_SETTINGS)
                      #if ENABLED(AUTO_BED_LEVELING_UBL)
                        gcode.process_subcommands_now(F("G29 S"));                 
                        AudioFeedback(true);
                        set_bed_leveling_enabled(level_state);
                      #else
                        set_bed_leveling_enabled(level_state);
                        AudioFeedback(settings.save());
                      #endif
                    #endif
                    liveadjust = false;
                    TERN_(AUTO_BED_LEVELING_BILINEAR, bbl.refresh_bed_level());
                    planner.synchronize();
                    Draw_Menu(Leveling, LEVELING_MANUAL);
                  }
                }
                break;
            case LEVELING_M_PREV:
                if (draw) {
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_LEVEL_BED_PREV_POINT));
                }
                else {
                  if (mesh_conf.mesh_x != 0 || mesh_conf.mesh_y != 0) {
                    if ((mesh_conf.mesh_x == (GRID_MAX_POINTS_X-1) && mesh_conf.mesh_y % 2 == 1) || (mesh_conf.mesh_x == 0 && mesh_conf.mesh_y % 2 == 0)) {
                      mesh_conf.mesh_y--;
                    }
                    else if (mesh_conf.mesh_y % 2 == 0) {
                      mesh_conf.mesh_x--;
                    }
                    else {
                      mesh_conf.mesh_x++;
                    }
                    mesh_conf.manual_move();
                  }
                }
                break;
            case LEVELING_M_OFFSET:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_OFFSET_Z));
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row, false, 100);
              }
              else {
                if (isnan(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y]))
                  Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] = 0;
                Modify_Value(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
              }
              break;
            case LEVELING_M_UP:
              if (draw){
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_UP));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
                }
              else if (Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] < MAX_Z_OFFSET) {
                Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] += 0.01;
                gcode.process_subcommands_now(F("M290 Z0.01"));
                planner.synchronize();
                current_position.z += 0.01f;
                sync_plan_position();
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row - 1, false, 100);
              }
              break;
            case LEVELING_M_DOWN:
              if (draw){
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_DOWN));
                Draw_Menu_Item(row, ICON_AxisD, F(cmd));
                }
              else if (Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] > MIN_Z_OFFSET) {
                Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] -= 0.01;
                gcode.process_subcommands_now(F("M290 Z-0.01"));
                planner.synchronize();
                current_position.z -= 0.01f;
                sync_plan_position();
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row - 2, false, 100);
              }
              break;
            case LEVELING_M_GOTO_VALUE:
              if (draw) {
                Draw_Menu_Item(row, ICON_StockConfiguration, GET_TEXT_F(MSG_MESH_SNAP_Z));
                Draw_Checkbox(row, mesh_conf.goto_mesh_value);
              }
              else {
                mesh_conf.goto_mesh_value = !mesh_conf.goto_mesh_value;
                current_position.z = 0;
                mesh_conf.manual_move(true);
                Draw_Checkbox(row, mesh_conf.goto_mesh_value);
              }
              break;
            #if ENABLED(AUTO_BED_LEVELING_UBL)
            case LEVELING_M_UNDEF:
              if (draw)
                Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_LEVEL_BED_CLEAR_POINT));
              else {
                mesh_conf.manual_value_update(true);
                Redraw_Menu(false);
              }
              break;
            #endif
          }
          break;
      #endif // HAS_MESH

      #if ENABLED(AUTO_BED_LEVELING_UBL) && !HAS_BED_PROBE
        case UBLMesh:

          #define UBL_M_BACK 0
          #define UBL_M_MODELIVE (UBL_M_BACK + 1)
          #define UBL_M_NEXT (UBL_M_MODELIVE + 1)
          #define UBL_M_PREV (UBL_M_NEXT + 1)
          #define UBL_M_OFFSET (UBL_M_PREV + 1)
          #define UBL_M_UP (UBL_M_OFFSET + 1)
          #define UBL_M_DOWN (UBL_M_UP + 1)
          #define UBL_M_TOTAL UBL_M_DOWN

          switch (item) {
            case UBL_M_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
              else {
                liveadjust = false;
                gcode.process_subcommands_now(F("M211 S1"));
                set_bed_leveling_enabled(level_state);
                Draw_Menu(Leveling, LEVELING_GET_MESH);
              }
              break;
            case UBL_M_MODELIVE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_LIVE_ADJUSTMENT));
                Draw_Checkbox(row, liveadjust);
              }
              else {
                liveadjust = !liveadjust;
                Draw_Checkbox(row, liveadjust);
              }
              break;
            case UBL_M_NEXT:
              if (draw) {
                if (mesh_conf.mesh_x != (GRID_MAX_POINTS_X - 1) || mesh_conf.mesh_y != (GRID_MAX_POINTS_Y - 1))
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_LEVEL_BED_NEXT_POINT));
                else
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_UBL_SAVE_MESH));
              }
              else {
                if (mesh_conf.mesh_x != (GRID_MAX_POINTS_X - 1) || mesh_conf.mesh_y != (GRID_MAX_POINTS_Y - 1)) {
                  if ((mesh_conf.mesh_x == (GRID_MAX_POINTS_X - 1) && mesh_conf.mesh_y % 2 == 0) || (mesh_conf.mesh_x == 0 && mesh_conf.mesh_y % 2 == 1))
                    mesh_conf.mesh_y++;
                  else if (mesh_conf.mesh_y % 2 == 0)
                    mesh_conf.mesh_x++;
                  else
                    mesh_conf.mesh_x--;
                  mesh_conf.manual_move();
                }
                else {
                  gcode.process_subcommands_now(F("G29 S"));
                  planner.synchronize();
                  AudioFeedback(true);
                  liveadjust = false;
                  gcode.process_subcommands_now(F("M211 S1"));
                  set_bed_leveling_enabled(level_state);
                  Draw_Menu(Leveling, LEVELING_GET_MESH);
                }
              }
              break;
            case UBL_M_PREV:
              if (draw)
                Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_LEVEL_BED_PREV_POINT));
              else {
                if (mesh_conf.mesh_x != 0 || mesh_conf.mesh_y != 0) {
                  if ((mesh_conf.mesh_x == (GRID_MAX_POINTS_X - 1) && mesh_conf.mesh_y % 2 == 1) || (mesh_conf.mesh_x == 0 && mesh_conf.mesh_y % 2 == 0))
                    mesh_conf.mesh_y--;
                  else if (mesh_conf.mesh_y % 2 == 0)
                    mesh_conf.mesh_x--;
                  else
                    mesh_conf.mesh_x++;
                  mesh_conf.manual_move();
                }
              }
              break;
            case UBL_M_OFFSET:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_OFFSET_Z));
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row, false, 100);
              }
              else {
                if (isnan(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y]))
                  Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] = 0;
                Modify_Value(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
              }
              break;
            case UBL_M_UP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_UP));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
              }
              else if (Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] < MAX_Z_OFFSET) {
                Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] += 0.01;
                gcode.process_subcommands_now(F("M290 Z0.01"));
                planner.synchronize();
                current_position.z += 0.01f;
                sync_plan_position();
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row - 1, false, 100);
                }
              break;
            case UBL_M_DOWN:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_DOWN));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
              }
              else if (Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] > MIN_Z_OFFSET) {
                Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y] -= 0.01;
                gcode.process_subcommands_now(F("M290 Z-0.01"));
                planner.synchronize();
                current_position.z -= 0.01f;
                sync_plan_position();
                Draw_Float(Z_VALUES_ARR[mesh_conf.mesh_x][mesh_conf.mesh_y], row - 2, false, 100);
              }
              break;
          }
          break;
      #endif // AUTO_BED_LEVELING_UBL && !HAS_BED_PROBE

      #if ENABLED(PROBE_MANUALLY)
        case ManualMesh:

          #define MMESH_BACK 0
          #define MMESH_MODELIVE (MMESH_BACK + 1)
          #define MMESH_NEXT (MMESH_MODELIVE + 1)
          #define MMESH_OFFSET (MMESH_NEXT + 1)
          #define MMESH_UP (MMESH_OFFSET + 1)
          #define MMESH_DOWN (MMESH_UP + 1)
          #define MMESH_OLD (MMESH_DOWN + 1)
          #define MMESH_TOTAL MMESH_OLD

          switch (item) {
            case MMESH_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BUTTON_CANCEL));
              else {
                liveadjust = false;
                gcode.process_subcommands_now(F("M211 S1\nG29 A"));
                planner.synchronize();
                set_bed_leveling_enabled(level_state);
                Draw_Menu(Leveling, LEVELING_GET_MESH);
              }
              break;
            case MMESH_MODELIVE:
              if (draw) {
                Draw_Menu_Item(row, ICON_Axis, GET_TEXT_F(MSG_LIVE_ADJUSTMENT));
                Draw_Checkbox(row, liveadjust);
              }
              else {
                liveadjust = !liveadjust;
                Draw_Checkbox(row, liveadjust);
              }
              break;
            case MMESH_NEXT:
              if (draw) {
                if (gridpoint < GRID_MAX_POINTS)
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_LEVEL_BED_NEXT_POINT));
                else
                  Draw_Menu_Item(row, ICON_More, GET_TEXT_F(MSG_UBL_SAVE_MESH));
              }
              else if (gridpoint < GRID_MAX_POINTS) {
                Popup_Handler(MoveWait);
                gcode.process_subcommands_now(F("G29"));
                planner.synchronize();
                gridpoint++;
                Redraw_Menu();
              }
              else {
                gcode.process_subcommands_now(F("G29"));
                planner.synchronize();
                AudioFeedback(settings.save());
                liveadjust = false;
                gcode.process_subcommands_now(F("M211 S1"));
                set_bed_leveling_enabled(level_state);
                Draw_Menu(Leveling, LEVELING_GET_MESH);
              }
              break;
            case MMESH_OFFSET:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_OFFSET_Z));
                #ifdef MANUAL_PROBE_START_Z
                  current_position.z = MANUAL_PROBE_START_Z;
                #endif
                Draw_Float(current_position.z, row, false, 100);
              }
              else
                Modify_Value(current_position.z, MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
              break;
            case MMESH_UP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_UP));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
              }
              else if (current_position.z < MAX_Z_OFFSET) {
                gcode.process_subcommands_now(F("M290 Z0.01"));
                planner.synchronize();
                current_position.z += 0.01f;
                sync_plan_position();
                Draw_Float(current_position.z, row - 1, false, 100);
                }
              break;
            case MMESH_DOWN:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BABYSTEP_Z), GET_TEXT(MSG_DOWN));
                Draw_Menu_Item(row, ICON_AxisD, F(cmd));
              }
              else if (current_position.z > MIN_Z_OFFSET) {
                gcode.process_subcommands_now(F("M290 Z-0.01"));
                planner.synchronize();
                current_position.z -= 0.01f;
                sync_plan_position();
                Draw_Float(current_position.z, row - 2, false, 100);
              }
              break;
            case MMESH_OLD:
              uint8_t mesh_x, mesh_y;
              // 0,0 -> 1,0 -> 2,0 -> 2,1 -> 1,1 -> 0,1 -> 0,2 -> 1,2 -> 2,2
              mesh_y = (gridpoint - 1) / GRID_MAX_POINTS_Y;
              mesh_x = (gridpoint - 1) % GRID_MAX_POINTS_X;

              if (mesh_y % 2 == 1)
                mesh_x = GRID_MAX_POINTS_X - mesh_x - 1;

              const float currval = Z_VALUES_ARR[mesh_x][mesh_y];

              if (draw) {
                Draw_Menu_Item(row, ICON_Zoffset, GET_TEXT_F(MSG_MESH_EDIT_Z));
                Draw_Float(currval, row, false, 100);
              }
              else if (!isnan(currval)) {
                current_position.z = currval;
                planner.synchronize();
                planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
                planner.synchronize();
                Draw_Float(current_position.z, row - 3, false, 100);
              }
              break;
          }
          break;
      #endif // PROBE_MANUALLY

      case Tune:

        #define TUNE_BACK 0
        #define TUNE_BACKLIGHT_OFF (TUNE_BACK + 1)
        #define TUNE_BACKLIGHT (TUNE_BACKLIGHT_OFF + 1)
        #define TUNE_SPEED (TUNE_BACKLIGHT + 1)
        #define TUNE_FLOW (TUNE_SPEED + ENABLED(HAS_HOTEND))
        #define TUNE_HOTEND (TUNE_FLOW + ENABLED(HAS_HOTEND))
        #define TUNE_BED (TUNE_HOTEND + ENABLED(HAS_HEATED_BED))
        #define TUNE_FAN (TUNE_BED + ENABLED(HAS_FAN))
        #define TUNE_ZOFFSET (TUNE_FAN + ENABLED(HAS_ZOFFSET_ITEM))
        #define TUNE_ZUP (TUNE_ZOFFSET + ENABLED(HAS_ZOFFSET_ITEM))
        #define TUNE_ZDOWN (TUNE_ZUP + ENABLED(HAS_ZOFFSET_ITEM))
        #define TUNE_FWRETRACT (TUNE_ZDOWN + ENABLED(FWRETRACT))
        #define TUNE_CHANGEFIL (TUNE_FWRETRACT + ENABLED(FILAMENT_LOAD_UNLOAD_GCODES))
        #define TUNE_FILSENSORENABLED (TUNE_CHANGEFIL + HAS_FILAMENT_SENSOR)
        #define TUNE_FILSENSORDISTANCE (TUNE_FILSENSORENABLED + HAS_FILAMENT_SENSOR)
        #define TUNE_SCREENLOCK (TUNE_FILSENSORDISTANCE + 1)     
        #define TUNE_TOTAL TUNE_SCREENLOCK

        switch (item) {
          case TUNE_BACK:
            if (draw)
              Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Print_Screen();
            break;
          case TUNE_SPEED:
            if (draw) {
              Draw_Menu_Item(row, ICON_Speed, GET_TEXT_F(MSG_SPEED));
              Draw_Float(feedrate_percentage, row, false, 1);
            }
            else
              Modify_Value(feedrate_percentage, MIN_PRINT_SPEED, MAX_PRINT_SPEED, 1);
            break;
          
          case TUNE_BACKLIGHT_OFF:
            if (draw)
              Draw_Menu_Item(row, ICON_Brightness, GET_TEXT_F(MSG_BRIGHTNESS_OFF));
            else
              ui.set_brightness(0);
            break;

          case TUNE_BACKLIGHT:
            if (draw) {
              Draw_Menu_Item(row, ICON_Brightness, GET_TEXT_F(MSG_BRIGHTNESS));
              Draw_Float(ui.brightness, row, false, 1);
            }
            else
              Modify_Value(ui.brightness, LCD_BRIGHTNESS_MIN, LCD_BRIGHTNESS_MAX, 1, ui.refresh_brightness);
            break;

          #if HAS_HOTEND
            case TUNE_FLOW:
              if (draw) {
                Draw_Menu_Item(row, ICON_Speed, GET_TEXT_F(MSG_FLOW));
                Draw_Float(planner.flow_percentage[0], row, false, 1);
              }
              else
                Modify_Value(planner.flow_percentage[0], MIN_FLOW_RATE, MAX_FLOW_RATE, 1);
              break;
            case TUNE_HOTEND:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_HOTEND_TEMPERATURE));
                Draw_Float(thermalManager.temp_hotend[0].target, row, false, 1);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].target, MIN_E_TEMP, MAX_E_TEMP, 1);
              break;
          #endif

          #if HAS_HEATED_BED
            case TUNE_BED:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetBedTemp, GET_TEXT_F(MSG_BED_TEMPERATURE));
                Draw_Float(thermalManager.temp_bed.target, row, false, 1);
              }
              else
                Modify_Value(thermalManager.temp_bed.target, MIN_BED_TEMP, MAX_BED_TEMP, 1);
              break;
          #endif

          #if HAS_FAN
            case TUNE_FAN:
              if (draw) {
                Draw_Menu_Item(row, ICON_FanSpeed, GET_TEXT_F(MSG_FAN_SPEED));
                Draw_Float(thermalManager.fan_speed[0], row, false, 1);
              }
              else
                Modify_Value(thermalManager.fan_speed[0], MIN_FAN_SPEED, MAX_FAN_SPEED, 1);
              break;
          #endif

          #if HAS_ZOFFSET_ITEM
            case TUNE_ZOFFSET:
              if (draw) {
                Draw_Menu_Item(row, ICON_SetZOffset, GET_TEXT_F(MSG_OFFSET_Z));
                Draw_Float(zoffsetvalue, row, false, 100);
              }
              else {
                Modify_Value(zoffsetvalue, MIN_Z_OFFSET, MAX_Z_OFFSET, 100);
              }
              break;
            case TUNE_ZUP:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_OFFSET_Z), GET_TEXT(MSG_UP));
                Draw_Menu_Item(row, ICON_Axis, F(cmd));
              }
              else if (zoffsetvalue < MAX_Z_OFFSET) {
                gcode.process_subcommands_now(F("M290 Z0.01"));
                zoffsetvalue += 0.01;
                Draw_Float(zoffsetvalue, row - 1, false, 100);
              }
              break;
            case TUNE_ZDOWN:
              if (draw) {
                sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_OFFSET_Z), GET_TEXT(MSG_DOWN));
                Draw_Menu_Item(row, ICON_AxisD, F(cmd));
              }
              else if (zoffsetvalue > MIN_Z_OFFSET) {
                gcode.process_subcommands_now(F("M290 Z-0.01"));
                zoffsetvalue -= 0.01;
                Draw_Float(zoffsetvalue, row - 2, false, 100);
              }
              break;
          #endif

          #if ENABLED(FWRETRACT)
            case TUNE_FWRETRACT:
              if (draw)
                Draw_Menu_Item(row, ICON_StepE, GET_TEXT_F(MSG_AUTORETRACT), nullptr, true);
              else {
                flag_tune = true;
                Draw_Menu(FwRetraction);
                }
              break;
          #endif

          #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
            case TUNE_CHANGEFIL:
              if (draw)
                Draw_Menu_Item(row, ICON_ResumeEEPROM, GET_TEXT_F(MSG_FILAMENTCHANGE));
              else {
                flag_tune = true;
                last_pos_selection = selection;
                Popup_Handler(ConfFilChange);
              }
              break;
          #endif

          #if HAS_FILAMENT_SENSOR
            case TUNE_FILSENSORENABLED:
              if (draw) {
                Draw_Menu_Item(row, ICON_Extruder, GET_TEXT_F(MSG_RUNOUT_SENSOR));
                Draw_Checkbox(row, runout.enabled[0]);
              }
              else {
                runout.enabled[0] = !runout.enabled[0];
                Draw_Checkbox(row, runout.enabled[0]);
              }
              break;
            case TUNE_FILSENSORDISTANCE:
              if (draw) {
                  editable_distance = runout.runout_distance();
                  Draw_Menu_Item(row, ICON_MaxAccE, GET_TEXT_F(MSG_RUNOUT_DISTANCE_MM));
                  Draw_Float(editable_distance, row, false, 10);
                }
                else
                  Modify_Value(editable_distance, 0, 999, 10);
              break;
          #endif
          case TUNE_SCREENLOCK:
            if (draw) 
                Draw_Menu_Item(row, ICON_Lock, GET_TEXT_F(MSG_LOCKSCREEN));
            else 
                DWIN_ScreenLock();
            break;

        }
        break;
        
      case PreheatHotend:

          #define PREHEATHOTEND_BACK 0
          #define PREHEATHOTEND_CONTINUE (PREHEATHOTEND_BACK + 1)
          #define PREHEATHOTEND_1 (PREHEATHOTEND_CONTINUE + (PREHEAT_COUNT >= 1))
          #define PREHEATHOTEND_2 (PREHEATHOTEND_1 + (PREHEAT_COUNT >= 2))
          #define PREHEATHOTEND_3 (PREHEATHOTEND_2 + (PREHEAT_COUNT >= 3))
          #define PREHEATHOTEND_4 (PREHEATHOTEND_3 + (PREHEAT_COUNT >= 4))
          #define PREHEATHOTEND_5 (PREHEATHOTEND_4 + (PREHEAT_COUNT >= 5))
          #define PREHEATHOTEND_CUSTOM (PREHEATHOTEND_5 + 1)
          #define PREHEATHOTEND_TOTAL PREHEATHOTEND_CUSTOM

          switch (item) {
            case PREHEATHOTEND_BACK:
              if (draw)
                Draw_Menu_Item(row, ICON_Back, GET_TEXT_F(MSG_BUTTON_CANCEL));
              else {
                thermalManager.setTargetHotend(0, 0);
                thermalManager.set_fan_speed(0, 0);
                Redraw_Menu(false, true, true, true);
              }
              break;
            case PREHEATHOTEND_CONTINUE:
              if (draw)
                Draw_Menu_Item(row, ICON_SetEndTemp, GET_TEXT_F(MSG_BUTTON_CONTINUE));
              else {
                Popup_Handler(Heating);
                Update_Status(GET_TEXT(MSG_HEATING));
                thermalManager.wait_for_hotend(0);
                switch (last_menu) {
                  case Prepare:
                    flag_chg_fil = true;
                    Popup_Handler(FilChange);
                    sprintf_P(cmd, PSTR("M600 B1 R%i"), thermalManager.temp_hotend[0].target);
                    gcode.process_subcommands_now(cmd);
                    flag_chg_fil = false;
                    Draw_Menu(Prepare, PREPARE_CHANGEFIL);
                    break;
                  #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
                    case ChangeFilament:
                      switch (last_selection) {
                        case CHANGEFIL_LOAD:
                          flag_chg_fil = true;
                          Popup_Handler(FilLoad);
                          Update_Status(GET_TEXT(MSG_FILAMENTLOAD)); 
                          gcode.process_subcommands_now(F("M701"));
                          planner.synchronize();
                          flag_chg_fil = false;
                          Draw_Menu(ChangeFilament, CHANGEFIL_LOAD);
                          //Redraw_Menu(true, true, true);
                          break;
                        case CHANGEFIL_UNLOAD:
                          Popup_Handler(FilLoad, true);
                          Update_Status(GET_TEXT(MSG_FILAMENTUNLOAD));
                          gcode.process_subcommands_now(F("M702"));
                          planner.synchronize();
                          Draw_Menu(ChangeFilament, CHANGEFIL_UNLOAD);
                          //Redraw_Menu(true, true, true);
                          break;
                        case CHANGEFIL_CHANGE:
                          flag_chg_fil = true;
                          Popup_Handler(FilChange);
                          Update_Status(GET_TEXT(MSG_FILAMENTCHANGE));
                          sprintf_P(cmd, PSTR("M600 B1 R%i"), thermalManager.temp_hotend[0].target);
                          gcode.process_subcommands_now(cmd);
                          flag_chg_fil = false;
                          Draw_Menu(ChangeFilament, CHANGEFIL_CHANGE);
                          break;
                      }
                      break;
                  #endif
                  default:
                    Redraw_Menu(true, true, true);
                    break;
                }
              }
              break;
            #if PREHEAT_COUNT >= 1
              case PREHEATHOTEND_1:
                if (draw)
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_1_LABEL));
                else
                  ui.preheat_hotend_and_fan(0);
                break;
            #endif
            #if PREHEAT_COUNT >= 2
              case PREHEATHOTEND_2:
                if (draw)
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_2_LABEL));
                else
                  ui.preheat_hotend_and_fan(1);
                break;
            #endif
            #if PREHEAT_COUNT >= 3
              case PREHEATHOTEND_3:
                if (draw)
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_3_LABEL));
                else
                  ui.preheat_hotend_and_fan(2);
                break;
            #endif
            #if PREHEAT_COUNT >= 4
              case PREHEATHOTEND_4:
                if (draw)
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_4_LABEL));
                else
                  ui.preheat_hotend_and_fan(3);
                break;
            #endif
            #if PREHEAT_COUNT >= 5
              case PREHEATHOTEND_5:
                if (draw)
                  Draw_Menu_Item(row, ICON_Temperature, F(PREHEAT_5_LABEL));
                else
                  ui.preheat_hotend_and_fan(4);
                break;
            #endif
            case PREHEATHOTEND_CUSTOM:
              if (draw) {
                Draw_Menu_Item(row, ICON_Temperature, GET_TEXT_F(MSG_PREHEAT_CUSTOM));
                Draw_Float(thermalManager.temp_hotend[0].target, row, false, 1);
              }
              else
                Modify_Value(thermalManager.temp_hotend[0].target, EXTRUDE_MINTEMP, MAX_E_TEMP, 1);
              break;
          }
          break;
    }
  }

  FSTR_P CrealityDWINClass::Get_Menu_Title(uint8_t menu) {
    switch (menu) {
      case MainMenu:          return GET_TEXT_F(MSG_MAIN);
      case Prepare:           return GET_TEXT_F(MSG_PREPARE);
      case HomeMenu:          return GET_TEXT_F(MSG_HOMING);
      case Move:              return GET_TEXT_F(MSG_MOVE_AXIS);
      case ManualLevel:       return GET_TEXT_F(MSG_BED_TRAMMING_MANUAL);
      #if HAS_ZOFFSET_ITEM
        case ZOffset:         return GET_TEXT_F(MSG_OFFSET_Z);
      #endif
      #if HAS_PREHEAT
        case Preheat:         return GET_TEXT_F(MSG_PREHEAT);
      #endif
      #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
        case ChangeFilament:  return GET_TEXT_F(MSG_FILAMENTCHANGE);
      #endif
      #if ENABLED(HOST_ACTION_COMMANDS)
        case HostActions:       return GET_TEXT_F(MSG_HOST_ACTIONS);
      #endif
      case Control:           return GET_TEXT_F(MSG_CONTROL);
      case TempMenu:          return GET_TEXT_F(MSG_TEMPERATURE);
      #if HAS_HOTEND || HAS_HEATED_BED
        case PID:             
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_PID), GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if HAS_HOTEND
        case HotendPID:       
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_HOTEND_TEMPERATURE), GET_TEXT(MSG_PID));
              return F(cmd);
      #endif
      #if HAS_HEATED_BED
        case BedPID:          
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BED_TEMPERATURE), GET_TEXT(MSG_PID));
              return F(cmd);
      #endif
      #if PREHEAT_COUNT >= 1
        case Preheat1:        
              sprintf_P(cmd, PSTR("%s %s"), PREHEAT_1_LABEL, GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if PREHEAT_COUNT >= 2
        case Preheat2:        
              sprintf_P(cmd, PSTR("%s %s"), PREHEAT_2_LABEL, GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if PREHEAT_COUNT >= 3
        case Preheat3:        
              sprintf_P(cmd, PSTR("%s %s"), PREHEAT_3_LABEL, GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if PREHEAT_COUNT >= 4
        case Preheat4:        
              sprintf_P(cmd, PSTR("%s %s"), PREHEAT_4_LABEL, GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if PREHEAT_COUNT >= 5
        case Preheat5:        
              sprintf_P(cmd, PSTR("%s %s"), PREHEAT_5_LABEL, GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      case Motion:
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_MOTION), GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #if ENABLED(FWRETRACT)
        case FwRetraction:
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_FWRETRACT), GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      #if ENABLED(NOZZLE_PARK_FEATURE)
        case Parkmenu:
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_FILAMENT_PARK_ENABLED), GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
      #endif
      case HomeOffsets:       return GET_TEXT_F(MSG_SET_HOME_OFFSETS);
      case MaxSpeed:          return GET_TEXT_F(MSG_SPEED);
      case MaxAcceleration:   return GET_TEXT_F(MSG_ACCELERATION);
      #if HAS_CLASSIC_JERK
        case MaxJerk:         return GET_TEXT_F(MSG_JERK);
      #endif
      #if HAS_JUNCTION_DEVIATION
        case JDmenu:          return GET_TEXT_F(MSG_JUNCTION_DEVIATION_MENU);
      #endif
      case Steps:             return GET_TEXT_F(MSG_STEPS_PER_MM);
      case Visual:            return GET_TEXT_F(MSG_VISUAL_SETTINGS);
      case HostSettings:      return GET_TEXT_F(MSG_HOST_SETTINGS);
      #if ENABLED(HOST_ACTION_COMMANDS)
        case ActionCommands:    return GET_TEXT_F(MSG_HOST_ACTIONS);
      #endif
      case Advanced:          return GET_TEXT_F(MSG_ADVANCED_SETTINGS);
      #if HAS_BED_PROBE
        case ProbeMenu:       return GET_TEXT_F(MSG_ZPROBE_SETTINGS);
      #endif
      case Filmenu:           return GET_TEXT_F(MSG_FILAMENT_SET);
      case ColorSettings:     return GET_TEXT_F(MSG_COLORS_SELECT);
      case Info:              return GET_TEXT_F(MSG_INFO_SCREEN);
      case InfoMain:          return GET_TEXT_F(MSG_INFO_SCREEN);
      #if HAS_MESH
        case Leveling:        return GET_TEXT_F(MSG_BED_LEVELING);
        case LevelView:       return GET_TEXT_F(MSG_MESH_VIEW);
        case LevelSettings:
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_BED_LEVELING), GET_TEXT(MSG_CONFIGURATION));
              return F(cmd);
        case MeshViewer:      return GET_TEXT_F(MSG_MESH_VIEW);
        case LevelManual:     return GET_TEXT_F(MSG_UBL_FINE_TUNE_MESH);
      #endif
      #if ENABLED(AUTO_BED_LEVELING_UBL) && !HAS_BED_PROBE
        case UBLMesh:         return GET_TEXT_F(MSG_UBL_LEVEL_BED);
      #endif
      #if ENABLED(PROBE_MANUALLY)
        case ManualMesh:      return GET_TEXT_F(MSG_MESH_LEVELING);
      #endif
      case Tune:              return GET_TEXT_F(MSG_TUNE);
      case PreheatHotend:
              sprintf_P(cmd, PSTR("%s %s"), GET_TEXT(MSG_PREHEAT), GET_TEXT(MSG_HOTEND));
              return F(cmd);
    }
    return F("");
  }

  uint8_t CrealityDWINClass::Get_Menu_Size(uint8_t menu) {
    switch (menu) {
      case Prepare:           return PREPARE_TOTAL;
      case HomeMenu:          return HOME_TOTAL;
      case Move:              return MOVE_TOTAL;
      case ManualLevel:       return MLEVEL_TOTAL;
      #if HAS_ZOFFSET_ITEM
        case ZOffset:         return ZOFFSET_TOTAL;
      #endif
      #if HAS_PREHEAT
        case Preheat:         return PREHEAT_TOTAL;
      #endif
      #if ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)
        case ChangeFilament:  return CHANGEFIL_TOTAL;
      #endif
      #if ENABLED(HOST_ACTION_COMMANDS)
        case HostActions:       return HOSTACTIONS_TOTAL;
      #endif
      case Control:           return CONTROL_TOTAL;
      case TempMenu:          return TEMP_TOTAL;
      #if HAS_HOTEND || HAS_HEATED_BED
        case PID:             return PID_TOTAL;
      #endif
      #if HAS_HOTEND
        case HotendPID:       return HOTENDPID_TOTAL;
      #endif
      #if HAS_HEATED_BED
        case BedPID:          return BEDPID_TOTAL;
      #endif
      #if PREHEAT_COUNT >= 1
        case Preheat1:        return PREHEAT1_TOTAL;
      #endif
      #if PREHEAT_COUNT >= 2
        case Preheat2:        return PREHEAT2_TOTAL;
      #endif
      #if PREHEAT_COUNT >= 3
        case Preheat3:        return PREHEAT3_TOTAL;
      #endif
      #if PREHEAT_COUNT >= 4
        case Preheat4:        return PREHEAT4_TOTAL;
      #endif
      #if PREHEAT_COUNT >= 5
        case Preheat5:        return PREHEAT5_TOTAL;
      #endif
      case Motion:            return MOTION_TOTAL;
      #if ENABLED(FWRETRACT)
        case FwRetraction:    return FWR_TOTAL;
      #endif
      #if ENABLED(NOZZLE_PARK_FEATURE)
        case Parkmenu:        return PARKMENU_TOTAL;
      #endif
      case HomeOffsets:       return HOMEOFFSETS_TOTAL;
      case MaxSpeed:          return SPEED_TOTAL;
      case MaxAcceleration:   return ACCEL_TOTAL;
      #if HAS_CLASSIC_JERK
        case MaxJerk:         return JERK_TOTAL;
      #endif
      #if HAS_JUNCTION_DEVIATION
        case JDmenu:          return JD_TOTAL;
      #endif
      case Steps:             return STEPS_TOTAL;
      case Visual:            return VISUAL_TOTAL;
      case HostSettings:      return HOSTSETTINGS_TOTAL;
      #if ENABLED(HOST_ACTION_COMMANDS)
        case ActionCommands:    return ACTIONCOMMANDS_TOTAL;
      #endif
      case Advanced:          return ADVANCED_TOTAL;
      #if HAS_BED_PROBE
        case ProbeMenu:       return PROBE_TOTAL;
      #endif
      case Filmenu:           return FIL_TOTAL;
      case Info:              return INFO_TOTAL;
      case InfoMain:          return INFO_TOTAL;
      #if ENABLED(AUTO_BED_LEVELING_UBL) && !HAS_BED_PROBE
        case UBLMesh:         return UBL_M_TOTAL;
      #endif
      #if ENABLED(PROBE_MANUALLY)
        case ManualMesh:      return MMESH_TOTAL;
      #endif
      #if HAS_MESH
        case Leveling:        return LEVELING_TOTAL;
        case LevelView:       return LEVELING_VIEW_TOTAL;
        case LevelSettings:   return LEVELING_SETTINGS_TOTAL;
        case MeshViewer:      return MESHVIEW_TOTAL;
        case LevelManual:     return LEVELING_M_TOTAL;
      #endif
      case Tune:              return TUNE_TOTAL;
      case PreheatHotend:     return PREHEATHOTEND_TOTAL;
      case ColorSettings:     return COLORSETTINGS_TOTAL;
    }
    return 0;
  }

  /* Popup Config */

  void CrealityDWINClass::Popup_Handler(PopupID popupid, bool option/*=false*/) {
    popup = last_popup = popupid;
    switch (popupid) {
      case Pause:         Draw_Popup(GET_TEXT_F(MSG_PAUSE_PRINT), F(""), F(""), Popup); break;
      case Stop:          Draw_Popup(GET_TEXT_F(MSG_STOP_PRINT), F(""), F(""), Popup); break;
      case Resume:        Draw_Popup(GET_TEXT_F(MSG_RESUME_PRINT), GET_TEXT_F(MSG_RESUME_PRINT2), GET_TEXT_F(MSG_RESUME_PRINT3), Popup); break;
      case ConfFilChange: 
            sprintf_P(cmd,PSTR("%s %s"),GET_TEXT(MSG_BUTTON_CONFIRM),GET_TEXT(MSG_FILAMENT_CHANGE));
            Draw_Popup(F(cmd), F(""), F(""), Popup); 
            break;
      case PurgeMore:     Draw_Popup(GET_TEXT_F(MSG_FILAMENT_CHANGE_OPTION_PURGE), GET_TEXT_F(MSG_FILAMENT_CHANGE_FINISH), F(""), Popup); break;
      case SaveLevel:     Draw_Popup(option ? GET_TEXT_F(MSG_UBL_FINE_TUNE_MESH_COMPLETE) : GET_TEXT_F(MSG_LEVEL_BED_DONE), GET_TEXT_F(MSG_UBL_SAVE_TO_EEPROM), F(""), Popup); break;
      case MeshSlot:      Draw_Popup(GET_TEXT_F(MSG_NO_VALID_MESH_SLOT), option ? GET_TEXT_F(MSG_NO_VALID_MESH2) : GET_TEXT_F(MSG_NO_VALID_MESH3), GET_TEXT_F(MSG_NO_VALID_MESH4), Popup, ICON_AutoLeveling); break;
      case ETemp:         Draw_Popup(GET_TEXT_F(MSG_HOTEND_TOO_COLD), GET_TEXT_F(MSG_HOTEND_TOO_COLD2), F(""), Popup); break;
      case ManualProbing: Draw_Popup(GET_TEXT_F(MSG_MANUAL_PROBING), GET_TEXT_F(MSG_MANUAL_PROBING_CONFIRM), GET_TEXT_F(MSG_MANUAL_PROBING_CANCEL), Popup, ICON_AutoLeveling); break;
      case Level:         Draw_Popup(option ? GET_TEXT_F(MSG_PROBING_IN_PROGRESS) : GET_TEXT_F(MSG_AUTO_BED_LEVELING), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_AutoLeveling); break;
      case Home:          Draw_Popup(option ? GET_TEXT_F(MSG_PAUSE_PRINT_PARKING) : GET_TEXT_F(MSG_HOMING), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case MoveWait:      Draw_Popup(GET_TEXT_F(MSG_MOVING), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case Heating:       Draw_Popup(GET_TEXT_F(MSG_HEATING), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case FilLoad:       Draw_Popup(option ? GET_TEXT_F(MSG_FILAMENT_UNLOADING) : GET_TEXT_F(MSG_FILAMENT_LOADING), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case FilChange:     Draw_Popup(option ? GET_TEXT_F(MSG_END_PROCESS) : GET_TEXT_F(MSG_FILAMENT_CHANGE), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case TempWarn:      Draw_Popup(option ? GET_TEXT_F(MSG_HOTEND_TOO_COLD) : GET_TEXT_F(MSG_HOTEND_TOO_HOT), F(""), F(""), Wait, option ? ICON_TempTooLow : ICON_TempTooHigh); break;
      case Runout:        Draw_Popup(GET_TEXT_F(MSG_FILAMENT_RUNOUT), F(""), F(""), Wait, ICON_BLTouch); break;
      case PIDWait:       Draw_Popup(option ? GET_TEXT_F(MSG_BED_PID_AUTOTUNE) : GET_TEXT_F(MSG_HOTEND_PID_AUTOTUNE), GET_TEXT_F(MSG_IN_PROGRESS), GET_TEXT_F(MSG_PLEASE_WAIT), Wait, ICON_BLTouch); break;
      case Resuming:      Draw_Popup(GET_TEXT_F(MSG_RESUMING_PRINT), GET_TEXT_F(MSG_PLEASE_WAIT), F(""), Wait, ICON_BLTouch); break;
      case ConfirmStartPrint: Draw_Popup(option ? GET_TEXT_F(MSG_LOADING_PREVIEW) : GET_TEXT_F(MSG_PRINT_FILE), F(""), F(""), Popup); break;
      case Reprint:       Draw_Popup(GET_TEXT_F(MSG_REPRINT_FILE), F(""), F(""), Popup); break;
      default: break;
    }
  }

  void CrealityDWINClass::Confirm_Handler(PopupID popupid, bool option/*=false*/) {
    popup = popupid;
    switch (popupid) {
      case FilInsert:         Draw_Popup(GET_TEXT_F(MSG_INSERT_FILAMENT), GET_TEXT_F(MSG_ADVANCED_PAUSE_WAITING), F(""), Confirm); break;
      case HeaterTime:        Draw_Popup(GET_TEXT_F(MSG_HEATER_TIMEOUT), GET_TEXT_F(MSG_FILAMENT_CHANGE_HEAT), F(""), Confirm); break;
      case UserInput:         Draw_Popup(option ? GET_TEXT_F(MSG_STOPPED) :  GET_TEXT_F(MSG_WAITING_FOR_INPUT), GET_TEXT_F(MSG_ADVANCED_PAUSE_WAITING), F(""), Confirm); break;
      case LevelError:        Draw_Popup(GET_TEXT_F(MSG_COULDNT_ENABLE_LEVELING), GET_TEXT_F(MSG_VALID_MESH_MUST_EXIST), F(""), Confirm); break;
      case InvalidMesh:       Draw_Popup(GET_TEXT_F(MSG_VALID_MESH_MUST_EXIST), GET_TEXT_F(MSG_VALID_MESH_MUST_EXIST2), GET_TEXT_F(MSG_VALID_MESH_MUST_EXIST3), Confirm); break;
      case NocreatePlane:     Draw_Popup(GET_TEXT_F(MSG_COULDNT_CREATE_PLANE), GET_TEXT_F(MSG_VALID_MESH_MUST_EXIST), F(""), Confirm); break;
      case BadextruderNumber: Draw_Popup(GET_TEXT_F(MSG_PID_FAILED), GET_TEXT_F(MSG_PID_BAD_EXTRUDER_NUM), F(""), Confirm); break;
      case TemptooHigh:       Draw_Popup(GET_TEXT_F(MSG_PID_FAILED), GET_TEXT_F(MSG_PID_TEMP_TOO_HIGH), F(""), Confirm); break;
      case PIDTimeout:        Draw_Popup(GET_TEXT_F(MSG_PID_FAILED), GET_TEXT_F(MSG_PID_TIMEOUT), F(""), Confirm); break;
      case PIDDone:           Draw_Popup(GET_TEXT_F(MSG_PID_AUTOTUNE_DONE), F(""), F(""), Confirm); break;
      case Level2:            Draw_Popup(GET_TEXT_F(MSG_AUTO_BED_LEVELING), GET_TEXT_F(MSG_PLEASE_WAIT), GET_TEXT_F(MSG_CANCEL_TO_STOP), Confirm, ICON_AutoLeveling); break;
      
      default: break;
    }
  }

  /* Navigation and Control */

  void CrealityDWINClass::Main_Menu_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW && selection < PAGE_COUNT - 1) {
      selection++; // Select Down
      Main_Menu_Icons();
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && selection > 0) {
      selection--; // Select Up
      Main_Menu_Icons();
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER)
      switch (selection) {
        case PAGE_PRINT: card.mount(); sd_item_flag = true; Draw_SD_List(); break;
        case PAGE_PREPARE: sd_item_flag = false; Draw_Menu(Prepare); break;
        case PAGE_CONTROL: sd_item_flag = false; Draw_Menu(Control); break;
        case PAGE_INFO_LEVELING: sd_item_flag = false; Draw_Menu(TERN(HAS_MESH, Leveling, InfoMain)); break;
        case PAGE_SHORTCUT0 : sd_item_flag = false; Apply_shortcut(shortcut0); break;
        case PAGE_SHORTCUT1 : sd_item_flag = false; Apply_shortcut(shortcut1); break;
      }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Menu_Control() {
    uint16_t cColor = GetColor(HMI_datas.cursor_color, Rectangle_Color);
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW && selection < Get_Menu_Size(active_menu)) {
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 33);
      selection++; // Select Down
      if (selection > scrollpos+MROWS) {
        scrollpos++;
        DWIN_Frame_AreaMove(1, 2, MLINE, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, 349);
        Menu_Item_Handler(active_menu, selection);
      }
      if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0)))
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 31);
      else
        DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 31);
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && selection > 0) {
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 33);
      selection--; // Select Up
      if (selection < scrollpos) {
        scrollpos--;
        DWIN_Frame_AreaMove(1, 3, MLINE, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, 349);
        Menu_Item_Handler(active_menu, selection);
      }
      if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0)))
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection-scrollpos) - 18, 14, MBASE(selection-scrollpos) + 31);
      else
        DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 31);
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER)
      Menu_Item_Handler(active_menu, selection, false);
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Value_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    float valuegap = 0; 
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW) {
      valuegap += EncoderRate.encoderMoveValue;
      tempvalue += EncoderRate.encoderMoveValue;
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW) {
      valuegap -= EncoderRate.encoderMoveValue;
      tempvalue -= EncoderRate.encoderMoveValue;
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      process = Menu;
      EncoderRate.enabled = false;
      Draw_Float(tempvalue / valueunit, selection - scrollpos, false, valueunit);
      DWIN_UpdateLCD();
      //if (active_menu == ZOffset && (adjustonclick || liveadjust)) {
      if (active_menu == ZOffset && zoffsetmode != 0) {
        planner.synchronize();
        if (zoffsetmode == 1) {
          current_position.z += (tempvalue / valueunit - zoffsetvalue);
          planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
        }
        current_position.z = 0;
        sync_plan_position();
      }
      else if (active_menu == Tune && selection == TUNE_ZOFFSET) {
        sprintf_P(cmd, PSTR("M290 Z%s"), dtostrf((tempvalue / valueunit - zoffsetvalue), 1, 3, str_1));
        gcode.process_subcommands_now(cmd);
      }
      if (TERN0(PIDTEMP, valuepointer == &thermalManager.temp_hotend[0].pid.Ki) || TERN0(PIDTEMPBED, valuepointer == &thermalManager.temp_bed.pid.Ki))
        tempvalue = scalePID_i(tempvalue);
      if (TERN0(PIDTEMP, valuepointer == &thermalManager.temp_hotend[0].pid.Kd) || TERN0(PIDTEMPBED, valuepointer == &thermalManager.temp_bed.pid.Kd))
        tempvalue = scalePID_d(tempvalue);
      switch (valuetype) {
        case 0: *(float*)valuepointer = tempvalue / valueunit; break;
        case 1: *(uint8_t*)valuepointer = tempvalue / valueunit; break;
        case 2: *(uint16_t*)valuepointer = tempvalue / valueunit; break;
        case 3: *(int16_t*)valuepointer = tempvalue / valueunit; break;
        case 4: *(uint32_t*)valuepointer = tempvalue / valueunit; break;
        case 5: *(int8_t*)valuepointer = tempvalue / valueunit; break;
      }
      switch (active_menu) {
        case Move:
          planner.synchronize();
          planner.buffer_line(current_position, manual_feedrate_mm_s[selection - 1], active_extruder);
          break;
        #if HAS_MESH
          case ManualMesh:
            planner.synchronize();
            planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
            planner.synchronize();
            break;
          case UBLMesh:     mesh_conf.manual_move(true); break;
          //case LevelManual: mesh_conf.manual_move(selection == LEVELING_M_OFFSET); break;
          case LevelManual: mesh_conf.manual_move(selection == LEVELING_M_OFFSET); break;
        #endif
      }
      if (valuepointer == &planner.flow_percentage[0])
        planner.refresh_e_factor(0);
      #if HAS_FILAMENT_SENSOR
        if (valuepointer == &editable_distance)
          runout.set_runout_distance(editable_distance, 0);
      #endif
      if (funcpointer) funcpointer();
      return;
    }
    NOLESS(tempvalue, (valuemin * valueunit));
    NOMORE(tempvalue, (valuemax * valueunit));
    Draw_Float(tempvalue / valueunit, selection - scrollpos, true, valueunit);
    DWIN_UpdateLCD();

      switch (active_menu) {
          case Move:
            if (livemove) {
              *(float*)valuepointer = tempvalue / valueunit;
              planner.buffer_line(current_position, manual_feedrate_mm_s[selection - 1], active_extruder);
            }
            break;
          #if HAS_MESH
            case ManualMesh:
              if (liveadjust) {
                planner.synchronize();
                *(float*)valuepointer = tempvalue / valueunit;
                planner.buffer_line(current_position, homing_feedrate(Z_AXIS), active_extruder);
                planner.synchronize();
              }
              break;
            case UBLMesh:
              if (liveadjust) {     
                planner.synchronize();
                sprintf_P(cmd, PSTR("M290 Z%s"), dtostrf((valuegap / valueunit), 1, 3, str_1));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
              }
              break;
            case LevelManual: 
              if (liveadjust && mesh_conf.goto_mesh_value) { 
                planner.synchronize();
                sprintf_P(cmd, PSTR("M290 Z%s"), dtostrf((valuegap / valueunit), 1, 3, str_1));
                gcode.process_subcommands_now(cmd);
                planner.synchronize();
              }
              break;
          #endif
          case ZOffset:
            if (zoffsetmode == 2) {
              planner.synchronize();
              sprintf_P(cmd, PSTR("M290 Z%s"), dtostrf((valuegap / valueunit), 1, 3, str_1));
              gcode.process_subcommands_now(cmd);
              planner.synchronize();
            }
            break;
          case Tune:
            if (selection == TUNE_ZOFFSET) {
              planner.synchronize();
              sprintf_P(cmd, PSTR("M290 Z%s"), dtostrf((valuegap / valueunit), 1, 3, str_1));
              gcode.process_subcommands_now(cmd);
              planner.synchronize();
            }
            break;
          default: break;   
        }
  }

  void CrealityDWINClass::Option_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW)
      tempvalue += EncoderRate.encoderMoveValue;
    else if (encoder_diffState == ENCODER_DIFF_CCW)
      tempvalue -= EncoderRate.encoderMoveValue;
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      process = Menu;
      EncoderRate.enabled = false;
      if (valuepointer == &color_names) {
        switch (selection) {
          case COLORSETTINGS_CURSOR: HMI_datas.cursor_color = tempvalue; break;
          case COLORSETTINGS_SPLIT_LINE: HMI_datas.menu_split_line = tempvalue; break;
          case COLORSETTINGS_ITEMS_MENU_TEXT: HMI_datas.items_menu_text =  tempvalue; break;
          case COLORSETTINGS_ICONS_MENU_TEXT: HMI_datas.icons_menu_text =  tempvalue; break;
          case COLORSETTINGS_BACKGROUND: HMI_datas.background =  tempvalue; break;
          case COLORSETTINGS_MENU_TOP_TXT: HMI_datas.menu_top_txt = tempvalue; break;
          case COLORSETTINGS_MENU_TOP_BG: HMI_datas.menu_top_bg = tempvalue; break;
          case COLORSETTINGS_SELECT_TXT: HMI_datas.select_txt = tempvalue; break;
          case COLORSETTINGS_SELECT_BG: HMI_datas.select_bg = tempvalue; break;
          case COLORSETTINGS_HIGHLIGHT_BORDER: HMI_datas.highlight_box = tempvalue; break;
          case COLORSETTINGS_POPUP_HIGHLIGHT: HMI_datas.popup_highlight = tempvalue; break;
          case COLORSETTINGS_POPUP_TXT: HMI_datas.popup_txt = tempvalue; break;
          case COLORSETTINGS_POPUP_BG: HMI_datas.popup_bg = tempvalue; break;
          case COLORSETTINGS_ICON_CONFIRM_TXT: HMI_datas.ico_confirm_txt = tempvalue; break;
          case COLORSETTINGS_ICON_CONFIRM_BG: HMI_datas.ico_confirm_bg = tempvalue; break;
          case COLORSETTINGS_ICON_CANCEL_TXT: HMI_datas.ico_cancel_txt = tempvalue; break;
          case COLORSETTINGS_ICON_CANCEL_BG: HMI_datas.ico_cancel_bg = tempvalue; break;
          case COLORSETTINGS_ICON_CONTINUE_TXT: HMI_datas.ico_continue_txt = tempvalue; break;
          case COLORSETTINGS_ICON_CONTINUE_BG: HMI_datas.ico_continue_bg = tempvalue; break;
          case COLORSETTINGS_PRINT_SCREEN_TXT: HMI_datas.print_screen_txt = tempvalue; break;
          case COLORSETTINGS_PRINT_FILENAME: HMI_datas.print_filename = tempvalue; break;
          case COLORSETTINGS_PROGRESS_BAR: HMI_datas.progress_bar = tempvalue; break;
          case COLORSETTINGS_PROGRESS_PERCENT: HMI_datas.progress_percent = tempvalue; break;
          case COLORSETTINGS_REMAIN_TIME: HMI_datas.remain_time = tempvalue; break;
          case COLORSETTINGS_ELAPSED_TIME: HMI_datas.elapsed_time = tempvalue; break;
          case COLORSETTINGS_PROGRESS_STATUS_BAR: HMI_datas.status_bar_text = tempvalue; break;
          case COLORSETTINGS_PROGRESS_STATUS_AREA: HMI_datas.status_area_text = tempvalue; break;
          case COLORSETTINGS_PROGRESS_STATUS_PERCENT: HMI_datas.status_area_percent = tempvalue; break;
          case COLORSETTINGS_PROGRESS_COORDINATES: HMI_datas.coordinates_text = tempvalue; break;
          case COLORSETTINGS_PROGRESS_COORDINATES_LINE: HMI_datas.coordinates_split_line = tempvalue; break;
        }
        Redraw_Screen();
      }
      else if (valuepointer == &shortcut_list) {
        if (flag_shortcut) shortcut1 = tempvalue;
        else shortcut0 = tempvalue;
      }
      else if (valuepointer == &preheat_modes)
        preheatmode = tempvalue;
      else if (valuepointer == &zoffset_modes) {
        zoffsetmode = tempvalue;
        if (zoffsetmode == 1 || zoffsetmode == 2) {
          if (axes_should_home()) {
            Popup_Handler(Home);
            gcode.home_all_axes(true);
          }
          Popup_Handler(MoveWait);
          #if ENABLED(Z_SAFE_HOMING)
            planner.synchronize();
            sprintf_P(cmd, PSTR("G0 F4000 X%s Y%s"), dtostrf(Z_SAFE_HOMING_X_POINT, 1, 3, str_1), dtostrf(Z_SAFE_HOMING_Y_POINT, 1, 3, str_2));
            gcode.process_subcommands_now(cmd);
          #else
            sprintf_P(cmd, PSTR("G0 F4000 X%s Y%s"), dtostrf((X_BED_SIZE + X_MIN_POS) / 2.0f, 1, 3, str_1), dtostrf((Y_BED_SIZE + Y_MIN_POS) / 2.0f, 1, 3, str_2));
            gcode.process_subcommands_now(cmd);
          #endif
          gcode.process_subcommands_now(F("G0 F300 Z0"));
          planner.synchronize();
          Redraw_Menu();
        }
      }
      #if HAS_FILAMENT_SENSOR
        else if (valuepointer == &runoutsensor_modes) {
          rsensormode = tempvalue;
          runout.reset();
          switch (rsensormode) {
           case 0: runout.mode[0] = RM_NONE; break; // None 
           case 1: runout.mode[0] = RM_OUT_ON_HIGH; break; // mode HIGH
           case 2: runout.mode[0] = RM_OUT_ON_LOW; break; // mode LOW
           case 3: runout.mode[0] = RM_MOTION_SENSOR; break; // mode MOTION
          }
          runout.setup();
          runout.reset();
          runout.enabled[0] = State_runoutenable;
          Redraw_Menu(false);
        }
      #endif  

      Draw_Option(tempvalue, static_cast<const char * const *>(valuepointer), selection - scrollpos, false, (valuepointer == &color_names));
      DWIN_UpdateLCD();
      return;
    }
    NOLESS(tempvalue, valuemin);
    NOMORE(tempvalue, valuemax);
    Draw_Option(tempvalue, static_cast<const char * const *>(valuepointer), selection - scrollpos, true);
    DWIN_UpdateLCD();
  }


  #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
  bool CrealityDWINClass::find_and_decode_gcode_preview(char *name, uint8_t preview_type, uint16_t *address, bool onlyCachedFileIcon/*=false*/) {
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
      //image_cache[file_path+to_string(preview_type)] = 0;
      image_cache[file_path+to_string(preview_type)] = 0;
    }

    card.closefile(); 
    gcode.process_subcommands_now(F("M117")); // Clear the message sent by the card API
    return encoded_image;
  }

  bool CrealityDWINClass::find_and_decode_gcode_header(char *name, uint8_t header_type) {
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
      sprintf_P(str_1, PSTR("%s"), out_buf);
    }
    card.closefile();
  return encoded_header;
  }
  #endif // G-code preview



  void CrealityDWINClass::File_Control() {
    uint16_t cColor = GetColor(HMI_datas.cursor_color, Rectangle_Color);
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    static uint8_t filescrl = 0;
    if (encoder_diffState == ENCODER_DIFF_NO) {
      if (selection > 0) {
        card.getfilename_sorted(SD_ORDER(selection - 1, card.get_num_Files()));
        char * const filename = card.longest_filename();
        size_t len = strlen(filename);
        int8_t pos = len;
        if (!card.flag.filenameIsDir)
          while (pos && filename[pos] != '.') pos--;
        if (pos > MENU_CHAR_LIMIT) {
          static millis_t time = 0;
          if (PENDING(millis(), time)) return;
          time = millis() + 200;
          pos -= filescrl;
          len = _MIN((size_t)pos, (size_t)MENU_CHAR_LIMIT);
          char name[len + 1];
          if (pos >= 0) {
            LOOP_L_N(i, len) name[i] = filename[i + filescrl];
          }
          else {
            LOOP_L_N(i, MENU_CHAR_LIMIT + pos) name[i] = ' ';
            LOOP_S_L_N(i, MENU_CHAR_LIMIT + pos, MENU_CHAR_LIMIT) name[i] = filename[i - (MENU_CHAR_LIMIT + pos)];
          }
          name[len] = '\0';
          DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), LBLX, MBASE(selection - scrollpos) - 14, 271, MBASE(selection - scrollpos) + 28);
          //Draw_Menu_Item(selection - scrollpos, card.flag.filenameIsDir ? ICON_More : ICON_File, name);
          Draw_Menu_Item(selection-scrollpos, card.flag.filenameIsDir ? ICON_More : ICON_File, name, NULL, NULL, false, true);
          if (-pos >= MENU_CHAR_LIMIT) filescrl = 0;
          filescrl++;
          DWIN_UpdateLCD();
        }
      }
      return;
    }
    if (encoder_diffState == ENCODER_DIFF_CW && selection < card.get_num_Files()) {
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 33);
      if (selection > 0) {
        DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), LBLX, MBASE(selection - scrollpos) - 14, 271, MBASE(selection - scrollpos) + 28);
        Draw_SD_Item(selection, selection - scrollpos, true);
      }
      filescrl = 0;
      selection++; // Select Down
      if (selection > scrollpos + MROWS) {
        scrollpos++;
        DWIN_Frame_AreaMove(1, 2, MLINE, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, 349);
        Draw_SD_Item(selection, selection - scrollpos, true);
      }

      #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
        thumbtime = millis() + SCROLL_WAIT;
        name_scroll_time = millis() + SCROLL_WAIT;
      #endif
      if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0))) {
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
      }
      else
      DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && selection > 0) {
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, MBASE(selection - scrollpos) - 18, 14, MBASE(selection - scrollpos) + 33);
      DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), LBLX, MBASE(selection - scrollpos) - 14, 271, MBASE(selection - scrollpos) + 28);
      Draw_SD_Item(selection, selection - scrollpos, true);
      filescrl = 0;
      selection--; // Select Up
      if (selection < scrollpos) {
        scrollpos--;
        DWIN_Frame_AreaMove(1, 3, MLINE, GetColor(HMI_datas.background, Color_Bg_Black), 0, 31, DWIN_WIDTH, 349);
        Draw_SD_Item(selection, selection - scrollpos, true);
      }

      #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
        thumbtime = millis() + SCROLL_WAIT;
        name_scroll_time = millis() + SCROLL_WAIT;
      #endif
      
      if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background == 0))) 
        DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
      else
        DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      if (selection == 0) {
        if (card.flag.workDirIsRoot) {
          process = Main;
          Draw_Main_Menu();
        }
        else {
          card.cdup();
          Draw_SD_List();
        }
      }
      else {
        card.getfilename_sorted(SD_ORDER(selection - 1, card.get_num_Files())); 
        if (card.flag.filenameIsDir) {
          card.cd(card.filename);
          Draw_SD_List();
        }
        else {
          #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
          uint16_t image_address;
          bool has_preview = find_and_decode_gcode_preview(card.filename, Thumnail_Preview, &image_address);
          file_preview = has_preview;
          bool has_header_time = find_and_decode_gcode_header(card.filename, Header_Time);
          if (has_header_time) {
            header_time_s = atof(str_1); 
            sprintf_P(header1, GET_TEXT(MSG_HEADER_TIME), (uint8_t)(header_time_s / 3600), (uint8_t)((header_time_s / 60)%60), (uint8_t)(header_time_s%60));
          }
          bool has_header_filament = find_and_decode_gcode_header(card.filename, Header_Filament);
          if (has_header_filament) {
            size_t nb = 0;
            for (size_t i = 0; i<strlen(str_1); i++)
            {
              nb = i;
              str_2[i] = str_1[i];
            if (str_1[i] == '.') break;
            }
            for (size_t i = 1; i<3; i++)
            {
            str_2[nb+i] = str_1[nb+i];
            }
            sprintf_P(header2, GET_TEXT(MSG_HEADER_FILAMENT_USED), str_2);
          }
          bool has_header_layer = find_and_decode_gcode_header(card.filename, Header_Layer);
          if (has_header_layer) {
            sprintf_P(header3, GET_TEXT(MSG_HEADER_LAYER_HEIGHT), dtostrf(atof(str_1), 1, 2, str_3));
          }
          Popup_Handler(ConfirmStartPrint, has_preview);
          Draw_Title(GET_TEXT(MSG_PRINT_FILE));
          if (has_preview) {
            file_preview_image_address = image_address;
            DWIN_SRAM_Memory_Icon_Display(48,78,image_address);
          }
          else gcode.process_subcommands_now(F("M117 Preview not found")); 

          if (has_header_time || has_header_filament || has_header_layer) {
            sprintf_P(cmd, PSTR("%s - %s - %s..."), (has_header_time) ? header1 : GET_TEXT(MSG_HEADER_NO_TIME), (has_header_filament) ? header2 : GET_TEXT(MSG_HEADER_NO_FILAMENT_USED), (has_header_layer) ? header3 : GET_TEXT(MSG_HEADER_NO_LAYER_HEIGHT));
            Update_Status(cmd);
          }
          else gcode.process_subcommands_now(F("M117 Header not found")); 
        #else
          gcode.process_subcommands_now(F("M220 S100\nM221 S100"));  // Initialize Flow and Feerate to 100%
          strcpy(reprint_filename, card.filename);
          #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
              CrealityDWIN.Autotilt_AfterNPrint(NPrinted);
          #endif
          card.openAndPrintFile(card.filename);
        #endif
        }
      }
    }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Print_Screen_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW && selection < PRINT_COUNT - 1) {
      selection++; // Select Down
      Print_Screen_Icons();
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && selection > 0) {
      selection--; // Select Up
      Print_Screen_Icons();
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      switch (selection) {
        case PRINT_SETUP:
          Draw_Menu(Tune);
          Update_Status_Bar(true);
          break;
        case PRINT_PAUSE_RESUME:
          if (paused) {
            if (sdprint) {
              wait_for_user = false;
              #if ENABLED(PARK_HEAD_ON_PAUSE)
                card.startOrResumeFilePrinting();
                TERN_(POWER_LOSS_RECOVERY, recovery.prepare());
              #else
                #if HAS_HEATED_BED
                  cmd[sprintf_P(cmd, PSTR("M140 S%i"), pausebed)] = '\0';
                  gcode.process_subcommands_now(cmd);
                #endif
                #if HAS_EXTRUDERS
                  cmd[sprintf_P(cmd, PSTR("M109 S%i"), pausetemp)] = '\0';
                  gcode.process_subcommands_now(cmd);
                #endif
                TERN_(HAS_FAN, thermalManager.fan_speed[0] = pausefan);
                planner.synchronize();
                TERN_(SDSUPPORT, queue.inject(F("M24")));
              #endif
            }
            else {
              TERN_(HOST_ACTION_COMMANDS, hostui.resume());
            }
            Draw_Print_Screen();
          }
          else
            Popup_Handler(Pause);
          break;
        case PRINT_STOP: Popup_Handler(Stop); break;
      }
    }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Popup_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW && selection < 1) {
      selection++;
      Popup_Select();
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && selection > 0) {
      selection--;
      Popup_Select();
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      switch (popup) {
        case Pause:
          if (selection == 0) {
            if (sdprint) {
              #if ENABLED(POWER_LOSS_RECOVERY)
                if (recovery.enabled) recovery.save(true);
              #endif
              #if ENABLED(PARK_HEAD_ON_PAUSE)
                Popup_Handler(Home, true);
                #if ENABLED(SDSUPPORT)
                  if (IS_SD_PRINTING()) card.pauseSDPrint();
                #endif
                planner.synchronize();
                queue.inject(F("M125"));
                planner.synchronize();
              #else
                queue.inject(F("M25"));
                TERN_(HAS_HOTEND, pausetemp = thermalManager.temp_hotend[0].target);
                TERN_(HAS_HEATED_BED, pausebed = thermalManager.temp_bed.target);
                TERN_(HAS_FAN, pausefan = thermalManager.fan_speed[0]);
                thermalManager.cooldown();
              #endif
            }
            else {
              TERN_(HOST_ACTION_COMMANDS, hostui.pause());
            }
          }
          Draw_Print_Screen();
          break;
        case Stop:
          if (selection == 0) {
            if (sdprint) {
              ui.abort_print();
              thermalManager.cooldown();
            }
            else {
              TERN_(HOST_ACTION_COMMANDS, hostui.cancel());
            }
            TERN_(DEBUG_DWIN, SERIAL_ECHOLNPGM("DWIN_Print_Aborted"));
          }
          else
            Draw_Print_Screen();
          break;
        case Resume:
          if (selection == 0)
            queue.inject(F("M1000"));
          else {
            queue.inject(F("M1000 C"));
            Draw_Main_Menu();
          }
          break;

        #if HAS_HOTEND
          case ETemp:
            if (selection == 0) {
              thermalManager.setTargetHotend(EXTRUDE_MINTEMP, 0);
              thermalManager.set_fan_speed(0, MAX_FAN_SPEED);
              Draw_Menu(PreheatHotend);
            }
            else
              Redraw_Menu(true, true, false);
            break;
        #endif

        #if HAS_BED_PROBE
          case ManualProbing:
            if (selection == 0) {
              Popup_Handler(Level, true);
              char buf[80];
              //const float dif = probe.probe_at_point(current_position.x, current_position.y, PROBE_PT_STOW, 0, false) - corner_avg;
              zval = probe.probe_at_point(current_position.x, current_position.y, PROBE_PT_STOW, 0, false);
              if (isnan(zval))
                Update_Status(GET_TEXT(MSG_ZPROBE_UNREACHABLE));
              else {
                const float dif = zval - corner_avg;
                if (dif == 0) Update_Status(GET_TEXT(MSG_CORNER_ZEROED));
                else sprintf_P(buf, dif > 0 ? GET_TEXT(MSG_CORNER_HIGH) : GET_TEXT(MSG_CORNER_LOW), dtostrf(abs(dif), 1, 3, str_1));
                if (abs(dif) <= 0.005) AudioFeedback(true);
                Update_Status(buf);
                Popup_Handler(ManualProbing);
              }
            }
            else {
              Redraw_Menu(true, true, false);
              Update_Status("");
            }
            break;
        #endif

        #if ENABLED(ADVANCED_PAUSE_FEATURE)
          case ConfFilChange:
            if (selection == 0) {
              if (thermalManager.temp_hotend[0].target < thermalManager.extrude_min_temp)
                Popup_Handler(ETemp);
              else {
                flag_chg_fil = true;
                if (thermalManager.temp_hotend[0].celsius < thermalManager.temp_hotend[0].target - 2) {
                  Popup_Handler(Heating);
                  Update_Status(GET_TEXT(MSG_HEATING));
                  thermalManager.wait_for_hotend(0);
                }
                Popup_Handler(FilChange);
                Update_Status(GET_TEXT(MSG_FILAMENTCHANGE));
                sprintf_P(cmd, PSTR("M600 B1 R%i"), thermalManager.temp_hotend[0].target);
                gcode.process_subcommands_now(cmd);
              }
            }
            else
              Redraw_Menu(true, true, false);
            break;
          case PurgeMore:
            if (selection == 0) {
              pause_menu_response = PAUSE_RESPONSE_EXTRUDE_MORE;
              Popup_Handler(FilChange);
            }
            else {
              pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT;
              if (printing) Popup_Handler(Resuming);
              else {
                if (flag_chg_fil) Popup_Handler(FilChange, true);
                else Redraw_Menu(true, true, (active_menu==PreheatHotend));
              }
            }
            break;
        #endif // ADVANCED_PAUSE_FEATURE
        case ConfirmStartPrint:
          if (selection==0) {
            #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
              CrealityDWIN.Autotilt_AfterNPrint(NPrinted);
            #endif
            gcode.process_subcommands_now(F("M220 S100\nM221 S100"));  // Initialize Flow and Feerate to 100%
            strcpy(reprint_filename, card.filename);
            card.openAndPrintFile(card.filename);}
          else{
            Redraw_Menu(true, true, true);
            gcode.process_subcommands_now(F("M117"));}
          break;

        case Reprint:
          if (selection==0) {
            #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
              CrealityDWIN.Autotilt_AfterNPrint(NPrinted);
            #endif
            #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
              file_preview = true;
            #endif
            gcode.process_subcommands_now(F("M220 S100\nM221 S100"));  // Initialize Flow and Feerate to 100%
            card.openAndPrintFile(reprint_filename);
            }
          else { 
          TERN_(DEBUG_DWIN, SERIAL_ECHOLNPGM("DWIN_Print_Finished"));
          Draw_Main_Menu();
          }
          break;

        #if ENABLED(EEPROM_SETTINGS) && HAS_MESH 
          case SaveLevel:
            if (selection == 0) {
              #if ENABLED(AUTO_BED_LEVELING_UBL)
                gcode.process_subcommands_now(F("G29 S"));
                planner.synchronize();
                AudioFeedback(true);
              #else
                AudioFeedback(settings.save());
              #endif
            }
            if (flag_leveling_m) {
              flag_leveling_m = false;
              Draw_Menu(Leveling, LEVELING_MANUAL);
            }
            else Draw_Menu(Leveling, LEVELING_GET_MESH);
            break;
        #endif

        #if ENABLED(AUTO_BED_LEVELING_UBL)
          case MeshSlot:
            if (selection==0) Draw_Menu(Leveling, LEVELING_SLOT);
            else Redraw_Menu(true, true, false);
            break;
        #endif
        default: break;
      }
    }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Confirm_Control() {
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_ENTER) {
      switch (popup) {
        #if EXTJYERSUI && HAS_MESH
          case Level2:
          TERN(AUTO_BED_LEVELING_UBL, HMI_flags.cancel_ubl = 1, HMI_flags.cancel_abl = 1);
          Update_Status(GET_TEXT(MSG_PROBING_CANCELLED));
          wait_for_user = false;
          Redraw_Menu(true, true, false);
          queue.inject(F("M84"));
          break;
        #endif
        #if HAS_ES_DIAG
          case endsdiag:
            wait_for_user = false;
			      Redraw_Menu(true, true, false);
            break;  
        #endif
        case Complete:
          #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
            file_preview = false;
          #endif
          queue.inject(F("M84"));
          if (sdprint) { // mmm need to fix
            sdprint = false;
            Popup_Handler(Reprint);
          }
          else
            Draw_Main_Menu();
          break;
        case FilInsert:
          Popup_Handler(FilChange);
          wait_for_user = false;
          break;
        case HeaterTime:
          Popup_Handler(Heating);
          Update_Status(GET_TEXT(MSG_HEATING));
          wait_for_user = false;
          break;
		#if HAS_MESH
          case viewmesh:
            while (mesh_conf.drawing_mesh != false);
            if (planner.leveling_active) {
              mesh_conf.viewer_asymmetric_range = mesh_conf.last_viewer_asymmetric_range;
              mesh_conf.viewer_print_value = mesh_conf.last_viewer_print_value;
            }
            flag_viewmesh = false;
            DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, KEY_Y_START, DWIN_WIDTH-2, DWIN_HEIGHT-2);
            Draw_Status_Area(true);
            Update_Status_Bar(true);
            wait_for_user = false;
            Popup_Handler(SaveLevel);
            break;
		#endif
        default:
          wait_for_user = false;
          Redraw_Menu(true, true, false);
          break;
      }
    }
    DWIN_UpdateLCD();
  }

  void CrealityDWINClass::Keyboard_Control() {
    const uint8_t keyboard_size = 34;
    static uint8_t key_selection = 0, cursor = 0;
    static char string[31];
    static bool uppercase = false, locked = false;
    if (reset_keyboard) {
      if (strcmp(stringpointer, "-") == 0) stringpointer[0] = '\0';
      key_selection = 0, cursor = strlen(stringpointer);
      uppercase = false, locked = false;
      reset_keyboard = false;
      strcpy(string, stringpointer);
    }
    EncoderState encoder_diffState = Encoder_ReceiveAnalyze();
    if (encoder_diffState == ENCODER_DIFF_NO) return;
    if (encoder_diffState == ENCODER_DIFF_CW && key_selection < keyboard_size) {
      Draw_Keys(key_selection, false, uppercase, locked);
      key_selection++;
      Draw_Keys(key_selection, true, uppercase, locked);
    }
    else if (encoder_diffState == ENCODER_DIFF_CCW && key_selection > 0) {
      Draw_Keys(key_selection, false, uppercase, locked);
      key_selection--;
      Draw_Keys(key_selection, true, uppercase, locked);
    }
    else if (encoder_diffState == ENCODER_DIFF_ENTER) {
      if (key_selection < 28) {
        if (key_selection == 19) {
          if (!numeric_keyboard) {
            if (locked) {
              uppercase = false, locked = false;
              Draw_Keyboard(keyboard_restrict, false, key_selection, uppercase, locked);
            } else if (uppercase) {
              locked = true;
              Draw_Keyboard(keyboard_restrict, false, key_selection, uppercase, locked);
            }
            else {
              uppercase = true;
              Draw_Keyboard(keyboard_restrict, false, key_selection, uppercase, locked);
            }
          }
        }
        else if (key_selection == 27) {
          cursor--;
          string[cursor] = '\0';
        }
        else {
          uint8_t index = key_selection;
          if (index > 19) index--;
          if (index > 27) index--;
          const char *keys;
          if (numeric_keyboard) keys = "1234567890&<>(){}[]*\"\':;!?";
          else keys = (uppercase) ? "QWERTYUIOPASDFGHJKLZXCVBNM" : "qwertyuiopasdfghjklzxcvbnm";
          if (!(keyboard_restrict && numeric_keyboard && index > 9)) {
            string[cursor] = keys[index];
            cursor++;
            string[cursor] = '\0';
          }
          if (!locked && uppercase) {
            uppercase = false;
            Draw_Keyboard(keyboard_restrict, false, key_selection, uppercase, locked);
          }
        }
      }
      else {
        switch (key_selection) {
          case 28:
            if (!numeric_keyboard) uppercase = false, locked = false;
            Draw_Keyboard(keyboard_restrict, !numeric_keyboard, key_selection, uppercase, locked);
            break;
          case 29:
            string[cursor] = '-';
            cursor++;
            string[cursor] = '\0';
            break;
          case 30:
            string[cursor] = '_';
            cursor++;
            string[cursor] = '\0';
            break;
          case 31:
            if (!keyboard_restrict) {
              string[cursor] = ' ';
              cursor++;
              string[cursor] = '\0';
            }
            break;
          case 32:
            if (!keyboard_restrict) {
              string[cursor] = '.';
              cursor++;
              string[cursor] = '\0';
            }
            break;
          case 33:
            if (!keyboard_restrict) {
              string[cursor] = '/';
              cursor++;
              string[cursor] = '\0';
            }
            break;
          case 34:
            if (string[0] == '\0') strcpy(string, "-");
            strcpy(stringpointer, string);
            process = Menu;
            DWIN_Draw_Rectangle(1, GetColor(HMI_datas.background, Color_Bg_Black), 0, KEY_Y_START, DWIN_WIDTH-2, DWIN_HEIGHT-2);
            Draw_Status_Area(true);
            Update_Status_Bar(true);
            break;
        }
      }
      if (strlen(string) > maxstringlen) string[maxstringlen] = '\0', cursor = maxstringlen;
      Draw_String(string, selection, (process==Keyboard), (maxstringlen > 10));
    }
    DWIN_UpdateLCD();
  }


  /* In-Menu Value Modification */

  void CrealityDWINClass::Setup_Value(float value, float min, float max, float unit, uint8_t type) {
    if (TERN0(PIDTEMP, valuepointer == &thermalManager.temp_hotend[0].pid.Ki) || TERN0(PIDTEMPBED, valuepointer == &thermalManager.temp_bed.pid.Ki))
      tempvalue = unscalePID_i(value) * unit;
    else if (TERN0(PIDTEMP, valuepointer == &thermalManager.temp_hotend[0].pid.Kd) || TERN0(PIDTEMPBED, valuepointer == &thermalManager.temp_bed.pid.Kd))
      tempvalue = unscalePID_d(value) * unit;
    else
      tempvalue = value * unit;
    valuemin = min;
    valuemax = max;
    valueunit = unit;
    valuetype = type;
    process = Value;
    EncoderRate.enabled = true;
    Draw_Float(tempvalue / unit, selection - scrollpos, true, valueunit);
  }

  void CrealityDWINClass::Modify_Value(float &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 0);
  }
  void CrealityDWINClass::Modify_Value(uint8_t &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 1);
  }
  void CrealityDWINClass::Modify_Value(uint16_t &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 2);
  }
  void CrealityDWINClass::Modify_Value(int16_t &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 3);
  }
  void CrealityDWINClass::Modify_Value(uint32_t &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 4);
  }
  void CrealityDWINClass::Modify_Value(int8_t &value, float min, float max, float unit, void (*f)()/*=nullptr*/) {
    valuepointer = &value;
    funcpointer = f;
    Setup_Value((float)value, min, max, unit, 5);
  }

  void CrealityDWINClass::Modify_Option(uint8_t value, const char * const * options, uint8_t max) {
    tempvalue = value;
    valuepointer = const_cast<const char * *>(options);
    valuemin = 0;
    valuemax = max;
    process = Option;
    EncoderRate.enabled = true;
    Draw_Option(value, options, selection - scrollpos, true);
  }

  void CrealityDWINClass::Modify_String(char * string, uint8_t maxlength, bool restrict) {
    stringpointer = string;
    maxstringlen = maxlength;
    reset_keyboard = true;
    Draw_Keyboard(restrict, false);
    Draw_String(string, selection, true, (maxstringlen > 10));
  }

  /* Main Functions */

  void CrealityDWINClass::Update_Status(const char * const text) {
    //char header[4];
    //LOOP_L_N(i, 3) header[i] = text[i];
    //header[3] = '\0';
    if (strncmp_P(text, PSTR("<F>"), 3) == 0) {
      LOOP_L_N(i, _MIN((size_t)LONG_FILENAME_LENGTH, strlen(text))) filename[i] = text[i + 3];
      filename[_MIN((size_t)LONG_FILENAME_LENGTH - 1, strlen(text))] = '\0';
      Draw_Print_Filename(true);
    }
    else {
      LOOP_L_N(i, _MIN((size_t)128, strlen(text))) statusmsg[i] = text[i];
      statusmsg[_MIN((size_t)128, strlen(text))] = '\0';
    }
  }

  void CrealityDWINClass::Start_Print(bool sd) {
    sdprint = sd;
    if (!printing) {
      printing = true;
      statusmsg[0] = '\0';
      if (sd) {
        #if ENABLED(POWER_LOSS_RECOVERY)
          if (recovery.valid()) {
            SdFile *diveDir = nullptr;
            const char * const fname = card.diveToFile(true, diveDir, recovery.info.sd_filename);
            card.selectFileByName(fname);
            #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
              uint16_t image_address;
              bool has_preview = find_and_decode_gcode_preview(card.filename, Thumnail_Preview, &image_address);
              file_preview = has_preview;
              if (has_preview)   file_preview_image_address = image_address;
              else  gcode.process_subcommands_now(F("M117 Preview not found")); 
            #endif
          }
        #endif
        strcpy(filename, card.longest_filename());
      }
      else
        //strcpy_P(filename, "Host Print");
        strcpy(filename, Hostfilename);
      TERN_(LCD_SET_PROGRESS_MANUALLY, ui.set_progress(0));
      TERN_(USE_M73_REMAINING_TIME, ui.set_remaining_time(0));
      Draw_Print_Screen();
    }
  }

  void CrealityDWINClass::DWIN_Hostheader(const char *text = nullptr) {
  if (text) {
    const int8_t size = _MIN((unsigned) LONG_FILENAME_LENGTH, strlen_P(text));
    LOOP_L_N(i, size) Hostfilename[i] = text[i];
    Hostfilename[size] = '\0';
    }
  }

  void CrealityDWINClass::Stop_Print() {
    printing = false;
    // sdprint = false; mmm
    thermalManager.cooldown();
    duration_t printing_time = print_job_timer.duration();
    sprintf_P(cmd, PSTR("%s: %02dh %02dm %02ds"), GET_TEXT(MSG_INFO_PRINT_TIME), (uint8_t)(printing_time.value / 3600), (uint8_t)((printing_time.value / 60) %60), (uint8_t)(printing_time.value %60));
    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
      if (!file_preview) Update_Status(cmd);
    #else
      Update_Status(cmd);
    #endif
    TERN_(LCD_SET_PROGRESS_MANUALLY, ui.set_progress(100 * (PROGRESS_SCALE)));
    TERN_(USE_M73_REMAINING_TIME, ui.set_remaining_time(0));
    #if HAS_MESH
      if(HMI_datas.leveling_active) set_bed_leveling_enabled(HMI_datas.leveling_active);
    #endif
    Draw_Print_confirm();
  }

  void CrealityDWINClass::Update() {
    State_Update();
    Screen_Update();
    switch (process) {
      case Main:      sd_item_flag = false; Main_Menu_Control();    break;
      case Menu:      sd_item_flag = false; Menu_Control();         break;
      case Value:     sd_item_flag = false; Value_Control();        break;
      case Option:    sd_item_flag = false; Option_Control();       break;
      case File:      sd_item_flag = true;  File_Control();         break;
      case Print:     sd_item_flag = false; Print_Screen_Control(); break;
      case Popup:     sd_item_flag = false; Popup_Control();        break;
      case Confirm:   sd_item_flag = false; Confirm_Control();      break;
      case Keyboard:  sd_item_flag = false; Keyboard_Control();     break;
      case Locked:    sd_item_flag = false; HMI_ScreenLock();       break;
      #if HAS_SHORTCUTS
        case Short_cuts : sd_item_flag = false; HMI_Move_Z();       break;
      #endif
    }
  }

  void MarlinUI::update() { CrealityDWIN.Update(); }

  #if HAS_LCD_BRIGHTNESS
    void MarlinUI::_set_brightness() { DWIN_LCD_Brightness(backlight ? brightness : 0); }
  #endif

  void CrealityDWINClass::State_Update() {
    if ((print_job_timer.isRunning() || print_job_timer.isPaused()) != printing) {
      if (!printing) Start_Print(card.isFileOpen() || TERN0(POWER_LOSS_RECOVERY, recovery.valid()));
      else Stop_Print();
    }
    if (print_job_timer.isPaused() != paused) {
      paused = print_job_timer.isPaused();
      if (process == Print) Print_Screen_Icons();
      if (process == Wait && !paused) Redraw_Menu(true, true);
    }
    if (wait_for_user && !(process == Confirm) && !print_job_timer.isPaused())
      Confirm_Handler(UserInput);
    #if ENABLED(ADVANCED_PAUSE_FEATURE)
      if (process == Popup && popup == PurgeMore) {
        if (pause_menu_response == PAUSE_RESPONSE_EXTRUDE_MORE)
          Popup_Handler(FilChange);
        else if (pause_menu_response == PAUSE_RESPONSE_RESUME_PRINT) {
          if (printing) Popup_Handler(Resuming);
          else {
            if (flag_chg_fil) Popup_Handler(FilChange, true);
            else Redraw_Menu(true, true, (active_menu==PreheatHotend));
          }
        }
      }
    #endif
    #if HAS_FILAMENT_SENSOR
      static bool ranout = false;
      if (runout.filament_ran_out != ranout) {
        ranout = runout.filament_ran_out;
        if (ranout) Popup_Handler(Runout);
      }
    #endif
  }

  void CrealityDWINClass::Screen_Update() {
    //const millis_t ms = millis();
    static millis_t scrltime = 0;
    if (ELAPSED(millis(), scrltime)) {
      scrltime = millis() + 200;
      if (process != Keyboard) Update_Status_Bar();
      if (process == Print) Draw_Print_Filename();
    }

    static millis_t statustime = 0;
    if (ELAPSED(millis(), statustime) && process != Keyboard) {
      statustime = millis() + 500;
      if (!flag_viewmesh) Draw_Status_Area();
      if (process == Confirm && popup == endsdiag) EndSDiag.Update_ends_diag();
    }

    static millis_t printtime = 0;
    if (ELAPSED(millis(), printtime)) {
      printtime = millis() + 1000;
      if (process == Print) {
        Draw_Print_ProgressBar();
        Draw_Print_ProgressElapsed();
        TERN_(USE_M73_REMAINING_TIME, Draw_Print_ProgressRemain());
      }
    }

    static bool mounted = card.isMounted();
    if (mounted != card.isMounted()) {
      mounted = card.isMounted();
      #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
      image_cache.clear();
      #endif
      if (process == File) {
        sd_item_flag = true;
        Draw_SD_List();
      }
      else sd_item_flag = false;
    }

    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW) && DISABLED(DACAI_DISPLAY)
    if (HMI_datas.show_gcode_thumbnails && ELAPSED(millis(), thumbtime)) {
      uint16_t cColor = GetColor(HMI_datas.cursor_color, Rectangle_Color);
      thumbtime = millis() + 60000;
      if (process == File) {
        sd_item_flag = true;
        // Draw_SD_List(!mounted, selection, scrollpos);
        if (selection-scrollpos > MROWS) scrollpos = selection - MROWS;
        LOOP_L_N(i, _MIN(card.get_num_Files() + 1, TROWS)) {
          if (Encoder_ReceiveAnalyze() != ENCODER_DIFF_NO) break;
          if (i + scrollpos == 0) {
            if (card.flag.workDirIsRoot)
              Draw_Menu_Item(0, ICON_Back, GET_TEXT_F(MSG_BACK));
            else
              Draw_Menu_Item(0, ICON_Back, "..");
          } else {
            card.getfilename_sorted(SD_ORDER(i + scrollpos - 1, card.get_num_Files()));

            Draw_Menu_Item(i, card.flag.filenameIsDir ? ICON_More : ICON_File);
          }
          DWIN_UpdateLCD();
        }
        if ((cColor == GetColor(HMI_datas.background, Color_Bg_Black)) || ((cColor == Color_Black) && (HMI_datas.background = 0)))
          DWIN_Draw_Rectangle(0, GetColor(HMI_datas.items_menu_text, Color_White), 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
        else
          DWIN_Draw_Rectangle(1, cColor, 0, MBASE(selection - scrollpos) - 18, 8, MBASE(selection - scrollpos) + 31);
      }
      else
        sd_item_flag = false;
    }
    #endif

    #if HAS_HOTEND
      static int16_t hotendtarget = -1;
    #endif
    #if HAS_HEATED_BED
      static int16_t bedtarget = -1;
    #endif
    #if HAS_FAN
      static int16_t fanspeed = -1;
    #endif

    #if HAS_ZOFFSET_ITEM
      static float lastzoffset = zoffsetvalue;
      if (zoffsetvalue != lastzoffset && !printing) {
        lastzoffset = zoffsetvalue;
        #if HAS_BED_PROBE
          probe.offset.z = zoffsetvalue;
        #else
          set_home_offset(Z_AXIS, -zoffsetvalue);
        #endif
      }

      #if HAS_BED_PROBE
        if (probe.offset.z != lastzoffset)
          zoffsetvalue = lastzoffset = probe.offset.z;
      #else
        if (-home_offset.z != lastzoffset)
          zoffsetvalue = lastzoffset = -home_offset.z;
      #endif
    #endif // HAS_ZOFFSET_ITEM

    if (process == Menu || process == Value) {
      switch (active_menu) {
        case TempMenu:
          #if HAS_HOTEND
            if (thermalManager.temp_hotend[0].target != hotendtarget) {
              hotendtarget = thermalManager.temp_hotend[0].target;
              if (scrollpos <= TEMP_HOTEND && TEMP_HOTEND <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.temp_hotend[0].target, TEMP_HOTEND - scrollpos, false, 1);
              }
            }
          #endif
          #if HAS_HEATED_BED
            if (thermalManager.temp_bed.target != bedtarget) {
              bedtarget = thermalManager.temp_bed.target;
              if (scrollpos <= TEMP_BED && TEMP_BED <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.temp_bed.target, TEMP_BED - scrollpos, false, 1);
              }
            }
          #endif
          #if HAS_FAN
            if (thermalManager.fan_speed[0] != fanspeed) {
              fanspeed = thermalManager.fan_speed[0];
              if (scrollpos <= TEMP_FAN && TEMP_FAN <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.fan_speed[0], TEMP_FAN - scrollpos, false, 1);
              }
            }
          #endif
          break;
        case Tune:
          #if HAS_HOTEND
            if (thermalManager.temp_hotend[0].target != hotendtarget) {
              hotendtarget = thermalManager.temp_hotend[0].target;
              if (scrollpos <= TUNE_HOTEND && TUNE_HOTEND <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.temp_hotend[0].target, TUNE_HOTEND - scrollpos, false, 1);
              }
            }
          #endif
          #if HAS_HEATED_BED
            if (thermalManager.temp_bed.target != bedtarget) {
              bedtarget = thermalManager.temp_bed.target;
              if (scrollpos <= TUNE_BED && TUNE_BED <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.temp_bed.target, TUNE_BED - scrollpos, false, 1);
              }
            }
          #endif
          #if HAS_FAN
            if (thermalManager.fan_speed[0] != fanspeed) {
              fanspeed = thermalManager.fan_speed[0];
              if (scrollpos <= TUNE_FAN && TUNE_FAN <= scrollpos + MROWS) {
                if (process != Value || selection != TEMP_HOTEND - scrollpos)
                  Draw_Float(thermalManager.fan_speed[0], TUNE_FAN - scrollpos, false, 1);
              }
            }
          #endif
          break;
      }
    }
  }

  void CrealityDWINClass::AudioFeedback(const bool success/*=true*/) {
    if (ui.buzzer_enabled)
    DONE_BUZZ(success);
    else
      Update_Status(success ? GET_TEXT(MSG_SUCCESS) : GET_TEXT(MSG_FAILED));
  }



  #if HAS_LEVELING_HEAT
      void CrealityDWINClass::HeatBeforeLeveling() {
        Popup_Handler(Heating);
        Update_Status(GET_TEXT(MSG_HEATING));
        #if HAS_HOTEND 
          if ((thermalManager.degTargetHotend(0) < HMI_datas.LevelingTemp_hotend) && (HMI_datas.ena_LevelingTemp_hotend))
                thermalManager.setTargetHotend(HMI_datas.LevelingTemp_hotend, 0);
        #endif
        #if HAS_HEATED_BED
          if ((thermalManager.degTargetBed() < HMI_datas.LevelingTemp_bed) && (HMI_datas.ena_LevelingTemp_bed))
                thermalManager.setTargetBed(HMI_datas.LevelingTemp_bed);
        #endif
        if (HMI_datas.ena_LevelingTemp_hotend) TERN_(HAS_HOTEND, thermalManager.wait_for_hotend(0));
        if (HMI_datas.ena_LevelingTemp_bed) TERN_(HAS_HEATED_BED, thermalManager.wait_for_bed_heating());
      }
  #endif

  #if HAS_PID_HEATING
      void CrealityDWINClass::PidTuning(const pidresult_t pidresult) {
        switch (pidresult) {
          case PID_STARTED:  break;
          case PID_BAD_EXTRUDER_NUM:  Confirm_Handler(BadextruderNumber);  break;
          case PID_TEMP_TOO_HIGH:  Confirm_Handler(TemptooHigh);  break;
          case PID_TUNING_TIMEOUT:  Confirm_Handler(PIDTimeout);  break;
          case PID_DONE:  Confirm_Handler(PIDDone);  break;
          default: break;
        }
      }
    #endif


  void CrealityDWINClass::Save_Settings(char *buff) {
    TERN_(AUTO_BED_LEVELING_UBL, HMI_datas.tilt_grid_size = mesh_conf.tilt_grid - 1);
    #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
      HMI_datas.N_Printed = NPrinted;
    #endif
    HMI_datas.corner_pos = corner_pos * 10;
    HMI_datas.shortcut_0 = shortcut0;
    HMI_datas.shortcut_1 = shortcut1;
    #if ENABLED(HOST_ACTION_COMMANDS)
      HMI_datas.host_action_label_1 = Encode_String(action1);
      HMI_datas.host_action_label_2 = Encode_String(action2);
      HMI_datas.host_action_label_3 = Encode_String(action3);
    #endif
    #if ENABLED(DWIN_ICON_SET)
      HMI_datas.iconset_index = iconset_current;
    #endif

    #if HAS_MESH
      HMI_datas.leveling_active = planner.leveling_active;
    #endif
    memcpy(buff, &HMI_datas, _MIN(sizeof(HMI_datas), eeprom_data_size));
  }

  void CrealityDWINClass::Load_Settings(const char *buff) {
    memcpy(&HMI_datas, buff, _MIN(sizeof(HMI_datas), eeprom_data_size));
    #if HAS_MESH
      if(HMI_datas.leveling_active) set_bed_leveling_enabled(HMI_datas.leveling_active);
    #endif
    TERN_(AUTO_BED_LEVELING_UBL, mesh_conf.tilt_grid = HMI_datas.tilt_grid_size + 1);
    #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
      NPrinted = HMI_datas.N_Printed;
    #endif
    if (HMI_datas.corner_pos == 0) HMI_datas.corner_pos = 325;
    corner_pos = HMI_datas.corner_pos / 10.0f;
    #if HAS_FILAMENT_SENSOR
      Get_Rsensormode(runout.mode[0]);
    #endif
    #if ENABLED(DWIN_ICON_SET)
      iconset_current = HMI_datas.iconset_index;
    #endif
    shortcut0 = HMI_datas.shortcut_0;
    shortcut1 = HMI_datas.shortcut_1;
    #if ENABLED(HOST_ACTION_COMMANDS)
      Decode_String(HMI_datas.host_action_label_1, action1);
      Decode_String(HMI_datas.host_action_label_2, action2);
      Decode_String(HMI_datas.host_action_label_3, action3);
    #endif
    Redraw_Screen();
    #if ENABLED(POWER_LOSS_RECOVERY)
      static bool init = true;
      if (init) {
        init = false;
        queue.inject(F("M1000 S"));
      }
    #endif
  }

  void CrealityDWINClass::Reset_Settings() {
    HMI_datas.time_format_textual = false;
    HMI_datas.fan_percent = false;
    TERN_(AUTO_BED_LEVELING_UBL, HMI_datas.tilt_grid_size = 0);
    HMI_datas.corner_pos = 325;
    HMI_datas.cursor_color = TERN(Ext_Config_JyersUI, Def_cursor_color, 0);
    HMI_datas.menu_split_line = TERN(Ext_Config_JyersUI, Def_menu_split_line, 0);
    HMI_datas.items_menu_text = TERN(Ext_Config_JyersUI, Def_items_menu_text, 0);
    HMI_datas.icons_menu_text = TERN(Ext_Config_JyersUI, Def_icons_menu_text, 0);
    HMI_datas.background = TERN(Ext_Config_JyersUI, Def_background, 0);
    HMI_datas.menu_top_bg = TERN(Ext_Config_JyersUI, Def_menu_top_bg, 0);
    HMI_datas.menu_top_txt = TERN(Ext_Config_JyersUI, Def_menu_top_txt, 0);
    HMI_datas.select_txt = TERN(Ext_Config_JyersUI, Def_select_txt, 0);
    HMI_datas.select_bg = TERN(Ext_Config_JyersUI, Def_select_bg, 0);
    HMI_datas.highlight_box = TERN(Ext_Config_JyersUI, Def_highlight_box, 0);
    HMI_datas.popup_highlight = TERN(Ext_Config_JyersUI, Def_popup_highlight, 0);
    HMI_datas.popup_txt = TERN(Ext_Config_JyersUI, Def_popup_txt, 0);
    HMI_datas.popup_bg = TERN(Ext_Config_JyersUI, Def_popup_bg, 0);
    HMI_datas.ico_confirm_txt = TERN(Ext_Config_JyersUI, Def_ico_confirm_txt, 0);
    HMI_datas.ico_confirm_bg = TERN(Ext_Config_JyersUI, Def_ico_confirm_bg, 0);
    HMI_datas.ico_cancel_txt = TERN(Ext_Config_JyersUI, Def_ico_cancel_txt, 0);
    HMI_datas.ico_cancel_txt = TERN(Ext_Config_JyersUI, Def_ico_cancel_bg, 0);
    HMI_datas.ico_continue_txt = TERN(Ext_Config_JyersUI, Def_ico_continue_txt, 0);
    HMI_datas.ico_continue_bg = TERN(Ext_Config_JyersUI, Def_ico_continue_bg, 0);
    HMI_datas.print_screen_txt = TERN(Ext_Config_JyersUI, Def_print_screen_txt, 0);
    HMI_datas.print_filename = TERN(Ext_Config_JyersUI, Def_print_filename, 0);
    HMI_datas.progress_bar = TERN(Ext_Config_JyersUI, Def_progress_bar, 0);
    HMI_datas.progress_percent = TERN(Ext_Config_JyersUI, Def_progress_percent, 0);
    HMI_datas.remain_time = TERN(Ext_Config_JyersUI, Def_remain_time, 0);
    HMI_datas.elapsed_time = TERN(Ext_Config_JyersUI, Def_elapsed_time, 0);
    HMI_datas.status_bar_text = TERN(Ext_Config_JyersUI, Def_status_bar_text, 0);
    HMI_datas.status_area_text = TERN(Ext_Config_JyersUI, Def_status_area_text, 0);
    HMI_datas.status_area_percent = TERN(Ext_Config_JyersUI, Def_status_area_percent, 0);
    HMI_datas.coordinates_text = TERN(Ext_Config_JyersUI, Def_coordinates_text, 0);
    HMI_datas.coordinates_split_line = TERN(Ext_Config_JyersUI, Def_coordinates_split_line, 0);
    #if ENABLED(HOST_ACTION_COMMANDS)
      HMI_datas.host_action_label_1 = 0;
      HMI_datas.host_action_label_2 = 0;
      HMI_datas.host_action_label_3 = 0;
      action1[0] = action2[0] = action3[0] = '-';
    #endif
    TERN_(AUTO_BED_LEVELING_UBL, mesh_conf.tilt_grid = HMI_datas.tilt_grid_size + 1);
    corner_pos = HMI_datas.corner_pos / 10.0f;
    TERN_(SOUND_MENU_ITEM, ui.buzzer_enabled = true);
    
    #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
      NPrinted = 0;
    #endif

    #if HAS_FILAMENT_SENSOR
      Get_Rsensormode(runout.mode[0]);
    #endif
    #if ENABLED(DWIN_ICON_SET)
      HMI_datas.iconset_index = iconset_current = DWIN_ICON_DEF;
    #endif
    shortcut0 = HMI_datas.shortcut_0;
    shortcut1 = HMI_datas.shortcut_1;

    #if ENABLED(BAUD_RATE_GCODE)
      if (BAUDRATE == 250000) HMI_datas.baudratemode = 0;
      else HMI_datas.baudratemode = 1;
      sprintf_P(cmd, PSTR("M575 P%i B%i"), BAUD_PORT, HMI_datas.baudratemode ? 115 : 250);
      gcode.process_subcommands_now(cmd);
    #endif

    #if HAS_LEVELING_HEAT
      HMI_datas.ena_LevelingTemp_hotend = true;
      HMI_datas.ena_LevelingTemp_bed = true;
      HMI_datas.LevelingTemp_hotend = LEVELING_NOZZLE_TEMP;
      HMI_datas.LevelingTemp_bed = LEVELING_BED_TEMP;
    #endif

    #if EXTJYERSUI   
      HMI_datas.invert_dir_extruder = INVERT_E0_DIR;
      DWIN_Invert_Extruder();
      #if ENABLED(NOZZLE_PARK_FEATURE)
        HMI_datas.Park_point = xyz_int_t DEF_NOZZLE_PARK_POINT;
      #endif
      #if HAS_BED_PROBE
        HMI_datas.probing_margin = DEF_PROBING_MARGIN;
        HMI_datas.zprobefeedfast = DEF_Z_PROBE_FEEDRATE_FAST;
        HMI_datas.zprobefeedslow = DEF_Z_PROBE_FEEDRATE_SLOW;
      #endif
      #if ENABLED(ADVANCED_PAUSE_FEATURE)
        HMI_datas.fil_unload_feedrate = DEF_FILAMENT_CHANGE_UNLOAD_FEEDRATE;
        HMI_datas.fil_fast_load_feedrate = DEF_FILAMENT_CHANGE_FAST_LOAD_FEEDRATE;
      #endif

    #endif

    TERN_(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW, HMI_datas.show_gcode_thumbnails = true);
    #if HAS_MESH
      HMI_datas.leveling_active = planner.leveling_active;
    #endif

    Redraw_Screen();
  }

  void CrealityDWINClass::DWIN_Invert_Extruder() {
    stepper.disable_e_steppers();
    current_position.e = 0;
    sync_plan_position_e();
  }

  #if HAS_FILAMENT_SENSOR
    // Filament Runout process
    void CrealityDWINClass::DWIN_Filament_Runout(const uint8_t extruder) { LCD_MESSAGE(MSG_RUNOUT_SENSOR); }

    void CrealityDWINClass::Get_Rsensormode(RunoutMode Rsmode) {
      switch (Rsmode) {
           case RM_NONE: rsensormode = 0; break; // None 
           case RM_OUT_ON_HIGH: rsensormode = 1; break; // mode HIGH
           case RM_OUT_ON_LOW: rsensormode = 2; break; // mode LOW
           case RM_RESERVED3: break;
           case RM_RESERVED4: break;
           case RM_RESERVED5: break;
           case RM_RESERVED6: break;
           case RM_MOTION_SENSOR: rsensormode = 3; break; // mode MOTION
          }
    }
  #endif

  void CrealityDWINClass::RebootPrinter() {                   
    wait_for_heatup = wait_for_user = false;    // Stop waiting for heating/user
    thermalManager.disable_all_heaters();
    planner.finish_and_disable();
    DWIN_RebootScreen();
    hal.reboot();
    #if ENABLED(BAUD_RATE_GCODE)
      sprintf_P(cmd, PSTR("M575 P%i B%i"), BAUD_PORT, HMI_datas.baudratemode ? 115 : 250);
      gcode.process_subcommands_now(cmd);
    #endif
  }

  void CrealityDWINClass::DWIN_RebootScreen() {
    DWIN_Frame_Clear(Color_Bg_Black);
    DWIN_JPG_ShowAndCache(0);
    JYERSUI::Draw_CenteredString(Color_White, 210, GET_TEXT_F(MSG_PLEASE_WAIT_REBOOT));
    for (uint16_t t = 0; t <= 100; t += 2) {
      #ifdef BOOTPERSO
        DRAW_IconWB(ICON, ICON_Bar, 15, 170);
        DWIN_Draw_Rectangle(1, Color_Bg_Black, 15 + t * 242 / 100, 170, 257, 190);
      #else
        DRAW_IconWB(ICON, ICON_Bar, 15, 260);
        DWIN_Draw_Rectangle(1, Color_Bg_Black, 15 + t * 242 / 100, 260, 257, 280);
      #endif
    }
    DWIN_UpdateLCD();
    delay(500);
  }

  #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
    void CrealityDWINClass::Autotilt_AfterNPrint(uint16_t NPrints) {
      printStatistics prdatas = print_job_timer.getStats();
      uint16_t _NPrints = prdatas.totalPrints;
      if ((NPrints > 0) && ( _NPrints >= NPrints)) {
          if (ubl.storage_slot < 0) { Popup_Handler(MeshSlot); return; }
          else {
            #if EITHER(PREHEAT_BEFORE_LEVELING, PREHEAT_BEFORE_LEVELING_PROBE_MANUALLY)
              HeatBeforeLeveling();
            #endif
            Update_Status("");
            Popup_Handler(Home);
            gcode.home_all_axes(true);
            Popup_Handler(Level);
            if (mesh_conf.tilt_grid > 1) {
                sprintf_P(cmd, PSTR("G29 J%i"), mesh_conf.tilt_grid);
                gcode.process_subcommands_now(cmd);
            }
            else  gcode.process_subcommands_now(F("G29 J"));
            planner.synchronize();
            #if ENABLED(EEPROM_SETTINGS)
              gcode.process_subcommands_now(F("G29 S"));
              AudioFeedback();  
              planner.synchronize();
            #endif
          }
        }  
      }
  #endif

  void CrealityDWINClass::DWIN_Init_diag_endstops() {
    last_process = process;
    last_selection = selection;
    process = Confirm;
    popup = endsdiag;
  }

  void CrealityDWINClass::CPU_type() {
    std::string cputype = CPU_TYPE;
    std::string search = "RE";
    std::string find;
    for (uint8_t f = 0; f <=2; f += 1){
      find = cputype.substr(f+7,2);
      if (find == search) { strcpy(STM_cpu, "RET6"); break; }
      else strcpy(STM_cpu, "RCT6");
    }
  }

  //=============================================================================
  // Extended G-CODES Cn
  //=============================================================================

  void CrealityDWINClass::DWIN_CError() {
    SERIAL_ECHO_START();
    Update_Status("This G-Code or parameter is not implemented in firmware");
    SERIAL_ECHOLNPGM("This G-Code or parameter is not implemented in firmware");
  }

  // Cancel a Wait for User without an Emergecy Parser
  void CrealityDWINClass::DWIN_C108() { 
    #if DEBUG_DWIN
      SERIAL_ECHOLNPGM(F("Wait for user was "), wait_for_user);
      SERIAL_ECHOLNPGM(F("Process was "), process);
    #endif
    wait_for_user = false;
    AudioFeedback();
  }

  // lock/unlock screen
  void CrealityDWINClass::DWIN_C510() {
    if (!parser.seen_any()) return CrealityDWIN.DWIN_CError();
    if (parser.seenval('U') && parser.value_int()) DWIN_ScreenUnLock();
    else DWIN_ScreenLock();
  }

  //#if DEBUG_DWIN
    void CrealityDWINClass::DWIN_C997() {
      #if ENABLED(POWER_LOSS_RECOVERY)
        if (printing && recovery.enabled) {
          planner.synchronize();
          recovery.save(true);
        }
      #endif
      DWIN_RebootScreen();
      Update_Status("Simulating a printer freeze");
      SERIAL_ECHOLNPGM("Simulating a printer freeze");
      while (1) {};
    }
  //#endif

  // Special Creality DWIN GCodes
  void CrealityDWINClass::DWIN_Gcode(const int16_t codenum) {
    switch(codenum) {
      case 108: DWIN_C108(); break;           // Cancel a Wait for User without an Emergecy Parser
      case 510: DWIN_C510(); break;           // lock screen
      #if DEBUG_DWIN
        case 997: DWIN_C997(); break;         // Simulate a printer freeze
      #endif
      #if EXTJYERSUI
        #if ENABLED(NOZZLE_PARK_FEATURE)
          case 125: ExtJyersui.C125(); break;      // Set park position
        #endif
        case 562: ExtJyersui.C562(); break;        // Invert Extruder
        #if HAS_BED_PROBE
          case 851: ExtJyersui.C851(); break;      // Set probing margin and z feed rate of the probe mesh leveling
        #endif
      #endif
      default: DWIN_CError(); break;
    }
  }

  //=============================================================================

  void MarlinUI::init_lcd() {
    delay(800);
    SERIAL_ECHOPGM("\nDWIN handshake ");
    if (DWIN_Handshake()) SERIAL_ECHOLNPGM("ok."); else SERIAL_ECHOLNPGM("error.");
    DWIN_Frame_SetDir(1); // Orientation 90°
    DWIN_UpdateLCD();     // Show bootscreen (first image)
    Encoder_Configuration();
    for (uint16_t t = 0; t <= 100; t += 2) {
      #ifdef BOOTPERSO
        DRAW_IconWB(ICON, ICON_Bar, 15, 170);
        DWIN_Draw_Rectangle(1, Color_Bg_Black, 15 + t * 242 / 100, 170, 257, 190);
      #else
        DRAW_IconWB(ICON, ICON_Bar, 15, 260);
        DWIN_Draw_Rectangle(1, Color_Bg_Black, 15 + t * 242 / 100, 260, 257, 280);
      #endif
      DWIN_UpdateLCD();
      delay(20);
    }
    CrealityDWIN.CPU_type();
    JYERSUI::cursor.x = 0;
    JYERSUI::cursor.y = 0;
    JYERSUI::pencolor = Color_White;
    JYERSUI::textcolor = Color_White;
    JYERSUI::backcolor = Color_Bg_Black;
    JYERSUI::buttoncolor = RGB( 0, 23, 16);
    JYERSUI::font = font8x16;
    DWIN_JPG_CacheTo1(Language_English);
    TERN(SHOW_BOOTSCREEN,,DWIN_Frame_Clear(Color_Bg_Black));
    DWIN_JPG_ShowAndCache(3);
    DWIN_UpdateLCD();
    CrealityDWIN.Redraw_Screen();
  }


  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    void MarlinUI::pause_show_message(const PauseMessage message, const PauseMode mode/*=PAUSE_MODE_SAME*/, const uint8_t extruder/*=active_extruder*/) {
      switch (message) {
        case PAUSE_MESSAGE_INSERT:  CrealityDWIN.Confirm_Handler(FilInsert);  break;
        case PAUSE_MESSAGE_PURGE: break;
        case PAUSE_MESSAGE_OPTION: 
          pause_menu_response = PAUSE_RESPONSE_WAIT_FOR;
          CrealityDWIN.Popup_Handler(PurgeMore);
          break;
        case PAUSE_MESSAGE_HEAT:    CrealityDWIN.Confirm_Handler(HeaterTime); break;
        case PAUSE_MESSAGE_WAITING: CrealityDWIN.Draw_Print_Screen(); break;
        default: break;
      }
    }
  #endif

#endif // DWIN_CREALITY_LCD_JYERSUI
