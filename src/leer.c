#include <stdio.h>
#include "../include/bloques.h"
#include "../include/ficheros_basico.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf
		    ("Número de argumentos erróneo en leer.c\nUso: ./leer <archivo> <ninodo>\n");
		return -1;
	}
	int ninodo = atoi(argv[2]);
	bmount(argv[1]);
	unsigned int offset = 0;
	unsigned char buffer[BLOCKSIZE];
	int prueba;
	while ((prueba = mi_read_f(ninodo, buffer, offset, BLOCKSIZE)) > 0
	       || prueba == -1) {
		if (prueba != -1) {
			write(1, buffer, BLOCKSIZE);
		}
		offset = offset + BLOCKSIZE;
	}
	bumount();
	return 0;
}
