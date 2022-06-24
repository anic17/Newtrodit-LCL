#include <stdio.h>

int main() {
  printf("Testing me :)aaaaaaaaaaaaaaaaaaaaaaaa\x1b[3B\x1b[2;10H");

  // Used to hold the script open
  getchar();

  return 0;
}
