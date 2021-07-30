#include "conexiones-server.h"

datosOperacion* deserializarPaqueteOperacion(t_paquete* paqueteSolicitud) {

	datosOperacion* contenido = malloc(sizeof(datosOperacion));

	int seek = sizeof(uint32_t);

	memcpy(&(contenido->largoDePath), paqueteSolicitud->payload + seek, sizeof(uint32_t));
	seek += sizeof(uint32_t);

	contenido->path = malloc(contenido->largoDePath);

	memcpy(contenido->path, paqueteSolicitud->payload + seek, contenido->largoDePath);
	seek += contenido->largoDePath;

	memcpy(&(contenido->argumento1), paqueteSolicitud->payload + seek, sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(&(contenido->argumento2), paqueteSolicitud->payload + seek, sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(&(contenido->largoContenidoExtra), paqueteSolicitud->payload + seek, sizeof(uint32_t));
	seek += sizeof(uint32_t);

	if(contenido->largoContenidoExtra > 0) {
		contenido->contenidoExtra = malloc(contenido->largoContenidoExtra);
		memcpy(contenido->contenidoExtra, paqueteSolicitud->payload + seek, contenido->largoContenidoExtra);
	}

	return contenido;
}

int enviarRespuesta(int cliente, t_paquete* resultadoDeOperacion) {

	int pesoPaquete = sizeof(uint32_t) + resultadoDeOperacion->header;

	void* paqueteParaEnviar = malloc(pesoPaquete);

	memcpy(paqueteParaEnviar, &(resultadoDeOperacion->header), sizeof(uint32_t));

	if(resultadoDeOperacion->header > 0){
		memcpy(paqueteParaEnviar + sizeof(uint32_t), resultadoDeOperacion->payload, resultadoDeOperacion->header);
	}

	int resultadoSend = send(cliente, paqueteParaEnviar, pesoPaquete, 0);

	free(paqueteParaEnviar);

	return resultadoSend;
}

int recibirPaquete(int cliente, t_paquete** paquete) {

	int operacionHeader = recv(cliente, &(*paquete)->header, sizeof(uint32_t), MSG_WAITALL);

	if (operacionHeader == -1)
		return -1;
	if (operacionHeader == 0)
		return 0;

	(*paquete)->payload = malloc((*paquete)->header);
	int operacionPayload = recv(cliente, (*paquete)->payload,(*paquete)->header, MSG_WAITALL);

	if (operacionPayload == -1)
		return -1;
	if (operacionPayload == 0)
		return 0;

	return 1;
}
