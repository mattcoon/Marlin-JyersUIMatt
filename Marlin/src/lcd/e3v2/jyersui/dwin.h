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
 * lcd/e3v2/jyersui/dwin.h
 */


#include "dwin_defines.h"
#include "dwin_lcd.h"
#include "jyersui.h"

#include "../common/encoder.h"
#include "../../../libs/BL24CXX.h"

#include "../../../inc/MarlinConfig.h"

//#define DWIN_CREALITY_LCD_CUSTOM_ICONS
//#define BOOTPERSO
#ifndef DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW
  #define DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW
#endif

enum processID : uint8_t {
  Main, Print, Menu, Value, Option, File, Popup, Confirm, Keyboard, Wait, Locked, Short_cuts
};

enum PopupID : uint8_t {
  Pause, Stop, Resume, SaveLevel, ETemp, ConfFilChange, PurgeMore, MeshSlot,
  Level, Home, MoveWait, Heating,  FilLoad, FilChange, TempWarn, Runout, PIDWait, Resuming, ManualProbing,
  FilInsert, HeaterTime, UserInput, LevelError, InvalidMesh, NocreatePlane, UI, Complete, ConfirmStartPrint, BadextruderNumber,
  TemptooHigh, PIDTimeout, PIDDone, viewmesh, Level2, endsdiag, Reprint
};

enum menuID : uint8_t {
  MainMenu,
    Prepare,
      Move,
      HomeMenu,
      ManualLevel,
      ZOffset,
      Preheat,
      ChangeFilament,
      HostActions,
    Control,
      TempMenu,
        PID,
          HotendPID,
          BedPID,
        Preheat1,
        Preheat2,
        Preheat3,
        Preheat4,
        Preheat5,
      Motion,
        HomeOffsets,
        MaxSpeed,
        MaxAcceleration,
        MaxJerk,
        JDmenu,
        Steps,
      FwRetraction,
      Parkmenu,
      Visual,
        ColorSettings,
      HostSettings,
        ActionCommands,
      Advanced,
        ProbeMenu,
        Filmenu,
      Info,
    Leveling,
      LevelManual,
      LevelView,
      MeshViewer,
      LevelSettings,
      ManualMesh,
      UBLMesh,
    InfoMain,
  Tune,
    //Tune_FwRetraction,
  PreheatHotend
};


enum colorID : uint8_t {
  Default, White, Light_White, Blue, Yellow, Orange, Red, Light_Red, Green, Light_Green, Magenta, Light_Magenta, Cyan, Light_Cyan, Brown, Black
};

enum shortcutID : uint8_t {
  Preheat_menu, Cooldown, Disable_stepper, Autohome, ZOffsetmenu , M_Tramming_menu, Change_Fil, Move_rel_Z, ScreenL
};


extern char Hostfilename[66];

#define Color_Shortcut_0    0x10E4
#define Color_Shortcut_1    0x29A6
#define NB_Shortcuts        8

#define Custom_Colors_no_Black 14
#define Custom_Colors       15
#define Color_Aqua          RGB(0x00,0x3F,0x1F)
#define Color_Light_White   0xBDD7
//#define Color_Green         RGB(0x00,0x3F,0x00)
#define Color_Green         0x07E0
#define Color_Light_Green   0x3460
#define Color_Cyan          0x07FF
#define Color_Light_Cyan    0x04F3
#define Color_Blue          0x015F
#define Color_Light_Blue    0x3A6A
#define Color_Magenta       0xF81F
#define Color_Light_Magenta 0x9813
#define Color_Light_Red     0x8800
#define Color_Orange        0xFA20
#define Color_Light_Orange  0xFBC0
#define Color_Light_Yellow  0x8BE0
#define Color_Brown         0xCC27
#define Color_Light_Brown   0x6204
#define Color_Black         0x0000
#define Color_Grey          0x18E3
#define Check_Color         0x4E5C  // Check-box check color
#define Confirm_Color       0x34B9
#define Cancel_Color        0x3186
// Custom icons



class CrealityDWINClass {
public:
  
  static bool printing;
  static uint8_t iconset_current;
  static constexpr const char * const color_names[Custom_Colors + 1] = {"Default","  White","L_White","   Blue"," Yellow"," Orange","    Red","  L_Red","  Green","L_Green","Magenta","L_Magen","   Cyan"," L_Cyan","  Brown","  Black"};
  static constexpr const char * const preheat_modes[3] = { "Both", "Hotend", "Bed" };
  static constexpr const char * const zoffset_modes[3] = { "No Live" , "OnClick", "   Live" };
  #if HAS_FILAMENT_SENSOR
   static constexpr const char * const runoutsensor_modes[4] = { "   NONE" , "   HIGH" , "    LOW", " MOTION" };
  #endif
  #if ENABLED(DWIN_ICON_SET)
  static constexpr const char * const icon_set[2] = { " Custom" , "  Stock" };
  static constexpr const uint8_t icon_set_num[2] = {7,9};
  #endif
  static constexpr const char * const shortcut_list[NB_Shortcuts + 1] = { "Preheat" , " Cooldn" , "D. Step" , "HomeXYZ" , "ZOffset" , "M.Tram." , "Chg Fil" , "Move Z", "ScreenL" };
  static constexpr const char * const _shortcut_list[NB_Shortcuts + 1] = { GET_TEXT(MSG_PREHEAT) , GET_TEXT(MSG_COOLDOWN) , GET_TEXT(MSG_DIS_STEPS) , GET_TEXT(MSG_AUTO_HOME) , GET_TEXT(MSG_OFFSET_Z) , GET_TEXT(MSG_M_TRAMMING) , GET_TEXT(MSG_CHGFIL) , GET_TEXT(MSG_MOVE_Z) , GET_TEXT(MSG_SCREENLOCK) };

  static void Clear_Screen(uint8_t e=3);
  static void Draw_Float(float value, uint8_t row, bool selected=false, uint8_t minunit=10);
  static void Draw_Option(uint8_t value, const char * const * options, uint8_t row, bool selected=false, bool color=false);
  static void Draw_String(char * string, uint8_t row, bool selected=false, bool below=false);
  static const uint64_t Encode_String(const char * string);
  static void Decode_String(const uint64_t num, char string[8]);
  static uint16_t GetColor(uint8_t color, uint16_t original, bool light=false);
  static void Apply_shortcut(uint8_t shortcut);
  static void Draw_Checkbox(uint8_t row, bool value);
  static void Draw_Title(const char * title);
  static void Draw_Title(FSTR_P const title);
  static void Draw_Menu_Item(uint16_t row, uint8_t icon=0, const char * const label1=nullptr, const char * const label2=nullptr, bool more=false, bool centered=false, bool onlyCachedFileIcon=false);
  static void Draw_Menu_Item(uint8_t row, uint8_t icon=0, FSTR_P const flabel1=nullptr, FSTR_P const flabel2=nullptr, bool more=false, bool centered=false, bool onlyCachedFileIcon=false);
  static void Draw_Menu(uint8_t menu, uint8_t select=0, uint8_t scroll=0);
  static void Redraw_Menu(bool lastprocess=true, bool lastselection=false, bool lastmenu=false, bool flag_scroll=false);
  static void Redraw_Screen();

  static void Main_Menu_Icons();
  static void Draw_Main_Menu(uint8_t select=0);
  static void Print_Screen_Icons();
  static void Draw_Print_Screen();
  static void Draw_Print_Filename(const bool reset=false);
  static void Draw_Print_ProgressBar();
  #if ENABLED(USE_M73_REMAINING_TIME)
    static void Draw_Print_ProgressRemain();
  #endif
  static void Draw_Print_ProgressElapsed();
  static void Draw_Print_confirm();
  static void Draw_SD_Item(uint8_t item, uint8_t row, bool onlyCachedFileIcon=false);
  static void Draw_SD_List(bool removed=false, uint8_t select=0, uint8_t scroll=0, bool onlyCachedFileIcon=false);
  static void Draw_Status_Area(bool icons=false);
  static void Draw_Popup(FSTR_P const line1, FSTR_P const line2, FSTR_P const line3, uint8_t mode, uint8_t icon=0);
  static void Popup_Select(bool stflag=false);
  static void Update_Status_Bar(bool refresh=false);
  static void Draw_Keyboard(bool restrict, bool numeric, uint8_t selected=0, bool uppercase=false, bool lock=false);
  static void Draw_Keys(uint8_t index, bool selected, bool uppercase=false, bool lock=false);

  #if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW)
    static bool find_and_decode_gcode_preview(char *name, uint8_t preview_type, uint16_t *address, bool onlyCachedFileIcon=false);
    static bool find_and_decode_gcode_header(char *name, uint8_t header_type);
  #endif

  #if ENABLED(AUTO_BED_LEVELING_UBL)
    static void Draw_Bed_Mesh(int16_t selected = -1, uint8_t gridline_width = 1, uint16_t padding_x = 8, uint16_t padding_y_top = 40 + 53 - 7);
    static void Set_Mesh_Viewer_Status();
  #endif

  static FSTR_P Get_Menu_Title(uint8_t menu);
  static uint8_t Get_Menu_Size(uint8_t menu);
  static void Menu_Item_Handler(uint8_t menu, uint8_t item, bool draw=true);

  static void Popup_Handler(PopupID popupid, bool option = false);
  static void Confirm_Handler(PopupID popupid, bool option = false);

  static void Main_Menu_Control();
  static void Menu_Control();
  static void Value_Control();
  static void Option_Control();
  static void File_Control();
  static void Print_Screen_Control();
  static void Popup_Control();
  static void Confirm_Control();
  static void Keyboard_Control();

  static void Setup_Value(float value, float min, float max, float unit, uint8_t type);
  static void Modify_Value(float &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint8_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint16_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(int16_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint32_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(int8_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Option(uint8_t value, const char * const * options, uint8_t max);
  static void Modify_String(char * string, uint8_t maxlength, bool restrict);

  static void Update_Status(const char * const text);
  static void Start_Print(bool sd);
  static void Stop_Print();
  static void Update();
  static void State_Update();
  static void Screen_Update();
  static void AudioFeedback(const bool success=true);
  static void Save_Settings(char *buff);
  static void Load_Settings(const char *buff);
  static void Reset_Settings();

  static void Viewmesh();
  static void RebootPrinter();
  static void DWIN_RebootScreen();
  static void DWIN_ScreenLock();
  static void DWIN_ScreenUnLock();
  static void HMI_ScreenLock();
  static void DWIN_Hostheader(const char *text);
  static void DWIN_Init_diag_endstops();
  static bool DWIN_iSprinting () { return printing; }

  #if HAS_SHORTCUTS
    static void DWIN_Move_Z();
    static void DWIN_QuitMove_Z();
    static void HMI_Move_Z();
  #endif


  #if HAS_FILAMENT_SENSOR
    static void DWIN_Filament_Runout(const uint8_t extruder);
  #endif

  static void DWIN_Invert_Extruder();
  static void CPU_type();

  static void DWIN_Gcode(const int16_t codenum);
  static void DWIN_CError();
  static void DWIN_C12();
  static void DWIN_C108();
  static void DWIN_C510();
  static void DWIN_C997();


  enum pidresult_t   : uint8_t { PID_STARTED, PID_BAD_EXTRUDER_NUM, PID_TEMP_TOO_HIGH, PID_TUNING_TIMEOUT, PID_DONE };
  static void PidTuning(const pidresult_t pidresult);

  
};

extern CrealityDWINClass CrealityDWIN;
