#include <stdio.h>
#include <stdlib.h>

char* hex_to_ansi(int num)
{
    char hex_table[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    char ansi_table[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 91, 95, 92, 96, 93, 97};
    char* buf = (char*)calloc(1, sizeof(char) * 32); // Allocate enough memory
    if (num < 0 || num > 0xff) // Keep a valid number range
    {
        return NULL;
    }
    int font = 0, background = 0;
    if (num > 0xf)
    {
        font = ansi_table[num >> 4];
        background = ansi_table[num &0xf]+10;
        snprintf(buf, 32*sizeof(char), "\x1b[%d;%dm", background, font);
    }
    else
    {
        font = ansi_table[num];
        snprintf(buf, 32*sizeof(char), "\x1b[%dm", font);
    }
    return buf;
}

int main() {
  printf("%sTest?", hex_to_ansi(0x34));

  return 0;
}
