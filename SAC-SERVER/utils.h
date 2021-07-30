#ifndef UTILS_H_
#define UTILS_H_

#include "estructuras.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include <commons/string.h>

pthread_mutex_t m_bitmap;
pthread_mutex_t m_nodos;

FILE* abrirFileSystem();
GFile leerNodo(uint32_t offset);
void escribirNodo(GFile nodo, uint32_t offset);

int obtenerOffsetDeNodoDisponible();
int32_t obtenerOffsetBloqueDeDatosDisponible();
int obtenerNumeroDeNodo(int offset);
int obtenerNumeroDeBloque(uint32_t offset);

void inicializarPunterosDeDatos(uint32_t* puntero);
void inicializarPunterosIndirectos(uint32_t* puntero);

char* obtenerPathPadre(char* pathSolicitud);
char* obtenerNombre(char* pathDeSolicitud);

void reservarBloqueBitmap(int numeroDeNodo);
void liberarBloqueBitmap(int numeroDeNodo);

int leerBitmap(int numeroDeNodo);

void leerHeader();
unsigned long pesoDeFS(const char* path);

#endif /* UTILS_H_ */
