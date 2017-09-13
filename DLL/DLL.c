#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <io.h> 
#include <fcntl.h> 
#include <stdio.h>
#include <time.h>

#include "DLL.h"

//variaveis globais
TCHAR NomeMemoriaPartilhada[] = TEXT("Memória Partilhada");
TCHAR *Mutex = { TEXT("Mutex") };
TCHAR EventoTecla[] = TEXT("EventoTecla");
TCHAR EventoMapa[] = TEXT("EventoMapa");
BOOL encerraThreads = FALSE;

//funções memória partilhada
HANDLE criaFileMapping() {

	HANDLE hMemoriaPartilhada;

	hMemoriaPartilhada = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(memoria), NomeMemoriaPartilhada);

	return hMemoriaPartilhada;
}

memoria * criaMapView(HANDLE hMemoria) {

	if (hMemoria == NULL) {
		return NULL;
	}

	memoria *mem;

	mem = (memoria *)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(memoria));

	return mem;

}

void fechaZonaMemoria(HANDLE hMemoria, memoria *mem) {

	UnmapViewOfFile(mem);
	CloseHandle(hMemoria);

}
