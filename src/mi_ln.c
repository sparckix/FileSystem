/* programa que crea enlaces físicos a un fichero: mi_ln */

#include "../include/directorios.h"

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf
		    ("Numero de argumentos incorrecto!\nUso de la funcion: ./mi_ln disco /ruta /enlace\n");
		return -1;
	}
	bmount(argv[1]);

	mi_link(argv[2], argv[3]);

	bumount();
	return 0;
}
