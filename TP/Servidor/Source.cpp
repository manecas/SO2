#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

#include "../../TP/FileMappingDLL/FileMappingDLL/FileMapping.h"

// funções de escrita/leitura de mensagens
// sem sincronização (mas deverá ter - os locais assinaldas)

void iniMsgArea(Shared_MSG * shared) { // sem validação de tamanho
		// fechar mutex
	TCHAR inicio[] = TEXT("empty");
	shared->msgnum = 0;
	CopyMemory((PVOID)shared->szMessage, inicio, _tcslen(inicio) * sizeof(TCHAR));

	// Abrir Mutex
}

// --------------------------

// Thread de leitura de mensagens
// não usa variáveis globais Calaro

typedef struct _ControlData {  // informação acerca do que a thread
	HANDLE hMapFile;			// deve fazer
	Shared_MSG * pBuf;
	int ThreadDeveContinuar;
} ContrData;


unsigned int _stdcall listenerThread(void * p) {
	ContrData * pcd = (ContrData *)p;
	unsigned int current = peekMensagem(pcd->pBuf);
	Shared_MSG rcv;

	while(pcd->ThreadDeveContinuar) {
		Sleep(500); // obviamente, uma solução fraca -> usar sincornização obviamente 
					// (fazer como exercicio): sugestão -> eventos 

		if (peekMensagem(pcd->pBuf) > current) {
			readMensagem(pcd->pBuf, &rcv);
			current = rcv.msgnum;
			_tprintf(TEXT("[%d]: %s\n"), current, rcv.szMessage);
		}
	}

	return 0;
}


// -----------------

// funcao principal
// interaçao com o utilizador



int _tmain() {
	ContrData cdata;
	DWORD tid;
	HANDLE thnd;

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	_tprintf(TEXT("Servidor de Msg a iniciar .\n"));
	_tprintf(TEXT("Vou criar a memoria partilhada. \n"));

	cdata.hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,  // usar page file (=> sem ficheiro)
		NULL,					// default security
		PAGE_READWRITE,			// read/write
		0,						// maximum object size (high-order DWORD)
		MSGBUFSIZE,				// maximum object size (low-order DWORD)
		szName);				// name of mapping object

	if (cdata.hMapFile == NULL) {
		_tprintf(TEXT("ERR0  %d"), GetLastError());
		return 1;
	}
	_tprintf(TEXT("A Criar View da  memória mapeada\n"));

	cdata.pBuf = (Shared_MSG *)MapViewOfFile(cdata.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, MSGBUFSIZE);

	if (cdata.pBuf == NULL) {
		_tprintf(TEXT("ERROOO  %d  \n "), GetLastError());
		CloseHandle(cdata.hMapFile);
		return 1;
	}

	_tprintf(TEXT("OK, a enviar mensg \n"));
	iniMsgArea(cdata.pBuf); // serve para colocar uma mensgaem
							// inicial com numero 0

	writeMensagem(cdata.pBuf, TEXT("Server: zona de msg criada \n"));
	cdata.ThreadDeveContinuar = 1;

	thnd = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)listenerThread, (LPVOID)&cdata, 0, &tid);
	//thnd = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	if (thnd == NULL) {
		_tprintf(TEXT("[ERRO] Criação do listener thread\n"), GetLastError());
		return -1;
	}

	_tprintf(TEXT("Careegar numa tecla para fechar o server\n"));
	_getch();

	cdata.ThreadDeveContinuar = 0; // indicar à thread que deve temrinar
	WaitForSingleObject(thnd, INFINITE); // agiuardar que thread termine
	_tprintf(TEXT("Thread encerrada\n"));

	writeMensagem(cdata.pBuf, TEXT("Servidor: fechar"));
	UnmapViewOfFile(cdata.pBuf);
	CloseHandle(cdata.hMapFile);
	CloseHandle(thnd);
	return 0;

}