#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <strsafe.h>
#include <io.h>
#include <process.h>
#include <conio.h>

#include "..\DLL\DLL.h"

memoria *mem;
jogador jog;
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

void criaMapa() {

	for (int i = 0; i < ALTURA; i++) {
		for (int j = 0; j < LARGURA; j++) {

			if (i == 0 || i == ALTURA - 1) {
				mem->matriz[i][j] = 'X';
			}

			if (j == 0 || j == LARGURA - 1) {
				mem->matriz[i][j] = 'X';
			}

		}
	}

}

void mostraJogo() {

	system("cls");

	for (int i = 0; i < ALTURA; i++) {
		for (int j = 0; j < LARGURA; j++) {

			if (jog.sna.corpo[0].y == i && jog.sna.corpo[0].x == j) {
				_tprintf(TEXT("%c"), '*');
			}
			else {
				//mutex
				_tprintf(TEXT("%c"), mem->matriz[i][j]);
				//mutex
			}

		}
		_tprintf(TEXT("\n"));
	}

}

void inicializaDadosJogador() {

	jog.sna.tamanho = 1;
	jog.sna.direcao = DIREITA;
	jog.sna.corpo[0].x = 5;
	jog.sna.corpo[0].y = 15;

}

void validaMovimento() {

	if (mem->tecla == 'w') {
		jog.sna.direcao = CIMA;
	}
	else if (mem->tecla == 'd') {
		jog.sna.direcao = DIREITA;
	}
	else if (mem->tecla == 's') {
		jog.sna.direcao = BAIXO;
	}
	else if (mem->tecla == 'a') {
		jog.sna.direcao = ESQUERDA;
	}

}

DWORD WINAPI threadMovimentoSnake(LPVOID lpvParam){

	while (1) {

		if (jog.sna.direcao == CIMA) {
			/*if (jog.sna.corpo[0].y > && jog.sna.corpo[0].y == ) {

			}*/
			jog.sna.corpo[0].y--;
		}
		else if (jog.sna.direcao == DIREITA) {
			jog.sna.corpo[0].x++;

		}
		else if (jog.sna.direcao == BAIXO) {
			jog.sna.corpo[0].y++;
		}
		else if (jog.sna.direcao == ESQUERDA) {
			jog.sna.corpo[0].x--;
		}

		mostraJogo();

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI recebeTecla(LPVOID lpvParam){

	while (TRUE) {

		WaitForSingleObject(hEventoTecla, INFINITE);

		//mutex
		validaMovimento();
		//mutex

	}

	ExitThread(0);
}

int main(void) {
	HANDLE hThreadMovimentoSnake;
	HANDLE hThreadRecebeTecla;

// UNICODE 
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	criaZonaMemoria(mem);

	hEventoTecla = CreateEvent(NULL, TRUE, FALSE, EventoTecla);
	if (hEventoTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria(hMemoria, mem);
		return -1;
	}

	criaMapa();

	inicializaDadosJogador();

	hThreadMovimentoSnake = CreateThread(NULL, 0, threadMovimentoSnake, NULL, 0, NULL);
	if (hThreadMovimentoSnake == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake(%d)\n"), GetLastError());
		CloseHandle(hEventoTecla);
		fechaZonaMemoria();
		return -1;
	}

	hThreadRecebeTecla = CreateThread(NULL, 0, recebeTecla, NULL, 0, NULL);
	if (hThreadRecebeTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread recebe tecla (%d)\n"), GetLastError());
		CloseHandle(hThreadMovimentoSnake);
		CloseHandle(hEventoTecla);
		fechaZonaMemoria();
		return -1;
	}

	WaitForSingleObject(hThreadMovimentoSnake, INFINITE);
	WaitForSingleObject(hThreadRecebeTecla, INFINITE);

	CloseHandle(hThreadRecebeTecla);
	CloseHandle(hThreadMovimentoSnake);
	CloseHandle(hEventoTecla);
	fechaZonaMemoria();

	return 0;

}