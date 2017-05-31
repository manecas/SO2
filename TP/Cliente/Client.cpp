#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>


#define LIN_MAX 100
#define COL_MAX 100

// DEFINES
#define MSGSIZE 75
TCHAR szName[] = TEXT("memoria"); // Nome da zona de memória partilhada
TCHAR mazeName[] = TEXT("labirinto");

// NOME DOS SEMAFOROS
TCHAR semaforoEscritaName[] = TEXT("escrita");
TCHAR semaforoLeituraName[] = TEXT("leitura");
#define N 1 //Quantos semaforos

// ESTRUTURAS 
typedef struct _MSG {
	int nMSG; // NUMERO DA ESTRUTURA MSG // DEBUG
	unsigned msgnum;
	TCHAR szMessage[MSGSIZE];
	TCHAR tecla;
	int procId;
} Shared_MSG;


typedef struct _MP {
	int indice_leitura;
	int indice_escrita;
	Shared_MSG mensagens[N];
} Shared_MEM;

#define MSGBUFSIZE sizeof(Shared_MEM)


typedef struct {
	TCHAR maze[LIN_MAX][COL_MAX];
	int nLin;
	int nCol;
} Map;

#define MAP_SIZE sizeof(Map)


// CONTROLDATA TAMBÈM TEM FLAG PARA SABER SE SERVIDOR TERMINOU
typedef struct _ControlData {
	HANDLE hMapFile;
	HANDLE hMapFileMaze;

	Shared_MEM * pBuf;
	Map * pMap;

	HANDLE hSemaforoEscrita;
	HANDLE hSemaforoLeitura;

	int ThreadDeveContinuar;
	int ServidorContinua;
} ControData;



// FUNCAO PARA ESCREVER PARA A MEMORIA PARTILHADA
unsigned writeMensagem(Shared_MEM * shared, TCHAR * msgtext, TCHAR letra, int procId)
{
	unsigned myNum;
	int numchars = _tcslen(msgtext);

	if (numchars > MSGSIZE - 1) {
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');
	}

	// Fechar mutex
	shared->mensagens[ shared->indice_escrita].msgnum++;
	myNum = shared->mensagens[shared->indice_escrita].msgnum;

	// ATRIBUI LETRA A TECLA
	shared->mensagens[shared->indice_escrita].tecla = letra;

	shared->mensagens[shared->indice_escrita].procId = procId;

	_tcscpy(shared->mensagens[shared->indice_escrita].szMessage, msgtext);
	// Abrir mutex

	return myNum;
}


// FUNCAO PARA SACAR O NUMERO DA MENSAGEM DA MEMORIA PARTILHADA
unsigned peekMensagem(Shared_MEM * shared)
{
	unsigned msgnum;
	// Fechar mutex
	msgnum = shared->mensagens[shared->indice_escrita].msgnum; // simples atribuição (cópia de estrututas)
							 // Abrir mutex
	return msgnum;
}

// FUNCAO PARA LER DA MEMORIA PARTILHADA
void readMensagem(Shared_MEM * shared, Shared_MEM * msg)
{
	// Fechar mutex
	///shared->msgnum;
	CopyMemory(msg, shared, sizeof(Shared_MEM));
	// Abrir mutex
}



// FUNCAO DA THREAD PARA LER DA MEMORIA PARTILHADA
unsigned int __stdcall listenerThread(void * p) {
	ControData * pcd = (ControData *)p;
	unsigned int current = peekMensagem(pcd->pBuf);

	Shared_MEM rcv;

	while (pcd->ThreadDeveContinuar) {
		///Sleep(500); // Isto não é o método correcto -> Usar um objecto de sincronização "evento" por exemplo

		if (peekMensagem(pcd->pBuf) > current) {
			readMensagem(pcd->pBuf, &rcv);
			current = rcv.mensagens[rcv.indice_escrita].msgnum;
			_tprintf(TEXT("[%d]: %s\n"), current, rcv.mensagens[rcv.indice_escrita].szMessage);

			if (_tcscmp(rcv.mensagens[rcv.indice_escrita].szMessage, TEXT("Servidor: fechar")) == 0) {
				pcd->ServidorContinua = 0;
			}
		}
	}

	return 0;
}








////////////////////////////    MAIN     /////////////////////////////////

int _tmain() {
	

	// VARIAVEIS
	DWORD tid;
	HANDLE thnd;

	

	// ESTAS VARIAVEIS AGORA ESTAO NO CONTROLDATA
	/*
	HANDLE hMapFile;
	Shared_MSG * pBuf;
	int ThreadDeveContinuar;
	int ServidorContinua;
	*/
	ControData cdata;

	TCHAR myText[MSGSIZE];
	Shared_MEM currentMSG; 



	// UNICODE 
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("Cliente de Msg a iniciar.\n"));
	_tprintf(TEXT("Vou abrir a memoria partilhada.\n"));


	cdata.hSemaforoEscrita = CreateSemaphore(NULL, N, N, semaforoEscritaName);
	cdata.hSemaforoLeitura = CreateSemaphore(NULL, 0, N, semaforoLeituraName); //Semaforo


	if (cdata.hSemaforoEscrita == NULL)
		_tprintf(TEXT("ERRO A ABRIR SEMAFORO ESCRITA"));

	if (cdata.hSemaforoLeitura == NULL)
		_tprintf(TEXT("ERRO A ABRIR SEMAFORO LEITURA"));


	// ABRE MEMORIA PARTILHADA CRIADA PELO SERVIDOR
	cdata.hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,		// read/write access
		FALSE,						// não herdar o nome
		szName);					// nome do objecto fich. mapeado

	if (cdata.hMapFile == NULL) {
		_tprintf(TEXT("A memória partilhada deu complicações (%d). Até amanhã.\n"), GetLastError());
		return 1;
	}
	_tprintf(TEXT("Vou criar a view da memoria partilhada.\n"));




	// CRIA VISTA PARA A MEMORIA PARTILHADA
	cdata.pBuf = (Shared_MEM *)MapViewOfFile(
		cdata.hMapFile,
		FILE_MAP_ALL_ACCESS,				// Permissões read/write
		0,
		0,
		MSGBUFSIZE);

	if (cdata.pBuf == NULL) {
		_tprintf(TEXT("A view da memória partilhada deu azar (erro %d).\n"), GetLastError());
		CloseHandle(cdata.hMapFile);
		return 1;
	}

	//CRIA VISTA DO MAPA
	cdata.hMapFileMaze = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,		// read/write access
		FALSE,						// não herdar o nome
		mazeName);					// nome do objecto fich. mapeado

	if (cdata.hMapFileMaze == NULL) {
		_tprintf(TEXT("A memória partilhada deu complicações (%d). Até amanhã.\n"), GetLastError());
		return 1;
	}
	_tprintf(TEXT("Vou criar a view do mapa em memoria partilhada.\n"));

	cdata.pMap = (Map*) MapViewOfFile(
		cdata.hMapFileMaze,
		FILE_MAP_ALL_ACCESS,				// Permissões read/write
		0,
		0,
		MAP_SIZE);

	if (cdata.pMap == NULL) {
		_tprintf(TEXT("A view da memória partilhada deu azar (erro %d).\n"), GetLastError());
		CloseHandle(cdata.hMapFileMaze);
		return 1;
	}

	//IMPRIMIR MAPA
	_tprintf(TEXT("\n\n"));
	for (int i = 0; i < cdata.pMap->nLin; i++) {
		for (int j = 0; j < cdata.pMap->nCol; j++) {
			_tprintf(TEXT("%c"), cdata.pMap->maze[i][j]);
		}
		_tprintf(TEXT("\n\n"));
	}

	// TEM DE ESTAR AQUI A INICIALIZAÇÂO 
	cdata.pBuf->indice_escrita = 0;
	cdata.pBuf->indice_leitura = 0;


	_tprintf(TEXT("Vou lançar a thread para ouvir o que se passa.\n"));
	cdata.ThreadDeveContinuar = 1;
	cdata.ServidorContinua = 1;

	// CRIA THREAD LISTNER PARA LER DO SERVIDOR 
	thnd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)listenerThread, (LPVOID)&cdata, 0, &tid);

	if (thnd == NULL) {
		_tprintf(TEXT("[ERRO] (%d) Criação do listener thread\n"), GetLastError());
		return -1;
	}

	_tprintf(TEXT("Tudo ok, Vou ver a mensagem actual.\n"));


	// IDEALMENTE ISTO DEVERIA ESTAR NA THREAD LISTENER, POIS ELA E' QUE E' RESPONSAVEL POR LER DA MEMORIA PARTILHADA
	// LE DA MEMORIA PARTILHADA PARA UMA ESTRUTURA DO TIPO SHARED_MSG
	readMensagem(cdata.pBuf, &currentMSG);
	_tprintf(TEXT("(%d): %s\n"), currentMSG.mensagens[currentMSG.indice_escrita].msgnum, currentMSG.mensagens[currentMSG.indice_escrita].szMessage);

	_tprintf(TEXT("Escreve aqui qualquer coisa. Não uses espaços. exit para sair\n"));



	while (1) {
		WaitForSingleObject(cdata.hSemaforoEscrita, INFINITE);
		// LE A TECLA PRIMIDA PELO UTILIZADOR


		//fflush(stdin);
		TCHAR letra = _getch();						// TECLA PREMIDA
		//_tscanf(TEXT("%s"), myText);
		
		_tcscpy(myText, TEXT("QUALQUER COISA"));   // TEXTO
		 int procId = GetCurrentProcessId();		// PROCESS ID

		if (_tcscmp(myText, TEXT("exit")) == 0) {
			break;
		}

		if (cdata.ServidorContinua == 0) { // colocado a 0 pela thread
			_tprintf(TEXT("Servidor mandou fechar. Vou fazer isso.\n"));
			// este teste devia estar na thread
			break;
		}
		writeMensagem(cdata.pBuf, myText, letra, procId); // ESCREVE PARA A MEMORIA PARTILHADA
		ReleaseSemaphore(cdata.hSemaforoLeitura, 1, NULL);
	}

	_tprintf(TEXT("Cliente vai fechar.\n"));
	cdata.ThreadDeveContinuar = 0; // Informa thread que deve terminar
	WaitForSingleObject(thnd, INFINITE);
	_tprintf(TEXT("Thread ouvinte encerrada.\n"));

	UnmapViewOfFile(cdata.pBuf);
	CloseHandle(cdata.hMapFile);
	_tprintf(TEXT("Ficheiro desmapeado e recursos libertados.\n"));

	return 0;
}