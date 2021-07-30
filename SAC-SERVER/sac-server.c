#include "sac-server.h"

int main() {

	nodoVacio = malloc(sizeof(GFile));
	nodoVacio->state = BUSQUEDA;

	fuseLog = log_create("fuseLog.txt", "fuseLog", 1, LOG_LEVEL_DEBUG);

	t_config* configFS = config_create("configuracionFS.txt");

	rootPath = config_get_string_value(configFS, "PATH");
	log_debug(fuseLog, "ROOT PATH: %s\n", rootPath);

	char* ip = config_get_string_value(configFS, "IP");
	log_debug(fuseLog, "IP: %s\n", ip);

	char* puerto = config_get_string_value(configFS, "PUERTO");
	log_debug(fuseLog, "PUERTO: %s\n", puerto);

	leerHeader();

	cargarBitmapEnMemoria();

	log_debug(fuseLog, "Tamaño del disco en bytes: %d\n", pesoDeFS(rootPath));
	log_debug(fuseLog, "Cantidad de bloques de datos: : %d\n", cantidadDeBloquesDeDatos);
	log_debug(fuseLog, "Bloques ocupados por el bitmap: %d\n", bloquesOcupadosPorBitmap);
	log_debug(fuseLog, "Bloque inicial de tabla de nodos: %d\n", comienzoDeTablaDeNodos);
	log_debug(fuseLog, "Bloque final de tabla de nodos: %d\n", finalDeTablaDeNodos);

	int serverSocket = crear_servidor(ip, puerto);

	if(pthread_mutex_init(&m_bitmap, NULL) != 0 || pthread_mutex_init(&m_nodos, NULL)  != 0){
		printf("Error al inicializar semaforo\n");
	}

	while (true) {

		int* cliente = malloc(sizeof(int));
		*cliente = esperar_cliente(serverSocket);

		if (cliente == -1) {
			log_error(fuseLog, "Error al conectar cliente");
			continue;
		}

		else {

			pthread_t nuevoThread;

			int creado = pthread_create(&nuevoThread, NULL, realizarOperacion, cliente);

			if (creado == 0) {
				printf("Nuevo hilo creado\n");
				pthread_detach(nuevoThread);
			} else {
				printf("Error al crear el hilo: %d\n", creado);
			}
		}
	}

	munmap(contenidoDelBitmap, bloquesOcupadosPorBitmap * BLOCK_SIZE);

	return 0;
}

void* realizarOperacion(int* cliente) {

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	size_t stacksize;
	pthread_attr_getstacksize(&attr, &stacksize);
	printf("Stack size: %d\n", stacksize);

	t_paquete* paqueteSolicitud = malloc(sizeof(t_paquete));

	int recibido = recibirPaquete(*cliente, &paqueteSolicitud);

	while (recibido != 0) {

		//TODO Arreglar
		if (recibido == -1) {
//			log_error(fuseLog, "Error al recibir paquete\n");
			return NULL;
		}

		uint32_t opcode;
		memcpy(&opcode, paqueteSolicitud->payload, sizeof(uint32_t));

		if (opcode == -1) {
//			log_error(fuseLog, "Error en el codigo de operacion\n");
			return NULL;
		}

		datosOperacion* contenido = deserializarPaqueteOperacion(paqueteSolicitud);

		const char* pathDeSolicitud = contenido->path;

		t_paquete* resultadoDeOperacion;

		switch (opcode) {

		case OPEN: {
			resultadoDeOperacion = solicitudApertura(pathDeSolicitud);
			break;
		}
		case READ: {
			resultadoDeOperacion = solicitudLectura(pathDeSolicitud, contenido);
			break;
		}
		case READDIR: {
			resultadoDeOperacion = solicitudListarDirectorio(pathDeSolicitud);
			break;
		}
		case GETATTR: {
			resultadoDeOperacion = solicitudAtributos(pathDeSolicitud);
			break;
		}
		case MKDIR: {
			resultadoDeOperacion = solicitudCrearDirectorio(pathDeSolicitud);
			break;
		}
		case MKNOD: {
			resultadoDeOperacion = solicitudCrearArchivoRegular(
					pathDeSolicitud);
			break;
		}
		case WRITE: {
			resultadoDeOperacion = solicitudEscritura(pathDeSolicitud,
					contenido);
			break;
		}
		case TRUNCATE: {
			resultadoDeOperacion = solicitudTruncate(pathDeSolicitud, contenido);
			break;
		}
		case REMOVE: {
			resultadoDeOperacion = solicitudEliminarArchivo(pathDeSolicitud);
			break;
		}
		case RMDIR: {
			resultadoDeOperacion = solicitudEliminarDirectorio(pathDeSolicitud);
			break;
		}
		case RENAME: {
			resultadoDeOperacion = solicitudRenombrarArchivo(pathDeSolicitud, contenido);
			break;
		}
		}

		if (resultadoDeOperacion == NULL) {
			perror("Resultado de operacion es nulo");
		}
		//TODO Controlar si resultadoDeOperacion == NULL; otra seria crear paquetes predeterminados para cada error
		// y usar el header para informarle esto a sac-cli. Ejemplo un struct t_paquete con header -20 que sea
		// no hay mas espacio, etc

		int* enviado = malloc(sizeof(int));
		*enviado = enviarRespuesta(*cliente, resultadoDeOperacion);

		if (*enviado == -1){
//			log_error(fuseLog, "Error al enviar paquete.\n");
		}

		free(enviado);

		if (resultadoDeOperacion->header > 0) {
			free(resultadoDeOperacion->payload);
		}

		free(resultadoDeOperacion);

		free(contenido->path);

		if (contenido->largoContenidoExtra > 0) {
			free(contenido->contenidoExtra);
		}

		free(contenido);
		free(paqueteSolicitud->payload);
		free(paqueteSolicitud);

		paqueteSolicitud = malloc(sizeof(t_paquete));
		recibido = recibirPaquete(*cliente, &paqueteSolicitud);
	}

	return NULL;
}

void cargarBitmapEnMemoria() {

	unsigned long peso = pesoDeFS(rootPath);

	cantidadDeBloquesDelFS = peso / BLOCK_SIZE;
	int cantidadDeBytesDeBitmap = cantidadDeBloquesDelFS / 8;

	double aux = (double) cantidadDeBytesDeBitmap / BLOCK_SIZE;
	bloquesOcupadosPorBitmap = ceil(aux);

	comienzoDeTablaDeNodos = 1 + bloquesOcupadosPorBitmap;
	finalDeTablaDeNodos = 1024 + comienzoDeTablaDeNodos;
	cantidadDeBloquesDeDatos = cantidadDeBloquesDelFS - finalDeTablaDeNodos;

	int fuseDataFD = open(rootPath, O_RDWR, S_IRUSR | S_IWUSR);

	contenidoDelBitmap = mmap(NULL, bloquesOcupadosPorBitmap * BLOCK_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fuseDataFD, BLOCK_SIZE);

	close(fuseDataFD);

	if (contenidoDelBitmap == (caddr_t) -1) {
		perror("Error al mapear archivo.");
		return;
	}

	bitmap = bitarray_create_with_mode(contenidoDelBitmap,
			bloquesOcupadosPorBitmap * BLOCK_SIZE, MSB_FIRST);
}
