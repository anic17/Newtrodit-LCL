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

void LineCountHighlight(File_info *tstack)
{
	if (linecountHighlightLine)
	{

		ClearPartial(0, tstack->last_dispy, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);

		printf("%d", tstack->last_y);

		tstack->last_dispy = tstack->display_y;
		tstack->last_y = tstack->ypos;
		SetColor(linecount_highlight_color);
	}
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

		LineCountHighlight(tstack);
		ClearPartial(0, disp, (lineCount ? (tstack->linecount_wide) : 0) - 1, 1);

		printf("%d", tstack->ypos);

		SetColor(bg_color);
	}
}

int LineContainsNewLine(File_info *tstack, int linenum)
{
	return !strncmp(tstack->strsave[linenum] + NoLfLen(tstack->strsave[linenum]), tstack->newline, strlen(tstack->newline));
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

int SetDisplayX(File_info *tstack)
{
	if (tstack->xpos > wrapSize)
	{
		tstack->scrolled_x = true;
		tstack->display_x = tstack->xpos - wrapSize;
	}
	else
	{
		tstack->scrolled_x = false;
		tstack->display_x = 0;
	}
	return tstack->display_x;
}

HANDLE OpenFileHandle(File_info *tstack, char *filename)
{
	HANDLE hFileTmp;
	hFileTmp = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFileTmp == INVALID_HANDLE_VALUE)
	{
		WriteLogFile(GetErrorDescription(GetLastError()));
		last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;
		tstack->hFile = NULL;
		return 0;
	}
	tstack->hFile = hFileTmp;

	GetFileTime(tstack->hFile, &tstack->fread_time, NULL, &tstack->fwrite_time); // Get the last write time of the file
	if (GetFileAttributes(filename) & FILE_ATTRIBUTE_READONLY)
	{
		tstack->is_readonly = true;
	}
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
		char *print_ln = calloc(sizeof(char), DEFAULT_ALLOC_SIZE + (tstack->linecount_wide + 1) * n);

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
			snprintf(print_ln, DEFAULT_ALLOC_SIZE + (tstack->linecount_wide + 1) * n, "%s%d\n", print_ln, startpos + i);
		}
		fwrite(print_ln, sizeof(char), strlen(print_ln), stdout);

		free(print_ln);
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
	SetWrapSize();
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
				print_line(tstack->strsave[i], i);
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

int SaveFile(File_info *tstack, char *fname)
{
	char *tmp_filename = calloc(MAX_PATH, sizeof(char));
	if (tstack->is_untitled)
	{
		if (!fname)
		{
			PrintBottomString("%s", NEWTRODIT_PROMPT_SAVE_FILE);
			fgets(tmp_filename, MAX_PATH, stdin); // Can't use sizeof filename because it's a pointer
		}

		if (NoLfLen(tmp_filename) <= 0)
		{
			LoadAllNewtrodit();
			DisplayFileContent(tstack, stdout, 0); // Display file content on screen
			PrintBottomString("%s", NEWTRODIT_FUNCTION_ABORTED);
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
			PrintBottomString("%s%s", NEWTRODIT_FS_FILE_INVALID_NAME, tmp_filename);
			WriteLogFile("%s%s", NEWTRODIT_FS_FILE_INVALID_NAME, tmp_filename);
			last_known_exception = NEWTRODIT_FS_FILE_INVALID_NAME;

			getch_n();
			return 0;
		}
		if (!CheckFile(tmp_filename))
		{
			LoadAllNewtrodit();
			DisplayFileContent(tstack, stdout, 0);
			PrintBottomString("%s", NEWTRODIT_PROMPT_OVERWRITE);

			if (!YesNoPrompt())
			{
				ShowBottomMenu();
				DisplayCursorPos(tstack);
				return 0;
			}
		}
		tstack->filename = strdup(tmp_filename);
	}
	if (GetFileAttributes(tmp_filename) & FILE_ATTRIBUTE_READONLY)
	{
		LoadAllNewtrodit();
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		PrintBottomString(NEWTRODIT_FS_READONLY_SAVE);
		getch_n();
		ShowBottomMenu();
		DisplayCursorPos(&Tab_stack[file_index]);
		free(tmp_filename);

		return 0;
	}

	FILE *fp = fopen(tmp_filename, "wb");

	if (WriteBuffer(fp, tstack)) // Write the file content
	{
		tstack->is_saved = true;
		tstack->is_modified = false;
		tstack->is_untitled = false;

		UpdateTitle(tstack);
		tstack->filename = strdup(tmp_filename);
		NewtroditNameLoad();
		DisplayTabIndex(tstack);
		CenterText(StrLastTok(tstack->filename, PATHTOKENS), 0);

		RightAlignNewline();
		DisplayTabIndex(tstack);

		PrintBottomString("%s", NEWTRODIT_FILE_SAVED);
		WriteLogFile("%s%s", NEWTRODIT_FILE_WROTE_BUFFER, tstack->filename);

		c = -2;
	}
	else
	{
		PrintBottomString("%s", NEWTRODIT_FS_FILE_SAVE_ERR);
		WriteLogFile("%s", NEWTRODIT_FS_FILE_SAVE_ERR);

		getch_n();
	}
	fclose(fp);
	free(tmp_filename);

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

enum BufferCopyOptions
{
	FLAG_EMPTY = 1,
	FLAG_ALL = 2,
};

char **CopyBuffer(char **src, size_t bufsz, size_t buflen, long options)
{
	char **dest = calloc(sizeof(char *), bufsz);
	for (int k = 0; k < bufsz; k++)
	{
		dest[k] = calloc(bufsz, sizeof(char));
		if (options & 2)
		{
			memcpy(dest[k], src[k], buflen);
		}
		if (!dest[k])
		{
			last_known_exception = NEWTRODIT_ERROR_OUT_OF_MEMORY;
			WriteLogFile("Failed to allocate temporary buffer memory");

			return NULL;
		}
	}
	return dest;
}

int LoadFile(File_info *tstack, char *filename, FILE *fpread)
{
	WriteLogFile("Loading file: %s", filename);
	if (!fpread)
	{
		if (CheckFile(filename))
		{
			last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;
			WriteLogFile("%s%s", NEWTRODIT_FS_FILE_NOT_FOUND, filename);
			return -ENOENT;
		}
		else
		{
			if (GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY)
			{
				last_known_exception = NEWTRODIT_FS_IS_A_DIRECTORY;
				WriteLogFile("%s%s", NEWTRODIT_FS_IS_A_DIRECTORY, filename);
				return - EISDIR;
			}
			else
			{
				last_known_exception = NEWTRODIT_FS_ACCESS_DENIED;
				WriteLogFile("%s%s", NEWTRODIT_FS_ACCESS_DENIED, filename);
				return -EACCES;
			}
		}
	}
	if (!trimLongLines)
	{
		fseek(fpread, 0, SEEK_END);
		if ((int)ftell(fpread) >= BUFFER_X * BUFFER_Y)
		{
			fseek(fpread, 0, SEEK_SET);
			last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
			WriteLogFile("%s%s", NEWTRODIT_FS_FILE_TOO_LARGE, filename);

			return -EFBIG;
		}
	}

	fseek(fpread, 0, SEEK_SET);
	int line_count = 0;
	if ((line_count = CountLines(fpread)) >= BUFFER_Y)
	{
		WriteLogFile("%s%s", NEWTRODIT_FS_FILE_TOO_LARGE, filename);
		last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;

		return -EFBIG;
	}

	char **temp_filebuf = CopyBuffer(NULL, tstack->bufy, tstack->bufx, FLAG_EMPTY);

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
				WriteLogFile("%s%s", NEWTRODIT_ERROR_INVALID_UNICODE_SEQUENCE, filename);

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

			if (convertNull && !chr)
			{

				chr = 32;
			}
			if (chr == 13 && strncmp(tstack->newline, "\r\n", strlen(tstack->newline)))
			{
				continue;
			}
			if (chr == 10) // LF / CRLF when using Windows newline format
			{
				temp_filebuf[read_y][read_x] = 10; // LF

				read_x = 0;
				read_y++;
			}
			else
			{

				if (chr == 9)
				{
					if (convertTabtoSpaces) // Convert tab to spaces
					{
						memset(temp_filebuf[read_y] + read_x, 32, TAB_WIDE);
						read_x += TAB_WIDE;
					}
					else
					{
						temp_filebuf[read_y][read_x++] = 9;
					}
				}
				else
				{
					temp_filebuf[read_y][read_x++] = chr;
				}
			}
			if (read_x > BUFFER_X)
			{
				temp_filebuf[read_y][read_x] = '\0'; // Terminate string
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

					allocate_buf = realloc(temp_filebuf, BUFFER_Y + BUFFER_INCREMENT_LOADFILE);
					BUFFER_Y += BUFFER_INCREMENT_LOADFILE;
					if (!allocate_buf)
					{
						last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
						WriteLogFile("%s%s", NEWTRODIT_FS_FILE_TOO_LARGE, filename);

						return -EFBIG;
					}
				}
				else
				{
					last_known_exception = NEWTRODIT_FS_FILE_TOO_LARGE;
					WriteLogFile("%s%s", NEWTRODIT_FS_FILE_TOO_LARGE, filename);

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
	OpenFileHandle(tstack, tstack->filename);

	if (GetFileAttributes(tstack->filename) & FILE_ATTRIBUTE_READONLY)
	{
		PrintBottomString(NEWTRODIT_WARNING_READONLY_FILE);
		tstack->is_readonly = true;
		c = -2;
	}
	fclose(fpread);

	for (int i = 0; i < tstack->bufy; i++)
	{
		memcpy(tstack->strsave[i], temp_filebuf[i], tstack->bufx); // Copy the temporary buffer to the file buffer
		free(temp_filebuf[i]);
	}
	tstack->linecount_wide = LINECOUNT_WIDE;
	if (strlen(itoa_n(line_count)) > strlen(itoa_n(YSIZE - 2)))
	{
		tstack->linecount_wide = LINECOUNT_WIDE + strlen(itoa_n(line_count)) - strlen(itoa_n(YSIZE - 2));
	} // If the line count is too long, increase the width of the line count

	WriteLogFile("Successfully loaded the file '%s'", filename);

	return 1;
}

int NewFile(File_info *tstack) // ^N = New file
{

	if (open_files < MAX_TABS)
	{
		file_index = open_files;
		tstack = &Tab_stack[file_index];
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
	UpdateTitle(tstack);

	gotoxy(tstack->xpos + (lineCount ? (tstack->linecount_wide) : 0), tstack->ypos);

	return 1;
}

int CloseFile(File_info *tstack)
{
	if (tstack->is_modified)
	{
		PrintBottomString("%s", NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);
		if (YesNoPrompt())
		{
			SaveFile(tstack, NULL);
		}
		tstack->is_modified = false;
	}
	if (open_files > 1)
	{

		if (FreeBufferMemory(&Tab_stack[file_index]))
		{
			AllocateBufferMemory(&Tab_stack[file_index]);
			FreeBufferMemory(&Tab_stack[open_files]);

			if (file_index < open_files - 1) // TODO: Fix strsave not copying
			{
				for (int k = file_index; k < open_files; k++)
				{
					printf("\r{'%s' %d} First char: %d ('%c') Address: %p\t\n", Tab_stack[k].filename, k, Tab_stack[k].strsave[1][0], Tab_stack[k].strsave[1][0], &Tab_stack[k].strsave);
					memcpy(&Tab_stack[k], &Tab_stack[k + 1], sizeof(*Tab_stack));
				}
			}
			open_files--;

			if (file_index > 0)
			{
				file_index--;
			}
		}
		else
		{
			return 0;
		}

		for (int i = 0; i < open_files; i++)
		{
			printf("Tab num %d {'%s'} First char: %d ('%c') Address: %p\n", i, Tab_stack[i].filename, Tab_stack[i].strsave[1][0], Tab_stack[i].strsave[1][0], &Tab_stack[i].strsave);
		}
	}
	else
	{
		if (tstack->is_untitled && !tstack->is_modified || tstack->is_modified || tstack->is_saved || !tstack->is_untitled)
		{
			if (FreeBufferMemory(tstack))
			{
				AllocateBufferMemory(tstack);
			}
		}
	}
	getch_n();
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

			PrintBottomString("%s", ErrorMessage(abs(n), tstack->filename)); // Errors are always negative, so abs() is used
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
	PrintBottomString("%s", NEWTRODIT_FUNCTION_ABORTED);
	getch_n();
	ShowBottomMenu();
	return;
}

int UpdateHorizontalScroll(File_info *tstack, bool reset)
{
	if (tstack->display_x > 0)
	{
		if (reset)
		{
			tstack->display_x = 0;
		}
		ClearPartial((lineCount ? (tstack->linecount_wide) : 0), 1, XSIZE - (lineCount ? (tstack->linecount_wide) : 0), YSCROLL);
		DisplayFileContent(&Tab_stack[file_index], stdout, 0);
		return 1;
	}
	return 0;
}

/* TODO: There are too many functions for updating scroll, remove 2 of them */
int UpdateScrolledScreen(File_info *tstack)
{
	if (tstack->ypos >= (YSIZE - 3) || tstack->display_x > 0 || tstack->scrolled_y)
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
		ClearPartial(lineCount ? (tstack->linecount_wide) : 0, 1, XSIZE - lineCount ? (tstack->linecount_wide) : 0, YSCROLL);
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
			return (signed long long)(-abs(errno)) - 1;
		}
		else
		{
			return (signed long long)(-abs(errno)) - 256;
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

int InsertNewRow(File_info *tstack, int *xps, int *yps, int dispy, int size)
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
	strncat(tstack->strsave[(*yps) - 1], tstack->newline, strlen(tstack->newline));
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

			PrintBottomString(NEWTRODIT_FS_FILE_INVALID_NAME, file);
			return 0;
		}
	}
	char *dir_tmp = calloc(MAX_PATH * 2, sizeof(char));
	char *out_dir = calloc(MAX_PATH * 2, sizeof(char));
	GetCurrentDirectory(MAX_PATH, dir_tmp);

	if (!isWildcard && get_path_directory(file, out_dir))
	{
		SetCurrentDirectory(out_dir);
		file = StrLastTok(strdup(file), PATHTOKENS);
	}
	SetColor(0x70);
	int box_x = 5;
	int box_y = 2;
	int box_margin_x = 4;
	int box_margin_y = 1;
	ClearPartial(box_x, box_y, XSIZE - 2 * box_x, YSIZE - 2 * box_y);
	gotoxy(box_x, box_y);

	printf("Current directory: %s\n", getcwd(NULL, 0));

	if ((hFindFiles = FindFirstFile(search_pattern, &FindFileData)) != INVALID_HANDLE_VALUE)
	{

		while (FindNextFile(hFindFiles, &FindFileData))
		{

			if (FindString(FindFileData.cFileName, file) != -1 || isWildcard) // Ignore the search result if wildcards are used
			{
				if (n < (ys - (3 + box_y)) + startpos)
				{
					gotoxy(box_x + box_margin_x, n + box_margin_y + box_y);
					FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &st);
					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && show_dir)
					{

						// Time format is "DD/MM/YYYY hh:mm:ss"
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t<DIR>\t%14lu\t%.*s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
					}
					else
					{
						printf(" %02d/%02d/%04d %02d:%02d:%02d\t\t%14lu\t%.*s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, FindFileData.nFileSizeLow, max_print, FindFileData.cFileName);
					}
					n++;
				}

				total++;
			}
		}
	}
	SetColor(0x07);
	if (!total)
	{
		PrintBottomString("%s%s", NEWTRODIT_FS_FILE_NOT_FOUND, file);
		last_known_exception = NEWTRODIT_FS_FILE_NOT_FOUND;

		chdir(dir_tmp);
		return 0;
	}
	else
	{
		PrintBottomString(NEWTRODIT_FS_FOUND_FILES, n, total);
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
		find_string_index = FindString(strlwr(strdup(str)), strlwr(strdup(find)));
	}
	else
	{
		find_string_index = FindString(str, find);
	}
	return find_string_index;
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

/* int ShowAutoCompletion(char *input, char **keywords, size_t keywords_len, char **matching)
{
	char **matching_tmp = (char **)calloc(sizeof(char *), keywords_len);

	for (int i = 0; i < keywords_len; i++)
	{
		matching_tmp[i] = calloc(keywords_len, sizeof(char)); // Allocate memory for the matching strings
	}
	// Use Levenshtein distance to find the matching strings
} */

void ErrorExit(char *s)
{
	WriteLogFile(s);
	PrintBottomString(s);
	getch_n();
	return;
}

DWORD GetKeyboardCurrentState(SHORT *keybd_layout) // Not to be confused with WinAPI's GetKeyState() or GetKeyboardState()
{
	for (int i = 0; i < 256; i++)
	{
		keybd_layout[i] = GetAsyncKeyState(i);
	}
	return 1;
}

int GetNewtroditInput(File_info *tstack)
{
	HANDLE hStdin, hStdout;
	DWORD cNumRead, fdwSaveOldMode;
	INPUT_RECORD irInBuffer;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SHORT *keybd_layout_ptr = {0}, *keybd_layout_ptr2 = {0};
	bool ignore_next_key = false;
	COORD oldSize = {0}, newSize = {0};
	// Get the standard input handle.

	int key_press = 0; // An intermediate variable will be used to avoid bugs with SetConsoleMode()

	int x = 0, y = 0, old_ypos = 1;
	int altnum_index = 0;
	char altnum_buf[300];

	SHORT ign_vkcodes[] = {
		VK_LBUTTON,
		VK_RBUTTON,
		VK_CANCEL,
		VK_MBUTTON,
		VK_XBUTTON1,
		VK_XBUTTON2,
		VK_HANGUL,
		VK_HANJA,
		0x16,
		0x17,
		VK_FINAL,
		0x1A,
		VK_CONVERT,
		VK_NONCONVERT,
		VK_ACCEPT,
		VK_MODECHANGE,
		VK_SHIFT,
		VK_CONTROL,
		VK_MENU,
		VK_CAPITAL,
		VK_NUMLOCK,
		VK_SCROLL,
		VK_LWIN,
		VK_RWIN,
		VK_LSHIFT,
		VK_RSHIFT,
		VK_LCONTROL,
		VK_RCONTROL,
		VK_LMENU,
		VK_RMENU,
		VK_SNAPSHOT,
		VK_PAUSE,
		VK_APPS,
		0xA6,
		0xA7,
		0xA8,
		0xA9,
		0xAA,
		0xAB,
		0xAC,
		0xAD,
		0xAE,
		0xAF,
		0xB0,
		0xB1,
		0xB2,
		0xB3,
		0xB4,
		0xB5,
		0xB6,
		0xB7,
		0xFF,
	};

	// Get the standard input handle.

	hStdin = GetStdHandle(STD_INPUT_HANDLE), hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE)
	{
		PrintBottomString(NEWTRODIT_ERROR_CONSOLE_HANDLE);
		return -1;
	}
	GetConsoleScreenBufferInfo(hStdout, &csbi);

	if (partialMouseSupport)
	{
		keybd_layout_ptr = calloc(sizeof(SHORT), 256);
		keybd_layout_ptr2 = calloc(sizeof(SHORT), 256); // Allocate size for each of the 256 possible keys
		if (!keybd_layout_ptr || !keybd_layout_ptr2)
		{
			ErrorExit(NEWTRODIT_ERROR_OUT_OF_MEMORY);
			return 0;
		}
	}
	GetConsoleMode(hStdin, &fdwSaveOldMode);
	SetConsoleMode(hStdin, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | (fdwSaveOldMode & 0x40)); // Only enable these 2 modes

	while (1)
	{
		if (partialMouseSupport)
		{
			GetKeyboardCurrentState(keybd_layout_ptr);
		}
		if (allowAutomaticResizing)
		{
			oldSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			oldSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}

		WaitForSingleObject(hStdin, INFINITE);
		// Wait for the events.
		if (PeekConsoleInput(hStdin, &irInBuffer, 1, &cNumRead) == 0)
		{
			ErrorExit("PeekConsoleInput() failed");
			SetConsoleMode(hStdin, fdwSaveOldMode);
			return 0;
		}

		if (irInBuffer.EventType == KEY_EVENT)
		{
			if (partialMouseSupport)
			{
				GetKeyboardCurrentState(keybd_layout_ptr2);

				ignore_next_key = false;
				for (int i = 0; i < 256; ++i)
				{
					if (keybd_layout_ptr[i] != keybd_layout_ptr2[i])
					{
						if (keybd_layout_ptr[i] == 0 || ((keybd_layout_ptr[i] & 0x8000) && (keybd_layout_ptr2[i] & 0x8000)))
						{
							// printf("\n[P] Key pressed %d\t'%c'. ", i, MapVirtualKey(i, 2));
							if (keybd_layout_ptr2[VK_LMENU] && i >= VK_NUMPAD0 && i <= VK_NUMPAD9 && altnum_index < sizeof(altnum_buf)) // Using the numpad Alt+Num
							{
								altnum_buf[altnum_index++] = MapVirtualKey(i, 2); // Map the key to the Alt+Num to a numeric string.
								// printf("Alt+Num %d\t'%c'. ", i, MapVirtualKey(i, 2));
								break;
							}
							for (int k = 0; k < sizeof(ign_vkcodes) / sizeof(ign_vkcodes[0]); k++) // Ignore all the keys in the list as getch_n() can't handle them.
							{
								if (i == ign_vkcodes[k])
								{
									ignore_next_key = true;
								}
							}

							if (!ignore_next_key)
							{
								key_press = getch_n();
								SetConsoleMode(hStdin, fdwSaveOldMode);

								return key_press;
							}
						}
						else
						{
							if (i == VK_LMENU)
							{
								if (altnum_buf[0] != 0)
								{
									SetConsoleMode(hStdin, fdwSaveOldMode);
									return atoi(altnum_buf) % 256;
								}
							}
						}
					}
				}
			}
			else
			{
				key_press = getch_n();
				SetConsoleMode(hStdin, fdwSaveOldMode);

				return key_press;
			}
		}
		if (irInBuffer.EventType == WINDOW_BUFFER_SIZE_EVENT)
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
						SetConsoleSize(oldSize.X, oldSize.Y, oldSize.X, oldSize.Y);
					}
					else
					{
						SetConsoleSize(newSize.X, newSize.Y, newSize.X, newSize.Y);

						oldSize.X = newSize.X;
						oldSize.X = newSize.Y; // Routine to resize the console window and adapt the current screen buffer size to the new console size.
					}
					clearAllBuffer = true;
					LoadAllNewtrodit();
					clearAllBuffer = false;
					DisplayFileContent(&Tab_stack[file_index], stdout, 0);
					DisplayCursorPos(&Tab_stack[file_index]);

					SetDisplayY(&Tab_stack[file_index]);
					SetDisplayCursorPos(&Tab_stack[file_index]);
				}
			}
		}
		if (partialMouseSupport)
		{

			if (irInBuffer.EventType == MOUSE_EVENT) // Mouse event
			{
				if (irInBuffer.Event.MouseEvent.dwEventFlags == MOUSE_WHEELED) // Vertical scroll
				{
					if (!(irInBuffer.Event.MouseEvent.dwButtonState & 0x80000000)) // Up
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
							DisplayCursorPos(tstack);
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
							UpdateScrolledScreen(tstack);

							DisplayCursorPos(tstack);
							SetDisplayY(tstack);

							SetDisplayCursorPos(tstack);
						}
					}
				}
				if (irInBuffer.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)
				{
					// TODO: Select the word
				}
				x = irInBuffer.Event.MouseEvent.dwMousePosition.X;
				y = irInBuffer.Event.MouseEvent.dwMousePosition.Y;
				if (irInBuffer.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && irInBuffer.Event.MouseEvent.dwEventFlags != MOUSE_MOVED)
				{

					if (x <= XSIZE && x < tstack->bufx && y < tstack->bufy && y <= YSCROLL && y <= tstack->linecount && (tstack->ypos == 1 || (tstack->strsave[tstack->ypos][0] != '\0' || LineContainsNewLine(tstack, tstack->ypos - 1))))
					{
						SetYFromDisplayY(tstack, y);
						SetTitle("%d (%d,%d) {%d,%d}", 0, x, y, tstack->xpos, tstack->ypos);

						tstack->xpos = (x - (lineCount ? tstack->linecount_wide : 0) > NoLfLen(tstack->strsave[tstack->ypos]) ? NoLfLen(tstack->strsave[tstack->ypos]) : x - (lineCount ? tstack->linecount_wide : 0));
						if (y < YSCROLL && old_ypos >= tstack->display_y) // Avoid reloading the screen unnecessarely
						{
							RedrawScrolledScreen(tstack, tstack->ypos);
						}
						DisplayCursorPos(tstack);
						SetDisplayY(tstack);
						SetDisplayCursorPos(tstack);
					}
				}
				else
				{
					tstack->ypos = old_ypos;
				}
			}
			FlushConsoleInputBuffer(hStdin);
		}
		FlushConsoleInputBuffer(hStdin);
	}

	return 0;
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
	for (size_t k = 0; k < strlen(delims); k++)
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

void KeypressWait(int max_time)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	WaitForSingleObject(hStdout, max_time);
}

int OpenNewtroditFile(File_info *tstack, char *fname)
{
	FILE *fileread;
	char fileopenread[MAX_PATH] = {0};
	int n = 0;
	if (tstack->is_modified)
	{
		PrintBottomString("%s", NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);

		if (YesNoPrompt())
		{
			if (!SaveFile(tstack, NULL))
			{
				LoadAllNewtrodit();
				DisplayFileContent(tstack, stdout, 0);
				return 0;
			}
		}
	}
	if (fname)
	{
		memcpy(fileopenread, fname, sizeof fileopenread);
	}
	else
	{
		PrintBottomString("%s", NEWTRODIT_PROMPT_FOPEN);

		fgets(fileopenread, sizeof fileopenread, stdin);
		if (NoLfLen(fileopenread) <= 0)
		{
			FunctionAborted(tstack);
			return 0;
		}
	}

	// Remove trailing LF
	fileopenread[strcspn(fileopenread, "\n")] = 0;
	RemoveQuotes(fileopenread, strdup(fileopenread));

	fileread = fopen(fileopenread, "rb");
	n = -errno;
	LoadAllNewtrodit();
	/* if (GetFileAttributes(tstack->filename) & FILE_ATTRIBUTE_DEVICE)
	{
		PrintBottomString(NEWTRODIT_ERROR_CANNOT_OPEN_DEVICE);
		c = -2;
		continue;
	} */

	for (int i = 0; i < open_files; ++i)
	{
		if (!strcmp(StrLastTok(Tab_stack[i].filename, PATHTOKENS), fileopenread) && i != file_index) // File is already open
		{
			DisplayFileContent(tstack, stdout, 0);

			PrintBottomString(NEWTRODIT_PROMPT_ALREADY_OPEN_TAB);
			if (YesNoPrompt())
			{
				file_index = i;
				LoadAllNewtrodit();
				DisplayFileContent(tstack, stdout, 0);

				PrintBottomString("%s %s (%d/%d).", NEWTRODIT_SWITCHING_FILE, fileopenread, file_index + 1, open_files);
				c = -2;
				return 1;
			}
			else
			{
				ClearPartial(0, BOTTOM, XSIZE, 1);
			}
			break;
		}
	}
	if (c != -2)
	{
		if (!fileread || (n = LoadFile(tstack, strdup(fileopenread), fileread)) != 1) // Failed to open the file
		{
			DisplayFileContent(tstack, stdout, 0);

			PrintBottomString("%s", ErrorMessage(abs(n), fileopenread)); // Errors are always negative, so abs() is used
			getch_n();
			ShowBottomMenu();

			DisplayCursorPos(tstack);

			return 0;
		}

		old_open_files[oldFilesIndex] = tstack->filename;
		NewtroditNameLoad();
		CenterText(StrLastTok(tstack->filename, PATHTOKENS), 0);
		DisplayTabIndex(tstack);
		RightAlignNewline();
		DisplayFileContent(tstack, stdout, 0);
	}

	fclose(fileread);

	return 1;
}
