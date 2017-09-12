/*
 * Intentemos mejorar el rendimiento general con estas directivas ya incorporadas
 * El compilador deberia optimizar apropiadamente el salto.
 */
#ifdef __GNUC__
#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)
#else
//si no esta definido, mantenemos las funciones
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>		/* open() function and open modes (on System V UNIX systems) */
#include <stdlib.h>		/* write() y close() functions    */
#include <string.h>
#include "semaforo.h"
#include "lectores_escritores.h"

#define BLOCKSIZE 1024		//definido en bytes
#ifdef DEFINE_SEMAFOROS
#define EXTERN			/* nada */
#else
#define EXTERN extern
#endif
EXTERN int sem, semBloque, semSB, semAI, semMB;

int bmount(const char *camino);
int bumount();
int bwrite(unsigned int bloque, const void *buf);
int bread(unsigned int bloque, void *buf);
void mi_waitSem();
void mi_signalSem();
void duplicaDescriptor();
void mataDescriptor();
