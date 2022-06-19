#include <linux/input-event-codes.h>
#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>

const char *keydev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";

int main (int argc, char *argv[])
{
  sleep(3);
  struct input_event ev;
  FILE *indevice = fopen(keydev, "r");
  while (fread(&ev, sizeof(ev), 1, indevice) == 1)
    if (ev.type == EV_KEY && ev.code == KEY_LEFTSHIFT)
      switch (ev.value) {
        case 1: printf("Shift pressed"); return 1; // What do I return if pressed???
        case 2: printf("Shift repeat"); return 1;
        default: break;
      }
  fclose(indevice);
  return 0;
}
