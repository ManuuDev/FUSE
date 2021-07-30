#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/bitarray.h>

enum OPCODE {
	OPEN = 0, READ = 1, READDIR = 2, GETATTR = 3, MKDIR = 4, MKNOD = 5, WRITE = 6, TRUNCATE = 7, REMOVE = 8, RMDIR = 9, RENAME = 10
};

enum TIPO {
	BORRADO = 0, REGULAR = 1, DIRECTORIO = 2, BUSQUEDA = 3
};

#define GFILEBYTABLE 1024
#define GFILEBYBLOCK 1
#define GFILENAMELENGTH 71
#define GHEADERBLOCKS 1
#define BLKINDIRECT 1000
#define BLOCK_SIZE 4096

#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))

typedef uint32_t ptrGBloque;

typedef struct sac_header_t { // un bloque
	unsigned char sac[3];
	uint32_t version;
	uint32_t blk_bitmap;
	uint32_t size_bitmap; // en bloques
	unsigned char padding[4081];
} GHeader;

typedef struct sac_file_t {  // un cuarto de bloque (256 bytes)
	uint8_t state; // 0: borrado, 1: archivo, 2: directorio, 3: busqueda
	unsigned char fname[GFILENAMELENGTH];
	uint32_t parent_dir_block;
	uint32_t file_size;
	uint64_t c_date;
	uint64_t m_date;
	ptrGBloque blk_indirect[BLKINDIRECT];
} GFile;

typedef struct puntero_bloque_indirecto {
	ptrGBloque offsetsDeDatos [1024];
} punteroIndirecto;

typedef struct paquete {
	uint32_t header;
	void* payload;
} __attribute__((packed)) t_paquete;

typedef struct paquete_operacion {
	uint32_t header;
	uint32_t largoDePath;
	char* path;
	uint32_t argumento1;
	uint32_t argumento2;
	uint32_t largoContenidoExtra;
	void* contenidoExtra;
}__attribute__((packed)) datosOperacion;

GFile* nodoVacio;

const char* rootPath;
t_log* fuseLog;
t_bitarray* bitmap;
void* contenidoDelBitmap;

int comienzoDeTablaDeNodos;
int finalDeTablaDeNodos;
int bloquesOcupadosPorBitmap;
int cantidadDeBloquesDeDatos;
int cantidadDeBloquesDelFS;

#endif
