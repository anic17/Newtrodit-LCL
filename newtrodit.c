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

/*

    Some remarks about the code:
    - 'c' is used for various purposes, and always has a negative value which means a the code of a key.
    - 'ch' is the character typed by the user.
    - 'Tab_stack[file_index].strsave' is the buffer where the file is stored on RAM.

    The source code of Newtrodit is composed by:

        dialog.h           : Dialogs
        globals.h          : Global variables
        manual.c           : Manual
        newtrodit.c        : Main source file
        newtrodit_core.h   : All core functions
        newtrodit_func.c   : Main functions
        newtrodit_gui.c    : GUI loading
        newtrodit_syntax.h : Syntax highlighting

     See 'newtrodit --help'

*/

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include "manual.c"

void sigsegv_handler(int signum)
{
    signal(SIGSEGV, sigsegv_handler);
    NewtroditCrash(join("Segmentation fault. Signum code: ", itoa_n(signum)), errno);
    fflush(stdout);
    exit(errno);
}

void sigabrt_handler(int signum)
{
    signal(SIGABRT, sigabrt_handler);
    NewtroditCrash(join("Abort signal. Signum code: ", itoa_n(signum)), errno);
    fflush(stdout);
    exit(errno);
}

void sigbreak_handler(int signum)
{
    signal(SIGBREAK, sigbreak_handler);
    fflush(stdout);
}

int LoadSettings(char *newtrodit_config_file, char *macro, int *sigsegv, int *linecount, int *devmode, File_info *tstack)
{
    /*
        Settings are stored in an INI-like format.
        The format is:
            key=value
            ;comment

    */

    _chdir(SInf.location);
    WriteLogFile(join("Loading settings file: ", newtrodit_config_file));

    FILE *settings = fopen(newtrodit_config_file, "rb");
    if (!settings)
    {
        return EXIT_FAILURE;
    }

    char setting_buf[1024]; // Max 1 kB per line
    char *iniptr = (char *)malloc(sizeof(setting_buf) + 1), *token = (char *)malloc(sizeof(setting_buf) + 1);
    int cnt = 0;
    int parse_hex;
    char equalchar[] = "=";
    char *setting_list[] = {
        "fontcolor",
        "codepage",
        "convertnull",
        "converttab",
        "cursize",
        "curinsert"
        "devmode",
        "linecount",
        "linecountwide",
        "macro",
        "manfile",
        "menucolor",
        "mouse"
        "newline",
        "oldkeybinds",
        "sigsegv",
        "syntax",
        "tabwide",
        "trimlonglines",
        "xsize",
        "ysize",
    }; // List of settings that can be changed

    // Set the non changing settings
    SetColor(FG_DEFAULT);
    SetColor(BG_DEFAULT);
    default_color = DEFAULT_SYNTAX_COLOR;
    strncpy_n(Tab_stack[file_index].newline, "\n", strlen(Tab_stack[file_index].newline));
    run_macro[0] = 0;
    last_known_exception = NEWTRODIT_CRASH_INVALID_SETTINGS;
    while (fgets(setting_buf, sizeof(setting_buf), settings))
    {
        setting_buf[strcspn(setting_buf, "\n")] = 0; // Remove newline

        cnt = strspn(setting_buf, " \t");
        snprintf(setting_buf, sizeof(setting_buf), "%s", setting_buf + cnt);

        if (setting_buf[cnt] == ';' || setting_buf[cnt] == 0) // Comment or newline found
        {
            continue;
        }
        iniptr = strtok(setting_buf, "=");

        while (iniptr != NULL) // Loop through the settings
        {
            for (int i = 0; i < sizeof(setting_list) / sizeof(char *); i++)
            {
                if (!strncmp(iniptr, setting_list[i], strlen(setting_list[i])))
                {
                    token = strtok(NULL, equalchar);
                    if (!strcmp(setting_list[i], "fontcolor"))
                    {
                        bg_color = hexstrtodec(token) % 256;
                        default_color = bg_color;
                    }
                    if (!strcmp(setting_list[i], "codepage"))
                    {
                        int cp = atoi(token);
                        SetConsoleOutputCP(cp);
                    }
                    if (!strcmp(setting_list[i], "convertnull"))
                    {
                        if (atoi(token))
                        {
                            convertNull = true;
                        }
                        else
                        {
                            convertNull = false;
                        }
                    }
                    if (!strcmp(setting_list[i], "converttab"))
                    {
                        if (atoi(token))
                        {
                            convertTabtoSpaces = true;
                        }
                        else
                        {
                            convertTabtoSpaces = false;
                        }
                        convertTabtoSpaces = true; // TODO: Remove this line and add full tab support
                    }

                    if (!strcmp(setting_list[i], "curinsert"))
                    {
                        if (atoi(token))
                        {
                            cursorSizeInsert = true;
                        }
                        else
                        {
                            cursorSizeInsert = false;
                        }
                    }

                    if (!strcmp(setting_list[i], "cursize"))
                    {
                        CURSIZE = atoi(token);
                    }

                    if (!strcmp(setting_list[i], "devmode"))
                    {

                        atoi(token) ? (*devmode = true) : (*devmode = false);
                    }
                    if (!strcmp(setting_list[i], "linecount"))
                    {
                        atoi(token) ? (*linecount = true) : (*linecount = false);
                    }
                    if (!strcmp(setting_list[i], "linecountwide"))
                    {
                        LINECOUNT_WIDE = abs(atoi(token)) % sizeof(long long);
                    }
                    if (!strcmp(setting_list[i], "macro"))
                    {

                        if (ValidString(token))
                        {
                            strncpy_n(run_macro, token, sizeof(setting_list));
                        }
                    }
                    if (!strcmp(setting_list[i], "manfile"))
                    {
                        RemoveQuotes(token, strdup(token)); // Remove quotes

                        if (ValidFileName(token))
                        {
                            strncpy_n(manual_file, token, sizeof(manual_file));
                            manual_file[strcspn(manual_file, "\n")] = 0;
                        }
                    }
                    if (!strcmp(setting_list[i], "menucolor"))
                    {
                        fg_color = (hexstrtodec(token) * 16) % 256;
                    }
                    if (!strcmp(setting_list[i], "mouse"))
                    {
                        atoi(token) ? (partialMouseSupport = true) : (partialMouseSupport = false);
                    }

                    if (!strcmp(setting_list[i], "newline"))
                    {
                        if (token != NULL)
                        {
                            if (!strncmp(token, "0x", 2))
                            {
                                parse_hex_string(token);
                            }
                            strncpy_n(Tab_stack[file_index].newline, token, strlen(Tab_stack[file_index].newline));
                        }
                    }

                    if (!strcmp(setting_list[i], "oldkeybinds"))
                    {
                        atoi(token) ? (useOldKeybinds = true) : (useOldKeybinds = false);
                    }
                    if (!strcmp(setting_list[i], "sigsegv"))
                    {
                        atoi(token) ? (*sigsegv = true) : (*sigsegv = false);
                    }
                    if (!strcmp(setting_list[i], "syntax"))
                    {
                        RemoveQuotes(token, strdup(token)); // Remove quotes
                        if (ValidFileName(token))
                        {

                            if (!strcmp(token, "1")) // Enable syntax highlighting but don't load any file
                            {
                                syntaxHighlighting = true;
                            }
                            else if (!strcmp(token, "0"))
                            {
                                syntaxHighlighting = false;
                            }
                            else
                            {
                                FILE *syntax = fopen(token, "rb");
                                if (!syntax)
                                {
                                    fprintf(stderr, "%s\b: %s\n", NEWTRODIT_FS_FILE_OPEN_ERR, token);
                                }
                                else
                                {

                                    strncpy_n(syntax_filename, token, sizeof(syntax_filename));
                                    syntax_filename[strcspn(syntax_filename, "\n")] = 0;
                                    EmptySyntaxScheme(&Tab_stack[file_index]);
                                    syntaxKeywordsSize = LoadSyntaxScheme(syntax, syntax_filename, &Tab_stack[file_index]);
                                    syntaxHighlighting = true;
                                }
                            }
                        }
                    }
                    if (!strcmp(setting_list[i], "tabwide"))
                    {
                        TAB_WIDE = atoi(token);
                    }
                    if (!strcmp(setting_list[i], "trimlonglines"))
                    {
                        atoi(token) ? (trimLongLines = true) : (trimLongLines = false);
                    }

                    if (!strcmp(setting_list[i], "xsize"))
                    {
                        int xs = atoi(token);
                        SetConsoleSize(xs, YSIZE);
                    }
                    if (!strcmp(setting_list[i], "ysize"))
                    {
                        int ys = atoi(token);
                        SetConsoleSize(XSIZE, ys);
                    }
                }
            }

            iniptr = strtok(NULL, equalchar);
        }
    }
    WriteLogFile("Finished loading settings file");

    fclose(settings);
    _chdir(SInf.dir);

    return 0;
}

int main(int argc, char *argv[])
{

    // Startup routine code Newtrodit must always execute
    char *startup_info = (char *)malloc(sizeof(char) * MAX_PATH * 2); // *2 for safety
    memset(startup_info, 0, sizeof(char) * MAX_PATH * 2);
    GetModuleFileNameA(NULL, startup_info, MAX_PATH);
    // Generate log file name
    SInf.log_file_name = (char *)calloc(MAX_PATH, sizeof(char));
    SInf.log_file_name = strdup(GetLogFileName());
    WriteLogFile("\nNewtrodit started");
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE), hStdin = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hStdout, &dwConsoleMode);
    GetConsoleMode(hStdin, &dwStdinMode);
    // Disable wrapping to avoid (or at least reduce) graphical bugs
    WriteLogFile(join("Changing console output mode: ", (SetConsoleMode(hStdout, dwConsoleMode & ~ENABLE_WRAP_AT_EOL_OUTPUT)) ? "Succeeded" : "Failed"));
    WriteLogFile(join("Changing console input mode: ", (SetConsoleMode(hStdin, dwConsoleMode | ENABLE_MOUSE_INPUT | 0x80 | ENABLE_ECHO_INPUT)) ? "Succeeded" : "Failed"));

    // _setmode(_fileno(stdout), _O_BINARY);

    WriteLogFile("Loading startup data");

    char *sinf_ptr = (char *)malloc(sizeof(char) * MAX_PATH * 2);
    if (get_path_directory(startup_info, sinf_ptr) != NULL)
    {
        SInf.dir = strdup(_getcwd(NULL, 0));
        SInf.location = strdup(sinf_ptr);
        //_chdir(SInf.location);
    }
    else
    {

        memset(SInf.location, 0, sizeof(char) * MAX_PATH * 2);
    }

#if _NEWTRODIT_EXPERIMENTAL_RESTORE_BUFFER == 1 && !_NEWTRODIT_OLD_SUPPORT

    if (ReadConsoleBuffer())
    {
        SInf.save_buffer = false;
    }
    // Read console buffer, memory will be automatically allocated
    SInf.cursor.x = GetConsoleInfo(XCURSOR);
    SInf.cursor.y = GetConsoleInfo(YCURSOR);
#endif

    // Get file date times
    FILETIME tmpTimeRead, tmpTimeWrite;

    tmpTimeWrite.dwLowDateTime = 0;
    tmpTimeWrite.dwHighDateTime = 0;
    tmpTimeRead.dwLowDateTime = 0;
    tmpTimeRead.dwHighDateTime = 0;

    SInf.argv = argv; // Save only its memory address, not the actual value
    SInf.argc = argc;
    SInf.xsize = XSIZE;
    SInf.ysize = YSIZE;
    SInf.manual_open = 0; // Times manual has been open
    SInf.save_buffer = false;
    /* if (!ValidSize())
    {
        ExitRoutine(1);
    } */

    wrapSize = XSIZE - LINECOUNT_WIDE;
    goto_len = strlen(itoa_n(BUFFER_Y));
    clearAllBuffer = true;

    int hasNewLine = false; // Bool for newline in insert char
    int dev_tools = false;  // Bool to enable or disable the dev tools
    int insertChar = false; // Bool to check if replace instead of insert
    int findInsensitive = false;
    int sigsegvScreen = true;
    int listDir = true;
    int isFullScreen = false;

    /*     char *run_macro = (char *)malloc(sizeof(char) * MAX_PATH + 1);
     */

    for (int i = 0; i < MAX_TABS; i++)
    {
        Tab_stack[i].filename = (char *)calloc(MAX_PATH, sizeof(char));
    }

    // Allocate buffer

    if (!AllocateBufferMemory(&Tab_stack[file_index]))
    {

        printf("%.*s\n", wrapSize, NEWTRODIT_ERROR_OUT_OF_MEMORY);
        ExitRoutine(ENOMEM);
    }

    old_open_files = (char **)malloc(MAX_PATH * sizeof(char *));

    for (int i = 1; i < MIN_BUFSIZE; i++)
    {
        old_open_files[i] = (char *)calloc(MAX_PATH, sizeof(char));
    }

    run_macro = (char *)malloc(sizeof(char) * MACRO_ALLOC_SIZE + 1);

    signal(SIGINT, SIG_IGN);          // Ctrl-C handler
    signal(SIGBREAK, SIG_IGN);        // Ctrl-Break handler
    signal(SIGSEGV, sigsegv_handler); // Segmentation fault handler
    signal(SIGABRT, sigabrt_handler); // Abort handler

    memset(run_macro, 0, sizeof(char) * MACRO_ALLOC_SIZE + 1);

    LoadSettings(settings_file, run_macro, &sigsegvScreen, &lineCount, &dev_tools, &Tab_stack[file_index]); // Load settings from settings file

    if (!sigsegvScreen)
    {
        signal(SIGSEGV, SIG_DFL);
    }

    char *err_msg = (char *)malloc(sizeof(char) * MIN_BUFSIZE);
    if (BUFFER_X < MIN_BUFSIZE || BUFFER_Y < MIN_BUFSIZE)
    {
        snprintf(err_msg, MIN_BUFSIZE, "Buffer is too small (Current size is %dx%d and minimim size is %dx%d)", BUFFER_X, BUFFER_Y, MIN_BUFSIZE, MIN_BUFSIZE);
        MessageBox(NULL, err_msg, "Newtrodit", MB_ICONERROR);
        fprintf(stderr, "%s", err_msg);
        free(err_msg);
        ExitRoutine(1);
    }
    char *temp_strsave = (char *)malloc(BUFFER_X) + 1;
    char *tmp = (char *)malloc(BUFFER_X) + 1;

    int undo_stack_line = 0, undo_stack_tree = 0;

    if (!lineCount)
    {
        LINECOUNT_WIDE = 0;
    }

    // Declare variables
    int old_x_size = 0, old_y_size = 0;
    int bs_tk = 0;

    SInf.color = GetConsoleInfo(COLOR);

    char *save_dest = (char *)malloc(MAX_PATH) + 1;
    char *line_number_str;

    char find_string[512], replace_string[512];
    int find_string_index = 0;
    char fileopenread[MAX_PATH];
    char *replace_str_ptr;
    int find_count = 0, replace_count = 0;

    char inbound_ctrl_key[100];
    char newname[MAX_PATH], syntaxfile[MAX_PATH], locate_file[MAX_PATH], macro_input[1024], fcomp1[MAX_PATH], fcomp2[MAX_PATH];

    convertTabtoSpaces = true;
    int n = 0, n2 = 0;
    signed long long ll_n = 0;
    char *ptr = (char *)calloc(sizeof(char), BUFFER_X), *buffer_clipboard;

    // File variables
    FILE *fileread, *fp_savefile, *open_argv, *syntax;

    // Position variables
    int *relative_xpos = calloc(sizeof(int) * BUFFER_Y, BUFFER_X);
    int *relative_ypos = calloc(sizeof(int) * BUFFER_X, BUFFER_Y);

    // getch() variables
    unsigned char ch = 0;

    int *file_arguments = {0}; // Array of ints for arguments that aren't switches
    int file_arguments_count = 0;

    file_arguments = (int *)malloc(sizeof(int *) * argc); // Allocate memory for file_arguments for each argument
    WriteLogFile("Finished loading startup data");

    SetColor(bg_color);
    int shiftargc = 1; // Can't be 0, because the first argument is the program name
    WriteLogFile("Parsing command-line arguments");

    for (int arg_parse = 1; arg_parse < argc; ++arg_parse)
    {
        if (!strcmp(argv[arg_parse], "--version") || !strcmp(argv[arg_parse], "-v")) // Version parameter
        {
            printf("%.*s", wrapSize, ProgInfo());
            return 0;
        }
        else if (!strcmp(argv[arg_parse], "--help") || !strcmp(argv[arg_parse], "-h")) // Manual parameter
        {
            NewtroditHelp();
            SetColor(SInf.color);
            ClearScreen();
            return 0;
        }
        else if (!strcmp(argv[arg_parse], "--syntax") || !strcmp(argv[arg_parse], "-s")) // Syntax parameter
        {
            if (argv[arg_parse + 1] != NULL)
            {
                strncpy_n(syntaxfile, argv[arg_parse + 1], MAX_PATH); // Copy the syntax file name
                syntaxfile[strcspn(syntaxfile, "\n")] = 0;
                syntax = fopen(syntaxfile, "rb");
                EmptySyntaxScheme(&Tab_stack[file_index]);
                if ((syntaxKeywordsSize = LoadSyntaxScheme(syntax, syntaxfile, &Tab_stack[file_index])) != 0) // Change keywords size
                {

                    syntaxHighlighting = true;
                    Tab_stack[file_index].Syntaxinfo.syntax_file = full_path(syntaxfile);
                    Tab_stack[file_index].Syntaxinfo.keyword_count = syntaxKeywordsSize;
                }

                arg_parse++;
            }
            else
            {
                fprintf(stderr, "%s\n", NEWTRODIT_ERROR_MISSING_ARGUMENT);
                return 1;
            }
        }
        else if (!strcmp(argv[arg_parse], "--line") || !strcmp(argv[arg_parse], "-l")) // Display line count
        {
            lineCount = true;
        }
        else if (!strcmp(argv[arg_parse], "--lfunix") || !strcmp(argv[arg_parse], "-n")) // Use UNIX new line
        {
            strncpy_n(Tab_stack[file_index].newline, "\n", 1); // Avoid any type of buffer overflows
            shiftargc++;
        }
        else if (!strcmp(argv[arg_parse], "--lfwin") || !strcmp(argv[arg_parse], "-w")) // Use Windows new line
        {
            strncpy_n(Tab_stack[file_index].newline, "\r\n", 2);
        }
        else if (!strcmp(argv[arg_parse], "--converttab") || !strcmp(argv[arg_parse], "-t")) // Convert tabs to spaces
        {
            convertTabtoSpaces = true;
        }
        else if (!strcmp(argv[arg_parse], "--devmode") || !strcmp(argv[arg_parse], "-d")) // Enable dev mode
        {
            dev_tools = true;
        }
        else if (!strcmp(argv[arg_parse], "--menucolor") || !strcmp(argv[arg_parse], "-m")) // Foreground color parameter
        {
            if (argv[arg_parse + 1] != NULL)
            {
                fg_color = hexstrtodec(argv[arg_parse + 1]);
                if (fg_color > 0x0F || fg_color < 0)
                {
                    fprintf(stderr, "%s\n", NEWTRODIT_ERROR_INVALID_COLOR);
                    return 1;
                }
                fg_color = (fg_color * 16) % 256;

                arg_parse++;
            }
            else
            {
                fprintf(stderr, "%s\n", NEWTRODIT_ERROR_MISSING_ARGUMENT);
                return 1;
            }
        }
        else if (!strcmp(argv[arg_parse], "--fontcolor") || !strcmp(argv[arg_parse], "-f")) // Foreground color parameter
        {
            if (argv[arg_parse + 1] != NULL)
            {
                bg_color = hexstrtodec(argv[arg_parse + 1]);
                if (bg_color > 0x0F || bg_color < 0)
                {
                    fprintf(stderr, "%s\n", NEWTRODIT_ERROR_INVALID_COLOR);
                    return 1;
                }
                bg_color = bg_color % 256;
                arg_parse++;
            }
            else
            {
                fprintf(stderr, "%s\n", NEWTRODIT_ERROR_MISSING_ARGUMENT);
                return 1;
            }
        }
        else
        {
            file_arguments_count++;
            file_arguments[file_arguments_count] = arg_parse;
        }
    }
    if (file_arguments_count > 0)
    {
        open_files = file_arguments_count;

        for (int i = 1; i <= file_arguments_count; i++)
        {

            if (strlen(argv[file_arguments[i]]) > MAX_PATH)
            {
                fprintf(stderr, "%s%s", NEWTRODIT_FS_FILE_NAME_TOO_LONG, argv[file_arguments[i]]);
                WriteLogFile(join(NEWTRODIT_FS_FILE_NAME_TOO_LONG, argv[file_arguments[i]]));
                ExitRoutine(ENAMETOOLONG);
            }
            Tab_stack[file_index].filename = argv[file_arguments[i]];
            open_argv = fopen(Tab_stack[file_index].filename, "rb");
            if (!CheckFile(Tab_stack[file_index].filename)) // File exists
            {
                if (!open_argv)
                {

                    fprintf(stderr, "%s%s\n", substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename)); // Hard-coded hack
                    WriteLogFile(join(NEWTRODIT_FS_FILE_OPEN_ERR, substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename)));
                    ExitRoutine(errno);
                }
            }
            else
            {
                fprintf(stderr, "%s%s\n", NEWTRODIT_FS_FILE_NOT_FOUND, Tab_stack[file_index].filename);
                WriteLogFile(join(NEWTRODIT_FS_FILE_NOT_FOUND, Tab_stack[file_index].filename));
                ExitRoutine(errno);
            }
            fseek(open_argv, 0, SEEK_SET);
            if (CountLines(open_argv) > BUFFER_Y) // Check if file is too big
            {
                fprintf(stderr, "%s%s", NEWTRODIT_FS_FILE_TOO_LARGE, Tab_stack[file_index].filename);
                WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, Tab_stack[file_index].filename));

                return EFBIG; // File too big
            }
            if (LoadFile(&Tab_stack[file_index], Tab_stack[file_index].filename, open_argv) <= -1)
            {
                fprintf(stderr, "%s%s\n", substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename));
                return errno;
            }

            if (file_arguments_count > 1 && i < file_arguments_count) // file_arguments_count - 1 iterations to not allocate another buffer
            {
                file_index++;
                if (!AllocateBufferMemory(&Tab_stack[file_index])) // Allocate more memory for next file
                {
                    printf("%.*s\n", wrapSize, NEWTRODIT_ERROR_OUT_OF_MEMORY);
                    ExitRoutine(ENOMEM);
                }
            }
        }

        n2 = LINECOUNT_WIDE;
        LINECOUNT_WIDE = 0;

        LoadAllNewtrodit();
        LINECOUNT_WIDE = n2;

        CenterText(strlasttok(Tab_stack[file_index].filename, PATHTOKENS), 0);
        DisplayTabIndex(&Tab_stack[file_index]);
        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
    }
    else
    {
        LoadAllNewtrodit();
    }

    clearAllBuffer = false;

    if (partialMouseSupport)
    {
        WriteLogFile("Mouse support enabled");
    }
    while (1)
    {

        UpdateTitle(&Tab_stack[file_index]);

        old_y_size = YSIZE;
        old_x_size = XSIZE;

        SetDisplayY(&Tab_stack[file_index]);

        if (lineCount && !isprint(ch))
        {
            DisplayLineCount(&Tab_stack[file_index], YSIZE - 3, Tab_stack[file_index].display_y);
        }

        if (c != -2) // Clear bottom line
        {
            DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
        }
        // wrapSize = XSIZE - 2 - ((lineCount ? Tab_stack[file_index].linecount_wide : 0));
        lineCount ? (wrapSize = XSIZE - 1 - Tab_stack[file_index].linecount_wide) : (wrapSize = XSIZE - 1);

        (wrapSize < 0) ? wrapSize = 0 : wrapSize; // Check if wrapSize is negative

        SetDisplayCursorPos(&Tab_stack[file_index]);

        GetFileTime(&Tab_stack[file_index].hFile, &tmpTimeRead, NULL, &tmpTimeWrite);

        ch = GetNewtroditInput(&Tab_stack[file_index]); // Register all input events, not only key presses

        if (Tab_stack[file_index].xpos < 0 || Tab_stack[file_index].ypos < 1)
        {
            PrintBottomString(NEWTRODIT_ERROR_INVALID_POS_RESET);
            WriteLogFile(NEWTRODIT_ERROR_INVALID_POS_RESET);
            getch_n();

            Tab_stack[file_index].xpos = 0;
            Tab_stack[file_index].ypos = 1;
        }
        // WaitForSingleObject(hStdin, INFINITE);
        // ch = getch(); // Get key pressed

        if (CompareFileTime(&tmpTimeWrite, &Tab_stack[file_index].fwrite_time) == 1)
        {
            Tab_stack[file_index].fwrite_time = tmpTimeWrite;
            PrintBottomString(NEWTRODIT_PROMPT_MODIFIED_FILE_DISK);
            if (YesNoPrompt())
            {
                LoadFile(&Tab_stack[file_index], Tab_stack[file_index].filename, open_argv);
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                continue;
            }
        }

        if (c == -2) // Inbound invalid control key
        {
            ShowBottomMenu();
            DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

            gotoxy(Tab_stack[file_index].xpos + relative_xpos[Tab_stack[file_index].ypos] + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].ypos + relative_ypos[Tab_stack[file_index].xpos]);
            c = 0;
        }
        if (!allowAutomaticResizing)
        {
            if (old_x_size != XSIZE || old_y_size != YSIZE) // Check if size has been modified
            {
                n = 0;
                while (!ValidSize()) // At 3 message boxes, close the program
                {
                    if (n == 2)
                    {
                        ClearScreen();
                        fprintf(stderr, "%s\n", NEWTRODIT_ERROR_WINDOW_TOO_SMALL);
                        ExitRoutine(1);
                    }
                    n++;
                }
                clearAllBuffer = true;

                LoadAllNewtrodit();
                clearAllBuffer = false;

                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
            }
        }

        if (ch == 3 || ch == 11) // ^C = Copy line to clipboard; ^K = Cut line
        {
            if (!CheckKey(VK_SHIFT))
            {
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] != '\0')
                {
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1);
                    memcpy(GlobalLock(hMem), Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1); // Copy line to the clipboard
                    GlobalUnlock(hMem);
                    OpenClipboard(0);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hMem);
                    CloseClipboard();
                    if (ch == 11 && useOldKeybinds)
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] != '\0')
                        {
                            memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 0, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]));
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline));
                            // ClearPartial(0, display_y, XSIZE, 1);

                            ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y, XSIZE - (lineCount ? Tab_stack[file_index].linecount_wide : 0), 1);
                            Tab_stack[file_index].xpos = 0;
                        }
                    }
                }
                ch = 0;
                continue;
            }
            else
            {
                if (ch == 11)
                {
                    ToggleOption(&useOldKeybinds, NEWTRODIT_OLD_KEYBINDS, true);

                    c = -2;
                    ch = 0;
                    continue;
                }
                if (ch == 3) // S-^C = File compare
                {
                    memset(fcomp1, 0, sizeof(fcomp1));
                    memset(fcomp2, 0, sizeof(fcomp2));
                    ClearPartial(0, YSIZE - 2, XSIZE, 2);
                    printf("%.*s\n", wrapSize, NEWTRODIT_PROMPT_FIRST_FILE_COMPARE);
                    printf("%.*s", wrapSize, NEWTRODIT_PROMPT_SECOND_FILE_COMPARE);
                    gotoxy(strlen(NEWTRODIT_PROMPT_FIRST_FILE_COMPARE), YSIZE - 2);
                    fgets(fcomp1, sizeof(fcomp1), stdin);
                    if (nolflen(fcomp1) <= 0)
                    {
                        FunctionAborted(&Tab_stack[file_index]);
                        continue;
                    }
                    gotoxy(strlen(NEWTRODIT_PROMPT_SECOND_FILE_COMPARE), BOTTOM);

                    fgets(fcomp2, sizeof(fcomp2), stdin);
                    if (nolflen(fcomp2) <= 0)
                    {
                        FunctionAborted(&Tab_stack[file_index]);
                        continue;
                    }

                    fcomp1[strcspn(fcomp1, "\n")] = 0;
                    fcomp2[strcspn(fcomp2, "\n")] = 0;
                    RemoveQuotes(fcomp1, strdup(fcomp1));
                    RemoveQuotes(fcomp2, strdup(fcomp2));

                    LoadAllNewtrodit();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                    if (!strncmp(fcomp1, fcomp2, sizeof(fcomp1)))
                    {
                        PrintBottomString(NEWTRODIT_FS_SAME_FILE);
                        getch_n();
                        ShowBottomMenu();
                        DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                        continue;
                    }

                    ll_n = FileCompare(fcomp1, fcomp2);
                    if (ll_n == -1)
                    {
                        PrintBottomString(NEWTRODIT_FILE_COMPARE_NODIFF);
                    }
                    else if (ll_n >= 0)
                    {
                        PrintBottomString(join(NEWTRODIT_FILE_COMPARE_DIFF, lltoa_n(ll_n)));
                    }
                    else
                    {
                        if (ll_n < -255)
                        {
                            PrintBottomString(join(ErrorMessage(-(ll_n + 256), fcomp2), fcomp2));
                        }
                        else
                        {
                            PrintBottomString(join(ErrorMessage(-(ll_n + 1), fcomp1), fcomp1));
                        }
                    }
                    getch_n();
                    ShowBottomMenu();
                    DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                    ch = 0;
                    continue;
                }
            }
        }

        if (ch == 12) // S-^L = Locate files ; ^L = Toggle line count
        {

            if (!CheckKey(VK_SHIFT))
            {
                ToggleOption(&lineCount, NEWTRODIT_LINE_COUNT, true);
                c = -2;
            }
            else
            {
                // List all files in the directory
                memset(locate_file, 0, sizeof(locate_file));
                PrintBottomString(NEWTRODIT_PROMPT_LOCATE_FILE);
                fgets(locate_file, sizeof locate_file, stdin);
                if (nolflen(locate_file) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }
                locate_file[strcspn(locate_file, "\n")] = 0;

                LoadAllNewtrodit();
                ClearPartial(0, 1, XSIZE, YSIZE - 1);
                LocateFiles(listDir, locate_file, 0);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                ClearPartial(0, 1, XSIZE, YSIZE - 2);
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
            }
            continue;
        }

        if (ch == 7) // ^G = Go to line
        {
            if (CheckKey(VK_SHIFT))
            {
                c = -7;
            }
            ch = 0;
            PrintBottomString(NEWTRODIT_PROMPT_GOTO_LINE);
            line_number_str = TypingFunction('0', '9', goto_len);
            if (atoi(line_number_str) < 1) // Line is less than 1
            {
                PrintBottomString(NEWTRODIT_ERROR_INVALID_YPOS);
                c = -2;

                continue;
            }

            n = Tab_stack[file_index].ypos;
            Tab_stack[file_index].ypos = atoi(line_number_str);

            if (BufferLimit(&Tab_stack[file_index])) // Avoid crashes
            {
                Tab_stack[file_index].ypos = n;

                c = -2;

                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

                continue;
            }

            if (Tab_stack[file_index].ypos != 1 && Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] == '\0' && strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1] + nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1]), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
            {
                PrintBottomString(NEWTRODIT_ERROR_INVALID_YPOS);
                Tab_stack[file_index].ypos = n;
                c = -2;
                continue;
            }
            if (c == -7)
            {
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] == '\0')
                {
                    Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    relative_xpos[Tab_stack[file_index].ypos] = tokcount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "\t");
                }
            }
            else
            {
                Tab_stack[file_index].xpos = 0;
            }

            (Tab_stack[file_index].ypos >= (YSIZE - 3)) ? UpdateScrolledScreen(lineCount, &Tab_stack[file_index]) : UpdateHomeScrolledScreen(lineCount, &Tab_stack[file_index]);

            ShowBottomMenu();
            DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

            continue;
        }
        if (ch == 14) // ^N = New file
        {
            if (!CheckKey(VK_SHIFT))
            {

                if ((n = NewFile(&Tab_stack[file_index])) > 0)
                {
                    PrintBottomString(NEWTRODIT_NEW_FILE_CREATED);
                }
                else
                {
                    ErrorMessage(n, Tab_stack[file_index].filename);
                }
                c = -2;

                ch = 0;
                continue;
            }
            else
            {

                ToggleOption(&convertNull, NEWTRODIT_NULL_CONVERSION, false);

                c = -2;
                ch = 0;
                continue;
            }
        }
        if (ch == 15) // ^O = Open file
        {
            if (!CheckKey(VK_SHIFT)) // Shift not pressed
            {
                if (Tab_stack[file_index].is_modified)
                {
                    PrintBottomString(NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);

                    if (YesNoPrompt())
                    {
                        if (!SaveFile(&Tab_stack[file_index]))
                        {
                            LoadAllNewtrodit();
                            DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                            continue;
                        }
                    }
                }
                PrintBottomString(NEWTRODIT_PROMPT_FOPEN);

                fgets(fileopenread, sizeof fileopenread, stdin);
                if (nolflen(fileopenread) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }

                // Remove trailing LF
                fileopenread[strcspn(fileopenread, "\n")] = 0;
                RemoveQuotes(fileopenread, strdup(fileopenread));

                fileread = fopen(fileopenread, "rb");
                n2 = Tab_stack[file_index].linecount_wide;
                Tab_stack[file_index].linecount_wide = 0;
                n = -errno;
                LoadAllNewtrodit();
                Tab_stack[file_index].linecount_wide = n2;
                /* if (GetFileAttributes(Tab_stack[file_index].filename) & FILE_ATTRIBUTE_DEVICE)
                {
                    PrintBottomString(NEWTRODIT_ERROR_CANNOT_OPEN_DEVICE);
                    c = -2;
                    continue;
                } */

                for (int i = 0; i < open_files; ++i)
                {
                    if (!strcmp(Tab_stack[i].filename, fileopenread) && !Tab_stack[file_index].is_untitled) // File is already open
                    {
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                        PrintBottomString(NEWTRODIT_PROMPT_ALREADY_OPEN_TAB);
                        if (YesNoPrompt())
                        {
                            file_index = i;

                            snprintf(tmp, DEFAULT_BUFFER_X, "%s %s (%d/%d).", NEWTRODIT_SWITCHING_FILE, fileopenread, file_index + 1, open_files);

                            LoadAllNewtrodit();
                            DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                            PrintBottomString(tmp);
                            c = -2;
                            continue;
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
                    if (!fileread || (n = LoadFile(&Tab_stack[file_index], strdup(fileopenread), fileread)) != 1) // Failed to open the file
                    {
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                        PrintBottomString(ErrorMessage(abs(n), fileopenread)); // Errors are always negative, so abs() is used
                        getch_n();
                        ShowBottomMenu();

                        DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

                        continue;
                    }

                    old_open_files[oldFilesIndex] = Tab_stack[file_index].filename;
                    NewtroditNameLoad();
                    CenterText(strlasttok(Tab_stack[file_index].filename, PATHTOKENS), 0);
                    DisplayTabIndex(&Tab_stack[file_index]);
                    RightAlignNewline();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                }

                fclose(fileread);
                ch = 0;
                continue;
            }
        }

        if (ch == 20) // S-^T = Toggle tab conversion
        {
            /* if (CheckKey(VK_SHIFT) && dev_tools)
            {
                ToggleOption(&convertTabtoSpaces, NEWTRODIT_TAB_CONVERSION, false);
                c = -2;

                ch = 0;
                continue;
            } */
            if (CheckKey(VK_SHIFT))
            {
                if (old_open_files[0][0] != '\0' && oldFilesIndex > 0)
                {
                    if (Tab_stack[file_index].is_modified)
                    {
                        PrintBottomString(NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);

                        if (YesNoPrompt())
                        {
                            if (SaveFile(&Tab_stack[file_index]))
                            {
                                LoadAllNewtrodit();
                                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                                continue;
                            }
                        }
                    }
                    PrintBottomString(NEWTRODIT_PROMPT_REOPEN_FILE);
                    if (YesNoPrompt())
                    {
                        if (LoadFile(&Tab_stack[file_index], old_open_files[oldFilesIndex], fileread))
                        {
                            strncpy_n(old_open_files[oldFilesIndex], Tab_stack[file_index].filename, MAX_PATH);
                            LoadAllNewtrodit();
                            DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                        }
                    }
                }
                continue;
            }
        }

        if (ch == 18) // ^R = Reload file and S-^R = Reload settings
        {
            if (CheckKey(VK_SHIFT)) // S-^R = Reload settings
            {
                PrintBottomString(NEWTRODIT_PROMPT_RELOAD_SETTINGS);
                if (YesNoPrompt())
                {
                    if (LoadSettings(settings_file, run_macro, &sigsegvScreen, &lineCount, &dev_tools, &Tab_stack[file_index]) == 0) // Reload settings
                    {
                        LoadAllNewtrodit();
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                        PrintBottomString(NEWTRODIT_SETTINGS_RELOADED);
                    }
                    else
                    {
                        PrintBottomString(NEWTRODIT_ERROR_RELOAD_SETTINGS);
                    }
                    getch_n();
                }
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                ShowBottomMenu();
            }
            else
            {

                // ^R = Reload file
                PrintBottomString(NEWTRODIT_PROMPT_RELOAD_FILE);

                if (YesNoPrompt())
                {
                    open_argv = fopen(Tab_stack[file_index].filename, "rb");

                    ReloadFile(&Tab_stack[file_index], open_argv);
                    if (GetFileAttributes(Tab_stack[file_index].filename) & FILE_ATTRIBUTE_READONLY)
                    {
                        PrintBottomString(NEWTRODIT_WARNING_READONLY_FILE);
                        c = -2;
                    }
                }
                else
                {
                    ShowBottomMenu();
                }
            }
            ch = 0;

            continue;
        }

        if (ch == 0xE0) // Special keys: 224
        {

            ch = getch();
            switch (ch)
            {

            case 72:
                // Up arrow
                if (Tab_stack[file_index].ypos > 1)
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1][Tab_stack[file_index].xpos] == '\0')
                    {
                        Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[--Tab_stack[file_index].ypos]);
                    }
                    else
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1][Tab_stack[file_index].xpos] == 9)
                        {
                            Tab_stack[file_index].xpos += relative_xpos[Tab_stack[file_index].ypos - 1];
                        }
                        --Tab_stack[file_index].ypos;
                    }
                    /* if(Tab_stack[file_index].display_y >= YSIZE - 2 && Tab_stack[file_index].display_y > 1)
                    {
                        Tab_stack[file_index].display_y--; // Scroll up
                    } */
                }
                UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                break;

            case 75:
                // Left arrow

                if (Tab_stack[file_index].xpos >= 1)
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos - 1] == 9)
                    {
                        relative_xpos[Tab_stack[file_index].ypos] -= TAB_WIDE;
                    }
#ifdef HORIZONTAL_SCROLL
                    if (horizontalScroll > 0)
                    {
                        horizontalScroll--;
                        UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                    }
#endif
                    --Tab_stack[file_index].xpos;
                }
                else
                {
                    if (Tab_stack[file_index].ypos > 1)
                    {

                        Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[--Tab_stack[file_index].ypos]);
#ifdef HORIZONTAL_SCROLL

                        horizontalScroll = Tab_stack[file_index].xpos - wrapSize;
                        if (horizontalScroll < 0)
                        {
                            horizontalScroll = 0;
                        }
#endif

                        UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                    }
                }

                break;
            case 77:
                // Right arrow
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] != '\0')
                {
                    if (Tab_stack[file_index].xpos == nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0' || !strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + Tab_stack[file_index].xpos, Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
                        {
                            if (Tab_stack[file_index].ypos < BUFFER_Y - 1)
                            {
                                Tab_stack[file_index].xpos = 0;
                                Tab_stack[file_index].ypos++;
#ifdef HORIZONTAL_SCROLL

                                if (horizontalScroll > 0)
                                {
                                    horizontalScroll = 0;
                                    ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), 1, XSIZE - (lineCount ? Tab_stack[file_index].linecount_wide : 0), YSIZE - 2);
                                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                                }
                                else
#endif
                                {
                                    UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                                }
                            }
                        }
                    }
                    else
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] == 9)
                        {

                            relative_xpos[Tab_stack[file_index].ypos] += TAB_WIDE;
                        }
                        Tab_stack[file_index].xpos++;

                        if (Tab_stack[file_index].xpos > wrapSize - 1)
                        {
#ifdef HORIZONTAL_SCROLL
                            horizontalScroll++;
#endif
                            ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), 1, wrapSize, YSIZE - 2);

                            DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                        }
#ifdef HORIZONTAL_SCROLL

                        else
                        {
                            if (horizontalScroll != 0)
                            {
                                horizontalScroll = 0;
                                ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), 1, wrapSize, YSIZE - 2);
                                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                            }
                        }
#endif
                    }
                }
                if (BufferLimit(&Tab_stack[file_index]))
                {
                    ShowBottomMenu();
                    continue;
                }

                break;

            case 80:
                // Down arrow
                n2 = Tab_stack[file_index].ypos;
                if (Tab_stack[file_index].ypos < BUFFER_Y - 1)
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0')
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][Tab_stack[file_index].xpos] == '\0')
                        {
                            SetConsoleTitle("Executed DOWN ARROW");
                            getch_n();
                            Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[++Tab_stack[file_index].ypos]) + (tokcount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1], "\t") * TAB_WIDE); // Add tab wide
                        }
                        else
                        {
                            n = tokcount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1] - Tab_stack[file_index].xpos, "\t");

                            Tab_stack[file_index].ypos++;
                        }
                    }
                    else
                    {
                        if (!strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
                        {
                            Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1]);
                            Tab_stack[file_index].ypos++;
                        }
                        break;
                    }
                }
                else
                {
                    break;
                }

                if (BufferLimit(&Tab_stack[file_index]))
                {
                    Tab_stack[file_index].ypos = n2; // Restore ypos if a position outside the buffer is reached
                    ShowBottomMenu();
                    continue;
                }
                UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);

                break;

            case 71:
                // HOME key
                Tab_stack[file_index].xpos = 0;
#ifdef HORIZONTAL_SCROLL

                if (horizontalScroll > 0)
                {
                    horizontalScroll = 0;
                    UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                }
#endif
                break;
            case 79:
                // END key
                n = Tab_stack[file_index].xpos;
                Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                if (n == Tab_stack[file_index].xpos) // Optimization
                {
                    break;
                }
#ifdef HORIZONTAL_SCROLL

                horizontalScroll = Tab_stack[file_index].xpos - wrapSize;
                if (horizontalScroll <= 0)
                {
                    horizontalScroll = 0;
                }
                else
#endif
                {
                    UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                }
                break;
            case 117:
                // ^END key
                for (int i = 1; i < BUFFER_Y; i++)
                {
                    if (Tab_stack[file_index].strsave[i][0] == '\0')
                    {
                        Tab_stack[file_index].ypos = i;

                        if (i > 1)
                        {
                            Tab_stack[file_index].ypos = i - 1;
                        }

                        Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
#ifdef HORIZONTAL_SCROLL

                        horizontalScroll = Tab_stack[file_index].xpos - wrapSize;
                        if (horizontalScroll < 0)
                        {
                            horizontalScroll = 0;
                        }
#endif

                        break;
                    }
                }
                UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);

                break;
            case 119:
                // ^HOME key
                horizontalScroll = 0;
                UpdateHomeScrolledScreen(lineCount, &Tab_stack[file_index]);
                Tab_stack[file_index].xpos = 0;
                Tab_stack[file_index].ypos = 1;

                break;

            case 82:
                // INS key
                insertChar = !insertChar;
                if (cursorSizeInsert)
                {
                    insertChar ? CursorSettings(true, 80) : CursorSettings(true, CURSIZE);
                }

                break;

            case 83: // DEL and S-DEL

                if (CheckKey(VK_SHIFT)) // S-DEL
                {
                    if (Tab_stack[file_index].ypos > 0)
                    {
                        memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 0, BUFFER_X);

                        ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y, XSIZE, 1);
                        strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline));
                        Tab_stack[file_index].xpos = 0;
                    }
                }
                else
                {                                                                              // DEL key
                    if (strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) > 0) // TODO: Fix this bug in the line number 1
                    {
                        undo_stack_line = Tab_stack[file_index].ypos;
                        tmp = delete_char(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].xpos);
                        if (Tab_stack[file_index].xpos < nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                        {
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                            ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].display_y, 1, 1);
                            gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);

                            print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                        }
                        else
                        {

                            n = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                            memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + n, 0, BUFFER_X - n);                                                                                                                   // Empty the new line
                            strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1])); // Concatenate the next line
                            if (Tab_stack[file_index].ypos != 1)
                            {
                                strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // Add the newline string only if the ypos isn't 1
                            }
                            delete_row(Tab_stack[file_index].strsave + 1, Tab_stack[file_index].ypos, BUFFER_X); // Delete the old row, shifting other rows up
                            Tab_stack[file_index].linecount--;
                            if (!UpdateScrolledScreen(lineCount, &Tab_stack[file_index]))
                            {
                                ClearPartial(0, 1, XSIZE, YSIZE - 2);
                                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                            }
                        }
                    }
                }

                break;

            case 133: // F11 (fullscreen)

                if (dev_tools)
                {
                    isFullScreen = !isFullScreen;
                    /* if (isFullScreen)
                    {
                        if (!SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), 1, NULL))
                        {
                            printf("Failed to set console display mode - (%d)\n", GetLastError());
                        }
                    }
                    else
                    {
                        if (!SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), 2, NULL))
                        {
                            printf("Failed to set console display mode - (%d)\n", GetLastError());
                        }
                    } */
                    ch = 0;
                    isFullScreen ? SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT) : SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
                    if (!isFullScreen)
                    {
                        ShowWindow(GetConsoleWindow(), SW_MAXIMIZE); // Show the window in fullscreen
                    }
                    else
                    {
                        ShowWindow(GetConsoleWindow(), SW_RESTORE); // Show the window in fullscreen
                    }
                    ch = 0;
                }

                break;

            case 134: // F12
                if (dev_tools)
                {
                    printf("[%d:%d %d:%d]", XSIZE, YSIZE, old_x_size, old_y_size);
                }

                break;
            }

            if (ch != 83)
            {
                ch = 0;
            }
            else
            {
                c = -1; // For undo stack
            }
            continue;
        }
        if (ch == 13 && !CheckKey(VK_RETURN) && !_NEWTRODIT_OLD_SUPPORT) // ^M = Toggle mouse
        {
            if (!CheckKey(VK_SHIFT))
            {
                ToggleOption(&partialMouseSupport, NEWTRODIT_MOUSE, true);
                c = -2;
                ch = 0;
                continue;
            }
        }
        if ((ch == 13 && CheckKey(VK_RETURN)) || (ch == 13 && _NEWTRODIT_OLD_SUPPORT)) // Newline character: CR (13)
        {
            if (Tab_stack[file_index].ypos < Tab_stack[file_index].bufy - 1)
            {
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0' || Tab_stack[file_index].xpos < nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                {
                    /*  n = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1;
                     printf("\n[%d]", n);
                     Tab_stack[file_index].Ustack = (Undo_stack *)realloc(Tab_stack[file_index].Ustack, sizeof(Undo_stack) * (sizeof(&Tab_stack[file_index].Ustack) + 1));
                     Tab_stack[file_index].Ustack->size = n;

                     Tab_stack[file_index].Ustack->line = (char *)malloc(Tab_stack[file_index].Ustack->size);
                     memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);
                     Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); // If it's not a duplicate, the value will change
                     Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                     Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                     Tab_stack[file_index].Ustack->create_nl = true;
                     Tab_stack[file_index].Ustack++->delete_nl = false; */
                    insert_new_row(&Tab_stack[file_index], &Tab_stack[file_index].xpos, &Tab_stack[file_index].ypos, Tab_stack[file_index].display_y, BUFFER_X, Tab_stack[file_index].newline);
                }
                else

                {

                    strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos++], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // Add newline to current line

                    UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                }
                Tab_stack[file_index].linecount++; // Increment line count
                Tab_stack[file_index].xpos = 0;
                Tab_stack[file_index].is_modified = true;
            }

            continue;
        }
        if (ch == 5) // ^E = Toggle syntax highlighting / S-^E = Set syntax highlighting rules file
        {
            if (CheckKey(VK_SHIFT))
            {
                PrintBottomString(NEWTRODIT_PROMPT_SYNTAX_FILE);

                memset(syntaxfile, 0, sizeof syntaxfile);
                fgets(syntaxfile, sizeof syntaxfile, stdin);
                if (nolflen(syntaxfile) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }
                LoadAllNewtrodit();
                syntaxfile[strcspn(syntaxfile, "\n")] = 0;
                syntax = fopen(syntaxfile, "rb");
                if (!syntax)
                {
                    ErrorMessage(errno, syntaxfile);
                }
                else
                {
                    EmptySyntaxScheme(&Tab_stack[file_index]);
                    if ((syntaxKeywordsSize = LoadSyntaxScheme(syntax, syntaxfile, &Tab_stack[file_index])) != 0) // Change keywords size
                    {
                        syntaxHighlighting = true;
                        PrintBottomString(NEWTRODIT_SYNTAX_HIGHLIGHTING_LOADED);
                        Tab_stack[file_index].Syntaxinfo.syntax_file = full_path(syntaxfile);
                        Tab_stack[file_index].Syntaxinfo.keyword_count = syntaxKeywordsSize;
                    }
                }
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                ch = 0;
                continue;
            }
            else
            {
                ToggleOption(&syntaxHighlighting, NEWTRODIT_SYNTAX_HIGHLIGHTING, true);
                c = -2;
            }

            ch = 0;
            continue;
        }
        if (ch == 6) // ^F = Find string
        {

            // Empty values
            memset(find_string, 0, sizeof find_string);
            find_string_index = 0;
            find_count = 0;
            n = 0, n2 = Tab_stack[file_index].ypos;

            c = 0;

            findInsensitive = false;
            if (CheckKey(VK_SHIFT))
            {
                findInsensitive = true;
                PrintBottomString(NEWTRODIT_PROMPT_FIND_STRING_INSENSITIVE);
            }
            else
            {
                PrintBottomString(NEWTRODIT_PROMPT_FIND_STRING);
            }

            fgets(find_string, sizeof find_string, stdin);
            if (nolflen(find_string) <= 0)
            {
                FunctionAborted(&Tab_stack[file_index]);
                continue;
            }
            find_string[strcspn(find_string, "\n")] = '\0';
            LoadAllNewtrodit();
            DisplayFileContent(&Tab_stack[file_index], stdout, 0);

            CountBufferLines(&Tab_stack[file_index]); // Count lines in the buffer

            for (int i = 1; i < Tab_stack[file_index].linecount; i++)
            {
                if (Tab_stack[file_index].strsave[i][0] == '\0')
                {
                    break;
                }

                find_string_index = 0;
                while (find_string_index >= 0 && c != 27)
                {
                    find_string_index = ReturnFindIndex(findInsensitive, Tab_stack[file_index].strsave[i] + n, find_string);

                    if (find_string_index >= 0)
                    {
                        find_count++;
                        n2 = i; //
                        n += find_string_index;
                        Tab_stack[file_index].ypos = i;

                        SetDisplayY(&Tab_stack[file_index]); // Calculate scroll position
                        // if(Tab_stack[file_index].scrolled_y)
                        {
                            UpdateScrolledScreen(lineCount, &Tab_stack[file_index]);
                        }
                        gotoxy(find_string_index + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                        SetColor(0x0e);
                        printf("%s", substring(find_string_index, strlen(find_string), Tab_stack[file_index].strsave[i])); // Print found string using substrings
                        SetColor(bg_color);
                        print_line(Tab_stack[file_index].strsave[i] + find_string_index + strlen(find_string));
                        ShowFindMenu();
                        gotoxy(find_string_index + (lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(find_string), Tab_stack[file_index].display_y);

                        c = getch_n();
                        if (c == 27)
                        {
                            break;
                        }
                        else if (c == 316) // F3
                        {

                            n = 0;
                            if (i < Tab_stack[file_index].bufy || i > Tab_stack[file_index].linecount)
                            {
                                i++;
                            }
                            else
                            {

                                PrintBottomString(NEWTRODIT_FIND_NO_MORE_MATCHES);
                                getch_n();
                                Tab_stack[file_index].ypos = n2; // Restore ypos to last found position
                            }
                        }
                        else if (c == 317) // F4
                        {
                            findInsensitive = !findInsensitive;
                            PrintBottomString(join(NEWTRODIT_FIND_CASE_SENSITIVE, findInsensitive ? NEWTRODIT_DIALOG_ENABLED : NEWTRODIT_DIALOG_DISABLED));
                            gotoxy(find_string_index + (lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(find_string), Tab_stack[file_index].display_y); // Keep the cursor in the same place

                            getch_n();
                        }
                        else if (c == 362) // A-F4
                        {
                            QuitProgram(SInf.color);
                        }
                    }
                }
                Tab_stack[file_index].xpos = find_string_index + strlen(find_string);
                Tab_stack[file_index].ypos = n2;
                if (c == 27)
                {
                    break;
                }
            }
            ShowBottomMenu();

            ch = 0;
            continue;
        }
        if (ch == 0)
        {
            ch = getch();
            switch (ch)
            {
            case 19: // ^A-R (ROT13)
                memset(temp_strsave, 0, BUFFER_X);
                strncpy_n(temp_strsave, Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]));
                if (rot13(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                {
                    ClearPartial(0, Tab_stack[file_index].display_y, XSIZE, 1);
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
                break;
            case 22: // A-^U (Uppercase)
                for (int i = 0; i < strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); i++)
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] = toupper(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i]);
                }
                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                break;
            case 32: // A-^D (Debug memory)
                if (dev_tools)
                {
                    printf("%p,%p\n", &Tab_stack[file_index].xpos, &Tab_stack[file_index].ypos);
                    n = 0;
                    printf("Address to change: ");
                    scanf("%x", &n);
                    printf("Value to change to: ");
                    scanf("%x", &n2);
                    // Change the specified address n to n2
                    *(int *)n = n2;
                    break;
                }

            case 38: // A-^L (Lowercase)
                for (int i = 0; i < strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); i++)
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] = tolower(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i]);
                }
                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                break;
            case 49: // A-^N (New file and save)

                if (CheckKey(VK_MENU))
                {

                    NewFile(&Tab_stack[file_index]);
                    PrintBottomString(NEWTRODIT_NEW_FILE_CREATED);
                    getch_n();

                    SaveFile(&Tab_stack[file_index]);

                    DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);

                    ShowBottomMenu();
                }
                break;

            case 59: // F1 key
                NewtroditHelp();

                LoadAllNewtrodit();

                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                break;
            case 60: // F2 key
                if (Tab_stack[file_index].is_untitled || !Tab_stack[file_index].is_saved)
                {
                    SaveFile(&Tab_stack[file_index]);
                }

                PrintBottomString(NEWTRODIT_PROMPT_RENAME_FILE);
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = 0;
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                if (nolflen(newname) > 0)
                {
                    if (!CheckFile(newname))
                    {
                        PrintBottomString(NEWTRODIT_PROMPT_OVERWRITE);
                        if (YesNoPrompt())
                        {
                            if (remove(newname) != 0)
                            {
                                PrintBottomString(NEWTRODIT_FS_FILE_DELETE);
                                getch_n();
                                ShowBottomMenu();
                                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                                continue;
                            }
                        }
                    }
                    if (MoveFile(Tab_stack[file_index].filename, newname))
                    {
                        PrintBottomString(join(NEWTRODIT_FILE_RENAMED, newname));
                        strncpy_n(Tab_stack[file_index].filename, newname, MAX_PATH);
                        NewtroditNameLoad();
                        CenterText(strlasttok(Tab_stack[file_index].filename, PATHTOKENS), 0);
                        DisplayTabIndex(&Tab_stack[file_index]);

                        RightAlignNewline();
                    }
                    else
                    {
                        PrintBottomString(NEWTRODIT_FS_FILE_RENAME);
                    }

                    getch_n();
                }
                else
                {
                    PrintBottomString(NEWTRODIT_FUNCTION_ABORTED);
                    getch_n();
                }
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                ch = 0;
                break;
            case 63: // F5 key = Run macro
                if (!run_macro)
                {
                    if (!Tab_stack[file_index].is_untitled && Tab_stack[file_index].is_saved)
                    {

                        GetFullPathName(Tab_stack[file_index].filename, sizeof(Tab_stack[file_index].filename), tmp, NULL);
                        StartProcess(tmp);
                    }
                }
                else
                {
                    tmp = strdup(run_macro);
                    tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_FILE, strlasttok(Tab_stack[file_index].filename, PATHTOKENS), &n);
                    if (Tab_stack[file_index].is_untitled)
                    {
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_FULL_PATH, Tab_stack[file_index].filename, &n);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_DIR, SInf.dir, &n);
                    }
                    else
                    {
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_FULL_PATH, full_path(Tab_stack[file_index].filename), &n);
                        get_path_directory(full_path(Tab_stack[file_index].filename), ptr);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_DIR, ptr, &n);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_EXTENSION, strlasttok(Tab_stack[file_index].filename, "."), &n);
                    }
                    StartProcess(tmp);
                }

                ch = 0;
                break;
            case 64: // F6 key = Insert date and time

                temp_strsave = GetTime(showMillisecondsInTime);

                if (Tab_stack[file_index].xpos >= BUFFER_X - BUFFER_INCREMENT)
                {
                    tmp = realloc_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], BUFFER_X, BUFFER_X + BUFFER_INCREMENT);
                    BUFFER_X += BUFFER_INCREMENT;
                    if (!tmp)
                    {
                        PrintBottomString(NEWTRODIT_ERROR_OUT_OF_MEMORY);
                        getch_n();

                        SaveFile(&Tab_stack[file_index]);
                        return ENOMEM;
                    }
                    free(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] = tmp;
                }

                insert_str(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, Tab_stack[file_index].xpos);

                Tab_stack[file_index].xpos += strlen(temp_strsave);

                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);

                if (Tab_stack[file_index].xpos <= XSIZE)
                {
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }

                break;
            case 88: // S-F5 (Set macro)
                PrintBottomString(NEWTRODIT_PROMPT_CREATE_MACRO);
                fgets(macro_input, sizeof(macro_input), stdin);
                if (nolflen(macro_input) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    break;
                }
                macro_input[strcspn(macro_input, "\n")] = 0;

                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                if (!ValidString(macro_input))
                {
                    PrintBottomString(NEWTRODIT_ERROR_INVALID_MACRO);
                    getch_n();
                    ShowBottomMenu();
                    DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                    break;
                }

                strncpy_n(run_macro, macro_input, MAX_PATH);
                PrintBottomString(join(NEWTRODIT_MACRO_SET, macro_input));
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                break;
            case 69: // F9 = Compile
                StartProcess(join(join(Tab_stack[file_index].Compilerinfo.path, " "), Tab_stack[file_index].Compilerinfo.flags));
                break;
            case 68:                            // F10 key
                StartProcess("explorer.exe ."); // Open current directory in explorer
                break;
            case 93:                     // S-F10 key
                StartProcess("cmd.exe"); // Open command prompt
                break;
            case 94: // ^F1 key
                LoadAllNewtrodit();
                n = strlen(join("Contribute at ", newtrodit_repository));
                ClearPartial(0, BOTTOM, XSIZE, 1); // Clear bottom line where "For help, press F1" is displayed
                DisplayCursor(false);
                SetColor(fg_color);
                ClearPartial((XSIZE / 2) - (n / 2) - 1, (YSIZE / 2) - 3, n + 2, 7); // Create a box
                CenterText("About Newtrodit", (YSIZE / 2) - 2);
                CenterText(ProgInfo(), (YSIZE / 2));
                // I know it's not the best way to do it, but it works
                CenterText(join("Contribute at ", newtrodit_repository), (YSIZE / 2) + 2);
                getch_n();
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                break;
            case 97: // ^F4
                if (!CheckKey(VK_SHIFT))
                {

                    NewFile(&Tab_stack[file_index]);
                    PrintBottomString(NEWTRODIT_FILE_CLOSED);
                    c = -2;
                    ch = 0;
                    break;
                }

            case 107: // A-F4 key
                if (!CheckKey(VK_CONTROL))
                {
                    QuitProgram(SInf.color);
                    ShowBottomMenu();
                    continue;
                }
                break;

            case 148: // ^TAB / S-^TAB (Switch file)

                if (open_files > 1)
                {
                    if (CheckKey(VK_SHIFT))
                    {

                        n = -1; // Switch to the next file
                    }
                    else
                    {

                        n = +1; // Switch to the previous file
                    }
                    if ((file_index + n >= MAX_TABS) || (file_index + n >= open_files))
                    {
                        file_index = 0;
                    }
                    else if (file_index + n < 0)
                    {
                        file_index = open_files - 1;
                    }
                    else
                    {
                        file_index += n;
                    }
                    if (n == -1)
                    {
                        snprintf(tmp, DEFAULT_BUFFER_X, "%s (%d/%d).", NEWTRODIT_SHOWING_PREVIOUS_FILE, file_index + 1, open_files);
                    }
                    else
                    {
                        snprintf(tmp, DEFAULT_BUFFER_X, "%s (%d/%d).", NEWTRODIT_SHOWING_NEXT_FILE, file_index + 1, open_files);
                    }
                    LoadAllNewtrodit();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                    PrintBottomString(tmp);
                    c = -2;
                }
                else
                {
                    PrintBottomString(NEWTRODIT_INFO_NO_FILES_TO_SWITCH);
                    getch_n();
                    ShowBottomMenu();
                    DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                }
                break;

                /*                 if (CheckKey(VK_SHIFT))
                                {
                                    if (file_index > 0)
                                    {

                                        file_index--;

                                        LoadAllNewtrodit();
                                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                                        PrintBottomString(tmp);
                                        c = -2;
                                    }
                                    else
                                    {
                                        PrintBottomString(NEWTRODIT_INFO_NO_FILES_TO_SWITCH);
                                        getch_n();
                                        ShowBottomMenu();
                                        DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                                    }
                                }
                                else
                                {
                                    if (file_index < MAX_TABS - 1 && file_index < open_files - 1)
                                    {

                                        file_index++;
                                        snprintf(tmp, DEFAULT_BUFFER_X, "%s (%d/%d).", NEWTRODIT_SHOWING_NEXT_FILE, file_index + 1, open_files);

                                        LoadAllNewtrodit();
                                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                                        PrintBottomString(tmp);
                                        c = -2;
                                    }
                                    else
                                    {
                                        PrintBottomString(NEWTRODIT_INFO_NO_FILES_TO_SWITCH);
                                        getch_n();
                                        ShowBottomMenu();
                                        DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                                    }
                                } */

            case 151: // A-HOME key "smart home" (go to the first non-whitespace character)
                Tab_stack[file_index].xpos = strspn(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], " \t");
                break;
            case 159: // A-END key "smart end" (go to the last non-whitespace character)
                n = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);

                while ((Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] == ' ' || Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] == '\t') && Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] != '\0')
                {
                    n--;
                }

                Tab_stack[file_index].xpos = n + (tokcount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "\t") * TAB_WIDE);
                if (Tab_stack[file_index].xpos < 0)
                {
                    Tab_stack[file_index].xpos = 0;
                }

                break;
            }

            ch = 0;
            continue;
        }

        if (ch == 19) // ^S
        {
            (CheckKey(VK_SHIFT)) ? (n = 2) : (n = 0);
            memset(save_dest, 0, MAX_PATH);
            strncpy_n(save_dest, Tab_stack[file_index].filename, MAX_PATH); // If no input is given
            if (!Tab_stack[file_index].is_saved || n == 2)
            {
                !Tab_stack[file_index].is_saved ? PrintBottomString(NEWTRODIT_PROMPT_SAVE_FILE) : PrintBottomString(NEWTRODIT_PROMPT_SAVE_FILE_AS);
                fgets(save_dest, MAX_PATH, stdin);

                if (nolflen(save_dest) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }
                save_dest[strcspn(save_dest, "\n")] = 0;
                RemoveQuotes(save_dest, strdup(save_dest));

                if (!CheckFile(save_dest))
                {
                    LoadAllNewtrodit();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                    PrintBottomString(NEWTRODIT_PROMPT_OVERWRITE);

                    if (!YesNoPrompt())
                    {
                        ShowBottomMenu();
                        DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                        continue;
                    }
                }
            }
            if (Tab_stack[file_index].is_readonly && !strcmp(save_dest, Tab_stack[file_index].filename))
            {
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                PrintBottomString(NEWTRODIT_FS_READONLY_SAVE);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                continue;
            }
            fp_savefile = fopen(save_dest, "wb"); // (Re)open the file to write
            if (!fp_savefile || !ValidFileName(save_dest))
            {
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                PrintBottomString(NEWTRODIT_FS_FILE_SAVE_ERR);
                getch_n();
                ShowBottomMenu();
                continue;
            }

            LoadAllNewtrodit();
            DisplayFileContent(&Tab_stack[file_index], stdout, 0);

            if (WriteBuffer(fp_savefile, &Tab_stack[file_index])) // Write the file content
            {
                Tab_stack[file_index].is_saved = true;
                Tab_stack[file_index].is_modified = false;
                Tab_stack[file_index].is_untitled = false;

                UpdateTitle(&Tab_stack[file_index]);
                strncpy_n(Tab_stack[file_index].filename, save_dest, MAX_PATH);
                NewtroditNameLoad();

                CenterText(strlasttok(Tab_stack[file_index].filename, PATHTOKENS), 0);
                // RightAlignNewline();
                // DisplayTabIndex(&Tab_stack[file_index]);

                PrintBottomString(NEWTRODIT_FILE_SAVED);

                c = -2;
            }
            else
            {
                PrintBottomString(NEWTRODIT_FS_FILE_SAVE_ERR);

                getch_n();
            }

            fclose(fp_savefile);

            ch = 0;
            continue;
        }
        if (ch == 22) // ^V
        {
            if (OpenClipboard(0))
            {
                buffer_clipboard = (char *)GetClipboardData(CF_TEXT);
                if (buffer_clipboard != NULL)
                {
                    n = 0;
                    // Split the string with the newline character as delimiter and print them separately
                    ptr = strtok(buffer_clipboard, "\n");
                    tmp = (char *)calloc(strlen(ptr) + 1, sizeof(char)); // Allocate memory and initialize all bytes to 0
                    while (ptr != NULL)
                    {
                        for (int i = 0; i < strlen(ptr); i++) // i is also character 9 :)
                        {
                            tmp = insert_char(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], ptr[i], Tab_stack[file_index].xpos - 1);
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                        }
                        printf("%s\n", tmp);
                        ptr = strtok(NULL, "\n");
                        n++;
                    }

                    Tab_stack[file_index].xpos = nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
            }
            else
            {
                PrintBottomString(NEWTRODIT_ERROR_CLIPBOARD_COPY);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
            }
            CloseClipboard();
            ch = 0;
            continue;
        }

        if (ch == 4) // ^D (Debug tool/dev mode) / S-^D = Toggle dev mode
        {
            ch = 0;
            if (CheckKey(VK_SHIFT))
            {
                ToggleOption(&dev_tools, NEWTRODIT_DEV_TOOLS, false);

                c = -2;
                continue;
            }
            if (dev_tools)
            {
                gotoxy(strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].ypos);
                print_line(join(join("\"", Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), "\""));
                getch_n();
                ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].display_y, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 2, 1);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
            }
        }
        if (ch == 23) // ^W
        {
            if (!CheckKey(VK_SHIFT))
            {
                CloseFile(&Tab_stack[file_index]);
                PrintBottomString(NEWTRODIT_FILE_CLOSED);
                getch_n();

                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                ch = 0;
                continue;
            }
        }

        if (ch == 17) // ^Q = Quit program (only with oldKeybinds = false)
        {
            if (!useOldKeybinds)
            {
                if (!CheckKey(VK_SHIFT))
                {
                    QuitProgram(SInf.color);
                    ShowBottomMenu();
                    SetColor(bg_color);
                    ch = 0;
                    continue;
                }
            }
        }
        if (ch == 24) // ^X = Quit program/cut
        {
            if (useOldKeybinds)
            {
                if (!CheckKey(VK_SHIFT))
                {
                    QuitProgram(SInf.color);
                    ShowBottomMenu();
                    SetColor(bg_color);
                    ch = 0;
                }
            }
            else
            {
                if (!CheckKey(VK_SHIFT))
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] != '\0')
                    {

                        Tab_stack[file_index].Ustack->size = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                        Tab_stack[file_index].Ustack->line = (char *)malloc(sizeof(char) * (Tab_stack[file_index].Ustack->size + 1));
                        memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);

                        Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                        Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                        Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                        Tab_stack[file_index].Ustack->create_nl = false;
                        Tab_stack[file_index].Ustack++->create_nl = false;

                        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1);
                        memcpy(GlobalLock(hMem), Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1); // Copy line to the clipboard
                        GlobalUnlock(hMem);
                        OpenClipboard(0);
                        EmptyClipboard();
                        SetClipboardData(CF_TEXT, hMem);
                        CloseClipboard();
                        memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 0, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]));
                        ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y, XSIZE - (lineCount ? Tab_stack[file_index].linecount_wide : 0), 1);
                        Tab_stack[file_index].xpos = 0;
                    }

                    ch = 0;
                    continue;
                }
            }

            continue;
        }
        if (ch == 127) // ^BS
        {
            if (Tab_stack[file_index].xpos > 0)
            {
                /*  Tab_stack[file_index].Ustack->size = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1;
                 Tab_stack[file_index].Ustack->line = (char *)malloc(Tab_stack[file_index].Ustack->size);
                 memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);
                 Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); // If it's not a duplicate, the value will change
                 Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                 Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                 Tab_stack[file_index].Ustack->create_nl = false;
                 Tab_stack[file_index].Ustack->delete_nl = false; */

                bs_tk = tokback_pos(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "()[]{}\t ", "?!");
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] == '\0')
                {

                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] == 9)
                    {
                        relative_xpos[Tab_stack[file_index].ypos] -= TAB_WIDE;
                    }
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][bs_tk] = 0;
                    n = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    for (int i = 0; i < bs_tk; i++)
                    {
                        Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n + i] = 0;
                    }
                    for (int i = Tab_stack[file_index].xpos; i >= bs_tk; i--)
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] == 9)
                        {
                            relative_xpos[Tab_stack[file_index].ypos] -= TAB_WIDE;
                        }
                        Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] = '\0';
                    }
                    n = XSIZE;
                    for (int i = 0; i < Tab_stack[file_index].xpos - bs_tk; ++i)
                    {

                        if ((Tab_stack[file_index].xpos - i) < n)
                        {
                            if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] == 9)
                            {
                                snprintf(tmp, DEFAULT_ALLOC_SIZE, "%s%s%s", memset(ptr, 0, TAB_WIDE), memset(ptr, ' ', TAB_WIDE), memset(ptr, 0, TAB_WIDE));
                                fputs(tmp, stdout);
                            }
                            else
                            {
                                fputs("\b \b", stdout);
                            }
                        }
                    }
                    Tab_stack[file_index].xpos = bs_tk;
                }
                else
                {

                    for (int i = 0; i < (Tab_stack[file_index].xpos - bs_tk <= 0) ? 0 : Tab_stack[file_index].xpos - bs_tk; i++)
                    {
                        temp_strsave = delete_char_left(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], bs_tk);
                        strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, BUFFER_X);
                        Tab_stack[file_index].xpos--;
                    }

                    ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + Tab_stack[file_index].xpos, Tab_stack[file_index].display_y, XSIZE, 1);
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
            }
            else
            {
                if (Tab_stack[file_index].ypos > 1)
                {
                    insert_deleted_row(&Tab_stack[file_index]);
                    if (!UpdateScrolledScreen(lineCount, &Tab_stack[file_index]))
                    {
                        ClearPartial(0, 1, XSIZE, YSIZE - 2);
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                    }
                    Tab_stack[file_index].linecount--;
                }
            }
            if (Tab_stack[file_index].xpos <= 0) // Don't allow xpos to be negative
            {
                Tab_stack[file_index].xpos = 0;
            }

            ch = 0;
        }

        if (ch == 25) // ^Y = Redo
        {

            /* strncpy_n(undo_stack, Tab_stack[file_index].strsave[undo_stack_line], BUFFER_X);
            strncpy_n(Tab_stack[file_index].strsave[undo_stack_line], redo_stack, BUFFER_X);
            LoadAllNewtrodit();
            FunctionAborted(&Tab_stack[file_index]);
            fflush(stdout); */

            ch = 0;
            continue;
        }

        if (ch == 9 && !CheckKey(VK_TAB)) // ^I = File info
        {
            CountBufferLines(&Tab_stack[file_index]);

            snprintf(tmp, DEFAULT_BUFFER_X, "File: \'%s\', size: %lld bytes (%u lines). Syntax highlighting: %s", strlasttok(Tab_stack[file_index].filename, PATHTOKENS), Tab_stack[file_index].size, Tab_stack[file_index].linecount, Tab_stack[file_index].Syntaxinfo.syntax_lang);
            PrintBottomString(tmp);
            c = -2;
            ch = 0;
        }
        if (ch == 26) // ^Z = Undo
        {
            if (undo_stack_tree > 0)
            {
                undo_stack_tree--;
                Tab_stack[file_index].xpos = Tab_stack[file_index].Ustack->line_pos;
                Tab_stack[file_index].ypos = Tab_stack[file_index].Ustack->line_count;
                if (Tab_stack[file_index].Ustack->create_nl == true)
                {
                    insert_deleted_row(&Tab_stack[file_index]);
                }
                if (Tab_stack[file_index].Ustack->delete_nl == true)
                {
                    insert_row(Tab_stack[file_index].strsave, Tab_stack[file_index].xpos, Tab_stack[file_index].ypos, Tab_stack[file_index].Ustack->line);
                    Tab_stack[file_index].strsave[Tab_stack[file_index].Ustack->line_count] = strdup(Tab_stack[file_index].Ustack->line);
                }
                if (!UpdateScrolledScreen(lineCount, &Tab_stack[file_index]))
                {
                    ClearPartial(0, Tab_stack[file_index].display_y, XSIZE, YSIZE - Tab_stack[file_index].display_y - 1);
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                }
                else
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] = Tab_stack[file_index].Ustack->line;
                }
                /* if (strlen(Tab_stack[file_index].Ustack->line) < strlen(Tab_stack[file_index].strsave[ypos]))
                {
                    xpos = nolflen(Tab_stack[file_index].Ustack->line);
                } */

                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].ypos);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
            }

            ch = 0;
            continue;
        }

        if (ch == 8 && !CheckKey(0x08) && CheckKey(VK_CONTROL)) // ^H = Replace string / S-^H = Same as F1 (opens help)
        {
            ch = 0;

            if (CheckKey(VK_SHIFT))
            {
                NewtroditHelp();

                LoadAllNewtrodit();

                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
            }
            else
            {
                replace_count = 0;
                ClearPartial(0, YSIZE - 2, XSIZE, 2);

                printf("%.*s\n", wrapSize, NEWTRODIT_PROMPT_FIND_STRING);
                printf("%.*s", wrapSize, NEWTRODIT_PROMPT_REPLACE_STRING);

                n = Tab_stack[file_index].xpos;
                gotoxy(strlen(NEWTRODIT_PROMPT_FIND_STRING), YSIZE - 2);

                fgets(find_string, sizeof find_string, stdin);
                gotoxy(strlen(NEWTRODIT_PROMPT_REPLACE_STRING), BOTTOM);
                if (nolflen(find_string) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }
                fgets(replace_string, sizeof find_string, stdin);
                find_string[strcspn(find_string, "\n")] = 0;
                replace_string[strcspn(replace_string, "\n")] = 0;
                if (!ValidString(find_string) || !ValidString(replace_string))
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    continue;
                }

                for (int i = 1; i < BUFFER_Y; i++) // Line 0 is unused
                {

                    replace_str_ptr = ReplaceString(Tab_stack[file_index].strsave[i], find_string, replace_string, &replace_count);
                    if (replace_str_ptr)
                    {
                        if (strlen(replace_str_ptr) < strlen(Tab_stack[file_index].strsave[i]))
                        {
                            Tab_stack[file_index].xpos = strlen(replace_str_ptr);
                        }

                        strncpy_n(Tab_stack[file_index].strsave[i], replace_str_ptr, BUFFER_X);
                    }
                }

                if (strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) < n)
                {
                    Tab_stack[file_index].xpos = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                PrintBottomString(join(join(join("Replaced ", itoa_n(replace_count)), " occurrences of "), find_string));
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(Tab_stack[file_index].xpos, Tab_stack[file_index].ypos);
                if (replace_count > 0)
                {
                    Tab_stack[file_index].is_modified = true;
                }
            }
            continue;
        }

        if ((ch == 8 && _NEWTRODIT_OLD_SUPPORT == 1) || (ch == 8 && CheckKey(0x08) && !CheckKey(VK_CONTROL))) // BS key (Avoiding Control-H)
        {

            c = -8; // Negative to avoid conflict
            if (Tab_stack[file_index].xpos > 0)
            {
                Tab_stack[file_index].is_modified = true;

                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos - 1] == 9) // TAB key
                {
                    relative_xpos[Tab_stack[file_index].ypos] -= TAB_WIDE;
                    gotoxy(Tab_stack[file_index].xpos + relative_xpos[Tab_stack[file_index].ypos] + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y + relative_ypos[Tab_stack[file_index].xpos]);
                }

                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] != '\0')
                {
                    /*
                        TODO: CRLF COMPATIBILITY

                    */

                    strncpy_n(temp_strsave, Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) - strlen(Tab_stack[file_index].newline));
                    hasNewLine = false;
                    // Remove ending newline character from Tab_stack[file_index].strsave[ypos]

                    if (!strcmp(temp_strsave, Tab_stack[file_index].newline))
                    {
                        Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) - strlen(Tab_stack[file_index].newline)] = '\0';
                        hasNewLine = true;
                    }

                    tmp = delete_char_left(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].xpos - 1);
                    if (hasNewLine)
                    {
                        strncat(tmp, Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // strcat for CRLF newline
                    }

                    strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                    ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + (nolflen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos])), Tab_stack[file_index].display_y, 2, 1); // Clear the character
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(tmp);
                }
                else
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos - 1] = '\0';
                    fputs("\b \b", stdout);
                }
                if (syntaxHighlighting)
                {
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
                Tab_stack[file_index].xpos -= 2;
            }
            else
            {
                /* Act as END key */
                if (Tab_stack[file_index].ypos > 1)
                {
                    Tab_stack[file_index].is_modified = true;
                    // TODO: Fix bugs
                    insert_deleted_row(&Tab_stack[file_index]);
                    // Tab_stack[file_index].linecount--;

                    if (!UpdateScrolledScreen(lineCount, &Tab_stack[file_index]))
                    {
                        /* ClearPartial(
                            (lineCount ? Tab_stack[file_index].linecount_wide : 0),
                            Tab_stack[file_index].display_y,
                            wrapSize),
                            (YSIZE - (Tab_stack[file_index].display_y + 2)) > 0 ? YSIZE - (Tab_stack[file_index].display_y + 2) : 1
                            ); */
                        ClearPartial(
                            (lineCount ? Tab_stack[file_index].linecount_wide : 0),
                            1,
                            wrapSize,
                            YSCROLL);
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                        DEBUG
                    }
                }

                Tab_stack[file_index].xpos--; // Because 1 will be added to xpos in the next iteration
            }
        }
        else

        {
            if (ch > 31 || (ch == 9 && CheckKey(0x09))) // Printable character
            {
                if (!Tab_stack[file_index].is_modified)
                {
                    Tab_stack[file_index].is_modified = true;
                }

                if (!insertChar && ch != 9) // Insert key not pressed
                {

                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] != '\0')
                    {
                        if (ch == 9)
                        {
                            relative_xpos[Tab_stack[file_index].ypos] += TAB_WIDE;
                        }
                        tmp = insert_char(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], ch, Tab_stack[file_index].xpos);
                        strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                        gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y); // Maybe this creates a bug, I don't know
                        print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);

                        ch = 0;
                    }
                }

                if (CheckKey(VK_TAB) && ch == 9) // TAB key
                {
                    if (convertTabtoSpaces)
                    {
                        for (int i = 0; i < TAB_WIDE; i++) // i is also character 9 :)
                        {
                            temp_strsave = insert_char(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], ' ', Tab_stack[file_index].xpos);
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, BUFFER_X);
                        }
                        Tab_stack[file_index].xpos += (TAB_WIDE - 1);
                        gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                        print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    }
                    else
                    {
                        temp_strsave = insert_char(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 9, Tab_stack[file_index].xpos);
                        strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, BUFFER_X);
                        PrintTab(TAB_WIDE);
                        relative_xpos[Tab_stack[file_index].xpos] += TAB_WIDE;
                        gotoxy(Tab_stack[file_index].xpos + relative_xpos[Tab_stack[file_index].ypos] + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].ypos + relative_ypos[Tab_stack[file_index].xpos]);
                    }
                }
                else

                {
                    if (ch != 0)
                    {
                        Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] = (int)ch; // Add character to buffer

                        if (Tab_stack[file_index].xpos < XSIZE - (lineCount ? Tab_stack[file_index].linecount_wide : 0))
                        {
                            putchar(ch);
                            if (syntaxHighlighting)
                            {
                                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                                // I'm using an intermediate variable to make the line shorter
                                n = strrpbrk(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].Syntaxinfo.separators);
                                color_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 0, Tab_stack[file_index].Syntaxinfo.override_color);
                            }
                        }
                    }
                }
            }
            else
            {
                if (ch != 0 && ch <= 26 && ch != 13)
                {
                    memset(inbound_ctrl_key, 0, sizeof(inbound_ctrl_key)); // Clear the string for the next key
                    if (CheckKey(VK_MENU))
                    {
                        strcat(inbound_ctrl_key, "A-");
                    }
                    if (CheckKey(VK_SHIFT))
                    {
                        strcat(inbound_ctrl_key, "S-");
                    }
                    if (CheckKey(VK_ESCAPE))
                    {
                        strcat(inbound_ctrl_key, "^^[");
                    }
                    if (CheckKey(VK_CONTROL))
                    {
                        strcat(inbound_ctrl_key, "^^");
                    }
                    inbound_ctrl_key[strlen(inbound_ctrl_key) - 1] = ch + 64; // Convert getch return value to ASCII
                    PrintBottomString(join(NEWTRODIT_ERROR_INVALID_INBOUND, inbound_ctrl_key));
                    c = -2; // For later use
                }
                Tab_stack[file_index].xpos--;
            }
        }

        Tab_stack[file_index].xpos++;
        if ((Tab_stack[file_index].strsave[1][0] == '\0' && Tab_stack[file_index].strsave[1][1] == '\0') || Tab_stack[file_index].strsave[1][0] == EOF) // If the document is empty set modified to false
        {
            Tab_stack[file_index].is_modified = false;
            UpdateTitle(&Tab_stack[file_index]);
        }

        if (strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) >= BUFFER_X - (TAB_WIDE * 2) || Tab_stack[file_index].ypos > BUFFER_Y || Tab_stack[file_index].xpos >= BUFFER_X - (TAB_WIDE * 2)) // Avoid buffer overflows by resizing the buffer
        {
            tmp = realloc_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], BUFFER_X, BUFFER_X + BUFFER_INCREMENT);
            BUFFER_X += BUFFER_INCREMENT;

            if (!tmp)
            {
                PrintBottomString(NEWTRODIT_ERROR_OUT_OF_MEMORY);
                getch_n();
                SaveFile(&Tab_stack[file_index]);
                ExitRoutine(ENOMEM);
            }
            else
            {
                free(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] = tmp;
            }
        }
    }

    SetColor(SInf.color);
    return 0;
}