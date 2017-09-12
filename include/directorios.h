#include "ficheros.h"
#include "semaforo.h"

struct entrada {
	char nombre[60];
	unsigned int inodo;
};

int extraer_camino(const char *camino, char *inicial, char *final);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir,
		   unsigned int *p_inodo, unsigned int *p_entrada,
		   char reservar, unsigned char modo);
int mi_creat(const char *camino, unsigned char modo);
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);
int mi_dir(const char *camino, char *buffer);
int mi_chmod(const char *camino, unsigned char modo);
int mi_stat(const char *camino, struct STAT *p_stat);
int mi_read(const char *camino, void *buf, unsigned int offset,
	    unsigned int nbytes);
int mi_write(const char *camino, const void *buf, unsigned int offset,
	     unsigned int nbytes);
