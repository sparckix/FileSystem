/* programa que crea un directorio: mi_mkdir*/

#include "../include/directorios.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf
		    ("Numero de argumentos incorrecto!\n Uso de la función: ./mi_mkdir disco MODO /RUTA.\n");
		return -1;
	}
	char *j = (argv[3] + strlen(argv[3]) - 1);
	if (*j != '/') {
		printf
		    ("La ruta tiene que ser un directorio! (no termina en /)\n");
		return -1;
	}
	bmount(argv[1]);
	mi_creat(argv[3], atoi(argv[2]));
	bumount();
	return 0;
}
