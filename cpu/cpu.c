#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser/parser.h>
#include "analizador.h"
#include "cpu.h"

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();
	testParser();


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
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puerto = config_get_string_value(config, "PUERTO");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}

/*
 * testParser();
 * Parametros: -
 * Descripcion: Procedimiento que simula ejecutar un codigo ansisop
 * Return: -
 */
void testParser() {
	// Definir variable
	printf("Ejecutando '%s'\n", DEFINICION_VARIABLES);
	analizadorLinea(strdup(DEFINICION_VARIABLES), &functions, &kernel_functions);
	printf("================\n");
	// Asignar
	printf("Ejecutando '%s'\n", ASIGNACION);
	analizadorLinea(strdup(ASIGNACION), &functions, &kernel_functions);
	printf("================\n");
	// Imprimir
	printf("Ejecutando '%s'\n", IMPRIMIR);
	analizadorLinea(strdup(IMPRIMIR), &functions, &kernel_functions);
	printf("================\n");
	// Imprimir texto
	printf("Ejecutando '%s'", IMPRIMIR_TEXTO);
	analizadorLinea(strdup(IMPRIMIR_TEXTO), &functions, &kernel_functions);
	printf("================\n");
}

/*
 * FUNCIONES ANALIZADOR
 */

t_puntero definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

// t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
// t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
// t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
// t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar, t_puntero_instruccion linea_en_ejecucion);
// t_puntero_instruccion retornar(t_valor_variable retorno);

void imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
}

// void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
// void wait(t_nombre_semaforo identificador_semaforo);
// void signal(t_nombre_semaforo identificador_semaforo);
