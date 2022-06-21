#include <stdio.h>

int main() {
  // \x30  - Blinking block
  // \x31  - Blinking block (Default)
  // \x32  - Steady   block
  // \x33  - Blinking underline
  // \x34  - Steady   underline
  // \x35  - Blinking bar
  // \x36  - Steady   bar

  printf("\x1b[\x33 q");

  // used to hold the script open
  getchar();

  return 0;
}
