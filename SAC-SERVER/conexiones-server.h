#ifndef CONEXIONES_SERVER_H_
#define CONEXIONES_SERVER_H_

#include <stdio.h>
#include <stdlib.h>

#include "estructuras.h"
#include "sockets.h"

datosOperacion* deserializarPaqueteOperacion(t_paquete* paqueteSolicitud);
int enviarRespuesta(int cliente, t_paquete* resultadoDeOperacion);
int recibirPaquete(int cliente, t_paquete** paquete);

#endif
