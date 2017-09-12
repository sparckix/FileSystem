#include "../include/simulacion.h"
#include <sys/wait.h>
#define NUM_PROCESOS 100
#define NUM_OPERACIONES 50
#define MAX 10000
int buscarDivisor(int i);
int notIn(unsigned int buscar, unsigned int *puntero, unsigned int cantidad);
int procesos_acabados = 0;
char dirSimulacion[128];	//directorio de la simulacion (/simul_aaaammddhhmmss/)

//unsigned int posMaxima = (16843019*BLOCKSIZE)/sizeof(struct registro);
unsigned int posMaxima = 5000;

//funcion para tratar procesos zombies. Llama wait3() para que se encargue de la gestion cuando un proceso hijo acaba
//WHOHANG -> wait3() no bloqueante.
void enterrador()
{
	while (wait3(NULL, WNOHANG, NULL) > 0) {
		procesos_acabados++;
	}
	printf("Procesos acabados: %d\n", procesos_acabados);
}

void crearDirSimulacion(char *dir)
{
	struct tm *tm;
	time_t tactual = time(NULL);
	tm = localtime(&tactual);
	sprintf(dir, "/simul_%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900,
		tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
		tm->tm_sec);
	printf("Intentando crear camino %s\n", dir);
	mi_creat(dir, 7);
}

void procesoHijo()
{
	char rutaProceso[256];	//buffer donde se guarda directorio de la simulacion + proceso_n/prueba.dat donde n es el pid del proceso
	memset(rutaProceso, '\0', strlen(rutaProceso));
	sprintf(rutaProceso, "%sproceso_%d/", dirSimulacion, getpid());
//      printf("ruta intermedia: %s\n", rutaProceso);
	if (mi_creat(rutaProceso, 7) < 0) {
//              printf("Error al crear ruta.\n");
		return;
	}			//creamos la ruta intermedia
	sprintf(rutaProceso, "%sproceso_%d/prueba.dat", dirSimulacion, getpid());	//creamos la ruta final (dat)
//      printf("ruta final: %s\n", rutaProceso);
	if (mi_creat(rutaProceso, 7) < 0) {
//              printf("Error al crear ruta.\n");
		return;
	}

	srand(time(NULL) + getpid());	//actualizamos semilla de los numeros aleatorios.

	struct registro registro;
	int i;
	for (i = 0; i < NUM_OPERACIONES; i++) {
		//inicializamos el registro
		registro.fecha = time(NULL);
		registro.pid = getpid();
		registro.nEscritura = i + 1;
		registro.posicion = rand() % posMaxima;
		if (mi_write(rutaProceso, &registro,
			     registro.posicion * sizeof(struct registro),
			     sizeof(struct registro)) < 0)
			return;

		usleep(50000);	//espera de 0,05 segundos entre una operacion y otra.
	}
}

void verificador()
{
	unsigned int pid;
	unsigned int numEscrituras;	//numero de registros validos del fichero prueba.dat (si el pid coincide)
	struct info primeraEscritura;	//menor fecha y hora y el numero de escritura en que ocurrio y posicion
	struct info ultimaEscritura;	//mayor fecha y hora y el numero de escritura en que ocurrio y posicion
	struct info menorPosicion;	//posicion mas baja, numero de escritura en que ocurrio y su fecha y hora
	struct info mayorPosicion;	//posicion mas alta, numero de escritura en que ocurrio y su fecha y hora

	struct STAT stat;
	mi_stat(dirSimulacion, &stat);
	unsigned int numEntradas = stat.tamEnBytesLog / sizeof(struct entrada);	//numero de entradas dentro del dirSimulacion
	unsigned int numEntrada;
	unsigned int numRegistro;
	struct registro registro;
	char *p;
	char rutaPrueba[256];	//ruta del fichero prueba.dat de cada proceso
	char rutaInforme[256];	//ruta del informe de la verificacion
	char buffer[2000];
	char bufferTemp[2000];
	struct tm *tm;

	memset(rutaPrueba, '\0', strlen(rutaPrueba));
	memset(rutaInforme, '\0', strlen(rutaInforme));
	memset(buffer, '\0', strlen(buffer));
	memset(bufferTemp, '\0', strlen(bufferTemp));
	struct entrada entrada;

	//creamos fichero informe
	strcpy(rutaInforme, dirSimulacion);
	strcat(rutaInforme, "informe.txt");
	mi_creat(rutaInforme, 7);

	for (numEntrada = 0; numEntrada < numEntradas; numEntrada++) {
		mi_read(dirSimulacion, &entrada,
			numEntrada * sizeof(struct entrada),
			sizeof(struct entrada));
		sprintf(rutaPrueba, "%s%s/prueba.dat", dirSimulacion,
			entrada.nombre);
		p = strchr(entrada.nombre, '_');	//buscamos '_' para extraer el pid del directorio proceso_pid
		pid = atoi(p + 1);	//saltamos '_' y obtenenemos pid
//              printf("Tam en byte antes %d\n",stat.tamEnBytesLog/sizeof(struct entrada));
		mi_stat(rutaPrueba, &stat);	//tamaño de prueba.dat
//                printf("Tam en byte despues %d\n",stat.tamEnBytesLog/sizeof(struct registro));

		numEscrituras = 0;
		//buscamos primer registro valido para inicializar las estructuras
		numRegistro = 0;
		registro.pid = 0;
		mi_read(rutaPrueba, &registro,
			numRegistro * sizeof(struct registro),
			sizeof(struct registro));
		while (registro.pid != pid) {
			numRegistro++;
			mi_read(rutaPrueba, &registro,
				numRegistro * sizeof(struct registro),
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
//                unsigned int *ptr;
//              ptr = (unsigned int *) malloc(NUM_OPERACIONES*sizeof(unsigned int)); //MAXIMO
//                if (NULL == ptr) return;
		for (numRegistro; numRegistro < totales; numRegistro = numRegistro + pl) {
			iteraciones++;
//                      registro.pid = 0;
			int it;
			leidos =
			    mi_read(rutaPrueba, registro_array,
				    numRegistro * sizeof(struct registro),
				    sizeof registro_array);
			for (it = 0; it < MAX; it++) {
				if (registro_array[it].pid == pid /* && !notIn(registro_array[it].nEscritura,ptr,count) */ ) {	//miramos que la lectura sea valida
//                                      printf("leidos: %d\n",leidos);
//                                      ptr[count] = registro_array[it].nEscritura;
//                                      count++;
//                                      int *m_ptr = realloc(ptr,count*sizeof(int));
//                                      if (m_ptr == NULL) return;
//                                      ptr = m_ptr;
//                                      printf("coinciden %d %d escritura %d posicion %d iteraciones %d valor it %d registro! %d offset %d maximo %d \n",registro_array[it].pid,pid,registro_array[it].nEscritura,registro_array[it].posicion,iteraciones,it,numRegistro,numRegistro*sizeof(struct registro),numRegistro*sizeof(struct registro) + sizeof registro_array-16);
					numEscrituras++;
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
				if ((numRegistro + it) == totales - 1) {
					incr = MAX;
					pl = totales;
				}
			}
		}

		sprintf(bufferTemp, "\nNumero proceso: %d\n", numEntrada);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		sprintf(bufferTemp, "PID: %d\n", pid);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		sprintf(bufferTemp, "Numero escrituras correctas: %d\n\n", numEscrituras);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		tm = localtime(&primeraEscritura.fecha);
		sprintf(bufferTemp,
			"\tPrimera escritura ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			primeraEscritura.nEscritura, primeraEscritura.posicion);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		tm = localtime(&ultimaEscritura.fecha);
		sprintf(bufferTemp,
			"\tUltima escritura  ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			ultimaEscritura.nEscritura, ultimaEscritura.posicion);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		tm = localtime(&menorPosicion.fecha);
		sprintf(bufferTemp,
			"\tMenor posicion    ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			menorPosicion.nEscritura, menorPosicion.posicion);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		tm = localtime(&mayorPosicion.fecha);
		sprintf(bufferTemp,
			"\tMayor posicion    ->\t fecha: %d-%02d-%02d %02d:%02d:%02d \tnEscritura: %d \tposicion: %d\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			mayorPosicion.nEscritura, mayorPosicion.posicion);
		strcat(buffer, bufferTemp);
		memset(bufferTemp, '\0', strlen(bufferTemp));

		mi_stat(rutaInforme, &stat);
		mi_write(rutaInforme, buffer, stat.tamEnBytesLog, strlen(buffer));	
		printf("%s\n", buffer);	//mostramos informe por pantalla
		memset(buffer, '\0', strlen(buffer));
//              free(ptr);
	}
}

int main(int argc, char **argv)
{

	if (argc != 2) {
		printf
		    ("Argumentos de la llamada incorrectos!\nModo de empleo: ./simulacion disco\n");
		return -1;
	}
	bmount(argv[1]);

	memset(dirSimulacion, '\0', strlen(dirSimulacion));
	crearDirSimulacion(dirSimulacion);
	signal(SIGCHLD, enterrador);	//informamos al SO que ejecute enterrador() cuando un proceso hijo termina y pasa a estado zombie.
	int i;
	for (i = 0; i < NUM_PROCESOS; i++) {	
		switch (fork()) {
		case 0:
//                      bmount();
			duplicaDescriptor();
			procesoHijo();
			mataDescriptor();
			exit(0);
		default:
			NULL;
		}
		usleep(200000);	//espera entre un proceso y otro.
	}
	while (procesos_acabados < NUM_PROCESOS) {
		pause();
	}
//      clock_t init_time = clock();
//      verificador();
//      printf("Tiempo transcurrido: %f\n",(((double)clock()- init_time)/ CLOCKS_PER_SEC));
	bumount();
}

/* Devuelvo 1 si está, 0 de lo contrario */
int notIn(unsigned int buscar, unsigned int *puntero, unsigned int cantidad)
{
	int i = 0;
	for (i; i < cantidad; i++) {
		if (puntero[i] == buscar) {
			return 1;
		}
	}
	return 0;
}
