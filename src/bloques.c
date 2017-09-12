/**
 Autor: Maria Rodriguez, Daniel Alami
 CAMBIOS:
 23/06/2012	Maria Rodriguez    Actualizacion de primitivas para incluir semaforos
*/

/* Funciones basicas para for I/O blocks */
#include "../include/bloques.h"
#define DEFINE_SEMAFOROS

static int fd = 0;
static int fd2 = 0;
//static int fd3 = 0;
const char *camino_f;
//int sem, semAI, semSB;
//int inside_sc;

/* Abre o crea un archivo y devuelve su descriptor de archivo */
int bmount(const char *camino)
{

	sem = iniciarSem(1, 1);	//inicialización semaforo
	semSB = iniciarSem(1, 3);	//inicializacion semaforo superbloque
	semAI = iniciarSem(1, 4);
	semMB = iniciarSem(1, 5);
	camino_f = camino;
	if (sem < 0) {
		printf("Error!\n\n");
		exit(1);
	}
	inicializar_le();
	fd = open(camino, O_WRONLY | O_CREAT, 0666);
	fd2 = open(camino, O_RDONLY);
//      fd3 = open(camino,O_RDONLY);
	if (unlikely(fd < 0)) {
		printf("Error abriendo/creando fichero\n");
		return -1;
	}
	return fd;
}

/* Retorna 0 si todo va bien o -1 en caso contrario. */
int bumount()
{
	int ret;
	eliminarSem(sem);
	eliminarSem(semSB);
	eliminarSem(semAI);
	eliminarSem(semMB);
	eliminar_le();
	if (unlikely(ret = close(fd) < 0) || unlikely(ret = close(fd2) < 0)) {
		printf("Error cerrando fichero\n");
		return -1;
	}
	return ret;
}

/* Retorna 0 si todo va bien o -1 en caso contrario. */
int bwrite(unsigned int bloque, const void *buf)
{

	/* Primero nos posicionamos en el lugar adecuado */
	if (likely(lseek(fd, BLOCKSIZE * bloque, SEEK_SET) >= 0)) {
		//Si estamos aqui no hubo un error al mover el puntero
		if (unlikely(write(fd, buf, BLOCKSIZE) == -1)) {
			printf("Hubo un error al escribir en el bloque indicado\n");
			return -1;
		}
		return 0;
	}
	return -1;
}

int bread(unsigned int bloque, void *buf)
{
	/* Primero nos posicionamos en el lugar adecuado */
	if (likely(lseek(fd2, BLOCKSIZE * bloque, SEEK_SET) >= 0)) {
		//Si estamos aqui no hubo un error al mover el puntero
		if (unlikely(read(fd2, buf, BLOCKSIZE) == -1)) {
			printf("Huo un error al leer del bloque indicado\n");
			return -1;
		}
		return 0;
	}
	return -1;
}

void duplicaDescriptor()
{
	insideCero();
	close(fd);
	fd = open(camino_f, O_WRONLY);
	fd2 = open(camino_f, O_RDONLY);
}

void mataDescriptor()
{
	close(fd);
	close(fd2);
}
