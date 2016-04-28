#include "nucleo.h"

int main(void)
{

	// Leer archivo config.conf
	leerArchivoConfig();

	//Inicializar nuestra direccion
	inicializarMiDireccion();

	//file descriptor para escuchar (listener), para una nueva conexión (new) y para explorar conexiones (explorer)
	int fd_listener, fd_new, fd_explorer;

	fd_set conj_master, conj_read;//conjuntos de fd's total(master) y para los que tienen datos para lectura(read)
	struct sockaddr_in direccionCliente;//direccion del cliente

	int maxfd/*cima del conjunto "conj_master" */, yes = 1/*para setsockopt()*/, nbytes/*número de bytes recibidos del cliente*/;
	char buffer[100];//buffer para datos recibidos del cliente
	unsigned int tam = sizeof(struct sockaddr_in);//tamaño de datos pedido por accept()

	FD_ZERO(&conj_master);
	FD_ZERO(&conj_read);

	if( (fd_listener = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
	{
		perror("Error Socket Escucha\n");
		exit(1);
	}

	if( setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( bind(fd_listener, (struct sockaddr*) &miDireccion, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( listen(fd_listener, BACKLOG) == -1 )
	{
		perror("Error listen()\n");
		exit(1);
	}

	maxfd = fd_listener;//Por ahora el max es el listener
	FD_SET(fd_listener, &conj_master);

	while(1)
	{
		conj_read = conj_master;
		select(maxfd+1, &conj_read, NULL, NULL, NULL);
		for(fd_explorer = 0; fd_explorer <= maxfd; fd_explorer++)
		{
			if(FD_ISSET(fd_explorer, &conj_read))//hay datos!
			{

				if(fd_explorer == fd_listener)//Hay una conexión nueva
				{
					fd_new = accept(fd_listener, (struct sockaddr*) &direccionCliente, &tam);
					FD_SET(fd_new, &conj_master);
					switch(reconocerCliente(&direccionCliente))
					{
					case 0:
						FD_CLR(fd_explorer, &conj_master);
						close(fd_explorer);
						printf("Proceso desconocido ha sido desconectado\n");
						break;
					case 1:
						printf("Se acepto una conexion de la Consola\n");
						if(fd_new > maxfd) maxfd = fd_new;
						break;
					case 2:
						printf("Se acepto una conexion de la CPU\n");
						if(fd_new > maxfd) maxfd = fd_new;
						break;
					case 3:
						printf("Se acepto una conexion de la UMC\n");
						if(fd_new > maxfd) maxfd = fd_new;
						break;
					}
				}
				else//Hay datos entrantes
				{
					nbytes = recv(fd_explorer, buffer, 100, 0);
					if(nbytes <= 0)//falló ó se desconectó el cliente
					{
						if(nbytes < 0)
						{
							perror("Error recv()\n");
							FD_CLR(fd_explorer, &conj_master);
							close(fd_explorer);
						}
						if(nbytes == 0)//se desconectó
						{
							printf("Un cliente se ha desconectado\n");
							FD_CLR(fd_explorer, &conj_master);
							close(fd_explorer);
						}
					}
					else printf("Mensaje recibido: %s\n", buffer);
				}
			}

		}
	}

	return EXIT_SUCCESS;
}