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
DLL_IMP_API TCHAR EventoTecla[] = TEXT("eventoTecla");

////funções memória partilhada
//HANDLE criaZonaMemoria(memoria *mem) {
//
//	HANDLE hMemoriaPartilhada;
//
//	hMemoriaPartilhada = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(memoria), NomeMemoriaPartilhada);
//
//	if (hMemoriaPartilhada == NULL) {
//		return hMemoriaPartilhada;
//	}
//
//	mem = (memoria *)MapViewOfFile(hMemoriaPartilhada, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(memoria));
//
//	return hMemoriaPartilhada;
//}
//
//void fechaZonaMemoria(HANDLE hMemoriaPartilhada, memoria *mem) {
//
//	UnmapViewOfFile(mem);
//	CloseHandle(hMemoriaPartilhada);
//
//}