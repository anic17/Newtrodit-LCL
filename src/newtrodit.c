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
            ~ win32/newtrodit_core_win.h
            ~ linux/newtrodit_core_linux.h
        newtrodit_func.c   : Main functions
            ~ win32/newtrodit_func_win.c
            ~ linux/newtrodit_func_linux.c
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

int LoadSettings(char *newtrodit_config_file, char *macro, int *sigsegv, int *linecount, int *devmode, File_info *tstack)
{
    /*
        Settings are stored in an INI-like format.
        The format is:
            key=value
            ;comment

    */

    chdir(SInf.location);
    WriteLogFile(join("Loading settings file: ", newtrodit_config_file));

    FILE *settings = fopen(newtrodit_config_file, "rb");
    if (!settings)
    {
        return EXIT_FAILURE;
    }

    char setting_buf[1024]; // Max 1 kB per line
    char *iniptr = malloc(sizeof(setting_buf) + 1), *token = malloc(sizeof(setting_buf) + 1);
    char *settingname;
    int cnt = 0;
    bool isCorrectSetting = true;
    char equalchar[] = "=";
    char *setting_list[] = {
        "fontcolor",
        "autoindent",
        "codepage",
        "convertnull",
        "converttab",
        "cursize",
        "curinsert",
        "devmode",
        "linecount",
        "linecountwide",
        "macro",
        "manfile",
        "menucolor",
        "mouse",
        "newline",
        "oldkeybindings",
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

        /* cnt = strspn(setting_buf, " \t");
        // memmove(&setting_buf[0], &setting_buf[cnt], (strlen(setting_buf) - cnt));
        //  memset(&setting_buf[strlen(setting_buf) - cnt], 0, (strlen(setting_buf) - cnt));
        snprintf(setting_buf, sizeof(setting_buf), "%s", setting_buf - 1); */

        if (setting_buf[cnt] == ';' || setting_buf[cnt] == 0) // Comment or newline found
        {
            continue;
        }
        iniptr = strtok(setting_buf, "=");
        settingname = strdup(iniptr);
        strlwr(settingname);

        while (iniptr != NULL) // Loop through the settings
        {
            isCorrectSetting = true;
            // iniptr[strcspn(iniptr, "\n")] = '\0';
            token = strtok(NULL, equalchar);
            if (!strcmp(settingname, "fontcolor"))
            {
                bg_color = HexStrToDec(token) % 256;
                default_color = bg_color;
            }
            else if (!strcmp(settingname, "autoindent"))
            {
                atoi(token) ? (autoIndent = true) : (autoIndent = false);
            }
            else if (!strcmp(settingname, "codepage"))
            {
                int cp = atoi(token);
#ifdef _WIN32
                SetConsoleOutputCP(cp);
#else
                // Linux TODO
#endif
            }
            else if (!strcmp(settingname, "convertnull"))
            {
                atoi(token) ? (convertNull = true) : (convertNull = false);
            }
            else if (!strcmp(settingname, "converttab"))
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
            else

                if (!strcmp(settingname, "curinsert"))
            {

                atoi(token) ? (cursorSizeInsert = true) : (cursorSizeInsert = false);
            }
            else

                if (!strcmp(settingname, "cursize"))
            {
                CURSIZE = atoi(token);
                SetCursorSettings(true, CURSIZE);
            }
            else

                if (!strcmp(settingname, "devmode"))
            {
                atoi(token) ? (*devmode = true) : (*devmode = false);
            }
            else if (!strcmp(settingname, "findinsensitive"))
            {
                atoi(token) ? (findInsensitive = true) : (findInsensitive = false);
            }
            else if (!strcmp(settingname, "linecount"))
            {
                atoi(token) ? (*linecount = true) : (*linecount = false);
            }
            else if (!strcmp(settingname, "linecountwide"))
            {
                LINECOUNT_WIDE = abs(atoi(token)) % sizeof(long long);
            }
            else if (!strcmp(settingname, "linehighlight"))
            {
                atoi(token) ? (linecountHighlightLine = true) : (linecountHighlightLine = false);
            }
            else if (!strcmp(settingname, "macro"))
            {

                if (ValidString(token))
                {
                    strncpy_n(run_macro, token, sizeof(setting_list));
                }
            }
            else if (!strcmp(settingname, "manfile"))
            {
                RemoveQuotes(token, strdup(token)); // Remove quotes

                if (ValidFileName(token))
                {
                    strncpy_n(manual_file, token, sizeof(manual_file));
                    manual_file[strcspn(manual_file, "\n")] = 0;
                }
            }
            else if (!strcmp(settingname, "menucolor"))
            {
                fg_color = (HexStrToDec(token) * 16) % 256;
            }
            else if (!strcmp(settingname, "mouse"))
            {
                atoi(token) ? (partialMouseSupport = true) : (partialMouseSupport = false);
            }
            else

                if (!strcmp(settingname, "newline"))
            {

                if (!strncmp(token, "0x", 2))
                {
                    ParseHexString(token);
                }
                else
                {
                    strncpy_n(Tab_stack[file_index].newline, token, strlen(Tab_stack[file_index].newline));
                }
            }
            else

                if (!strcmp(settingname, "oldkeybindings"))
            {
                atoi(token) ? (useOldKeybindings = true) : (useOldKeybindings = false);
            }
            else if (!strcmp(settingname, "sigsegv"))
            {
                atoi(token) ? (*sigsegv = true) : (*sigsegv = false);
            }
            else if (!strcmp(settingname, "syntax"))
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
            else if (!strcmp(settingname, "tabwide"))
            {
                TAB_WIDE = atoi(token);
            }
            else if (!strcmp(settingname, "trimlonglines"))
            {
                atoi(token) ? (trimLongLines = true) : (trimLongLines = false);
            }
            else if (!strcmp(settingname, "wholeword"))
            {
                atoi(token) ? (matchWholeWord = true) : (matchWholeWord = false);
            }
            if (!strcmp(settingname, "xsize"))
            {
                int xs = atoi(token);
                SetConsoleSize(xs, YSIZE);
            }
            else if (!strcmp(settingname, "ysize"))
            {
                int ys = atoi(token);
                SetConsoleSize(XSIZE, ys);
            }
            else
            {
                isCorrectSetting = false;
            }
            if (isCorrectSetting)
            {
                WriteLogFile(join("Successfully loaded the setting: ", settingname));
            }
            else
            {
                WriteLogFile(join("Unexistent setting: ", settingname));
            }
            iniptr = strtok(NULL, "=");
        }
    }
    WriteLogFile("Finished loading settings file");

    fclose(settings);
    chdir(SInf.dir);

    return 0;
}

int main(int argc, char *argv[])
{
    // Newtrodit initialization begins

    char *startup_info = malloc(sizeof(char) * MAX_PATH * 2); // *2 for safety
    memset(startup_info, 0, sizeof(char) * MAX_PATH * 2);
#ifdef _WIN32
    GetModuleFileNameA(NULL, startup_info, MAX_PATH);
#else
    startup_info = argv[0]; // Program name
#endif
    // Generate log file name
    SInf.log_file_name = calloc(MAX_PATH, sizeof(char));
    SInf.log_file_name = strdup(GetLogFileName());
    if (WriteLogFile("\nNewtrodit started"))
    {
        fprintf(stderr, "WARNING: Could not open logfile '%s'", SInf.log_file_name);
        getch_n();
    }

    WriteLogFile(join("Logfile created with name: ", SInf.log_file_name));

// LINUX: Buffers do not work as expected
#if _NEWTRODIT_EXPERIMENTAL_RESTORE_BUFFER == 1 && !_NEWTRODIT_OLD_SUPPORT
#ifndef _WIN32
    EnterAltConsoleBuffer();
#endif
#endif

#ifdef _WIN32
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE), hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdout, &dwStdoutMode);
    GetConsoleMode(hStdin, &dwStdinMode);
    // Disable wrapping to avoid (or at least reduce) graphical bugs
    WriteLogFile(join("Changing console output mode: ", (SetConsoleMode(hStdout, dwStdoutMode & ~ENABLE_WRAP_AT_EOL_OUTPUT)) ? "Succeeded" : "Failed"));
    WriteLogFile(join("Changing console input mode: ", (SetConsoleMode(hStdin, ENABLE_MOUSE_INPUT | 0x80 | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT)) ? "Succeeded" : "Failed")); // 0x40 is quick edit mode and 0x80 are extended flags

    WriteLogFile("Loading startup data");
#endif
    char *sinf_ptr = calloc(sizeof(char), MAX_PATH * 2);
    if (get_path_directory(startup_info, sinf_ptr) != NULL)
    {
        SInf.dir = strdup(getcwd(NULL, 0));
        SInf.location = strdup(sinf_ptr);
        // chdir(SInf.location);
    }
    else
    {
        memset(SInf.location, 0, sizeof(char) * MAX_PATH * 2);
    }
    free(sinf_ptr);

    // Get file date times
#ifdef _WIN32
    FILETIME tmpTimeRead, tmpTimeWrite;
    tmpTimeWrite.dwLowDateTime = 0;
    tmpTimeWrite.dwHighDateTime = 0;
    tmpTimeRead.dwLowDateTime = 0;
    tmpTimeRead.dwHighDateTime = 0;
#else
    time_t tmpTimeRead, tmpTimeWrite;
    tmpTimeRead = 0;
    tmpTimeWrite = 0;
#endif

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
    LINECOUNT_WIDE = LINECOUNT_WIDE_; // Set line count wide

    int hasNewLine = false; // Bool for newline in insert char
    int dev_tools = false;  // Bool to enable or disable the dev tools
    int insertChar = false; // Bool to check if replace instead of insert
    int sigsegvScreen = true;
    int listDir = true;
    int isFullScreen = false;
    char wholeWordDelims[] = "`~!@#$%^&*()[]{}-=+\\|/;:\'\",.<>? \n\t\r";

    /*     char *run_macro = malloc(sizeof(char) * MAX_PATH + 1);
     */

    for (int i = 0; i < MAX_TABS; i++)
    {
        Tab_stack[i].filename = calloc(MAX_PATH, sizeof(char));
    }

    // Allocate buffer

    if (!AllocateBufferMemory(&Tab_stack[file_index]))
    {

        printf("%.*s\n", wrapSize, NEWTRODIT_ERROR_OUT_OF_MEMORY);
        ExitRoutine(ENOMEM);
    }
    old_open_files = malloc(MAX_PATH * sizeof(char *));

    for (int i = 1; i < MIN_BUFSIZE; i++)
    {
        old_open_files[i] = calloc(MAX_PATH, sizeof(char));
    }

    run_macro = malloc(sizeof(char) * MACRO_ALLOC_SIZE + 1);

    WriteLogFile("Setting console signal handlers");
#ifdef _WIN32

    signal(SIGBREAK, SIG_IGN); // Ctrl-Break handler
#else
    signal(SIGTSTP, SIG_IGN); // Ctrl-Z handler
#endif
    signal(SIGINT, SIG_IGN);          // Ctrl-C handler
    signal(SIGSEGV, sigsegv_handler); // Segmentation fault handler
    signal(SIGABRT, sigabrt_handler); // Abort handler

    WriteLogFile("Finished setting console signal handlers");

    memset(run_macro, 0, sizeof(char) * MACRO_ALLOC_SIZE + 1);

    LoadSettings(settings_file, run_macro, &sigsegvScreen, &lineCount, &dev_tools, &Tab_stack[file_index]); // Load settings from settings file

    if (!sigsegvScreen)
    {
        signal(SIGSEGV, SIG_DFL);
    }

    char *err_msg = malloc(sizeof(char) * MIN_BUFSIZE);
    if (BUFFER_X < MIN_BUFSIZE || BUFFER_Y < MIN_BUFSIZE)
    {
        snprintf(err_msg, MIN_BUFSIZE, "Buffer is too small (Current size is %dx%d and minimim size is %dx%d)", BUFFER_X, BUFFER_Y, MIN_BUFSIZE, MIN_BUFSIZE);
#ifdef _WIN32
        MessageBox(NULL, err_msg, "Newtrodit", MB_ICONERROR);
#endif
        WriteLogFile(err_msg);
        fprintf(stderr, "%s", err_msg);
        free(err_msg);
        ExitRoutine(1);
    }
    char *temp_strsave = calloc(sizeof(char), BUFFER_X + 1);
    char *tmp = calloc(sizeof(char), BUFFER_X + 1);

    int undo_stack_tree = 0;

    // Declare variables
    int old_x_size = 0, old_y_size = 0;
    int bs_tk = 0;

    SInf.color = GetConsoleInfo(COLOR);

    char *save_dest = calloc(sizeof(char), MAX_PATH + 1);
    char *line_number_str;

    char find_string[512] = {0}, replace_string[512] = {0};
    int find_string_index = 0;
    char fileopenread[MAX_PATH];
    char *replace_str_ptr;
    int find_count = 0, replace_count = 0;

    char inbound_ctrl_key[100] = {0};
    char newname[MAX_PATH], syntaxfile[MAX_PATH], locate_file[MAX_PATH], macro_input[MACRO_ALLOC_SIZE], fcomp1[MAX_PATH], fcomp2[MAX_PATH];
    convertTabtoSpaces = true;
    int n = 0, n2 = 0;
    signed long long ll_n = 0;
    char *ptr = calloc(sizeof(char), BUFFER_X), *buffer_clipboard;

    // File variables
    FILE *fileread = {0}, *fp_savefile = {0}, *open_argv = {0}, *syntax = {0};

    // Position variables
    int *relative_xpos = calloc(sizeof(int) * BUFFER_Y, BUFFER_X);
    int *relative_ypos = calloc(sizeof(int) * BUFFER_X, BUFFER_Y);

    // getch() variables
    int ch = 0;

    int *file_arguments = {0}; // Array of ints for arguments that aren't switches
    int file_arguments_count = 0;

    file_arguments = calloc(sizeof(int *), argc); // Allocate memory for file_arguments for each argument
    WriteLogFile("Finished loading startup data");

    int shiftargc = 1; // Can't be 0, because the first argument is the program name
    WriteLogFile("Parsing command-line arguments");

    for (int arg_parse = 1; arg_parse < argc; ++arg_parse)
    {
        if (!strcmp(argv[arg_parse], "--version") || !strcmp(argv[arg_parse], "-v")) // Version parameter
        {
            printf("%.*s", wrapSize, ProgInfo());
            ExitRoutine(0);
        }
        else if (!strcmp(argv[arg_parse], "--help") || !strcmp(argv[arg_parse], "-h")) // Manual parameter
        {
            NewtroditHelp();
            SetColor(SInf.color);
            ClearScreen();
            ExitRoutine(0);
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
                    Tab_stack[file_index].Syntaxinfo.syntax_file = FullPath(syntaxfile);
                    Tab_stack[file_index].Syntaxinfo.keyword_count = syntaxKeywordsSize;
                }

                arg_parse++;
            }
            else
            {
                fprintf(stderr, "%s\n", NEWTRODIT_ERROR_MISSING_ARGUMENT);
                ExitRoutine(1);
            }
        }
        else if (!strcmp(argv[arg_parse], "--line") || !strcmp(argv[arg_parse], "-l")) // Display line count
        {
            lineCount = true;
        }
        else if (!strcmp(argv[arg_parse], "--lfunix") || !strcmp(argv[arg_parse], "-n")) // Use UNIX new line
        {
            strncpy_n(Tab_stack[file_index].newline, "\n", 1); // Avoid any type of buffer overflows
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
        else if (!strcmp(argv[arg_parse], "--mouse") || !strcmp(argv[arg_parse], "-m")) // Use mouse
        {
            partialMouseSupport = true;
        }
        else if (!strcmp(argv[arg_parse], "--menucolor") || !strcmp(argv[arg_parse], "-c")) // Foreground color parameter
        {
            if (argv[arg_parse + 1] != NULL)
            {
                fg_color = HexStrToDec(argv[arg_parse + 1]);
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
                bg_color = HexStrToDec(argv[arg_parse + 1]);
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

                    fprintf(stderr, "%s\n", Substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename)); // Hard-coded hack
                    WriteLogFile(join(NEWTRODIT_FS_FILE_OPEN_ERR, Substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename)));
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
                fprintf(stderr, "%s%s\n", NEWTRODIT_FS_FILE_TOO_LARGE, Tab_stack[file_index].filename);
                WriteLogFile(join(NEWTRODIT_FS_FILE_TOO_LARGE, Tab_stack[file_index].filename));

                return EFBIG; // File too big
            }
            if (LoadFile(&Tab_stack[file_index], Tab_stack[file_index].filename, open_argv) <= -1)
            {
                fprintf(stderr, "%s\n", (char *)Substring(0, strlen(NEWTRODIT_FS_FILE_OPEN_ERR) - 1, Tab_stack[file_index].filename));
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

        CenterText(StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), 0);
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

        if ((lineCount && !isprint(ch) && c != -32) || linecountHighlightLine) // -32 = scroll
        {
            DisplayLineCount(&Tab_stack[file_index], YSIZE - 3, Tab_stack[file_index].display_y);
        }
        if (c == -32)
        {
            c = 0;
        }

        if (c != -2) // Clear bottom line
        {
            DisplayCursorPos(&Tab_stack[file_index]);
        }
        // wrapSize = XSIZE - 2 - ((lineCount ? Tab_stack[file_index].linecount_wide : 0));

        SetWrapSize();

        SetDisplayCursorPos(&Tab_stack[file_index]);
#ifdef _WIN32
        GetFileTime(&Tab_stack[file_index].hFile, &tmpTimeRead, NULL, &tmpTimeWrite);
#endif
        ch = GetNewtroditInput(&Tab_stack[file_index]); // Register all input events, not only key presses

        if (Tab_stack[file_index].xpos < 0 || Tab_stack[file_index].ypos < 1)
        {
            PrintBottomString(NEWTRODIT_ERROR_INVALID_POS_RESET);
            WriteLogFile(NEWTRODIT_ERROR_INVALID_POS_RESET);
            getch_n();

            Tab_stack[file_index].xpos = 0;
            Tab_stack[file_index].ypos = 1;
        }
#ifdef _WIN32
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
#endif

        if (c == -2) // Inbound invalid control key
        {
            ShowBottomMenu();
            DisplayCursorPos(&Tab_stack[file_index]);

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

        if (ch == CTRLC || ch == CTRLK) // ^C = Copy line to clipboard; ^K = Cut line
        {
            if (!CheckKey(VK_SHIFT))
            {
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] != '\0')
                {

#ifdef _WIN32
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1);
                    memcpy(GlobalLock(hMem), Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1); // Copy line to the clipboard
                    GlobalUnlock(hMem);
                    OpenClipboard(0);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hMem);
                    CloseClipboard();
                    if (ch == CTRLK && useOldKeybindings)
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
#endif
                }
                ch = 0;
                continue;
            }
            else
            {
                if (ch == CTRLK)
                {
                    ToggleOption(&useOldKeybindings, NEWTRODIT_OLD_KEYBINDINGS, true);

                    c = -2;
                    ch = 0;
                    continue;
                }
                if (ch == CTRLC) // S-^C = File compare
                {
                    memset(fcomp1, 0, sizeof(fcomp1));
                    memset(fcomp2, 0, sizeof(fcomp2));
                    ClearPartial(0, YSIZE - 2, XSIZE, 2);
                    printf("%.*s\n", wrapSize, NEWTRODIT_PROMPT_FIRST_FILE_COMPARE);
                    printf("%.*s", wrapSize, NEWTRODIT_PROMPT_SECOND_FILE_COMPARE);
                    gotoxy(strlen(NEWTRODIT_PROMPT_FIRST_FILE_COMPARE), YSIZE - 2);
                    fgets(fcomp1, sizeof(fcomp1), stdin);
                    if (NoLfLen(fcomp1) <= 0)
                    {
                        FunctionAborted(&Tab_stack[file_index]);
                        continue;
                    }
                    gotoxy(strlen(NEWTRODIT_PROMPT_SECOND_FILE_COMPARE), BOTTOM);

                    fgets(fcomp2, sizeof(fcomp2), stdin);
                    if (NoLfLen(fcomp2) <= 0)
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
                        DisplayCursorPos(&Tab_stack[file_index]);
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
                    DisplayCursorPos(&Tab_stack[file_index]);
                    ch = 0;
                    continue;
                }
            }
        }

        if (ch == CTRLL) // ^L = Toggle line count; S-^L = Locate files
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
                PrintBottomString("%s", NEWTRODIT_PROMPT_LOCATE_FILE);
                fgets(locate_file, sizeof locate_file, stdin);
                if (NoLfLen(locate_file) <= 0)
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
                DisplayCursorPos(&Tab_stack[file_index]);
                ClearPartial(0, 1, XSIZE, YSIZE - 2);
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
            }
            continue;
        }

        if (ch == CTRLG) // ^G = Go to line
        {
            if (CheckKey(VK_SHIFT))
            {
                c = -7;
            }
            ch = 0;
            PrintBottomString("%s", NEWTRODIT_PROMPT_GOTO_LINE);
            line_number_str = TypingFunction('0', '9', strlen(itoa_n(Tab_stack[file_index].linecount)));
            if (atoi(line_number_str) < 1) // Line is less than 1
            {
                PrintBottomString("%s", NEWTRODIT_ERROR_INVALID_YPOS);
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
                DisplayCursorPos(&Tab_stack[file_index]);

                continue;
            }

            if (Tab_stack[file_index].ypos != 1 && Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][0] == '\0' && strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1] + NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1]), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
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
                    Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    relative_xpos[Tab_stack[file_index].ypos] = TokCount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "\t");
                }
            }
            else
            {
                Tab_stack[file_index].xpos = 0;
            }

            (Tab_stack[file_index].ypos >= (YSIZE - 3)) ? UpdateScrolledScreen(&Tab_stack[file_index]) : UpdateHomeScrolledScreen(&Tab_stack[file_index]);

            ShowBottomMenu();
            DisplayCursorPos(&Tab_stack[file_index]);

            continue;
        }
        if (ch == CTRLN) // ^N = New file
        {
            if (!CheckKey(VK_SHIFT))
            {

                if ((n = NewFile(&Tab_stack[file_index])) > 0)
                {
                    PrintBottomString("%s", NEWTRODIT_NEW_FILE_CREATED);
                }
                else
                {
                    ErrorMessage(n, Tab_stack[file_index].filename);
                }
            }
            else
            {

                ToggleOption(&convertNull, NEWTRODIT_NULL_CONVERSION, false);
            }
            c = -2;

            ch = 0;
            continue;
        }
        if (ch == CTRLO) // ^O = Open file
        {
            if (!CheckKey(VK_SHIFT)) // Shift not pressed
            {
                if (Tab_stack[file_index].is_modified)
                {
                    PrintBottomString("%s", NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);

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
                PrintBottomString("%s", NEWTRODIT_PROMPT_FOPEN);

                fgets(fileopenread, sizeof fileopenread, stdin);
                if (NoLfLen(fileopenread) <= 0)
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
                    if (!strcmp(StrLastTok(Tab_stack[i].filename, PATHTOKENS), fileopenread) && i != file_index) // File is already open
                    {
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                        PrintBottomString(NEWTRODIT_PROMPT_ALREADY_OPEN_TAB);
                        if (YesNoPrompt())
                        {
                            file_index = i;
                            LoadAllNewtrodit();
                            DisplayFileContent(&Tab_stack[file_index], stdout, 0);

                            PrintBottomString("%s %s (%d/%d).", NEWTRODIT_SWITCHING_FILE, fileopenread, file_index + 1, open_files);
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

                        PrintBottomString("%s", ErrorMessage(abs(n), fileopenread)); // Errors are always negative, so abs() is used
                        getch_n();
                        ShowBottomMenu();

                        DisplayCursorPos(&Tab_stack[file_index]);

                        continue;
                    }

                    old_open_files[oldFilesIndex] = Tab_stack[file_index].filename;
                    NewtroditNameLoad();
                    CenterText(StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), 0);
                    DisplayTabIndex(&Tab_stack[file_index]);
                    RightAlignNewline();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                }

                fclose(fileread);
                ch = 0;
                continue;
            }
        }

        if (ch == CTRLT) // S-^T = Toggle tab conversion
        {
            /* if (CheckKey(VK_SHIFT) && dev_tools)
            {
                ToggleOption(&convertTabtoSpaces, NEWTRODIT_TAB_CONVERSION, false);
                c = -2;

                ch = 0;
                continue;
            } */
            if (CheckKey(VK_SHIFT)) // S-^T
            {
                if (old_open_files[0][0] != '\0' && oldFilesIndex > 0)
                {
                    if (Tab_stack[file_index].is_modified)
                    {
                        PrintBottomString("%s", NEWTRODIT_PROMPT_SAVE_MODIFIED_FILE);

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
                    PrintBottomString("%s", NEWTRODIT_PROMPT_REOPEN_FILE);
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

        if (ch == CTRLR) // ^R = Reload file and S-^R = Reload settings
        {
            if (CheckKey(VK_SHIFT)) // S-^R = Reload settings
            {
                PrintBottomString("%s", NEWTRODIT_PROMPT_RELOAD_SETTINGS);
                if (YesNoPrompt())
                {
                    if (LoadSettings(settings_file, run_macro, &sigsegvScreen, &lineCount, &dev_tools, &Tab_stack[file_index]) == 0) // Reload settings
                    {
                        LoadAllNewtrodit();
                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                        PrintBottomString("%s", NEWTRODIT_SETTINGS_RELOADED);
                    }
                    else
                    {
                        PrintBottomString("%s", NEWTRODIT_ERROR_RELOAD_SETTINGS);
                    }
                    getch_n();
                }
                DisplayCursorPos(&Tab_stack[file_index]);
                ShowBottomMenu();
            }
            else
            {

                // ^R = Reload file
                PrintBottomString("%s", NEWTRODIT_PROMPT_RELOAD_FILE);

                if (YesNoPrompt())
                {
                    open_argv = fopen(Tab_stack[file_index].filename, "rb");

                    ReloadFile(&Tab_stack[file_index], open_argv);
#ifdef _WIN32
                    if (GetFileAttributes(Tab_stack[file_index].filename) & FILE_ATTRIBUTE_READONLY)
                    {
                        PrintBottomString("%s", NEWTRODIT_WARNING_READONLY_FILE);
                        c = -2;
                    }
#endif
                }
                else
                {
                    ShowBottomMenu();
                }
            }
            ch = 0;

            continue;
        }

        if (ch & BIT_ESC224) // Special keys: 224 (0xE0)
        {

            c = -32;

#ifdef _WIN32
            switch (ch & (~BIT_ESC224))
#else
            switch (ch)
#endif
            {

            case UP:
                // Up arrow
                if (Tab_stack[file_index].ypos > 1)
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1][Tab_stack[file_index].xpos] == '\0')
                    {
                        Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[--Tab_stack[file_index].ypos]);
                    }
                    else
                    {
                        Tab_stack[file_index].ypos--;
                    }
                    /* if(Tab_stack[file_index].display_y >= YSIZE - 2 && Tab_stack[file_index].display_y > 1)
                    {
                        Tab_stack[file_index].display_y--; // Scroll up
                    } */
                }
                UpdateScrolledScreen(&Tab_stack[file_index]);
                break;

            case LEFT:
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
                        UpdateScrolledScreen(&Tab_stack[file_index]);
                    }
#endif
                    --Tab_stack[file_index].xpos;
                }
                else
                {
                    if (Tab_stack[file_index].ypos > 1)
                    {

                        Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[--Tab_stack[file_index].ypos]);
#ifdef HORIZONTAL_SCROLL

                        horizontalScroll = Tab_stack[file_index].xpos - wrapSize;
                        if (horizontalScroll < 0)
                        {
                            horizontalScroll = 0;
                        }
#endif

                        UpdateScrolledScreen(&Tab_stack[file_index]);
                    }
                }

                break;
            case RIGHT:
                // Right arrow
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] != '\0')
                {
                    if (Tab_stack[file_index].xpos == NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
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
                                    UpdateScrolledScreen(&Tab_stack[file_index]);
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

            case DOWN:
                // Down arrow
                n2 = Tab_stack[file_index].ypos;
                if (Tab_stack[file_index].ypos < BUFFER_Y - 1)
                {
                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0')
                    {
                        if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][Tab_stack[file_index].xpos] == '\0')
                        {
                            Tab_stack[file_index].ypos++;
                            Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + (TokCount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1], "\t") * TAB_WIDE); // Add tab wide
                        }
                        else
                        {
                            n = TokCount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1] - Tab_stack[file_index].xpos, "\t");

                            Tab_stack[file_index].ypos++;
                        }
                    }
                    else
                    {
                        if (!strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
                        {
                            Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1]);
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
                UpdateScrolledScreen(&Tab_stack[file_index]);

                break;

            case HOME:
                // HOME key
                Tab_stack[file_index].xpos = 0;
#ifdef HORIZONTAL_SCROLL

                if (horizontalScroll > 0)
                {
                    horizontalScroll = 0;
                    UpdateScrolledScreen(&Tab_stack[file_index]);
                }
#endif
                break;
            case END:
                // END key
                n = Tab_stack[file_index].xpos;
                Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
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
                    UpdateScrolledScreen(&Tab_stack[file_index]);
                }
                break;
            case CTRLEND:
                // ^END key
                for (int i = 1; i < BUFFER_Y; i++)
                {
                    if (Tab_stack[file_index].strsave[i][0] == '\0')
                    {
                        Tab_stack[file_index].ypos = i;
                        if (strncmp(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1] + NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1]), Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)))
                        {
                            Tab_stack[file_index].ypos++;
                        }

                        if (i > 1)
                        {
                            Tab_stack[file_index].ypos = i - 1;
                        }

                        Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
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
                Tab_stack[file_index].last_y = Tab_stack[file_index].ypos;
                UpdateScrolledScreen(&Tab_stack[file_index]);

                break;
            case CTRLHOME:
                // ^HOME key
                horizontalScroll = 0;
                UpdateHomeScrolledScreen(&Tab_stack[file_index]);
                Tab_stack[file_index].xpos = 0;
                Tab_stack[file_index].ypos = 1;
                Tab_stack[file_index].last_y = Tab_stack[file_index].ypos;

                break;

            case INS:
                // INS key
                insertChar = !insertChar;
                if (cursorSizeInsert)
                {
                    insertChar ? SetCursorSettings(true, CURSIZE_INS) : SetCursorSettings(true, CURSIZE);
                }

                break;

            case DEL: // DEL and S-DEL

                if (CheckKey(VK_SHIFT)) // S-DEL
                {
                    if (Tab_stack[file_index].ypos > 0)
                    {
                        memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 0, BUFFER_X);

                        ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y, wrapSize, 1);
                        strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline));
                        Tab_stack[file_index].xpos = 0;
                    }
                }
                else // DEL key
                {
                    n = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);

                    if (n > 0) // TODO: Fix this bug in the line number 1
                    {
                        DeleteChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].xpos);
                        if (Tab_stack[file_index].xpos < NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                        {

                            gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);

                            print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                            ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].display_y, 1, 1);
                        }
                        else
                        {
                            if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0' && Tab_stack[file_index].ypos < BUFFER_Y - 1) // Don't try to delete a non-existing row
                            {
                                printf("{%d:%d}", NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), Tab_stack[file_index].xpos);
                                getch();
                                n = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                                memset(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] + n, 0, BUFFER_X - n);                                                                                                                   // Empty the new line
                                strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1])); // Concatenate the next line
                                if (Tab_stack[file_index].ypos != 1)
                                {
                                    strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // Add the newline string only if the ypos isn't 1
                                }
                                DeleteRow(Tab_stack[file_index].strsave, Tab_stack[file_index].ypos + 1, BUFFER_Y); // Delete the old row, shifting other rows up
                                Tab_stack[file_index].linecount--;
                                if (!UpdateScrolledScreen(&Tab_stack[file_index]))
                                {
                                    ClearPartial(0, 1, XSIZE, YSIZE - 2);
                                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                                }
                            }
                        }
                    }
                }

                break;

            case F11: // F11 (fullscreen)

                if (dev_tools)
                {
                    isFullScreen = !isFullScreen;

                    /* if (!isFullScreen)
                    {
                        ShowWindow(GetConsoleWindow(), SW_MAXIMIZE); // Show the window in fullscreen
                    }
                    else
                    {
                        ShowWindow(GetConsoleWindow(), SW_RESTORE); // Show the window in fullscreen
                    } */
                    ch = 0;
                }

                break;

            case F12: // F12
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

            continue;
        }
#ifdef _WIN32
        if (ch == CTRLM && !CheckKey(VK_RETURN) && !_NEWTRODIT_OLD_SUPPORT) // ^M = Toggle mouse
        {

            if (!CheckKey(VK_SHIFT))
            {

                ToggleOption(&partialMouseSupport, NEWTRODIT_MOUSE, false);
                c = -2;
                ch = 0;
                continue;
            }
        }

        if ((ch == ENTER && CheckKey(VK_RETURN)) || (ch == ENTER && _NEWTRODIT_OLD_SUPPORT)) // Newline character: CR (13)
#else
        if (ch == ENTER)
#endif
        {
            if (Tab_stack[file_index].ypos < Tab_stack[file_index].bufy - 1)
            {
                if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos + 1][0] != '\0' || Tab_stack[file_index].xpos < NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                {
                    /*  n = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1;
                     Tab_stack[file_index].Ustack = (Undo_stack *)realloc(Tab_stack[file_index].Ustack, sizeof(Undo_stack) * (sizeof(&Tab_stack[file_index].Ustack) + 1));
                     Tab_stack[file_index].Ustack->size = n;

                     Tab_stack[file_index].Ustack->line = malloc(Tab_stack[file_index].Ustack->size);
                     memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);
                     Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); // If it's not a duplicate, the value will change
                     Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                     Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                     Tab_stack[file_index].Ustack->create_nl = true;
                     Tab_stack[file_index].Ustack++->delete_nl = false; */
                    InsertNewRow(&Tab_stack[file_index], &Tab_stack[file_index].xpos, &Tab_stack[file_index].ypos, Tab_stack[file_index].display_y, BUFFER_X, Tab_stack[file_index].newline);
                    Tab_stack[file_index].xpos = AutoIndent(&Tab_stack[file_index]); // Set the X position depending if auto indent is enabled or not
                }
                else
                {
                    strncat(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos++], Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // Add newline to current line
                    Tab_stack[file_index].xpos = AutoIndent(&Tab_stack[file_index]);

                    UpdateScrolledScreen(&Tab_stack[file_index]);
                }

                Tab_stack[file_index].linecount++; // Increment line count
                Tab_stack[file_index].is_modified = true;
            }

            continue;
        }
        if (ch == CTRLE) // ^E = Toggle syntax highlighting / S-^E = Set syntax highlighting rules file
        {
            if (CheckKey(VK_SHIFT))
            {
                PrintBottomString("%s", NEWTRODIT_PROMPT_SYNTAX_FILE);

                memset(syntaxfile, 0, sizeof syntaxfile);
                fgets(syntaxfile, sizeof syntaxfile, stdin);
                if (NoLfLen(syntaxfile) <= 0)
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
                        PrintBottomString("%s", NEWTRODIT_SYNTAX_HIGHLIGHTING_LOADED);
                        Tab_stack[file_index].Syntaxinfo.syntax_file = FullPath(syntaxfile);
                        Tab_stack[file_index].Syntaxinfo.keyword_count = syntaxKeywordsSize;
                    }
                }
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
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
        if (ch == CTRLF) // ^F = Find string
        {

            // Empty values
            memset(find_string, 0, sizeof find_string);
            find_string_index = 0;
            find_count = 0;
            n2 = Tab_stack[file_index].ypos;

            c = 0;

            findInsensitive = false;
            if (CheckKey(VK_SHIFT))
            {
                findInsensitive = true;
                PrintBottomString("%s", NEWTRODIT_PROMPT_FIND_STRING_INSENSITIVE);
            }
            else
            {
                PrintBottomString("%s", NEWTRODIT_PROMPT_FIND_STRING);
            }

            fgets(find_string, sizeof find_string, stdin);
            if (NoLfLen(find_string) <= 0)
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
                n = 0;
                if (Tab_stack[file_index].strsave[i][0] == '\0')
                {
                    break;
                }

                find_string_index = 0;
                while (n >= 0 && c != 27)
                {
                    n = ReturnFindIndex(findInsensitive, Tab_stack[file_index].strsave[i] + find_string_index, find_string);
                    if (n >= 0)
                    {

                        /* printf("{%d:%d:%d:%d}", IsWholeWord(Tab_stack[file_index].strsave[i] + find_string_index, find_string, wholeWordDelims), i, find_string_index, n);
                        getch_n(); */
                        if (!matchWholeWord || (matchWholeWord && IsWholeWord(Tab_stack[file_index].strsave[i] + find_string_index, find_string, wholeWordDelims)) > 0)
                        {

                            n2 = i; //
                            Tab_stack[file_index].ypos = i;

                            SetDisplayY(&Tab_stack[file_index]); // Calculate scroll position

                            UpdateScrolledScreen(&Tab_stack[file_index]);
                            SetCharColor(strlen(find_string), FIND_HIGHLIGHT_COLOR, find_string_index + n + (lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                            SetColor(bg_color);
                            ShowFindMenu();
                            gotoxy(find_string_index + n + (lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(find_string), Tab_stack[file_index].display_y);

                            c = getch_n();
                            if (c != 0x80000000)
                            {
                                if (c == ESC) // Quit find
                                {
                                    break;
                                }
                                else if (c & BIT_ESC0 && (c & ~BIT_ESC0) == F3) // F3 = Next
                                {

                                    if (i >= Tab_stack[file_index].bufy || i > Tab_stack[file_index].linecount)
                                    {
                                        PrintBottomString("%s", NEWTRODIT_FIND_NO_MORE_MATCHES);
                                        getch_n();
                                        Tab_stack[file_index].ypos = n2; // Restore ypos to last found position
                                        break;
                                    }
                                    else
                                    {
                                        c = -3;
                                    }
                                }
                                else if (c & BIT_ESC0 && (c & ~BIT_ESC0) == F4) // F4 = Toggle case sensitive
                                {
                                    ToggleOption(&findInsensitive, NEWTRODIT_FIND_CASE_SENSITIVE, false);
                                    gotoxy(find_string_index + n + (lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(find_string), Tab_stack[file_index].display_y); // Keep the cursor in the same place
                                    getch_n();
                                    c = -1;
                                }
                                else if (c & BIT_ESC0 && (c & ~BIT_ESC0) == F5) // F5 = Full words
                                {
                                    ToggleOption(&matchWholeWord, NEWTRODIT_FIND_MATCH_WHOLE_WORD, false);
                                    gotoxy(find_string_index + n + (lineCount ? Tab_stack[file_index].linecount_wide : 0) + strlen(find_string), Tab_stack[file_index].display_y); // Keep the cursor in the same place
                                    getch_n();
                                    c = -1;
                                }
                                else if (c & BIT_ESC0 && (c & ~(BIT_ESC0) == ALTF4)) // A-F4
                                {
                                    QuitProgram(SInf.color);
                                    c = -1;
                                }
                            }

                            if (c == -3)
                            {
                                find_string_index += n + 1;
                            }
                        }
                    }
                }

                i++;

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
#ifdef _WIN32
        if (ch & BIT_ESC0)
#else
        if (ch & BIT_ESC224)
#endif
        {
            switch (ch & ~(BIT_ESC0))
            {
            case CTRLALTR: // ^A-R (ROT13)
                memset(temp_strsave, 0, BUFFER_X);
                strncpy_n(temp_strsave, Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]));
                if (rot13(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]))
                {
                    ClearPartial(0, Tab_stack[file_index].display_y, XSIZE, 1);
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
                break;
            case CTRLALTU: // A-^U (Uppercase)
                for (int i = 0; i < strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); i++)
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] = toupper(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i]);
                }
                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                break;

            case CTRLALTL: // A-^L (Lowercase)
                for (int i = 0; i < strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); i++)
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i] = tolower(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][i]);
                }
                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                break;
            case CTRLALTN: // A-^N (New file and save)

                if (CheckKey(VK_MENU))
                {

                    NewFile(&Tab_stack[file_index]);
                    PrintBottomString("%s", NEWTRODIT_NEW_FILE_CREATED);
                    getch_n();

                    SaveFile(&Tab_stack[file_index]);

                    DisplayCursorPos(&Tab_stack[file_index]);

                    ShowBottomMenu();
                }
                break;

            case F1: // F1 key
                NewtroditHelp();

                LoadAllNewtrodit();

                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                DisplayCursorPos(&Tab_stack[file_index]);
                break;
            case F2: // F2 key
                if (Tab_stack[file_index].is_untitled || !Tab_stack[file_index].is_saved)
                {
                    if (!SaveFile(&Tab_stack[file_index]))
                    {
                        PrintBottomString("%s", NEWTRODIT_FS_FILE_RENAME);
                        WriteLogFile("%s", NEWTRODIT_FS_FILE_RENAME);
                        ShowBottomMenu();
                        getch_n();
                        ch = 0;
                        continue;
                    }
                }

                PrintBottomString(NEWTRODIT_PROMPT_RENAME_FILE);
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = 0;
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                if (NoLfLen(newname) > 0)
                {
                    if (!CheckFile(newname))
                    {
                        PrintBottomString("%s", NEWTRODIT_PROMPT_OVERWRITE);
                        if (YesNoPrompt())
                        {
                            if (remove(newname) != 0)
                            {
                                PrintBottomString("%s%s", NEWTRODIT_FS_FILE_DELETE, newname);
                                WriteLogFile("%s%s", NEWTRODIT_FS_FILE_DELETE, newname);
                                getch_n();
                                ShowBottomMenu();
                                DisplayCursorPos(&Tab_stack[file_index]);
                                continue;
                            }
                        }
                    }
#ifdef _WIN32
                    if (MoveFile(Tab_stack[file_index].filename, newname))
#else
                    if (rename(Tab_stack[file_index].filename, newname))
#endif
                    {
                        PrintBottomString("%s", NEWTRODIT_FILE_RENAMED, newname);
                        WriteLogFile("%s", NEWTRODIT_FILE_RENAMED, newname);
                        strncpy_n(Tab_stack[file_index].filename, newname, MAX_PATH);

                        UpdateTitle(&Tab_stack[file_index]);
                        NewtroditNameLoad();
                        CenterText(StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), 0);
                        DisplayTabIndex(&Tab_stack[file_index]);

                        RightAlignNewline();
                    }
                    else
                    {
                        PrintBottomString("%s", NEWTRODIT_FS_FILE_RENAME);
                    }

                    getch_n();
                }
                else
                {
                    PrintBottomString("%s", NEWTRODIT_FUNCTION_ABORTED);
                    getch_n();
                }
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                ch = 0;
                break;
            case F5: // F5 key = Run macro
                if (!run_macro)
                {
                    if (!Tab_stack[file_index].is_untitled && Tab_stack[file_index].is_saved)
                    {

#ifdef _WIN32
                        GetFullPathName(Tab_stack[file_index].filename, sizeof(Tab_stack[file_index].filename), tmp, NULL);
#else
                        tmp = strdup(FullPath(Tab_stack[file_index].filename));
                        StartProcess(tmp);
#endif
                        StartProcess(tmp);
                    }
                }
                else
                {
                    tmp = strdup(run_macro);
                    tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_FILE, StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), &n);
                    if (Tab_stack[file_index].is_untitled)
                    {
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_FULL_PATH, Tab_stack[file_index].filename, &n);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_DIR, SInf.dir, &n);
                    }
                    else
                    {
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_FULL_PATH, FullPath(Tab_stack[file_index].filename), &n);
                        get_path_directory(FullPath(Tab_stack[file_index].filename), ptr);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_DIR, ptr, &n);
                        tmp = ReplaceString(strdup(tmp), NEWTRODIT_MACRO_CURRENT_EXTENSION, StrLastTok(Tab_stack[file_index].filename, "."), &n);
                    }
                    WriteLogFile("%s", "Running macro: ", tmp);
                    StartProcess(tmp);
                }

                ch = 0;
                break;
            case F6: // F6 key = Insert date and time

                temp_strsave = GetTime(showMillisecondsInTime);

                if (Tab_stack[file_index].xpos >= BUFFER_X - BUFFER_INCREMENT)
                {
                    tmp = realloc_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], BUFFER_X, BUFFER_X + BUFFER_INCREMENT);
                    if (!tmp)
                    {
                        PrintBottomString("%s", NEWTRODIT_ERROR_OUT_OF_MEMORY);
                        WriteLogFile("%s", NEWTRODIT_ERROR_OUT_OF_MEMORY);
                        getch_n();

                        break;
                    }
                    BUFFER_X += BUFFER_INCREMENT;

                    free(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] = tmp;
                }

                InsertStr(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, Tab_stack[file_index].xpos);

                Tab_stack[file_index].xpos += strlen(temp_strsave);

                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);

                if (Tab_stack[file_index].xpos <= XSIZE)
                {
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }

                break;

            case F9: // F9 = Compile
                if (!Tab_stack[file_index].is_untitled)
                {
                    snprintf(tmp, DEFAULT_BUFFER_X, "%s %s %s -o %s.exe", Tab_stack[file_index].Compilerinfo.path, Tab_stack[file_index].Compilerinfo.flags, FullPath(Tab_stack[file_index].filename), FullPath(Tab_stack[file_index].filename));
                    StartProcess(tmp);
                    printf("[%s]", tmp);
                }

                break;
            case F10:                           // F10 key
                StartProcess("explorer.exe ."); // Open current directory in explorer
                break;
            case SHIFTF5: // S-F5 (Set macro)
                PrintBottomString("%s", NEWTRODIT_PROMPT_CREATE_MACRO);
                fgets(macro_input, sizeof(macro_input), stdin);
                if (NoLfLen(macro_input) <= 0)
                {
                    FunctionAborted(&Tab_stack[file_index]);
                    break;
                }
                macro_input[strcspn(macro_input, "\n")] = 0;

                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                if (!ValidString(macro_input))
                {
                    PrintBottomString("%s", NEWTRODIT_ERROR_INVALID_MACRO);
                    getch_n();
                    ShowBottomMenu();
                    DisplayCursorPos(&Tab_stack[file_index]);
                    break;
                }

                strncpy_n(run_macro, macro_input, MACRO_ALLOC_SIZE);
                PrintBottomString("%s%s", NEWTRODIT_MACRO_SET, macro_input);
                WriteLogFile("%s%s", NEWTRODIT_MACRO_SET, macro_input);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                break;
            case SHIFTF10:               // S-F10 key
                StartProcess("cmd.exe"); // Open command prompt
                break;
            case CTRLF1: // ^F1 key
                SetCursorSettings(false, GetConsoleInfo(CURSOR_SIZE));
                ClearPartial(0, 1, XSIZE, YSIZE - 1);
                n = strlen(join("Contribute at ", newtrodit_repository));
                SetColor(fg_color);
                ClearPartial((XSIZE / 2) - (n / 2) - 1, (YSIZE / 2) - 3, n + 2, 7); // Create a box
                CenterText("About Newtrodit", (YSIZE / 2) - 2);
                CenterText(ProgInfo(), (YSIZE / 2));
                // I know it's not the best way to do it, but it works
                CenterText(join("Contribute at ", newtrodit_repository), (YSIZE / 2) + 2);
                getch_n();
                ClearPartial(0, 1, XSIZE, YSIZE - 1);
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                SetCursorSettings(true, GetConsoleInfo(CURSOR_SIZE));

                break;
            case CTRLF4: // ^F4
                if (!CheckKey(VK_SHIFT))
                {

                    NewFile(&Tab_stack[file_index]);
                    PrintBottomString("%s", NEWTRODIT_FILE_CLOSED);
                    c = -2;
                    ch = 0;
                    break;
                }

            case ALTF4: // A-F4 key
                if (!CheckKey(VK_CONTROL))
                {
                    QuitProgram(SInf.color);
                    ShowBottomMenu();
                    continue;
                }
                break;

            /* This one (148) doesn't work on Linux */
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
                    PrintBottomString("%s", NEWTRODIT_INFO_NO_FILES_TO_SWITCH);
                    c = -2;
                }
                break;

            case ALTHOME: // A-HOME key "smart home" (go to the first non-whitespace character)
                Tab_stack[file_index].xpos = strspn(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], " \t");
                break;
            case ALTEND: // A-END key "smart end" (go to the last non-whitespace character)
                n = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);

                while ((Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] == ' ' || Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] == '\t') && Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][n - 1] != '\0')
                {
                    n--;
                }

                Tab_stack[file_index].xpos = n + (TokCount(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "\t") * TAB_WIDE);
                if (Tab_stack[file_index].xpos < 0)
                {
                    Tab_stack[file_index].xpos = 0;
                }

                break;
            }

            ch = 0;
            continue;
        }

        if (ch == CTRLS) // ^S
        {
            (CheckKey(VK_SHIFT)) ? (n = 2) : (n = 0);
            n2 = false; // Refresh screen
            memset(save_dest, 0, MAX_PATH);
            strncpy_n(save_dest, Tab_stack[file_index].filename, MAX_PATH); // If no input is given
            if (!Tab_stack[file_index].is_saved || n == 2)
            {
                PrintBottomString("%s", n != 2 ? NEWTRODIT_PROMPT_SAVE_FILE : NEWTRODIT_PROMPT_SAVE_FILE_AS);
                fgets(save_dest, MAX_PATH, stdin);
                if (NoLfLen(save_dest) <= 0)
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
                        DisplayCursorPos(&Tab_stack[file_index]);
                        continue;
                    }
                }
                n2 = true;
                LoadAllNewtrodit();
                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
            }
#ifdef _WIN32
            if (GetFileAttributes(Tab_stack[file_index].filename) & FILE_ATTRIBUTE_READONLY)
#else
            if (Tab_stack[file_index].is_readonly)
#endif
            {
                if (!n2)
                {
                    LoadAllNewtrodit();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                }

                PrintBottomString(NEWTRODIT_FS_READONLY_SAVE);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                continue;
            }
            fp_savefile = fopen(save_dest, "wb"); // (Re)open the file to write
            if (!fp_savefile || !ValidFileName(save_dest))
            {
                if (!n2)
                {
                    LoadAllNewtrodit();
                    DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                }
                PrintBottomString(NEWTRODIT_FS_FILE_SAVE_ERR);
                getch_n();
                ShowBottomMenu();
                continue;
            }
            if (WriteBuffer(fp_savefile, &Tab_stack[file_index])) // Write the file content
            {
                Tab_stack[file_index].is_saved = true;
                Tab_stack[file_index].is_modified = false;
                Tab_stack[file_index].is_untitled = false;

                UpdateTitle(&Tab_stack[file_index]);
                Tab_stack[file_index].filename = strdup(save_dest);
                NewtroditNameLoad();
                DisplayTabIndex(&Tab_stack[file_index]);
                CenterText(StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), 0);

                RightAlignNewline();
                DisplayTabIndex(&Tab_stack[file_index]);

                PrintBottomString(NEWTRODIT_FILE_SAVED);
                WriteLogFile(join(NEWTRODIT_FILE_WROTE_BUFFER, Tab_stack[file_index].filename));

                c = -2;
            }
            else
            {
                PrintBottomString(NEWTRODIT_FS_FILE_SAVE_ERR);
                WriteLogFile(NEWTRODIT_FS_FILE_SAVE_ERR);

                getch_n();
            }

            fclose(fp_savefile);

            ch = 0;
            continue;
        }
        if (ch == CTRLV) // ^V
        {
#ifdef _WIN32
            if (OpenClipboard(0))
            {
                buffer_clipboard = (char *)GetClipboardData(CF_TEXT);
                if (buffer_clipboard != NULL)
                {
                    n = 0;
                    // Split the string with the newline character as delimiter and print them separately
                    ptr = strtok(buffer_clipboard, "\n");
                    tmp = calloc(strlen(ptr) + 1, sizeof(char)); // Allocate memory and initialize all bytes to 0
                    while (ptr != NULL)
                    {
                        for (int i = 0; i < strlen(ptr); i++) // i is also character 9 :)
                        {
                            tmp = InsertChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], ptr[i], Tab_stack[file_index].xpos - 1);
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                        }
                        printf("%s\n", tmp);
                        ptr = strtok(NULL, "\n");
                        n++;
                    }

                    Tab_stack[file_index].xpos = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
            }
            else
            {
                PrintBottomString(NEWTRODIT_ERROR_CLIPBOARD_COPY);
                WriteLogFile(NEWTRODIT_ERROR_CLIPBOARD_COPY);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
            }
            CloseClipboard();
#endif
            ch = 0;
            continue;
        }

        if (ch == CTRLD) // ^D (Debug tool/dev mode) / S-^D = Toggle dev mode
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
        if (ch == CTRLW) // ^W
        {
            if (!CheckKey(VK_SHIFT))
            {
                CloseFile(&Tab_stack[file_index]);
                PrintBottomString(NEWTRODIT_FILE_CLOSED);
                getch_n();

                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                ch = 0;
                continue;
            }
        }

        if (ch == CTRLQ) // ^Q = Quit program
        {

            QuitProgram(SInf.color);
            ShowBottomMenu();
            SetColor(bg_color);
            ch = 0;
            continue;
        }
        if (ch == CTRLX) // ^X = Cut
        {
            if (useOldKeybindings)
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
                        Tab_stack[file_index].Ustack->line = malloc(sizeof(char) * (Tab_stack[file_index].Ustack->size + 1));
                        memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);

                        Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                        Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                        Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                        Tab_stack[file_index].Ustack->create_nl = false;
                        Tab_stack[file_index].Ustack++->create_nl = false;
#ifdef _WIN32
                        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1);
                        memcpy(GlobalLock(hMem), Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1); // Copy line to the clipboard
                        GlobalUnlock(hMem);
                        OpenClipboard(0);
                        EmptyClipboard();
                        SetClipboardData(CF_TEXT, hMem);
                        CloseClipboard();
#endif
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
        if (ch == CTRLBS) // ^BS
        {
            if (Tab_stack[file_index].xpos > 0)
            {
                /*  Tab_stack[file_index].Ustack->size = strlen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]) + 1;
                 Tab_stack[file_index].Ustack->line = malloc(Tab_stack[file_index].Ustack->size);
                 memset(Tab_stack[file_index].Ustack->line, 0, Tab_stack[file_index].Ustack->size);
                 Tab_stack[file_index].Ustack->line = strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]); // If it's not a duplicate, the value will change
                 Tab_stack[file_index].Ustack->line_count = Tab_stack[file_index].ypos;
                 Tab_stack[file_index].Ustack->line_pos = Tab_stack[file_index].xpos;
                 Tab_stack[file_index].Ustack->create_nl = false;
                 Tab_stack[file_index].Ustack->delete_nl = false; */

                bs_tk = TokBackPos(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], "()[]{}\t ", "?!");
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
                                snprintf(tmp, DEFAULT_ALLOC_SIZE, "%s%s%s", (char *)memset(ptr, 0, TAB_WIDE), (char *)memset(ptr, ' ', TAB_WIDE), (char *)memset(ptr, 0, TAB_WIDE));
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

                    for (int i = 0; i < (Tab_stack[file_index].xpos - bs_tk <= 0) ? 0 : Tab_stack[file_index].xpos - bs_tk; i++) // TODO: Use DeleteStr instead
                    {
                        temp_strsave = DeleteChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], bs_tk);
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
                    InsertDeletedRow(&Tab_stack[file_index]);
                    if (!UpdateScrolledScreen(&Tab_stack[file_index]))
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

        if (ch == CTRLY) // ^Y = Redo
        {

            /* strncpy_n(undo_stack, Tab_stack[file_index].strsave[undo_stack_line], BUFFER_X);
            strncpy_n(Tab_stack[file_index].strsave[undo_stack_line], redo_stack, BUFFER_X);
            LoadAllNewtrodit();
            FunctionAborted(&Tab_stack[file_index]);
            fflush(stdout); */

            ch = 0;
            continue;
        }

        if (ch == TAB && !CheckKey(VK_TAB)) // ^I = File info
        {
            if (!CheckKey(VK_SHIFT))
            {
                CountBufferLines(&Tab_stack[file_index]);
                PrintBottomString("File: \'%s\', size: %lld bytes (%u lines). File type: %s", StrLastTok(Tab_stack[file_index].filename, PATHTOKENS), Tab_stack[file_index].size, Tab_stack[file_index].linecount, Tab_stack[file_index].language);
                c = -2;
                ch = 0;
            }
        }
        if (ch == CTRLZ) // ^Z = Undo
        {
            if (undo_stack_tree > 0)
            {
                undo_stack_tree--;
                Tab_stack[file_index].xpos = Tab_stack[file_index].Ustack->line_pos;
                Tab_stack[file_index].ypos = Tab_stack[file_index].Ustack->line_count;
                if (Tab_stack[file_index].Ustack->create_nl == true)
                {
                    InsertDeletedRow(&Tab_stack[file_index]);
                }
                if (Tab_stack[file_index].Ustack->delete_nl == true)
                {
                    InsertRow(Tab_stack[file_index].strsave, Tab_stack[file_index].xpos, Tab_stack[file_index].ypos, Tab_stack[file_index].Ustack->line);
                    Tab_stack[file_index].strsave[Tab_stack[file_index].Ustack->line_count] = strdup(Tab_stack[file_index].Ustack->line);
                }
                if (!UpdateScrolledScreen(&Tab_stack[file_index]))
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
                    xpos = NoLfLen(Tab_stack[file_index].Ustack->line);
                } */

                gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].ypos);
                print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
            }

            ch = 0;
            continue;
        }

        if (ch == CTRLH && !CheckKey(BS) && CheckKey(VK_CONTROL)) // ^H = Replace string / S-^H = Same as F1 (opens help)
        {
            ch = 0;

            if (CheckKey(VK_SHIFT))
            {
                NewtroditHelp();

                LoadAllNewtrodit();

                DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                DisplayCursorPos(&Tab_stack[file_index]);
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
                if (NoLfLen(find_string) <= 0)
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
                PrintBottomString("Replaced %d occurrences of %s", replace_count, find_string);
                getch_n();
                ShowBottomMenu();
                DisplayCursorPos(&Tab_stack[file_index]);
                if (replace_count > 0)
                {
                    Tab_stack[file_index].is_modified = true;
                }
            }
            continue;
        }

#ifdef _WIN32
        if ((ch == BS && _NEWTRODIT_OLD_SUPPORT == 1) || (ch == BS && CheckKey(BS) && !CheckKey(VK_CONTROL))) // BS key (Not Control-H)
#else
        if (ch == BS)
#endif
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

                    DeleteChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], --Tab_stack[file_index].xpos);
                    Tab_stack[file_index].xpos--;
                    if (hasNewLine)
                    {
                        strncat(tmp, Tab_stack[file_index].newline, strlen(Tab_stack[file_index].newline)); // strcat for CRLF newline
                    }

                    // strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], tmp, BUFFER_X);
                    ClearPartial((lineCount ? Tab_stack[file_index].linecount_wide : 0) + (NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos])), Tab_stack[file_index].display_y, 2, 1); // Clear the character
                    gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                    print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                }
                else
                {
                    Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos - 1] = '\0';
                    fputs("\b \b", stdout);

                    if (syntaxHighlighting)
                    {
                        gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                        print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    }
                    Tab_stack[file_index].xpos -= 2;
                }
            }
            else
            {
                /* Act as END key */
                if (Tab_stack[file_index].ypos > 1)
                {
                    Tab_stack[file_index].is_modified = true;
                    // TODO: Fix bugs
                    n = NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1]); // Store the length of the line so we can change the X position later
                    memcpy(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1] + NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos - 1]), Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], NoLfLen(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]));
                    DeleteRow(Tab_stack[file_index].strsave, Tab_stack[file_index].ypos, BUFFER_Y);

                    Tab_stack[file_index].linecount--;
                    Tab_stack[file_index].ypos--;
                    Tab_stack[file_index].xpos = n; // Assign the X position to the old length of the previous file

                    if (!UpdateScrolledScreen(&Tab_stack[file_index]))
                    {
                        /* ClearPartial(
                            (lineCount ? Tab_stack[file_index].linecount_wide : 0),
                            Tab_stack[file_index].display_y,
                            wrapSize),
                            (YSIZE - (Tab_stack[file_index].display_y + 2)) > 0 ? YSIZE - (Tab_stack[file_index].display_y + 2) : 1
                            ); */
                        ClearPartial(
                            (lineCount ? Tab_stack[file_index].linecount_wide : 0),
                            Tab_stack[file_index].display_y,
                            wrapSize,
                            YSIZE - Tab_stack[file_index].display_y - 1);
                        ClearPartial(0, Tab_stack[file_index].display_y, (lineCount ? Tab_stack[file_index].linecount_wide : 0), 1);

                        DisplayFileContent(&Tab_stack[file_index], stdout, 0);
                    }
                }

                Tab_stack[file_index].xpos--; // Because 1 will be added to xpos in the next iteration
            }
        }
        else
        {
            if ((ch > 31 && ch != BS) || (ch == TAB && CheckKey(TAB))) // Printable character
            {

                Tab_stack[file_index].is_modified = true;

                if (!insertChar && ch != 9) // Insert key not pressed
                {

                    if (Tab_stack[file_index].strsave[Tab_stack[file_index].ypos][Tab_stack[file_index].xpos] != '\0')
                    {
                        if (ch == 9)
                        {
                            relative_xpos[Tab_stack[file_index].ypos] += TAB_WIDE;
                        }
                        Tab_stack[file_index].strsave[Tab_stack[file_index].ypos] = InsertChar(strdup(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]), ch, Tab_stack[file_index].xpos);
                        RefreshLine(&Tab_stack[file_index], Tab_stack[file_index].ypos, Tab_stack[file_index].display_y, false); // No need to clear the line as it will get overwritten
                        ch = 0;
                    }
                }

                if (CheckKey(VK_TAB) && ch == TAB) // TAB key
                {
                    if (convertTabtoSpaces)
                    {
                        for (int i = 0; i < TAB_WIDE; i++) // i is also character 9 :)
                        {
                            temp_strsave = InsertChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], ' ', Tab_stack[file_index].xpos);
                            strncpy_n(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], temp_strsave, BUFFER_X);
                        }
                        Tab_stack[file_index].xpos += (TAB_WIDE - 1);
                        gotoxy((lineCount ? Tab_stack[file_index].linecount_wide : 0), Tab_stack[file_index].display_y);
                        print_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos]);
                    }
                    else
                    {
                        temp_strsave = InsertChar(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], 9, Tab_stack[file_index].xpos);
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
                                color_line(Tab_stack[file_index].strsave[Tab_stack[file_index].ypos], n, Tab_stack[file_index].Syntaxinfo.override_color);
                            }
                        }
                    }
                }
            }
            else
            {
                if (ch != 0 && ch <= 26 && ch != ENTER)
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