#ifndef CONEXIONES_CLIENTE_H_
#define CONEXIONES_CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sockets.h"
#include "estructurasEnComun.h"

typedef struct paquete_operacion{
	uint32_t header;
	uint32_t largoDePath;
	char* path;
	uint32_t argumento1;
	uint32_t argumento2;
	uint32_t largoContenidoExtra;
	void* contenidoExtra;
}__attribute__((packed)) datosOperacion;


void enviarSolicitud(uint32_t opcode, int conexion, datosOperacion* contenido);
datosOperacion* armarPaqueteOperacion(const char* path, uint32_t arg1, uint32_t arg2, uint32_t largoContenido, void* extra);

int enviarSolicitudApertura(int, const char*);
void* enviarSolicitudLectura(int, const char*, uint32_t seek, uint32_t size);
void* enviarSolicitudLecturaDeDirectorio(int, const char*);
void* enviarSolicitudObtenerAtributos(int, const char*);
int enviarSolicitudCreacionDeArchivoRegular(int conexion, const char* path);
int enviarSolicitudCreacionDeDirectorio(int conexion, const char* path);
void* enviarSolicitudEscritura(int conexion, const char* path, void* buffer, uint32_t seek, uint32_t size);
int enviarSolicitudTruncate(int conexion, const char* path, uint32_t length);
int enviarSolicitudEliminarArchivo(int conexion, const char* path);
int enviarSolicitudEliminarDirectorio(int conexion, const char* path);
int enviarSolicitudRenombrarArchivo(int conexion, char* pathAnterior, char* pathNuevo);

#endif /* CONEXIONES_CLIENTE_H_ */
