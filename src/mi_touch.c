/* programa que crea principalmente modifica metainformacion o puede ser usado para crear ficheros : mi_touch*/

#include "../include/directorios.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf
		    ("Numero de argumentos incorrecto!\n Uso de la función: ./mi_touch disco /RUTA opciones.\n");
		return -1;
	}
	char *j = (argv[2] + strlen(argv[2]) - 1);
	if (*j == '/') {
		printf("La ruta tiene que ser un fichero! (termina en /)\n");
		return -1;
	}
	int flags[4] = { 0 };
	char *cvalue = NULL;
	int index;
	int c;
	time_t ntime;
	opterr = 0;

	while ((c = getopt(argc, argv, "acdm:")) != -1)
		switch (c) {
		case 'a':
			flags[0] = 1;
			break;
		case 'c':
			flags[1] = 1;
			break;
		case 'm':
			flags[2] = 1;
			break;
		case 'd':
			ntime = getdate(optarg, NULL);
			if (ntime == (time_t) - 1) {
				printf("Formato de fecha invalido\n", optarg);
				return -1;
			}
			// Informacion para imprimir por pantalla
			struct tm *ts;
			char ttime[80];
			ts = localtime(&ntime);
			strftime(ttime, sizeof(ttime), "%a %Y-%m-%d %H:%M:%S",
				 ts);
			printf("ttime: %s", ttime);	//check
			flags[3] = 1;
			bmount(argv[1]);
			mi_touch(argv[2], flags, ntime);
			bumount();
			return 0;
			break;
		case '?':
			if (optopt == 'd')
				fprintf(stderr,
					"La opcion -%c requiere un argumento.\n",
					optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Opcion desconocida `-%c'.\n",
					optopt);
			else
				fprintf(stderr,
					"Unknown option chracter `\\x%x'.\n",
					optopt);
			return 1;
		default:
			abort();
		}
	bmount(argv[1]);
	mi_touch(argv[2], flags, NULL);
	bumount();
	return 0;
}
