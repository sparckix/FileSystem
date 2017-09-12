main: bloques.o ficheros.o directorios.o semaforo.o lectores_escritores.o mi_mkfs.o mi_chmod.o leer_sf.o mi_ls.o mi_cat.o mi_stat.o mi_ln.o mi_rm.o simulacion.o verificacion.o mi_mkdir.o mi_touch.o mi_rename.o ficheros_basico.o
	gcc -Wall -O -o mi_mkfs bloques.o ficheros.o directorios.o semaforo.o mi_mkfs.o ficheros_basico.o lectores_escritores.o -g
	gcc -Wall -O -o mi_chmod bloques.o ficheros.o directorios.o semaforo.o mi_chmod.o ficheros_basico.o lectores_escritores.o -g
	gcc -Wall -O -o leer_sf leer_sf.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_ls mi_ls.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_cat mi_cat.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_stat mi_stat.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_ln mi_ln.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_rm mi_rm.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o simulacion simulacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o verificacion verificacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_mkdir mi_mkdir.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_touch mi_touch.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	gcc -Wall -O -o mi_rename mi_rename.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

leer_sf: leer_sf.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o leer_sf leer_sf.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_chmod: mi_chmod.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_chmod mi_chmod.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_ls: mi_ls.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_ls mi_ls.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_cat: mi_cat.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_cat mi_cat.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_stat: mi_stat.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_stat mi_stat.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_ln: mi_ln.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_ln mi_ln.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g
	
mi_rm: mi_rm.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_rm mi_rm.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

mi_mkdir: mi_mkdir.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_mkdir mi_mkdir.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

mi_touch: mi_touch.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_touch mi_touch.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

mi_rename: mi_rename.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o mi_rename mi_rename.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g


simulacion: simulacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o simulacion simulacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

verificacion: verificacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o bloques.o
	gcc -o verificacion verificacion.o ficheros_basico.o ficheros.o directorios.o semaforo.o bloques.o lectores_escritores.o -g

bloques.o: bloques.c ../include/bloques.h
	gcc -c bloques.c -g

ficheros_basico.o: ficheros_basico.c ../include/ficheros_basico.h
	gcc -c ficheros_basico.c -g
	
ficheros.o: ficheros.c ../include/ficheros.h
	gcc -c ficheros.c -g
	
directorios.o: directorios.c ../include/directorios.h
	gcc -c directorios.c -g
	
semaforo.o: semaforo.c ../include/semaforo.h
	gcc -c semaforo.c -g
lectores_escritores.o: lectores_escritores.c ../include/lectores_escritores.h
	gcc -c lectores_escritores.c -g

mi_mkfs.o: mi_mkfs.c ../include/bloques.h
	gcc -c mi_mkfs.c -g
	
leer_sf.o: leer_sf.c ../include/ficheros_basico.h
	gcc -c leer_sf.c -g

mi_chmod.o: mi_chmod.c ../include/directorios.h
	gcc -c mi_chmod.c -g
	
mi_ls.o: mi_ls.c ../include/directorios.h
	gcc -c mi_ls.c -g
	
mi_cat.o: mi_cat.c ../include/directorios.h
	gcc -c mi_cat.c -g
	
mi_stat.o: mi_stat.c ../include/directorios.h
	gcc -c mi_stat.c -g
	
mi_ln.o: mi_ln.c ../include/directorios.h
	gcc -c mi_ln.c -g
	
mi_rm.o: mi_rm.c ../include/directorios.h
	gcc -c mi_rm.c -g
	
mi_touch.o: mi_touch.c ../include/directorios.h
	gcc -c mi_touch.c -g

mi_mkdir.o: mi_mkdir.c ../include/directorios.h
	gcc -c mi_mkdir.c -g

mi_rename.o: mi_rename.c ../include/directorios.h
	gcc -c mi_rename.c -g

simulacion.o: simulacion.c ../include/simulacion.h
	gcc -c simulacion.c -g

verificacion.o: verificacion.c ../include/simulacion.h
	gcc -c verificacion.c -g

.PHONY: clean
clean:
	rm -f mi_mkfs leer_sf mi_chmod mi_ls mi_cat mi_stat mi_ln mi_rm simulacion prueba bloques.o mi_mkfs.o ficheros_basico.o ficheros.o directorios.o semaforo.o lectores_escritores.o leer_sf.o mi_chmod.o mi_ls.o mi_cat.o mi_stat.o mi_ln.o mi_rm.o simulacion.o prueba.o
