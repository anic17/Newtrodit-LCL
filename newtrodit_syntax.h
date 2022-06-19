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

    This file is from my other project syntax, with some modifications.
    https://github.com/anic17/syntax

*/

int syntaxKeywordsSize = sizeof(keywords) / sizeof(keywords[0]);

int is_separator(int c) // Taken from https://github.com/antirez/kilo/blob/69c3ce609d1e8df3956cba6db3d296a7cf3af3de/kilo.c#L366
{
    return c == '\0' || isspace(c) || strchr(syntax_separators, c) != NULL;
}

int EmptySyntaxScheme(File_info *tstack)
{

    for (int i = 0; i < tstack->Syntaxinfo.keyword_count; ++i)
    {
        memset(tstack->Syntaxinfo.keywords[i], 0, strlen(tstack->Syntaxinfo.keywords[i]));
        keywords[i].color = 0;
    }
    return tstack->Syntaxinfo.keyword_count;
}

int LoadSyntaxScheme(FILE *syntaxfp, char *syntax_fn, File_info *tstack)
{

    WriteLogFile("Loading syntax highlighting scheme");

    fseek(syntaxfp, 0, SEEK_SET); // Set the file pointer to the beginning of the file
    char *read_syntax_buf = (char *)malloc(sizeof(char) * LINE_MAX);

    char *syntax_language = malloc(sizeof(char) * MAX_PATH);
    char *tokchar = "=";
    char *iniptr;
    char comments[MAX_PATH] = ";\r\n";
    int c = 0, highlight_color;
    bool isNull = false;
    memset(syntax_language, 0, sizeof(char) * MAX_PATH);
    bool hasComment = false, hasMagicNumber = false, hasLanguage = false;

    while (fgets(read_syntax_buf, LINE_MAX, syntaxfp))
    {
        if (strcspn(read_syntax_buf, comments) == 0)
        {
            continue;
        }
        read_syntax_buf[strcspn(read_syntax_buf, "\r\n")] = 0;

        if (c == 0)
        {
            if (!hasMagicNumber)
            {
                if (strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_MAGIC_NUMBER, strlen(NEWTRODIT_SYNTAX_MAGIC_NUMBER)))
                {
                    PrintBottomString(join(NEWTRODIT_ERROR_INVALID_SYNTAX, syntax_fn));
                    return 0;
                }
                else
                {
                    hasMagicNumber = true;
                    continue;
                }
            }
        }
        if (!hasComment && !strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_COMMENT, strlen(NEWTRODIT_SYNTAX_COMMENT)))
        {
            hasComment = true;
            comment[0].keyword = read_syntax_buf + strlen(NEWTRODIT_SYNTAX_COMMENT) + 1;

            continue;
        }

        if (!hasLanguage && !strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_LANGUAGE, strlen(NEWTRODIT_SYNTAX_LANGUAGE)))
        {

            hasLanguage = true;

            strncpy_n(syntax_language, read_syntax_buf + strlen(NEWTRODIT_SYNTAX_LANGUAGE) + 1, MAX_PATH); // Whitespace character

            tstack->Syntaxinfo.syntax_lang = (char *)calloc(strlen(syntax_language), sizeof(char));

            tstack->Syntaxinfo.syntax_lang = strdup(syntax_language);

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_DEFAULT_COLOR, strlen(NEWTRODIT_SYNTAX_DEFAULT_COLOR)))
        {
            default_color = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_DEFAULT_COLOR) + 1);
            tstack->Syntaxinfo.default_color = default_color;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_QUOTE_COLOR, strlen(NEWTRODIT_SYNTAX_QUOTE_COLOR)))
        {
            quote_color = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_QUOTE_COLOR) + 1);
            tstack->Syntaxinfo.quote_color = quote_color;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_COMMENT_COLOR, strlen(NEWTRODIT_SYNTAX_COMMENT_COLOR)))
        {
            comment_color = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_COMMENT_COLOR) + 1);
            tstack->Syntaxinfo.comment_color = comment_color;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_NUMBER_COLOR, strlen(NEWTRODIT_SYNTAX_NUMBER_COLOR)))
        {
            num_color = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_NUMBER_COLOR) + 1);
            tstack->Syntaxinfo.num_color = num_color;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_CAPITAL_COLOR, strlen(NEWTRODIT_SYNTAX_CAPITAL_COLOR)))
        {
            capital_color = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_CAPITAL_COLOR) + 1);
            tstack->Syntaxinfo.capital_color = capital_color;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_CAPITAL_MIN, strlen(NEWTRODIT_SYNTAX_CAPITAL_MIN)))
        {
            capital_min_len = hexstrtodec(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_CAPITAL_MIN) + 1);
            tstack->Syntaxinfo.capital_min = capital_min_len;

            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_CAPITAL, strlen(NEWTRODIT_SYNTAX_CAPITAL)))
        {
            if (atoi(read_syntax_buf + strlen(NEWTRODIT_SYNTAX_CAPITAL) + 1) == 1)
            {
                tstack->Syntaxinfo.capital_enabled = true;
            }
            else
            {
                tstack->Syntaxinfo.capital_enabled = false;
            }
            continue;
        }
        if (!strncmp(read_syntax_buf, NEWTRODIT_SYNTAX_SEPARATORS, strlen(NEWTRODIT_SYNTAX_SEPARATORS)))
        {
            strncpy_n(syntax_separators, read_syntax_buf + strlen(NEWTRODIT_SYNTAX_SEPARATORS) + 1, sizeof syntax_separators);
            tstack->Syntaxinfo.separators = strdup(syntax_separators);

            continue;
        }

        iniptr = strtok(read_syntax_buf, tokchar);
        if (c >= tstack->Syntaxinfo.keyword_count)
        {
            tstack->Syntaxinfo.keywords = (char **)realloc(tstack->Syntaxinfo.keywords, sizeof(char *) * (c + 1));
            tstack->Syntaxinfo.color = (int *)realloc(tstack->Syntaxinfo.color, sizeof(int) * (c + 1));
            tstack->Syntaxinfo.keywords[c] = (char *)calloc(DEFAULT_ALLOC_SIZE, sizeof(char)); // Allocate memory for the new keyword
            tstack->Syntaxinfo.color[c] = (int)calloc(1, sizeof(int));
        }
        tstack->Syntaxinfo.keywords[c] = strdup(iniptr);

        // If strdup is not used, the value will be overwritten by the next strtok call
        iniptr = strtok(NULL, tokchar);

        highlight_color = hexstrtodec(iniptr);
        tstack->Syntaxinfo.color[c] = abs(highlight_color) % 16; // Range from 0 to 15
        WriteLogFile(join(
            join(
                join(
                    "Keyword: ",
                    tstack->Syntaxinfo.keywords[c]),

                ", color: "),
            itoa_n(tstack->Syntaxinfo.color[c])));
        c++;
        isNull = false;
        /* if (c > (sizeof(keywords) / sizeof(keywords[0])))
        {
            PrintBottomString(NEWTRODIT_WARNING_SYNTAX_TOO_BIG);
            getch_n();
            tstack->Syntaxinfo.keyword_count = c;
            return c;
        } */
    }

    if (isNull)
    {
        PrintBottomString(NEWTRODIT_ERROR_SYNTAX_RULES);
        getch_n();
        return 0;
    }
    tstack->Syntaxinfo.keyword_count = c;

    return c;
}

void color_line(char *line, int startpos, int override_color)
{
    if (override_color <= 0)
    {
        override_color = 0; // Default color
    }
    // Color a specific line
    int tab_count = 0, keyword_len = 0, skipChars = 0, find_pos; // Always initialize variables to 0 to avoid undefined behavior (= crash)

    char quotechar;
    if (!Tab_stack[file_index].Syntaxinfo.multi_line_comment)
    {
        SetColor(Tab_stack[file_index].Syntaxinfo.default_color);
    }
    int internal_bps_count = Tab_stack[file_index].Syntaxinfo.bracket_pair_count; // Used to count the number of brackets in a string
    size_t len = strlen(line);
    // This causes visual bugs
    // SetCharColor(wrapSize, Tab_stack[file_index].Syntaxinfo.default_color, lineCount ? (Tab_stack[file_index].linecount_wide - 1) : 0, GetConsoleInfo(YCURSOR));

    for (size_t i = 0; i < len; ++i)
    {
        if (line[i] == '\n') // Shouldn't happen (editor should already parse this), but just in case
        {
            SetColor(Tab_stack[file_index].Syntaxinfo.default_color);

            return;
        }

        if (i < wrapSize)
        {
            while(i < len && i < wrapSize && line[i] != '\0' && isspace(line[i]))
            {
                ++i;
            }

            /*  if (!memcmp(line + i, comment[1].keyword, strlen(comment[1].keyword)))
             {
                 SetColor(comment[1].color);
                 Tab_stack[file_index].Syntaxinfo.multi_line_comment = true;
                 skipChars = 0;

                 while (!memcmp(line + i + skipChars, comment[1].keyword, strlen(comment[1].keyword)))
                 {
                     if (i + skipChars >= wrapSize || i + skipChars >= len || line[i] + skipChars == '\0')
                     {
                         fwrite(line + i, sizeof(char), skipChars, stdout);

                         return;
                     }

                     skipChars++;
                 }
                 fwrite(line + i, sizeof(char), skipChars, stdout);

                 i += skipChars + strlen(comment[1].keyword); // -1 because we want to skip the comment
             }
             if (!memcmp(line + i, comment[2].keyword, strlen(comment[2].keyword)))
             {
                 printf("%.*s", wrapSize - i, comment[2].keyword);
                 Tab_stack[file_index].Syntaxinfo.multi_line_comment = false;
                 SetColor(Tab_stack[file_index].Syntaxinfo.default_color);
                 i += strlen(comment[2].keyword);
             } */

            if (isdigit(line[i]) && !Tab_stack[file_index].Syntaxinfo.multi_line_comment)
            {
                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR));
                if (i == 0 || is_separator(line[i - 1]))
                {
                    if (i < len)
                    {
                        SetColor(Tab_stack[file_index].Syntaxinfo.num_color + (16 * override_color));
                        putchar(line[i++]);
                        switch (tolower(line[i]))
                        {
                        case 'b':
                            do
                            {
                                putchar(line[i++]);
                            } while ((line[i] == '0' || line[i] == '1') && i < len && i < wrapSize);
                            break;
                        case 'o':

                            do
                            {
                                putchar(line[i++]);
                            } while (isdigit(line[i]) && line[i] < '8' && i < len && i < wrapSize);

                            break;
                        case 'x':
                            do
                            {
                                putchar(line[i++]);
                            } while (isxdigit(line[i]) && i < len && i < wrapSize);
                            break;

                        default:
                            if (isdigit(line[i]))
                            {
                                do
                                {
                                    putchar(line[i++]);
                                } while (isdigit(line[i]) && i < len && i < wrapSize);
                            }

                            break;
                        }
                        if (i < len && i < wrapSize)
                        {
                            switch (tolower(line[i])) // For long, unsigned and float
                            {
                            case 'l':
                                putchar(line[i++]);
                                if (i < len && tolower(line[i]) == 'l' && i < wrapSize)
                                {
                                    putchar(line[i++]);
                                }
                                break;
                            case 'u':
                                putchar(line[i++]);
                                break;
                            case 'f':
                                putchar(line[i++]);
                                break;
                            default:
                                break;
                            }
                        }

                        SetColor(Tab_stack[file_index].Syntaxinfo.default_color + (16 * override_color));
                    }
                }
            }
            if (!memcmp(line + i, comment[0].keyword, strlen(comment[0].keyword)))
            {
                SetCharColor(wrapSize - i - 1, Tab_stack[file_index].Syntaxinfo.comment_color + (16 * override_color), (lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR)); // +1 because we want to also color the quote
                SetColor(Tab_stack[file_index].Syntaxinfo.default_color + (16 * override_color));
                return;
            }

            if ((line[i] == '\"' || (line[i] == '\'' && singleQuotes)) && !Tab_stack[file_index].Syntaxinfo.multi_line_comment)
            {
                if (i < len)
                {
                    quotechar = line[i];
                    skipChars = 0;
                    do
                    {
                        skipChars++;
                    } while (line[i + skipChars] != quotechar && i + skipChars < len && i + skipChars < wrapSize);

                    SetCharColor(skipChars + 1, Tab_stack[file_index].Syntaxinfo.quote_color + (16 * override_color), (lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR)); // +1 because we want to also color the quote
                    i += skipChars;

                    continue;
                }
            }
            else
            {
                if (bpsPairHighlight)
                {
                    for (int j = 0; j < sizeof(bps_chars_open) / sizeof(bps_chars_open[0]); j++)
                    {
                        if (!memcmp(line + i, bps_chars_open[j], strlen(bps_chars_open[j])))
                        {
                            SetCharColor(strlen(bps_chars_open[j]), bps_pair_colors[Tab_stack[file_index].Syntaxinfo.bracket_pair_count % (sizeof(bps_pair_colors) / sizeof(bps_pair_colors[0]))] + (16 * override_color), (lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR));
                            i += strlen(bps_chars_open[j]);

                            break;
                        }
                    }
                }

                if (i == 0 || is_separator(line[i - 1]))
                {
                    for (int k = 0; k < syntaxKeywordsSize; k++)
                    {
                        keyword_len = strlen(Tab_stack[file_index].Syntaxinfo.keywords[k]);
                        if (is_separator(line[i + keyword_len]) && !memcmp(line + i, Tab_stack[file_index].Syntaxinfo.keywords[k], keyword_len))
                        {
                            gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR));

                            if (i + keyword_len < wrapSize)
                            {
                                SetCharColor(keyword_len, Tab_stack[file_index].Syntaxinfo.color[k] + (16 * override_color), GetConsoleInfo(XCURSOR), GetConsoleInfo(YCURSOR)); // +1 because we want to also color the quote
                            }
                            else
                            {
                                SetCharColor(wrapSize - i, Tab_stack[file_index].Syntaxinfo.color[k] + (16 * override_color), GetConsoleInfo(XCURSOR), GetConsoleInfo(YCURSOR)); // +1 because we want to also color the quote
                            }

                            i += keyword_len;
                            break;
                        }
                    }
                    if ((isupper(line[i]) || line[i] == '_') && i < len && i < wrapSize && Tab_stack[file_index].Syntaxinfo.capital_enabled) // Also add underscores because variable names can contain them
                    {
                        skipChars = 0;
                        do
                        {
                            skipChars++;
                        } while (i + skipChars < len && i + skipChars < wrapSize && (isupper(line[i + skipChars]) || line[i + skipChars] == '_'));

                        if (is_separator(line[i + skipChars]) && skipChars >= Tab_stack[file_index].Syntaxinfo.capital_min) // All the string is in capital and more than 1 character
                        {
                            SetCharColor(skipChars, Tab_stack[file_index].Syntaxinfo.capital_color + (16 * override_color), (lineCount ? Tab_stack[file_index].linecount_wide : 0) + i, GetConsoleInfo(YCURSOR));
                            i += skipChars;
                        }
                    }
                }
            }
        }
        else
        {
            // No need to parse the line that is not rendered
            SetColor(bg_color);

            return;
        }
    }
    SetColor(bg_color);
    return;
}