#ifndef SAC_SERVER_H_
#define SAC_SERVER_H_

#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>

#include "sockets.h"

#include "conexiones-server.h"
#include "estructuras.h"
#include "utils.h"
#include "operaciones.h"

void cargarBitmapEnMemoria();
void* realizarOperacion(int* cliente);

#endif /* SAC_SERVER_H_ */
