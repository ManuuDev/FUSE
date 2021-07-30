#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <commons/string.h>

#include "estructuras.h"
#include "utils.h"

t_paquete* solicitudApertura(const char *path);
t_paquete* solicitudAtributos(const char* path);
t_paquete* solicitudListarDirectorio(const char* path);
t_paquete* solicitudLectura(const char* pathAbsoluto, datosOperacion* contenido);
t_paquete* solicitudCrearArchivoRegular(const char* pathDeSolicitud);
t_paquete* solicitudCrearDirectorio(const char* pathDeSolicitud);
t_paquete* solicitudEscritura(const char* path, datosOperacion* contenido);
t_paquete* solicitudTruncate(const char *path, datosOperacion* contenido);
t_paquete* solicitudEliminarArchivo(const char* pathDeSolicitud);
t_paquete* solicitudEliminarDirectorio(const char* pathDeSolicitud);
t_paquete* solicitudRenombrarArchivo(const char* pathDeSolicitud, datosOperacion* contenido);

int truncar(const char *path, int32_t length);
int escribir(const char* pathDeSolicitud, void* buffer, uint32_t seek, uint32_t size);
int buscarNodo(const char* pathSolicitud, GFile* nodo, FILE* fuseData);
int renombrarArchivo(char* pathAnterior, char* nuevoPath);

#endif
