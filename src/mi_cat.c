/* programa que muestra el contenido de un fichero por pantalla: mi_cat */

#include "../include/directorios.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf
		    ("¡Numero de argumentos incorrecto!\nUso de la función: ./mi_cat disco /fichero\n");
		return -1;
	}

	bmount(argv[1]);

	//Se comprueba que la ruta se corresponda con un fichero, si es un directorio NO podrá hacerse un cat
	struct STAT stat;
	if (mi_stat(argv[2], &stat) < 0)
		return -1;
	if (stat.tipo == 'd') {
		printf("%s es un directorio! No puede hacerse mi_cat\n",
		       argv[2]);
		bumount();
		return -1;
	}
	//Si se trata de un fichero continuamos con el cat
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, '\0', strlen(buffer));
	unsigned int bytes_leidos = 0;
	unsigned int i = 0;
	bytes_leidos = mi_read(argv[2], buffer, i, BLOCKSIZE);
	while (bytes_leidos > 0) {
		/*leemos hasta que el numero de bytes leídos sea mas pequeño que BLOCKSIZE, 
		   esto indicara que se ha llegado al final del fichero */
		write(1, buffer, bytes_leidos);
		memset(buffer, '\0', strlen(buffer));
		i++;
		bytes_leidos =
		    mi_read(argv[2], buffer, i * BLOCKSIZE, BLOCKSIZE);
	}

	bumount();
	return 0;
}
