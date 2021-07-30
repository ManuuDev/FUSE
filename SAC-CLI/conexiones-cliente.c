#include "conexiones-cliente.h"

int enviarSolicitudApertura(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(OPEN, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

int enviarSolicitudEliminarArchivo(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(REMOVE, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

int enviarSolicitudEliminarDirectorio(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(RMDIR, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

void* enviarSolicitudLectura(int conexion, const char* path, uint32_t seek, uint32_t size) {

	datosOperacion* paquete = armarPaqueteOperacion(path, seek, size, 0, NULL);

	enviarSolicitud(READ, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(uint32_t), MSG_WAITALL);

	void* payload;

	if(header > 0){
		payload = malloc(header);
		recv(conexion, payload, header, MSG_WAITALL);
	}

	void* retorno = malloc(sizeof(uint32_t) + header);

	int seekRetorno = 0;

	memcpy(retorno, &header, sizeof(uint32_t));

	if (header > 0) {
		seekRetorno += sizeof(uint32_t);

		memcpy(retorno + seekRetorno, payload, header);
		free(payload);
	}

	return retorno;
}

int enviarSolicitudTruncate(int conexion, const char* path, uint32_t length) {

	datosOperacion* paquete = armarPaqueteOperacion(path, length, 0, 0, NULL);

	enviarSolicitud(TRUNCATE, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

void* enviarSolicitudEscritura(int conexion, const char* path, void* buffer, uint32_t seek, uint32_t size) {

	datosOperacion* paquete = armarPaqueteOperacion(path, seek, size, size, buffer);

	enviarSolicitud(WRITE, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(uint32_t), MSG_WAITALL);

	void* payload = malloc(header);
	recv(conexion, payload, header, MSG_WAITALL);

	void* retorno = malloc(sizeof(uint32_t) + header);

	int seekRetorno = 0;

	memcpy(retorno, &header, sizeof(uint32_t));
	seekRetorno += sizeof(uint32_t);

	memcpy(retorno + seekRetorno, payload, sizeof(uint32_t));

	free(payload);
	return retorno;
}

void* enviarSolicitudLecturaDeDirectorio(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(READDIR, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(uint32_t), MSG_WAITALL);

	if (header > 0) {
		void* payload = malloc(header);
		recv(conexion, payload, header, MSG_WAITALL);
		return payload;
	} else {
		return NULL;
	}
}

void* enviarSolicitudObtenerAtributos(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(GETATTR, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(uint32_t), MSG_WAITALL);

	void* payload = malloc(header);
	recv(conexion, payload, header, MSG_WAITALL);

	return payload;
}

int enviarSolicitudCreacionDeDirectorio(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(MKDIR, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

int enviarSolicitudCreacionDeArchivoRegular(int conexion, const char* path) {

	datosOperacion* paquete = armarPaqueteOperacion(path, 0, 0, 0, NULL);

	enviarSolicitud(MKNOD, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

int enviarSolicitudRenombrarArchivo(int conexion, char* pathAnterior, char* pathNuevo){

	datosOperacion* paquete = armarPaqueteOperacion(pathAnterior, 0, 0, strlen(pathNuevo) + 1, pathNuevo);

	enviarSolicitud(RENAME, conexion, paquete);

	uint32_t header;
	recv(conexion, &header, sizeof(int32_t), MSG_WAITALL);

	int32_t resultado;
	recv(conexion, &resultado, header, MSG_WAITALL);

	return resultado;
}

datosOperacion* armarPaqueteOperacion(const char* path, uint32_t arg1, uint32_t arg2, uint32_t largoContenido, void* extra) {

	datosOperacion* paquete = malloc(sizeof(datosOperacion));

	paquete->largoDePath = strlen(path) + 1;

	paquete->path = path;

	paquete->argumento1 = arg1;

	paquete->argumento2 = arg2;

	paquete->largoContenidoExtra = largoContenido;

	if (largoContenido > 0) {
		paquete->contenidoExtra = malloc(largoContenido);
		memcpy(paquete->contenidoExtra, extra, largoContenido);
	}

	int pesoDeArgumentos = 2 * sizeof(uint32_t);

	paquete->header = sizeof(uint32_t) + paquete->largoDePath + pesoDeArgumentos + sizeof(uint32_t) + largoContenido;

	return paquete;
}

void enviarSolicitud(uint32_t opcode, int conexion, datosOperacion* datos) {

	t_paquete* paqueteEnvio = malloc(sizeof(t_paquete));

	paqueteEnvio->header = sizeof(uint32_t) + datos->header;
	paqueteEnvio->payload = malloc(paqueteEnvio->header);

	int seek = 0;

	memcpy(paqueteEnvio->payload, &opcode, sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(paqueteEnvio->payload + seek, &(datos->largoDePath), sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(paqueteEnvio->payload + seek, datos->path, datos->largoDePath);
	seek += datos->largoDePath;

	memcpy(paqueteEnvio->payload + seek, &(datos->argumento1), sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(paqueteEnvio->payload + seek, &(datos->argumento2), sizeof(uint32_t));
	seek += sizeof(uint32_t);

	memcpy(paqueteEnvio->payload + seek, &(datos->largoContenidoExtra), sizeof(uint32_t));
	seek += sizeof(uint32_t);

	if(datos->largoContenidoExtra > 0){
		memcpy(paqueteEnvio->payload + seek, datos->contenidoExtra, datos->largoContenidoExtra);
	}

	int operacionHeader = send(conexion, &paqueteEnvio->header, sizeof(uint32_t), 0);
	int operacionPayload = send(conexion, paqueteEnvio->payload, paqueteEnvio->header, 0);

	if(datos->largoContenidoExtra > 0){
		free(datos->contenidoExtra);
	}

	free(datos);

	free(paqueteEnvio->payload);
	free(paqueteEnvio);

	if (operacionHeader == -1 || operacionPayload == -1) {
		printf("Error al enviar paquete");
		return;
	}
}
