#ifndef SAC_CLI_CHECKPOINT_H_
#define SAC_CLI_CHECKPOINT_H_


#include <fuse.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/config.h>

#include "sockets.h"

#include "conexiones-cliente.h"
#include "estructurasEnComun.h"

#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))

int conexion;

static int do_getattr(const char* path, struct stat* st);
static int do_readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int do_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi);
static int do_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *info);
static int do_open(const char *path, struct fuse_file_info *fi);
static int do_mknod(const char* path, mode_t mode, dev_t rdev);
static int do_mkdir(const char* path, mode_t mode);
static int do_truncate(const char* path, off_t length);
static int do_unlink(const char *path);
static int do_rmdir(const char *path);
static int do_rename(const char* pathAnterior, const char* pathNuevo);

#endif /* SAC_CLI_CHECKPOINT_H_ */
