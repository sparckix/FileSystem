/* fichero semaforo.c */
#include "../include/semaforo.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

unsigned int inside_sc = 0;
int sem, semBloque, semSB, semMB, semAI;
int iniciarSem(int valor_inicial, int semilla)
{
	int sem1;
	int key = ftok("/bin/ls", semilla);

	sem1 = semget(key, 1, 0600);
	/*      VALOR DEVUELTO
	   Si hubo �xito, el valor devuelto ser� el identificador del conjunto de sem�foros 
	   (un entero no negativo), de otro modo, se devuelve -1 con errno indicando el error.
	   ERRORES
	   En caso de fallo, errno tendr� uno de los siguientes valores:
	   EACCES
	   Existe un conjunto de sem�foros para key, pero el proceso que realiza la llamada 
	   no tiene permisos para acceder al conjunto.
	   EEXIST
	   Existe un conjunto de sem�foros para key y semflg tiene a 1 tanto IPC_CREAT como IPC_EXCL.
	   ENOENT       
	   No existe ning�n conjunto de sem�foros para key y semflg no tiene a 1 IPC_CREAT.
	   EINVAL
	   nsems es menor que 0 o mayor que el l�mite en el n�mero de sem�foros por conjunto de 
	   sem�foros (SEMMSL), o ya existe un conjunto de sem�foros que se corresponde con key , 
	   y nsems es mayor que el n�mero de sem�foros en ese conjunto.
	   ENOMEM
	   Se ha de crear un conjunto de sem�foros, pero el sistema no tiene suficiente 
	   memoria para la nueva estructura de datos.
	   ENOSPC       
	   Se ha de crear un conjunto de sem�foros, pero el l�mite del sistema para 
	   el n�mero m�ximo de conjuntos de sem�foros (SEMMNI), o el n�mero de sem�foros 
	   m�ximo del sistema (SEMMNS), ser�a excedido.
	 */

	if (sem1 < 0 && errno == ENOENT) {
		// No existia antes!!!
		sem1 = semget(key, 1, 0600 | IPC_CREAT);	// 1 es la cantidad de sem�foros
		semctl(sem1, 0, SETVAL, valor_inicial);	//0 es el n� de sem�foro
	}
	return sem1;
}

void eliminarSem(int sem)
{
	semctl(sem, 0, IPC_RMID, 0);
	//IPC_RMID indica al Kernel que debe borrar el conj. de semáforos
	//No tiene efecto mientras exista algún proceso que esté usando los sem.
}

void signalSem(int sem)
{
	inside_sc -= 1;
	if (inside_sc == 0) {
		struct sembuf s;

		s.sem_num = 0;
		//Nª de semáforo dentro del conjunto

		s.sem_op = 1;
		//clase de operación: <0 decrementa (Wait), >0 incrementa (Signal)
		// =0 no se altera

		s.sem_flg = 0;	// modificadores de operación

		semop(sem, &s, 1);
		/* semop realiza operaciones de incremento o decremento con bloqueo
		 *  &s es un puntero al array de operaciones
		 * El 1 indica el nº de elementos que tiene el array de operaciones */
	}
}

void signalSemSB(int sem_a)
{
	struct sembuf s;
	s.sem_num = 0;
	s.sem_op = 1;
	s.sem_flg = 0;

	semop(sem_a, &s, 1);
}

void waitSemSB(int sem_a)
{
	struct sembuf s;
	s.sem_num = 0;
	s.sem_op = -1;
	s.sem_flg = 0;

	semop(sem_a, &s, 1);
}

void waitSem(int sem)
{
	if (inside_sc++ > 0)
		return;
	struct sembuf w;
	w.sem_num = 0;
	w.sem_op = -1;
	w.sem_flg = 0;
	semop(sem, &w, 1);
}

int getvalSem(int sem)
{
	return semctl(sem, 0, GETVAL);
}

void insideCero()
{
	inside_sc = 0;
}
