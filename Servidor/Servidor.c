//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <strsafe.h>
#include <io.h>
#include <process.h>
#include <conio.h>

#include "..\DLL\DLL.h"

#define TAM 200

memoria *mem;
jogador jog;
HANDLE hMemoria;
HANDLE hEventoTecla;
HANDLE hEventoMapa;

BOOL servidorExiste() {

		HKEY chave;
		DWORD queAconteceu, versao, tamanho;

		//Criar/abrir uma chave em HKEY_CURRENT_USER\Software\MinhaAplicacao
		if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Snake"), 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) !=
			ERROR_SUCCESS) {
			_tprintf(TEXT("Erro ao criar/abrir chave (%d)\n"), GetLastError());

			RegCloseKey(chave);
			return TRUE;
		}
		else {
			//Se a chave foi criada, inicializar os valores
			if (queAconteceu == REG_CREATED_NEW_KEY) {
				_tprintf(TEXT("Chave: HKEY_CURRENT_USER\\Software\\SnakeCriada\n"));

				//Criar valor "Versao" = 2
				versao = 2;
				RegSetValueEx(chave, TEXT("Versao"), 0, REG_DWORD, (LPBYTE)&versao, sizeof(DWORD));
				_tprintf(TEXT("Versão guardados\n"));

				RegCloseKey(chave);
				return FALSE;
			}
			//Se a chave foi aberta, ler os valores lá guardados
			else if (queAconteceu == REG_OPENED_EXISTING_KEY) {
				_tprintf(TEXT("Chave: HKEY_CURRENT_USER\\Software\\SnakeAberta\n"));
				tamanho = sizeof(versao);
				RegQueryValueEx(chave, TEXT("Versao"), NULL, NULL, (LPBYTE)&versao, &tamanho);
				if (versao == 2) {
					_tprintf(TEXT("Já existe uma instância do servidor a correr..."));

					RegCloseKey(chave);
					return TRUE;
				}
				else {
					versao = 2;
					RegSetValueEx(chave, TEXT("Versao"), 0, REG_DWORD, (LPBYTE)&versao, sizeof(DWORD));

					RegCloseKey(chave);
					return FALSE;
				}
				
			}
		}

		return TRUE;
}

void fechaServidorRegistry() {

	HKEY chave;
	DWORD queAconteceu, versao, tamanho;

	//Criar/abrir uma chave em HKEY_CURRENT_USER\Software\MinhaAplicacao
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Snake"), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) !=
		ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao criar/abrir chave (%d)\n"), GetLastError());

		RegCloseKey(chave);
		return;
	}
	else {
		//Se a chave foi criada, inicializar os valores
		if (queAconteceu == REG_CREATED_NEW_KEY) {
			_tprintf(TEXT("Chave: HKEY_CURRENT_USER\\Software\\SnakeCriada\n"));

			//Criar valor "Versao" = 2
			versao = 1;
			RegSetValueEx(chave, TEXT("Versao"), 0, REG_DWORD, (LPBYTE)&versao, sizeof(DWORD));
			_tprintf(TEXT("Versão guardados\n"));

			RegCloseKey(chave);
			return;
		}
		//Se a chave foi aberta, ler os valores lá guardados
		else if (queAconteceu == REG_OPENED_EXISTING_KEY) {
			_tprintf(TEXT("Chave: HKEY_CURRENT_USER\\Software\\SnakeFechada\n"));

			versao = 1;
			RegSetValueEx(chave, TEXT("Versao"), 0, REG_DWORD, (LPBYTE)&versao, sizeof(DWORD));

			RegCloseKey(chave);
			return;

		}
	}
}

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

	mem->matriz[2][40] = '*';
	mem->matriz[9][30] = '*';
	mem->matriz[20][50] = '*';
	mem->matriz[25][1] = '*';

}

void inicializaDadosJogador() {

	jog.sna.tamanho = 4;
	jog.sna.direcao = DIREITA;
	int xComeco = 7;
	jog.sna.corpo[0].y = 15;
	jog.sna.corpo[0].x = xComeco;
	xComeco--;

	for (int i = 1; i < jog.sna.tamanho; i++, xComeco--){
		jog.sna.corpo[i].x = xComeco;
		jog.sna.corpo[i].y = 15;
	}

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
	else if (mem->tecla == 'p') {
		encerraThreads = TRUE;
	}

}

BOOL dentroLimitesMapa() {

	if (jog.sna.corpo[0].y > 0 && jog.sna.corpo[0].y < ALTURA 
		&& jog.sna.corpo[0].x > 0 && jog.sna.corpo[0].x < LARGURA) {
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI threadMovimentoSnake(LPVOID lpvParam){

	while (!encerraThreads) {

		int xAnt = jog.sna.corpo[0].y;
		int yAnt = jog.sna.corpo[0].x;
		BOOL comeu = FALSE;

		if (jog.sna.direcao == CIMA) {
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

		if (mem->matriz[jog.sna.corpo[0].y][jog.sna.corpo[0].x] == '*') {
			comeu = TRUE;
		}

		if (dentroLimitesMapa() == FALSE) {
			encerraThreads = TRUE;
			ExitThread(0);
		}

		mem->matriz[jog.sna.corpo[0].y][jog.sna.corpo[0].x] = 'O';

		for (int i = 1; i < jog.sna.tamanho; i++){

			int xTemp = jog.sna.corpo[i].x;
			int	yTemp = jog.sna.corpo[i].y;

			jog.sna.corpo[i].x = xAnt;
			jog.sna.corpo[i].y = yAnt;

			//mutex
			mem->matriz[xAnt][yAnt] = 'O';
			//mutex

			xAnt = xTemp;
			yAnt = yTemp;
		}

		if (comeu == TRUE) {
			jog.sna.tamanho++;
			jog.sna.corpo[jog.sna.tamanho - 1].x = xAnt;
			jog.sna.corpo[jog.sna.tamanho - 1].y = yAnt;
			mem->matriz[xAnt][yAnt] = 'O';
			comeu = FALSE;
		}
		else {
			mem->matriz[xAnt][yAnt] = ' ';
		}

		SetEvent(hEventoMapa); //Mete a true
		ResetEvent(hEventoMapa); //Mete a false

		Sleep(200);
	}

	ExitThread(0);
}

DWORD WINAPI threadRecebeTecla(LPVOID lpvParam){

	while (!encerraThreads) {

		WaitForSingleObject(hEventoTecla, INFINITE);

		//mutex
		validaMovimento();
		//mutex

	}

	ExitThread(0);
}

int _tmain(int argc, TCHAR *argv[]) {
	HANDLE hThreadMovimentoSnake;
	HANDLE hThreadRecebeTecla;

// UNICODE 
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (servidorExiste() == TRUE) {
		return -1;
	}

	hMemoria = criaFileMapping();

	mem = criaMapView(hMemoria);
	if (mem == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hEventoTecla = CreateEvent(NULL, TRUE, FALSE, EventoTecla);
	if (hEventoTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hEventoMapa = CreateEvent(NULL, TRUE, FALSE, EventoMapa);
	if (hEventoMapa == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		CloseHandle(hEventoTecla);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	criaMapa();

	inicializaDadosJogador();

	hThreadMovimentoSnake = CreateThread(NULL, 0, threadMovimentoSnake, NULL, 0, NULL);
	if (hThreadMovimentoSnake == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake(%d)\n"), GetLastError());
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hThreadRecebeTecla = CreateThread(NULL, 0, threadRecebeTecla, NULL, 0, NULL);
	if (hThreadRecebeTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread recebe tecla (%d)\n"), GetLastError());
		CloseHandle(hThreadMovimentoSnake);
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	WaitForSingleObject(hThreadMovimentoSnake, INFINITE);
	WaitForSingleObject(hThreadRecebeTecla, INFINITE);

	CloseHandle(hThreadRecebeTecla);
	CloseHandle(hThreadMovimentoSnake);
	CloseHandle(hEventoTecla);
	CloseHandle(hEventoMapa);
	fechaZonaMemoria(hMemoria, mem);
	fechaServidorRegistry();

	return 0;

}