#include <commons/config.h>
#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include "consola.h"

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();



	return EXIT_SUCCESS;
}


/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig() {

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		// No se pudo cargar el archivo config.conf
		config_destroy(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puerto = config_get_string_value(config, "PUERTO");

	config_destroy(config);
}
