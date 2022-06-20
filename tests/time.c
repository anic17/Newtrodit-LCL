
/* localtime example */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

int main()
{
  struct stat attr;
  stat("testing.c", &attr);

  printf("%ld", attr.st_mtime);

  return 0;
}


/*
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef int bool;

#define false 0
#define true 1


int GetTime(bool display_ms) {
  char *time_buf = (char *)malloc(512);
  if (!time_buf) {

    return NULL;
  }

  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /*
  if (display_ms) {
    sprintf(time_buf, "%02d:%02d:%02d %02d/%02d/%04d",
        lt->tm_hour%12, lt->tm_min, lt->tm_sec,
        lt->tm_mday, lt->tm_mon+1, 1900+lt->tm_year);
  } else {
    sprintf(time_buf, "%02d:%02d:%02d %02d/%02d/%04d",
        lt->tm_hour%12, lt->tm_min, lt->tm_sec,
        lt->tm_mday, lt->tm_mon+1, 1900+lt->tm_year);
  }
  //return time_buf;
  return lt->;
}


int main() {
  printf("%d", GetTime(false));

  return 0;
}
*/
