#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Define boolean
typedef int bool;
#define false 0
#define true  1

// Define path depth restriction
#define MAX_PATH 260

#define YSIZE 200
#define PATHTOKENS "\\/"
int wrapSize = 100; // Default wrap size

int TokLastPos(char *s, char *token) {
  int lastpos = -1;
  for (int i = 0; i < strlen(s); i++) {
    for (int j = 0; j < strlen(token); j++) {
      if (s[i] == token[j]) {
        lastpos = i;
      }
    }
  }
  return lastpos;
}

char *StrLastTok(char *tok, char *char_token) {
  int pos = -1; // Always initialize variables
  if ((pos = TokLastPos(tok, char_token)) == -1) {
    return strdup(tok);
  } else {
    return strdup(tok + pos + 1);
  }
}

char *get_path_directory(char *path, char *dest) // Not a WinAPI function
{
  strcpy(dest, path);
  int tmp_int = TokLastPos(path, "\\");
  if (tmp_int != -1) {

    memset(dest + tmp_int, 0, MAX_PATH - tmp_int);

    if (dest[strlen(dest) - 1] != '\\') {
      dest[strlen(dest)] = '\\';
    }
    return dest;
  } else {
    return NULL;
  }
}

int ValidFileName(char *filename)
{
	return strpbrk(filename, "/") == NULL;
}

int LocateFiles(int show_dir, char *file, int startpos) {
  int n = startpos, total = startpos;

  bool wildcard = false;
  DIR *dir_stream;                 // Directory stream
  struct dirent *dir;
  char cwd[MAX_PATH];              // Allocation for a directory stream
  int max_print = wrapSize;

  // Test if "file" is a wildcard (*)
  if (strstr(file, "*")) {
		wildcard = true;
  } else {
    // Check if "file" is a valid format
    if (strstr(file, "/")) {
      printf("Invalid file name\n");
      return 0;
    }
  }
  // Allocate memory to hold ???
  char *dir_tmp = (char *)calloc(MAX_PATH * 2, sizeof(char));
	char *out_dir = (char *)calloc(MAX_PATH * 2, sizeof(char));

  if (!wildcard && get_path_directory(file, out_dir))
	{
		chdir(out_dir);
		file = StrLastTok(strdup(file), PATHTOKENS);
	}

  if (getcwd(cwd, sizeof(cwd)) == NULL)
  {
    printf("Cannot obtain current working directory path\n");
    return 0;
  }

  dir_stream = opendir(".");
  if (dir_stream) // Check if dir steam was obtained
  {
    printf("Current directory: %s\n", cwd);

    // Check for wildcard, if so print all files.
    while ((dir = readdir(dir_stream)) != NULL)
    {
      struct stat attr;         // Struct to store file attributes
      stat(dir->d_name, &attr); // Pull file attributes
      struct tm *tm = localtime(&attr.st_mtim.tv_sec); // Convert time in seconds to Years, Months, etc...
      int fsize = S_ISREG(attr.st_mode) ? attr.st_size : 0;
      if (strstr(dir->d_name, file) || wildcard)
      {
        if (n < (YSIZE - 3) + startpos)
        {
          if (dir->d_type == 4)
          {
            if (show_dir == false) {
              continue;
            }
            printf(" %02d/%02d/%04d %02d:%02d:%02d\t<DIR>\t%14d\t%.*s\n",
				      tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
				      tm->tm_hour%12, tm->tm_min, tm->tm_sec,
				      fsize, max_print, dir->d_name);
          }
          else
          {
            printf(" %02d/%02d/%04d %02d:%02d:%02d\t\t%14d\t%.*s\n",
				      tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
					    tm->tm_hour%12, tm->tm_min, tm->tm_sec,
					    fsize, max_print, dir->d_name);
          }

          n++;
        }

        total++;
      }
    }
  }
  else
  {
    printf("Cannot obtain current working directory stream\n");
  }

  if (!total)
	{
    printf("File not found.\n");
  }
  else
  {
    printf("Found %d files in total.\n", total);
  }
  closedir(dir_stream);

  return 0;
}


int main() {
  LocateFiles(false, "cursor", 0);

  return 0;
}
