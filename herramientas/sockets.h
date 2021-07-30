#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

int crear_servidor(char* ip,char* puerto);
int crear_conexion(char *ip, char* puerto);
int esperar_cliente(int socket_servidor);
int liberar_conexion(int socket_cliente);
char *get_client_ip(int socket_cliente);

/*
 * @NAME: observar_bolsa_de_sockets
 * @DESC: dado un servidor, esta funcion hace un loop infinito escuchando a los distintos sockets
 * que se van conectando con el servidor. Requiere de una funcion de CALLBACK que utilice RECV. Esta
 * funcion deberá retornar un entero, 0 en el caso de que el RECV utilizado retorne 0 (indicando que
 * el cliente se desconectó del socket) o cualquiero otro numero (1), indicando que el CALLBACK pudo
 * hacer el RECV y realizar su trabajo con exito. Tambien recive una funcion CALLBACK_DESCONEXION que
 * recive el socket y se ejecuta cuando este se desconecta.
 */
int observar_bolsa_de_sockets(int servidor, int (callback_evento_cliente(int)), void (callback_desconexion(int)));

#endif
