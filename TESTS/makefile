HILOLAY := ~/tp-2019-2c-NoCurseCreatividad/Hilolay/
HILOLAY_SO_PATH := $(HILOLAY)/Debug

LIBMUSE := ~/tp-2019-2c-NoCurseCreatividad/muse/
LIBMUSE_SO_PATH := $(LIBMUSE)/Debug



audiencia:
	gcc -o AUDIENCIA audiencia.c -I  $(HILOLAY) -L $(HILOLAY_SO_PATH) -lHilolay

solo: 
	gcc -o SOLO caballeros_de_SisOp_Solo.c -I $(HILOLAY) -L $(HILOLAY_SO_PATH) -lHilolay

afinador:
	gcc -o AFINADOR caballeros_de_SisOp_Afinador.c -I $(HILOLAY) -L $(HILOLAY_SO_PATH) -lHilolay

tests_suse: audiencia solo afinador



archivo_swap_supermasivo:
	gcc -o ARCHIVO_SWAP_SUPERMASIVO archivo_de_swap_supermasivo.c -I $(LIBMUSE) -L $(LIBMUSE_SO_PATH) -lmuse

recursiva: 
	gcc -o RECURSIVA recursiva.c -I $(LIBMUSE) -L $(LIBMUSE_SO_PATH) -lmuse

tests_muse: archivo_swap_supermasivo recursiva



estres_compartido:
	gcc -o ESTRES_COMPARTIDO estres_compartido.c -I $(HILOLAY) -I $(LIBMUSE) -L $(HILOLAY_SO_PATH) -L $(LIBMUSE_SO_PATH) -lHilolay -lmuse

estres_privado:
	gcc -o ESTRES_PRIVADO estres_privado.c -I $(HILOLAY) -I $(LIBMUSE) -L $(HILOLAY_SO_PATH) -L $(LIBMUSE_SO_PATH) -lHilolay -lmuse

revolucion_compartida:
	gcc -o REVOLUCION_COMPARTIDA revolucion_compartida.c -I $(HILOLAY) -I $(LIBMUSE) -L $(HILOLAY_SO_PATH) -L $(LIBMUSE_SO_PATH) -lHilolay -lmuse

revolucion_privada:
	gcc -o REVOLUCION_PRIVADA revolucion_privada.c -I $(HILOLAY) -I $(LIBMUSE) -L $(HILOLAY_SO_PATH) -L $(LIBMUSE_SO_PATH) -lHilolay -lmuse

tests_compartidos: estres_compartido estres_compartido revolucion_compartida revolucion_privada



all: tests_suse tests_muse tests_compartidos

clean: 
	find . -maxdepth 1 -type f -executable -exec rm {} +