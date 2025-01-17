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

int CountLines(FILE *fp)
{
	int n = 0, c;
	while ((c = getc(fp)) != EOF)
	{
		if (c == '\n')
		{
			n++;
		}
	}
	fseek(fp, 0, SEEK_SET);
	return n;
}

int CountBufferLines(File_info *tstack)
{
	int lines = 0;
	long long filesize = 0;
	for (int i = 1; i < tstack->bufy; ++i)
	{
		if (tstack->strsave[i][0] == '\0')
		{
			break;
		}
		lines++; // Count lines

		filesize += strlen(tstack->strsave[i]);
	}
	if (!strncmp(Tab_stack[file_index].strsave[lines] + (strlen(Tab_stack[file_index].strsave[lines]) - strlen(Tab_stack[file_index].newline)), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
	{
		lines++;
	}
	tstack->linecount = lines;
	tstack->size = filesize;
	return lines;
}

void DisplayLineCount(File_info *tstack, int size, int disp)
{
	if (lineCount)
	{

		while (strlen(itoa_n(disp)) > (lineCount ? (tstack->linecount_wide) : 0))
		{
			(tstack->linecount_wide)++;
		}

		if (tstack->bufy < tstack->ypos && tstack->strsave[tstack->ypos + 1][0] == '\0' && disp <= (YSIZE - 3) && strncmp(tstack->strsave[tstack->ypos] + NoLfLen(tstack->strsave[tstack->ypos]), tstack->newline, strlen(tstack->newline)) != 0)
		{
			ClearPartial(0, disp + 1, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);
		}
		SetColor(linecount_color);

		if (linecountHighlightLine)
		{

			ClearPartial(0, tstack->last_dispy, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);
			printf("%d", tstack->last_y);

			tstack->last_dispy = tstack->display_y;
			tstack->last_y = tstack->ypos;
			SetColor(linecount_highlight_color);
		}
		ClearPartial(0, disp, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);

		printf("%d", tstack->ypos);

		SetColor(bg_color);
	}
}

void LoadLineCount(File_info *tstack, int startpos, int starty)
{
	if (lineCount)
	{

		int n = YSIZE;
		int lines_load = 0;

		// 5 and 7 are the optimal values for the better performance.
		// Algorithm tested successfully with 5 and 7 giving the best performance results.
		int skipamount = 5;
		(n > 45) ? (skipamount = 7) : (skipamount = 5);

		if (tstack->ypos >= (n - 2))
		{
			startpos = tstack->ypos - (n - 2);
		}
		else
		{
			startpos = 0;
		}

		while (strlen(itoa_n(startpos + n - 1)) >= (lineCount ? (tstack->linecount_wide) : 0) - 1)
		{
			tstack->linecount_wide++;
		}

		for (int k = startpos + 1; k < startpos + (n - 1); ++k)
		{
			if (k + skipamount < tstack->bufy && tstack->strsave[k + skipamount][0] != '\0' && k + skipamount < startpos + (n - 1)) // Try to skip 'skipamount' lines for optimization
			{
				k += skipamount; // Instead of reading counting line by line, try to skip 5 or 7 lines
				lines_load += skipamount + 1;
			}
			else if (tstack->strsave[k][0] != '\0' || (!strncmp(tstack->strsave[k - 1] + NoLfLen(tstack->strsave[k - 1]), tstack->newline, strlen(tstack->newline))))
			{
				lines_load++;
			}
		}
		SetColor(0x80);

		ClearPartial(0, 1, (lineCount ? (tstack->linecount_wide) : 0) - 1, lines_load); // Clear the line count area
		for (int i = 1; i <= lines_load; i++)											// Print line numbers
		{
			if (lines_load % 2 == 0)
			{
				printf("%d\n%d\n", startpos + i, startpos + i + 1); // Try to print 2 line numbers every time
				i++;
			}
			else
			{
				printf("%d\n", startpos + i);
			}
		}
		SetColor(bg_color);
	}
}

int WriteBuffer(FILE *fstream, File_info *tstack)
{
	if (tstack->utf8)
	{
#if _NEWTRODIT_OLD_SUPPORT == 0
		wchar_t *wstr = malloc(BUFFER_X * sizeof(wchar_t));
		size_t sz;
		for (int i = 1; i < BUFFER_Y; ++i)
		{
			if ((sz = strlen(Tab_stack[file_index].strsave[i])) < BUFFER_X * sizeof(wchar_t))
			{
				sz = BUFFER_X * sizeof(wchar_t);
			}
			mbstowcs(wstr, Tab_stack[file_index].strsave[i], sz);
			if (fputws(wstr, fstream) == EOF)
			{
				return 0;
			}
		}
#endif
	}
	else
	{
		for (int i = 1; i < BUFFER_Y; ++i)
		{
			if (fputs(Tab_stack[file_index].strsave[i], fstream) == EOF)
			{
				return 0;
			}
		}
	}

	return 1;
}

int DisplayFileContent(File_info *tstack, FILE *fstream, int starty)
{
	SetCursorSettings(false, GetConsoleInfo(CURSOR_SIZE));

	if (lineCount)
	{
		LoadLineCount(tstack, tstack->ypos, starty);
	}
	int startpos = 0;
	int window_size = YSIZE;

	if (tstack->ypos >= (window_size - 2))
	{
		startpos = tstack->ypos - (window_size - 2);
	}
	/*
	 Don't confuse 'startpos' with 'starty'

	'starty' is for the optimizations, 'startpos' is for the display
	*/

	if (starty < 0)
	{
		starty = 0;
	}
	syntaxAfterDisplay = true;
	for (int i = 1 + startpos + starty; i < startpos + (window_size - 1); i++)
	{
		if (tstack->strsave[i][0] != '\0')
		{
			gotoxy((lineCount ? (tstack->linecount_wide) : 0), i - startpos);
			print_line(tstack->strsave[i]);
		}
	}
	if (syntaxHighlighting && syntaxAfterDisplay)
	{
		syntaxAfterDisplay = false;
		for (int i = 1 + startpos + starty; i < startpos + (window_size - 1); i++)
		{
			if (tstack->strsave[i][0] != '\0')
			{
				gotoxy((lineCount ? (tstack->linecount_wide) : 0), i - startpos);
				color_line(tstack->strsave[i], 0, tstack->Syntaxinfo.override_color);
			}
		}
	}
	SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));

	return 0;
}

int ValidFileName(char *filename)
{
	return strpbrk(filename, "/") == NULL;
}

int ValidString(char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (!isprint(str[i]))
		{
			return 0;
		}
	}
	return 1;
}

void SetDisplayCursorPos(File_info *tstack)
{
	if (tstack->xpos <= wrapSize)
	{
		if (tstack->xpos > YSIZE - 1)
		{
			gotoxy(tstack->xpos + (lineCount ? (tstack->linecount_wide) : 0), tstack->display_y); // Relative position is for tab key
		}
		else
		{
			gotoxy(tstack->xpos + (lineCount ? (tstack->linecount_wide) : 0), tstack->display_y);
		}
	}
	else
	{
		gotoxy(wrapSize + (lineCount ? (tstack->linecount_wide) : 0), tstack->display_y);
	}
}

int SaveFile(File_info *tstack)
{
	char *tmp_filename = malloc(sizeof(char) * MAX_PATH);
	if (tstack->is_untitled)
	{
		PrintBottomString(NEWTRODIT_PROMPT_SAVE_FILE);
		fgets(tmp_filename, MAX_PATH, stdin); // Can't use sizeof filename because it's a pointer

		if (NoLfLen(tmp_filename) <= 0)
		{
			LoadAllNewtrodit();
			DisplayFileContent(tstack, stdout, 0); // Display file content on screen
			PrintBottomString(NEWTRODIT_FUNCTION_ABORTED);
			getch_n();
			ShowBottomMenu();
			return 0;
		}
		tmp_filename[strcspn(tmp_filename, "\n")] = 0; // Remove newline character

		RemoveQuotes(tmp_filename, strdup(tmp_filename));

		if (!ValidFileName(tmp_filename))
		{
			LoadAllNewtrodit();
			DisplayFileContent(tstack, stdout, 0);
			PrintBottomString(join(NEWTRODIT_FS_FILE_INVALID_NAME, tmp_filename));
			last_known_exception = NEWTRODIT_FS_FILE_INVALID_NAME;

			getch_n();
			return 0;
		}
		tstack->filename = strdup(tmp_filename);
	}

	FILE *fp = fopen(tstack->filename, "wb");

	WriteBuffer(fp, tstack);

	LoadAllNewtrodit();
	DisplayFileContent(tstack, stdout, 0);

	if (fp)
	{
		tstack->is_modified = false;
		tstack->is_untitled = false;
		tstack->is_saved = true;
		PrintBottomString(NEWTRODIT_FILE_SAVED);
	}
	else
	{
		PrintBottomString(NEWTRODIT_FS_FILE_SAVE_ERR);
		last_known_exception = NEWTRODIT_FS_FILE_SAVE_ERR;
	}
	fclose(fp);

	getch_n();
	ShowBottomMenu();

	return 1;
}

char *extension_filetype(char *filename)
{
	char *extension = "File";
	char *ptr;
	if (!strpbrk(filename, "."))
	{
		return extension;
	}
	extension = StrLastTok(Tab_stack[file_index].filename, "."); // Get the file extension
	for (size_t i = 0; i < sizeof(FileLang) / sizeof(FileLang[0]); i++)
	{
		for (size_t k = 0; k < FileLang[i].extcount; k++)
		{

			ptr = strtok_n(FileLang[i].extensions, "|");
			if (FileLang[i].extcount == 1)
			{
				ptr = FileLang[i].extensions;
			}
			while (ptr != NULL)
			{

				if (!strcmp(extension, ptr))
				{
					return FileLang[i].display_name;
				}

				ptr = strtok_n(ptr, "|");
			}
		}
	}
	return (char *)"Unknown file type";
}

int LoadFile(File_info *tstack, char *filename, FILE *fpread)
{
	WriteLogFile(join("Loading file: ", filename));

	if (!trimLongLines)
	{
		fseek(fpread, 0, SEEK_END);
		if ((int)ftell(fpread) >= BUFFER_X * BUFFER_Y)
		{
			fseek(fpread, 0, SEEK_SET);
			last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
			WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, filename));

			return -EFBIG;
		}
	}
	if (!fpread)
	{
		last_known_exception = NEWTRODIT_FS_ACCESS_DENIED;
		WriteLogFile(join(NEWTRODIT_FS_ACCESS_DENIED, filename));

		return -EACCES;
	}
	fseek(fpread, 0, SEEK_SET);
	int line_count = 0;
	if ((line_count = CountLines(fpread)) >= BUFFER_Y)
	{
		WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, filename));

		return -EFBIG;
	}
	tstack->linecount_wide = LINECOUNT_WIDE;
	if (strlen(itoa_n(line_count)) > strlen(itoa_n(YSIZE - 2)))
	{
		tstack->linecount_wide = LINECOUNT_WIDE + strlen(itoa_n(line_count)) - strlen(itoa_n(YSIZE - 2));
	} // If the line count is too long, increase the width of the line count

	for (int i = 1; i < BUFFER_Y; i++)
	{
		memset(tstack->strsave[i], 0, BUFFER_X);
	}

	BUFFER_X = DEFAULT_BUFFER_X;

	char *allocate_buf;
	int chr = 0, read_x = 0, read_y = 1;
	size_t mb_conv_len;

#if _NEWTRODIT_OLD_SUPPORT == 0
	wchar_t wc = 0;
	mbstate_t mbs = {0};
	char wchar_buf[MIN_BUFSIZE];
	mbrlen(NULL, 0, &mbs);
#else
	int wc;
#endif

	while ((!tstack->utf8 && (chr = getc(fpread)) != EOF) || (_NEWTRODIT_OLD_SUPPORT && tstack->utf8 && (wc = getwc(fpread)) != WEOF))
	{
		if (tstack->utf8)
		{
#if _NEWTRODIT_OLD_SUPPORT == 0
			mb_conv_len = wcrtomb(wchar_buf, wc, &mbs);
			if (errno == EILSEQ)
			{
				last_known_exception = NEWTRODIT_ERROR_INVALID_UNICODE_SEQUENCE;
				WriteLogFile(join(NEWTRODIT_ERROR_INVALID_UNICODE_SEQUENCE, filename));

				return -EILSEQ;
			}
#endif
		}
		else
		{
			mb_conv_len = 1;
		}
		for (int i = 0; i < mb_conv_len; i++)
		{
			if (tstack->utf8)
			{
#if _NEWTRODIT_OLD_SUPPORT == 0
				chr = wchar_buf[i];
#endif
			}
			if (bpsPairHighlight)
			{
				if (chr == '(' || chr == '[' || chr == '(')
				{
					tstack->Syntaxinfo.bracket_pair_count++;
				}
				else if (chr == ')' || chr == ']' || chr == ')')
				{
					if (tstack->Syntaxinfo.bracket_pair_count > 0)
					{
						tstack->Syntaxinfo.bracket_pair_count--;
					}
				}
			}

			if (chr == 0)
			{
				if (convertNull)
				{
					chr = 32;
				}
			}
			if (chr == 13 && strncmp(tstack->newline, "\r\n", strlen(tstack->newline)))
			{
				continue;
			}
			if (chr == 10) // LF / CRLF when using Windows newline format
			{
				tstack->strsave[read_y][read_x] = 10; // LF

				read_x = 0;
				read_y++;
			}
			else
			{

				if (chr == 9)
				{
					if (convertTabtoSpaces) // Convert tab to spaces
					{
						for (int i = 0; i < TAB_WIDE; i++) // TAB_CHANGE: Change this in a future
						{
							tstack->strsave[read_y][read_x++] = ' ';
						}
					}
					else
					{
						tstack->strsave[read_y][read_x++] = 9;
					}
				}
				else
				{
					tstack->strsave[read_y][read_x++] = chr;
				}
			}
			if (read_x > BUFFER_X)
			{
				tstack->strsave[read_y][read_x] = '\0'; // Terminate string
				if (!trimLongLines)
				{
					return -EFBIG;
				}
				else
				{

					read_x = 0;
					read_y++;
				}
			}
			if (read_y > BUFFER_Y)
			{
				if (allocateNewBuffer)
				{

					allocate_buf = realloc(tstack->strsave, BUFFER_Y + BUFFER_INCREMENT_LOADFILE);
					BUFFER_Y += BUFFER_INCREMENT_LOADFILE;
					if (!allocate_buf)
					{
						last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
						WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, filename));

						return -EFBIG;
					}
				}
				else
				{
					last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
					WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, filename));

					return -EFBIG;
				}
			}
		}
	}
	tstack->xpos = 0;
	tstack->ypos = 1;
	tstack->is_loaded = true;
	tstack->filename = filename;
	tstack->linecount = read_y;
	tstack->is_untitled = false;
	tstack->is_saved = true;
	tstack->is_modified = false;
	tstack->is_readonly = false; // We'll check if it's read-only later
	tstack->language = extension_filetype(tstack->filename);

	// Get last write time of a file
	struct stat attr;
	stat(tstack->filename, &attr);
	// TODO: Check if the file was successfully opened
	tstack->fread_time = attr.st_mtime;
	tstack->fwrite_time = attr.st_mtime;
	// TODO: Check if a file is readonly

	fclose(fpread);
	WriteLogFile(join("Successfully loaded the file ", tstack->filename));

	return 1;
}

int NewFile(File_info *tstack) // ^N = New file
{

	if (open_files < MAX_TABS)
	{
		if (file_index + 1 < open_files)
		{
			for (int i = open_files; i < file_index; i--) // Shift all struct pointers to the right
			{

				memmove((void *)&tstack[i + 1], (void *)&tstack[i], sizeof tstack[i]);
			}
			/* {
			} */
		}
		file_index = open_files;
		tstack++;
		open_files++;
	}
	else
	{
		PrintBottomString(NEWTRODIT_ERROR_TOO_MANY_FILES_OPEN);
		last_known_exception = NEWTRODIT_ERROR_TOO_MANY_FILES_OPEN;
		return -EMFILE;
	}

	old_open_files[oldFilesIndex] = tstack->filename;
	BUFFER_X = DEFAULT_BUFFER_X;

	if (!AllocateBufferMemory(tstack))
	{
		last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;

		return -ENOMEM;
	}
	LoadAllNewtrodit();
	DisplayCursorPos(tstack);
	SetDisplayCursorPos(tstack);
	UpdateTitle(tstack);

	return 1;
}

int CloseFile(File_info *tstack)
{
	if (tstack->is_modified)
	{
		PrintBottomString(NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);
		if (YesNoPrompt())
		{
			SaveFile(tstack);
		}
		tstack->is_modified = false;
	}

	if (open_files > 1)
	{
		for (int i = file_index; i < open_files; i++)
		{
			memcpy((void *)&tstack[i], (void *)&tstack[i + 1], sizeof tstack[i]);
		}
		// Empty the last element
		memset((void *)&tstack[open_files], 0, sizeof tstack[open_files]);

		free((void *)&tstack[open_files]);
		open_files--;
	}

	LoadAllNewtrodit();
	return 1;
}

void ReloadFile(File_info *tstack, FILE *fstream)
{
	int n = 0;
	LoadAllNewtrodit();
	if (!tstack->is_untitled)
	{
		if ((n = LoadFile(tstack, tstack->filename, fstream)) != 1)
		{

			PrintBottomString(ErrorMessage(abs(n), tstack->filename)); // Errors are always negative, so abs() is used
			getch_n();
			ShowBottomMenu();

			return;
		}
		else
		{
			DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		}
	}
	else
	{
		DisplayFileContent(tstack, stdout, 0);
	}
	if (tstack->strsave[tstack->ypos][tstack->xpos] == '\0')
	{
		tstack->xpos = 0;
		tstack->ypos = 1;
	}
	PrintBottomString(NEWTRODIT_FILE_RELOADED);
	getch_n();
	ShowBottomMenu();
	DisplayCursorPos(tstack);
	return;
}

void FunctionAborted(File_info *tstack)
{
	LoadAllNewtrodit();
	DisplayFileContent(tstack, stdout, 0); // Display file content on screen
	PrintBottomString(NEWTRODIT_FUNCTION_ABORTED);
	getch_n();
	ShowBottomMenu();
	return;
}

int UpdateScrolledScreen(File_info *tstack)
{
	if (tstack->ypos >= (YSIZE - 3) || horizontalScroll > 0)
	{
		ClearPartial((lineCount ? (tstack->linecount_wide) : 0), 1, XSIZE - (lineCount ? (tstack->linecount_wide) : 0), YSCROLL);
		// ClearPartial(LINECOUNT_WIDE - 1 > 0 ? LINECOUNT_WIDE - 1 : LINECOUNT_WIDE, 1, XSIZE - (LINECOUNT_WIDE - 1 > 0 ? LINECOUNT_WIDE - 1 : LINECOUNT_WIDE), YSCROLL); // Check if the linecount is too short
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		return 1;
	}
	return 0;
}
int UpdateHomeScrolledScreen(File_info *tstack)
{
	if (tstack->ypos >= (YSIZE - 3))
	{
		tstack->ypos = 1;
		ClearPartial(0, 1, XSIZE, YSCROLL);
		DisplayFileContent(tstack, stdout, 0);
		return 1;
	}
	return 0;
}

char *TypingFunction(int min_ascii, int max_ascii, int max_len)
{
	int cursor_visible = GetConsoleInfo(CURSOR_VISIBLE);
	SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
	int chr = 0, index = 0;
	char *num_str = calloc(max_len + 1, sizeof(char));
	int startx = GetConsoleInfo(XCURSOR), starty = GetConsoleInfo(YCURSOR), orig_cursize = GetConsoleInfo(CURSOR_SIZE);
	bool overwrite_mode = false;

	while (chr != ENTER) // Loop while enter isn't pressed
	{
		chr = getch_n();
		if (chr == ESC)
		{
			break;
		}
		if (chr == BS) // Backspace
		{
			if (index > 0)
			{
				DeleteChar(num_str, --index);
				ClearPartial(startx, starty, (startx + strlen(num_str)) >= XSIZE ? (XSIZE - startx) : startx + strlen(num_str), 1);
				fputs(num_str, stdout);
				gotoxy(startx + index, starty);
			}
			continue;
		}

		if (chr & BIT_ESC0)
		{
			switch (chr & ~(BIT_ESC0))
			{
			case ALTF4:

				QuitProgram(SInf.color);
				break;
			default:
				break;
			}
		}
		if (chr & BIT_ESC224) // Special keys: 224 (0xE0)
		{
			switch (chr & (~BIT_ESC224))
			{
			case LEFT:
				if (index > 0)
				{
					putchar('\b');
					index--;
				}
				break;
			case RIGHT:
				if (index < max_len && num_str[index] != '\0')
				{
					putchar(num_str[index++]);
				}
				break;
			case DEL:
				if (index < max_len)
				{
					DeleteChar(num_str, index);
					ClearPartial(startx, starty, (startx + strlen(num_str)) >= XSIZE ? (XSIZE - startx) : startx + strlen(num_str), 1);
					fputs(num_str, stdout);
					gotoxy(startx + index, starty);
				}
				break;
			case INS:
				overwrite_mode = !overwrite_mode;
				SetCursorSettings(true, overwrite_mode ? CURSIZE_INS : CURSIZE);

				break;
			default:
				break;
			}
			continue;
		}
		if (chr >= min_ascii && chr <= max_ascii && chr != 0 && ((!overwrite_mode && (strlen(num_str) < max_len && index <= max_len)) || (overwrite_mode && (strlen(num_str) <= max_len && index < max_len)))) // Check if character is a between the range
		{
			if (overwrite_mode)
			{
				num_str[index++] = chr;
				putchar(chr);
			}
			else
			{
				num_str = InsertChar(strdup(num_str), chr, index++);
				gotoxy(startx, starty);
				fputs(num_str, stdout);
				gotoxy(startx + index, starty);
			}

			WriteLogFile(num_str);
		}
		else
		{
			if (chr != ENTER)
			{
				putchar('\a');
			}
		}
	}
	SetCursorSettings(cursor_visible, orig_cursize);
	return num_str;
}

int AutoIndent(File_info *tstack)
{
	if (autoIndent)
	{
		int tablen = strspn(tstack->strsave[tstack->ypos - 1], convertTabtoSpaces ? " " : "\t");
		tablen -= tablen % TAB_WIDE; // Make it a multiple of TAB_WIDE
		size_t increment_tab = tablen / (convertTabtoSpaces ? 1 : TAB_WIDE);

		size_t linelen = 0;
		char *tmpbuf = NULL;
		if (tstack->strsave[tstack->ypos][0] != '\0')
		{
			linelen = strlen(tstack->strsave[tstack->ypos]);
			tmpbuf = calloc(sizeof(char), linelen + 1);
			memcpy(tmpbuf, tstack->strsave[tstack->ypos], linelen);
		}
		char *tab_buf = calloc(sizeof(char), increment_tab);
		memset(tab_buf, convertTabtoSpaces ? ' ' : '\t', increment_tab);
		memcpy(tstack->strsave[tstack->ypos], tab_buf, increment_tab);
		memcpy(tstack->strsave[tstack->ypos] + increment_tab, tmpbuf, linelen);
		if (tmpbuf)
		{
			free(tmpbuf);
		}
		free(tab_buf);
		SetDisplayY(tstack);
		RefreshLine(tstack, tstack->ypos, tstack->display_y, true);

		return increment_tab;
	}
	return 0;
}

signed long long FileCompare(char *file1, char *file2) // Compare files up to 8 EB in size
{
	if (!ValidFileName(file1) || !ValidFileName(file2))
	{
		last_known_exception = NEWTRODIT_FS_FILE_INVALID_NAME;

		return -EINVAL - 1;
	}
	FILE *f1 = fopen(file1, "rb");
	FILE *f2 = fopen(file2, "rb");
	if (!f1 || !f2)
	{
		fclose(f1);
		fclose(f2);
		// Convert errno to negative number
		if (!f1)
		{
			return (signed long long)(abs(errno) * -1) - 1;
		}
		else
		{
			return (signed long long)(abs(errno) * -1) - 256;
		}
	}

	fseek(f1, 0, SEEK_SET);
	fseek(f2, 0, SEEK_SET);

	long long comp_index = 0;
	PrintBottomString(NEWTRODIT_LOADING_COMPARING_FILES);

	while (!feof(f1) && !feof(f2))
	{
		if (getc(f1) != getc(f2))
		{
			fclose(f1);
			fclose(f2);
			return comp_index;
		}
		++comp_index;
	}
	fclose(f1);
	fclose(f2);
	return (signed long long)-1;
}

int InsertNewRow(File_info *tstack, int *xps, int *yps, int dispy, int size, char *newline)
{
	int n = *yps; // Save old y
	InsertRow(tstack->strsave, *yps, size, NULL);
	(*yps)++;

	if (BufferLimit(tstack)) // Don't overflow
	{
		*yps = n;
		ShowBottomMenu();
		return 1;
	}

	// Allocate the memory for the new line

	tstack->strsave[*yps] = calloc(size, sizeof(char));

	// Copy all characters starting from xpos from the old line to the new line

	strncpy_n(tstack->strsave[*yps], tstack->strsave[(*yps) - 1] + *xps, size);
	tstack->strsave[(*yps) - 1][*xps] = '\0';
	strncat(tstack->strsave[(*yps) - 1], newline, strlen(newline));
	ClearPartial((lineCount ? (tstack->linecount_wide) : 0), dispy, XSIZE - (lineCount ? (tstack->linecount_wide) : 0), YSIZE - dispy - 1); // I should optimize this
	if (Tab_stack[file_index].display_y > YSIZE - 3)
	{
		ClearPartial(0, 1, XSIZE, YSCROLL);
	}
	DisplayFileContent(tstack, stdout, 0);
	return *yps;
}

int LocateFiles(int show_dir, char *file, int startpos)
{
	/*
	int n = startpos, total = startpos;

	bool isWildcard = false;
	HANDLE hFindFiles;
	WIN32_FIND_DATA FindFileData;
	int ys = YSIZE;
	int max_print = wrapSize;
	char *search_pattern = "*";
	// Pull current system time

	time_t now;
	struct tm *tm;
	now = time(0);
	if (!(tm = localtime(&now)))
	{
		printf("Error extracting system time\n");
		return 1;
	}

	if (strpbrk(file, "*?"))
	{
		search_pattern = strdup(file); // Create a duplicate if needed
		isWildcard = true;
	}
	else
	{
		if (!ValidFileName(file))
		{
			last_known_exception = NEWTRODIT_FS_FILE_INVALID_NAME;

			PrintBottomString(join(NEWTRODIT_FS_FILE_INVALID_NAME, file));
			return 0;
		}
	}
	char *dir_tmp = calloc(MAX_PATH * 2, sizeof(char));
	char *out_dir = calloc(MAX_PATH * 2, sizeof(char));
	GetCurrentDirectory(MAX_PATH, dir_tmp);

	if (!isWildcard && get_path_directory(file, out_dir))
	{
		chdir(out_dir);
		file = strlasttok(strdup(file), PATHTOKENS);
	}
		// Get current working directory
	char cwd[MAX_PATH];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
	prinf("Error extracting current working directory\n");
  }
	printf("Current directory: %s\n", cwd);
	if ((hFindFiles = FindFirstFile(search_pattern, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hFindFiles, &FindFileData))
		{
			if (FindString(FindFileData.cFileName, file) != -1 || isWildcard) // Ignore the search result if wildcards are used
			{
				if (n < (ys - 3) + startpos)
				{
					FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &st);
					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && show_dir)
					{
						// Time format is "DD/MM/YYYY hh:mm:ss"
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t<DIR>\t%14lu\t%.*s\n",
								tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
								tm->tm_hour, tm->tm_minute, tm->tm_second,
								(FindFileData.nFileSizeHigh * (MAXDWORD+1)) + FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
					}
					else
					{
						#ifdef _WIN32
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t\t%14lu\t%.*s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, (FindFileData.nFileSizeHigh * (MAXDWORD+1)) + FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
						#endif
					}
					n++;
				}
				total++;
			}
		}
	}
	if (!total)
	{
		PrintBottomString(join(NEWTRODIT_FS_FILE_NOT_FOUND, file));
		last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;
		chdir(dir_tmp);
		return 0;
	}
	else
	{
		PrintBottomString(join(join(join(join("Showing ", itoa_n(n)), " of "), itoa_n(total)), NEWTRODIT_FS_FOUND_FILES));
	}
	chdir(dir_tmp);
	free(dir_tmp);
	free(out_dir);
	return 0;
	*/
}

int ReturnFindIndex(int insensitive, char *str, char *find)
{
	int find_string_index = -1;
	if (insensitive)
	{
		find_string_index = FindString(strlwr(str), strlwr(strdup(find)));
	}
	else
	{
		find_string_index = FindString(str, find);
	}
	return find_string_index;
}

int SetDisplayY(File_info *tstack)
{
	if (tstack->ypos >= YSCROLL)
	{
		tstack->scrolled_y = true;
		tstack->display_y = YSCROLL;
	}
	else
	{
		tstack->scrolled_y = false;
		tstack->display_y = tstack->ypos;
	}
	return tstack->display_y;
}

int SetYFromDisplayY(File_info *tstack, int disp_y) // This function is actually unused for the Linux version (for now)
{
	// Get the line number from the display line number
	int return_y = 0;
	if (tstack->scrolled_y)
	{

		return_y = tstack->ypos - (tstack->display_y - disp_y);
		if (return_y < YSCROLL)
		{
			tstack->scrolled_y = false;
		}
	}
	else
	{
		return_y = disp_y;
	}
	if (return_y < 1)
	{
		return_y = 1;
	}

	tstack->ypos = return_y;
	return return_y;
}

/* int ShowAutoCompletion(char *input, char **keywords, size_t keywords_len, char **matching)
{
	char **matching_tmp = (char **)calloc(sizeof(char *), keywords_len);

	for (int i = 0; i < keywords_len; i++)
	{
		matching_tmp[i] = calloc(keywords_len, sizeof(char)); // Allocate memory for the matching strings
	}
	// Use Levenshtein distance to find the matching strings
} */

int GetNewtroditInput(File_info *tstack) // The same arguments are used to keep compatibility with the Windows version
{

	int keycode = getch_n();
	return keycode;
}

int IsWholeWord(char *str, char *word, char *delims)
{
    if (!memcmp(str, word, strlen(str)))
    {
        return 1;
    }
    char *ptr = strstr(str, word);
    if (!ptr)
    {
        return -1;
    }
    size_t wordindex = ptr - str;
    int isBeginning = 0, isEnd = 0;

    if (!wordindex)
    {
        isBeginning = 1;
    }
    else if (!memcmp(ptr, word, strlen(ptr)))
    {
        isEnd = 1;
    }
    int beginningWhole = 0, endWhole = 0;
    for (int k = 0; k < strlen(delims); k++)
    {

        if (!isBeginning && str[wordindex - 1] == delims[k])
        {
            beginningWhole = 1;
        }

        if (!isEnd && ptr[strlen(word)] == delims[k])
        {
            endWhole = 1;
        }

        if ((beginningWhole && (endWhole || isEnd)) || (endWhole && (isBeginning || beginningWhole)))
        {
            return 1;
        }
    }
    return 0;
}