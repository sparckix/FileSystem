#include "../include/directorios.h"
#include <stdio.h>


int extraer_camino(const char *camino, char *inicial, char *final)
{
	int i, longitud, cont;
	if (camino[0] != '/' || camino[1] == '\0')	//No es un camino válido.
		return -1;
	for (i = 1; camino[i] != '/' && camino[i] != '\0'; i++)
		inicial[i - 1] = camino[i];
	inicial[i - 1] = '\0';
	(void)strcpy(final, camino + i);
	if (camino[i] == '/') {
		return 1;	//directorio
	} else {
		return 0;	//fichero
	}
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir,
		   unsigned int *p_inodo, unsigned int *p_entrada,
		   char reservar, unsigned char modo)
{
	char inicial[60];	//maximo permitido
	char final[500];	//tamaño maximo de ruta
	struct inodo in;
	struct entrada entrada;
	int activado = 0;
	int tipo;
	unsigned int num_entradas, nentrada;
//ESC   entrada_escritores();
	entrada_lectores();
	if (strcmp(camino_parcial, "/") == 0) {	//se trata del directorio raiz?
		*p_inodo = 0;
		*p_entrada = 0;
		return 0;
	}

	memset(inicial, '\0', 60);
	memset(final, '\0', 500);
	tipo = extraer_camino(camino_parcial, inicial, final);	//Separa inicial y final
	if (tipo == -1) {
		salida_lectores();
//ESC           salida_escritores();
		return -1;
	}
	in = leer_inodo(*p_inodo_dir);	//Lee el directorio que lo contiene para leer su entrada
	memset(entrada.nombre, '\0', strlen(entrada.nombre));
	num_entradas = in.tamEnBytesLog / sizeof(struct entrada);	//numero de entradas del inodo
//      printf("NUM_Entradas: %d del directorio %d tam %d permisos %d inicial %s final %s pid %d\n",num_entradas,*p_inodo_dir,in.tamEnBytesLog, in.permisos,inicial, final, getpid());
	nentrada = 0;
	if ( /*num_entradas > 0 */ in.tamEnBytesLog != 0) {	// si es 0 no tiene entradas que se puedan leer
		if (mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)) == -1) {	//Lee la primera entrada
//                      printf("No tiene permisos de lectura %d\n",getpid());
			salida_lectores();
//ESC                   salida_escritores();
			return -2;	//No hay permisos de lectura
		}
		while ((nentrada <= num_entradas - 1) && (strcmp(inicial, entrada.nombre) != 0)) {	//recorre las entradas hasta que acabe o encuentre el nombre
			nentrada++;
			if (mi_read_f
			    (*p_inodo_dir, &entrada,
			     nentrada * sizeof(struct entrada),
			     sizeof(struct entrada)) < 0) {
				salida_lectores();
				return -2;
			}
		}
	}
	if (nentrada == num_entradas) {	//la entrada que buscabamos no existe
		switch (reservar) {
		case 0:	//modo consulta
			salida_lectores();
//ESC                   salida_escritores();
			return -4;	//no existe
		case 1:	//modo escritura. se crea la entrada de directorio en el directorio especificado
			//entradda escri
			salida_lectores();
			entrada_escritores();
			strcpy(entrada.nombre, inicial);	//copiar nombre en la entrada y reservar un inodo
			//revisar que no haya cambiado tamenbyteslog
			in = leer_inodo(*p_inodo_dir);
			nentrada = in.tamEnBytesLog / sizeof(struct entrada);	//numero de entradas del inodo

			if (tipo == 1) {	//es un directorio
				if (strcmp(final, "/") == 0) {	//si el resultado vale cero final=/ y se trata de un directorio simple.
					waitSem(sem);
					entrada.inodo = reservar_inodo('d', modo);	//inodo como directorio
					signalSem(sem);
				} else {
					//entrada.inodo = reservar_inodo('d',7); //directorio intermedio (no!)
					printf
					    ("Creacion de directorio intermedio deshabilitada. inicial %s final: %s, num_entradas %d del inodo %d\n",
					     inicial, final, num_entradas,
					     *p_inodo_dir);
					salida_escritores();
					return -7;
				}
			} else {
				waitSem(sem);
				entrada.inodo = reservar_inodo('f', modo);	//inodo como fichero
				signalSem(sem);
			}
			if (mi_write_f
			    (*p_inodo_dir, &entrada,
			     nentrada * sizeof(struct entrada),
			     sizeof(struct entrada), 1) == -1) {
				if (entrada.inodo != -1)
					liberar_inodo(entrada.inodo);
				printf("no tiene permisos de escritura %s %s\n",
				       inicial, final);
//                              signalSem(sem);
				salida_escritores();
				return -3;	//el directorio al que apunta p_inodo_dir no tiene permisos (de escritura).
			}
			salida_escritores();
			break;
		}
	} else {
		activado = 1;
	}
	if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) {
		//es el final, devolvemos...
		if ((nentrada < num_entradas) && (reservar == 1)) {
//ESC                   salida_escritores();
			if (activado)
				salida_lectores();
			return -5;	//modo escritura y ya existe
		}
		if (activado)
			salida_lectores();
		*p_inodo = entrada.inodo;
		*p_entrada = nentrada;
//ESC           salida_escritores();
		return 0;
	} else {		//no es el final: extrae de nuevo el camino
//ESC           salida_escritores();
		if (activado)
			salida_lectores();
		*p_inodo_dir = entrada.inodo;
		return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada,
				      reservar, modo);
	}
}

int mi_creat(const char *camino, unsigned char modo)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;
	resultadoBe = buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 1, modo);	//modo escritura
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido\n");
			break;
		case -2:
			printf("La entrada no tiene permisos de lectura\n");
			break;
		case -3:
			printf
			    ("Algun directorio intermedio no tiene permiso de escritura a y no se pueden escribir entradas\n");
			break;
		case -5:
			printf("El directorio/fichero ya existe\n");
			break;
		case -7:
			printf
			    ("Creacion de directorio intermedio deshabilitada\n");
			break;
		}
		return -1;
	}
//      printf("La entrada %s se creo correctamente en inodo %d\n", camino,p_inod);
	return 0;
}

int mi_dir(const char *camino, char *buffer)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe [mi_dir]\n", camino);
			break;
		}
		return -1;
	} else {
		struct inodo inodo = leer_inodo(p_inod);
		if ((inodo.tipo == 'd') && (inodo.permisos & 4)) {	//directorio con permisos de lectura?
			struct inodo inodo = leer_inodo(p_inod);
			struct entrada entrada;
			unsigned int num_entradas, it;
			struct tm *tm;
			char tmp[100];
			unsigned char padding[30];

			num_entradas =
			    inodo.tamEnBytesLog / sizeof(struct entrada);
			for (it = 0; it < num_entradas; it++) {
				mi_read_f(p_inod, &entrada,
					  it * sizeof(struct entrada),
					  sizeof(struct entrada));
				inodo = leer_inodo(entrada.inodo);
				sprintf(padding, "%c", inodo.tipo);
				strcat(buffer, padding);

				//Información de permisos
				if (inodo.permisos & 4)
					strcat(buffer, "r");
				else
					strcat(buffer, "-");
				if (inodo.permisos & 2)
					strcat(buffer, "w");
				else
					strcat(buffer, "-");
				if (inodo.permisos & 1)
					strcat(buffer, "x");
				else
					strcat(buffer, "-");
				//Informacón de tamaño, nlinks inodo...
				sprintf(padding, "   %d  %d  %d  ",
					entrada.inodo, inodo.tamEnBytesLog,
					inodo.nlinks);
				strcat(buffer, padding);

				//Información de tiempo
				tm = localtime(&inodo.mtime);
				sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
					tm->tm_year + 1900, tm->tm_mon + 1,
					tm->tm_mday, tm->tm_hour, tm->tm_min,
					tm->tm_sec);
				strcat(buffer, tmp);
				strcat(buffer, entrada.nombre);
				strcat(buffer, "|");
			}
			return num_entradas;
		} else if (inodo.tipo != 'd') {
			printf("La entrada %s no es un directorio", camino);
			return -3;
		} else if (inodo.permisos & 4) {
			printf
			    ("El directorio %s parece no tener permisos de lectura",
			     camino);
			return -2;
		}
	}
	return -1;		//no se alcanza
}

int mi_chmod(const char *camino, unsigned char modo)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe [mi_chmod]\n", camino);
			break;
		}
		return -1;
	} else {
		mi_chmod_f(p_inod, modo);
		char bufferTemporal[3];
		memset(bufferTemporal, '\0', strlen(bufferTemporal));
		if (modo & 4)
			strcat(bufferTemporal, "r");
		else if (modo & 2)
			strcat(bufferTemporal, "w");
		else if (modo & 1)
			strcat(bufferTemporal, "x");
		else
			strcat(bufferTemporal, "-");
		printf("Los permisos de %s se cambiaron a: %s !\n", camino,
		       bufferTemporal);
	}
	return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

	resultadoBe
	    = buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);

	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido\n");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas\n");
			break;
		case -4:
			printf("La entrada %s no existe [mi_stat]\n", camino);
			break;
		}
		return -1;
	} else {
		mi_stat_f(p_inod, p_stat);
	}
	return 0;
}

int mi_read(const char *camino, void *buf, unsigned int offset,
	    unsigned int nbytes)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;
	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe [mi_read]\n", camino);
			break;
		}
		return -1;
	} else {
		int leidos = mi_read_f(p_inod, buf, offset, nbytes);
		if (leidos == -1) {
			printf
			    ("La entrada %s parece no tener permisos de lectura",
			     camino);
			return -1;
		} else
			return leidos;
	}
}

int mi_write(const char *camino, const void *buf, unsigned int offset,
	     unsigned int nbytes)
{
	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;
	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe [mi_write] %d\n",
			       camino, getpid());
			break;
		}
		return -1;
	} else {
		int escritos = mi_write_f(p_inod, buf, offset, nbytes, 1);
		if (escritos == -1) {
			printf
			    ("La entrada %s no parece tener permisos de escritura!\n",
			     camino);
			return -1;
		} else
			return escritos;
	}
}

int mi_link(const char *camino1, const char *camino2)
{
	//int sem;
	//obtenerSem(&sem);

	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

	resultadoBe =
	    buscar_entrada(camino1, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);

	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe [mi_link]\n", camino1);
			break;
		}
		return -1;
	}

	int ninodo = p_inod;	//numero inodo asociado a camino1
	p_inodo_dir = 0, p_inod = 0, p_entrada = 0;

	//creacion del enlace de camino2 al inodo de camino 1. precondicion->camino2 no puede existir
	resultadoBe = buscar_entrada(camino2, &p_inodo_dir, &p_inod, &p_entrada, 1, 0);	//en modo escritura. si existe->error

	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -3:
			printf
			    ("Algun directorio intermedio no tiene permiso de escritura a y no se pueden escribir entradas");
			break;
		case -5:
			printf("El directorio/fichero ya existe");
			break;
		}
		return -1;
	} else {
		struct entrada entrada;
		struct inodo inodo;
		//Leer entrada creada
		if (mi_read_f
		    (p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada),
		     sizeof(struct entrada)) == -1) {
//                      signalSem(sem);
			return -1;
		}
		liberar_inodo(entrada.inodo);	//liberamos el inodo que se ha asociado a la entrada creada
		entrada.inodo = ninodo;	//asociamos a esta entrada el mismo inodo que el que esta asociado a la entrada camino1
		//Escribir entrada modificada
		if (mi_write_f
		    (p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada),
		     sizeof(struct entrada), 1) == -1) {
//                      signalSem(sem);
			return -1;
		} else {
			//Leemos el inodo e incrementaos la cantidad de enlaces de entrada a directorio
			inodo = leer_inodo(ninodo);
			inodo.nlinks++;
			inodo.ctime = time(NULL);
			escribir_inodo(inodo, ninodo);
			printf("El enlace %s se ha creado con exito", camino2);
		}
	}
	return 0;
}

int mi_unlink(const char *camino)
{
	//int sem;
	//obtenerSem(&sem);

	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

//      waitSem(sem);
	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);

	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf
			    ("Algun directorio intermedio no tiene permiso de lectura y no se pueden leer las entradas");
			break;
		case -4:
			printf("La entrada %s no existe  [mi_unlink]\n",
			       camino);
			break;
		}
//              signalSem(sem);
		return -1;
	} else {
		waitSem(sem);
		//llegados a este punto la entrada existe
		struct inodo inodo;

		inodo = leer_inodo(p_inodo_dir);	//leemos el inodo asociado al directorio (que contiene la entrada que queremos eliminar)
		unsigned int tbytes =
		    inodo.tamEnBytesLog - sizeof(struct entrada);
		unsigned int nentradas =
		    inodo.tamEnBytesLog / sizeof(struct entrada);
		if (p_entrada == nentradas - 1) {	//ultima entrada. Basta con truncar el inodo a su tamaño menos una entrada
			if (mi_truncar_f(p_inodo_dir, tbytes) == -1) {
				printf
				    ("El directorio que contiene la entrada no tiene permisos de escritura");
				signalSem(sem);
				return -1;
			}
		} else {
			//leemos la ultima entrada y la colocamos en el lugar de la entrada a eliminar. Luego, podemos truncar
			struct entrada entrada;
			mi_read_f(p_inodo_dir, &entrada, (nentradas - 1) * sizeof(struct entrada), sizeof(struct entrada));	//leemos la ultima entrada
			if (mi_write_f
			    (p_inodo_dir, &entrada,
			     p_entrada * sizeof(struct entrada),
			     sizeof(struct entrada), 1) == -1) {
				printf
				    ("El directorio que contiene la entrada no tiene permisos de escritura");
				signalSem(sem);
				return -1;
			}
			mi_read_f(p_inodo_dir, &entrada,
				  p_entrada * sizeof(struct entrada),
				  sizeof(struct entrada));
			mi_truncar_f(p_inodo_dir,
				     inodo.tamEnBytesLog -
				     sizeof(struct entrada));
		}
		//leemos el inodo asociado a la entrada (eliminada) y decrementamos nlinks. si es 0, liberamos el inodo.
		inodo = leer_inodo(p_inod);
		inodo.nlinks--;
		if (inodo.nlinks == 0) {
			liberar_inodo(p_inod);
		} else {
			inodo.ctime = time(NULL);
			escribir_inodo(inodo, p_inod);
		}
	}
	signalSem(sem);
	printf("La entrada %s se ha eliminado correctamente\n", camino);
	return 0;
}

int mi_touch(const char *camino, int flag[], time_t ntime)
{

	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;
	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf("La entrada no tiene permisos de lectura");
			break;
		case -3:
			printf
			    ("El directorio al que se apunta no tiene permisos de escritura");
			break;
		case -4:	//si no existe se crea el archivo.
			if (!flag[1])
				if (mi_creat(camino, 7) == 0)
					return 0;
			break;
		}
		return -1;
	} else {
		//si existe, hacemos el tratamiento de flags.
		struct inodo inodo = leer_inodo(p_inod);
		if (flag[0])
			inodo.atime = time(NULL);
		else if (flag[2])
			inodo.mtime = time(NULL);
		else {
			if (!flag[3]) {
				inodo.atime = time(NULL);
				inodo.mtime = time(NULL);
			} else {
				inodo.atime = ntime;	//TODO
				inodo.mtime = ntime;	//TODO
			}
		}
		escribir_inodo(inodo, p_inod);
		printf("Metainformación actualizada con exito!\n");
	}
	return 0;
}

int mi_rename(const char *camino, char nombre[60])
{

	unsigned int p_inodo_dir = 0, p_inod = 0, p_entrada = 0;
	int resultadoBe;

	resultadoBe =
	    buscar_entrada(camino, &p_inodo_dir, &p_inod, &p_entrada, 0, 0);
	if (resultadoBe < 0) {
		switch (resultadoBe) {
		case -1:
			printf("Tipo no válido");
			break;
		case -2:
			printf("La entrada no tiene permisos de lectura");
			break;
		case -3:
			printf
			    ("El directorio al que se apunta no tiene permisos de escritura");
			break;
		case -4:	//si no existe se crea el archivo.
			printf("La entrada %s no existe  [mi_rename]\n",
			       camino);
			break;
		}
		return -1;
	} else {
		//si existe, hacemos el renombramiento.
		struct entrada entrada;
		mi_read_f(p_inodo_dir, &entrada,
			  p_entrada * sizeof(struct entrada),
			  sizeof(struct entrada));
		strcpy(entrada.nombre, nombre);
		mi_write_f(p_inodo_dir, &entrada,
			   p_entrada * sizeof(struct entrada),
			   sizeof(struct entrada), 1);
		return 0;
	}
}
