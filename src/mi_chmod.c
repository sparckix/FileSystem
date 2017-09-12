/* programa que cambia los permisos de acceso de un fichero: mi_chmod */

#include "../include/directorios.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf
		    ("¡Numero de argumentos incorrecto!\nUso de la función: ./mi_chmod disco MODO /ruta\t donde MODO debera indicarse en octal (0-7)\n");
		return -1;
	}
	bmount(argv[1]);
	mi_chmod(argv[3], atoi(argv[2]));
	bumount();
	return 0;
}
