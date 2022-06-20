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

#ifdef _WIN32
#include "win32/newtrodit_core_win.h"
#else
#include "linux/newtrodit_core_linux.h"
#endif
#include "newtrodit_syntax.h"

void DisplayLineCount(File_info *tstack, int size, int yps);
int SaveFile(File_info *tstack);
int DisplayFileContent(File_info *tstack, FILE *fstream, int starty);

#include "newtrodit_gui.c"
#ifdef _WIN32
#include "win32/newtrodit_func_win.c"
#else
#include "linux/newtrodit_func_linux.c"
#endif
#include "newtrodit_api.c"

int DownArrow(int man_line_count)
{
	if (man_line_count - (YSIZE - 4) > 0)
	{
		man_line_count = man_line_count - (1 * (YSIZE - 3) - 1);
	}
	else
	{
		man_line_count = man_line_count - (1 * (YSIZE - 3));
	}
	return man_line_count;
}

int VTSettings(bool enabled)
{
#ifdef _WIN32
	DWORD lmode; // Process ANSI escape sequences
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	GetConsoleMode(hStdout, &lmode);
	if (enabled)
	{
		lmode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING & ~DISABLE_NEWLINE_AUTO_RETURN;
	}
	else
	{

		lmode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	}
	return SetConsoleMode(hStdout, lmode);
}

int NewtroditHelp()
{

	_chdir(SInf.location);

	SetConsoleTitle("Newtrodit help");
	SetColor(BG_DEFAULT); // Don't change manual color (Maybe in a future update?)

	CursorSettings(false, GetConsoleInfo(CURSOR_SIZE));
	ClearScreen();

	TopHelpBar();

	FILE *manual = fopen(manual_file, "rb");
	if (!manual)
	{
		PrintBottomString(join(NEWTRODIT_ERROR_MISSING_MANUAL, StrLastTok(manual_file, PATHTOKENS)));
		CursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
		getch_n();
		_chdir(SInf.dir);

		return 1;
	}

	if (CountLines(manual) >= MANUAL_BUFFER_Y)
	{
		PrintBottomString(NEWTRODIT_ERROR_MANUAL_TOO_BIG);
		getch_n();
		_chdir(SInf.dir);

		return 1;
	}
	if (++SInf.manual_open == 42)
	{
		PrintBottomString("The answer for any problem is 42. No need to ask me."); // Easter egg.
		getch_n();
		_chdir(SInf.dir);

		return 0;
	}


	if (!VTSettings(true) && !RGB24bit) // Process ANSI escape sequences and check if the console supports ANSI escape sequences
	{
		PrintBottomString(NEWTRODIT_ERROR_LOADING_MANUAL);
		getch_n();
		_chdir(SInf.dir);

		return 1;
	}

#define reset_color "\x1b[0m"

	int quit_manual = 0, end_manual = 0, max_manual_lines = 0;
	int man_line_count = 0, disable_clear = 0;

	int temp_escape = 0;
	int manual_ch;
	char escape_char = '$';

	char **manual_buf = calloc(MANUAL_BUFFER_Y, MANUAL_BUFFER_X); // Allocate 1000 char pointers
	for (int i = 0; i < MANUAL_BUFFER_X; ++i)
	{
		manual_buf[i] = malloc(MANUAL_BUFFER_Y); // Allocate 350 bytes for each string
	}

	char *gotoline_man = (char *)malloc(MANUAL_BUFFER_X);

	while (fgets(manual_buf[man_line_count], MANUAL_BUFFER_X, manual)) // Load manual into memory
	{
		if (man_line_count == 0 && strncmp(NEWTRODIT_MANUAL_MAGIC_NUMBER, manual_buf[man_line_count], strlen(NEWTRODIT_MANUAL_MAGIC_NUMBER)))
		{
			PrintBottomString(join(NEWTRODIT_ERROR_INVALID_MANUAL, manual_file));
			getch_n();
			VTSettings(false);
			_chdir(SInf.dir);

			return 1;
		}
		if (man_line_count + 1 >= MANUAL_BUFFER_Y)
		{
			PrintBottomString(NEWTRODIT_ERROR_MANUAL_TOO_BIG);
			getch_n();
			VTSettings(false);
			_chdir(SInf.dir);

			return 1;
		}
		man_line_count++;
		max_manual_lines++;
	}

	man_line_count = 1; // Reset line count

	while (quit_manual == 0)
	{
		if (end_manual == 1)
		{
			quit_manual = 1;
		}
		if (manual_ch != 0)
		{
			TopHelpBar();
			BottomHelpBar();
			for (int i = 1; i < YSIZE - 2; ++i)
			{
				SetColor(BG_DEFAULT);
				if (man_line_count <= 0)
				{
					man_line_count = 1;
				}
				if (man_line_count <= max_manual_lines)
				{
					gotoxy(0, i);
					manual_buf[man_line_count][strcspn(manual_buf[man_line_count], "\n")] = 0;

					if (strrchr(manual_buf[man_line_count], escape_char) != 0)
					{
						for (int k = 0; k < strlen(manual_buf[man_line_count]); ++k)
						{
							if (manual_buf[man_line_count][k] == escape_char)
							{
								switch (manual_buf[man_line_count][k + 1])
								{
								case 'Y':
									printf("%d", BUFFER_Y);
									k++;
									break;
								case 'X':
									printf("%d", BUFFER_X);
									k++;
									break;
								case 'I':
									printf("%d", BUFFER_INCREMENT);
									k++;
									break;
								case 'C':
									printf("%s", newtrodit_commit);
									k++;
									break;
								case 'B':
									printf("%s", newtrodit_build_date);
									k++;
									break;
								case 'V':
									printf("%s", newtrodit_version);
									k++;
									break;
								case 'G':
									printf("%s", newtrodit_repository);
									k++;
									break;
								default:
									printf("%c", manual_buf[man_line_count][++k]);
								}
							}
							else
							{
								putchar(manual_buf[man_line_count][k]);
							}
						}
					}
					else
					{
						printf("%.*s\n", MANUAL_BUFFER_X, manual_buf[man_line_count]);
					}

					man_line_count++;
					printf("%s", reset_color); // Clear all text attributes
				}
				else
				{
					printf("%s", reset_color);

					man_line_count = max_manual_lines - (YSIZE - 4);
					disable_clear = false;
					quit_manual = 1;
				}
			}
		}

		if (quit_manual == 0)
		{
			manual_ch = getch();
			switch (manual_ch)
			{
			case 24: // ^X
				CursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
				VTSettings(false);
				_chdir(SInf.dir);

				return 0;
				break;
			case 27: // ESC
				CursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
				VTSettings(false);
				_chdir(SInf.dir);

				return 0;
				break;
			case 0:

				manual_ch = getch();
				if (manual_ch == 107) // A-F4
				{
					QuitProgram(SInf.color);
					printf("%s", reset_color); // Clear all text attributes
					BottomHelpBar();
					manual_ch = 0;
					break;
				}
				if (manual_ch == 59) // F1
				{
					CursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
					VTSettings(false);
					for (int i = 0; i < MANUAL_BUFFER_Y; i++)
					{
						free(manual_buf[i]);
					}
					_chdir(SInf.dir);

					return 0;
				}

				break;
			case 13: // Enter
				printf("%s", reset_color);
				man_line_count = DownArrow(man_line_count);
				break;
			case 224:
				manual_ch = getch();
				switch (manual_ch)
				{
				case 73: // PageUp
					printf("%s", reset_color);

					if ((man_line_count - (YSIZE - 3)) > (YSIZE - 3))
					{
						man_line_count -= (2 * (YSIZE - 3)); // Multiplied by 2
					}
					else
					{
						man_line_count = 1; // Set manual position to first line
					}

					break;
				case 71: // HOME key
					printf("%s", reset_color);

					ClearPartial(0, 1, XSIZE, YSIZE - 2);
					man_line_count = 1;
					disable_clear = true;
					break;
				case 119: // ^HOME key
					printf("%s", reset_color);

					man_line_count = 1;
					disable_clear = true;
					break;

				case 79: // END key
					printf("%s", reset_color);

					ClearPartial(0, 1, XSIZE, YSIZE - 2);
					man_line_count = max_manual_lines - (YSIZE - 4);
					break;
				case 117: // ^END key
					printf("%s", reset_color);
					ClearPartial(0, 1, XSIZE, YSIZE - 2);
					man_line_count = max_manual_lines - (YSIZE - 4);
					break;

				case 72: // Up arrow
					printf("%s", reset_color);
					if ((man_line_count - (1 * (YSIZE - 3) + 1)) > 0)
					{
						man_line_count = man_line_count - (1 * (YSIZE - 3) + 1);
					}
					else
					{
						man_line_count = man_line_count - (1 * (YSIZE - 3));
						disable_clear = true;
					}
					break;
				case 80: // Down arrow
					printf("%s", reset_color);
					man_line_count = DownArrow(man_line_count);
					break;
				default:
					// man_line_count = man_line_count - (1 * (YSIZE - 3));

					break;
				}
				break;
			case 7: // ^G
				PrintBottomString(NEWTRODIT_PROMPT_GOTO_LINE);

				gotoline_man = TypingFunction('0', '9', strlen(itoa_n(MANUAL_BUFFER_Y)));
				if (atoi(gotoline_man) < 0 || atoi(gotoline_man) > max_manual_lines || atoi(gotoline_man) >= MANUAL_BUFFER_Y) // Line is less than 1
				{
					PrintBottomString(NEWTRODIT_ERROR_INVALID_YPOS);
					getch_n();
					man_line_count -= -(1 * (YSIZE - 3));
				}
				else
				{
					if ((man_line_count > 0 - (YSIZE + 2)))
					{
						man_line_count = atoi(gotoline_man) - (1 * (YSIZE - 3) + 1);
					}
				}
				ClearPartial(0, 1, XSIZE, YSIZE - 2);
				break;
			default:

				break;
			}
		}

		if (manual_ch != 0)
		{
			if (disable_clear == 1)
			{
				disable_clear = 0;
			}
			else
			{
				ClearPartial(0, 1, XSIZE, YSIZE - 2);
			}
		}
	}
	for (int i = 0; i < MANUAL_BUFFER_Y; i++)
	{
		free(manual_buf[i]);
	}
	CursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
	VTSettings(false);
	_chdir(SInf.dir);

	return 0;
}
