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

void TopHelpBar()
{
	int xs = XSIZE;

	SetColor(fg_color);
	ClearPartial(0, 0, xs, 1);
	printf("%.*s", xs, NEWTRODIT_DIALOG_MANUAL_TITLE);
	SetColor(bg_color);
	return;
}

void BottomHelpBar()
{
	SetColor(fg_color);
	PrintBottomString(NEWTRODIT_DIALOG_MANUAL);
	SetColor(bg_color);
	return;
}

int SetWrapSize()
{
	lineCount ? (wrapSize = XSIZE - 1 - Tab_stack[file_index].linecount_wide) : (wrapSize = XSIZE - 1);

	(wrapSize < 0) ? wrapSize = 0 : wrapSize; // Check if wrapSize is negative
	return wrapSize;
}

int CenterText(char *text, int yps) // Algorithm: (XSIZE / 2) - (len / 2)
{
	SetColor(fg_color);
	int center_text = (XSIZE / 2) - (strlen(text) / 2);
	gotoxy(center_text, yps);
	int pos = wrapSize - strlen(text);
	if (pos < 0)
	{
		pos = abs(pos);
	}

	printf("%.*s", pos, text);
	SetColor(bg_color);
	return center_text;
}

int DisplayTabIndex(File_info *tstack)
{

	if (open_files > 1)
	{
		int pos = (XSIZE / 2) + (strlen(StrLastTok(tstack->filename, PATHTOKENS)) / 2) + (strlen(StrLastTok(tstack->filename, PATHTOKENS)) % 2) + 1; // Center text and add a space
		gotoxy(pos, 0);

		SetColor(fg_color);

		printf("(%d/%d)", file_index + 1, open_files);
		SetColor(bg_color);
	}
	return 1;
}

char *RightAlignNewline()
{
	SetColor(fg_color);

	char *nl_type = (char *)malloc(sizeof(char) * DEFAULT_ALLOC_SIZE);
	memset(nl_type, 0, sizeof(char) * DEFAULT_ALLOC_SIZE);
	if (!strcmp(Tab_stack[file_index].newline, "\n"))
	{
		strncpy_n(nl_type, "Unix (LF)", DEFAULT_ALLOC_SIZE);
	}
	else if (!strcmp(Tab_stack[file_index].newline, "\r\n"))
	{
		strncpy_n(nl_type, "Windows (CR LF)", DEFAULT_ALLOC_SIZE);
	}
	else if (!strcmp(Tab_stack[file_index].newline, "\r"))
	{
		strncpy_n(nl_type, "Mac (CR)", DEFAULT_ALLOC_SIZE);
	}
	else if (!strcmp(Tab_stack[file_index].newline, "\n\r"))
	{
		strncpy_n(nl_type, "Risc OS (LF CR)", DEFAULT_ALLOC_SIZE);
	}
	else if (!strcmp(Tab_stack[file_index].newline, "\025"))
	{
		strncpy_n(nl_type, "IBM Mainframe (NL)", DEFAULT_ALLOC_SIZE);
	}
	else if (!strcmp(Tab_stack[file_index].newline, "\x1e"))
	{
		strncpy_n(nl_type, "QNX pre-POSIX (RS)", DEFAULT_ALLOC_SIZE);
	}
	else if (nl_type[0] == '\0')
	{
		strncpy_n(nl_type, "Unknown newline character", DEFAULT_ALLOC_SIZE);
	}

	gotoxy(XSIZE - strlen(nl_type) - 2, 0); // XSIZE - strlen(nl_type) - 2
	fputs(nl_type, stdout);
	free(nl_type);
	SetColor(bg_color);

	return nl_type;
}

void ShowFindMenu()
{
	SetColor(bg_color);
	ClearPartial(0, YSIZE - 1, XSIZE, 1);

	SetColor(fg_color);
	fputs("F3", stdout);
	SetColor(bg_color);
	fputs(": Next occurrence | ", stdout);
	SetColor(fg_color);
	fputs("F4", stdout);
	SetColor(bg_color);
	fputs(": Toggle case sensitive | ", stdout);
	SetColor(fg_color);
	fputs("ESC", stdout);
	SetColor(bg_color);
	fputs(": Quit", stdout);
}

void ShowBottomMenu()
{
	PrintBottomString(NEWTRODIT_DIALOG_BOTTOM_HELP);
	return;
}

void SetCursorSettings(int visible, int size)
{
#ifdef _WIN32
	HANDLE Cursor = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = size;
	info.bVisible = visible;
	SetConsoleCursorInfo(Cursor, &info);
#else
	/* 	info.dwSize = size;
		info.bVisible = visible; */

	// Set cursor size
	if (size > 50)
	{
		printf("\x1B[\x31 q"); // Blinking block
	}
	else
	{
		printf("\x1B[\x33 q"); // Blinking underline
	}

	// Set cursor visibility
	if (visible)
	{
		printf("\x1B[?25h");
	}
	else
	{
		printf("\x1B[?25l");
	}
#endif
}

void NewtroditNameLoad()
{
	SetColor(0x70);
	ClearPartial(0, 0, XSIZE, 1);
	printf(" Newtrodit %s", newtrodit_version);
	SetColor(0x07);
}

void DisplayCursorPos(int xps, int yps)
{
	int cursorvis = GetConsoleInfo(CURSOR_VISIBLE);
	SetCursorSettings(false, GetConsoleInfo(CURSOR_SIZE));
	size_t len = strlen(NEWTRODIT_DIALOG_BOTTOM_HELP);
	ClearPartial(len, BOTTOM, XSIZE - len, 1);

	printf(longPositionDisplay ? "Line %d, Column %d" : "Ln %d, Col %d", yps, xps + 1); // +1 because it's zero indexed
	SetCursorSettings(cursorvis, GetConsoleInfo(CURSOR_SIZE));
	// ClearPartial(GetConsoleInfo(XCURSOR), BOTTOM, wrapSize - GetConsoleInfo(XCURSOR), 1);
}

void LoadAllNewtrodit()
{
	SetCursorSettings(false, GetConsoleInfo(CURSOR_SIZE)); // Hide cursor to reduce flickering
	SetColor(bg_color);
	SetWrapSize();

	switch (clearAllBuffer)
	{
	case 0:
		ClearScreen();
		break;
	case 1:
		ClearPartial(0, 0, XSIZE, YSIZE);
		break;
	default: // We will be using this when we want a full screen refresh from outside the load function
		break;
	}

	NewtroditNameLoad();
	CenterText(StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), 0);
	DisplayTabIndex(&Tab_stack[file_index]);
	RightAlignNewline();
	ShowBottomMenu();
	if (lineCount)
	{
		DisplayLineCount(Tab_stack, YSIZE - 3, 1);
	}
	SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));

	gotoxy(0, 1);
}

void NewtroditCrash(char *crash_reason, int crash_retval)
{
	signal(SIGSEGV, SIG_DFL); // Reset signal handler to avoid infinite crash loops
	int get_le = 0;

	char *crash_desc;

#ifdef _WIN32
	get_le = GetLastError();

	crash_desc = (LPSTR)GetErrorDescription(get_le);
#else
	get_le = errno;
	crash_desc = (char *)GetErrorDescription(get_le);
#endif
	crash_desc[strcspn(crash_desc, "\r")] = 0;
	LoadAllNewtrodit(); // Just to not have a blank screen and make it scary
	int errno_temp = errno;
	if (!errno_temp)
	{
		errno_temp = crash_retval;
	}
	ClearPartial(0, 1, XSIZE, YSIZE - 1);
	printf("\aNewtrodit ran into a problem and it crashed. We're sorry.\nPlease report this issue to %s/issues\n\nDebug info:\nerrno: 0x%x (%s)\nGetLastError: 0x%x (%s)\nLast known debug information: %s\n\nProgram information:\nVersion: %s\nBuild date: %s\nCommand line arguments:\n", newtrodit_repository, errno_temp, strerror(errno_temp), get_le, crash_desc, last_known_exception, newtrodit_version, newtrodit_build_date);
	for (int i = 0; i < SInf.argc; i++)
	{
		printf("%s ", SInf.argv[i]); // Prints the command line arguments
	}

	printf("\n\nEditing file '%s', line %d, column %d\nTabs open: %d, current tab: %d\n\nReason: %s\n\nPress enter to exit...\n", Tab_stack[file_index].filename, Tab_stack[file_index].ypos, Tab_stack[file_index].xpos + 1, open_files, file_index + 1, crash_reason);
	DisplayCursor(true);
	getchar();
	ExitRoutine(crash_retval); // Don't perform any cleanup, just exit
}

int QuitProgram(int color_quit)
{
	if (Tab_stack[file_index].is_modified) // Second condition should never happen
	{	printf("readyok");

		PrintBottomString(NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);
		if (YesNoPrompt())
		{
			SaveFile(&Tab_stack[file_index]);
		}
	}

	PrintBottomString(NEWTRODIT_PROMPT_QUIT);

	if (YesNoPrompt())
	{
		SetColor(color_quit);
		ClearScreen();
		SetCursorSettings(true, CURSIZE);
		RestoreConsoleBuffer();
		ExitRoutine(0);
	}
	else
	{
		SetColor(bg_color);
	}

	return 0;
}

void UpdateTitle(File_info *tstack)
{
	if (tstack->is_saved && fullPathTitle)
	{
		char *path = tstack->filename;
		tstack->filename = strdup(FullPath(path));
	}
	if (tstack->is_modified)
	{
		SetTitle(join(join("Newtrodit - ", tstack->filename), " (Modified)"));
	}
	else
	{
		SetTitle(join("Newtrodit - ", tstack->filename));
	}
}

void print_line(char *line)
{
#ifdef HORIZONTAL_SCROLL
	if (horizontalScroll < 0)
	{
		horizontalScroll = 0;
	}
#endif

	int remove_chars = strlen(Tab_stack[file_index].newline);
	if (strncmp(line + strlen(line) - strlen(Tab_stack[file_index].newline), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline))) // Not matching
	{
		remove_chars = 0;
	}

	fwrite(line + horizontalScroll, 1, (strlen(line + horizontalScroll) - remove_chars) >= wrapSize ? wrapSize : (strlen(line + horizontalScroll) - remove_chars), stdout); // Should be faster than printf
	if (syntaxHighlighting && !syntaxAfterDisplay)
	{
		color_line(line + horizontalScroll, 0, Tab_stack[file_index].Syntaxinfo.override_color);
	}
}

void ToggleOption(int *option, char *text, int reloadScreen)
{
	PrintBottomString(text);
	*option = !*option;

	if (reloadScreen)
	{
		LoadAllNewtrodit();
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
	}

	*option ? PrintBottomString(join(text, NEWTRODIT_DIALOG_ENABLED)) : PrintBottomString(join(text, NEWTRODIT_DIALOG_DISABLED));
}

int DrawBox(int x, int y, int max_width, int height, int color, int margin)
{
	int old_color = GetConsoleInfo(COLOR);
	SetColor(color);
	ClearPartial(x - margin, y - margin, max_width + 2 * margin, height + 2 * margin);
	SetColor(old_color);
	return x - margin;
}
