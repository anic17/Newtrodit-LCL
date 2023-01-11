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

int SelectStart(File_info *tstack, size_t start_x, size_t start_y)
{
    if (tstack->selection.is_selected)
    {
        tstack->selection.start.x = start_x;
        tstack->selection.start.y = start_y;
    }
    return tstack->selection.is_selected;
}

int SelectEnd(File_info *tstack, size_t end_x, size_t end_y)
{
    if (tstack->selection.is_selected)
    {
        tstack->selection.end.x = end_x;
        tstack->selection.end.y = end_y;
    }
    return tstack->selection.is_selected;
}

int SelectSet(File_info *tstack, size_t start_x, size_t start_y, size_t end_x, size_t end_y)
{
    SelectStart(tstack, start_x, start_y);
    SelectEnd(tstack, start_x, start_y);
    return tstack->selection.is_selected;
}

int SelectClear(File_info *tstack)
{
    SelectStart(tstack, 0, 0);
    SelectEnd(tstack, 0, 0);
    tstack->selection.is_selected = false;
    return tstack->selection.is_selected;
}

int SelectAppend(File_info *tstack, size_t add_x, size_t add_y)
{
    tstack->selection.is_selected = true;
    tstack->selection.end.x += add_x;
    tstack->selection.end.y += add_y;
    return tstack->selection.is_selected;
}

int SelectCurrentPos(File_info *tstack)
{
    tstack->selection.is_selected = true;
    tstack->selection.end.x = tstack->xpos;
    tstack->selection.end.y = tstack->ypos;
    return tstack->selection.is_selected;
}

int SelectPrint(File_info *tstack, size_t yps)
{
    // TODO: Horizontal scroll support
    size_t startpos = tstack->selection.start.x > wrapSize ? wrapSize : tstack->selection.start.x;
    size_t endpos = tstack->selection.end.x, count = endpos - startpos;
    if (tstack->selection.is_selected && tstack->selection.start.y <= yps && tstack->selection.end.y >= yps) // Current line is selected
    {
        if (tstack->selection.start.y != yps)
        {
            startpos = 0;
        }
        if (tstack->selection.end.y != yps)
        {
            endpos = NoLfLen(tstack->strsave[tstack->ypos]) > wrapSize ? wrapSize : NoLfLen(tstack->strsave[tstack->ypos]);
        }

        gotoxy((lineCount ? tstack->linecount_wide : 0), GetConsoleInfo(YCURSOR));
        printf("%.*s", startpos, tstack->strsave[yps] + tstack->display_x);
        if (startpos < wrapSize)
        {
            SetColor(SELECTION_COLOR * 16 + fg_color);

            printf("%.*s", count >= wrapSize ? wrapSize : count, tstack->strsave[yps] + startpos);
            SetColor(bg_color);

            if (startpos + count < wrapSize)
            {
                printf("%.*s", wrapSize - (startpos + count), tstack->strsave[yps] + startpos + count);
            }
        }

        WriteLogFile("Writing color at pos %d, len %X, end %d", startpos, count, endpos);
    }
    else
    {
        WriteLogFile("Conditions not met");
    }
    return tstack->selection.is_selected;
}

int SelectCheck(File_info *tstack) // Start selection
{
    if (CheckKey(VK_SHIFT))
    {

        if (!tstack->selection.is_selected)
        {
            tstack->selection.is_selected = true;
            SelectStart(tstack, tstack->xpos, tstack->ypos);
            SelectPrint(tstack, tstack->ypos);
        }
    }
    else
    {
        SelectClear(tstack);
    }
    return tstack->selection.is_selected;
}

int atoi_tf(char *s)
{
    int retval = 0;
    if (!strcmp(s, "true"))
    {
        retval = 1;
    }
    else if (!strcmp(s, "false"))
    {
        retval = atoi(s);
    }
    return retval;
}

int SetBoolValue(int *boolv, char *s)
{
    int retval = atoi_tf(s);
    *boolv = !!retval;
    return retval;
}

int IsNumberString(char *s)
{
    for (size_t i = 0; i < strlen(s); i++)
    {
        if (!isdigit(s[i]))
        {
            return 0;
        }
    }
    return 1;
}

int GotoBufferPosition(File_info *tstack, int numbuf, bool is_column)
{
    bool xgoto = true;
    size_t n = 0;
    char *ptr;
    char *line_number_str;
    if (is_column)
    {
        xgoto = true;
        if (!numbuf)
        {
            PrintBottomString("%s", NEWTRODIT_PROMPT_GOTO_COLUMN);
        }
        ptr = NEWTRODIT_ERROR_INVALID_XPOS;
    }
    else
    {
        xgoto = false;
        if (!numbuf)
        {
            PrintBottomString("%s", NEWTRODIT_PROMPT_GOTO_LINE);
        }
        ptr = NEWTRODIT_ERROR_INVALID_YPOS;
    }
    if (!numbuf)
    {
        line_number_str = calloc(sizeof(char), strlen(itoa_n(tstack->linecount)) * 10 + 10);
        line_number_str = TypingFunction('0', '9', xgoto ? strlen(itoa_n(NoLfLen(tstack->strsave[tstack->ypos]))) : strlen(itoa_n(tstack->linecount)));
        numbuf = atoi(line_number_str);
        free(line_number_str);
        if (numbuf < 1) // Line is less than 1
        {
            PrintBottomString("%s", ptr);
            c = -2;

            return 0;
        }
    }

    if (!xgoto) // Go to line
    {
        n = tstack->ypos;
        tstack->ypos = numbuf;
        if (BufferLimit(tstack)) // Avoid crashes
        {
            tstack->ypos = n;

            ShowBottomMenu();
            DisplayCursorPos(tstack);
            c = -2;

            return 0;
        }
        if (!xgoto && tstack->ypos != 1 && tstack->strsave[tstack->ypos][0] == '\0' && !LineContainsNewLine(tstack, tstack->ypos))
        {
            PrintBottomString(ptr);
            tstack->ypos = n;
            c = -2;
            return 0;
        }
        RedrawScrolledScreen(tstack, n);
    }
    else
    { // Go to column
        n = tstack->xpos;
        tstack->xpos = numbuf - 1;

        if (NoLfLen(tstack->strsave[tstack->ypos]) < tstack->xpos)
        {
            PrintBottomString(ptr);
            tstack->xpos = n; // n-1 because it is zero indexed
            c = -2;
            return 0;
        }
    }
    if (!xgoto)
    {
        (tstack->ypos >= (YSIZE - 3)) ? UpdateScrolledScreen(tstack) : UpdateHomeScrolledScreen(tstack);
    }

    ShowBottomMenu();
    DisplayCursorPos(tstack);
    return 1;
}

int SwitchTab(File_info *tstack, int new_tab_index)
{
    bool n = 0;
    if (!new_tab_index)
    {
        n = 1;
    }
    else if (new_tab_index > 0)
    {
        n = -1;
    }
    else
    {
        n = 0;
    }

    if (open_files > 1)
    {

        if (n)
        {

            if (file_index + n < 0)
            {
                file_index = open_files - 1;
            }
            else
            {

                file_index += n;
            }
            file_index = file_index % open_files;
        }
        else
        {

            file_index = abs(new_tab_index) - 1;
        }
        if (file_index >= open_files)
        {
            PrintBottomString("%s", NEWTRODIT_ERROR_INVALID_FILE_INDEX);
            getch_n();
            file_index = 0;
        }
        LoadAllNewtrodit();
        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

        PrintBottomString("%s (%d/%d).", (n == -1) ? NEWTRODIT_SHOWING_PREVIOUS_FILE : NEWTRODIT_SHOWING_NEXT_FILE, file_index + 1, open_files);
    }
    else
    {
        PrintBottomString("%s", NEWTRODIT_INFO_NO_FILES_TO_SWITCH);
    }

    c = -2;
}

int ParseCommandPalette(File_info *tstack, char *command_palette)
{
    size_t numbuf;
    if (IsNumberString(command_palette + 1))
    {
        numbuf = atoi(command_palette + 1);
    }
    switch (command_palette[0])
    {
    case ':':
    {
        GotoBufferPosition(tstack, numbuf, false);
        return 1;
    }
    case ',':
    {
        GotoBufferPosition(tstack, numbuf, true);
        return 1;
    }

    case '-':
    {
        if (tstack->ypos - numbuf >= 1)
        {
            tstack->ypos -= numbuf;
            if (tstack->xpos >= NoLfLen(tstack->strsave[tstack->ypos]))
            {
                tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
            }
            UpdateScrolledScreen(tstack);
        }
        return 1;
    }

    case '+':
    {
        if (tstack->ypos + numbuf > tstack->linecount && tstack->ypos + numbuf >= tstack->bufy)
        {
            tstack->ypos += numbuf;
            if (tstack->xpos >= NoLfLen(tstack->strsave[tstack->ypos]))
            {
                tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
            }
            UpdateScrolledScreen(tstack);
        }
    }
    case '0':
    {
        tstack->xpos = 0;

        UpdateHorizontalScroll(&Tab_stack[file_index], true);

        return 1;
    }
    case '^':
    {
        tstack->xpos = strspn(tstack->strsave[tstack->ypos], " \t");
        UpdateHorizontalScroll(&Tab_stack[file_index], true);
        return 1;
    }

    case '$':

    {
        // END key
        numbuf = tstack->xpos;
        tstack->xpos = NoLfLen(tstack->strsave[tstack->ypos]);
        if (numbuf == tstack->xpos)
        {
            return 1;
        }
        SetDisplayX(&Tab_stack[file_index]);
        UpdateHorizontalScroll(&Tab_stack[file_index], false);
        return 1;
    }
    case '@':
    {
        if (numbuf)
        {
            SwitchTab(tstack, -numbuf);
        }
        return 1;
    }
    case '?':
    {
        if (&command_palette[1])
        {
            OpenNewtroditFile(tstack, command_palette + 1);
        }
        return 1;
    }
    case '!':
    {
        if (&command_palette[1])
        {
            SaveFile(tstack, command_palette+1);
        }
        return 1;
    }
    }
}