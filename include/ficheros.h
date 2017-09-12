#include <time.h>
#include "ficheros_basico.h"
struct STAT {
	unsigned char tipo;	//Tipo (libre, directorio o fichero)
	unsigned char permisos;	//Permisos (lectura y/o escritura y/o ejecución)
	unsigned char reservado_alineacion[2];
	time_t atime;		//Fecha y hora del último acceso a datos: atime
	time_t mtime;		//Fecha y hora de la última modificación de datos: mtime
	time_t ctime;		//Fecha y hora de la última modificación del inodo: ctime
	unsigned int nlinks;	//Cantidad de enlaces de entradas en directorio
	unsigned int tamEnBytesLog;	//Tamaño en bytes lógicos
	unsigned int numBloquesOcupados;	//Cantidad de bloques ocupados en la zona de datos
};

int mi_write_f(unsigned int ninodo, const void *buf_original,
	       unsigned int offset, unsigned int nbytes, int tipo);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset,
	      unsigned int nbytes);
int mi_chmod_f(unsigned int ninodo, unsigned char modo);
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
