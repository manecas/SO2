//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <conio.h>

#include "..\DLL\DLL.h"

memoria *mem;
HANDLE hMemoria;
HANDLE hEventoTecla;
HANDLE hEventoMapa;

//void criaZonaMemoria() {
//
//	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(memoria), NomeMemoriaPartilhada);
//
//	if (hMemoria == NULL) {
//		_tprintf(TEXT("hMemoria is NULL\n"));
//	}
//
//	mem = (memoria *)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(memoria));
//
//	if (mem == NULL) {
//		_tprintf(TEXT("mem is NULL\n"));
//	}
//}

//void fechaZonaMemoria() {
//
//	UnmapViewOfFile(mem);
//	CloseHandle(hMemoria);
//
//}

void mostraJogo() {

	system("cls");

	for (int i = 0; i < ALTURA; i++) {
		for (int j = 0; j < LARGURA; j++) {

			//mutex
			_tprintf(TEXT("%c"), mem->matriz[i][j]);
			//mutex

		}
		_tprintf(TEXT("\n"));
	}

}

DWORD WINAPI threadRecebeMapa(LPVOID lpvParam)
{

	while (!encerraThreads) {

		WaitForSingleObject(hEventoMapa, INFINITE);

		mostraJogo();

	}

	ExitThread(0);
}

int _tmain(int argc, TCHAR *argv[]) {

	HANDLE hThreadRecebeMapa;
	TCHAR key;

// UNICODE 
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hMemoria = criaFileMapping();

	mem = criaMapView(hMemoria);
	if (mem == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria(hMemoria, mem);
		return -1;
	}

	hEventoTecla = CreateEvent(NULL, TRUE, FALSE, EventoTecla);
	if (hEventoTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria(hMemoria, mem);
		return -1;
	}

	hEventoMapa = CreateEvent(NULL, TRUE, FALSE, EventoMapa);
	if (hEventoMapa == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		CloseHandle(hEventoTecla);
		fechaZonaMemoria(hMemoria, mem);
		return -1;
	}

	hThreadRecebeMapa = CreateThread(NULL, 0, threadRecebeMapa, NULL, 0, NULL);
	if (hThreadRecebeMapa == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake(%d)\n"), GetLastError());
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		return -1;
	}

	while (!encerraThreads) {

		key = _gettch();

		//mutex
		mem->tecla = key;
		//mutex

		SetEvent(hEventoTecla); //Mete a true
		ResetEvent(hEventoTecla); //Mete a false

		if (key == 'p') {
			_tprintf(TEXT("A sair..."));
			break;
		}

	}

	CloseHandle(hEventoTecla);
	CloseHandle(hEventoMapa);
	fechaZonaMemoria(hMemoria, mem);

	return 0;

}