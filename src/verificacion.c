#include "../include/simulacion.h"
#include <time.h>
#define MAX 300
//#define MAX 10000

void verificador(char *dirSimulacion)
{
	unsigned int pid;
	unsigned int nEscrituras;	//registros validos del fichero prueba.dat (si el pid coincide)
	struct info primeraEscritura;	//menor fecha y hora en forma epoch, el número de escritura en que ocurrió y su posición
	struct info ultimaEscritura;	//mayor fecha y hora en forma epoch, el número de escritura en que ocurrió y su posición
	struct info menorPosicion;	//posición más baja, el número de escritura en que ocurrió y su fecha y hora en forma epoch
	struct info mayorPosicion;	//posición más alta, el número de escritura en que ocurrió y su fecha y hora en forma epoch

	struct STAT stat;
	mi_stat(dirSimulacion, &stat);
	unsigned int nEntradas = stat.tamEnBytesLog / sizeof(struct entrada);	//numero de entradas que hay dentro del dirSimulacion. Por norma general 100.
	unsigned int nEntrada;
	unsigned int nRegistro;
	struct registro registro;
	char *p;
	char rutaPrueba[256];	//ruta donde se encuentra el prueba.dat de cada proceso
	char rutaInforme[256];	//ruta donde se creara el informe de la verificacion
	char buffer[2000];
	char bufferTmp[2000];
	struct tm *tm;

	memset(rutaPrueba, '\0', strlen(rutaPrueba));
	memset(rutaInforme, '\0', strlen(rutaInforme));
	memset(buffer, '\0', strlen(buffer));
	memset(bufferTmp, '\0', strlen(bufferTmp));
	struct entrada entrada;

	//creamos el fichero informe
	strcpy(rutaInforme, dirSimulacion);
	strcat(rutaInforme, "informe.txt");
	if (mi_creat(rutaInforme, 7) < 0)
		return;

/*	int maximo_asignado=10;
	int i;
	nEntrada = 0;

        for (i = 10; i < nEntradas; i=i+10) {
                switch (fork()) {
                case 0:
                        duplicaDescriptor();
                        nEntrada = i;
			maximo_asignado = nEntrada+i;
                default:
                        NULL;
                }
        }
*/
	for (nEntrada = 0; nEntrada < nEntradas; nEntrada++) {
		mi_read(dirSimulacion, &entrada,
			nEntrada * sizeof(struct entrada),
			sizeof(struct entrada));
		sprintf(rutaPrueba, "%s%s/prueba.dat", dirSimulacion,
			entrada.nombre);
		p = strchr(entrada.nombre, '_');	//buscamos el caracter '_' para extraer el pid del directorio proceso_pid
		pid = atoi(p + 1);	//p+1 para saltar el '_' y obtener el pid.
//              printf("Tam en byte antes %d\n",stat.tamEnBytesLog/sizeof(struct entrada));
		mi_stat(rutaPrueba, &stat);	//para saber el tamaño
//                printf("Tam en byte despues %d\n",stat.tamEnBytesLog/sizeof(struct registro));

		nEscrituras = 0;
		//buscamos el primer registro valido para inicializar las estructuras
		nRegistro = 0;
		registro.pid = 0;
		mi_read(rutaPrueba, &registro,
			nRegistro * sizeof(struct registro),
			sizeof(struct registro));
		while (registro.pid != pid) {
			nRegistro++;
			mi_read(rutaPrueba, &registro,
				nRegistro * sizeof(struct registro),
				sizeof(struct registro));
		}

		primeraEscritura.fecha = registro.fecha;
		primeraEscritura.nEscritura = registro.nEscritura;
		primeraEscritura.posicion = registro.posicion;

		ultimaEscritura.fecha = registro.fecha;
		ultimaEscritura.nEscritura = registro.nEscritura;
		ultimaEscritura.posicion = registro.posicion;

		menorPosicion.fecha = registro.fecha;
		menorPosicion.nEscritura = registro.nEscritura;
		menorPosicion.posicion = registro.posicion;

		mayorPosicion.fecha = registro.fecha;
		mayorPosicion.nEscritura = registro.nEscritura;
		mayorPosicion.posicion = registro.posicion;

		unsigned int pl = MAX;
		unsigned int totales =
		    stat.tamEnBytesLog / sizeof(struct registro);
		int iteraciones = 0;
		struct registro registro_array[MAX];
		int incr = 1;
		int leidos;
		int ultimo;
		unsigned int count = 0;
		for (nRegistro; nRegistro < totales; nRegistro = nRegistro + pl) {
			iteraciones++;
			int it;
			leidos =
			    mi_read(rutaPrueba, registro_array,
				    nRegistro * sizeof(struct registro),
				    sizeof registro_array);
			for (it = 0; it < MAX; it++) {
				if (registro_array[it].pid == pid /* && !notIn(registro_array[it].nEscritura,ptr,count) */ ) {	//miramos que la lectura sea valida
					nEscrituras++;
					if (registro_array[it].fecha <
					    primeraEscritura.fecha) {
						primeraEscritura.fecha =
						    registro_array[it].fecha;
						primeraEscritura.nEscritura =
						    registro_array[it].
						    nEscritura;
						primeraEscritura.posicion =
						    registro_array[it].posicion;
					}
					if (registro_array[it].fecha >
					    ultimaEscritura.fecha) {
						ultimaEscritura.fecha =
						    registro_array[it].fecha;
						ultimaEscritura.nEscritura =
						    registro_array[it].
						    nEscritura;
						ultimaEscritura.posicion =
						    registro_array[it].posicion;
					}

					if (registro_array[it].posicion <
					    menorPosicion.posicion) {
						menorPosicion.fecha =
						    registro_array[it].fecha;
						menorPosicion.nEscritura =
						    registro_array[it].
						    nEscritura;
						menorPosicion.posicion =
						    registro_array[it].posicion;
					}

					if (registro_array[it].posicion >
					    mayorPosicion.posicion) {
						mayorPosicion.fecha =
						    registro_array[it].fecha;
						mayorPosicion.nEscritura =
						    registro_array[it].
						    nEscritura;
						mayorPosicion.posicion =
						    registro_array[it].posicion;
					}

				}
				registro_array[it].pid = 0;
				registro_array[it].nEscritura = 0;
				if ((nRegistro + it) == totales - 1) {
					incr = MAX;
					pl = totales;
				}
			}
		}

		sprintf(bufferTmp, "\nNº proceso: %d\n", nEntrada);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		sprintf(bufferTmp, "PID: %d\n", pid);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		sprintf(bufferTmp, "Escrituras correctas: %d\n\n", nEscrituras);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		tm = localtime(&primeraEscritura.fecha);
		sprintf(bufferTmp,
			"\tPrimera escritura ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			primeraEscritura.nEscritura, primeraEscritura.posicion);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		tm = localtime(&ultimaEscritura.fecha);
		sprintf(bufferTmp,
			"\tUltima escritura  ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			ultimaEscritura.nEscritura, ultimaEscritura.posicion);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		tm = localtime(&menorPosicion.fecha);
		sprintf(bufferTmp,
			"\tMenor posicion    ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			menorPosicion.nEscritura, menorPosicion.posicion);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		tm = localtime(&mayorPosicion.fecha);
		sprintf(bufferTmp,
			"\tMayor posicion    ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			mayorPosicion.nEscritura, mayorPosicion.posicion);
		strcat(buffer, bufferTmp);
		memset(bufferTmp, '\0', strlen(bufferTmp));

		mi_stat(rutaInforme, &stat);
		mi_write(rutaInforme, buffer, stat.tamEnBytesLog, strlen(buffer));	//escribimos el informe en el sistema de ficheros
		printf("%s\n", buffer);	//y lo mostramos por pantalla
		memset(buffer, '\0', strlen(buffer));
	}
//      if(maximo_asignado != 10)  { mataDescriptor(); exit(0); }
}

int main(int argc, char **argv)
{

	if (argc != 3) {
		printf
		    ("Argumentos de la llamada erroneos!\nModo de empleo: ./verificacion disco directorio_simulacion\n");
		return -1;
	}
	bmount(argv[1]);
	clock_t init_time = clock();
	verificador(argv[2]);
	printf("Tiempo transcurrido: %f\n",
	       (((double)clock() - init_time) / CLOCKS_PER_SEC));
	bumount();
}
