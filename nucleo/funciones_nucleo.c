#include "nucleo.h"

/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig(void)
	{
	t_config *config = config_create("config.conf");

	if (config == NULL)
	{
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.puerto_prog = config_get_string_value(config, "PUERTO_PROG");
	infoConfig.puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
	infoConfig.puerto_umc = config_get_string_value(config, "PUERTO_UMC");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}

void inicializarDirecciones(void)
{
	direccionParaConsola.sin_family = AF_INET;
	direccionParaConsola.sin_port = htons(atoi(infoConfig.puerto_prog));
	direccionParaConsola.sin_addr.s_addr = INADDR_ANY;
	memset(direccionParaConsola.sin_zero, '\0', sizeof(direccionParaConsola.sin_zero));

	direccionParaCPU.sin_family = AF_INET;
	direccionParaCPU.sin_port = htons(atoi(infoConfig.puerto_cpu));
	direccionParaCPU.sin_addr.s_addr = INADDR_ANY;
	memset(direccionParaCPU.sin_zero, '\0', sizeof(direccionParaCPU.sin_zero));

	direccionUMC.sin_family = AF_INET;
	direccionUMC.sin_port = htons(atoi(infoConfig.puerto_umc));
	direccionUMC.sin_addr.s_addr = INADDR_ANY;
	memset(direccionUMC.sin_zero, '\0', sizeof(direccionUMC.sin_zero));

}

void conectar_a_umc(void)
{
	fd_umc = socket(AF_INET, SOCK_STREAM, 0);
	connect(fd_umc, (struct sockaddr*) &direccionUMC, sizeof(struct sockaddr));
}

void abrirPuertos(void)
{
	int yes1 = 1, yes2 = 1;/*para setsockopt()*/
	//Setear listener para consola
	if( (fd_listener_consola = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
	{
		perror("Error Socket Listener Consola\n");
		exit(1);
	}

	if( setsockopt(fd_listener_consola, SOL_SOCKET, SO_REUSEADDR, &yes1, sizeof(int)) == -1 )
	{
		perror("Error setsockpt Listener Consola\n");
		exit(1);
	}
	if( bind(fd_listener_consola, (struct sockaddr*) &direccionParaConsola, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error bind Listener Consola\n");
		exit(1);
	}
	if( listen(fd_listener_consola, BACKLOG) == -1 )
	{
		perror("Error listen(consola)\n");
		exit(1);
	}

	//Setear listener para cpu
	if( (fd_listener_cpu = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
	{
		perror("Error Socket Escucha\n");
		exit(1);
	}

	if( setsockopt(fd_listener_cpu, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof(int)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( bind(fd_listener_cpu, (struct sockaddr*) &direccionParaCPU, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( listen(fd_listener_cpu, BACKLOG) == -1 )
	{
		perror("Error listen(cpu)\n");
		exit(1);
	}
}

int maximofd(int fd1, int fd2)
{
	if(fd1 > fd2)
		return fd1;
	else
		return fd2;
}

void administrarConexiones(void)
{
	int maxfd/*cima del conjunto "master_fds" */, nbytes/*número de bytes recibidos del cliente*/;
	unsigned int tam = sizeof(struct sockaddr_in);//tamaño de datos pedido por accept()
	char mensajeParaCPU[100] = "Nucleo dice: Conectado a CPU\n";
	fd_set conj_master, conj_read;//conjuntos de fd's total(master) y para los que tienen datos para lectura(read)

	fd_set conj_consola, conj_cpu;

	FD_ZERO(&conj_master);
	FD_ZERO(&conj_read);
	FD_ZERO(&conj_consola);
	FD_ZERO(&conj_cpu);

	maxfd = maximofd(fd_listener_consola, maximofd(fd_listener_cpu, fd_umc));//Encontrar el maximo fd

	FD_SET(fd_listener_consola, &conj_master);
	FD_SET(fd_listener_cpu, &conj_master);
	FD_SET(fd_umc, &conj_master);

	while(1)
	{
		conj_read = conj_master;//se ponen a disposicion todos los fd para despues filtrar los que tengan entrada
		select(maxfd+1, &conj_read, NULL, NULL, NULL);

		for(fd_explorer = 0; fd_explorer <= maxfd; fd_explorer++)//recorrer todos los fd
		{
			if(FD_ISSET(fd_explorer, &conj_read))//hay datos!
			{
				if(fd_explorer == fd_listener_consola)//Hay una conexión de una nueva consola
				{
					if( (fd_new = accept(fd_listener_consola, (struct sockaddr*) &direccionCliente, &tam)) == -1)
					{
						perror("Error Accept consola");
						FD_CLR(fd_new, &conj_master);
						close(fd_new);
					}

					else//Se acepto correctamente
					{
						FD_SET(fd_new, &conj_master);
						FD_SET(fd_new, &conj_consola);
						printf("Se acaba de aceptar una conexion de una nueva Consola\n");
						if(fd_new > maxfd) maxfd = fd_new;
					}
				}
				if(fd_explorer == fd_listener_cpu)//Hay una conexión de una nueva cpu
				{
					if( (fd_new = accept(fd_listener_cpu, (struct sockaddr*) &direccionCliente, &tam)) == -1)
					{
						perror("Error Accept cpu");
						FD_CLR(fd_new, &conj_master);
						close(fd_new);
					}
					else//se acepto correctamente
					{
						FD_SET(fd_new, &conj_master);
						FD_SET(fd_new, &conj_cpu);
						printf("Se acaba de aceptar una conexion de una nueva CPU\n");
						if(fd_new > maxfd) maxfd = fd_new;
					}
				}
				else//Hay datos entrantes
				{
					if(FD_ISSET(fd_explorer, &conj_consola))//Los datos vienen de una Consola
					{
						nbytes = recv(fd_explorer, bufferConsola, 100, 0);//Recibir los datos al buffer
						if(nbytes <= 0)//Hubo un error o se desconecto
						{
							if(nbytes < 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_consola);
								close(fd_explorer);
								perror("Error de recepción en una consola\n");
							}
							if(nbytes == 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_consola);
								close(fd_explorer);
								printf("Se ha desconectado una consola\n");
							}
						}
						else//No hubo error ni desconexion
							{
								printf("Mensaje recibido de una Consola: %s\n", bufferConsola);
								send(fd_new, mensajeParaCPU, 100, 0);
							}
					}

					if(FD_ISSET(fd_explorer, &conj_cpu))//Los datos vienen de una CPU
					{
						nbytes = recv(fd_explorer, bufferCPU, 100, 0);//Recibir los datos al buffer
						if(nbytes <= 0)//Hubo un error o se desconecto
						{
							if(nbytes < 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_cpu);
								close(fd_explorer);
								perror("Error de recepción en una cpu\n");
							}
							if(nbytes == 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_cpu);
								close(fd_explorer);
								printf("Se ha desconectado una cpu\n");
							}
						}
					else//No hubo error ni desconexion
						printf("Mensaje recibido de una CPU: %s\n", bufferCPU);
					}

				}
			}
		}
	}
}
