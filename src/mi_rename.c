/* programa que crea un directorio: mi_rename*/

#include "../include/directorios.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf
		    ("Numero de argumentos incorrecto!\n Uso de la función: ./mi_rename disco /RUTA nuevo_nombre.\n");
		return -1;
	}
	/*char* j = (argv[3]+strlen(argv[3])-1);
	   if(*j != '/') {
	   printf("La ruta tiene que ser un directorio! (no termina en /)\n");
	   return -1;
	   } */
	bmount(argv[1]);
//   char nombre[60] = argv[3];
	mi_rename(argv[2], argv[3]);
	bumount();
	return 0;
}
