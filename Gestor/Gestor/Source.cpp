#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

TCHAR szName[] = TEXT("fmMsgSpace"); // nome para a mem�ria mapeada

#define MSGSIZE 75

typedef struct _MSG {  // memoria partilhada ter� uma estrutura destas
	unsigned msgnum;  // haverao mais coisas depois 
	TCHAR szMessage[75];
} Shared_MSG;

#define MSGBUFSIZE sizeof(Shared_MSG)


//----------------------------



// fun��es de escrita/leitura de mensagens
// sem sincroniza��o (mas dever� ter - os locais assinaldas)

void iniMsgArea(Shared_MSG * shared) { // sem valida��o de tamanho
		// fechar mutex
	TCHAR inicio[] = TEXT("empty");
	shared->msgnum = 0;
	CopyMemory((PVOID)shared->szMessage, inicio, _tcslen(inicio) * sizeof(TCHAR));

	// Abrir Mutex
}


// Sem sincroniza��o
unsigned writeMensagem(Shared_MSG * shared, TCHAR * msgtext) {
			// faltam algumas valida��es de tamanho 

	unsigned myNum;
	int numchars = _tcslen(msgtext);
	if (numchars > MSGSIZE - 1) {
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');  // m�todo algo primitivo
	}

	// fechar mutex
	shared->msgnum++;
	myNum = shared->msgnum;
	_tcscpy_s(shared->szMessage, msgtext); // ou copyMemory
	// Abrir mutex 
	return myNum;
}


// sem sincroniza��o (locais onde deve haver assinalados -> TPC
void readMensagem(Shared_MSG * shared, Shared_MSG * msg) {
	// fechar mutex
	shared->msgnum;
	CopyMemory(msg, shared, sizeof(Shared_MSG));
	// abrir mutex
}

// sem sincroniza��o - locais onde deve haver estao assinalados -> TPC

unsigned peekMensagem(Shared_MSG * shared) {
	unsigned msgnum;
	//fechar mutex
	msgnum = shared->msgnum;
	//abrir mutex
	return msgnum;
}






// --------------------------

// Thread de leitura de mensagens
// n�o usa vari�veis globais Calaro

typedef struct _ControlData {  // informa��o acerca do que a thread
	HANDLE hMapFile;			// deve fazer
	Shared_MSG * pBuf;
	int ThreadDeveContinuar;
} ContrData;


unsigned int _stdcall listenerThread(void * p) {
	ContrData * pcd = (ContrData *)p;
	unsigned int current = peekMensagem(pcd->pBuf);
	Shared_MSG rcv;

	while(pcd->ThreadDeveContinuar) {
		Sleep(500); // obviamente, uma solu��o fraca -> usar sincorniza��o obviamente 
					// (fazer como exercicio): sugest�o -> eventos 

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
// intera�ao com o utilizador



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
	_tprintf(TEXT("A Criar View da  mem�ria mapeada\n"));

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
		_tprintf(TEXT("[ERRO] Cria��o do listener thread\n"), GetLastError());
		return -1;
	}

	_tprintf(TEXT("Careegar numa tecla para fechar o server\n"));
	_getch();

	cdata.ThreadDeveContinuar = 0; // indicar � thread que deve temrinar
	WaitForSingleObject(thnd, INFINITE); // agiuardar que thread termine
	_tprintf(TEXT("Thread encerrada\n"));

	writeMensagem(cdata.pBuf, TEXT("Servidor: fechar"));
	UnmapViewOfFile(cdata.pBuf);
	CloseHandle(cdata.hMapFile);
	CloseHandle(thnd);
	return 0;

}