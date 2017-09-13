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

#define VELOCIDADE_SNAKES 400

memoria *mem;
jogador jog;
snake snakeAutomatica;

int random(int min, int max) {

	return rand() % (max - min + 1) + min;

}

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
	DWORD queAconteceu, versao;

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

void criaMapa() {

	for (int i = 0; i < ALTURA; i++) {
		for (int j = 0; j < LARGURA; j++) {

			if (i == 0 || i == ALTURA - 1) {
				WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
				mem->matriz[i][j] = 'X';
				ReleaseMutex(hMutexMemoriaPartilhada);
			}

			if (j == 0 || j == LARGURA - 1) {
				WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
				mem->matriz[i][j] = 'X';
				ReleaseMutex(hMutexMemoriaPartilhada);
			}

			if (i > 0 && i < ALTURA - 1 && j > 0 && j < LARGURA - 1) {

				if (random(1, 100) == 15) {
					WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
					mem->matriz[i][j] = '*';
					ReleaseMutex(hMutexMemoriaPartilhada);
				}

			}

		}
	}

}

void inicializaDadosJogador() {

	WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
	mem->pontos1 = 0;
	mem->pontos2 = 0;
	ReleaseMutex(hMutexMemoriaPartilhada);

	for (int i = 0; i < NUM_JOGADORES; i++)
	{

		jog.snakes[i].pontos = 0;
		jog.snakes[i].tamanho = TAM_CORPO_SNAKES_JOGADORES;
		jog.snakes[i].direcao = BAIXO;

		int xComeco, yComeco;

		if (i == 0) {

			xComeco = 3;
			yComeco = 5;

		}
		else if(i == 1) {

			xComeco = 56;
			yComeco = 5;

		}

		jog.snakes[i].corpo[0].y = yComeco;
		jog.snakes[i].corpo[0].x = xComeco;
		yComeco--;

		//Começar depois da cabeça
		for (int k = 1; k < jog.snakes[i].tamanho; k++, yComeco--) {
			jog.snakes[i].corpo[k].x = xComeco;
			jog.snakes[i].corpo[k].y = yComeco;
		}
		
	}

}

void inicializaDadosSnakeAutomatica() {

	snakeAutomatica.tamanho = TAM_CORPO_SNAKE_AUTOMATICA;
	snakeAutomatica.direcao = CIMA;

	int xComeco = 30;
	int yComeco = 20;

	snakeAutomatica.corpo[0].y = yComeco;
	snakeAutomatica.corpo[0].x = xComeco;
	yComeco--;

	//Começar depois da cabeça
	for (int k = 1; k < snakeAutomatica.tamanho; k++, yComeco--) {
		snakeAutomatica.corpo[k].x = xComeco;
		snakeAutomatica.corpo[k].y = yComeco;
	}

}

void limpaCasasSnakeAutomatica() {

	for (int k = 0; k < snakeAutomatica.tamanho; k++) {
		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		mem->matriz[snakeAutomatica.corpo[k].y][snakeAutomatica.corpo[k].x] = ' ';
		ReleaseMutex(hMutexMemoriaPartilhada);
	}

	for (int i = 0; i < ALTURA; i++) {
		for (int j = 0; j < LARGURA; j++) {

			if (i == 0 || i == ALTURA - 1) {
				WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
				mem->matriz[i][j] = 'X';
				ReleaseMutex(hMutexMemoriaPartilhada);
			}

			if (j == 0 || j == LARGURA - 1) {
				WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
				mem->matriz[i][j] = 'X';
				ReleaseMutex(hMutexMemoriaPartilhada);
			}

		}
	}

}

void validaMovimento() {

	if (mem->tecla == 'w') {
		jog.snakes[0].direcao = CIMA;
	}
	else if (mem->tecla == 'd') {
		jog.snakes[0].direcao = DIREITA;
	}
	else if (mem->tecla == 's') {
		jog.snakes[0].direcao = BAIXO;
	}
	else if (mem->tecla == 'a') {
		jog.snakes[0].direcao = ESQUERDA;
	}
	else if (mem->tecla == 'i') {
		jog.snakes[1].direcao = CIMA;
	}
	else if (mem->tecla == 'l') {
		jog.snakes[1].direcao = DIREITA;
	}
	else if (mem->tecla == 'k') {
		jog.snakes[1].direcao = BAIXO;
	}
	else if (mem->tecla == 'j') {
		jog.snakes[1].direcao = ESQUERDA;
	}
	else if (mem->tecla == 'p') {
		encerraThreads = TRUE;
	}

}

BOOL jogadoresDentroLimitesMapa() {

	int quantosDentroLimite = 0;

	for (int i = 0; i < NUM_JOGADORES; i++){

		if (jog.snakes[i].corpo[0].y > 0 && jog.snakes[i].corpo[0].y < ALTURA - 1
			&& jog.snakes[i].corpo[0].x > 0 && jog.snakes[i].corpo[0].x < LARGURA - 1) {
			quantosDentroLimite++;
		}

	}
	
	return quantosDentroLimite == 2 ? TRUE : FALSE;
}

BOOL SnakeAutomaticadentroLimitesMapa() {

	if (snakeAutomatica.corpo[0].y > 0 && snakeAutomatica.corpo[0].y < ALTURA - 1
		&& snakeAutomatica.corpo[0].x > 0 && snakeAutomatica.corpo[0].x < LARGURA - 1) {
		return TRUE;
	}

	return FALSE;
}

void movimentaSnake(int qual) {

	if (jogadoresDentroLimitesMapa() == FALSE) {
		
		return;
	}

	int xAnt = jog.snakes[qual].corpo[0].x;
	int yAnt = jog.snakes[qual].corpo[0].y;
	BOOL comeu = FALSE;

	if (jog.snakes[qual].direcao == CIMA) {
		jog.snakes[qual].corpo[0].y--;
	}
	else if (jog.snakes[qual].direcao == DIREITA) {
		jog.snakes[qual].corpo[0].x++;
	}
	else if (jog.snakes[qual].direcao == BAIXO) {
		jog.snakes[qual].corpo[0].y++;
	}
	else if (jog.snakes[qual].direcao == ESQUERDA) {
		jog.snakes[qual].corpo[0].x--;
	}

	if (mem->matriz[jog.snakes[qual].corpo[0].y][jog.snakes[qual].corpo[0].x] == '*') {
		comeu = TRUE;
	}

	if (qual == 0) {
		mem->matriz[jog.snakes[qual].corpo[0].y][jog.snakes[qual].corpo[0].x] = 'O';
	}
	else if (qual == 1) {
		mem->matriz[jog.snakes[qual].corpo[0].y][jog.snakes[qual].corpo[0].x] = '#';
	}

	for (int i = 1; i < jog.snakes[qual].tamanho; i++) {

		int xTemp = jog.snakes[qual].corpo[i].x;
		int	yTemp = jog.snakes[qual].corpo[i].y;

		jog.snakes[qual].corpo[i].x = xAnt;
		jog.snakes[qual].corpo[i].y = yAnt;

		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		if (qual == 0) {
			mem->matriz[yAnt][xAnt] = 'O';
		}
		else if (qual == 1) {
			mem->matriz[yAnt][xAnt] = '#';
		}
		ReleaseMutex(hMutexMemoriaPartilhada);

		xAnt = xTemp;
		yAnt = yTemp;
	}

	if (comeu == TRUE) {

		jog.snakes[qual].tamanho++;
		jog.snakes[qual].pontos += 2;
		jog.snakes[qual].corpo[jog.snakes[qual].tamanho - 1].x = xAnt;
		jog.snakes[qual].corpo[jog.snakes[qual].tamanho - 1].y = yAnt;

		if (qual == 0) {
			WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
			mem->matriz[yAnt][xAnt] = 'O';
			mem->pontos1 = jog.snakes[qual].pontos;
			ReleaseMutex(hMutexMemoriaPartilhada);
		}
		else if (qual == 1) {
			WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
			mem->matriz[yAnt][xAnt] = '#';
			mem->pontos2 = jog.snakes[qual].pontos;
			ReleaseMutex(hMutexMemoriaPartilhada);
		}

		comeu = FALSE;
	}
	else {
		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		mem->matriz[yAnt][xAnt] = ' ';
		ReleaseMutex(hMutexMemoriaPartilhada);
	}

}

void movimentaSnakeAutomatica() {

	if (SnakeAutomaticadentroLimitesMapa() == FALSE) {
		limpaCasasSnakeAutomatica();
		inicializaDadosSnakeAutomatica();
		return;
	}

	int xAnt = snakeAutomatica.corpo[0].x;
	int yAnt = snakeAutomatica.corpo[0].y;
	BOOL comeu = FALSE;

	if (snakeAutomatica.direcao == CIMA) {
		snakeAutomatica.corpo[0].y--;
	}
	else if (snakeAutomatica.direcao == DIREITA) {
		snakeAutomatica.corpo[0].x++;
	}
	else if (snakeAutomatica.direcao == BAIXO) {
		snakeAutomatica.corpo[0].y++;
	}
	else if (snakeAutomatica.direcao == ESQUERDA) {
		snakeAutomatica.corpo[0].x--;
	}

	if (mem->matriz[snakeAutomatica.corpo[0].y][snakeAutomatica.corpo[0].x] == '*') {
		comeu = TRUE;
	}

	mem->matriz[snakeAutomatica.corpo[0].y][snakeAutomatica.corpo[0].x] = 'A';

	for (int i = 1; i < snakeAutomatica.tamanho; i++) {

		int xTemp = snakeAutomatica.corpo[i].x;
		int	yTemp = snakeAutomatica.corpo[i].y;

		snakeAutomatica.corpo[i].x = xAnt;
		snakeAutomatica.corpo[i].y = yAnt;

		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		mem->matriz[yAnt][xAnt] = 'A';
		ReleaseMutex(hMutexMemoriaPartilhada);

		xAnt = xTemp;
		yAnt = yTemp;
	}

	if (comeu == TRUE) {

		snakeAutomatica.tamanho++;
		snakeAutomatica.corpo[snakeAutomatica.tamanho - 1].x = xAnt;
		snakeAutomatica.corpo[snakeAutomatica.tamanho - 1].y = yAnt;

		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		mem->matriz[yAnt][xAnt] = 'A';
		ReleaseMutex(hMutexMemoriaPartilhada);

		comeu = FALSE;
	}
	else {
		WaitForSingleObject(hMutexMemoriaPartilhada, INFINITE);
		mem->matriz[yAnt][xAnt] = ' ';
		ReleaseMutex(hMutexMemoriaPartilhada);
	}

}

void escolheDirecaoRandomSnakeAutomatica() {

	srand((unsigned)time(NULL));

	int randomInt = random(1, 100);
	
	if (snakeAutomatica.direcao == CIMA) {

		if (randomInt <= 30) {
			snakeAutomatica.direcao = ESQUERDA;
		}

		if (randomInt >= 70) {
			snakeAutomatica.direcao = DIREITA;
		}

		if (randomInt > 33 && randomInt < 70) {
			snakeAutomatica.direcao = CIMA;
		}

	}
	else if (snakeAutomatica.direcao == BAIXO) {

		if (randomInt <= 30) {
			snakeAutomatica.direcao = ESQUERDA;
		}

		if (randomInt >= 70) {
			snakeAutomatica.direcao = DIREITA;
		}

		if (randomInt > 30 && randomInt < 70) {
			snakeAutomatica.direcao = BAIXO;
		}

	}
	else if (snakeAutomatica.direcao == ESQUERDA) {

		if (randomInt <= 30) {
			snakeAutomatica.direcao = CIMA;
		}

		if (randomInt >= 70) {
			snakeAutomatica.direcao = ESQUERDA;
		}

		if (randomInt > 30 && randomInt < 70) {
			snakeAutomatica.direcao = BAIXO;
		}

	}
	else if (snakeAutomatica.direcao == DIREITA) {

		if (randomInt <= 30) {
			snakeAutomatica.direcao = CIMA;
		}

		if (randomInt >= 70) {
			snakeAutomatica.direcao = DIREITA;
		}

		if (randomInt > 30 && randomInt < 70) {
			snakeAutomatica.direcao = BAIXO;
		}

	}

}

DWORD WINAPI threadMovimentoSnakeAutomatica(LPVOID lpvParam) {

	while (!encerraThreads) {

		escolheDirecaoRandomSnakeAutomatica();
		movimentaSnakeAutomatica();

		SetEvent(hEventoMapa); //Mete a true
		ResetEvent(hEventoMapa); //Mete a false

		Sleep(VELOCIDADE_SNAKES);
	}

	ExitThread(0);
}

DWORD WINAPI threadMovimentoSnake2(LPVOID lpvParam) {

	while (!encerraThreads) {

		movimentaSnake(SNAKE2);

		SetEvent(hEventoMapa); //Mete a true
		ResetEvent(hEventoMapa); //Mete a false

		Sleep(VELOCIDADE_SNAKES);
	}

	ExitThread(0);

}

DWORD WINAPI threadMovimentoSnake1(LPVOID lpvParam){

	while (!encerraThreads) {

		movimentaSnake(SNAKE1);

		SetEvent(hEventoMapa); //Mete a true
		ResetEvent(hEventoMapa); //Mete a false

		Sleep(VELOCIDADE_SNAKES);
	}

	ExitThread(0);
}

DWORD WINAPI threadRecebeTecla(LPVOID lpvParam){

	while (!encerraThreads) {

		WaitForSingleObject(hEventoTecla, INFINITE);

		validaMovimento();

	}

	ExitThread(0);
}

int _tmain(int argc, TCHAR *argv[]) {
	HANDLE hThreadMovimentoSnake1;
	HANDLE hThreadMovimentoSnake2;
	HANDLE hThreadMovimentoSnakeAutomatica;
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

	hMutexMemoriaPartilhada = CreateMutex(NULL, FALSE, MUTEX_MEMORIA_PARTILHADA);
	if (hMutexMemoriaPartilhada == NULL)	{
		_tprintf(TEXT("[Erro]Criação de objecto do mutexMemoriaPartilhada(%d)\n"), GetLastError());
		return -1;
	}

	hMemoria = criaFileMapping();

	mem = criaMapView(hMemoria);
	if (mem == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do criaMapView(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hEventoTecla = CreateEvent(NULL, TRUE, FALSE, EventoTecla);
	if (hEventoTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do EventoTecla(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hEventoMapa = CreateEvent(NULL, TRUE, FALSE, EventoMapa);
	if (hEventoMapa == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do EventoMapa(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		CloseHandle(hEventoTecla);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	criaMapa();

	inicializaDadosJogador();

	inicializaDadosSnakeAutomatica();

	hThreadMovimentoSnake1 = CreateThread(NULL, 0, threadMovimentoSnake1, NULL, 0, NULL);
	if (hThreadMovimentoSnake1 == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake 1(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hThreadMovimentoSnake2 = CreateThread(NULL, 0, threadMovimentoSnake2, NULL, 0, NULL);
	if (hThreadMovimentoSnake2 == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake 2(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		CloseHandle(hThreadMovimentoSnake1);
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hThreadMovimentoSnakeAutomatica = CreateThread(NULL, 0, threadMovimentoSnakeAutomatica, NULL, 0, NULL);
	if (hThreadMovimentoSnakeAutomatica == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread movimento snake automatica(%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		CloseHandle(hThreadMovimentoSnake1);
		CloseHandle(hThreadMovimentoSnake2);
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	hThreadRecebeTecla = CreateThread(NULL, 0, threadRecebeTecla, NULL, 0, NULL);
	if (hThreadRecebeTecla == NULL) {
		_tprintf(TEXT("[Erro] Criação da thread recebe tecla (%d)\n"), GetLastError());
		CloseHandle(hMutexMemoriaPartilhada);
		CloseHandle(hThreadMovimentoSnake1);
		CloseHandle(hThreadMovimentoSnake2);
		CloseHandle(hThreadMovimentoSnakeAutomatica);
		CloseHandle(hEventoTecla);
		CloseHandle(hEventoMapa);
		fechaZonaMemoria(hMemoria, mem);
		fechaServidorRegistry();
		return -1;
	}

	WaitForSingleObject(hThreadMovimentoSnake1, INFINITE);
	WaitForSingleObject(hThreadMovimentoSnake2, INFINITE);
	WaitForSingleObject(hThreadMovimentoSnakeAutomatica, INFINITE);
	WaitForSingleObject(hThreadRecebeTecla, INFINITE);

	CloseHandle(hMutexMemoriaPartilhada);
	CloseHandle(hThreadMovimentoSnake1);
	CloseHandle(hThreadMovimentoSnake2);
	CloseHandle(hThreadMovimentoSnakeAutomatica);
	CloseHandle(hThreadRecebeTecla);
	CloseHandle(hEventoTecla);
	CloseHandle(hEventoMapa);
	fechaZonaMemoria(hMemoria, mem);
	fechaServidorRegistry();

	return 0;

}