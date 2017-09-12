
#include <string.h>
#include <stdio.h>
#include "../include/bloques.h"
#include "../include/ficheros_basico.h"
void leerSB();
void leerMB();
void leerAI();
int main(int argc, char **argv)
{
	if (argc != 2) {
		printf
		    ("Número de argumentos erróneo en leer_sf.c\nUso: ./leer_sf <archivo>\n");
		return -1;
	}
	bmount(argv[1]);
	leerSB();
// leerMB();
	leerAI();
	bumount();
}

void leerSB()
{
	struct superbloque sb;
	bread(posSB, &sb);
	printf("Información del superBloque\n");
	printf("Número del primer bloque del mapa de bits: %d\n",
	       sb.posPrimerBloqueMB);
	printf("Número del último bloque del mapa de bits: %d\n",
	       sb.posUltimoBloqueMB);
	printf("Número del primer bloque del array de inodos: %d\n",
	       sb.posPrimerBloqueAI);
	printf("Número del último bloque del array de inodos: %d\n",
	       sb.posUltimoBloqueAI);
	printf("Número del primer bloque de datos: %d\n",
	       sb.posPrimerBloqueDatos);
	printf("Número del último bloque de datos: %d\n",
	       sb.posUltimoBloqueDatos);
	printf("Número del inodo del directorio raíz: %d\n", sb.posInodoRaiz);
	printf("Número del primer inodo libre: %d\n", sb.posPrimerInodoLibre);
	printf("Número de bloques libres: %d\n", sb.cantBloquesLibres);
	printf("Número de inodos libres: %d\n", sb.cantInodosLibres);
	printf("Cantidad total de bloques: %d\n", sb.totBloques);
	printf("Cantidad total de inodos: %d\n", sb.totInodos);
}

void leerMB()
{
	struct superbloque sb;
	bread(posSB, &sb);
	int i;
	unsigned char resultado;
	printf("Información del mapa de bits\n");
	for (i = 1; i <= sb.totBloques; i++) {
		resultado = leer_bit(i);
		if (resultado == 0)
			printf("El bloque %d está libre\n", i);
		else
			printf("El bloque %d está ocupado\n", i);
	}
}

void leerAI()
{
	struct superbloque sb;
	bread(posSB, &sb);
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	struct inodo inodo;
	int in;
	printf("Información del array de inodos\n");
	for (in = 0; in < sb.totInodos; in++) {
		inodo = leer_inodo(in);
		if (inodo.tipo == 'f') {
			ts = localtime(&inodo.atime);
			strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S",
				 ts);
			ts = localtime(&inodo.mtime);
			strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S",
				 ts);
			ts = localtime(&inodo.ctime);
			strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S",
				 ts);
			printf
			    ("ID: %d SIGUIENTE: %d ATIME: %s MTIME: %s CTIME: %s\n",
			     in, inodo.punterosDirectos[0], atime, mtime,
			     ctime);
		}
	}
}
