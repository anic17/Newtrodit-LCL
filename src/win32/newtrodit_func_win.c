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
		ClearPartial(0, disp, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);

		printf("%d", tstack->ypos);

		SetColor(bg_color);
	}
}
HANDLE OpenFileHandle(File_info *tstack, char *filename)
{
	HANDLE hFileTmp;
	hFileTmp = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileTmp == INVALID_HANDLE_VALUE)
	{
		last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;
		return 0;
	}
	GetFileTime(tstack->hFile, &tstack->fread_time, NULL, &tstack->fwrite_time); // Get the last write time of the file
	if (GetFileAttributes(tstack->filename) & FILE_ATTRIBUTE_READONLY)
	{
		tstack->is_readonly = true;
	}
	tstack->hFile = hFileTmp;
	return tstack->hFile;
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
		char *print_line = (char *)calloc(sizeof(char), DEFAULT_ALLOC_SIZE + (tstack->linecount_wide + 1) * n), *tempbuf = (char *)calloc(sizeof(char), DEFAULT_ALLOC_SIZE + (tstack->linecount_wide));

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
		SetColor(linecount_color);

		ClearPartial(0, 1, tstack->linecount_wide - 1, lines_load); // Clear the line count area
		for (int i = 1; i <= lines_load; i++)						// Print line numbers
		{
			snprintf(print_line, DEFAULT_ALLOC_SIZE + (tstack->linecount_wide + 1) * n, "%s%d\n", print_line, startpos + i);
		}
		fwrite(print_line, sizeof(char), strlen(print_line), stdout);
		free(print_line);
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
			if (!strncmp(tstack->strsave[i], tstack->newline, strlen(tstack->newline)))
			{
				continue;
			}
			else
			{
				gotoxy((lineCount ? (tstack->linecount_wide) : 0), i - startpos);
				print_line(tstack->strsave[i]);
			}
		}
	}
	if (syntaxHighlighting && syntaxAfterDisplay)
	{
		syntaxAfterDisplay = false;
		for (int i = 1 + startpos + starty; i < startpos + (window_size - 1); i++)
		{

			if (tstack->strsave[i][0] != '\0')
			{
				if (!strncmp(tstack->strsave[i], tstack->newline, strlen(tstack->newline)))
				{
					continue;
				}
				else
				{
					gotoxy((lineCount ? (tstack->linecount_wide) : 0), i - startpos);
					color_line(tstack->strsave[i], 0, tstack->Syntaxinfo.override_color);
				}
			}
		}
	}
	SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));

	return 0;
}

int ValidFileName(char *filename)
{
	return strpbrk(filename, "*?\"<>|\x1b") == NULL;
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
		if (!CheckFile(tmp_filename))
		{
			LoadAllNewtrodit();
			DisplayFileContent(tstack, stdout, 0);
			PrintBottomString(NEWTRODIT_PROMPT_OVERWRITE);

			if (!YesNoPrompt())
			{
				ShowBottomMenu();
				DisplayCursorPos(tstack->xpos, tstack->ypos);
				return 0;
			}
		}
		tstack->filename = strdup(tmp_filename);
	}

	FILE *fp = fopen(tstack->filename, "wb");

	if (GetFileAttributes(tmp_filename) & FILE_ATTRIBUTE_READONLY)
	{
		LoadAllNewtrodit();
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		PrintBottomString(NEWTRODIT_FS_READONLY_SAVE);
		getch_n();
		ShowBottomMenu();
		DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
		return 0;
	}

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
	int remove_cr = 0;
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

	tstack->hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // Open a handle to the file
	if (tstack->hFile == INVALID_HANDLE_VALUE)
	{
		// WriteLogFile(join(join(join(join("Error opening file handle: ", filename), "(0x"), itoa(GetLastError(), NULL, 16)), ")"));
	}
	OpenFileHandle(tstack->hFile, filename);
	GetFileTime(tstack->hFile, &tstack->fread_time, NULL, &tstack->fwrite_time); // Get the last write time of the file
	if (GetFileAttributes(tstack->filename) & FILE_ATTRIBUTE_READONLY)
	{
		PrintBottomString(NEWTRODIT_WARNING_READONLY_FILE);
		tstack->is_readonly = true;
		c = -2;
	}
	fclose(fpread);
	WriteLogFile(join("Successfully loaded the file ", filename));

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
	printf("%d", tstack->linecount_wide);
	getch_n();
	LoadAllNewtrodit();
	DisplayCursorPos(tstack->xpos, tstack->ypos);
	UpdateTitle(tstack);

	gotoxy(tstack->xpos + (lineCount ? (tstack->linecount_wide) : 0), tstack->ypos);

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
	File_info *tmp;
	tmp = tstack;
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
	DisplayCursorPos(tstack->xpos, tstack->ypos);
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

/* TODO: There are too many functions for updating scroll, remove 2 of them */
int UpdateScrolledScreen(File_info *tstack)
{
	if (tstack->ypos >= (YSIZE - 3) || horizontalScroll > 0 || tstack->scrolled_y)
	{
		ClearPartial((lineCount ? (tstack->linecount_wide) : 0), 1, XSIZE - (lineCount ? (tstack->linecount_wide) : 0), YSCROLL);
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		c = -32; // -32 means that the screen has been scrolled
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
		c = -32; // -32 means that the screen has been scrolled
		return 1;
	}
	return 0;
}

int RedrawScrolledScreen(File_info *tstack, int yps)
{
	if (!UpdateScrolledScreen(tstack))
	{
		if (!tstack->scrolled_y && yps <= YSCROLL)

		{
			ClearPartial((lineCount ? (tstack->linecount_wide) : 0), 1, XSIZE - (lineCount ? (tstack->linecount_wide) : 0), YSCROLL);
			DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

char *TypingFunction(int min_ascii, int max_ascii, int max_len)
{
	int cursor_visible = GetConsoleInfo(CURSOR_VISIBLE);
	SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));
	int chr = 0, index = 0;
	char *num_str = (char *)malloc(max_len) + 1;

	memset(num_str, 0, max_len); // Clear string
	while (chr != 13)			 // Loop while enter isn't pressed
	{
		chr = getch();
		if (chr == 27)
		{
			break;
		}
		if (chr == 8) // Backspace
		{
			if (index > 0)
			{
				num_str[index--] = '\0';
				printf("\b \b");
			}
			continue;
		}

		if (chr == 0xE0)
		{
			chr = getch();
			switch (chr)
			{
			case 75:
				if (index > 0)
				{
					putchar('\b');
					index--;
				}
				break;
			case 77:
				if (index < max_len)
				{
					printf("%c", num_str[index]);

					index++;
				}
				break;
			default:
				break;
			}
			continue;
		}
		if (chr >= min_ascii && chr <= max_ascii && chr != 0) // Check if character is a between the range
		{
			if (strlen(num_str) >= max_len && index >= max_len) // Check if max length is reached
			{
				putchar('\a');
			}
			else
			{
				num_str[index++] = chr;
				putchar(chr);
			}
		}
	}
	SetCursorSettings(cursor_visible, GetConsoleInfo(CURSOR_SIZE));

	return num_str;
}

int AutoIndent(File_info *tstack, int yps, int *xps)
{
	if (autoIndent)
	{
		if (yps < 2 || yps >= BUFFER_X)
		{
			return -1; // Too big
		}
		if (convertTabtoSpaces)
		{
			if (!strncmp(tstack->strsave[yps - 1], PrintTab(TAB_WIDE), TAB_WIDE))
			{
				strncpy_n(tstack->strsave[yps], tstack->newline, strlen(tstack->newline));
				*xps = TAB_WIDE;
				return 1;
			}
		}
		return 0;
	}
	return -2;
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

	tstack->strsave[*yps] = (char *)calloc(size, sizeof(char));

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

int LocateFiles(bool show_dir, char *file, int startpos)
{
	int n = startpos, total = startpos;

	bool isWildcard = false;
	HANDLE hFindFiles;
	WIN32_FIND_DATA FindFileData;
	int ys = YSIZE;
	int max_print = wrapSize;
	char *search_pattern = "*";
	SYSTEMTIME st;

	if (strpbrk(file, "*?") != NULL)
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
	char *dir_tmp = (char *)calloc(MAX_PATH * 2, sizeof(char));
	char *out_dir = (char *)calloc(MAX_PATH * 2, sizeof(char));
	GetCurrentDirectory(MAX_PATH, dir_tmp);

	if (!isWildcard && get_path_directory(file, out_dir))
	{
		SetCurrentDirectory(out_dir);
		file = StrLastTok(strdup(file), PATHTOKENS);
	}

	printf("Current directory: %s\n", getcwd(NULL, 0));

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
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t<DIR>\t%14lu\t%.*s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, (FindFileData.nFileSizeHigh * (MAXDWORD + 1)) + FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
					}
					else
					{
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t\t%14lu\t%.*s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, (FindFileData.nFileSizeHigh * (MAXDWORD + 1)) + FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
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

int SetYFromDisplayY(File_info *tstack, int disp_y)
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

int ShowAutoCompletion(char *input, char **keywords, size_t keywords_len, char **matching)
{
	char **matching_tmp = (char **)calloc(sizeof(char *), keywords_len);

	for (int i = 0; i < keywords_len; i++)
	{
		matching_tmp[i] = (char *)calloc(keywords_len, sizeof(char)); // Allocate memory for the matching strings
	}
	// Use Levenshtein distance to find the matching strings
}

void ErrorExit(char *s)
{
	MessageBox(0, s, "Newtrodit", 16);
	return;
}


int GetNewtroditInput(File_info *tstack)
{
	HANDLE hStdin, hStdout;
	DWORD fdwSaveOldMode;
	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuffer[128];
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	COORD oldSize = {0}, newSize = {0};
	// Get the standard input handle.

	int x = 0, y = 0, old_ypos = 1;

	// Get the standard input handle.

	hStdin = GetStdHandle(STD_INPUT_HANDLE), hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE)
	{
		PrintBottomString(NEWTRODIT_ERROR_CONSOLE_HANDLE);
		return -1;
	}
	GetConsoleScreenBufferInfo(hStdout, &csbi);

	while (1)
	{
		if (allowAutomaticResizing)
		{
			oldSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			oldSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}
		WaitForSingleObject(hStdin, INFINITE);
		// Wait for the events.
		if (PeekConsoleInput(hStdin, irInBuffer, 128, &cNumRead) == 0)
		{
			ErrorExit("PeekConsoleInput");
			return 0;
		}
		for (int i = 0; i < cNumRead; i++)
		{
			if (irInBuffer[i].EventType == KEY_EVENT && irInBuffer->Event.KeyEvent.bKeyDown) // Must have pressed the key, not released
			{
				return getch();
			}
			if (irInBuffer[i].EventType == FOCUS_EVENT)
			{
				// Ignore window focus events.
			}
			if (irInBuffer[i].EventType == WINDOW_BUFFER_SIZE_EVENT)
			{
				if (allowAutomaticResizing)
				{

					GetConsoleScreenBufferInfo(hStdout, &csbi);
					newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
					newSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

					if ((oldSize.X != newSize.X || oldSize.Y != newSize.Y))
					{

						if (ValidSize()) // At 3 message boxes, close the program
						{
							SetConsoleSize(oldSize.X, oldSize.Y);
						}
						else
						{
							SetConsoleSize(newSize.X, newSize.Y);
							oldSize.X = newSize.X;
							oldSize.X = newSize.Y; // Routine to resize the console window and adapt the current screen buffer size to the new console size.
						}
						WriteLogFile(join(join(join("Resized the console window: ", itoa_n(newSize.X)), "x"), itoa_n(newSize.Y)));
						// printf("Console size changed from %dx%d to %dx%d\n", old_x_size, old_y_size, new_x_size, new_y_size);
						clearAllBuffer = true;
						LoadAllNewtrodit();
						clearAllBuffer = false;
						DisplayFileContent(&Tab_stack[file_index], stdout, 0);
						DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

						SetDisplayY(&Tab_stack[file_index]);
						SetDisplayCursorPos(&Tab_stack[file_index]);
					}
				}
			}
			if (partialMouseSupport)
			{

				if (irInBuffer[i].EventType == MOUSE_EVENT) // Mouse event
				{
					if (irInBuffer[i].Event.MouseEvent.dwEventFlags == MOUSE_WHEELED) // Vertical scroll
					{
						if (!(irInBuffer[i].Event.MouseEvent.dwButtonState & 0x80000000)) // Up
						{
							if (tstack->ypos >= 1 && tstack->strsave[tstack->ypos][0] != '\0')
							{
								(tstack->ypos - scrollRate > 1) ? (tstack->ypos -= scrollRate) : (tstack->ypos = 1);
								if (tstack->last_pos_scroll == -1)
								{
									tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
								}
								if (tstack->xpos > NoLfLen(tstack->strsave[tstack->ypos]))
								{
									tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
								}
								UpdateScrolledScreen(tstack);
								DisplayCursorPos(tstack->xpos, tstack->ypos);
								SetDisplayY(tstack);
								SetDisplayCursorPos(tstack);
							}
						}
						else // Down
						{
							if (tstack->ypos + scrollRate < tstack->bufy && tstack->strsave[tstack->ypos + scrollRate][0] != '\0')
							{
								tstack->ypos += scrollRate;
								if (tstack->last_pos_scroll == -1)
								{
									tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
								}
								if (tstack->xpos > NoLfLen(tstack->strsave[tstack->ypos]))
								{
									tstack->last_pos_scroll = -1; // End of line
									tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
								}
								DisplayCursorPos(tstack->xpos, tstack->ypos);
								SetDisplayY(tstack);
								UpdateScrolledScreen(tstack);

								SetDisplayCursorPos(tstack);
							}
						}
					}
					if (irInBuffer[i].Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)
					{
						// TODO: Select the word
					}
					if (irInBuffer[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && irInBuffer[i].Event.MouseEvent.dwEventFlags != MOUSE_MOVED)
					{

						x = irInBuffer[i].Event.MouseEvent.dwMousePosition.X;
						y = irInBuffer[i].Event.MouseEvent.dwMousePosition.Y;

						if (x <= XSIZE && x < tstack->bufx && y < tstack->bufy && y <= YSCROLL && tstack->strsave[y][0] != '\0')
						{

							old_ypos = tstack->ypos;
							SetYFromDisplayY(tstack, y);
							tstack->xpos = (x - (lineCount ? tstack->linecount_wide : 0) > NoLfLen(tstack->strsave[tstack->ypos]) ? NoLfLen(tstack->strsave[tstack->ypos]) : x - (lineCount ? tstack->linecount_wide : 0));
							if (y < YSCROLL && old_ypos >= tstack->display_y) // Avoid reloading the screen unnecessarely
							{
								RedrawScrolledScreen(tstack, tstack->ypos);
							}
							DisplayCursorPos(tstack->xpos, tstack->ypos);
							SetDisplayY(tstack);
							SetDisplayCursorPos(tstack);
						}
					}
				}
				FlushConsoleInputBuffer(hStdin);
			}
		}
	}

	return 0;
}