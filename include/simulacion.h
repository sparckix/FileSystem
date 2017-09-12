#include "directorios.h"

struct registro {
	time_t fecha;		// Entero (time_t) con la fecha y hora en forma epoch
	unsigned int pid;	// Entero con el PID del proceso que lo creó
	unsigned int nEscritura;	// Entero con el número de escritura (de 1 a 50)
	unsigned int posicion;	// Entero con la posición del registro dentro del fichero (número de registro)
};

struct info {
	time_t fecha;
	unsigned int nEscritura;
	unsigned int posicion;
};
