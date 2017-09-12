#include "../include/ficheros_basico.h"

int liberar(unsigned int buffer[256], unsigned int nivel, struct inodo *in,
	    int *pbloque);

int tamMB(unsigned int nbloques)
{
	int tamMB;
	//Obtenemos numero de bloques para el mapa de bits
	if (((nbloques / 8) % BLOCKSIZE) == 0)
		tamMB = (nbloques / 8) / BLOCKSIZE;
	else
		tamMB = ((nbloques / 8) / BLOCKSIZE) + 1;
	return tamMB;
}

int tamAI(unsigned int ninodos)
{
	int tamAI;
	//Obtenemos numero de bloques para el array de inodos
	if (((ninodos * T_INODO) % BLOCKSIZE) == 0)
		tamAI = (ninodos * T_INODO) / BLOCKSIZE;
	else
		tamAI = ((ninodos * T_INODO) / BLOCKSIZE) + 1;
	return tamAI;
}

int initSB(unsigned int nbloques, unsigned int ninodos)
{
	struct superbloque sb;
	//sb = (struct superbloque *) malloc(sizeof(struct superbloque));
	unsigned int tamMB1, tamAI1;
	tamMB1 = tamMB(nbloques);
	tamAI1 = tamAI(ninodos);
	//El superbloque ocupa el primer bloque
	sb.posPrimerBloqueMB = posSB + BLOCKSIZE;
	sb.posUltimoBloqueMB = sb.posPrimerBloqueMB + tamMB1 - 1;
	sb.posPrimerBloqueAI = sb.posUltimoBloqueMB + 1;
	sb.posUltimoBloqueAI = sb.posPrimerBloqueAI + tamAI1 - 1;
	sb.posPrimerBloqueDatos = sb.posUltimoBloqueAI + 1;
	sb.posUltimoBloqueDatos = nbloques - 1;
	sb.posInodoRaiz = 0;
	sb.posPrimerInodoLibre = 0;
	sb.cantBloquesLibres = nbloques;
	sb.cantInodosLibres = ninodos;
	sb.totBloques = nbloques;
	sb.totInodos = ninodos;
	
	if (bwrite(posSB, &sb) == 0) {
		//free(sb);
		return 0;
	}
	//free(sb);
	return -1;
}

int initMB(unsigned int nbloques)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueMB;
	unsigned int pos_final = sb.posUltimoBloqueMB;
	unsigned char var[BLOCKSIZE];
	memset(var, 0, BLOCKSIZE);
	for (; pos_inicial <= pos_final; pos_inicial++) {
		if (bwrite(pos_inicial, var) != 0)
			return -1;
		escribir_bit(pos_inicial, 1);	//MB bloques ocupados
		sb.cantBloquesLibres--;
	}
	//Superbloque ocupado
	escribir_bit(posSB, 1);
	sb.cantBloquesLibres--;
	bwrite(posSB, &sb);
	return 0;
}

int initAI(unsigned int ninodos)
{
	struct superbloque sb;
// sb = (struct superbloque *) malloc(sizeof(struct superbloque));
	bread(posSB, &sb);
	struct inodo inodos[BLOCKSIZE / T_INODO];
	int it;

	unsigned int pos_inicial = sb.posPrimerBloqueAI;
	unsigned int pos_final = sb.posUltimoBloqueAI;
	unsigned int x = 1;

	for (; pos_inicial <= pos_final; pos_inicial++) {
		int j;
		for (j = 0; j < BLOCKSIZE / T_INODO /* && !ultimo */ ; j++) {
//                      inodos[j].tipo = 'l';
			if (x < sb.totInodos) {
				inodos[j].tipo = 'l';
				inodos[j].punterosDirectos[0] = x;
				x++;
			} else {
				//Se puede dar el caso de que queden inodos no asignados en el ultimo bloque.
				inodos[j].punterosDirectos[0] = UINT_MAX;
				j = 8;	//final bucle
			}
		}
		if (bwrite(pos_inicial, inodos) != 0)
			return -1;
		//AI ocupado
		escribir_bit(pos_inicial, 1);
		sb.cantBloquesLibres--;
	}
	bwrite(posSB, &sb);
	return 0;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueMB;
	unsigned int pos_byte = nbloque / 8;
	unsigned int pos_bit = nbloque % 8;
	unsigned int bloque_leer = pos_byte / BLOCKSIZE + pos_inicial;
	unsigned char mascara = 128;
	mascara >>= pos_bit;
	unsigned char var[BLOCKSIZE];
	bread(bloque_leer, var);
	pos_byte = pos_byte % BLOCKSIZE;
	if (bit == 1)
		var[pos_byte] |= mascara;
	else if (bit == 0)
		var[pos_byte] &= ~mascara;
	else
		return -1;
	if (bwrite(bloque_leer, var) != 0)
		return -1;
	return 0;
}

unsigned char leer_bit(unsigned int nbloque)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueMB;
	unsigned int pos_byte = nbloque / 8;
	unsigned int pos_bit = nbloque % 8;
	unsigned int bloque_leer = pos_byte / BLOCKSIZE + pos_inicial;
	unsigned char resultado = 128;
	resultado >>= pos_bit;
	unsigned char var[BLOCKSIZE];
	bread(bloque_leer, var);
	pos_byte = pos_byte % BLOCKSIZE;
	resultado &= var[pos_byte];
	resultado >>= (7 - pos_bit);
	return resultado;
}

int reservar_bloque()
{
	struct superbloque sb;
//      waitSemSB(semSB);
	bread(posSB, &sb);
	if (sb.cantBloquesLibres > 0) {
		unsigned int pos_inicial = sb.posPrimerBloqueMB;
		unsigned int bloqueMB = pos_inicial;
		unsigned char bufferAux[BLOCKSIZE];
		unsigned char var[BLOCKSIZE];
		memset(bufferAux, 255, BLOCKSIZE);
		int encontrado = 0;
//              waitSemSB(semMB);
		while (!encontrado) {
			bread(bloqueMB++, var);
			if (memcmp(var, bufferAux, BLOCKSIZE) != 0)
				encontrado = 1;
		}
		bloqueMB--;
		unsigned int pos_byte = 0;
		unsigned char mascara = 128;
		int pos_bit = 0;
		encontrado = 0;
		while (var[pos_byte] == 255)
			pos_byte++;
		unsigned char byte = var[pos_byte];
		while (byte & mascara) {
			byte <<= 1;
			pos_bit++;
		}

		unsigned int num_bloque =
		    ((bloqueMB - pos_inicial) * BLOCKSIZE + pos_byte) * 8 +
		    pos_bit;
		escribir_bit(num_bloque, 1);
//              signalSemSB(semMB);
		sb.cantBloquesLibres--;
		bwrite(posSB, &sb);
//              signalSemSB(semSB);
		return num_bloque;
	} else {
//              signalSemSB(semSB);
		return -1;
	}
}

int liberar_bloque(unsigned int nbloque)
{
	struct superbloque sb;
//      waitSemSB(semSB);
//      waitSemSB(semMB);
	escribir_bit(nbloque, 0);
//      signalSemSB(semMB);
	bread(posSB, &sb);
	sb.cantBloquesLibres++;
	bwrite(posSB, &sb);
//      signalSemSB(semSB);
	return nbloque;
}

int escribir_inodo(struct inodo inodo, unsigned int ninodo)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE / T_INODO];
	unsigned int bloque = ninodo / (BLOCKSIZE / T_INODO) + pos_inicial;
//      waitSemSB(semAI);
	bread(bloque, inodos);
	int i = (ninodo % (BLOCKSIZE / T_INODO));
	inodos[i] = inodo;
	bwrite(bloque, inodos);
//      signalSemSB(semAI);
	return 0;
}

struct inodo leer_inodo(unsigned int ninodo)
{
	struct superbloque sb;
	bread(posSB, &sb);
	struct inodo inodos[BLOCKSIZE / T_INODO];
	unsigned int bloque =
	    ninodo / (BLOCKSIZE / T_INODO) + sb.posPrimerBloqueAI;
//      waitSemSB(semAI);
	bread(bloque, inodos);
//      signalSemSB(semAI);
	return inodos[(ninodo % (BLOCKSIZE / T_INODO))];
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
	struct superbloque sb;
//      waitSemSB(semSB);
	bread(posSB, &sb);
	if (sb.cantInodosLibres == 0) {
		printf("No hay inodos libres. No se puede reservar inodo!\n");
		return -1;
	}
	unsigned int pos_pinodo = sb.posPrimerInodoLibre;
	struct inodo inodoAux;
	inodoAux = leer_inodo(pos_pinodo);
//      printf("He leido el inodo primero %d pid %d\n",pos_pinodo,getpid());
	struct inodo inodo;
	inodo.tipo = tipo;
	inodo.permisos = permisos;
	inodo.atime = time(NULL);
	inodo.mtime = time(NULL);
	inodo.ctime = time(NULL);
	inodo.nlinks = 1;
	inodo.tamEnBytesLog = 0;
	inodo.numBloquesOcupados = 0;
	int i;
	for (i = 0; i < 12; i++)
		inodo.punterosDirectos[i] = 0;
	for (i = 0; i < 3; i++)
		inodo.punterosIndirectos[i] = 0;
	escribir_inodo(inodo, pos_pinodo);
	sb.posPrimerInodoLibre = inodoAux.punterosDirectos[0];
	sb.cantInodosLibres--;
	bwrite(posSB, &sb);
//      signalSemSB(semSB);
	return pos_pinodo;
}

int liberar_inodo(unsigned int ninodo)
{
	struct inodo inodo = leer_inodo(ninodo);
	if (inodo.tipo != 'l') {
		liberar_bloques_inodo(ninodo, 0);
		inodo.tipo = 'l';
		struct superbloque sb;
//              waitSemSB(semSB);
		bread(posSB, &sb);
		inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
		sb.posPrimerInodoLibre = ninodo;
		sb.cantInodosLibres++;
		escribir_inodo(inodo, ninodo);
		bwrite(posSB, &sb);
//              signalSemSB(semSB);
		return ninodo;
	}
	printf("El inodo esta libre!\n");
	return -1;
}

int liberar_bloques_inodo1(unsigned int ninodo, unsigned int blogico)
{
	int blog;
	int num_pDirectos = 12;
	int num_punteros = BLOCKSIZE/sizeof(unsigned int);
	int num_punteros2 = num_punteros * num_punteros;
	int num_punteros3 = num_punteros2 * num_punteros;
	int bufferIndirectos[num_punteros];
	int bufferIndirectos0[num_punteros];
	int bufferIndirectos1[num_punteros];
	int bufferIndirectos2[num_punteros];
	struct inodo in;
	int indirectos0 = 0, indirectos1 = 0, indirectos2 = 0;

	in = leer_inodo(ninodo);
	int pbloque[3] = { /*UINT_MAX, UINT_MAX, UINT_MAX */ 0, 0, 0 };
	int ultimoBloque = in.tamEnBytesLog / BLOCKSIZE;
	blog = blogico;
	if (blog < num_pDirectos) {
		for (blog; blog < num_pDirectos; blog++) {
			if (in.punterosDirectos[blog] > 0) {
				liberar_bloque(in.punterosDirectos[blog]);
				in.punterosDirectos[blog] = 0;
				in.numBloquesOcupados--;
			}
		}
		pbloque[0] = 0;
		pbloque[1] = 0;
		pbloque[2] = 0;
		//Tenemos que hacer 3 llamadas si existe el bloque de punteros
		if (in.punterosIndirectos[0] > 0) {
			printf("indirectos0 blog < npundirectos\n");
			bread(in.punterosIndirectos[0], bufferIndirectos0);
			liberar(bufferIndirectos0, 1, &in, pbloque);
			in.punterosIndirectos[0] = 0;
		}

	} else if (blog < num_pDirectos + num_punteros) {	//Si está en los punteroindirectos0
		printf("indirectos0\n");
		pbloque[0] = blog - num_pDirectos;
		if (in.punterosIndirectos[0] > 0) {
			bread(in.punterosIndirectos[0], bufferIndirectos0);
			//puede que tengamos que conservar algunos punteros si son < blog - num_pDirectos
			liberar(bufferIndirectos0, 1, &in, pbloque);
			in.punterosIndirectos[0] = 0;
		}
	} else if (blog < num_pDirectos + num_punteros + num_punteros2)	//Si está en indirectos1
		indirectos1 = 1;

	else if (blog < num_pDirectos + num_punteros + num_punteros3)	//Si está en indirectos2
		indirectos2 = 1;

	if (indirectos1 == 1) {
		pbloque[0] =
		    (blog - (num_pDirectos + num_punteros)) % num_punteros;
		pbloque[1] =
		    (blog - (num_pDirectos + num_punteros)) / num_punteros;
	} else {
		pbloque[0] = 0;
		pbloque[1] = 0;
	}
	if (in.punterosIndirectos[1] > 0 && indirectos2 == 0) {
		bread(in.punterosIndirectos[1], bufferIndirectos1);
		liberar(bufferIndirectos1, 2, &in, pbloque);
		in.punterosIndirectos[1] = 0;
	}
	if (indirectos2) {
		pbloque[0] = ((blog -
			       (num_pDirectos + num_punteros +
				num_punteros2)) % num_punteros2) % num_punteros;
		pbloque[1] =
		    ((blog -
		      (num_pDirectos + num_punteros +
		       num_punteros2)) % num_punteros2) / num_punteros;
		pbloque[2] =
		    (blog -
		     (num_pDirectos + num_punteros +
		      num_punteros2)) / num_punteros2;
	} else {
		pbloque[0] = 0;
		pbloque[1] = 0;
	}
	if (in.punterosIndirectos[2] > 0) {
		bread(in.punterosIndirectos[2], bufferIndirectos2);
		liberar(bufferIndirectos2, 3, &in, pbloque);
		in.punterosIndirectos[2] = 0;
	}
	escribir_inodo(in, ninodo);
	return 0;
}

int liberar(unsigned int buffer[256], unsigned int nivel, struct inodo *in,
	    int *pbloque)
{
	unsigned int bufferAux[256];
	if (nivel > 0) {
		int i, borrados = 0;
		for (i = 0; i < 256; i++) {
			if (nivel == 3 && buffer[i] > 0 && i >= pbloque[2]) {
				borrados++;
				bread(buffer[i], bufferAux);	//leemos el puntero que apunta al bloque y lo guardamos.
				int resultado =
				    liberar(bufferAux, nivel - 1, in, pbloque);
				if (resultado == 1) {
					buffer[i] = 0;
					liberar_bloque(buffer[i]);
					in->numBloquesOcupados--;
				} else
					bwrite(buffer[i], bufferAux);
			} else if (nivel == 2 && buffer[i] > 0
				   && i >= pbloque[1]) {
				borrados++;
				bread(buffer[i], bufferAux);	//leemos el puntero que apunta al bloque y lo guardamos.
				int resultado =
				    liberar(bufferAux, nivel - 1, in, pbloque);
				if (resultado == 1) {
					buffer[i] = 0;
					liberar_bloque(buffer[i]);
					in->numBloquesOcupados--;
				} else
					bwrite(buffer[i], bufferAux);
			} else if (nivel == 1 && buffer[i] > 0 && i >= pbloque[0]) {	//nivel 1
				borrados++;
				buffer[i] = 0;
				liberar_bloque(buffer[i]);
				in->numBloquesOcupados--;
			}
		}
		if (borrados == 256)
			return 1;	//si se han borrado todos
		return 0;
	}
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico)
{
	unsigned int num_pDirectos = 12;
	unsigned int num_punteros = BLOCKSIZE / sizeof(unsigned int);	//numero de punteros que caben en un bloque = 256.
	unsigned int num_punteros2 = num_punteros * num_punteros;
	unsigned int num_punteros3 = num_punteros * num_punteros * num_punteros;
	unsigned int bufferAux[BLOCKSIZE / sizeof(unsigned int)];	//mascara
	unsigned int bufferIndirectos0[num_punteros];
	unsigned int bufferIndirectos1[num_punteros];
	unsigned int bufferIndirectos2[num_punteros];
	unsigned int punt0, punt1, punt2;

	struct inodo inodo = leer_inodo(ninodo);
	unsigned int ultimoBloque = inodo.tamEnBytesLog / BLOCKSIZE;	//ultimo bloque con contenido.
	memset(bufferAux, 0, BLOCKSIZE);
	int blog;
	for (blog = blogico; blog <= ultimoBloque; blog++) {
		//PUNTEROS DIRECTOS
		//El bloque logico es uno de los 12 primeros bloques logicos del inodo.
		if (blog < num_pDirectos) {
			if (inodo.punterosDirectos[blog] > 0) {
				liberar_bloque(inodo.punterosDirectos[blog]);
				inodo.punterosDirectos[blog] = 0;
				inodo.numBloquesOcupados--;
			}
		}
		//PUNTERO INDIRECTOS 0 
		//El bloque logico lo encontramos en el puntero a bloques Indirectos 0, es decir, esta comprendido entre el 0+12 y el 0+12+256-1: entre el 12 y el 267.
		else if (blog < num_pDirectos + num_punteros) {
			if (inodo.punterosIndirectos[0] > 0) {
				bread(inodo.punterosIndirectos[0], bufferIndirectos0);	//leemos el bloque de punteros indirectos 0.
				if (bufferIndirectos0[blog - num_pDirectos] > 0) {
					liberar_bloque(bufferIndirectos0[blog - num_pDirectos]);	//liberamos el bloque fisico de datos.
					bufferIndirectos0[blog -
							  num_pDirectos] = 0;
					inodo.numBloquesOcupados--;

					//si no quedan punteros ocupados en el bloque de indirectos 0, tambien lo liberamos
					if (memcmp
					    (bufferAux, bufferIndirectos0,
					     BLOCKSIZE) == 0) {
						liberar_bloque(inodo.
							       punterosIndirectos
							       [0]);
						inodo.punterosIndirectos[0] = 0;
					} else	//si no, escribimos el bloque de indirectos 0 modificado.
						bwrite(inodo.
						       punterosIndirectos[0],
						       bufferIndirectos0);
				}
			}
		}
		//PUNTERO INDIRECTOS 1 
		//El bloque logico lo encontramos en el puntero a bloques Indirectos 1, es decir, los comprendidos entre el 0+12+256 y el 0+12+256+256^2-1: entre el 268 y el 65.803.
		else if (blog < num_pDirectos + num_punteros + num_punteros2) {
			punt0 = (blog - (num_pDirectos + num_punteros)) % num_punteros;	//puntero nivel 0
			punt1 = (blog - (num_pDirectos + num_punteros)) / num_punteros;	//puntero nivel 1
			if (inodo.punterosIndirectos[1] > 0) {
				bread(inodo.punterosIndirectos[1], bufferIndirectos1);	//leemos el bloque de punteros indirectos 1.
				if (bufferIndirectos1[punt1] > 0) {
					bread(bufferIndirectos1[punt1], bufferIndirectos0);	//leemos el bloque de punteros indirectos 0.
					if (bufferIndirectos0[punt0] > 0) {
						liberar_bloque(bufferIndirectos0[punt0]);	//liberamos el bloque fisico de datos.
						bufferIndirectos0[punt0] = 0;
						inodo.numBloquesOcupados--;

						//si no quedan punteros ocupados en el bloque de indirectos 0, tambien lo liberamos
						if (memcmp
						    (bufferAux,
						     bufferIndirectos0,
						     BLOCKSIZE) == 0) {
							liberar_bloque
							    (bufferIndirectos1
							     [punt1]);
							bufferIndirectos1[punt1]
							    = 0;
						} else	//si no, escribimos el bloque de indirectos 0 modificado.
							bwrite(bufferIndirectos1
							       [punt1],
							       bufferIndirectos0);

						//si no quedan punteros ocupados en el bloque de indirectos 1, tambien lo liberamos
						if (memcmp
						    (bufferAux,
						     bufferIndirectos1,
						     BLOCKSIZE) == 0) {
							liberar_bloque(inodo.
								       punterosIndirectos
								       [1]);
							inodo.
							    punterosIndirectos
							    [1] = 0;
						} else	//si no, escribimos el bloque de indirectos 1 modificado.
							bwrite(inodo.
							       punterosIndirectos
							       [1],
							       bufferIndirectos1);
					}
				}
			}
		}
		//PUNTERO INDIRECTOS 2 
		//El bloque logico lo encontramos en el puntero a bloques Indirectos 2, es decir, los comprendidos entre el 0+12+256+256^2 y el 0+12+256+256^2+256^3-1: entre el 65.804 y el 16.843.019.
		else if (blog <
			 num_pDirectos + num_punteros + num_punteros2 +
			 num_punteros3) {
			punt0 =
			    ((blog -
			      (num_pDirectos + num_punteros +
			       num_punteros2)) % num_punteros2) % num_punteros;
			punt1 =
			    ((blog -
			      (num_pDirectos + num_punteros +
			       num_punteros2)) % num_punteros2) / num_punteros;
			punt2 =
			    (blog -
			     (num_pDirectos + num_punteros +
			      num_punteros2)) / num_punteros2;
			if (inodo.punterosIndirectos[2] > 0) {
				bread(inodo.punterosIndirectos[2], bufferIndirectos2);	//leemos el bloque de indirectos 2
				if (bufferIndirectos2[punt2] > 0) {
					bread(bufferIndirectos2[punt2], bufferIndirectos1);	//leemos el bloque de indirectos 1
					if (bufferIndirectos1[punt1] > 0) {
						bread(bufferIndirectos1[punt1], bufferIndirectos0);	//leemos el bloque de indirectos 0
						if (bufferIndirectos0[punt0] >
						    0) {
							liberar_bloque(bufferIndirectos0[punt0]);	//liberamos el bloque fisico de datos.
							bufferIndirectos0[punt0]
							    = 0;
							inodo.
							    numBloquesOcupados--;

							//si no quedan punteros ocupados en el bloque de indirectos 0, tambien lo liberamos
							if (memcmp
							    (bufferAux,
							     bufferIndirectos0,
							     BLOCKSIZE) == 0) {
								liberar_bloque
								    (bufferIndirectos1
								     [punt1]);
								bufferIndirectos1
								    [punt1] = 0;
							} else	//si no, escribimos el bloque de indirectos 0 modificado.
								bwrite
								    (bufferIndirectos1
								     [punt1],
								     bufferIndirectos0);

							//si no quedan punteros ocupados en el bloque de indirectos 1, tambien lo liberamos
							if (memcmp
							    (bufferAux,
							     bufferIndirectos1,
							     BLOCKSIZE) == 0) {
								liberar_bloque
								    (bufferIndirectos2
								     [punt2]);
								bufferIndirectos2
								    [punt2] = 0;
							} else	//si no, escribimos el bloque de indirectos 1 modificado.
								bwrite
								    (bufferIndirectos2
								     [punt2],
								     bufferIndirectos1);

							//si no quedan punteros ocupados en el bloque de indirectos 2, tambien lo liberamos
							if (memcmp
							    (bufferAux,
							     bufferIndirectos2,
							     BLOCKSIZE) == 0) {
								liberar_bloque
								    (inodo.
								     punterosIndirectos
								     [2]);
								inodo.
								    punterosIndirectos
								    [2] = 0;
							} else	//si no, escribimos el bloque de indirectos 2 modificado.
								bwrite(inodo.
								       punterosIndirectos
								       [2],
								       bufferIndirectos2);
						}
					}
				}
			}
		}
	}
	escribir_inodo(inodo, ninodo);
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico,
			  unsigned int *bfisico, char reservar)
{
	unsigned int num_pDirectos = 12;
	unsigned int num_punteros = BLOCKSIZE / sizeof(unsigned int);	//numero de punteros que caben en un bloque = 256.
	unsigned int num_punteros2 = num_punteros * num_punteros;
	unsigned int num_punteros3 = num_punteros * num_punteros * num_punteros;
	//leemos el inodo solicitado.
	struct inodo inodo;
	inodo = leer_inodo(ninodo);
	int act = 0;
	if (blogico < num_pDirectos) {	//el bloque logico es uno de los 12 primeros bloques logicos del inodo.
		switch (reservar) {
		case 0:	//modo consulta
			if (inodo.punterosDirectos[blogico] == 0)	//no existe bloque fisico.
				return -1;
			else {
				*bfisico = inodo.punterosDirectos[blogico];
				//return 0;
			}
			break;
		case 1:	//modo escritura
//                              waitSem(sem);
			if (inodo.punterosDirectos[blogico] == 0) {	//si no existe bloque fisico le asignamos uno.
				waitSem(sem);
				inodo.punterosDirectos[blogico] =
				    reservar_bloque();
				inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
				*bfisico = inodo.punterosDirectos[blogico];
				//escribimos el inodo con la info actualizada. 
				escribir_inodo(inodo, ninodo);
				signalSem(sem);
				//return 0;
			} else {	//existe el bloque fisico y lo devolvemos
				*bfisico = inodo.punterosDirectos[blogico];
				//return 0;
			}
//                              signalSem(sem);
			break;
		}
		return 0;
	}
	//PUNTERO INDIRECTOS 0 
	//El bloque logico lo encontramos en el puntero a bloques Indirectos 0, es decir, esta comprendido entre el 0+12 y el 0+12+256-1: entre el 12 y el 267.
	else if (blogico < num_pDirectos + num_punteros) {
		unsigned int bufferIndirectos0[BLOCKSIZE /
					       sizeof(unsigned int)];
		switch (reservar) {
		case 0:	//modo consulta
			if (inodo.punterosIndirectos[0] == 0)	//no existe el bloque fisico de punteros indirectos de nivel 0.
				return -1;
			else {	//existe el bloque de punteros y lo leemos del sistema de ficheros
				bread(inodo.punterosIndirectos[0],
				      bufferIndirectos0);
				if (bufferIndirectos0[blogico - num_pDirectos] == 0)	//no existe el bloque fisico de datos.
					return -1;
				else {
					*bfisico =
					    bufferIndirectos0[blogico -
							      num_pDirectos];
				}
			}
			break;
		case 1:	//modo escritura
//                              waitSem(sem);
			if (inodo.punterosIndirectos[0] == 0) {	//no existe el bloque fisico de punteros indirectos de nivel 0, asi que lo creamos.
				memset(bufferIndirectos0, 0, BLOCKSIZE);	//iniciamos a 0 los 256 punteros.
				//reservamos un nuevo bloque en el puntero correspondiente. Este es el bloque fisico que tenemos que devolver.
				waitSem(sem);
				act = 1;
				bufferIndirectos0[blogico - num_pDirectos] = reservar_bloque();	//para datos
				inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
				//ahora reservamos otro bloque para almacenar el buffer de punteros indirectos 0, y lo escribimos en el SF.
				inodo.punterosIndirectos[0] = reservar_bloque();	//para punteros
				bwrite(inodo.punterosIndirectos[0],
				       bufferIndirectos0);
				// devolvemos el bloque fisico traducido
				*bfisico =
				    bufferIndirectos0[blogico - num_pDirectos];
			} else {	//existe el bloque de punteros y lo leemos del sistema de ficheros
				bread(inodo.punterosIndirectos[0],
				      bufferIndirectos0);
				if (bufferIndirectos0[blogico - num_pDirectos] == 0) {	//no existe el bloque fisico de datos, entonces lo reservamos
					waitSem(sem);
					act = 1;
					bufferIndirectos0[blogico -
							  num_pDirectos] =
					    reservar_bloque();
					inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
					bwrite(inodo.punterosIndirectos[0], bufferIndirectos0);	//escribimos en el SF el bloque de punteros 0 modificado.
					*bfisico = bufferIndirectos0[blogico - num_pDirectos];	//devolvemos el bloque fisico traducido.
				} else {	// si existe el bloque fisico de datos.
					*bfisico =
					    bufferIndirectos0[blogico -
							      num_pDirectos];
				}
			}
			//escribimos en el SF el inodo actualizado
			escribir_inodo(inodo, ninodo);
			if (act)
				signalSem(sem);
			break;
		}
		return 0;
	}
	//PUNTERO INDIRECTOS 1
	//El bloque logico lo encontramos en el puntero a bloques Indirectos 1, es decir, los comprendidos entre el 0+12+256 y el 0+12+256+256^2-1: entre el 268 y el 65.803.
	else if (blogico < num_pDirectos + num_punteros + num_punteros2) {
		unsigned int bufferIndirectos0[BLOCKSIZE /
					       sizeof(unsigned int)];
		unsigned int bufferIndirectos1[BLOCKSIZE /
					       sizeof(unsigned int)];
		unsigned int punt0 = (blogico - (num_pDirectos + num_punteros)) % num_punteros;	//puntero nivel 0
		unsigned int punt1 = (blogico - (num_pDirectos + num_punteros)) / num_punteros;	//puntero nivel 1
		switch (reservar) {
		case 0:	//modo consulta
			if (inodo.punterosIndirectos[1] == 0)	//el bloque fisico de punteros indirectos 1 no existe.
				return -1;
			else {
				bread(inodo.punterosIndirectos[1], bufferIndirectos1);	//leemos el bloque de punteros indirectos 1.
				if (bufferIndirectos1[punt1] == 0)	//el bloque fisico de punteros indirectos 0 no existe.
					return -1;
				else {
					bread(bufferIndirectos1[punt1], bufferIndirectos0);	//leemos el bloque de punteros indirectos 0.
					if (bufferIndirectos0[punt0] == 0)	//el bloque fisico de datos no existe.
						return -1;
					else {
						*bfisico =
						    bufferIndirectos0[punt0];
					}
				}
			}
			break;
		case 1:	//modo escritura
//                              waitSem(sem);
			if (inodo.punterosIndirectos[1] == 0) {	//el bloque fisico de punteros indirectos 1 no existe, asi que lo creamos.
				waitSem(sem);
				act = 1;
				inodo.punterosIndirectos[1] = reservar_bloque();	//reservamos el bloque de los punteros indirectos 1
				memset(bufferIndirectos1, 0, BLOCKSIZE);
				bufferIndirectos1[punt1] = reservar_bloque();	//reservamos el bloque de los punteros indirectos 0
				memset(bufferIndirectos0, 0, BLOCKSIZE);
				bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque de datos
				inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
				//escribimos los dos bloques de punteros
				bwrite(inodo.punterosIndirectos[1],
				       bufferIndirectos1);
				bwrite(bufferIndirectos1[punt1],
				       bufferIndirectos0);
				//devolvemos el bloque fisico traducido
				*bfisico = bufferIndirectos0[punt0];
			} else {	//existe el bloque de punteros indirectos 1 y lo leemos
				bread(inodo.punterosIndirectos[1],
				      bufferIndirectos1);
				if (bufferIndirectos1[punt1] == 0) {	// el bloque de punteros indirectos 0 no existe, asi que lo creamos
					waitSem(sem);
					act = 1;
					bufferIndirectos1[punt1] = reservar_bloque();	//reservamos el bloque de los punteros indirectos 0
					memset(bufferIndirectos0, 0, BLOCKSIZE);
					bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque de datos
					inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
					//escribimos los dos bloques de punteros
					bwrite(inodo.punterosIndirectos[1],
					       bufferIndirectos1);
					bwrite(bufferIndirectos1[punt1],
					       bufferIndirectos0);
					//devolvemos el bloque fisico traducido
					*bfisico = bufferIndirectos0[punt0];
				} else {
					bread(bufferIndirectos1[punt1], bufferIndirectos0);	//leemos el bloque de punteros indirectos 0
					if (bufferIndirectos0[punt0] == 0) {	//el bloque fisico de datos no existe, asi que lo creamos (reservamos).
						waitSem(sem);
						act = 1;
						bufferIndirectos0[punt0] =
						    reservar_bloque();
						inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
						//escribimos el bloque de punteros indirectos 0 modificado
						bwrite(bufferIndirectos1[punt1],
						       bufferIndirectos0);
						*bfisico =
						    bufferIndirectos0[punt0];
					} else {
						*bfisico =
						    bufferIndirectos0[punt0];
					}
				}
			}
			//escribimos en el SF el inodo actualizado
			escribir_inodo(inodo, ninodo);
			if (act)
				signalSem(sem);
			break;
		}
		return 0;
	}
	//PUNTERO INDIRECTOS 2 
	//El bloque logico lo encontramos en el puntero a bloques Indirectos 2, es decir, los comprendidos entre el 0+12+256+256^2 y el 0+12+256+256^2+256^3-1: entre el 65.804 y el 16.843.019.
	else if (blogico <
		 num_pDirectos + num_punteros + num_punteros2 + num_punteros3) {
		unsigned int bufferIndirectos0[BLOCKSIZE /
					       sizeof(unsigned int)];
		unsigned int bufferIndirectos1[BLOCKSIZE /
					       sizeof(unsigned int)];
		unsigned int bufferIndirectos2[BLOCKSIZE /
					       sizeof(unsigned int)];
		unsigned int punt2 =
		    (blogico -
		     (num_pDirectos + num_punteros +
		      num_punteros2)) / num_punteros2;
		unsigned int punt1 =
		    ((blogico -
		      (num_pDirectos + num_punteros +
		       num_punteros2)) % num_punteros2) / num_punteros;
		unsigned int punt0 =
		    ((blogico -
		      (num_pDirectos + num_punteros +
		       num_punteros2)) % num_punteros2) % num_punteros;
		switch (reservar) {
		case 0:	//modo consulta
			if (inodo.punterosIndirectos[2] == 0)	//el bloque de punteros indirectos de nivel 2 no existe.
				return -1;
			else {
				bread(inodo.punterosIndirectos[2], bufferIndirectos2);	//leemos el bloque de punteros indirectos de nivel 2
				if (bufferIndirectos2[punt2] == 0)	//el bloque de punteros indirectos de nivel 1 no existe.
					return -1;
				else {
					bread(bufferIndirectos2[punt2], bufferIndirectos1);	//leemos el bloque de punteros indirectos de nivel 1
					if (bufferIndirectos1[punt1] == 0)	//el bloque de punteros indirectos de nivel 0 no existe.
						return -1;
					else {
						bread(bufferIndirectos1[punt1],
						      bufferIndirectos0);
						if (bufferIndirectos0[punt0] == 0)	//el bloque fisico de datos no existe
							return -1;
						else {
							*bfisico = bufferIndirectos0[punt0];	//devolvemos el bloque fisico solicitado
						}
					}
				}
			}
			break;
		case 1:	//modo escritura
//                              waitSem(sem);
			if (inodo.punterosIndirectos[2] == 0) {	//el bloque de punteros indirectos de nivel 2 no existe, por tanto tenemos que reservarlo
				waitSem(sem);
				act = 1;
				inodo.punterosIndirectos[2] = reservar_bloque();	//reservamos el bloque para los punteros indirectos de nivel 2.
				memset(bufferIndirectos2, 0, BLOCKSIZE);
				bufferIndirectos2[punt2] = reservar_bloque();	//reservamos el bloque para los punteros indirectos de nivel 1.
				memset(bufferIndirectos1, 0, BLOCKSIZE);
				bufferIndirectos1[punt1] = reservar_bloque();	//reservamos el bloque para los punteros indirectos de nivel 0.
				memset(bufferIndirectos0, 0, BLOCKSIZE);
				bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque fisico de datos.
				inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
				//escribimos en el SF los 3 bloques de punteros indirectos
				bwrite(inodo.punterosIndirectos[2],
				       bufferIndirectos2);
				bwrite(bufferIndirectos2[punt2],
				       bufferIndirectos1);
				bwrite(bufferIndirectos1[punt1],
				       bufferIndirectos0);
				//devolvemos el bloque fisico solicitado
				*bfisico = bufferIndirectos0[punt0];
			} else {	//existe el bloque de punteros indirectos de nivel 2, asi que lo leemos.
				bread(inodo.punterosIndirectos[2],
				      bufferIndirectos2);
				if (bufferIndirectos2[punt2] == 0) {	//el bloque de punteros indirectos de nivel 1 no existe, asi que lo reservamos
					waitSem(sem);
					act = 1;
					bufferIndirectos2[punt2] = reservar_bloque();	//reservamos el bloque para los punteros indirectos 1.
					memset(bufferIndirectos1, 0, BLOCKSIZE);
					bufferIndirectos1[punt1] = reservar_bloque();	//reservamos el bloque para los punteros indirectos 0.
					memset(bufferIndirectos0, 0, BLOCKSIZE);
					bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque fisico de datos.
					inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
					//escribimos en el SF el bloque de punteros 2 actualizado y los otros dos bloques de punteros indirectos(1,0).
					bwrite(inodo.punterosIndirectos[2],
					       bufferIndirectos2);
					bwrite(bufferIndirectos2[punt2],
					       bufferIndirectos1);
					bwrite(bufferIndirectos1[punt1],
					       bufferIndirectos0);
					//devolvemos el bloque fisico solicitado
					*bfisico = bufferIndirectos0[punt0];
				} else {	//existe el bloque de punteros indirectos de nivel 1, asi que lo leemos.
					bread(bufferIndirectos2[punt2],
					      bufferIndirectos1);
					if (bufferIndirectos1[punt1] == 0) {	//el bloque de punteros indirectos de nivel 0 no existe, asi que lo reservamos.
						waitSem(sem);
						act = 1;
						bufferIndirectos1[punt1] = reservar_bloque();	//reservamos el bloque para los punteros indirectos 0.
						memset(bufferIndirectos0, 0,
						       BLOCKSIZE);
						bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque fisico de datos.
						inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
						//escribimos en el SF el bloque de punteros indirectos 1 actualizado y el nuevo bloque de punteros indirectos de nivel 0.
						bwrite(bufferIndirectos2[punt2],
						       bufferIndirectos1);
						bwrite(bufferIndirectos1[punt1],
						       bufferIndirectos0);
						//devolvemos el bloque fisico solicitado
						*bfisico =
						    bufferIndirectos0[punt0];
					} else {	//existe el bloque de punteros indirectos de nivel 0, asi que lo leemos
						bread(bufferIndirectos1[punt1],
						      bufferIndirectos0);
						if (bufferIndirectos0[punt0] == 0) {	//el bloque fisico de datos no existe, lo reservamos.
							waitSem(sem);
							act = 1;
							bufferIndirectos0[punt0] = reservar_bloque();	//reservamos el bloque fisico de datos.
							inodo.numBloquesOcupados++;	//aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
							//escribimos en el SF el bloque de punteros indirectos 0.
							bwrite(bufferIndirectos1
							       [punt1],
							       bufferIndirectos0);
							*bfisico =
							    bufferIndirectos0
							    [punt0];
						} else {
							*bfisico =
							    bufferIndirectos0
							    [punt0];
						}
					}
				}
			}
			//escribimos en el SF el inodo actualizado
			escribir_inodo(inodo, ninodo);
			if (act)
				signalSem(sem);
			break;
		}
		return 0;
	}
}
