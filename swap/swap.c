/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"
#include "messageCode.h"
#include "initialize.h"

int main(){
	readConfigFile();
	crearArchivoSWAP();
	crearEstructurasDeManejo();
	setSocket();
	bindSocket();
	acceptSocket();
	int codigo = recibirCabecera();
	switch(codigo){
		case RESERVE_SPACE: reservarEspacio();
			break;
		case SAVE_PROGRAM: saveProgram();
			break;
		case SAVE_PAGE: saveNewPage();
			break;
		case END_PROGRAM: endProgram();
				break;
		case RETURN_PAGE: returnPage();
				break;
	}
	accionesDeFinalizacion();
	return 0;
}

void saveNewPage(){
	unsigned pid;
	unsigned nroPagDentroProg;
	recv(socketCliente,&pid,4,0);
	recv(socketCliente,&nroPagDentroProg,4,0);
	recv(socketCliente,searchedPage,TAMANIO_PAGINA,0);
	int pagInicial = buscarPagInicial(pid);
	savePage(pagInicial+nroPagDentroProg);

}

void savePage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	fwrite(searchedPage,TAMANIO_PAGINA,1,SWAPFILE);
}

void setNewPage(unsigned nroPag){
	bitarray_set_bit(DISP_PAGINAS, nroPag);
}

void unSetPage(unsigned nroPag){
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
}

char* getPage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	fread(searchedPage,TAMANIO_PAGINA,1,SWAPFILE);
	return searchedPage;
}

int recibirCabecera(){
	int codigo = 0;
	recv(socketCliente, &codigo, 4, 0);
	return codigo;
}

void reservarEspacio(){
	unsigned pid, espacio;
	recv(socketCliente, &pid, 4, 0);
	recv(socketCliente, &espacio, 4, 0);
	int lugar = searchSpaceToFill(espacio);
	if(lugar == -1){
		compactar();
		lugar = searchSpaceToFill(espacio);
	}
	if(lugar == -2){
		negarEjecucion(pid);
		return;
	}
	asignarEspacio(pid,lugar,espacio);
}

void compactar(){
	int contador=0, inicioEspacioBlanco =0, estadoPagina;
	int espacioBlancoDetras=0;
	while(contador<CANTIDAD_PAGINAS){
		estadoPagina=bitarray_test_bit(DISP_PAGINAS,contador);
		if(estadoPagina==0){
			if(!espacioBlancoDetras){
				inicioEspacioBlanco=contador;
			}
			espacioBlancoDetras=1;
			contador++;
		}
		else if(espacioBlancoDetras){
			moveProgram(contador,inicioEspacioBlanco);
			espacioBlancoDetras=0;
		}
		else
			contador++;
	}
}

void moveProgram(int inicioProg, int inicioEspacioBlanco){
	int contador=0;
	int pid = buscarPIDSegunPagInicial(inicioProg);
	int longitudPrograma= buscarLongPrograma(pid);
	while(contador<=longitudPrograma){
		searchedPage = getPage(inicioProg+contador);
		savePage(inicioEspacioBlanco+contador);
		unSetPage(inicioProg+contador);
		setNewPage(inicioEspacioBlanco+contador);
		contador++;
		//FALTA CAMBIAR LOS ESTADOS DEL PID, NUMERO DE PAG INICIAL, OFFSET
	}
}

void asignarEspacio(unsigned pid, int lugar, unsigned espacio){
	agregarAlprogInfo(pid,lugar,espacio);
	int paginasReservadas;
	while(paginasReservadas<= espacio){
		setPage(lugar);
		lugar++;
		paginasReservadas++;
	}
}

void saveProgram(){
	int espacio, pagInicial, cantidadGuardada=0;
	unsigned pid;
	char* pagAGuardar = malloc(TAMANIO_PAGINA);
	recv(socketCliente,&pid,4,0);
	espacio = buscarLongPrograma(pid);
	pagInicial = buscarPagInicial(pid);
	while(cantidadGuardada<=espacio){
		recv(socketCliente,&pagAGuardar,TAMANIO_PAGINA,0);
		savePage(pagInicial+cantidadGuardada);
		cantidadGuardada++;
	}
}

int buscarPIDSegunPagInicial(int inicioProg){
	int i=0;
	while(i<=CANTIDAD_PAGINAS){
		if(inicioProg == INFO_PROG[i].PAG_INICIAL)
			return INFO_PROG[i].PID;
		i++;
	}
	return -1;
}

int buscarLongPrograma(int pid){
	int i=0;
		while(i<=CANTIDAD_PAGINAS){
			if(pid == INFO_PROG[i].PID)
				return INFO_PROG[i].LONGITUD;
			i++;
		}
		return -1;
}

int buscarPagInicial(int pid){
	int i=0;
	while(i<=CANTIDAD_PAGINAS){
		if(pid == INFO_PROG[i].PID)
			return INFO_PROG[i].PAG_INICIAL;
		i++;
	}
	return -1;
}

int searchSpaceToFill(unsigned programSize){
	int freeSpace =0; 			//PARA REALIZAR COMPACTACION
	int freeSpaceInARow=0;		//PARA ASIGNAR SIN COMPACTAR
	int counter=0;				//CONTADOR DE PAGINAS
	while(counter<CANTIDAD_PAGINAS){
		if(bitarray_test_bit(DISP_PAGINAS, counter)!=0){
			freeSpaceInARow=0;
		}
		else{
			freeSpace++;
			freeSpaceInARow++;
			if(programSize<=freeSpaceInARow)
				return (counter-freeSpaceInARow+1); //DEVUELVE EL NRO DE PAGINA DONDE INICIA EL SEGMENTO LIBRE PARA ASIGNAR EL PROGRAMA
		}
		counter++;
	}
	if(programSize<=freeSpace){
		return -1; //HAY QUE COMPACTAR
	}
	return -2; //NO HAY LUGAR PARA ALBERGARLO
}
