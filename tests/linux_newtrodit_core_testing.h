/*
 * Test file specificly target to testing the Linux functions.
 */

#pragma once

#ifndef _NEWTRODIT_CORE_H_
#define _NEWTRODIT_CORE_H_
#else
#error "newtrodit_core.h already included"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#include <direct.h>	 // _access()
#include <process.h> // _beginthread()
#include <cstdlib>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/input.h>
#include <unistd.h>
#include <time.h>

const char *keydev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
#endif
#include <fcntl.h> // _setmode, _fileno
#include <ctype.h>
#include <wchar.h> // For UTF-8 support
#include "dialog.h"

/* =============================== SETTINGS ================================== */
#define _NEWTRODIT_OLD_SUPPORT 0				         // Toggle support for old versions of Windows (Windows XP and below)
#define _NEWTRODIT_EXPERIMENTAL_RESTORE_BUFFER 0 // Toggle support for restoring buffer on exit (currently experimental)
#define DEBUG_MODE 1

#define TAB_WIDE_ 2
#define CURSIZE_ 20
#define LINECOUNT_WIDE_ 4 // To backup original value
#define MIN_BUFSIZE 256
#define LINE_MAX 8192
#define MAX_TABS 48       // Maximum number of files opened at once
#define MAX_PATH 260

#if 0
#define HORIZONTAL_SCROLL
#endif

#ifndef __bool_true_false_are_defined
#define _bool_true_false_are_defined
typedef int bool;

#define false 0
#define true 1
#endif

enum ConsoleQueryList
{
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

#ifndef _WIN32
/* ========================== LINUX REQUIRED FUNCTIONS =========================== */
void EchoOff()
{
	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~ECHO;
	tcsetattr(1, TCSANOW, &term);
}

void EchoOn()
{
	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag |= ECHO;
	tcsetattr(1, TCSANOW, &term);
}

void CanonOn()
{
	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag |= ICANON;
	tcsetattr(1, TCSANOW, &term);
}

void CanonOff()
{
	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~ICANON;
	tcsetattr(1, TCSANOW, &term);
}

/* Get char (GETCH). Returns the char as a keycode */
int sgetch()
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr );
  newattr = oldattr;
  newattr.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
  return ch;
}

/* Get physical key state (pressed, released, repeat) */
// NOTE: PLEASE PLEASE FIX THIS AT SOME POINT!
//       This code *is* a keylogger. It can be unsafe, this will be fixed in the future.
int GetKeyState(int keycode)
{
	int is_pressed = 0;
  struct input_event ev;
  FILE *indevice = fopen(keydev, "r");
  while (fread(&ev, sizeof(ev), 1, indevice) == 1)
    if (ev.type == EV_KEY && ev.code == keycode)
      switch (ev.value) {
        case 1: is_pressed=1; break;
        case 2: is_pressed=1; break;
        default: break;
      }
  fclose(indevice);
  return is_pressed;
}

/* Convert hex to ANSI (BIOS to ANSI standards)*/
char* HexToAnsi(int num)
{
    char hex_table[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    char ansi_table[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 91, 95, 92, 96, 93, 97};
    char* buf = (char*)calloc(1, sizeof(char) * 32); // Allocate enough memory
    if (num < 0 || num > 0xff)                       // Keep a valid number range
    {
        return NULL;
    }
    int font = 0, background = 0;
    if (num > 0xf)
    {
        font = ansi_table[num >> 4];
        background = ansi_table[num &0xf]+10;
        snprintf(buf, 32*sizeof(char), "\x1b[%d;%dm", background, font);
    }
    else
    {
        font = ansi_table[num];
        snprintf(buf, 32*sizeof(char), "\x1b[%dm", font);
    }
    return buf;
}


/* Convert Windows keycode to Linux standards */
int WinKeyToLin(int key_code) {
  return 0;
}

/* Convert Linux keycode to Windows standards */
int LinKeyToWin(int key_code) {
  return 0;
}

#endif

/* =================================== TERM  ===================================== */
/* -------------------------------- TERM CURSOR ---------------------------------- */

/* Set cursor's X, Y position */
void gotoxy(int x, int y)
{
#ifdef _WIN32
	COORD dwPos = {x, y};

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), dwPos);
#else
	printf("\x1B[%d;%dH", y + 1, x + 1);
#endif
}

/* Change the visibility of the terminal cursor */
void DisplayCursor(bool disp)
{
#ifdef _WIN32
	CONSOLE_CURSOR_INFO cursor;
	cursor.bVisible = disp;
	cursor.dwSize = CURSIZE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
#else
	if (disp) {
		printf("\x1B[?25h");
	} else {
		printf("\x1B[?25l");
	}
#endif
}

/* ---------------------------- TERM SCREEN CONTROL ------------------------------ */

/* Enter alternate screen buffer */
int EnterAltConsoleBuffer()
{
#ifdef _WIN32
	return 0;
#else
	printf("\x1B[?1049h\x1B[H");
	return 0;
#endif
}

/* Restore original screen buffer (if not already active) */
int RestoreConsoleBuffer()
{
#ifdef _WIN32
	SetConsoleActiveScreenBuffer(hOldBuf);
	CloseHandle(hNewBuf);
	SetStdHandle(STD_OUTPUT_HANDLE, hOldBuf);
	fclose(stdout);
	fclose(stderr);
	return 0;
#else
	printf("\x1B[?1049l");
	return 0;
#endif
}

/* Set default FG color for text */
void SetColor(int color)
{
#ifdef _WIN32
	HANDLE hConsoleColor = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsoleColor, HexColorToConsole(color));
#else
	// Not quite doing the right thing
	//printf(HexColorToTerminal(color, 1));
#endif
}

/* Set color of a specific terminal cell (specified by X, and Y) */
void SetCharColor(size_t count, int color, int x, int y)
{
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD writePos = {x, y};
	DWORD dwWritten = 0;
	FillConsoleOutputAttribute(hConsole, HexColorToConsole(color), count, writePos, &dwWritten);
#else

#endif
}

/* Clear entire screen (keeping terminal cursor in the same location) */
void ClearScreen()
{
#ifdef _WIN32
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count, cellCount;
	COORD homeCoords = {0, 0};

	if (hStdOut == INVALID_HANDLE_VALUE)
	{
		return;
	}

	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		return;
	}
	cellCount = csbi.dwSize.X * csbi.dwSize.Y; // All the buffer, not only the visible part

	FillConsoleOutputCharacter(hStdOut, L' ', cellCount, homeCoords, &count);
	FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count);

	SetConsoleCursorPosition(hStdOut, homeCoords);
#else
	printf("\x1B[2J");
#endif
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
#ifdef _WIN32
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count;
	COORD homeCoords = {x, y};

	if (hStdOut == INVALID_HANDLE_VALUE)
	{
		return;
	}

	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		return;
	}
	for (int i = 0; i < height; i++)
	{
		FillConsoleOutputCharacter(hStdOut, ' ', width, homeCoords, &count);
		FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, width, homeCoords, &count);

		homeCoords.Y++;
	}
	homeCoords.Y = y;
	SetConsoleCursorPosition(hStdOut, homeCoords);
	return;
#else
	gotoxy(x, y);
	for (int i = 0; i < height + 1; i++)
	{
		gotoxy(x, y + i);
		printf("%*s\n", width, "");
	}
	gotoxy(x, y); // TODO: Make this go to old position
#endif
}

/* Function for printing a string on the last row of the screen */
void PrintBottomString(char *bottom_string)
{
	int xs = XSIZE;
	ClearPartial(0, BOTTOM, xs, 1);
	printf("%.*s", xs, bottom_string); // Don't get out of the buffer
	return;
}

/* Set size of the console window */
void SetConsoleSize(int xsize, int ysize)
{
#ifdef _WIN32
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD a = {xsize, ysize};

	SMALL_RECT rect;
	rect.Top = 0;
	rect.Left = 0;
	rect.Bottom = xsize - 1;
	rect.Right = ysize - 1;

	SetConsoleScreenBufferSize(handle, a);

	SetConsoleWindowInfo(handle, 1, &rect);
#else

#endif
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


/* =============================== END OF TERM  ================================== */

/* Count the number of chars in a string, chars to search set via delim.
 *
 * 	E.g. `TokCount(str, "!*")` will return the number of occurrences of `!`
 * and `*` in `s`
 */
int TokCount(char *s, char *delim)
{
	int count = 0;
	for (int i = 0; i < strlen(s); i++)
	{
		for (int j = 0; j < strlen(delim); j++)
		{
			if (s[i] == delim[j])
			{
				count++;
			}
		}
	}
	return count;
}

/* ? */
int TokLastPos(char *s, char *token)
{
	int lastpos = -1;
	for (int i = 0; i < strlen(s); i++)
	{
		for (int j = 0; j < strlen(token); j++)
		{
			if (s[i] == token[j])
			{
				lastpos = i;
			}
		}
	}
	return lastpos;
}

/* ? */
char *StrLastTok(char *tok, char *char_token)
{
	int pos = -1; // Always initialize variables
	if ((pos = TokLastPos(tok, char_token)) == -1)
	{
		return strdup(tok);
	}
	else
	{
		return strdup(tok + pos + 1);
	}
}

/* ? */
int TokBackPos(char *s, char *p, char *p2)
{
	/*
		p is the delimiter on the last position
		p2 is the delimiter for an index of + 1
	*/
	for (int i = strlen(s); i > 0; i--)
	{
		for (int k = 0; k < strlen(p2); k++)
		{

			if (s[i] == p2[k])
			{
				while (s[i] == p2[k])
				{
					i--;
				}
				return i + 2;
			}
		}
		for (int j = 0; j < strlen(p); j++)
		{

			if (s[i] == p[j])
			{
				while (s[i] == p[j])
				{
					i--;
				}
				return i + 1;
			}
		}
	}
	return 0;
}


/* Find a needle in a haystack.
 *
 * Search a string for a sub-string
 */
int FindString(char *str, char *find)
{
	char *ptr = strstr(str, find);
	return (ptr) ? (ptr - str) : -1;
}


/* Returns a string where all characters are lower case */
char *strlwr(char *s)
{
	char *s2 = (char *)malloc(strlen(s) + 1);
	for (int i = 0; i < strlen(s); i++)
	{
		s2[i] = tolower(s[i]);
	}
	s2[strlen(s)] = '\0';
	return s2;
}

/* ========================= END OF STRING MANIPULATION ========================== */


/* =================================== SYSTEM  =================================== */

/* Checks the physical state of a key (Pressed or not pressed) */
int CheckKey(int keycode)
{
	return GetKeyState(keycode) <= -127;
}

/* Get absolute path to a file on the drive */
char *FullPath(char *file)
{
	char *path = (char *)calloc(MAX_PATH, sizeof(char));
#ifdef _WIN32
	GetFullPathName(file, MAX_PATH * sizeof(char), path, NULL);
#else
	realpath(file, path);
#endif
	return path;
}

/* Check if a file exists (will return 0 if file exists) */
int CheckFile(char *filename)
{
	return (((access(filename, 0)) != -1 && (access(filename, 6)) != -1)) ? 0 : 1;
}

/* ================================ END OF SYSTEM  =============================== */



char *lltoa_n(long long n) // https://stackoverflow.com/a/18858248/12613647
{

	static char buf[256] = {0};

	int i = 62, base = 10;
	int sign = (n < 0);
	if (sign)
		n = -n;

	if (n == 0)
		return "0";

	for (; n && i; --i, n /= base)
	{
		buf[i] = "0123456789abcdef"[n % base];
	}

	if (sign)
	{
		buf[i--] = '-';
	}
	return &buf[i + 1];
}

char *rot13(char *s)
{
	for (int i = 0; i < strlen(s); i++)
	{
		if (isalpha(s[i]))
		{
			tolower(s[i]) >= 'n' ? (s[i] -= 13) : (s[i] += 13);
		}
	}
	return s;
}

void *realloc_n(void *old, size_t old_sz, size_t new_sz)
{
#ifdef _WIN32
	void *new = malloc(new_sz);
	if (!new)
	{	last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

		return NULL;
	}
	memcpy(new, old, old_sz);
	free(old);
	return new;
#else
	// Yeah I was getting a lot of error messages here
	// So I gave it the good ol' put it off for later treatment
#endif
}

// What is this function?
#ifdef _WIN32
LPSTR GetErrorDescription(DWORD dwError)
{
	LPSTR lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
	return lpMsgBuf;
}
#endif

// Start a process to what?
#ifdef _WIN32
void StartProcess(char *command_line)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pinf;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pinf, sizeof(pinf));
	if (!CreateProcess(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pinf))
	{
		MessageBox(0, join(NEWTRODIT_ERROR_FAILED_PROCESS_START, GetErrorDescription(GetLastError())), "Error", MB_ICONERROR);
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
	if (tmp_int != -1)
	{

		memset(dest + tmp_int, 0, MAX_PATH - tmp_int);

		if (dest[strlen(dest) - 1] != '\\')
		{
			dest[strlen(dest)] = '\\';
		}
		return dest;
	}
	else
	{
		return NULL;
	}
}

void print_box_char()
{
	printf("\u2610");
}




/*
 *   I've put some of the bigger functions here.
 * Makes scrolling through the code much easier
 */


int GetConsoleInfo(int type)
{
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	CONSOLE_CURSOR_INFO cci;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi) || !GetConsoleCursorInfo(hConsole, &cci))
	{
		return -1;
	}
	switch (type)
	{
	// Console width in cells
	case XWINDOW:
		return csbi.srWindow.Right - csbi.srWindow.Left + 1;

	// Console height in cells
	case YWINDOW:
		return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	// Buffer width in cells
	case XBUFFER_SIZE:
		return csbi.dwSize.X;

	// Buffer height in cells
	case YBUFFER_SIZE:
		return csbi.dwSize.Y;

	// Cursor's X position
	case XCURSOR:
		return csbi.dwCursorPosition.X;

	// Cursor's Y position
	case YCURSOR:
		return csbi.dwCursorPosition.Y;

	// Current console color
	case COLOR:
		return csbi.wAttributes;

	// Console's number of columns -- May be wrong
	case XMAX_WINDOW:
		return csbi.dwMaximumWindowSize.X;

	// Console's number of rows -- May be wrong
	case YMAX_WINDOW:
		return csbi.dwMaximumWindowSize.Y;

	// Cursor's size - From 0 (invisible) to 100 (full cell)
	case CURSOR_SIZE:
		return cci.dwSize;

	// Returns true or false depending on cursor visibility
	case CURSOR_VISIBLE:
		return cci.bVisible;

	default:
		return 1;
	}
#else
	switch (type)
	{
	// Console width in cells
	case XWINDOW:
	{
		/*
		struct winsize w;
		ioctl(0, TIOCGWINSZ, &w);

		return w.ws_col;
		*/
	}

	// Console height in cells
	case YWINDOW:
	{
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
	case XCURSOR:
	{
		EchoOff();
		CanonOff();
		int *X, *Y;
		printf("\x1B[6n");
		scanf("\x1B[%d;%dR", X, Y);
		return *X;
	}

	// Cursor's Y position
	case YCURSOR:
	{
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
	case XMAX_WINDOW:
	{
		struct winsize w;
		ioctl(0, TIOCGWINSZ, &w);

		return w.ws_col;
	}

	// Console's number of rows -- May be wrong
	case YMAX_WINDOW:
	{
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

#endif
}
