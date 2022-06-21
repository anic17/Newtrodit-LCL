#include <stdio.h>

int main() {
  if (0) {
    printf("\x1B[?25h");
  } else {
    printf("\x1B[?25l");
  }

  // used to hold open the script
  getchar();
}
