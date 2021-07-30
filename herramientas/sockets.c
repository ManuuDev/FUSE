#include "sockets.h"
#include "arpa/inet.h"

int crear_servidor(char* ip,char* puerto) {

	int socket_servidor;

    struct addrinfo *servinfo, *p, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p = servinfo; p != NULL; p = p -> ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *informacion_servidor;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &informacion_servidor);

	int socket_cliente = socket(informacion_servidor->ai_family, informacion_servidor->ai_socktype, informacion_servidor->ai_protocol);

	if(connect(socket_cliente, informacion_servidor->ai_addr, informacion_servidor->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(informacion_servidor);

	return socket_cliente;
}

int esperar_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	size_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	return socket_cliente;
}

int liberar_conexion(int socket_cliente) {
	return close(socket_cliente);
}

char *get_client_ip(int socket_cliente){
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getpeername(socket_cliente, (struct sockaddr *)&addr, &addr_size);

	char *client_ip = malloc(20);
	strcpy(client_ip, inet_ntoa(addr.sin_addr));

	return client_ip;
}


int observar_bolsa_de_sockets(int servidor, int (callback_evento_cliente(int)), void (callback_desconexion(int))){
	int max_fd = 1;

	fd_set master;
	fd_set readfds;

	FD_ZERO(&master);
	FD_ZERO(&readfds);

	FD_SET(servidor, &master);
	max_fd = servidor;


	while (1) {
		readfds = master;
		select(max_fd+1, &readfds, NULL, NULL, NULL);

		for(int i = 0; i <= max_fd; i++){
			if(FD_ISSET(i, &readfds)){
				if(i == servidor){ // SE CONECTO ALGUIEN NUEVO AL SERVIDOR

					int nuevo_cliente = esperar_cliente(servidor);
					FD_SET(nuevo_cliente, &master);

					if(nuevo_cliente > max_fd)
						max_fd = nuevo_cliente;

				} else { // UN CLIENTE MANDO UN MENSAJE

					if(callback_evento_cliente(i) == 0){ // CLIENTE CERRO LA CONEXION
						callback_desconexion(i);
						liberar_conexion(i);
						FD_CLR(i, &master);
					}
					 // SI NO, YA HIZO LO QUE TENIA QUE HACER
				}
			}
		}
	}
}

