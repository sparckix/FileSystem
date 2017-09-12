#include "../include/ficheros_basico.h"
int liberar(unsigned int buffer[256], unsigned int nivel, struct inodo *in,
	    int *pbloque);
int tamMB(unsigned int nbloques)
{
	int tamMB;
	if (((nbloques / 8) % BLOCKSIZE) == 0)
		tamMB = (nbloques / 8) / BLOCKSIZE;
	else
		tamMB = ((nbloques / 8) / BLOCKSIZE) + 1;
	return tamMB;
}

int tamAI(unsigned int ninodos)
{
	int tamAI;
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
	//ojo
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
	}
	//Escribimos en el MB los bloques ocupados.
	//Superbloque ocupado
	escribir_bit(posSB, 1);
	sb.cantBloquesLibres--;
	//MB ocupado
	pos_inicial = sb.posPrimerBloqueMB;
	int bloques = pos_final - pos_inicial;
	for (; bloques >= 0; bloques--) {
		escribir_bit(pos_inicial++, 1);
		sb.cantBloquesLibres--;
	}
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
// for(it=0; it < BLOCKSIZE; it++) {
//    inodos[it] = (struct inodo *) malloc(sizeof(struct inodo));
// }
	unsigned int pos_inicial = sb.posPrimerBloqueAI;
	unsigned int pos_final = sb.posUltimoBloqueAI;
	unsigned int x = 1;
	int j;
	int ultimo = 0;
	for (; pos_inicial <= pos_final; pos_inicial++) {
		for (j = 0; j < BLOCKSIZE / T_INODO && !ultimo; j++) {
			inodos[j].tipo = 'l';
			if (x < ninodos) {
				inodos[j].punterosDirectos[0] = x;
				x++;
			} else {
				//Pueden quedar inodos no asignados en el último bloque.
				ultimo = 1;
				inodos[j].punterosDirectos[0] = UINT_MAX;
			}
		}
		if (bwrite(pos_inicial, inodos) != 0)
			return -1;
	}
	//AI ocupado
	pos_inicial = sb.posPrimerBloqueAI;
	pos_final = sb.posUltimoBloqueAI;
	int bloques = pos_final - pos_inicial;
	while (bloques >= 0) {
		escribir_bit(pos_inicial++, 1);
		sb.cantBloquesLibres--;
		bloques--;
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
	bread(posSB, &sb);
	if (sb.cantBloquesLibres > 0) {
		unsigned int pos_inicial = sb.posPrimerBloqueMB;
		unsigned int bloqueMB = pos_inicial;
		unsigned char bufferAux[BLOCKSIZE];
		unsigned char var[BLOCKSIZE];
		memset(bufferAux, 255, BLOCKSIZE);
		int encontrado = 0;
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
		while (!encontrado) {
			if (var[pos_byte] < 255) {
				while (var[pos_byte] & mascara) {
					var[pos_byte] <<= 1;
					pos_bit++;
				}
				encontrado = 1;
			} else
				pos_byte++;
		}
		/* unsigned */ int num_bloque =
		    ((bloqueMB - pos_inicial) * BLOCKSIZE + pos_byte) * 8 +
		    pos_bit;
		escribir_bit(num_bloque, 1);
		sb.cantBloquesLibres--;
		bwrite(posSB, &sb);
		return num_bloque;
	} else
		return -1;
}

int liberar_bloque(unsigned int nbloque)
{
	struct superbloque sb;
	escribir_bit(nbloque, 0);
	bread(posSB, &sb);
	sb.cantBloquesLibres++;
	bwrite(posSB, &sb);
	return nbloque;
}

int escribir_inodo(struct inodo inodo, unsigned int ninodo)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE / T_INODO];
	unsigned int bloque = ninodo / (BLOCKSIZE / T_INODO) + pos_inicial;
	bread(bloque, inodos);
	int i = (ninodo % (BLOCKSIZE / T_INODO));
	inodos[i] = inodo;
	bwrite(bloque, inodos);
	return 0;
}

struct inodo leer_inodo(unsigned int ninodo)
{
	struct superbloque sb;
	bread(posSB, &sb);
	unsigned int pos_inicial = sb.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE / T_INODO];
	unsigned int bloque = ninodo / (BLOCKSIZE / T_INODO) + pos_inicial;
	bread(bloque, inodos);
	return inodos[(ninodo % (BLOCKSIZE / T_INODO))];
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
	struct superbloque sb;
	bread(posSB, &sb);
	/*unsigned ?? */ int pos_pinodo = sb.posPrimerInodoLibre;
	struct inodo inodoAux;
	inodoAux = leer_inodo(pos_pinodo);
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
	//printf("Primer inodo libre %d",pos_pinodo);
	//printf("\nPrimer inodo libre despues de reservar %d\n",sb.posPrimerInodoLibre);
	sb.cantInodosLibres--;
	bwrite(posSB, &sb);
	return pos_pinodo;
}

int liberar_inodo(unsigned int ninodo)
{
	struct superbloque sb;
	bread(posSB, &sb);
	struct inodo inodo = leer_inodo(ninodo);
	liberar_bloques_inodo(ninodo, 0);
	inodo.tipo = 'l';
	inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
	sb.posPrimerInodoLibre = ninodo;
	sb.cantInodosLibres++;
	escribir_inodo(inodo, ninodo);
	bwrite(posSB, &sb);
	return 0;
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico)
{
	int blog;
	int num_pDirectos = 12;
	int num_punteros = 256;
	int num_punteros2 = num_punteros * num_punteros;
	int num_punteros3 = num_punteros2 * num_punteros;
	int bufferIndirectos[num_punteros];
	int bufferIndirectos0[num_punteros];
	int bufferIndirectos1[num_punteros];
	int bufferIndirectos2[num_punteros];
	struct inodo in;
	int indirectos0 = 0;
	int indirectos1 = 0;
	int indirectos2 = 0;
	in = leer_inodo(ninodo);
	int pbloque[3] = { 0, 0, 0 };
	int ultimoBloque = in.tamEnBytesLog / BLOCKSIZE;
	blog = blogico;
	printf("\npDirectos[0]: %d\n", in.punterosDirectos[0]);
	if (blog < num_pDirectos) {
		for (blog; blog < num_pDirectos; blog++) {
//                      printf("blog for %d\n",in.punterosDirectos[blog]);
			if (in.punterosDirectos[blog] > 0) {
				printf("BLOG dentro : %d\n",
				       in.punterosDirectos[blog]);
				liberar_bloque(blog);
				in.punterosDirectos[blog] = 0;
				in.numBloquesOcupados--;
			}
		}
		pbloque[0] = 0;
		pbloque[1] = 0;
		pbloque[2] = 0;
		//Tenemos que hacer 3 llamadas si existe el bloque de punteros
		if (in.punterosIndirectos[0] > 0) {
			printf("liberarr %d\n", in.punterosIndirectos[0]);
			bread(in.punterosIndirectos[0], bufferIndirectos0);
			liberar(bufferIndirectos0, 1, &in, pbloque);
			in.punterosIndirectos[0] = 0;
		}
		if (in.punterosIndirectos[1] > 0) {
			printf("Liberarr %d\n", in.punterosIndirectos[1]);
			bread(in.punterosIndirectos[1], bufferIndirectos1);
			liberar(bufferIndirectos1, 2, &in, pbloque);
			in.punterosIndirectos[1] = 0;
		}
		if (in.punterosIndirectos[2] > 0) {
			printf("LIBErarRR %d\n", in.punterosIndirectos[2]);
			bread(in.punterosIndirectos[1], bufferIndirectos2);
			liberar(bufferIndirectos2, 3, &in, pbloque);
			in.punterosIndirectos[2] = 0;
		}

	} else if (blog < num_pDirectos + num_punteros) {	//Si está en los punteroindirectos0
		printf("indirectos0\n");
		pbloque[0] = blog - num_pDirectos;
		if (in.punterosIndirectos[0] != 0) {
			bread(in.punterosIndirectos[0], bufferIndirectos0);
			//puede que tengamos que conservar algunos punteros si son < blog - num_pDirectos
			liberar(bufferIndirectos0, 1, &in, pbloque);
			in.punterosIndirectos[0] = 0;
		}
		pbloque[0] = 0;
		if (in.punterosIndirectos[1] != 0) {
			bread(in.punterosIndirectos[1], bufferIndirectos1);
			liberar(bufferIndirectos1, 2, &in, pbloque);
			in.punterosIndirectos[1] = 0;
		}
		if (in.punterosIndirectos[2] != 0) {
			bread(in.punterosIndirectos[1], bufferIndirectos2);
			liberar(bufferIndirectos2, 3, &in, pbloque);
			in.punterosIndirectos[2] = 0;
		}
	} else if (blog < num_pDirectos + num_punteros + num_punteros2) {	//Si está en indirectos1
		printf("indirectos1\n");
		pbloque[0] =
		    (blog - (num_pDirectos + num_punteros)) % num_punteros;
		pbloque[1] =
		    (blog - (num_pDirectos + num_punteros)) / num_punteros;
		indirectos1 = 1;
		if (in.punterosIndirectos[1] != 0) {
			bread(in.punterosIndirectos[1], bufferIndirectos1);
			liberar(bufferIndirectos1, 2, &in, pbloque);
			in.punterosIndirectos[1] = 0;
		}
		pbloque[0] = 0, pbloque[1] = 0;
		if (!in.punterosIndirectos[2]) {
			bread(in.punterosIndirectos[1], bufferIndirectos2);
			liberar(bufferIndirectos2, 3, &in, pbloque);
			in.punterosIndirectos[2] = 0;
		}

	} else if (blog < num_pDirectos + num_punteros + num_punteros3) {	//Si está en indirectos2
		printf("indirectos2\n");

		indirectos2 = 1;
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

		if (in.punterosIndirectos[2] != 0) {
			bread(in.punterosIndirectos[1], bufferIndirectos2);
			liberar(bufferIndirectos2, 3, &in, pbloque);
			in.punterosIndirectos[2] = 0;
		}
		escribir_inodo(in, ninodo);
		return 0;
	}
/*        if (!in.punterosIndirectos[0]) {
                bread(in.punterosIndirectos[0], bufferIndirectos0);
                liberar(bufferIndirectos0, 1, &in, pbloque);
                in.punterosIndirectos[0] = 0;
        }
	if(indirectos1) {
                pbloque[0] =
                    (blog - (num_pDirectos + num_punteros)) % num_punteros;
                pbloque[1] =
                    (blog - (num_pDirectos + num_punteros)) / num_punteros;
	}
	else
		pbloque[0] = 0; pbloque[1]=0;
	if (!in.punterosIndirectos[1]) {
		bread(in.punterosIndirectos[1], bufferIndirectos1);
		liberar(bufferIndirectos1, 2, &in, pbloque);
		in.punterosIndirectos[1] = 0;
	}
        if(indirectos2) {
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
        }
	if (!in.punterosIndirectos[2]) {
		bread(in.punterosIndirectos[1], bufferIndirectos2);
		liberar(bufferIndirectos2, 3, &in, pbloque);
		in.punterosIndirectos[2] = 0;
	} EO*/
	escribir_inodo(in, ninodo);
	return 0;
}

int liberar(unsigned int buffer[256], unsigned int nivel, struct inodo *in,
	    int *pbloque)
{
	unsigned int bufferAux[256];
	if (nivel > 0) {
		int i;
		for (i = 0; i < 256; i++) {
			if (nivel > 1 && buffer[i] != 0) {
				bread(buffer[i], bufferAux);	//leemos el puntero que apunta al bloque y lo guardamos.
				liberar(bufferAux, nivel - 1, in, pbloque);
				buffer[i] = 0;
				liberar_bloque(buffer[i]);
				in->numBloquesOcupados--;
			} else if (nivel == 1 && buffer[i] != 0 && i >= pbloque[0]) {	//nivel 1
				liberar_bloque(buffer[i]);
				buffer[i] = 0;
				in->numBloquesOcupados--;
			}
		}
	}
// liberar_bloque(buffer[0]);
}

int liberar_bloques_inodo1(unsigned int ninodo, unsigned int blogico)
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
						liberar_bloque
						    (inodo.punterosIndirectos
						     [0]);
						inodo.punterosIndirectos[0] = 0;
					} else	//si no, escribimos el bloque de indirectos 0 modificado.
						bwrite(inodo.punterosIndirectos
						       [0], bufferIndirectos0);
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
							liberar_bloque
							    (inodo.punterosIndirectos
							     [1]);
							inodo.punterosIndirectos
							    [1] = 0;
						} else	//si no, escribimos el bloque de indirectos 1 modificado.
							bwrite
							    (inodo.punterosIndirectos
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
							inodo.numBloquesOcupados--;

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
								    (inodo.punterosIndirectos
								     [2]);
								inodo.punterosIndirectos
								    [2] = 0;
							} else	//si no, escribimos el bloque de indirectos 2 modificado.
								bwrite
								    (inodo.punterosIndirectos
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

int liberar_bloques_inodo2(unsigned int ninodo, unsigned int blogico)
{
	struct inodo in;

	int i, blog, punt0, punt1, punt2;
	int num_pDirectos = 12;
	int num_punteros = 256;
	int num_punteros2 = num_punteros * num_punteros;
	int num_punteros3 = num_punteros2 * num_punteros;
	int bufferIndirectos[num_punteros];
	int bufferIndirectos0[num_punteros];
	int bufferIndirectos1[num_punteros];
	int bufferIndirectos2[num_punteros];

	in = leer_inodo(ninodo);
	int ultimoBloque = in.tamEnBytesLog / BLOCKSIZE;

	for (i = 0; i < num_punteros; i++) {	//Inicializa punteros
		bufferIndirectos[i] = 0;
	}

	for (blog = blogico; blog <= ultimoBloque; blog++) {	//Recorre y libera los bloques
		if (blog < num_pDirectos) {	//Si está en los bloques directos
			if (in.punterosDirectos[blog] > 0) {	//Si existe libera el bloque
				liberar_bloque(blog);
				in.punterosDirectos[blog] = 0;
				in.numBloquesOcupados--;
			}
		} else if (blog < num_pDirectos + num_punteros) {	//Si está en los punteroindirectos0
			if (in.punterosIndirectos[0] == 0) {	//No existe indirectos0
				return -1;
			} else {
				bread(in.punterosIndirectos[0],
				      bufferIndirectos0);
				if (bufferIndirectos0[blog - num_pDirectos] == 0) {	//No existe el bloque
					return -1;
				} else {	//Libera el bloque de datos y actualiza indirectos0
					liberar_bloque(bufferIndirectos0
						       [blog - num_pDirectos]);
					bufferIndirectos0[blog - num_pDirectos]
					    = 0;
					in.numBloquesOcupados--;
					if (memcmp(bufferIndirectos0, bufferIndirectos, num_punteros) == 0) {	//Si indirectos0 está vacío lo libera
						liberar_bloque
						    (in.punterosIndirectos[0]);
						in.punterosIndirectos[0]
						    = 0;
					} else {
						bwrite(in.punterosIndirectos[0], bufferIndirectos0);	//Guarda indirectos0 modificado
					}
				}
			}
		} else if (blog < num_pDirectos + num_punteros + num_punteros2) {	//Si está en indirectos1
			punt0 =
			    (blog -
			     (num_pDirectos + num_punteros)) % num_punteros;
			punt1 =
			    (blog -
			     (num_pDirectos + num_punteros)) / num_punteros;
			if (in.punterosIndirectos[1] == 0) {	//No existe indirectos1
				return -1;
			} else {
				bread(in.punterosIndirectos[1],
				      bufferIndirectos1);
				if (bufferIndirectos1[punt1] == 0) {	//No existe array de indirectos0
					return -1;
				} else {
					bread(bufferIndirectos1[punt1],
					      bufferIndirectos0);
					if (bufferIndirectos0[punt0] == 0) {	//No existe bloque
						return -1;
					} else {	//Libera el bloque de datos y actualiza indirectos1 y 0
						liberar_bloque(bufferIndirectos0[punt0]);	//Libera bloque de datos
						bufferIndirectos0[punt0]
						    = 0;
						in.numBloquesOcupados--;
						if (memcmp(bufferIndirectos0, bufferIndirectos, num_punteros) == 0) {	//Si vacío                                                  libera indirectos0
							liberar_bloque
							    (bufferIndirectos1
							     [punt1]);
							bufferIndirectos1[punt1]
							    = 0;
						} else {	//Escribe indirectos0 modificado
							bwrite
							    (bufferIndirectos1
							     [punt1],
							     bufferIndirectos0);
						}
						if (memcmp(bufferIndirectos1, bufferIndirectos, num_punteros) == 0) {	//Si vacío                                                  libera indirectos1
							liberar_bloque
							    (in.punterosIndirectos
							     [1]);
							in.punterosIndirectos[1]
							    = 0;
						} else {	//Escribe indirectos1 modificado
							bwrite
							    (in.punterosIndirectos
							     [1],
							     bufferIndirectos1);
						}
					}
				}
			}
		} else if (blog < num_pDirectos + num_punteros + num_punteros2 + num_punteros3) {	//Si está en indirectos2
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
			if (in.punterosIndirectos[2] == 0) {	//Si no existe indirectos2
				return -1;
			} else {
				bread(in.punterosIndirectos[2],
				      bufferIndirectos2);
				if (bufferIndirectos2[punt2] == 0) {	//Si no existe indirectos1
					return -1;
				} else {
					bread(bufferIndirectos2[punt2],
					      bufferIndirectos1);
					if (bufferIndirectos1[punt1] == 0) {	//Si no existe indirectos0
						return -1;
					} else {
						bread(bufferIndirectos1
						      [punt1],
						      bufferIndirectos0);
						if (bufferIndirectos0[punt0] == 0) {	//Si no existe el bloque
							return -1;
						} else {
							liberar_bloque(bufferIndirectos0[punt0]);	//Libera el bloque
							bufferIndirectos0[punt0]
							    = 0;
							in.numBloquesOcupados--;
							if (memcmp(bufferIndirectos0, bufferIndirectos, num_punteros) == 0) {	//Si vacío libera                                           indirectos0
								liberar_bloque
								    (bufferIndirectos1
								     [punt1]);
								bufferIndirectos1
								    [punt1]
								    = 0;
							} else {	//Escribe indirectos0 modificado
								bwrite
								    (bufferIndirectos1
								     [punt1],
								     bufferIndirectos0);
							}
							if (memcmp(bufferIndirectos1, bufferIndirectos, num_punteros) == 0) {	//Si vacío libera                                           indirectos1
								liberar_bloque
								    (bufferIndirectos2
								     [punt2]);
								bufferIndirectos2
								    [punt2]
								    = 0;
							} else {	//Escribe indirectos1 modificado
								bwrite
								    (bufferIndirectos2
								     [punt2],
								     bufferIndirectos1);
							}
							if (memcmp(bufferIndirectos2, bufferIndirectos, num_punteros) == 0) {	//Si vacío libera                                           indirectos2
								liberar_bloque
								    (in.punterosIndirectos
								     [2]);
								in.punterosIndirectos[2] = 0;
							} else {	//Escribe indirectos2 modificado
								bwrite
								    (in.punterosIndirectos
								     [2],
								     bufferIndirectos2);
							}
						}
					}
				}
			}
		}
	}
	escribir_inodo(in, ninodo);	//Escribe inodo modificado
	return 0;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico,
			  unsigned int *bfisico, char reservar)
{
	int pdir, npun, npun2, npun3, i, punt0, punt1, punt2;
	struct inodo in;

	pdir = 12;
	npun = 256;
	unsigned int bufferIndirectos0[npun];
	unsigned int bufferIndirectos1[npun];
	unsigned int bufferIndirectos2[npun];
	npun2 = npun * npun;
	npun3 = npun * npun2;
	int dentrosc = 0;

	in = leer_inodo(ninodo);

	if (blogico < pdir) {	//Punteros directos
		switch (reservar) {
		case '0':	//Lectura
			if (in.punterosDirectos[blogico] == 0) {	//No existe
				return -1;
			} else {
				*bfisico = in.punterosDirectos[blogico];	//Devuelve el bloque fisico
				return 0;
			}
			break;
		case '1':	//Escritura
			if (in.punterosDirectos[blogico] == 0) {	//Si no existe reserva uno nuevo
				//waitSem(sem);
				in.punterosDirectos[blogico] =
				    reservar_bloque();
				in.numBloquesOcupados++;
				*bfisico = in.punterosDirectos[blogico];
				escribir_inodo(in, ninodo);
				//signalSem(sem);
			} else {
				*bfisico = in.punterosDirectos[blogico];	//Devuelve el bloque físico
			}
			break;
		default:	//Número entrado por error
			return -2;
			break;
		}
	} else if (blogico < (pdir + npun)) {	//Punteros indirectos 0
		switch (reservar) {
		case '0':	//Lectura
			if (in.punterosIndirectos[0] == 0) {	//No existe
				return -1;
			} else {
				if (bread
				    (in.punterosIndirectos[0],
				     bufferIndirectos0) == -1) {
					return -1;	/*no alcanzando */
				}
				if (bufferIndirectos0[blogico - pdir] == 0) {
					return -1;
				} else {
					*bfisico = bufferIndirectos0[blogico - pdir];	//Devuelve el bloque físico
					return 0;
				}
			}
			break;
		case '1':	//Escritura
			dentrosc = 0;
			if (in.punterosIndirectos[0] == 0) {	//Si no existe crea array de punteros0 y bloque final
				for (i = 0; i < npun; i++) {
					bufferIndirectos0[i] = 0;
				}
				//waitSem(sem);
				dentrosc = 1;
				bufferIndirectos0[blogico - pdir] =
				    reservar_bloque();
				in.numBloquesOcupados++;
				in.punterosIndirectos[0] = reservar_bloque();
				in.numBloquesOcupados++;
				bwrite(in.punterosIndirectos[0],
				       bufferIndirectos0);
				*bfisico = bufferIndirectos0[blogico - pdir];	//Devuelve el bloque físico
			} else {
				//waitSem(sem);
				dentrosc = 1;
				bread(in.punterosIndirectos[0],
				      bufferIndirectos0);
				if (bufferIndirectos0[blogico - pdir] == 0) {
					bufferIndirectos0[blogico -
							  pdir] =
					    reservar_bloque();
					in.numBloquesOcupados++;
					bwrite(in.punterosIndirectos[0], bufferIndirectos0);	//Escribimos buffer modificado
					*bfisico =
					    bufferIndirectos0[blogico - pdir];
				} else {
					*bfisico = bufferIndirectos0[blogico - pdir];	//Devuelve el bloque físico
				}
			}
			escribir_inodo(in, ninodo);	//Escribe el inodo modificado
			//if(dentrosc) //signalSem(sem);
			return 0;
			break;
		default:
			return -2;
			break;
		}
	} else if (blogico < (pdir + npun + npun2)) {	//Punteros indirectos 1
		punt0 = (blogico - (pdir + npun)) % npun;
		punt1 = (blogico - (pdir + npun)) / npun;
		switch (reservar) {
		case '0':	//Lectura
			if (in.punterosIndirectos[1] == 0) {	//No existe array de punteros1
				//printf("No existe array punteros 1");
				return -1;
			} else {
				if (bread(in.punterosIndirectos[1], bufferIndirectos1) == -1) {	/*printf("hola\n"); no alcanzado */
					return -1;
				}
				if (bufferIndirectos1[punt1] == 0) {	//No existe array de punteros0
					//printf("No existe array punteros0\n");
					return -1;
				} else {
					if (bread
					    (bufferIndirectos1[punt1],
					     bufferIndirectos0) == -1) {
						return -1;
					}
					if (bufferIndirectos0[punt0] == 0) {	//No existe bloque de datos
						//printf("No existe bloque datos\n");
						return -1;
					} else {
						//printf("Mira: %d\n",bufferIndirectos0[punt0]);
						*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque físico
					}
				}
			}
			break;
		case '1':	//Escritura
			if (in.punterosIndirectos[1] == 0) {	////Si no existe crea array de punteros0,1 y bloque final
				//waitSem(sem);
				dentrosc = 1;
				in.punterosIndirectos[1] = reservar_bloque();
				in.numBloquesOcupados++;
				for (i = 0; i < npun; i++) {
					bufferIndirectos1[i] = 0;
				}
				bufferIndirectos1[punt1] = reservar_bloque();
				in.numBloquesOcupados++;
				for (i = 0; i < npun; i++) {
					bufferIndirectos0[i] = 0;
				}
				bufferIndirectos0[punt0] = reservar_bloque();
				in.numBloquesOcupados++;
				bwrite(in.punterosIndirectos[1],
				       bufferIndirectos1);
				bwrite(bufferIndirectos1[punt1],
				       bufferIndirectos0);
				//printf("Bloque de datos: %d\n",bufferIndirectos0[punt0]);
				*bfisico = bufferIndirectos0[punt0];	//bloque físico
			} else {
				if (bread(in.punterosIndirectos[1], bufferIndirectos1) == -1) {	/*printf("Hola\n"); no alcanzado */
					return -1;
				}
				if (bufferIndirectos1[punt1] == 0) {	//Si no existe crea array de punteros0 y bloque final
					//waitSem(sem);
					dentrosc = 1;
					bufferIndirectos1[punt1] =
					    reservar_bloque();
					in.numBloquesOcupados++;
					for (i = 0; i < npun; i++) {
						bufferIndirectos0[i] = 0;
					}
					bufferIndirectos0[punt0] =
					    reservar_bloque();
					in.numBloquesOcupados++;
					bwrite(in.punterosIndirectos[1],
					       bufferIndirectos1);
					bwrite(bufferIndirectos1[punt1],
					       bufferIndirectos0);
					*bfisico = bufferIndirectos0[punt0];
				} else {
					if (bread(bufferIndirectos1[punt1], bufferIndirectos0) == -1) {	/*printf("Hola\n"); no alcanzado */
						return -1;
					}
					if (bufferIndirectos0[punt0] == 0) {	//Si no existe reserva bloque
						//waitSem(sem);
						dentrosc = 1;
						bufferIndirectos0[punt0]
						    = reservar_bloque();
						in.numBloquesOcupados++;
						bwrite(bufferIndirectos1
						       [punt1],
						       bufferIndirectos0);
						*bfisico =
						    bufferIndirectos0[punt0];
					} else {
						*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque físico
					}
				}
			}
			escribir_inodo(in, ninodo);	//Escribe bloque modificado
			//if(dentrosc) //signalSem(sem);
			return 0;
			break;
		}
	} else if (blogico < pdir + npun + npun2 + npun3) {	//Punteros indirectos 2
		punt2 = (blogico - (pdir + npun + npun2)) / npun2;
		punt1 = ((blogico - (pdir + npun + npun2)) % npun2) / npun;
		punt0 = ((blogico - (pdir + npun + npun2)) % npun2) % npun;
		switch (reservar) {
		case '0':	//Lectura
			if (in.punterosIndirectos[2] == 0) {	//No existe array de punteros2
				return -1;
			} else {
				bread(in.punterosIndirectos[2], bufferIndirectos2);	//No existe array de punteros1
				if (bufferIndirectos2[punt2] == 0) {
					return -1;
				} else {
					bread(bufferIndirectos2[punt2], bufferIndirectos1);	//No existe array de punteros0
					if (bufferIndirectos1[punt1] == 0) {
						return -1;
					} else {
						bread(bufferIndirectos1[punt1], bufferIndirectos0);	//No existe bloque de                                                                                                        datos
						if (bufferIndirectos0
						    [punt0] == 0) {
							return -1;
						} else {
							*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque de datos
						}
					}
				}
			}
			break;
		case '1':	//Escritura
			if (in.punterosIndirectos[2] == 0) {	//Si no existe crea array de punteros2,1,0 y bloque de datos
				//waitSem(sem);
				dentrosc = 1;
				in.punterosIndirectos[2] = reservar_bloque();
				for (i = 0; i < npun; i++) {
					bufferIndirectos2[i] = 0;
				}
				bufferIndirectos2[punt2] = reservar_bloque();
				for (i = 0; i < npun; i++) {
					bufferIndirectos1[i] = 0;
				}
				bufferIndirectos1[punt1] = reservar_bloque();
				for (i = 0; i < npun; i++) {
					bufferIndirectos0[i] = 0;
				}
				bufferIndirectos0[punt0] = reservar_bloque();
				in.numBloquesOcupados++;
				bwrite(in.punterosIndirectos[2],
				       bufferIndirectos2);
				bwrite(bufferIndirectos2[punt2],
				       bufferIndirectos1);
				bwrite(bufferIndirectos1[punt1],
				       bufferIndirectos0);
				*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque de datos
			} else {
				bread(in.punterosIndirectos[2], bufferIndirectos2);	//Si no existe crea array de                                                                                 punteros2,0 y bloque de datos
				if (bufferIndirectos2[punt2] == 0) {
					//waitSem(sem);
					dentrosc = 1;
					bufferIndirectos2[punt2] =
					    reservar_bloque();
					for (i = 0; i < npun; i++) {
						bufferIndirectos1[i] = 0;
					}
					in.numBloquesOcupados++;
					bufferIndirectos1[punt1] =
					    reservar_bloque();
					for (i = 0; i < npun; i++) {
						bufferIndirectos0[i] = 0;
					}
					in.numBloquesOcupados++;
					bufferIndirectos0[punt0] =
					    reservar_bloque();
					in.numBloquesOcupados++;
					bwrite(in.punterosIndirectos[2],
					       bufferIndirectos2);
					bwrite(bufferIndirectos2[punt2],
					       bufferIndirectos1);
					bwrite(bufferIndirectos1[punt1],
					       bufferIndirectos0);
					*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque de datos
				} else {
					bread(bufferIndirectos2[punt2],
					      bufferIndirectos1);
					if (bufferIndirectos1[punt1] == 0) {	//Si no existe crea array de punteros0 y bloque                                                                                              de datos
						//waitSem(sem);
						dentrosc = 1;
						bufferIndirectos1[punt1]
						    = reservar_bloque();
						in.numBloquesOcupados++;
						for (i = 0; i < npun; i++) {
							bufferIndirectos0
							    [i] = 0;
						}
						bufferIndirectos0[punt0]
						    = reservar_bloque();
						in.numBloquesOcupados++;
						bwrite(bufferIndirectos2
						       [punt2],
						       bufferIndirectos1);
						bwrite(bufferIndirectos1
						       [punt1],
						       bufferIndirectos0);
						*bfisico = bufferIndirectos0[punt0];	//Devuelve el bloque de datos
					} else {
						bread(bufferIndirectos1
						      [punt1],
						      bufferIndirectos0);
						if (bufferIndirectos0[punt0] == 0) {	//Si no existe crea el bloque de datos
							//waitSem(sem);
							dentrosc = 1;
							bufferIndirectos0[punt0]
							    = reservar_bloque();
							in.numBloquesOcupados++;
							bwrite
							    (bufferIndirectos1
							     [punt1],
							     bufferIndirectos0);
							*bfisico =
							    bufferIndirectos0
							    [punt0];
						} else {
							*bfisico = bufferIndirectos0[punt0];	//Devuelve bloque de datos
						}
					}
				}
			}
			//if(dentrosc) //signalSem(sem);
			escribir_inodo(in, ninodo);
			break;
		}
	}
	return 0;
}
