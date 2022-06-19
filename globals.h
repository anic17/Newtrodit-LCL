/*
    Newtrodit: A console text editor
    Copyright (C) 2021-2022 anic17 Software

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

const char newtrodit_version[] = "0.6";
const char newtrodit_build_date[] = "18/4/2022";
const char newtrodit_repository[] = "https://github.com/anic17/Newtrodit";
char manual_file[MAX_PATH] = "newtrodit.man";
char settings_file[MAX_PATH] = "newtrodit.config";
const char newtrodit_commit[] = ""; // Example commit

#define DEFAULT_NL "\n"

const int MANUAL_BUFFER_X = 350;
const int MANUAL_BUFFER_Y = 1000;

int TAB_WIDE = TAB_WIDE_;
int CURSIZE = CURSIZE_;
int LINECOUNT_WIDE = LINECOUNT_WIDE_;
int lineCount = false; // Is line count enabled?

int goto_len = 4;

// Boolean settings
int convertTabtoSpaces = true;
int convertNull = true;
int trimLongLines = false;
int cursorSizeInsert = true;
int wrapLine = false;
int autoIndent = true;
int fullPathTitle = true;
int useOldKeybinds = false; // Bool to use old keybinds (^X instead of ^Q, ^K instead of ^X)
int longPositionDisplay = false;
int generalUtf8Preference = false;
int partialMouseSupport = true; // Partial mouse support, only changes cursor position when mouse is clicked
int showMillisecondsInTime = false; // Show milliseconds in time insert function (F6)
int useLogFile = true;
int createNewLogFiles = false; // Create new log files when logging is enabled


int bpsPairHighlight = false; // Defining this as a global rather than a local variable to make it easier to change settings in different tabs

int bps_pair_colors[] = {0x5, 0xb, 0xd, 0x6, 0xc, 0xe, 0x8, 0xf, 0x9}; // Brackets, parenthesis and square brackets colors
char bps_chars_open[10][3] = {"(", "{", "["};                          // Allocate 10 chars for each char array to support new syntax
char bps_chars_close[10][3] = {")", "}", "]"};

// Interal global variables
int clearAllBuffer = true;
int allowAutomaticResizing = true; // Changing this value can cause issues with the editor functions

int c = 0; // Different error/debug codes

static int multiLineComment = false;
int syntaxHighlighting = false;
int syntaxAfterDisplay = false; // Display syntax after display
int allocateNewBuffer = true;

int file_index = 0;
int open_files = 1;

int horizontalScroll = 0;
int wrapSize = 100; // Default wrap size
int scrollRate = 3; // Default scroll rate (scroll 3 lines per mouse wheel). Needs to have partialMouseSupport enabled

#define BG_DEFAULT 0x07 // Background black, foreground (font) white
#define FG_DEFAULT 0x70 // Background white, foreground (font) black

int bg_color = BG_DEFAULT; // Background color (menu)
int fg_color = FG_DEFAULT; // Foreground color (text)

char **old_open_files;
int oldFilesIndex = 0;

char *run_macro, *last_known_exception;

#define DEFAULT_COMPILER "gcc"
#define DEFAULT_COMPILER_FLAGS "-Wall -O2 -pedantic -std=c99" // Assuming it's GCC and it's C99 compliant

// Syntax highlighting

#define DEFAULT_SYNTAX_LANG "C"
#define DEFAULT_SEPARATORS ",()+-/*=~[];{}<> \t"
#define DEFAULT_COMMENTS "//"

#define DEFAULT_SYNTAX_COLOR 0x7  // White
#define DEFAULT_COMMENT_COLOR 0x8 // Dark grey
#define DEFAULT_QUOTE_COLOR 0xe   // Yellow

#define DEFAULT_NUM_COLOR 0x2     // Dark green
#define DEFAULT_CAPITAL_COLOR 0xc // Red
#define DEFAULT_CAPITAL_MIN_LEN 3 // Highlight capital words 3 or more characters long

#define SEPARATORS DEFAULT_SEPARATORS

char syntax_separators[512] = SEPARATORS;
char syntax_filename[MAX_PATH] = "";

int default_color = DEFAULT_SYNTAX_COLOR;
int comment_color = DEFAULT_COMMENT_COLOR;
int quote_color = DEFAULT_QUOTE_COLOR;
int num_color = DEFAULT_NUM_COLOR;
int capital_color = DEFAULT_CAPITAL_COLOR;
int capital_min_len = DEFAULT_CAPITAL_MIN_LEN;
int capitalMinEnabled = true;
int singleQuotes = false;

typedef struct keywords
{
    char *keyword;
    int color;
} keywords_t;

keywords_t keywords[] = {
    {"break", 5},
    {"continue", 5},
    {"return", 5},

    {"auto", 9},
    {"const", 9},
    {"volatile", 9},
    {"extern", 9},
    {"static", 9},

    {"inline", 9},
    {"restrict", 9},

    {"char", 9},
    {"int", 9},
    {"short", 9},
    {"float", 9},
    {"double", 9},
    {"long", 9},
    {"bool", 9},
    {"void", 9},

    {"double_t", 0xa},
    {"div_t", 0xa},
    {"float_t", 0xa},
    {"fpos_t", 0xa},
    {"max_align_t", 0xa},
    {"mbstate_t", 0xa},
    {"nullptr_t", 0xa},
    {"ptrdiff_t", 0xa},
    {"sig_atomic_t", 0xa},
    {"size_t", 0xa},
    {"time_t", 0xa},
    {"wchar_t", 0xa},
    {"wint_t", 0xa},
    {"FILE", 0xa},

    {"uint8_t", 0xa},
    {"uint16_t", 0xa},
    {"uint32_t", 0xa},
    {"uint64_t", 0xa},
    {"uint128_t", 0xa},

    {"int8_t", 0xa},
    {"int16_t", 0xa},
    {"int32_t", 0xa},
    {"int64_t", 0xa},
    {"int128_t", 0xa},

    {"struct", 6},
    {"enum", 6},
    {"union", 6},
    {"typedef", 6},
    {"unsigned", 9},
    {"signed", 9},
    {"sizeof", 9},
    {"register", 9},

    {"do", 6},
    {"if", 6},
    {"else", 6},
    {"while", 6},
    {"switch", 6},
    {"for", 6},
    {"case", 6},
    {"default", 6},
    {"goto", 6},

    {"#include", 0xb},
    {"#pragma", 0xb},
    {"#define", 0xb},
    {"#ifdef", 0xb},
    {"#undef", 0xb},

    {"#ifndef", 0xb},
    {"#endif", 0xb},
    {"#if", 0xb},
    {"#else", 0xb},
    {"#elif", 0xb},
    {"#error", 0xb},
    {"#warning", 0xb},
    {"#line", 0xb},
    {"defined", 0xb},
    {"not", 0xb},

    {"main", 0x6},
    {"WinMain", 0x6},

    // Macro constants
    {"NULL", 0x9},
    {"EOF", 0x9},
    {"WEOF", 0x9},
    {"FILENAME_MAX", 0x9},
    {"WEOF", 0x9},
    {"true", 0x9},
    {"false", 0x9},
    {"errno", 0x9},
    {"stdin", 0x9},
    {"stdout", 0x9},
    {"stderr", 0x9},
    {"__LINE__", 0x9},
    {"__TIME__", 0x9},
    {"__DATE__", 0x9},
    {"__FILE__", 0x9},

    {"__STDC__", 0x9},
    {"__STDC_VERSION__", 0x9},
    {"__STDC_HOSTED__", 0x9},
    {"__OBJC__", 0x9},
    {"__ASSEMBLER__", 0x9},

    {"__cplusplus", 0x9},

    {"WINAPI", 0x6},
    {"APIENTRY", 0x6},

    {"_stdcall", 0x6},
    {"_CRTIMP", 0x6},
    {"__CRTIMP_INLINE", 0x6},

    {"__cdecl", 0x6},
    {"__asm__", 0x6},
    {"__volatile__", 0x6},
    {"__attribute__", 0x6},
    {"__AW_SUFFIXED__", 0x6},
    {"__AW_EXTENDED__", 0x6},
    {"__NAME__", 0x6},

};

struct
{
    char *keyword;
    int color;
} comment[] = {
    {"//", 0x8},
    {"/*", 0x8},
    {"*/", 0x8},
};
