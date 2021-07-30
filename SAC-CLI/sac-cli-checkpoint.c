#include "sac-cli-checkpoint.h"

char* archivosDelSistema [25]= {
		  "libnss_files.so.2", "libpthread.so.0", ".versions.conf", ".rbenv-version",
		  ".ruby-version", ".rbfu-version", "libpcre.so.3", "libnsl.so.1",
		  "libc.so.6", "Gemfile", ".rvmrc", "cmov",
		  "sse2", "i686", ".rvm","tls", ".xdg-volume-info", "autorun.inf",
		  "libselinux.so.1", "libm.so.6", "libdl.so.2", "libnss_nis.so.2",
		  "libnss_compat.so.2", ".Trash-1000", ".Trash"
};

int esArchivoDelSistema(const char* path){

	for(int i = 0; i < 25; i++){

		if(string_contains(path, archivosDelSistema[i])){
			return 1;
		}
	}

	return 0;
}

static int do_getattr(const char* path, struct stat* st) {

	if(strcmp(path, "/") == 0) {
		st->st_mode = S_IFDIR | 0777;
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_nlink = 2;
		return 0;
	}

	if(do_open(path,NULL)==-1) {
		return -ENOENT;
	}

	printf("\nObtener atributos: %s %d\n", path, strlen(path));

	void* atributos = enviarSolicitudObtenerAtributos(conexion, path);

	uint64_t tiempoDeCreacion;
	uint64_t tiempoDeModificacion;
	uint32_t tipoDeArchivo;
	uint32_t peso;

	int offset = 0;

	memcpy(&tiempoDeCreacion, atributos, sizeof(uint64_t));
	offset += sizeof(uint64_t);

	memcpy(&tiempoDeModificacion, atributos + offset, sizeof(uint64_t));
	offset += sizeof(uint64_t);

	memcpy(&tipoDeArchivo, atributos + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	st->st_uid = fuse_get_context()->uid;
	st->st_gid = fuse_get_context()->gid;

	st->st_atime = time( NULL);
	st->st_ctime = tiempoDeCreacion;
	st->st_mtime = tiempoDeModificacion;

	if (tipoDeArchivo == DIRECTORIO) {
		printf("Tipo: DIRECTORIO\n");
		st->st_mode = S_IFDIR | 0777;
		st->st_nlink = 2;
		st->st_size = 4096;
	} else if (tipoDeArchivo == REGULAR) {
		memcpy(&peso, atributos + offset, sizeof(uint32_t));
		printf("Tipo: REGULAR ; Peso:%d\n", peso);
		st->st_mode = S_IFREG | 0777;
		st->st_nlink = 1;
		st->st_size = peso;
	} else {
		printf("No se reconoce el tipo.\n");
		return -ENOENT;
	}

	free(atributos);

	return 0;
}

static int do_readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	printf("\nLeer directorio: %s\n", path);

	if(do_open(path,NULL)==-1) {
		return -ENOENT;
	}

	void* payload = enviarSolicitudLecturaDeDirectorio(conexion, path);

	filler( buffer, ".", NULL, 0 );
	filler( buffer, "..", NULL, 0 );

	if (payload == NULL){
		return 0;
	}

	char* token = strtok((char*) payload, ",");

	while (token != NULL) {
		printf("%s\n", token);
		char nombre [strlen(token) + 1];
		strcpy(nombre, token);
		filler(buffer, nombre, NULL, 0);
		token = strtok(NULL, ",");
	}

	free(payload);

	return 0;
}

static int do_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi) {

	printf("\nLeer: %s\nOffset:%lld\nSize:%d\n", path, offset, size);

	if(do_open(path,NULL)==-1) {
		return -1;
	}

	void* contenido = enviarSolicitudLectura(conexion, path, offset, size);

	uint32_t leido;
	memcpy(&leido, contenido, sizeof(uint32_t));

	if(leido > 0){
		memcpy(buffer, contenido + sizeof(uint32_t), leido);
	}

	free(contenido);

	return leido;
}

static int do_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *info) {

	printf("\nEscribir: %s\nOffset:%lld\nSize:%d\n", path, offset, size);

	if(do_open(path,NULL)==-1) {
		return -1;
	}

	void* contenido = enviarSolicitudEscritura(conexion, path, buffer, offset, size);

	uint32_t header;
	memcpy(&header, contenido, sizeof(uint32_t));

	uint32_t escrito;
	memcpy(&escrito, contenido + sizeof(uint32_t), sizeof(uint32_t));

	free(contenido);
	printf("Escrito: %u\n", escrito);
	return escrito;
}

static int do_mkdir(const char* path, mode_t mode) {
	printf("\nCrear directorio: %s\n", path);
	return enviarSolicitudCreacionDeDirectorio(conexion, path);
}

static int do_mknod(const char* path, mode_t mode, dev_t rdev) {
	printf("\nCrear archivo: %s\n", path);
	return enviarSolicitudCreacionDeArchivoRegular(conexion, path);
}

static int do_truncate(const char* path, off_t length) {

	printf("Truncar: %s\n", path);

	if(((length/1024)/1024) > 4000){
		printf("El tamaño es mas grande de lo permitido");
		return -EFBIG;
	}

	struct stat atributos;

	int resultadoAtributos = do_getattr(path, &atributos);

	if(resultadoAtributos == -ENOENT){
		printf("El archivo no existe");
		return -ENOENT;
	}else if(atributos.st_mode == (S_IFDIR | 0777)){
		printf("El archivo es un directorio");
		return -EISDIR;
	}

	return enviarSolicitudTruncate(conexion, path, length);
}

static int do_open(const char *path, struct fuse_file_info *fi) {

	if(esArchivoDelSistema(path)){
		return -1;
	}

	if(strcmp(path, "/") == 0) {
		return 0;
	}

	printf("\nAbrir: %s\n", path);

	int32_t resultado = enviarSolicitudApertura(conexion, path);

	if(resultado >= 0)
		return 0;
	else
		return -1;
}

static int do_unlink(const char *path) {
	printf("Eliminar archivo: %s\n", path);
	return enviarSolicitudEliminarArchivo(conexion, path);
}

static int do_rmdir(const char *path) {
	printf("Eliminar directorio: %s\n", path);
	return enviarSolicitudEliminarDirectorio(conexion, path);
}

static int do_rename(const char* pathAnterior, const char* pathNuevo) {
	printf("Rename: %s a %s\n", pathAnterior, pathNuevo);
	return enviarSolicitudRenombrarArchivo(conexion, pathAnterior, pathNuevo);
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
	.open		= do_open,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
	.truncate 	= do_truncate,
	.unlink		= do_unlink,
	.rmdir		= do_rmdir,
	.rename		= do_rename,
};

int main(int argc, char *argv[]) {

	t_config* configuracionCLI = config_create("configuracion-SAC-CLI.config");
	char* ip = config_get_string_value(configuracionCLI,"IP");
	char* puerto = config_get_string_value(configuracionCLI,"PUERTO");

	conexion = crear_conexion(ip, puerto);

	if (conexion == -1){
		printf("Error al conectar con el servidor");
		return -1;
	}

	return fuse_main(argc, argv, &operations, NULL);
}
