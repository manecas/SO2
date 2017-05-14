#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

TCHAR szName[] = TEXT("fmMsgSpace"); // nome para a memória mapeada

#define MSGSIZE 75

typedef struct _MSG {  // memoria partilhada terá uma estrutura destas
	unsigned msgnum;  // haverao mais coisas depois 
	TCHAR szMessage[75];
} Shared_MSG;

#define MSGBUFSIZE sizeof(Shared_MSG)


//----------------------------



// funções de escrita/leitura de mensagens
// sem sincronização (mas deverá ter - os locais assinaldas)

void iniMsgArea(Shared_MSG * shared) { // sem validação de tamanho
		// fechar mutex
	TCHAR inicio[] = TEXT("empty");
	shared->msgnum = 0;
	CopyMemory((PVOID)shared->szMessage, inicio, _tcslen(inicio) * sizeof(TCHAR));

	// Abrir Mutex
}


// Sem sincronização
unsigned writeMensagem(Shared_MSG * shared, TCHAR * msgtext) {
			// faltam algumas validações de tamanho 

	unsigned myNum;
	int numchars = _tcslen(msgtext);
	if (numchars > MSGSIZE - 1) {
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');  // método algo primitivo
	}

	// fechar mutex
	shared->msgnum++;
	myNum = shared->msgnum;
	_tcscpy_s(shared->szMessage, msgtext); // ou copyMemory
	// Abrir mutex 
	return myNum;
}


// sem sincronização (locais onde deve haver assinalados -> TPC
void readMensagem(Shared_MSG * shared, Shared_MSG * msg) {
	// fechar mutex
	shared->msgnum;
	CopyMemory(msg, shared, sizeof(Shared_MSG));
	// abrir mutex
}

// sem sincronização - locais onde deve haver estao assinalados -> TPC

unsigned peekMensagem(Shared_MSG * shared) {
	unsigned msgnum;
	//fechar mutex
	msgnum = shared->msgnum;
	//abrir mutex
	return msgnum;
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