/**
 * JYERSUI Enhanced implementation
 * Version:1
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


#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_CREALITY_LCD_JYERSUI)

#include "../../../inc/MarlinConfig.h"
#include "../../../core/types.h"
#include "dwin_lcd.h"
#include "jyersui.h"
//#include "dwin_defines.h"

//#define DEBUG_OUT 1
#include "../../../core/debug_out.h"

xy_int_t JYERSUI::cursor = { 0 };
uint16_t JYERSUI::pencolor = Color_White;
uint16_t JYERSUI::textcolor = Color_White;
uint16_t JYERSUI::backcolor = Color_Bg_Black;
uint16_t JYERSUI::buttoncolor = RGB( 0, 23, 16);
uint8_t  JYERSUI::font = font8x16;

FSTR_P const JYERSUI::Author = F(STRING_CONFIG_H_AUTHOR);

// Set text/number font
void JYERSUI::setFont(uint8_t cfont) {
  font = cfont;
}

// Get font character width
uint8_t JYERSUI::fontWidth(uint8_t cfont) {
  switch (cfont) {
    case font6x12 : return 6;
    case font8x16 : return 8;
    case font10x20: return 10;
    case font12x24: return 12;
    case font14x28: return 14;
    case font16x32: return 16;
    case font20x40: return 20;
    case font24x48: return 24;
    case font28x56: return 28;
    case font32x64: return 32;
    default: return 0;
  }
}

// Get font character height
uint8_t JYERSUI::fontHeight(uint8_t cfont) {
  switch (cfont) {
    case font6x12 : return 12;
    case font8x16 : return 16;
    case font10x20: return 20;
    case font12x24: return 24;
    case font14x28: return 28;
    case font16x32: return 32;
    case font20x40: return 40;
    case font24x48: return 48;
    case font28x56: return 56;
    case font32x64: return 64;
    default: return 0;
  }
}

// Get screen x coordinates from text column
uint16_t JYERSUI::ColToX(uint8_t col) {
  return col * fontWidth(font);
}

// Get screen y coordinates from text row
uint16_t JYERSUI::RowToY(uint8_t row) {
  return row * fontHeight(font);
}

// Set text/number color
void JYERSUI::SetColors(uint16_t fgcolor, uint16_t bgcolor, uint16_t alcolor) {
  textcolor = fgcolor;
  backcolor = bgcolor;
  buttoncolor = alcolor;
}
void JYERSUI::SetTextColor(uint16_t fgcolor) {
  textcolor = fgcolor;
}
void JYERSUI::SetBackgroundColor(uint16_t bgcolor) {
  backcolor = bgcolor;
}

// Moves cursor to point
//  x: abscissa of the display
//  y: ordinate of the display
//  point: xy coordinate
void JYERSUI::MoveTo(int16_t x, int16_t y) {
  cursor.x = x;
  cursor.y = y;
}
void JYERSUI::MoveTo(xy_int_t point) {
  cursor = point;
}

// Moves cursor relative to the actual position
//  x: abscissa of the display
//  y: ordinate of the display
//  point: xy coordinate
void JYERSUI::MoveBy(int16_t x, int16_t y) {
  cursor.x += x;
  cursor.y += y;
}
void JYERSUI::MoveBy(xy_int_t point) {
  cursor += point;
}

// Draw a Centered string using arbitrary x1 and x2 margins
void JYERSUI::Draw_CenteredString(bool bShow, uint8_t size, uint16_t color, uint16_t bColor, uint16_t x1, uint16_t x2, uint16_t y, const char * const string) {
  const uint16_t x = _MAX(0U, x2 + x1 - strlen_P(string) * fontWidth(size)) / 2 - 1;
  DWIN_Draw_String(bShow, size, color, bColor, x, y, string);
}

// Draw a Centered string using DWIN_WIDTH
// void JYERSUI::Draw_CenteredString(bool bShow, uint8_t size, uint16_t color, uint16_t bColor, uint16_t y, const char * const string) {
//   const int8_t x = _MAX(0U, DWIN_WIDTH - strlen_P(string) * fontWidth(size)) / 2 - 1;
//   DWIN_Draw_String(bShow, size, color, bColor, x, y, string);
// }


// Draw a char at cursor position
void JYERSUI::Draw_Char(uint16_t color, const char c) {
  const char string[2] = { c, 0};
  DWIN_Draw_String(false, font, color, backcolor, cursor.x, cursor.y, string, 1);
  MoveBy(fontWidth(font), 0);
}

// Draw a string at cursor position
//  color: Character color
//  *string: The string
//  rlimit: For draw less chars than string length use rlimit
void JYERSUI::Draw_String(const char * const string, uint16_t rlimit) {
  DWIN_Draw_String(false, font, textcolor, backcolor, cursor.x, cursor.y, string, rlimit);
  MoveBy(strlen(string) * fontWidth(font), 0);
}
void JYERSUI::Draw_String(uint16_t color, const char * const string, uint16_t rlimit) {
  DWIN_Draw_String(false, font, color, backcolor, cursor.x, cursor.y, string, rlimit);
  MoveBy(strlen(string) * fontWidth(font), 0);
}

void JYERSUI::Draw_Button(uint16_t color, uint16_t bcolor, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const char * const string) {
  DWIN_Draw_Rectangle(1, bcolor, x1, y1, x2, y2);
  Draw_CenteredString(0, font, color, bcolor, x1, x2, (y2 + y1 - fontHeight())/2, string);
}

void JYERSUI::Draw_Button(uint8_t id, uint16_t x, uint16_t y) {
  switch (id) {
    case BTN_Cancel  : Draw_Button(GET_TEXT_F(MSG_BUTTON_CANCEL), x, y); break;
    case BTN_Confirm : Draw_Button(GET_TEXT_F(MSG_BUTTON_CONFIRM), x, y); break;
    case BTN_Continue: Draw_Button(GET_TEXT_F(MSG_BUTTON_CONTINUE), x, y); break;
    case BTN_Print   : Draw_Button(GET_TEXT_F(MSG_BUTTON_PRINT), x, y); break;
    case BTN_Save    : Draw_Button(GET_TEXT_F(MSG_BUTTON_SAVE), x, y); break;
    default: break;
  }
}

// Draw a circle
//  color: circle color
//  x: the abscissa of the center of the circle
//  y: ordinate of the center of the circle
//  r: circle radius
void JYERSUI::Draw_Circle(uint16_t color, uint16_t x, uint16_t y, uint8_t r) {
  int a = 0, b = 0;
  while (a <= b) {
    b = SQRT(sq(r) - sq(a));
    if (a == 0) b--;
    DWIN_Draw_Point(color, 1, 1, x + a, y + b);   // Draw some sector 1
    DWIN_Draw_Point(color, 1, 1, x + b, y + a);   // Draw some sector 2
    DWIN_Draw_Point(color, 1, 1, x + b, y - a);   // Draw some sector 3
    DWIN_Draw_Point(color, 1, 1, x + a, y - b);   // Draw some sector 4
    DWIN_Draw_Point(color, 1, 1, x - a, y - b);   // Draw some sector 5
    DWIN_Draw_Point(color, 1, 1, x - b, y - a);   // Draw some sector 6
    DWIN_Draw_Point(color, 1, 1, x - b, y + a);   // Draw some sector 7
    DWIN_Draw_Point(color, 1, 1, x - a, y + b);   // Draw some sector 8
    a++;
  }
}

// Draw a circle filled with color
//  bcolor: fill color
//  x: the abscissa of the center of the circle
//  y: ordinate of the center of the circle
//  r: circle radius
void JYERSUI::Draw_FillCircle(uint16_t bcolor, uint16_t x,uint16_t y,uint8_t r) {
  int a = 0, b = 0;
  while (a <= b) {
    b = SQRT(sq(r) - sq(a)); // b=sqrt(r*r-a*a);
    if (a == 0) b--;
    DWIN_Draw_Line(bcolor, x-b,y-a,x+b,y-a);
    DWIN_Draw_Line(bcolor, x-a,y-b,x+a,y-b);
    DWIN_Draw_Line(bcolor, x-b,y+a,x+b,y+a);
    DWIN_Draw_Line(bcolor, x-a,y+b,x+a,y+b);
    a++;
  }
}

// Color Interpolator
//  val : Interpolator minv..maxv
//  minv : Minimum value
//  maxv : Maximum value
//  color1 : Start color
//  color2 : End color
uint16_t JYERSUI::ColorInt(int16_t val, int16_t minv, int16_t maxv, uint16_t color1, uint16_t color2) {
  uint8_t B, G, R;
  const float n = (float)(val - minv) / (maxv - minv);
  R = (1-n) * GetRColor(color1) + n * GetRColor(color2);
  G = (1-n) * GetGColor(color1) + n * GetGColor(color2);
  B = (1-n) * GetBColor(color1) + n * GetBColor(color2);
  return RGB(R, G, B);
}

// Color Interpolator through Red->Yellow->Green->Blue
//  val : Interpolator minv..maxv
//  minv : Minimum value
//  maxv : Maximum value
uint16_t JYERSUI::RainbowInt(int16_t val, int16_t minv, int16_t maxv) {
  uint8_t B, G, R;
  const uint8_t maxB = 28, maxR = 28, maxG = 38;
  const int16_t limv = _MAX(abs(minv), abs(maxv)); 
  float n = minv >= 0 ? (float)(val - minv) / (maxv - minv) : (float)val / limv;
  LIMIT(n, -1, 1);
  if (n < 0) {
    R = 0;
    G = (1 + n) * maxG;
    B = (-n) * maxB;
  }
  else if (n < 0.5) {
    R = maxR * n * 2;
    G = maxG;
    B = 0;
  }
  else {
    R = maxR;
    G = maxG * (1 - n);
    B = 0;
  }
  return RGB(R, G, B);
}

// Draw a checkbox
//  Color: frame color
//  bColor: Background color
//  x/y: Upper-left point
//  mode : 0 : unchecked, 1 : checked
void JYERSUI::Draw_Checkbox(uint16_t color, uint16_t bcolor, uint16_t x, uint16_t y, bool checked=false) {
  DWIN_Draw_String(true, font8x16, color, bcolor, x + 4, y, checked ? F("x") : F(" "));
  DWIN_Draw_Rectangle(0, color, x + 2, y + 2, x + 17, y + 17);
}

// Clear Menu by filling the menu area with background color
void JYERSUI::ClearMenuArea() {
  DWIN_Draw_Rectangle(1, backcolor, 0, TITLE_HEIGHT, DWIN_WIDTH - 1, STATUS_Y - 1);
}

#endif // end
