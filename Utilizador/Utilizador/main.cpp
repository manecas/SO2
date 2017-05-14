#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

#pragma comment(lib, "user32.lib")

TCHAR szName[] = TEXT("fmMsgSpace"); // Nome da zona de memória partilhada

#define MSGSIZE 75

typedef	struct _MSG{
	unsigned msgnum;
	TCHAR szMessage[75];
} Shared_MSG;

#define MSGBUFSIZE sizeof(Shared_MSG)

// Sem sinc
unsigned writeMensagem(Shared_MSG * shared, TCHAR * msgtext) {
											// Sem validação de tamanho

	unsigned myNum;
	int numchars = _tcslen(msgtext);

	if(numchars > MSGSIZE - 1){
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');
	}

	// Fechar mutex
	shared->msgnum++;
	myNum = shared->msgnum;
	_tcscpy_s(shared->szMessage, msgtext);
	// Abrir mutex

	return myNum;
}

// Sem sinc
void readMensagem(Shared_MSG * shared, Shared_MSG * msg) {
										// Falta validar tamanho do texto
	// Fechar mutex
	shared->msgnum;
	CopyMemory(msg, shared, sizeof(Shared_MSG));
	// Abrir mutex
}

// Sem sinc
unsigned peekMensagem(Shared_MSG * shared) {

	unsigned msgnum;
	// Fechar mutex
	msgnum = shared->msgnum; // simples atribuição (cópia de estrututas)
	// Abrir mutex

	return msgnum;
}

// ---------------------------------------------------------------

// thread de leitura de mensagens
// thread não usa variáveis globais (usa uma estrutura passada por parâmetro)

typedef struct _ControlData {
	HANDLE hMapFile;
	Shared_MSG * pBuf;
	int ThreadDeveContinuar;
	int ServidorContinua;
} ControData;

unsigned int __stdcall listenerThread(void * p) {
	ControData * pcd = (ControData *)p;
	unsigned int current = peekMensagem(pcd->pBuf);
	Shared_MSG rcv;

	while (pcd->ThreadDeveContinuar) {
		Sleep(500); // Isto não é o método correcto -> Usar um objecto de sincronização "evento" por exemplo

		if (peekMensagem(pcd->pBuf) > current) {
			readMensagem(pcd->pBuf, &rcv);
			current = rcv.msgnum;
			_tprintf(TEXT("[%d]: %s\n"), current, rcv.szMessage);

			if (_tcscmp(rcv.szMessage, TEXT("Servidor: fechar")) == 0) {
				pcd->ServidorContinua = 0;
			}
		}
	}

	return 0;
}

// -----------------------------------------------------------------------------

// função principal
// lê (do teclado) as mensagens do utilizador e envia-as (para memória partilhada)

int _tmain() {
	ControData cdata;
	DWORD tid;
	HANDLE thnd;

	TCHAR myText[MSGSIZE];
	Shared_MSG currentMSG;

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	_tprintf(TEXT("Cliente de Msg a iniciar.\n"));
	_tprintf(TEXT("Vou abrir a memoria partilhada.\n"));

	cdata.hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,		// read/write access
		FALSE,						// não herdar o nome
		szName);					// nome do objecto fich. mapeado

	if (cdata.hMapFile == NULL) {
		_tprintf(TEXT("A memória partilhada deu complicações (%d). Até amanhã.\n"), GetLastError());
		return 1;
	}

	_tprintf(TEXT("Vou criar a view da memoria partilhada.\n"));

	cdata.pBuf = (Shared_MSG *)MapViewOfFile(
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

	_tprintf(TEXT("Vou lançar a thread para ouvir o que se passa.\n"));
	cdata.ThreadDeveContinuar = 1;
	cdata.ServidorContinua = 1;
	thnd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)listenerThread, (LPVOID)&cdata, 0, &tid);

	if (thnd == NULL) {
		_tprintf(TEXT("[ERRO] Criação do listener thread\n", GetLastError()));
		return -1;
	}

	_tprintf(TEXT("Tudo ok, Vou ver a mensagem actual.\n"));
	readMensagem(cdata.pBuf, &currentMSG);
	_tprintf(TEXT("(%d): %s\n"), currentMSG.msgnum, currentMSG.szMessage);

	_tprintf(TEXT("Escreve aqui qualquer coisa. Não uses espaços. exit para sair\n"));

	while (1) {
		_tscanf(TEXT("%s"), myText);

		if (_tcscmp(myText, TEXT("exit")) == 0) {
			break;
		}

		if (cdata.ServidorContinua == 0) { // colocado a 0 pela thread
			_tprintf(TEXT("Servidor mandou fechar. Vou fazer isso.\n"));
			// este teste devia estar na thread
			break;
		}
		writeMensagem(cdata.pBuf, myText);
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




