/**
 * DWIN general defines and data structs
 * Author: LChristophe68 (tititopher68-dev)
 * Version: 1.0
 * Date: 2022/02/03
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define EXTJYERSUI 1
//#define DEBUG_DWIN 1

#include "../../../core/types.h"
#include "../common/dwin_color.h"


#if EXTJYERSUI
  #include "extjyersui.h"
#endif

#ifndef LCD_SET_PROGRESS_MANUALLY
  #define LCD_SET_PROGRESS_MANUALLY
#endif
#ifndef STATUS_MESSAGE_SCROLLING
  #define STATUS_MESSAGE_SCROLLING
#endif
#ifndef BAUD_RATE_GCODE
  #define BAUD_RATE_GCODE
#endif
#ifndef HAS_LCD_BRIGHTNESS
  #define HAS_LCD_BRIGHTNESS 1
#endif
#ifndef LCD_BRIGHTNESS_DEFAULT
  #define LCD_BRIGHTNESS_DEFAULT 127
#endif
#ifndef SOUND_MENU_ITEM
  #define SOUND_MENU_ITEM
#endif

#if EITHER(PREHEAT_BEFORE_LEVELING, PREHEAT_BEFORE_LEVELING_PROBE_MANUALLY) && HAS_MESH
    #define HAS_LEVELING_HEAT 1
#endif

#define HAS_ES_DIAG 1

#define HAS_SHORTCUTS 1

#ifndef DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW
  #define DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW
#endif

#ifdef BABYSTEP_ZPROBE_OFFSET
  #undef BABYSTEP_ZPROBE_OFFSET
#endif

#if NONE(AQUILA_DISPLAY, DACAI_DISPLAY)
  #define DWIN_DISPLAY
#endif

#if DISABLED(REVERSE_ENCODER_DIRECTION) && ENABLED(AQUILA_DISPLAY)
  #define REVERSE_ENCODER_DIRECTION
#endif

#if (MB(CREALITY_V4) || MB(CREALITY_V422))
  #if ANY(DWIN_DISPLAY, DACAI_DISPLAY)
    #define PRINTERNAME "Ender-3 V2"
  #else
    #define PRINTERNAME "Aquila"
  #endif
#elif MB(CREALITY_V427)
  #if ANY(DWIN_DISPLAY, DACAI_DISPLAY)
    #define PRINTERNAME "Ender-3 Series"
  #else
    #define PRINTERNAME "Aquila"
  #endif
#elif MB(CREALITY_V423)
  #define PRINTERNAME "Ender-2 Pro"
#elif (MB(CREALITY_V24S1_301) || MB(CREALITY_V24S1_F401RC))
  #define PRINTERNAME "Ender-3 S1"
#else
  #define PRINTERNAME "Ender-3 Series"
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
  #if ENABLED(BLTOUCH)
    #define MODEFW " BLTouch"
  #elif ENABLED(FIX_MOUNTED_PROBE)
    #define MODEFW " ABL Probe"
  #elif ENABLED(TOUCH_MI_PROBE)
    #define MODEFW " TouchMI"
  #elif ENABLED(PROBE_MANUALLY)
    #define MODEFW " ManualMesh"
  #endif
#elif ENABLED(AUTO_BED_LEVELING_UBL)
  #if NONE(PROBE_MANUALLY, BLTOUCH, FIX_MOUNTED_PROBE, TOUCH_MI_PROBE)
    #define MODEFW " UBL-Noprobe"
  #elif ENABLED(BLTOUCH)
    #define MODEFW " UBL-BLTouch"
  #elif ENABLED(FIX_MOUNTED_PROBE)
    #define MODEFW " UBL-ABL"
  #elif ENABLED(TOUCH_MI_PROBE)
    #define MODEFW " UBL-TouchMI"
  #endif
#else
  #define MODEFW ""
#endif

#if ANY(AUTO_BED_LEVELING_BILINEAR, AUTO_BED_LEVELING_UBL)
  #define GRIDFW " " STRINGIFY(GRID_MAX_POINTS_X) "x" STRINGIFY(GRID_MAX_POINTS_Y)
#else
  #define GRIDFW ""
#endif


#ifndef CUSTOM_MACHINE_NAME
  #define CUSTOM_MACHINE_NAME PRINTERNAME MODEFW GRIDFW
#endif

//#define BOOTPERSO

typedef struct { 

  #if EXTJYERSUI && HAS_LEVELING
    #if ENABLED(AUTO_BED_LEVELING_UBL)
      bool cancel_ubl : 1; // cancel current ubl
    #else
      bool cancel_abl : 1;   // cancel current abl
    #endif
  #endif

} HMI_flags_t;

extern HMI_flags_t HMI_flags;

  
typedef struct { 
    bool time_format_textual = false;
    #if ENABLED(AUTO_BED_LEVELING_UBL)
      uint8_t tilt_grid_size : 3;
    #endif
    uint16_t corner_pos : 10;
    uint8_t cursor_color  : 4;
    uint8_t menu_split_line  : 4;
    uint8_t items_menu_text  : 4;
    uint8_t icons_menu_text  : 4;
    uint8_t background  : 4;
    uint8_t menu_top_bg  : 4;
    uint8_t menu_top_txt  : 4;
    uint8_t select_txt  : 4;
    uint8_t select_bg  : 4;
    uint8_t highlight_box  : 4;
    uint8_t popup_highlight  : 4;
    uint8_t popup_txt  : 4;
    uint8_t popup_bg  : 4;
    uint8_t ico_confirm_txt  : 4;
    uint8_t ico_confirm_bg  : 4;
    uint8_t ico_cancel_txt  : 4;
    uint8_t ico_cancel_bg  : 4;
    uint8_t ico_continue_txt  : 4;
    uint8_t ico_continue_bg  : 4;
    uint8_t print_screen_txt  :4;
    uint8_t print_filename  :4;
    uint8_t progress_bar  :4;
    uint8_t progress_percent  : 4;
    uint8_t remain_time  : 4;
    uint8_t elapsed_time  : 4;
    uint8_t status_bar_text  : 4;
    uint8_t status_area_text  : 4;
    uint8_t status_area_percent  : 4;
    uint8_t coordinates_text  : 4;
    uint8_t coordinates_split_line  : 4;
    #if ENABLED(DWIN_ICON_SET)
    uint8_t iconset_index : 4;
    #endif
    #if ENABLED(HOST_ACTION_COMMANDS)
      uint64_t host_action_label_1 : 48;
      uint64_t host_action_label_2 : 48;
      uint64_t host_action_label_3 : 48;
    #endif
    
    #if BOTH(HAS_BED_PROBE, AUTO_BED_LEVELING_UBL)
      uint16_t N_Printed : 8;
    #endif

    uint8_t shortcut_0 = 0;
    uint8_t shortcut_1 = 1;

    #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW)
      bool show_gcode_thumbnails : 1;
    #endif
    
    bool invert_dir_extruder = DEF_INVERT_E0_DIR;
    bool fan_percent : 1;
    #if ANY(AUTO_BED_LEVELING_BILINEAR, AUTO_BED_LEVELING_UBL, MESH_BED_LEVELING)
      bool leveling_active : 1;
    #endif
    #if ENABLED(BAUD_RATE_GCODE)
      bool baudratemode : 1;
    #endif

    #if HAS_LEVELING_HEAT
      bool ena_LevelingTemp_hotend : 1;
      bool ena_LevelingTemp_bed : 1;
      celsius_t LevelingTemp_hotend = LEVELING_NOZZLE_TEMP;
      celsius_t LevelingTemp_bed = LEVELING_BED_TEMP;
    #endif

    #if ALL(SDSUPPORT, SDCARD_SORT_ALPHA, SDSORT_GCODE)
      bool sdsort_alpha : 1;
    #endif
    #if EXTJYERSUI
      #if ENABLED(NOZZLE_PARK_FEATURE)
          xyz_int_t Park_point = DEF_NOZZLE_PARK_POINT;
      #endif
      #if HAS_BED_PROBE
          float probing_margin = DEF_PROBING_MARGIN;
          uint16_t zprobefeedfast = DEF_Z_PROBE_FEEDRATE_FAST;
          uint16_t zprobefeedslow = DEF_Z_PROBE_FEEDRATE_SLOW;
      #endif
      #if ENABLED(ADVANCED_PAUSE_FEATURE)
          uint8_t fil_unload_feedrate = DEF_FILAMENT_CHANGE_UNLOAD_FEEDRATE;
          uint8_t fil_fast_load_feedrate = DEF_FILAMENT_CHANGE_FAST_LOAD_FEEDRATE;
      #endif
    #endif
    

  } HMI_datas_t;

  static constexpr size_t eeprom_data_size = 112;
  extern HMI_datas_t HMI_datas;

//
// Undef :
//
#if EXTJYERSUI

  #if ENABLED(NOZZLE_PARK_FEATURE)
    #undef NOZZLE_PARK_POINT
  #endif

  #if HAS_BED_PROBE
    #undef PROBING_MARGIN
    #undef Z_PROBE_FEEDRATE_FAST
    #undef Z_PROBE_FEEDRATE_SLOW
  #endif

  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    #undef FILAMENT_CHANGE_UNLOAD_FEEDRATE
    #undef FILAMENT_CHANGE_FAST_LOAD_FEEDRATE
  #endif

  #undef INVERT_E0_DIR
  //
  // New Defines :
  //
  #if HAS_BED_PROBE
    #define PROBING_MARGIN HMI_datas.probing_margin
    #define Z_PROBE_FEEDRATE_FAST HMI_datas.zprobefeedfast
    #define Z_PROBE_FEEDRATE_SLOW HMI_datas.zprobefeedslow
  #endif
  #define INVERT_E0_DIR HMI_datas.invert_dir_extruder
  #if ENABLED(NOZZLE_PARK_FEATURE)
    #define NOZZLE_PARK_POINT {(float)HMI_datas.Park_point.x, (float)HMI_datas.Park_point.y, (float)HMI_datas.Park_point.z}
  #endif
  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    #define FILAMENT_CHANGE_UNLOAD_FEEDRATE (float)HMI_datas.fil_unload_feedrate
    #define FILAMENT_CHANGE_FAST_LOAD_FEEDRATE (float)HMI_datas.fil_fast_load_feedrate
  #endif


#endif


