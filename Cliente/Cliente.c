#define _CRT_SECURE_NO_WARNINGS

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

void criaZonaMemoria() {

	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(memoria), NomeMemoriaPartilhada);

	if (hMemoria == NULL) {
		_tprintf(TEXT("hMemoria is NULL\n"));
	}

	mem = (memoria *)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(memoria));

	if (mem == NULL) {
		_tprintf(TEXT("mem is NULL\n"));
	}
}

void fechaZonaMemoria() {

	UnmapViewOfFile(mem);
	CloseHandle(hMemoria);

}

DWORD WINAPI recebeMensagens(LPVOID lpvParam)
{

	while (TRUE) {

		

	}

	ExitThread(0);
}

int main(void) {

	TCHAR key;

// UNICODE 
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	criaZonaMemoria(mem);

	hEventoTecla = CreateEvent(NULL, TRUE, FALSE, EventoTecla);
	if (hEventoTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria();
		return -1;
	}

	while (TRUE) {

		key = _gettch();

		//mutex
		mem->tecla = key;
		//mutex

		SetEvent(hEventoTecla); //Mete a true
		ResetEvent(hEventoTecla); //Mete a false

		if (key == 's') {
			_tprintf(TEXT("A sair..."));
			break;
		}

		_tprintf(TEXT("\n"));
	}

	CloseHandle(hEventoTecla);
	fechaZonaMemoria();

	return 0;

}