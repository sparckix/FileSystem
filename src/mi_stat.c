/* programa que muestra la metainformacion de un fichero: mi_stat */

#include "../include/directorios.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf
		    ("¡Numero de argumentos incorrecto!\nUso de la funcion: ./mi_stat disco /ruta\n");
		return -1;
	}
	bmount(argv[1]);
	struct STAT stat;
	if (mi_stat(argv[2], &stat) == -1) {
		bumount();
		printf("Error al obtener los datos del fichero o directorio");
		return -1;
	}

	printf("Ruta: %s\n", argv[2]);
	printf("Tamaño fichero: %d\tBloques ocupados: %d\t",
	       stat.tamEnBytesLog, stat.numBloquesOcupados);
	if (stat.tipo == 'd')
		printf("Tipo: directorio\n");
	else
		printf("Tipo: fichero\n");

	printf("Disco: %s\tNumero de links: %d\n", argv[1], stat.nlinks);
	printf("Permisos: (%d/", stat.permisos);
	if (stat.tipo == 'd')
		printf("D");
	else
		printf("-");

	if (stat.permisos & 4)
		printf("R");
	else
		printf("-");

	if (stat.permisos & 2)
		printf("W");
	else
		printf("-");

	if (stat.permisos & 1)
		printf("X");
	else
		printf("-");

	printf(")\n");

	struct tm *tm;

	tm = localtime(&stat.atime);
	printf
	    ("Fecha y hora del ultimo acceso a datos (atime): %d-%02d-%02d %02d:%02d:%02d\n",
	     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
	     tm->tm_min, tm->tm_sec);

	tm = localtime(&stat.mtime);
	printf
	    ("Fecha y hora de la última modificacion de datos (mtime): %d-%02d-%02d %02d:%02d:%02d\n",
	     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
	     tm->tm_min, tm->tm_sec);

	tm = localtime(&stat.ctime);
	printf
	    ("Fecha y hora de la última modificacion del inodo (ctime): %d-%02d-%02d %02d:%02d:%02d\n",
	     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
	     tm->tm_min, tm->tm_sec);

	bumount();
	return 0;
}
