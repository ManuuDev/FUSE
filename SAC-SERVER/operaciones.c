#include "operaciones.h"

t_paquete* solicitudApertura(const char *path) {
	printf("Abrir: %s\n", path);
	FILE* fuseData = abrirFileSystem();
	int32_t operacionApertura = buscarNodo(path, nodoVacio, fuseData);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &operacionApertura, sizeof(int32_t));

	fclose(fuseData);

	return paquete;
}

t_paquete* solicitudEliminarDirectorio(const char* pathDeSolicitud) {

	printf("Eliminar directorio: %s\n", pathDeSolicitud);

	FILE* fuseData = abrirFileSystem();

	int32_t offsetNodo = buscarNodo(pathDeSolicitud, nodoVacio, fuseData);

	if(offsetNodo == -1){
		offsetNodo = -ENOENT;
	} else {
		GFile archivo = leerNodo(offsetNodo);
		archivo.state = 0;
		escribirNodo(archivo, offsetNodo);
		offsetNodo = 0;
	}

	fclose(fuseData);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &offsetNodo, sizeof(int32_t));

	return paquete;
}

t_paquete* solicitudEliminarArchivo(const char* pathDeSolicitud) {

	printf("Eliminar archivo: %s\n", pathDeSolicitud);

	FILE* fuseData = abrirFileSystem();

	int32_t offsetNodo = buscarNodo(pathDeSolicitud, nodoVacio, fuseData);

	if(offsetNodo == -1){
		offsetNodo = -ENOENT;
	} else {

		GFile archivo = leerNodo(offsetNodo);
		punteroIndirecto punterosDeDatos;
		int32_t offsetPunteroADatos = 0;
		archivo.state = 0;

		for(int i = 0; i < BLKINDIRECT; i++) {

			if(archivo.blk_indirect[i] != 0) {

				offsetPunteroADatos = archivo.blk_indirect[i] * BLOCK_SIZE;
				fseek(fuseData, offsetPunteroADatos, SEEK_SET);
				fread(&punterosDeDatos, sizeof(punteroIndirecto), 1, fuseData);

				for(int j = 0; j < GFILEBYTABLE; j++){

					if(punterosDeDatos.offsetsDeDatos[j] != 0){
						liberarBloqueBitmap(punterosDeDatos.offsetsDeDatos[j]);
					}
				}

				liberarBloqueBitmap(archivo.blk_indirect[i]);
			}
		}

		escribirNodo(archivo, offsetNodo);
		offsetNodo = 0;
	}

	fclose(fuseData);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &offsetNodo, sizeof(int32_t));

	return paquete;
}

int truncar(const char *path, int32_t length) {

	printf("Truncar: %s, length: %d\n", path, length);

	FILE* fuseData = abrirFileSystem();

	int32_t offsetNodo = buscarNodo(path, nodoVacio, fuseData);

	if (offsetNodo == -1) {
		return -1;
	}

	int fd = open(rootPath, O_RDWR);

	lseek(fd, offsetNodo, SEEK_SET);
	lockf(fd, F_LOCK, BLOCK_SIZE);

	GFile archivo = leerNodo(offsetNodo);

	if (length > archivo.file_size) {

		int espacioNecesario = length - archivo.file_size;

		int espacioLibreEnUltimoBloque = BLOCK_SIZE - (archivo.file_size % BLOCK_SIZE);

		if (espacioNecesario < espacioLibreEnUltimoBloque) {

			archivo.file_size = length;
			escribirNodo(archivo, offsetNodo);
			fclose(fuseData);

			return 0;
		} else {

			int bloquesNecesarios = ceil((double) (espacioNecesario - espacioLibreEnUltimoBloque) / BLOCK_SIZE);

			int bloqueDeDondeEmpezar = ceil((double) archivo.file_size / BLOCK_SIZE);

			uint32_t numeroDeBloqueIndirecto = (uint32_t) floor((double) (bloqueDeDondeEmpezar / 1024));

			uint32_t offsetPunteroADatos;

			punteroIndirecto punteroADatos;
			inicializarPunterosDeDatos(punteroADatos.offsetsDeDatos);

			if (archivo.blk_indirect[numeroDeBloqueIndirecto] == 0) {

				int offsetDisponible = obtenerOffsetBloqueDeDatosDisponible();

				if (offsetDisponible == -1) {
					perror("TRUNCATE: No hay mas bloques disponibles");
					fclose(fuseData);
					return -1;
				}

				archivo.blk_indirect[numeroDeBloqueIndirecto] = obtenerNumeroDeBloque(offsetDisponible);

				offsetPunteroADatos = offsetDisponible;

			} else {
				offsetPunteroADatos = archivo.blk_indirect[numeroDeBloqueIndirecto] * BLOCK_SIZE;
				fseek(fuseData, offsetPunteroADatos, SEEK_SET);
				fread(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);
			}

			uint32_t numeroRelativoDeDatos = bloqueDeDondeEmpezar - (numeroDeBloqueIndirecto * 1024);

			int bloquesPendientes = bloquesNecesarios;
			int offsetDisponible = 0;

			while (bloquesPendientes > 0) {

				for (int i = numeroRelativoDeDatos; i < 1024 - numeroRelativoDeDatos; i++) {

					offsetDisponible = obtenerOffsetBloqueDeDatosDisponible();

					if (offsetDisponible == -1) {
						perror("TRUNCATE: No hay espacio disponible");
						fclose(fuseData);
						return -1;
					}

					punteroADatos.offsetsDeDatos[i] = obtenerNumeroDeBloque(offsetDisponible);

					fseek(fuseData, offsetPunteroADatos, SEEK_SET);
					fwrite(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);

					bloquesPendientes--;

					if(bloquesPendientes == 0){
						break;
					}
				}

				if (bloquesPendientes > 0) {

					numeroRelativoDeDatos = 0;
					numeroDeBloqueIndirecto++;

					int bloqueDisponible = obtenerOffsetBloqueDeDatosDisponible();

					if (bloqueDisponible == -1) {
						perror("TRUNCATE: No hay mas bloques disponibles");
						fclose(fuseData);
						return -1;
					}

					archivo.blk_indirect[numeroDeBloqueIndirecto] = obtenerNumeroDeBloque(bloqueDisponible);

					offsetPunteroADatos = bloqueDisponible;
					inicializarPunterosDeDatos(punteroADatos.offsetsDeDatos);
				}
			}

		}

	} else if (length < archivo.file_size) {

		int espacioAReducir = archivo.file_size - length;
		int bloquesAReducir = ceil((double) espacioAReducir / BLOCK_SIZE);
		int bloqueDeDondeEmpezar = floor((double) archivo.file_size / BLOCK_SIZE);

		uint32_t numeroDeBloqueIndirecto = (uint32_t) floor((double) (bloqueDeDondeEmpezar / 1024));
		uint32_t numeroRelativoDeDatos = bloqueDeDondeEmpezar - (numeroDeBloqueIndirecto * 1024);

		punteroIndirecto punteroADatos;

		int32_t offsetPunteroADatos = archivo.blk_indirect[numeroDeBloqueIndirecto] * BLOCK_SIZE;
		fseek(fuseData, offsetPunteroADatos, SEEK_SET);
		fread(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);

		while (bloquesAReducir > 0) {

			for (int i = numeroRelativoDeDatos; i >= 0; i--) {
				liberarBloqueBitmap(punteroADatos.offsetsDeDatos[i]);			
				punteroADatos.offsetsDeDatos[i] = 0;
				bloquesAReducir--;
			}

			if (bloquesAReducir >= 0) {
				liberarBloqueBitmap(archivo.blk_indirect[numeroDeBloqueIndirecto]);
				archivo.blk_indirect[numeroDeBloqueIndirecto] = 0;
				numeroDeBloqueIndirecto--;
				numeroRelativoDeDatos = 1023;
			}
		}

	} else {
		printf("No hay nada para hacer, el length solicitado es igual al tamaño del archivo.\n");
	}

	archivo.file_size = length;
	archivo.m_date = (unsigned) time(NULL);

	escribirNodo(archivo, offsetNodo);

	lockf(fd, F_ULOCK, BLOCK_SIZE);

	fclose(fuseData);

	close(fd);

	return 0;
}

int renombrarArchivo(char* pathAnterior, char* nuevoPath){

	FILE* fuseData = abrirFileSystem();

	int32_t offsetNuevoPath = buscarNodo(nuevoPath, nodoVacio, fuseData);

	if(offsetNuevoPath > 0){
		return -1;
	}

	int32_t offsetNodo = buscarNodo(pathAnterior, nodoVacio, fuseData);

	if(offsetNodo == -1){
		return -ENOENT;
	}

	int fd = open(rootPath, O_RDWR);

	lseek(fd, offsetNodo, SEEK_SET);
	lockf(fd, F_LOCK, BLOCK_SIZE);

	GFile nodo = leerNodo(offsetNodo);

	int32_t offsetPadre = buscarNodo(obtenerPathPadre(nuevoPath), nodoVacio, fuseData);

	if(offsetPadre == -1){
		return -ENOENT;
	}

	GFile nodoPadre = leerNodo(offsetPadre);

	if(nodoPadre.state == REGULAR){
		return -1;
	}

	nodo.parent_dir_block = obtenerNumeroDeBloque(offsetPadre);

	escribirNodo(nodo, offsetNodo);

	fclose(fuseData);

	lockf(fd, F_ULOCK, BLOCK_SIZE);
	close(fd);
	return 0;
}

t_paquete* solicitudRenombrarArchivo(const char* pathDeSolicitud, datosOperacion* contenido){

	printf("Renombrar: %s\n", pathDeSolicitud);

	int32_t operacionRenombrar = renombrarArchivo(pathDeSolicitud, contenido->contenidoExtra);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &operacionRenombrar, sizeof(int32_t));

	return paquete;
}

t_paquete* solicitudTruncate(const char *path, datosOperacion* contenido) {

	int32_t length = contenido->argumento1;

	int32_t resultadoTruncate = truncar(path, length);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &resultadoTruncate, sizeof(int32_t));

	return paquete;
}

t_paquete* solicitudAtributos(const char* path) {

	printf("GETATTR: %s\n", path);

	FILE* fuseData = abrirFileSystem();

	uint32_t offset = buscarNodo(path, nodoVacio, fuseData);

	fclose(fuseData);

	GFile nodo = leerNodo(offset);

	uint32_t tipo = nodo.state;

	if (tipo == 0) {
		perror("El archivo no existe o esta eliminado");
		return NULL;
	}

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->header = (sizeof(uint64_t)  * 2) +  (sizeof(uint32_t) * 2);
	paquete->payload = malloc(paquete->header);

	int offsetPayload = 0;

	memcpy(paquete->payload, &nodo.c_date, sizeof(uint64_t));
	offsetPayload += sizeof(uint64_t);

	memcpy(paquete->payload + offsetPayload, &nodo.m_date, sizeof(uint64_t));
	offsetPayload += sizeof(uint64_t);

	memcpy(paquete->payload + offsetPayload, &tipo, sizeof(uint32_t));
	offsetPayload += sizeof(uint32_t);

	memcpy(paquete->payload + offsetPayload, &nodo.file_size, sizeof(uint32_t));

	return paquete;
}

t_paquete* solicitudListarDirectorio(const char* path) {

	printf("Listar directorio: %s\n", path);

	FILE* fuseData = abrirFileSystem();

	char* listaDeDirectorios = string_new();
	int numeroDeBloqueDirectorio = 0;

	if (strcmp(path, "/") != 0) {
		numeroDeBloqueDirectorio = obtenerNumeroDeBloque(buscarNodo(path, nodoVacio, fuseData));
	}

	uint32_t offset = comienzoDeTablaDeNodos * BLOCK_SIZE;
	GFile* archivo = malloc(sizeof(GFile));

	for (int i = offset; i < finalDeTablaDeNodos * BLOCK_SIZE; i += BLOCK_SIZE) {

		fseek(fuseData, i, SEEK_SET);
		fread(archivo, BLOCK_SIZE, 1, fuseData);

		if (archivo->parent_dir_block == numeroDeBloqueDirectorio && archivo->state != BORRADO) {
			printf("Ruta: %s\n", archivo->fname);
			string_append_with_format(&listaDeDirectorios, "%s,", archivo->fname);
		}
	}

	free(archivo);
	fclose(fuseData);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = strlen(listaDeDirectorios) + 1;
	paquete->payload = listaDeDirectorios;

	return paquete;
}

t_paquete* solicitudLectura(const char* path, datosOperacion* argumentos) {

	uint32_t seek = argumentos->argumento1;
	uint32_t size = argumentos->argumento2;

	printf("Leer archivo: %s, seek:%u, size:%u\n", path, seek, size);

	FILE* fuseData = abrirFileSystem();

	uint32_t offset = buscarNodo(path, nodoVacio, fuseData);

	if (offset == -1) {
		perror("No existe el archivo");
		return NULL;
	}

	GFile nodo = leerNodo(offset);

	void* contenido;

	if (nodo.state == REGULAR) {

		if (nodo.file_size > 0) {

			uint32_t numeroDeBloqueDeDatos = (uint32_t) floor((double) (seek / BLOCK_SIZE));
			uint32_t numeroDeBloqueIndirecto = (uint32_t) floor((double) (numeroDeBloqueDeDatos / 1024));
			uint32_t numeroRelativoDeDatos = numeroDeBloqueDeDatos - (numeroDeBloqueIndirecto * 1024);
			uint32_t offsetInterno = seek - (BLOCK_SIZE * numeroDeBloqueDeDatos);

			int pendiente = MIN(size, nodo.file_size);

			contenido = malloc(pendiente);


			uint32_t offsetContenido = 0;
			punteroIndirecto punteroADatos;

			while (pendiente > 0) {

				if (numeroDeBloqueIndirecto > BLKINDIRECT) {
					perror("El seek no es correcto");
					return NULL;
				}

				uint32_t offsetPunteroADatos = nodo.blk_indirect[numeroDeBloqueIndirecto] * BLOCK_SIZE;

				fseek(fuseData, offsetPunteroADatos, SEEK_SET);
				fread(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);

				for (int i = numeroRelativoDeDatos; i < 1024; i++) {

					int proximaLectura = MIN(pendiente, BLOCK_SIZE);

					void* buffer = malloc(proximaLectura);

					int seekDelBloqueDeDatos = (punteroADatos.offsetsDeDatos[i] * BLOCK_SIZE) + offsetInterno;

					fseek(fuseData, seekDelBloqueDeDatos, SEEK_SET);
					fread(buffer, proximaLectura, 1, fuseData);

					offsetInterno = 0;

					memcpy(contenido + offsetContenido, buffer, proximaLectura);

					free(buffer);

					offsetContenido += proximaLectura;

					pendiente -= proximaLectura;

					if (pendiente <= 0) {
						break;
					}
				}

				if (pendiente > 0) {
					numeroRelativoDeDatos = 0;
					numeroDeBloqueIndirecto++;
				}
			}
		}

		t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->header = MIN(nodo.file_size, size);
		paquete->payload = contenido;

		fclose(fuseData);

		return paquete;

	} else {
//		log_error(fuseLog, "Lectura no posible, no es una archivo regular\n");
		return NULL; //TODO Retornar error correspondiente
	}
}

int crearArchivo(const char* pathDeSolicitud, int tipo) {

	FILE* fuseData = abrirFileSystem();

	int resultadoDeBusqueda = buscarNodo(pathDeSolicitud, nodoVacio, fuseData);

	if (resultadoDeBusqueda != -1) {
		fclose(fuseData);
		return -EEXIST;
	}

	char* nombre = obtenerNombre(pathDeSolicitud);
	char* pathPadre = obtenerPathPadre(pathDeSolicitud);

	GFile archivo;

	if (strcmp(pathPadre, "/") == 0) {
		archivo.parent_dir_block = 0;
	} else {

		int resultadoPadre = buscarNodo(pathPadre, nodoVacio, fuseData);

		if (resultadoPadre == -1) {
			free(nombre);
			free(pathPadre);
			fclose(fuseData);
			return -ENOENT;
		}

		archivo.parent_dir_block = obtenerNumeroDeBloque(resultadoPadre);
	}

	int offset = obtenerOffsetDeNodoDisponible();

	if (offset == -1) {
		perror("No hay nodos disponibles");
		free(nombre);
		free(pathPadre);
		fclose(fuseData);
		return -EDQUOT;
	}

	archivo.state = tipo;
	strcpy(archivo.fname, nombre);
	archivo.file_size = 0;
	archivo.c_date = (unsigned) time(NULL);
	archivo.m_date = archivo.c_date;

	if (tipo == DIRECTORIO) {
		archivo.file_size = sizeof(GFile);
	}

	inicializarPunterosIndirectos(archivo.blk_indirect);

	int fd = open(rootPath, O_RDWR);

	lseek(fd, offset, SEEK_SET);
	lockf(fd, F_LOCK, BLOCK_SIZE);

	escribirNodo(archivo, offset);

	free(nombre);

	fclose(fuseData);

	lockf(fd, F_ULOCK, BLOCK_SIZE);
	close(fd);

	return 0;
}

t_paquete* solicitudCrearDirectorio(const char* pathDeSolicitud) {

	printf("Crear directorio: %s\n", pathDeSolicitud);

	int32_t resultado = crearArchivo(pathDeSolicitud, 2);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &resultado, sizeof(int32_t));

	return paquete;
}

t_paquete* solicitudCrearArchivoRegular(const char* pathDeSolicitud) {

	printf("Crear archivo regular: %s\n", pathDeSolicitud);

	int32_t resultado = crearArchivo(pathDeSolicitud, 1);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &resultado, sizeof(int32_t));

	return paquete;
}

t_paquete* solicitudEscritura(const char* path, datosOperacion* contenido) {

	printf("ESCRIBIENDO EN: %s\n", path);
	int32_t resultado = escribir(path, contenido->contenidoExtra, contenido->argumento1, contenido->argumento2);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header = sizeof(uint32_t);
	paquete->payload = malloc(sizeof(int32_t));

	memcpy(paquete->payload, &resultado, sizeof(int32_t));

	return paquete;
}

int escribir(const char* pathDeSolicitud, void* buffer, uint32_t seek, uint32_t size) {

	FILE* fuseData = abrirFileSystem();

	uint32_t offsetNodo = buscarNodo(pathDeSolicitud, nodoVacio, fuseData);

	if (offsetNodo == -1) {
		perror("No existe el archivo");
		fclose(fuseData);
		return -1;
	}

	int fd = open(rootPath, O_RDWR);

	lseek(fd, offsetNodo, SEEK_SET);
	lockf(fd, F_LOCK, BLOCK_SIZE);

	GFile nodo = leerNodo(offsetNodo);

	if (nodo.state == 2) {
		fclose(fuseData);
		return -1;
	}

	uint32_t numeroDeBloqueDeDatos = (uint32_t) floor((double) (seek / BLOCK_SIZE));
	uint32_t numeroDeBloqueIndirecto = (uint32_t) floor((double) (numeroDeBloqueDeDatos / 1024));
	uint32_t numeroRelativoDeDatos = numeroDeBloqueDeDatos - (numeroDeBloqueIndirecto * 1024);
	uint32_t offsetInterno = seek - (BLOCK_SIZE * numeroDeBloqueDeDatos);

	punteroIndirecto punteroADatos;
	inicializarPunterosDeDatos(punteroADatos.offsetsDeDatos);

	uint32_t offsetPunteroADatos;

	if (nodo.blk_indirect[numeroDeBloqueIndirecto] == 0) {

		int offsetDisponible = obtenerOffsetBloqueDeDatosDisponible();

		if (offsetDisponible == -1) {
			perror("Imposible inicializar el bloque indirecto, no hay mas bloques disponibles");
			fclose(fuseData);
			return -EDQUOT;
		}

		nodo.blk_indirect[numeroDeBloqueIndirecto] = obtenerNumeroDeBloque(offsetDisponible);

		offsetPunteroADatos = offsetDisponible;

	} else {
		offsetPunteroADatos = nodo.blk_indirect[numeroDeBloqueIndirecto] * BLOCK_SIZE;
		fseek(fuseData, offsetPunteroADatos, SEEK_SET);
		fread(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);
	}

	int pendiente = size;
	int offsetBuffer = 0;
	int offsetDisponible = 0;
	int proximaEscritura = 0;
	int seekDeBloqueDeDatos = 0;

	while (pendiente > 0) {

		if (numeroDeBloqueIndirecto > BLKINDIRECT) {
			perror("No hay mas punteros indirectos");
			fclose(fuseData);
			return -EDQUOT;
		}

		for (int i = numeroRelativoDeDatos; i < 1024; i++) {

			if (punteroADatos.offsetsDeDatos[i] == 0) {

				offsetInterno = 0;

				offsetDisponible = obtenerOffsetBloqueDeDatosDisponible();

				if (offsetDisponible == -1) {
					perror("No hay mas bloques disponibles");
					fclose(fuseData);
					return -EDQUOT;
				}

				punteroADatos.offsetsDeDatos[i] = obtenerNumeroDeBloque(offsetDisponible);

				fseek(fuseData, offsetPunteroADatos, SEEK_SET);
				fwrite(&punteroADatos, sizeof(punteroIndirecto), 1, fuseData);
			}

			proximaEscritura = MIN(BLOCK_SIZE - offsetInterno, MIN(pendiente, BLOCK_SIZE));

			seekDeBloqueDeDatos = (punteroADatos.offsetsDeDatos[i] * BLOCK_SIZE) + offsetInterno;

			fseek(fuseData, seekDeBloqueDeDatos, SEEK_SET);
			fwrite(buffer + offsetBuffer, proximaEscritura, 1, fuseData);
			offsetBuffer += proximaEscritura;

			pendiente -= proximaEscritura;

			if (pendiente <= 0) {
				break;
			}
		}

		if (pendiente > 0) {

			numeroRelativoDeDatos = 0;
			numeroDeBloqueIndirecto++;

			int bloqueDisponible = obtenerOffsetBloqueDeDatosDisponible();

			if (bloqueDisponible == -1) {
				perror("Escritura pendiente imposible de ralizar, no hay mas bloques disponibles");
				fclose(fuseData);
				return -EDQUOT;
			}

			nodo.blk_indirect[numeroDeBloqueIndirecto] = obtenerNumeroDeBloque(bloqueDisponible);
			offsetPunteroADatos = bloqueDisponible;

			inicializarPunterosDeDatos(punteroADatos.offsetsDeDatos);
		}
	}

	fclose(fuseData);

	nodo.file_size = seek + size;
	nodo.m_date = (unsigned) time(NULL);

	escribirNodo(nodo, offsetNodo);

	lockf(fd, F_ULOCK, BLOCK_SIZE);

	close(fd);

	return size;
}

int buscarNodo(const char* pathSolicitud, GFile* nodo, FILE* fuseData) {

	char* pathPrefijo = obtenerPathPadre(pathSolicitud);

	char* nombre = obtenerNombre(pathSolicitud);

	if (nodo->state == nodoVacio->state) {

		int offset = comienzoDeTablaDeNodos * BLOCK_SIZE;

		GFile* nodoTemporal = malloc(sizeof(GFile));

		int resultado = 0;

		for (int i = 0; i < 1023; i++) {

			fseek(fuseData, offset + (i * BLOCK_SIZE), SEEK_SET);
			fread(nodoTemporal, sizeof(GFile), 1, fuseData);

			if (strcmp(nodoTemporal->fname, nombre) == 0 && nodoTemporal->state != BORRADO) {

				resultado = buscarNodo(pathPrefijo, nodoTemporal, fuseData);

				if (resultado == 0) {
					free(nombre);
					free(nodoTemporal);
					return offset + (i * BLOCK_SIZE);
				}
			}
		}

		free(nombre);
		free(nodoTemporal);
		return -1;

	} else {

		int offsetPadre = nodo->parent_dir_block * BLOCK_SIZE;

		if (offsetPadre == 0 && strcmp("/", pathSolicitud) == 0) {
			free(nombre);
			return 0;
		}

		if (offsetPadre == 0 && strcmp("/", pathSolicitud) != 0) {
			free(nombre);
			return -1;
		}

		if(offsetPadre != 0 && strcmp("/", pathSolicitud) == 0) {
			free(nombre);
			return -1;
		}

		fseek(fuseData, offsetPadre, SEEK_SET);

		fread(nodo, sizeof(GFile), 1, fuseData);

		if (strcmp(nodo->fname, nombre) == 0) {
			free(nombre);
			return buscarNodo(pathPrefijo, nodo, fuseData);
		} else {
			free(nombre);
			return -1;
		}
	}
}
