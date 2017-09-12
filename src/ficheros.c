#include "../include/ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original,
	       unsigned int offset, unsigned int nbytes, int tipo)
{
	unsigned int despPBloque = offset % BLOCKSIZE;	//desplazamiento primer bloque
	unsigned int despUBloque = (offset + nbytes - 1) % BLOCKSIZE;	//desplazamiento ultimo bloque
	unsigned int primerBLogico = offset / BLOCKSIZE;	//primer bloque logico donde escribiremos
	unsigned int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;	//ultimo bloque logico
	unsigned char bufbloque[BLOCKSIZE];
	unsigned int bfisico;
	unsigned int numBEscritos = 0;	//numero de bytes escritos (sera retornado).
	int posibleAct = 0;

	struct inodo in = leer_inodo(ninodo);

	if (!(in.permisos & 2)) {	//si no tiene permisos de escritura salimos
		printf("no tiene permisos de escritura\n");
		return -1;
	} else {
		if ((offset + nbytes >= in.tamEnBytesLog) || tipo == 1)	//si se verifica es muy probable que tengamos que reservar bloques o sea un directorio
			posibleAct = 1;
		if (nbytes > 0) {
			if (primerBLogico == ultimoBLogico) {	//si coinciden cabe en un bloque
				if (posibleAct == 1)
					waitSem(sem);
				traducir_bloque_inodo(ninodo, primerBLogico,
						      &bfisico, 1);
				bread(bfisico, bufbloque);
				memcpy(bufbloque + despPBloque, buf_original,
				       nbytes);
				bwrite(bfisico, bufbloque);
				numBEscritos = nbytes;
			} else {
				if (posibleAct == 1)
					waitSem(sem);
				//Tratamiento del primer bloque logico
				traducir_bloque_inodo(ninodo, primerBLogico,
						      &bfisico, 1);
				bread(bfisico, bufbloque);
				memcpy(bufbloque + despPBloque, buf_original,
				       BLOCKSIZE - despPBloque);
				bwrite(bfisico, bufbloque);
				numBEscritos = BLOCKSIZE - despPBloque;

				//Tratamiento del ultimo bloque logico
				traducir_bloque_inodo(ninodo, ultimoBLogico,
						      &bfisico, 1);
				bread(bfisico, bufbloque);
				memcpy(bufbloque,
				       buf_original + (BLOCKSIZE - despPBloque) +
				       (ultimoBLogico - primerBLogico -
					1) * BLOCKSIZE, despUBloque + 1);
				bwrite(bfisico, bufbloque);
				numBEscritos = numBEscritos + despUBloque + 1;
				
				//Tratamiento de bloques intermedios
				unsigned int i;
				for (i = primerBLogico + 1; i < ultimoBLogico;
				     i++) {
					traducir_bloque_inodo(ninodo, i,
							      &bfisico, 1);
					bwrite(bfisico,
					       buf_original + (BLOCKSIZE -
							       despPBloque) + (i -
									 primerBLogico
									 -
									 1) *
					       BLOCKSIZE);
					numBEscritos += BLOCKSIZE;
				}
			}
			//actualizamos de metainformacion y reescribimos el inodo.

			in = leer_inodo(ninodo);
			if (offset + numBEscritos /* nbytes */  > in.tamEnBytesLog)	//si ha cambiado actualizamos el nuevo tamaño
				in.tamEnBytesLog =
				    offset + /* nbytes */ numBEscritos;
			in.mtime = time(NULL);
			in.ctime = time(NULL);
			escribir_inodo(in, ninodo);
			if (posibleAct)
				signalSem(sem);

		}
		return numBEscritos;
	}
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset,
	      unsigned int nbytes)
{
	unsigned int Bleidos = 0;
	struct inodo in = leer_inodo(ninodo);	
	if (offset > in.tamEnBytesLog) {
		nbytes = 0;
		return nbytes;
	}
	if (offset + nbytes >= in.tamEnBytesLog)	//no leer mas alla del fin de fichero
		nbytes = in.tamEnBytesLog - offset;

	if (!(in.permisos & 4)) {	//no tiene permisos de lectura?
		return -1;
	} else {
		if (nbytes > 0) {
			unsigned int primerBLogico = offset / BLOCKSIZE;
			unsigned int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
			unsigned int despPBloque = offset % BLOCKSIZE;	//desplazamiento primer bloque
			unsigned int despUBloque = (offset + nbytes - 1) % BLOCKSIZE;	//desplazamiento ultimo bloque
			unsigned int bfisico;
			unsigned char bufbloque[BLOCKSIZE];
			memset(bufbloque, '\0', BLOCKSIZE);

			if (primerBLogico == ultimoBLogico) {
				//la lectura cabe en un bloque
				if (traducir_bloque_inodo(ninodo, primerBLogico,
							  &bfisico, 0) >= 0) {
					bread(bfisico, bufbloque);
					memcpy(buf_original, bufbloque + despPBloque,
					       nbytes);
					Bleidos = nbytes;
				}
			} else {
				//Tratamiento del primer bloque logico
				if (traducir_bloque_inodo(ninodo, primerBLogico,
							  &bfisico, 0) >= 0) {
					bread(bfisico, bufbloque);
					memcpy(buf_original, bufbloque + despPBloque,
					       BLOCKSIZE - despPBloque);
					Bleidos = BLOCKSIZE - despPBloque;
				}
				//Tratamiento del ultimo bloque logico
				if (traducir_bloque_inodo(ninodo, ultimoBLogico,
							  &bfisico, 0) >= 0) {
					bread(bfisico, bufbloque);
					memcpy(buf_original +
					       (BLOCKSIZE - despPBloque) +
					       (ultimoBLogico - primerBLogico -
						1) * BLOCKSIZE, bufbloque,
					       despUBloque + 1);
					Bleidos += despUBloque + 1;
				}
				//Tratamiento bloques intermedios
				unsigned int i;
				for (i = primerBLogico + 1; i < ultimoBLogico;
				     i++) {
					if (traducir_bloque_inodo(ninodo, i,
								  &bfisico,
								  0) >= 0) {
						bread(bfisico,
						      buf_original +
						      (BLOCKSIZE - despPBloque) + (i -
									     primerBLogico
									     -
									     1)
						      * BLOCKSIZE);
						Bleidos += BLOCKSIZE;
					}
				}
			}
			//actualizacion de metainformacion y reescritura de inodo para guardar los cambios
			in.atime = time(NULL);
			escribir_inodo(in, ninodo);
		}
		return Bleidos;
	}
}

int mi_chmod_f(unsigned int ninodo, unsigned char modo)
{
	struct inodo inodo = leer_inodo(ninodo);
	inodo.permisos = modo;
	inodo.ctime = time(NULL);
	escribir_inodo(inodo, ninodo);
	return 0;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
	struct inodo inodo = leer_inodo(ninodo);
	int primer_bloque, ultimo_bloque;
	int pbloc, dbloc;
	//Comprobamos permisos de escritura.
	if ((inodo.permisos & 2) == 2) { //tiene permisos de lectura?
		int i;
		//liberamos bloques;
		unsigned int nbloques;	//numero de bloques que quedaran ocupados despues de truncar. Es posible que haya bloques sin datos debido al offset)
		if (nbytes % BLOCKSIZE == 0)
			nbloques = nbytes / BLOCKSIZE;
		else
			nbloques = (nbytes / BLOCKSIZE) + 1;
		liberar_bloques_inodo(ninodo, nbloques);	//liberamos el resto de bloques
		inodo.mtime = time(NULL);
		inodo.ctime = time(NULL);
		inodo.tamEnBytesLog = nbytes;
		escribir_inodo(inodo, ninodo);
		return 0;
	} else
		return -1;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
	struct inodo inodo;
	inodo = leer_inodo(ninodo);

	p_stat->permisos = inodo.permisos;
	p_stat->tipo = inodo.tipo;
	p_stat->atime = inodo.atime;
	p_stat->mtime = inodo.mtime;
	p_stat->ctime = inodo.ctime;
	p_stat->nlinks = inodo.nlinks;
	p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
	p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
	return 0;
}
