#include "utils.h"

FILE* abrirFileSystem() {
	FILE* fuseData = fopen(rootPath, "rb+");
	return fuseData;
}

void reservarBloqueBitmap(int numeroDeNodo) {
	pthread_mutex_lock(&m_bitmap);
	bitarray_set_bit(bitmap, numeroDeNodo);
	pthread_mutex_unlock(&m_bitmap);
}

void liberarBloqueBitmap(int numeroDeNodo) {
	pthread_mutex_lock(&m_bitmap);
	bitarray_clean_bit(bitmap, numeroDeNodo);
	pthread_mutex_unlock(&m_bitmap);
}

int leerBitmap(int numeroDeNodo) {
	pthread_mutex_lock(&m_bitmap);
	int resultado = bitarray_test_bit(bitmap, numeroDeNodo);
	pthread_mutex_unlock(&m_bitmap);
	return resultado;
}

GFile leerNodo(uint32_t offset) {

	FILE* fuseData = abrirFileSystem();

	GFile nodo;
	fseek(fuseData, offset, SEEK_SET);
	fread(&nodo, sizeof(GFile), 1, fuseData);
	fclose(fuseData);

	return nodo;
}

void escribirNodo(GFile nodo, uint32_t offset) {

	FILE* fuseData = abrirFileSystem();

	fseek(fuseData, offset, SEEK_SET);
	fwrite(&nodo, sizeof(GFile), 1, fuseData);

	fclose(fuseData);
}

void inicializarPunterosDeDatos(uint32_t* puntero) {
	memset(puntero, 0, 1024 * sizeof(*puntero));
}

void inicializarPunterosIndirectos(uint32_t* puntero) {
	memset(puntero, 0, 1000 * sizeof(*puntero));
}

char* obtenerNombre(char* pathDeSolicitud) {

	int memoria = strlen(pathDeSolicitud) + 1;

	char* path = malloc(memoria);

	strcpy(path, pathDeSolicitud);

	char* token = strtok(path, "/");
	char* nombre = malloc(GFILENAMELENGTH);

	while (token != NULL) {

		strcpy(nombre, token);

		token = strtok(NULL, "/");

		if (token == NULL)
			break;
	}

	free(path);
	free(token);

	return nombre;
}

char* obtenerPathPadre(char* pathSolicitud) {

	int contador = 0;

	for (int i = 0; i < strlen(pathSolicitud); i++) {
		if (pathSolicitud[i] == '/') {
			contador++;
		}
	}

	if (contador == 1) {
		return "/";
	}

	char* path = string_reverse(pathSolicitud);

	char** array = string_n_split(path, 2, "/");

	char* nombreDelPadre = malloc(strlen(array[1]) + 1);
	char* inverso = string_reverse(array[1]);
	strcpy(nombreDelPadre, inverso);

	free(inverso);
	free(path);
	free(array[0]);
	free(array[1]);
	free(array);

	return nombreDelPadre;
}

int obtenerNumeroDeBloque(uint32_t offset) {
	return offset / BLOCK_SIZE;
}

int obtenerOffsetDeNodoDisponible() {

	pthread_mutex_lock(&m_nodos);

	FILE* fuseData = abrirFileSystem();

	int offset = comienzoDeTablaDeNodos * BLOCK_SIZE;

	GFile* nodo = malloc(sizeof(GFile));
	int contador = 0;

	fseek(fuseData, offset, SEEK_SET);
	fread(nodo, sizeof(GFile), 1, fuseData);

	while (nodo->state != 0) {

		contador++;

		if (contador > 1023) {
			free(nodo);
			return -1;
		}

		offset += BLOCK_SIZE;

		fread(nodo, sizeof(GFile), 1, fuseData);
	}

	free(nodo);

	fclose(fuseData);

	pthread_mutex_unlock(&m_nodos);

	return offset;
}

int32_t obtenerOffsetBloqueDeDatosDisponible() {

	int offset = finalDeTablaDeNodos * BLOCK_SIZE;

	for (int i = finalDeTablaDeNodos; i < cantidadDeBloquesDelFS - 1; i++) {

		if (leerBitmap(i) == 0) {
			reservarBloqueBitmap(i);
			return offset;
		}

		offset += BLOCK_SIZE;
	}

	return -1;
}

void leerHeader() {

	GHeader header;

	FILE* fuseData = abrirFileSystem();

	fread(&header, BLOCK_SIZE, 1, fuseData);

	fclose(fuseData);

	printf("\nIdentificador: %s\n", header.sac);
	printf("Version: %u\n", header.version);
}

unsigned long pesoDeFS(const char* path) {

	struct stat statArchivo;

	if (stat(path, &statArchivo) == 0) {
		return statArchivo.st_size;
	} else {
		return -1;
	}
}
