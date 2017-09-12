
#include <stdio.h>
#include "../include/ficheros.h"

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf
		    ("Número de argumentos erróneo en escribir.c\nUso: ./escribir <archivo> <nº inodos a escribir> <offset>\n");
		return -1;
	}
	int it = atoi(argv[2]);
	int i, j;
	unsigned int offset = atoi(argv[3]);
	if (offset < 0) {
		printf("Offset introducido incorrecto\n");
		return -1;
	}
	int ninodo;
	int resultado = 0;
	bmount(argv[1]);
	/*ninodo = reservar_inodo('f',6);
	   printf("Número de inodo reservado: %d\n",ninodo);
	   resultado=mi_write_f(ninodo,"Esto es una tonteria\0",5120,21);
	   resultado=mi_write_f(ninodo,"Esto es una tonteria\0",256000,21);
	   resultado=mi_write_f(ninodo,"Esto es una tonteria\0",30720000,21);
	   resultado=mi_write_f(ninodo,"Esto es una tonteria\0",71680000,21);
	 */
	for (i = 0; i < it; i++) {
		//ninodo = reservar_inodo('f',6);
		for (j = 0; j < 1; j++) {
			resultado =
			    mi_write_f(ninodo, "Esto es una tonteria\0", offset,
				       21);
			if (resultado == -1) {
				printf("Error al escribir");
				continue;
			}
			offset = offset + resultado;
		}
		printf("Número de inodo reservado: %d\n", ninodo);
	}
	bumount();
	return 0;
}
