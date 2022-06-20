/*
        Newtrodit: A console text editor
        Copyright (c) 2021-2022 anic17 Software

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>
*/
#pragma once

#ifndef _NEWTRODIT_CORE_H_
#define _NEWTRODIT_CORE_H_
#else
#error "A newtrodit_core file has already been included"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/input.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "../dialog.h"
#include <ctype.h>
#include <fcntl.h> // _setmode, _fileno
#include <wchar.h> // For UTF-8 support

/* ================================= SETTINGS ==================================== */
#define _NEWTRODIT_OLD_SUPPORT 0
#define DEBUG_MODE 1

#define TAB_WIDE_       2
#define CURSIZE_        20
#define LINECOUNT_WIDE_ 4    // To backup original value
#define MIN_BUFSIZE     256
#define LINE_MAX        8192 
#define MAX_TABS        48   // Maximum number of files opened at once
#define MAX_PATH        260

// These need to be changed for linux
#define VK_TAB     8
#define VK_RETURN  13
#define VK_SHIFT   16
#define VK_CONTROL 17
#define VK_MENU    18
#define VK_ESCAPE  27



#if 0
#define HORIZONTAL_SCROLL
#endif

#ifndef __bool_true_false_are_defined
#define _bool_true_false_are_defined
typedef int bool;

#define false 0
#define true 1
#endif

#ifndef UNDO_STACK_SIZE
#define UNDO_STACK_SIZE 2048
#endif

#define filename_text_ "Untitled" // Default filename
#define DEFAULT_BUFFER_X 640

#define MAKE_INT64(hi, lo) ((LONGLONG(DWORD(hi) & 0xffffffff) << 32) | LONGLONG(DWORD(lo) & 0xffffffff)) // https://stackoverflow.com/a/21022647/12613647

// Only used for debugging, not for release
#if DEBUG_MODE
#define DEBUG printf("OK. File:%s Line:%d ", __FILE__, __LINE__); getch_n();
#else
#define DEBUG
#endif

#include "../globals.h"

enum ConsoleQueryList {
  XWINDOW = 1,
  XBUFFER_SIZE = 2,
  YWINDOW = 4,
  YBUFFER_SIZE = 8,
  XCURSOR = 16,
  YCURSOR = 32,
  COLOR = 64,
  XMAX_WINDOW = 128,
  YMAX_WINDOW = 256,
  CURSOR_SIZE = 512,
  CURSOR_VISIBLE = 1024,
};

int GetConsoleInfo(int type);

#define XSIZE GetConsoleInfo(XWINDOW)
#define YSIZE GetConsoleInfo(YWINDOW)

#define YSCROLL GetConsoleInfo(YWINDOW) - 2

#define BOTTOM GetConsoleInfo(YWINDOW) - 1

#define DEFAULT_ALLOC_SIZE 512
#define MACRO_ALLOC_SIZE 2048

int BUFFER_X = DEFAULT_BUFFER_X;
int BUFFER_Y = 5600;

int BUFFER_INCREMENT = 150;         // How much to increment the buffer size by when it is full.
int BUFFER_INCREMENT_LOADFILE = 50; // When loading a file, how much to increment the buffer size by.
/* ============================= END OF SETTINGS ================================= */

#if !defined ENABLE_VIRTUAL_TERMINAL_PROCESSING || !defined DISABLE_NEWLINE_AUTO_RETURN
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#endif

#define PATHTOKENS "\\/"

typedef struct Startup_info {
  char *location; // Location of the file (full path)
  char *dir;      // Directory that is changed with 'cd' or '_chdir'
  int xsize;      // X size of the window
  int ysize;      // Y size of the window
  int xbuf;
  int ybuf;
  int color;
  char **argv;
  int argc;
  int manual_open;
  bool save_buffer;
  char *log_file_name;
  struct cursor {

    int x;
    int y;
  } cursor;

} Startup_info; // Startup data when Newtrodit is started

typedef struct Undo_stack {
  char *line;
  int line_count;
  int line_pos;
  size_t size;
  bool create_nl;
  bool delete_nl;
} Undo_stack;

typedef struct Syntax_info {
  // File and file type
  char *syntax_file;
  char *syntax_lang;
  char *separators;

  // Colors
  int capital_color;
  int comment_color;
  int default_color;
  int num_color;
  int override_color;
  int quote_color;

  // Various size parameters

  size_t keyword_count;
  size_t capital_min; // Minimum length of a word to be highlighted as capital

  bool capital_enabled;
  bool multi_line_comment;

  // Highlight pairs of brackets, parenthesis, and square brackets {}, (), []
  size_t bracket_pair_count;
  size_t parenthesis_pair_count;
  size_t square_bracket_pair_count;

  // Keyword info
  char *comments;
  char **keywords;
  int *color;
} Syntax_info; // Syntax highlighting information

typedef struct Compiler {
  char *path;
  char *flags;
  char *output;
} Compiler; // Compiler settings, used for macro building command lines

typedef struct File_info {
  char *filename;
  char *fullpath;

  bool is_readonly;
  int permissions;

  bool is_loaded;
  bool utf8;

  // File information
  int xpos;
  int ypos;
  int display_x;
  int display_y;
  int topmost_display_y; // Used for scrolling
  int last_pos_scroll;   // Last X position before scrolling

  bool scrolled_x; // If the cursor is scrolled to the right
  bool scrolled_y; // If the file is scrolled vertically

  char **strsave; // Buffer to store the file
  int **tabcount; // Number of tabs in the line
  int **linesize; // Size of the line
  size_t linecount;
  size_t linecount_wide;
  long long size;
  size_t bufx;
  size_t bufy;

  char *newline;

  bool is_saved; // Replacing old variables 'isSaved', 'isModified', 'isUntitled'
  bool is_modified;
  bool is_untitled;
  Undo_stack *Ustack;
  Syntax_info Syntaxinfo;
  Compiler Compilerinfo;
  void* hFile;        // File handle
  time_t fwrite_time; // When last written to
  time_t fread_time;  // The time Newtrodit read the file
} File_info; // File information. This is used to store all the information
             // about the file.

Startup_info SInf;
File_info Tab_stack[MAX_TABS];

/* ========================== LINUX REQUIRED FUNCTIONS =========================== */
void EchoOff() {
  struct termios term;
  tcgetattr(1, &term);
  term.c_lflag &= ~ECHO;
  tcsetattr(1, TCSANOW, &term);
}

void EchoOn() {
  struct termios term;
  tcgetattr(1, &term);
  term.c_lflag |= ECHO;
  tcsetattr(1, TCSANOW, &term);
}

void CanonOn() {
  struct termios term;
  tcgetattr(1, &term);
  term.c_lflag |= ICANON;
  tcsetattr(1, TCSANOW, &term);
}

void CanonOff() {
  struct termios term;
  tcgetattr(1, &term);
  term.c_lflag &= ~ICANON;
  tcsetattr(1, TCSANOW, &term);
}

/* Convert hex to ANSI (BIOS to ANSI standards)*/
char *HexToAnsi(int num) {
  char hex_table[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                        0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
  char ansi_table[16] = {30, 34, 32, 36, 31, 35, 33, 37,
                         90, 94, 91, 95, 92, 96, 93, 97};
  char *buf = (char *)calloc(1, sizeof(char) * 32); // Allocate enough memory
  if (num < 0 || num > 0xff)                        // Keep a valid number range
  {
    return NULL;
  }
  int font = 0, background = 0;
  if (num > 0xf) {
    font = ansi_table[num >> 4];
    background = ansi_table[num & 0xf] + 10;
    snprintf(buf, 32 * sizeof(char), "\x1b[%d;%dm", background, font);
  } else {
    font = ansi_table[num];
    snprintf(buf, 32 * sizeof(char), "\x1b[%dm", font);
  }
  return buf;
}

/* reads from keypress, doesn't echo */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

/* reads from keypress, echoes */
int getche(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

/* =================================== TERM ====================================== */
/* -------------------------------- TERM CURSOR ---------------------------------- */

/* Set cursor's X, Y position */
void gotoxy(int x, int y) {
  printf("\x1B[%d;%dH", y + 1, x + 1);
}

/* Change the visibility of the terminal cursor */
void DisplayCursor(bool disp) {
  if (disp) {
    printf("\x1B[?25h");
  } else {
    printf("\x1B[?25l");
  }
}

/* ---------------------------- TERM SCREEN CONTROL ------------------------------ */

/* Enter alternate screen buffer */
int EnterAltConsoleBuffer() {
  printf("\x1B[?1049h\x1B[H");

  return 0;
}

/* Restore original screen buffer (if not already active) */
int RestoreConsoleBuffer() {
  printf("\x1B[?1049l");

  return 0;
}

/* Set default FG color for text */
void SetColor(int color) {
  printf("%s", HexToAnsi(color));
}

/* Set color of a specific terminal cell (specified by X, and Y) */
void SetCharColor(size_t count, int color, int x, int y)
{
/* Not working
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD writePos = {x, y};
	DWORD dwWritten = 0;
	FillConsoleOutputAttribute(hConsole, HexColorToConsole(color), count, writePos, &dwWritten);
*/
}

/* Clear entire screen (keeping terminal cursor in the same location) */
void ClearScreen() {
  printf("\x1B[2J");
}

/* Clear a section of the screen
 *
 * Think of it as a rectangle. All this function does is create a rectangle
 * of a specified width and height at a position on the screen. Anything in
 * the rectangle will be cleared are replaced by a empty cell.
 *
 * BUG: ZSH has a weird bug causing gotoxy to not work properly. Hense
 *      ClearPartial() will not work properly on ZSH, a fix is being worked on.
 */
void ClearPartial(int x, int y, int width, int height) // Clears a section of the screen
{
  gotoxy(x, y);
  for (int i = 0; i < height + 1; i++) {
    gotoxy(x, y + i);
    printf("%*s\n", width, "");
  }
  gotoxy(x, y); // TODO: Make this go to old position
}

/* Function for printing a string on the last row of the screen */
void PrintBottomString(char *bottom_string) {
  int xs = XSIZE;
  ClearPartial(0, BOTTOM, xs, 1);
  printf("%.*s", xs, bottom_string); // Don't get out of the buffer
  return;
}

/* Set size of the console window */
void SetConsoleSize(int xsize, int ysize)
{
/* Not working
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD a = {xsize, ysize};

	SMALL_RECT rect;
	rect.Top = 0;
	rect.Left = 0;
	rect.Bottom = xsize - 1;
	rect.Right = ysize - 1;

	SetConsoleScreenBufferSize(handle, a);

	SetConsoleWindowInfo(handle, 1, &rect);
*/
}

/* ----------------------------- COLOR FUNCTIONS --------------------------------- */

int HexColorToConsole(int color_hex)
{
	return (color_hex / 16) << 4 | (color_hex % 16);
}

// !!!THIS IS OLD AND NEEDS TO BE REPLACED BY ANIC'S IMPLEMENTATION!!!
// Hex converts to RGB. If RGB is not supported in the terminal
// then this function will not work properly. I hope to fix this.
//
// Layer being 1: FG, 0: BG
/*
 char* HexColorToTerminal(int color_hex, int layer)
{
	char *esc;
	int r, g, b;
 	r = ((color_hex >> 16) & 0xFF) / 255.0; // Extract the RR byte
 	g = ((color_hex >> 8) & 0xFF) / 255.0;  // Extract the GG byte
 	b = ((color_hex) & 0xFF) / 255.0;       // Extract the BB byte
	if (!layer) {
		asprintf(&esc, "\x1B[48;2;%d;%d;%dm", r, g, b);
	} else {
		asprintf(&esc, "\x1B[38;2;%d;%d;%dm", r, g, b);
	}
}
*/

/* Convert a string hex to a decimal number
 *
 * Makes life a bit easier by removing repeditive calls
 */
int HexStrToDec(char *s)
{
	return strtol(s, NULL, 16);
}

/* Parse string hex to BIOS standards */
char *ParseHexString(char *hexstr)
{
	int count = 0, index = 0;
	char *hex_prefix = "0x";
	char *strpbrk_ptr;
	char *str = (char *)malloc(strlen(hexstr) + 1);
	if (!strpbrk(hexstr, "01234567890abcdefABCDEF"))
	{
		return hexstr;
	}

	// Convert an hex string to a string
	strpbrk_ptr = strpbrk(hexstr, ", \t");

	// Convert the string to a hex string
	for (int i = 0; i < strlen(hexstr); i++)
	{
		if (isxdigit(hexstr[i]) && isxdigit(hexstr[i + 1]))
		{
			str[index] = hexstr[i] * 16 + hexstr[i + 1];
			index++;
		}
		else if (hexstr[i] != ' ' && hexstr[i] != ',' && hexstr[i] != '\t')
		{
			return NULL;
		}
	}
}


/* =============================== END OF TERM ================================== */

/* Get line length, ignoring linefeeds */
size_t nolflen(char *s) {
  char *exclude = Tab_stack[file_index].newline;
  if (!strchr(exclude,
              '\n')) // Always exclude a newline (\n) even if it's not present
  {
    strncat(exclude, "\n", 1);
  }
  size_t len = 0;
  while (*s) {
    if (!strchr(exclude, *s)) {
      len++;
    }
    s++;
  }
  return len;
}

/* Count the number of chars in a string, chars to search set via delim.
 *
 * 	E.g. `TokCount(str, "!*")` will return the number of occurrences of `!`
 * and `*` in `s`
 */
int TokCount(char *s, char *delim) {
  int count = 0;
  for (int i = 0; i < strlen(s); i++) {
    for (int j = 0; j < strlen(delim); j++) {
      if (s[i] == delim[j]) {
        count++;
      }
    }
  }
  return count;
}

/* ? */
int TokLastPos(char *s, char *token) {
  int lastpos = -1;
  for (int i = 0; i < strlen(s); i++) {
    for (int j = 0; j < strlen(token); j++) {
      if (s[i] == token[j]) {
        lastpos = i;
      }
    }
  }
  return lastpos;
}

/* ? */
char *StrLastTok(char *tok, char *char_token) {
  int pos = -1; // Always initialize variables
  if ((pos = TokLastPos(tok, char_token)) == -1) {
    return strdup(tok);
  } else {
    return strdup(tok + pos + 1);
  }
}

/* ? */
int TokBackPos(char *s, char *p, char *p2) {
  /*
    p is the delimiter on the last position
    p2 is the delimiter for an index of + 1
  */
  for (int i = strlen(s); i > 0; i--) {
    for (int k = 0; k < strlen(p2); k++) {

      if (s[i] == p2[k]) {
        while (s[i] == p2[k]) {
          i--;
        }
        return i + 2;
      }
    }
    for (int j = 0; j < strlen(p); j++) {

      if (s[i] == p[j]) {
        while (s[i] == p[j]) {
          i--;
        }
        return i + 1;
      }
    }
  }
  return 0;
}

/* Print tab of set size? */
char *PrintTab(int tab_count) {
  char *s = (char *)malloc(sizeof(char) * (tab_count + 1));
  memset(s, 32, tab_count);
  return s;
}

/* ============================ STRING MANIPULATION ============================== */

/* ? */
char *strncpy_n(char *dest, const char *src, size_t count) {
  // Better version that strncpy() because it always null terminates strings

  if (count) {
    memset(dest, 0, count);
    strncat(dest, src, count);
    return dest;
  }
  return NULL;
}

/* ? */
size_t strrpbrk(char *s, char *find) // Reverse strpbrk, just like strrchr but for multiple characters
{
	size_t findlen = strlen(find);
	size_t slen = strlen(s);
	for (size_t i = slen; i > 0; i--)
	{
		for (size_t j = 0; j < findlen; j++)
		{
			if (s[i - 1] == find[j])
			{
				return i - 1;
			}
		}
	}
	return 0;
}

/* Find a needle in a haystack.
 *
 * Search a string for a sub-string
 */
int FindString(char *str, char *find) {
  char *ptr = strstr(str, find);
  return (ptr) ? (ptr - str) : -1;
}

/* ? */
// Pretty sure this just removes quotes on a string?
char *RemoveQuotes(char *dest, char *src) {
  if (src[0] == '\"' && src[strlen(src) - 1] == '\"') {
    strncpy_n(dest, src + 1, strlen(src) - 2);
  } else {
    strncpy_n(dest, src, strlen(src));
  }
  return dest;
}

/* ? */
// Pretty sure this just replaces specific characters in a string n amount of
// times
char *ReplaceString(char *s, char *find, char *replace, int *occurenceCount) {
  char *result;
  int c = 0;
  int replacelen = strlen(replace);
  int findlen = strlen(find);
  // BUG: Assigning to 'char *' from incompatible type 'void *'
  result = (char *)malloc(strlen(s) + 1 + BUFFER_X);
  for (size_t i = 0; s[i] != '\0'; i++) {
    if (strncmp(s + i, find, findlen) == 0) {
      strcpy(result + c, replace);
      (*occurenceCount)++;
      c += replacelen;
      i += findlen - 1;
    } else {
      result[c++] = s[i];
    }
  }
  result[c] = '\0';
  return result;
}

/* Safely join two strings by allocating memory */
char *join(const char *s1, const char *s2) {
  size_t arr_size = strlen(s1) + strlen(s2) + 1;
  char *s = (char *)malloc(arr_size);
  strncpy_n(s, s1, arr_size);
  strncat(s, s2, arr_size);
  last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

  return s;
}

/* Returns a string where all characters are lower case */
char *strlwr(char *s) {
  char *s2 = (char *)malloc(strlen(s) + 1);
  for (int i = 0; i < strlen(s); i++) {
    s2[i] = tolower(s[i]);
  }
  s2[strlen(s)] = '\0';
  return s2;
}

/* ? */
// Pretty sure this inserts a char at a specified index
char *InsertChar(char *str, char c, int pos) {
  char *new_str = (char *)malloc(strlen(str) + 8); // For safety
  if (!new_str) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NULL;
  }
  strcpy(new_str, str);
  memmove(new_str + pos + 1, str + pos, strlen(str) - (pos - 1));
  new_str[strlen(new_str)] = '\0';
  new_str[pos] = c;

  return new_str;
}

/* ? */
// Pretty sure this inserts a string at a specific index
char *InsertStr(char *s1, char *s2, int pos) {
  char *new_str = (char *)malloc(strlen(s1) + 8); // For safety
  if (!new_str) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NULL;
  }
  strcpy(new_str, s1);
  memmove(new_str + pos + strlen(s2), s1 + pos,
          strlen(s1) - (pos - strlen(s2)));
  new_str[strlen(new_str)] = '\0';
  memcpy(new_str + pos, s2, strlen(s2));
  last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

  return new_str;
}

/* ? */
// Pretty sure this deletes a char at a specific index
char *DeleteChar(char *str, int pos) {
  char *new_str = (char *)malloc(strlen(str));
  if (!new_str) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NULL;
  }
  int i;
  for (i = 0; i < pos; i++) {
    new_str[i] = str[i];
  }
  for (i = pos; i < strlen(str) - 1; i++) {
    new_str[i] = str[i + 1];
  }
  new_str[i] = '\0';

  return new_str;
}

/* ? */
// Pretty sure this deletes the char to the left of the specified index
char *DeleteCharLeft(char *str, int pos) {
  char *new_str = (char *)malloc(strlen(str) + 1);
  if (!new_str) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NULL;
  }
  strcpy(new_str, str);
  memmove(new_str + pos, str + pos + 1, strlen(str) - pos);
  new_str[strlen(new_str)] = '\0';

  return new_str;
}

/* ? */
char *InsertRow(char **arr, int startpos, size_t arrsize, char *arrvalue) {
  for (int i = arrsize; i > startpos; i--) {
    arr[i] = arr[i - 1];
  }
  arr[startpos + 1] = arrvalue;
  last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
  return arr[startpos];
}

/* ? */
char *DeleteRow(char **arr, int startpos, size_t arrsize) {
  for (int i = startpos; i < arrsize; i++) {
    arr[i] = arr[i + 1];
  }
  last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

  return arr[startpos];
}

/* ? */
char *InsertDeletedRow(File_info *tstack) {
  int n = nolflen(tstack->strsave[tstack->ypos]);
  strncat(tstack->strsave[tstack->ypos - 1], tstack->strsave[tstack->ypos],
          strlen(tstack->strsave[tstack->ypos])); // Concatenate the next line
  memset(tstack->strsave[tstack->ypos] + n, 0,
         BUFFER_X - n); // Empty the new line

  if (tstack->ypos != 1) {
    // Add the newline string only if the ypos isn't 1
    strncat(tstack->strsave[tstack->ypos - 1], tstack->newline,
            strlen(tstack->newline));
  }
  // Delete the old row, shifting other rows down
  DeleteRow(tstack->strsave, tstack->ypos, BUFFER_X);

  // Decrease the yp pointer by one
  tstack->xpos = nolflen(tstack->strsave[tstack->ypos]);
  last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

  return tstack->strsave[tstack->ypos];
}

/* Replace all tabs with 8 spaces and return another string */
char *RemoveTab(char *s) {
  char *new_s = (char *)malloc(strlen(s) + 1);
  if (!new_s) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NULL;
  }
  int n = TokCount(s, "\t");
  int tmp;

  new_s = ReplaceString(s, "\t", PrintTab(TAB_WIDE), &tmp);
  printf("%s", new_s);
  return new_s;
}

/* ? */
char *Substring(size_t start, size_t count, char *str) {
  char *new_str = (char *)malloc(count + 1);
  memset(new_str, 0, count + 1);
  strncat(new_str, str + start, count);
  return new_str;
}

/* ? */
char *StringToJSON(char *s) {
  char *s2 =
      (char *)malloc(strlen(s) * 2 + 100); // Allocate memory for the new string
  char squot = '\'', dquot = '"', brack = '{', brack2 = '}', comma = ',', colon,
       sqbr1 = '[', sqbr2 = ']';
  snprintf(s2, strlen(s) * 2 + 100, "%c%c%s%c%c%c ", brack, dquot, s, dquot,
           brack2, comma);
  return s2;
}

/* ========================= END OF STRING MANIPULATION ========================== */

/* =================================== SYSTEM ==================================== */

/* Checks the physical state of a key (Pressed or not pressed) */
int CheckKey(int keycode)
{
  return 1;
}

/* Get absolute path to a file on the drive */
char *FullPath(char *file) {
  char *path = (char *)calloc(MAX_PATH, sizeof(char));
  realpath(file, path);
  return path;
}

/* Return a string representing the current date and time */
char *GetTime(bool display_ms) {
  char *time_buf = (char *)malloc(512);
  if (!time_buf) {

    return NULL;
  }

  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  if (display_ms) {
    sprintf(time_buf, "%02d:%02d:%02d %02d/%02d/%04d",
        lt->tm_hour%12, lt->tm_min, lt->tm_sec,
        lt->tm_mday, lt->tm_mon+1, 1900+lt->tm_year);
  } else {
    sprintf(time_buf, "%02d:%02d:%02d %02d/%02d/%04d",
        lt->tm_hour%12, lt->tm_min, lt->tm_sec,
        lt->tm_mday, lt->tm_mon+1, 1900+lt->tm_year);
  }
  return time_buf;
}

/* Create a log file in the CWD (current working directory) */
int WriteLogFile(char *data) {
  if (useLogFile) {
    char *filename = SInf.log_file_name; // Get the log file name
    FILE *f = fopen(filename, "a");
    if (!f) {
      return -1;
    }
    fprintf(f, "[%s] %s\n", GetTime(true), data);
    fclose(f);
    return 0;
  } else {
    return -1; // No log file
  }
}

/* Cleanup routine (aka quitin' Newtrodit)*/
void ExitRoutine(int retval) // Cleanup routine
{
  if (SInf.location != NULL) {
    chdir(SInf.dir);
    free(SInf.location);
  }
#if _NEWTRODIT_EXPERIMENTAL_RESTORE_BUFFER == 1 && !_NEWTRODIT_OLD_SUPPORT
  if (SInf.save_buffer) {
    RestoreConsoleBuffer();
  }
#endif
  exit(retval);
}

/* Check if a file exists (will return 0 if file exists) */
int CheckFile(char *filename) {
  return (((access(filename, 0)) != -1 && (access(filename, 6)) != -1)) ? 0 : 1;
}

/* Improved getch(), returns all codes in a single call */
int getch_n() {
  int gc_n;
  gc_n = getch();
  if (gc_n == 0 || gc_n == 0xE0) {
    gc_n += 255;
    gc_n += getch();
  }
  return gc_n;
}

/* ================================ END OF SYSTEM =============================== */

char *itoa_n(int n) {
  char *s = (char *)malloc(DEFAULT_ALLOC_SIZE);
#ifdef _WIN32
  itoa(n, s, 10);
#else

#endif
  return s;
}

char *lltoa_n(long long n) // https://stackoverflow.com/a/18858248/12613647
{

  static char buf[256] = {0};

  int i = 62, base = 10;
  int sign = (n < 0);
  if (sign)
    n = -n;

  if (n == 0)
    return "0";

  for (; n && i; --i, n /= base) {
    buf[i] = "0123456789abcdef"[n % base];
  }

  if (sign) {
    buf[i--] = '-';
  }
  return &buf[i + 1];
}

char *ProgInfo() {

  char *info = (char *)malloc(1024);
  snprintf(info, 1024, "Newtrodit %s [Built at %s %s]", newtrodit_version,
           newtrodit_build_date, __TIME__);
  return info;
}

/* Check if the Terminal is a valid size to use Newtrodit */
int ValidSize() {
  if (XSIZE < 60 || YSIZE < 6) // Check for console size
  {
#ifdef _WIN32
    MessageBox(0, NEWTRODIT_ERROR_WINDOW_TOO_SMALL, "Newtrodit", 16);
#else

#endif
    return 0;
  }
  last_known_exception = NEWTRODIT_ERROR_WINDOW_TOO_SMALL;
  return 1;
}

int YesNoPrompt() {
  int chr = 0;
  while (1) {
    chr = getch_n();

    if (tolower(chr) == 'y') {
      return 1;
    } else if (tolower(chr) == 'n' || chr == 27) {
      return 0;
    }
  }
}

char *rot13(char *s) {
  for (int i = 0; i < strlen(s); i++) {
    if (isalpha(s[i])) {
      tolower(s[i]) >= 'n' ? (s[i] -= 13) : (s[i] += 13);
    }
  }
  return s;
}

int BufferLimit(File_info *tstack) {
  if (tstack->xpos >= BUFFER_X || tstack->ypos >= BUFFER_Y) {
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    PrintBottomString(NEWTRODIT_ERROR_OUT_OF_MEMORY);
    getch_n();
    return 1;
  }
  return 0;
}

void *realloc_n(void *old, size_t old_sz, size_t new_sz) {
  // Yeah I was getting a lot of error messages here
  // So I gave it the good ol' put it off for later treatment
}

char *ErrorMessage(int err, const char *filename) {

  switch (err) {
  case ENOMEM:
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
    return NEWTRODIT_ERROR_OUT_OF_MEMORY;
    break;
  case EACCES:
    last_known_exception = NEWTRODIT_FS_ACCESS_DENIED;
    return join(NEWTRODIT_FS_ACCESS_DENIED, filename);
    break;
  case ENOENT:
    last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;

    return join(NEWTRODIT_FS_FILE_NOT_FOUND, filename);
    break;
  case EFBIG:
    last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;

    return join(NEWTRODIT_FS_FILE_TOO_LARGE, filename);
    break;
  case EINVAL:
    last_known_exception = NEWTRODIT_FS_FILE_INVALID_NAME;

    return join(NEWTRODIT_FS_FILE_INVALID_NAME, filename);
    break;
  case EMFILE:
    last_known_exception = NEWTRODIT_ERROR_TOO_MANY_FILES_OPEN;
    return NEWTRODIT_ERROR_TOO_MANY_FILES_OPEN;
    break;
  case ENAMETOOLONG:

    last_known_exception = NEWTRODIT_FS_FILE_NAME_TOO_LONG;

    return join(NEWTRODIT_FS_FILE_NAME_TOO_LONG, filename);
    break;
  case ENOSPC:
    last_known_exception = NEWTRODIT_FS_DISK_FULL;

    return NEWTRODIT_FS_DISK_FULL;
    break;
  case EILSEQ:
    last_known_exception = NEWTRODIT_ERROR_INVALID_UNICODE_SEQUENCE;

    return NEWTRODIT_ERROR_INVALID_UNICODE_SEQUENCE;
    break;
  default:
    last_known_exception = NEWTRODIT_CRASH_UNKNOWN_EXCEPTION;

    return NEWTRODIT_ERROR_UNKNOWN;
    break;
  }
}

// What is this function?
#ifdef _WIN32
LPSTR GetErrorDescription(DWORD dwError) {
  LPSTR lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&lpMsgBuf, 0, NULL);
  return lpMsgBuf;
}
#endif

// Start a process to what?
#ifdef _WIN32
void StartProcess(char *command_line) {
  STARTUPINFO si;
  PROCESS_INFORMATION pinf;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pinf, sizeof(pinf));
  if (!CreateProcess(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
                     NULL, NULL, &si, &pinf)) {
    MessageBox(0,
               join(NEWTRODIT_ERROR_FAILED_PROCESS_START,
                    GetErrorDescription(GetLastError())),
               "Error", MB_ICONERROR);
  }
  CloseHandle(pinf.hProcess);
  CloseHandle(pinf.hThread);
  return;
}
#endif

char *get_path_directory(char *path, char *dest) // Not a WinAPI function
{
  strcpy(dest, path);
  int tmp_int = TokLastPos(path, "\\");
  if (tmp_int != -1) {

    memset(dest + tmp_int, 0, MAX_PATH - tmp_int);

    if (dest[strlen(dest) - 1] != '\\') {
      dest[strlen(dest)] = '\\';
    }
    return dest;
  } else {
    return NULL;
  }
}

void print_box_char() { printf("\u2610"); }

char *GetLogFileName() {
}

/*
 *   I've put some of the bigger functions here.
 * Makes scrolling through the code much easier
 */

int GetConsoleInfo(int type) {
  switch (type) {
  // Console width in cells
  case XWINDOW: {
    /*
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    return w.ws_col;
    */
  }

  // Console height in cells
  case YWINDOW: {
    /*
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    return w.ws_row;
    */
  }

  // Buffer width in cells
  case XBUFFER_SIZE:
    return 0;

  // Buffer height in cells
  case YBUFFER_SIZE:
    return 0;

  // Cursor's X position
  case XCURSOR: {
    EchoOff();
    CanonOff();
    int *X, *Y;
    printf("\x1B[6n");
    scanf("\x1B[%d;%dR", X, Y);
    return *X;
  }

  // Cursor's Y position
  case YCURSOR: {
    EchoOff();
    CanonOff();
    int *X, *Y;
    printf("\x1B[6n");
    scanf("\x1B[%d;%dR", X, Y);
    return *Y;
  }

  // Undocumented
  case COLOR:
    return 0;

  // Console's number of columns -- May be wrong
  case XMAX_WINDOW: {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    return w.ws_col;
  }

  // Console's number of rows -- May be wrong
  case YMAX_WINDOW: {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    return w.ws_row;
  }

  // Undocumented
  case CURSOR_SIZE:
    return 0;

  // Returns true or false depending on cursor visibility
  case CURSOR_VISIBLE:
    return 0;
  }
}

int AllocateBufferMemory(File_info *tstack) {
  WriteLogFile("Allocating buffer memory");

  tstack->strsave = (char **)calloc(sizeof(char *), BUFFER_Y);
  tstack->tabcount = (int **)malloc(sizeof(int *) * BUFFER_Y);
  tstack->linesize = (int **)malloc(sizeof(int *) * BUFFER_Y);
  BUFFER_X = DEFAULT_BUFFER_X;
  tstack->bufx = BUFFER_X;
  tstack->bufy = BUFFER_Y;
  tstack->utf8 = generalUtf8Preference;
  for (int i = 0; i < tstack->bufy; i++) {
    tstack->strsave[i] = (char *)calloc(tstack->bufx, sizeof(char));
    tstack->tabcount[i] = (int *)calloc(1, sizeof(int));
    tstack->linesize[i] = (int *)calloc(1, sizeof(int));
    if (!tstack->strsave[i] || !tstack->tabcount[i]) {
      last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

      WriteLogFile("Failed to allocate buffer memory");

      return 0;
    }
  }
  tstack->Ustack = 0; // Initialize the undo stack
  tstack->newline = (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
  tstack->newline = strdup(DEFAULT_NL);

  tstack->Compilerinfo.path = (char *)calloc(MAX_PATH, sizeof(char));
  tstack->Compilerinfo.flags = (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
  tstack->Compilerinfo.output = (char *)calloc(MAX_PATH, sizeof(char));
  tstack->Compilerinfo.path = strdup(DEFAULT_COMPILER);
  tstack->Compilerinfo.flags = strdup(DEFAULT_COMPILER_FLAGS);

  tstack->Syntaxinfo.syntax_lang =
      (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
  tstack->Syntaxinfo.syntax_file = (char *)calloc(MAX_PATH, sizeof(char));
  tstack->Syntaxinfo.separators =
      (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
  tstack->Syntaxinfo.comments =
      (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
  tstack->Syntaxinfo.syntax_lang = strdup(DEFAULT_SYNTAX_LANG);
  tstack->Syntaxinfo.syntax_file = "Default syntax highlighting";
  tstack->Syntaxinfo.separators = DEFAULT_SEPARATORS;
  tstack->Syntaxinfo.comments = DEFAULT_COMMENTS;
  tstack->Syntaxinfo.num_color = DEFAULT_NUM_COLOR;
  tstack->Syntaxinfo.capital_color = DEFAULT_CAPITAL_COLOR;
  tstack->Syntaxinfo.capital_min = DEFAULT_CAPITAL_MIN_LEN;
  tstack->Syntaxinfo.capital_enabled = capitalMinEnabled;
  tstack->Syntaxinfo.override_color = 0; // No override color

  tstack->Syntaxinfo.quote_color = DEFAULT_QUOTE_COLOR;
  tstack->Syntaxinfo.default_color = DEFAULT_SYNTAX_COLOR;
  tstack->Syntaxinfo.comment_color = DEFAULT_COMMENT_COLOR;
  tstack->Syntaxinfo.keyword_count = 0; // TODO: Initialize the keyword array
  tstack->Syntaxinfo.keywords =
      (char **)calloc(sizeof(keywords) / sizeof(keywords[0]), sizeof(char *));
  tstack->Syntaxinfo.color =
      (int *)calloc(sizeof(keywords) / sizeof(keywords[0]), sizeof(int));

  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {

    tstack->Syntaxinfo.keywords[i] =
        (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char));
    tstack->Syntaxinfo.keywords[i] = strdup(keywords[i].keyword);
    //tstack->Syntaxinfo.color[i] = (int)calloc(1, sizeof(int));
    tstack->Syntaxinfo.color[i] = keywords[i].color;
  }

  tstack->Syntaxinfo.multi_line_comment = false;
  tstack->Syntaxinfo.bracket_pair_count = 0;
  tstack->Syntaxinfo.parenthesis_pair_count = 0;
  tstack->Syntaxinfo.square_bracket_pair_count = 0;

  tstack->linecount_wide = LINECOUNT_WIDE;
  tstack->filename = (char *)calloc(MAX_PATH, sizeof(char));
  if (!tstack->filename) {
    WriteLogFile("Failed to allocate buffer memory");
    last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

    return 0;
  }
  tstack->filename = filename_text_;
  tstack->is_untitled = true;
  tstack->is_modified = false;
  tstack->is_saved = false;
  tstack->is_loaded = false;
  tstack->xpos = 0;
  tstack->ypos = 1;
  tstack->linecount = 1; // Only 1 line
  tstack->display_x = tstack->xpos;
  tstack->display_y = tstack->ypos;
  tstack->scrolled_x = false;
  tstack->scrolled_y = false;
  tstack->last_pos_scroll = 0;
  tstack->topmost_display_y = 1;
  tstack->is_readonly = false;
  //  tstack->hFile = INVALID_HANDLE_VALUE;
  // Initialize the file time structures
  
  /*tstack->fwrite_time.dwHighDateTime = 0;
  tstack->fwrite_time.dwLowDateTime = 0;
  tstack->fread_time.dwHighDateTime = 0;
  tstack->fread_time.dwLowDateTime = 0;*/
  WriteLogFile("Buffer memory successfully allocated");
  return 1;
}
