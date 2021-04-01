#include <stdio.h>
#include <stdlib.h>

static char buff [256];
static char* string;
int main ()
{
	printf ("Please input a string: ");
	gets (buff);
    string = buff;
	printf ("\nYour string is: %s\n", string);
}