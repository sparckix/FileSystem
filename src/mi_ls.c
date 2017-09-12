/* programa que liste los directorios de un fichero: mi_ls */

#include "../include/directorios.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf
		    ("¡Numero de argumentos incorrecto!\nUso de la funcion: ./mi_ls disco /directorio\n");
		return -1;
	}
	bmount(argv[1]);
	char buffer[70000];
	memset(buffer, '\0', strlen(buffer));
	int listaDir = mi_dir(argv[2], buffer);
	printf("\n");
	int x;
	for (x = 0; x < strlen(buffer); x++) {
		if (buffer[x] == '|')
			printf("\n");
		else
			printf("%c", buffer[x]);
	}
	printf("\nLista directorios: %d\n\n", listaDir);
	bumount();
	return 0;
}
