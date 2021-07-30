#ifndef ESTRUCTURASENCOMUN_H_
#define ESTRUCTURASENCOMUN_H_

typedef struct paquete{
	uint32_t header;
	char* payload;
}t_paquete;

enum OPCODE {
	OPEN = 0, READ = 1, READDIR = 2, GETATTR = 3, MKDIR = 4, MKNOD = 5, WRITE = 6, TRUNCATE = 7, REMOVE = 8, RMDIR = 9, RENAME = 10
};

enum TIPO {
	BORRADO = 0, REGULAR = 1, DIRECTORIO = 2, BUSQUEDA = 3
};

#endif
