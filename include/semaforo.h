/* fichero semaforo.h */
/*#ifdef DEFINE_SEMAFOROS
#define EXTERN /* nada 
#else
#define EXTERN extern
#endif
EXTERN unsigned inside_sc;*/

int iniciarSem(int valor_inicial, int semilla);
void eliminarSem(int sem);
void signalSem(int sem);
void signalSemSB(int sem_a);
void waitSemSB(int sem_a);
void signalBloque();
void waitSem(int sem);
void waitBloque();
int getvalSem(int sem);
void insideCero();
