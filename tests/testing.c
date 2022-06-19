#include "linux_newtrodit_core_testing.h"

int main() {
	/* Moving cursor  | Passes
	gotoxy(10, 10);
	puts("Hello");
	gotoxy(10, 9);
	*/

	/* Changing cursor visibility  | Passes
	DisplayCursor(false);
	*/

	/* Enter & exit alt buffer  | Passes (seems to create newlines for no reason though *fixed*)
	EnterAltConsoleBuffer();
	puts("Hello");
	RestoreConsoleBuffer();
	*/



	/* Clear screen  | Passes
	ClearScreen();
	*/

	// Clear partial  | Passes
	//EnterAltConsoleBuffer();
	//printf("Hello from line 1\nHello from line 2\nHello from line 3\nHello from line 4\ntest");
	//ClearPartial(3, 1, 5, 3);
	//gotoxy(0, 20);


	/* Testing ideas */

	// Seting default color  | Works
	//printf("\x1B[1;31m");
	//printf("Testing some colour :)\n");
	//printf("Here we gooooo!");
	//printf("\x1B[0m");
	//printf("More testing no color");

	// Setting char color
	EnterAltConsoleBuffer();
	printf("Testing some color :)");
	gotoxy(5, 0);
	printf("\x1B[1;31m");
	gotoxy(0, 3);
}
