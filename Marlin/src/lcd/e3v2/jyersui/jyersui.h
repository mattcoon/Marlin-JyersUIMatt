/**
 * JYERSUI UI Enhanced implementation
 * Version: 1.0
 * Date: 2022/02/04
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

#include "../../../core/types.h"
#include "dwin_lcd.h"
#include "../common/dwin_set.h"
#include "../common/dwin_font.h"
#include "../common/dwin_color.h"

// ICON ID

#ifndef USE_STOCK_DWIN_SET
  #define USE_STOCK_DWIN_SET           // Official Marlin DWIN_SET
#endif
#ifdef ICON
    #undef ICON
#endif
// #ifndef USE_STOCK_DWIN_SET
//   #define ICON                    0x07  // Default MarlinUI DWIN_SET
// #else 
//   #define ICON                    0x09  // Default Stock DWIN_SET
//   #define ICON_PACK               0x03  // Default MarlinUI DWIN_SET
// #endif
#define USE_STOCK_DWIN_SET  // Use the Creality stock DWIN_SET instead of Marlin's unified DWIN_SET by The-EG & thinkyhead
  #ifdef USE_STOCK_DWIN_SET
    #define DWIN_ICON_DEF 9 // 9.ICO
  #else
    #define DWIN_ICON_DEF 7 // 7.ICO
  #endif
// #endif
#if ENABLED(DWIN_ICON_SET)
    #define ICON HMI_datas.iconset
#else
    #define ICON DWIN_ICON_DEF
#endif


#if ENABLED(DWIN_CREALITY_LCD_CUSTOM_ICONS)
  //index of every custom icon should be >= CUSTOM_ICON_START
  #define CUSTOM_ICON_START         ICON_Checkbox_F
  #define ICON_Checkbox_F           200
  #define ICON_Checkbox_T           201
  #define ICON_Fade                 202
  #define ICON_Mesh                 203
  #define ICON_Tilt                 204
  #define ICON_Brightness           205
  #define ICON_Preview              ICON_File
  #define ICON_AxisD                249
  #define ICON_AxisBR               250
  #define ICON_AxisTR               251
  #define ICON_AxisBL               252
  #define ICON_AxisTL               253
  #define ICON_AxisC                254
  #if ENABLED(FWRETRACT)
    #define ICON_FWRetLength          ICON_StepE
    #define ICON_FWRecExtLength       ICON_StepE
    #define ICON_FWRetSpeed           ICON_Setspeed
    #define ICON_FWRetZRaise          ICON_MoveZ
    #define ICON_FWRecSpeed           ICON_Setspeed
  #endif
#else
  #define ICON_Fade                 ICON_Version
  #define ICON_Mesh                 ICON_Version
  #define ICON_Tilt                 ICON_Version
  #define ICON_Brightness           ICON_Version
  #define ICON_AxisD                ICON_Axis
  #define ICON_AxisBR               ICON_Axis
  #define ICON_AxisTR               ICON_Axis
  #define ICON_AxisBL               ICON_Axis
  #define ICON_AxisTL               ICON_Axis
  #define ICON_AxisC                ICON_Axis
  #define ICON_Preview              ICON_File
  #if ENABLED(FWRETRACT)
    #define ICON_FWRetLength          ICON_StepE
    #define ICON_FWRecExtLength       ICON_StepE
    #define ICON_FWRetSpeed           ICON_Setspeed
    #define ICON_FWRetZRaise          ICON_MoveZ
    #define ICON_FWRecSpeed           ICON_Setspeed
  #endif
#endif


#if ENABLED(DWIN_CREALITY_LCD_JYERSUI_GCODE_PREVIEW)
   #define Thumnail_Icon       0x00
   #define Thumnail_Preview    0x01
   #define Header_Time         0x00
   #define Header_Filament     0x01
   #define Header_Layer        0x02
#endif

// Extra Icons
#define ICON_Shortcut             ICON_Contact
#define ICON_AdvSet               ICON_Language
#define ICON_BedSizeX             ICON_PrintSize
#define ICON_BedSizeY             ICON_PrintSize
#define ICON_Binary               ICON_Contact
#define ICON_Cancel               ICON_StockConfiguration
#define ICON_Error                ICON_TempTooHigh
#define ICON_ESDiag               ICON_Info
#define ICON_FilLoad              ICON_WriteEEPROM
#define ICON_FilMan               ICON_ResumeEEPROM
#define ICON_FilSet               ICON_ResumeEEPROM
#define ICON_FilUnload            ICON_ReadEEPROM
#define ICON_Flow                 ICON_StepE
#define ICON_Folder               ICON_More
#define ICON_FWRetLength          ICON_StepE
#define ICON_FWRetSpeed           ICON_Setspeed
#define ICON_FWRetZRaise          ICON_MoveZ
#define ICON_FWRecSpeed           ICON_Setspeed
#define ICON_HomeX                ICON_MoveX
#define ICON_HomeY                ICON_MoveY
#define ICON_HomeZ                ICON_MoveZ
#define ICON_HomeOffset           ICON_AdvSet
#define ICON_HomeOffsetX          ICON_StepX
#define ICON_HomeOffsetY          ICON_StepY
#define ICON_HomeOffsetZ          ICON_StepZ
#define ICON_HSMode               ICON_StockConfiguration
#define ICON_InvertE0             ICON_StepE
#define ICON_Tram                 ICON_SetEndTemp
#define ICON_Level                ICON_HotendTemp
#define ICON_Lock                 ICON_Cool
#define ICON_ManualMesh           ICON_HotendTemp
#define ICON_MaxPosX              ICON_MoveX
#define ICON_MaxPosY              ICON_MoveY
#define ICON_MaxPosZ              ICON_MoveZ
#define ICON_MeshNext             ICON_Axis
#define ICON_MeshPoints           ICON_SetEndTemp
#define ICON_MeshSave             ICON_WriteEEPROM
#define ICON_MeshViewer           ICON_HotendTemp
#define ICON_MoveZ0               ICON_HotendTemp
#define ICON_Park                 ICON_Motion
#define ICON_ParkPos              ICON_AdvSet
#define ICON_ParkPosX             ICON_StepX
#define ICON_ParkPosY             ICON_StepY
#define ICON_ParkPosZ             ICON_StepZ
#define ICON_PhySet               ICON_PrintSize
#define ICON_PIDbed               ICON_SetBedTemp
#define ICON_PIDcycles            ICON_ResumeEEPROM
#define ICON_PIDValue             ICON_Contact
#define ICON_PrintStats           ICON_PrintTime
#define ICON_PrintStatsReset      ICON_RemainTime
#define ICON_ProbeDeploy          ICON_SetEndTemp
#define ICON_ProbeMargin          ICON_PrintSize
#define ICON_ProbeOffsetX         ICON_StepX
#define ICON_ProbeOffsetY         ICON_StepY
#define ICON_ProbeOffsetZ         ICON_StepZ
#define ICON_ProbeSet             ICON_SetEndTemp
#define ICON_ProbeStow            ICON_SetEndTemp
#define ICON_ProbeTest            ICON_SetEndTemp
#define ICON_ProbeZSpeed          ICON_MaxSpeedZ
#define ICON_Pwrlossr             ICON_Motion
#define ICON_Reboot               ICON_ResumeEEPROM
#define ICON_Runout               ICON_MaxAccE
#define ICON_Scolor               ICON_MaxSpeed
#define ICON_SetBaudRate          ICON_Setspeed
#define ICON_SetCustomPreheat     ICON_SetEndTemp
#define ICON_Sound                ICON_Cool

// Buttons
#define BTN_Continue          85
#define BTN_Cancel            87
#define BTN_Confirm           89
#define BTN_Print             90
#define BTN_Save              91


// UI element defines and constants
#define DWIN_FONT_MENU font8x16
#define DWIN_FONT_STAT font10x20
#define DWIN_FONT_HEAD font10x20
#define DWIN_FONT_ALERT font10x20
#define STATUS_Y 352
#define LCD_WIDTH (DWIN_WIDTH / 8)

constexpr uint16_t TITLE_HEIGHT = 30,                          // Title bar height
                   MLINE = 53,                                 // Menu line height
                   TROWS = (STATUS_Y - TITLE_HEIGHT) / MLINE,  // Total rows
                   MROWS = TROWS - 1,                          // Other-than-Back
                   ICOX = 26,                                  // Menu item icon X position
                   LBLX = 60,                                  // Menu item label X position
                   VALX = 210,                                 // Menu item value X position
                   MENU_CHR_W = 8, MENU_CHR_H = 16,            // Menu font 8x16
                   STAT_CHR_W = 10;

// Menuitem Y position
#define MYPOS(L) (TITLE_HEIGHT + MLINE * (L))

typedef struct { uint16_t left, top, right, bottom; } rect_t;
typedef struct { uint16_t x, y, w, h; } frame_rect_t;


namespace JYERSUI {
  extern xy_int_t cursor;
  extern uint16_t pencolor;
  extern uint16_t textcolor;
  extern uint16_t backcolor;
  extern uint16_t buttoncolor;
  extern uint8_t  font;

  extern FSTR_P const Author;


  // Set text/number font
  void setFont(uint8_t cfont);

  // Get font character width
  uint8_t fontWidth(uint8_t cfont);
  inline uint8_t fontWidth() { return fontWidth(font); };

  // Get font character height
  uint8_t fontHeight(uint8_t cfont);
  inline uint8_t fontHeight() { return fontHeight(font); };

  // Get screen x coordinates from text column
  uint16_t ColToX(uint8_t col);

  // Get screen y coordinates from text row
  uint16_t RowToY(uint8_t row);

  // Set text/number color
  void SetColors(uint16_t fgcolor, uint16_t bgcolor, uint16_t alcolor);
  void SetTextColor(uint16_t fgcolor);
  void SetBackgroundColor(uint16_t bgcolor);

  // Moves cursor to point
  //  x: abscissa of the display
  //  y: ordinate of the display
  //  point: xy coordinate
  void MoveTo(int16_t x, int16_t y);
  void MoveTo(xy_int_t point);

  // Moves cursor relative to the actual position
  //  x: abscissa of the display
  //  y: ordinate of the display
  //  point: xy coordinate
  void MoveBy(int16_t x, int16_t y);
  void MoveBy(xy_int_t point);

  // Draw a line from the cursor to xy position
  //  color: Line segment color
  //  x/y: End point
  inline void LineTo(uint16_t color, uint16_t x, uint16_t y) {
    DWIN_Draw_Line(color, cursor.x, cursor.y, x, y);
  }
  inline void LineTo(uint16_t x, uint16_t y) {
    DWIN_Draw_Line(pencolor, cursor.x, cursor.y, x, y);
  }

  // Extend a frame box
  //  v: value to extend
  inline frame_rect_t ExtendFrame(frame_rect_t frame, uint8_t v) {
    frame_rect_t t;
    t.x = frame.x - v;
    t.y = frame.y - v;
    t.w = frame.w + 2*v;
    t.h = frame.h + 2*v;
    return t;
  };


  // Draw a char at cursor position
  void Draw_Char(uint16_t color, const char c);
  inline void Draw_Char(const char c) { Draw_Char(textcolor, c); }

  // Draw a string at cursor position
  //  color: Character color
  //  *string: The string
  //  rlimit: For draw less chars than string length use rlimit
  void Draw_String(const char * const string, uint16_t rlimit = 0xFFFF);
  void Draw_String(uint16_t color, const char * const string, uint16_t rlimit = 0xFFFF);
  inline void Draw_String(FSTR_P  string, uint16_t rlimit = 0xFFFF) {
    Draw_String(FTOP(string), rlimit);
  }
  inline void Draw_String(uint16_t color, FSTR_P string, uint16_t rlimit = 0xFFFF) {
    Draw_String(color, FTOP(string), rlimit);
  }

  // Draw a string
  //  size: Font size
  //  color: Character color
  //  bColor: Background color
  //  x/y: Upper-left coordinate of the string
  //  *string: The string
  inline void Draw_String(uint16_t x, uint16_t y, const char * const string) {
    DWIN_Draw_String(false, font, textcolor, backcolor, x, y, string);
  }
  inline void Draw_String(uint16_t x, uint16_t y, FSTR_P title) {
    DWIN_Draw_String(false, font, textcolor, backcolor, x, y, FTOP(title));
  }
  inline void Draw_String(uint16_t color, uint16_t x, uint16_t y, const char * const string) {
    DWIN_Draw_String(false, font, color, backcolor, x, y, string);
  }
  inline void Draw_String(uint16_t color, uint16_t x, uint16_t y, FSTR_P title) {
    DWIN_Draw_String(false, font, color, backcolor, x, y, title);
  }
  inline void Draw_String(uint16_t color, uint16_t bgcolor, uint16_t x, uint16_t y, const char * const string) {
    DWIN_Draw_String(true, font, color, bgcolor, x, y, string);
  }
  inline void Draw_String(uint16_t color, uint16_t bgcolor, uint16_t x, uint16_t y, FSTR_P title) {
    DWIN_Draw_String(true, font, color, bgcolor, x, y, title);
  }
  inline void Draw_String(uint8_t size, uint16_t color, uint16_t bgcolor, uint16_t x, uint16_t y, const char * const string) {
    DWIN_Draw_String(true, size, color, bgcolor, x, y, string);
  }
  inline void Draw_String(uint8_t size, uint16_t color, uint16_t bgcolor, uint16_t x, uint16_t y, FSTR_P title) {
    DWIN_Draw_String(true, size, color, bgcolor, x, y, title);
  }

  // Draw a centered string using DWIN_WIDTH
  //  bShow: true=display background color; false=don't display background color
  //  size: Font size
  //  color: Character color
  //  bColor: Background color
  //  y: Upper coordinate of the string
  //  *string: The string
  void Draw_CenteredString(bool bShow, uint8_t size, uint16_t color, uint16_t bColor, uint16_t x1, uint16_t x2, uint16_t y, const char * const string);
  inline void Draw_CenteredString(bool bShow, uint8_t size, uint16_t color, uint16_t bColor, uint16_t y, const char * const string) {
    Draw_CenteredString(bShow, size, color, bColor, 0, DWIN_WIDTH, y, string);
  }
  inline void Draw_CenteredString(bool bShow, uint8_t size, uint16_t color, uint16_t bColor, uint16_t y, FSTR_P string) {
    Draw_CenteredString(bShow, size, color, bColor, y, FTOP(string));
  }
  inline void Draw_CenteredString(uint16_t color, uint16_t bcolor, uint16_t y, const char * const string) {
    Draw_CenteredString(true, font, color, bcolor, y, string);
  }
  inline void Draw_CenteredString(uint8_t size, uint16_t color, uint16_t y, const char * const string) {
    Draw_CenteredString(false, size, color, backcolor, y, string);
  }
  inline void Draw_CenteredString(uint8_t size, uint16_t color, uint16_t y, FSTR_P title) {
    Draw_CenteredString(false, size, color, backcolor, y, title);
  }
  inline void Draw_CenteredString(uint16_t color, uint16_t y, const char * const string) {
    Draw_CenteredString(false, font, color, backcolor, y, string);
  }
  inline void Draw_CenteredString(uint16_t color, uint16_t y, FSTR_P title) {
    Draw_CenteredString(false, font, color, backcolor, y, title);
  }
  inline void Draw_CenteredString(uint16_t y, const char * const string) {
    Draw_CenteredString(false, font, textcolor, backcolor, y, string);
  }
  inline void Draw_CenteredString(uint16_t y, FSTR_P title) {
    Draw_CenteredString(false, font, textcolor, backcolor, y, title);
  }


  // Draw a box
  //  mode: 0=frame, 1=fill, 2=XOR fill
  //  color: Rectangle color
  //  frame: Box coordinates and size
  inline void Draw_Box(uint8_t mode, uint16_t color, frame_rect_t frame) {
    DWIN_Draw_Box(mode, color, frame.x, frame.y, frame.w, frame.h);
  }

  // Draw a circle
  //  Color: circle color
  //  x: abscissa of the center of the circle
  //  y: ordinate of the center of the circle
  //  r: circle radius
  void Draw_Circle(uint16_t color, uint16_t x, uint16_t y, uint8_t r);

  inline void Draw_Circle(uint16_t color, uint8_t r) {
    Draw_Circle(color, cursor.x, cursor.y, r);
  }

  // Draw a checkbox
  //  Color: frame color
  //  bColor: Background color
  //  x/y: Upper-left point
  //  checked : 0 : unchecked, 1 : checked
  void Draw_Checkbox(uint16_t color, uint16_t bcolor, uint16_t x, uint16_t y, bool checked);
  inline void Draw_Checkbox(uint16_t x, uint16_t y, bool checked=false) {
    Draw_Checkbox(textcolor, backcolor, x, y, checked);
  }

  // Color Interpolator
  //  val : Interpolator minv..maxv
  //  minv : Minimum value
  //  maxv : Maximum value
  //  color1 : Start color
  //  color2 : End color
  uint16_t ColorInt(int16_t val, int16_t minv, int16_t maxv, uint16_t color1, uint16_t color2);
  
  // ------------------------- Buttons ------------------------------//

  void Draw_Button(uint16_t color, uint16_t bcolor, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const char * const string);
  inline void Draw_Button(uint16_t color, uint16_t bcolor, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, FSTR_P string) {
    Draw_Button(color, bcolor, x1, y1, x2, y2, FTOP(string));
  }
  inline void Draw_Button(FSTR_P string, uint16_t x, uint16_t y) {
    Draw_Button(textcolor, buttoncolor, x, y, x + 99, y + 37, string);
  }
  void Draw_Button(uint8_t id, uint16_t x, uint16_t y);

  // -------------------------- Extra -------------------------------//

  // Draw a circle filled with color
  //  bcolor: fill color
  //  x: abscissa of the center of the circle
  //  y: ordinate of the center of the circle
  //  r: circle radius
  void Draw_FillCircle(uint16_t bcolor, uint16_t x,uint16_t y,uint8_t r);
  inline void Draw_FillCircle(uint16_t bcolor, uint8_t r) {
    Draw_FillCircle(bcolor, cursor.x, cursor.y, r);
  }

  // Color Interpolator through Red->Yellow->Green->Blue
  //  val : Interpolator minv..maxv
  //  minv : Minimum value
  //  maxv : Maximum value
  uint16_t RainbowInt(int16_t val, int16_t minv, int16_t maxv);

  // Clear Menu by filling the area with background color
  // Area (0, TITLE_HEIGHT, DWIN_WIDTH, STATUS_Y - 1)
  void ClearMenuArea();

};
