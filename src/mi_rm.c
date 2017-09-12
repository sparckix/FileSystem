/* programa que borra ficheros y directorios: mi_rm */

#include "../include/directorios.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf
		    ("¡Numero de argumentos incorrecto!\nUso de la función: ./mi_rm disco /ruta\n");
		return -1;
	}
	bmount(argv[1]);

	//si se trata de un directorio debera comprobarse que este vacio para poder borrarlo
	struct STAT stat;
	if (mi_stat(argv[2], &stat) < 0)
		return -1;
	if (stat.tipo == 'd' && stat.tamEnBytesLog != 0) {
		printf
		    ("%s se trata de un directorio y NO esta vacio!\n No se puede eliminar\n",
		     argv[2]);
		bumount();
		return -1;
	}
	//en caso contrario podemos borrarlo
	mi_unlink(argv[2]);

	bumount();
	return 0;
}
