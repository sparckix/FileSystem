#include <stdio.h>
#include "../include/bloques.h"
int main(int argc, char **argv)
{
	if (unlikely(argc < 3)) {
		printf
		    ("Wrong syntax, expected 3, found %d. Use ./mi_mkfs <filesystem name> <# blocks>\n",
		     argc);
		return -1;
	}
	char *path;
	path = *++argv;
	bmount(path);
	int iter;
	int num_bloques;
	num_bloques = my_atoi(*++argv);
	unsigned char var[BLOCKSIZE];
	memset(var, 0, sizeof(var));
	for (iter = 0; iter < num_bloques; iter++) {
		bwrite(iter, var);
	}
	unsigned int ninodos = num_bloques / 4;
	initSB(num_bloques, ninodos);
	initMB(num_bloques);
	initAI(ninodos);
	reservar_inodo('d', 7);
	bumount();
	return 0;
}

/* Our own implementation of the atoi function. Returns 0
 * if string cannot be parsed.
 */
int my_atoi(char *s)
{
	int n = 0;
	for (; *s >= '0' && *s <= '9'; *s++)
		n = 10 * n + (*s - '0');
	return n;
}
