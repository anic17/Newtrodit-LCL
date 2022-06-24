#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <unistd.h>
#include <termios.h>

int getch(void)
{
  char buf = 0;
  struct termios old;
  fflush(stdout);
  if(tcgetattr(0, &old) < 0) {
    perror("tcsetattr()");
  }
  old.c_lflag &= ~(ICANON | ISIG);
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if(tcsetattr(0, TCSANOW, &old) < 0) {
    perror("tcsetattr ICANON");
  }
  if(read(0, &buf, 1) < 0) {
    perror("read()");
  }
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if(tcsetattr(0, TCSADRAIN, &old) < 0) {
    perror("tcsetattr ~ICANON");
  }
  return buf;
 }

int main() {
  int c = getch();

  printf("%d", c);
  return 0;
}