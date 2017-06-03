#define _CRT_SECURE_NO_WARNINGS

// SERVIDOR


#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>



 // DEFINES E NOME DA MEMORIA PARTILHADA
#define MSGSIZE 75
TCHAR szName[] = TEXT("memoria"); // Nome da zona de memória partilhada
TCHAR nomeEvento[] = TEXT("evento");

// NOME DOS SEMAFOROS
TCHAR semaforoEscritaName[] = TEXT("escrita");
TCHAR semaforoLeituraName[] = TEXT("leitura");
#define N 2 //Quantos semaforos


// ESTRUTURAS SHARED_MSG E CONTROLDATA (PARA A THREAD)
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



typedef struct _ControlData {  // informação acerca do que a thread
							   // deve fazer
	
	HANDLE hMapFile;	// HANDLE PARA A MEMORIA PARTILHADA
	Shared_MEM * pBuf;  // HANDLE PARA A ESTRUTURA SHARED_MSG QUE ESTA NA MEMORIA PARTILHADA

	int ThreadDeveContinuar;

	HANDLE hSemaforoEscrita;
	HANDLE hSemaforoLeitura;
	HANDLE hEventoLe;

} ContrData;



// FUNCAO APENAS PARA SACAR O NUMERO DA MENSAGEM
unsigned peekMensagem(Shared_MEM * shared)
{
	unsigned msgnum;
	// Fechar mutex
	msgnum = shared->mensagens[ shared->indice_leitura].msgnum; // simples atribuição (cópia de estrututas)
							 // Abrir mutex
	return msgnum;
}



// FUNCAO PARA LER DA MEMORIA PARTILHADA
void readMensagem(Shared_MEM * shared, Shared_MEM * msg)
{
	// Fechar mutex
	///shared->msgnum;

	// COPIA PARA MSG O QUE ESTA NA MEMORIA PARTILHADA
	CopyMemory(msg, shared, sizeof(Shared_MEM));
	// Abrir mutex
}





// FUNCAO DA THREAD LISTENER
unsigned int _stdcall listenerThread(void * p) {
	ContrData * pcd = (ContrData *)p;



	unsigned int current = peekMensagem(pcd->pBuf);
	Shared_MEM rcv;

	while (pcd->ThreadDeveContinuar) {

		WaitForSingleObject(pcd->hEventoLe, INFINITE);
		//Sleep(500); // obviamente, uma solução fraca -> usar sincornização obviamente 
					// (fazer como exercicio): sugestão -> eventos 

		WaitForSingleObject(pcd->hSemaforoLeitura, INFINITE);
		

		///if (peekMensagem(pcd->pBuf) > current) {
			readMensagem(pcd->pBuf, &rcv);
			current = rcv.mensagens[ rcv.indice_leitura ].msgnum;
			_tprintf(TEXT("INDICE STRUCT %d    [%d]: MENSAGEM: %s   PROCESSID %d     TECLA %c \n"), rcv.mensagens[rcv.indice_leitura].nMSG, current, rcv.mensagens[rcv.indice_leitura].szMessage, rcv.mensagens[rcv.indice_leitura].procId, rcv.mensagens[rcv.indice_leitura].tecla);
		///}

		pcd->pBuf->indice_leitura++;
		if (pcd->pBuf->indice_leitura == N)
			pcd->pBuf->indice_leitura = 0;

		ReleaseSemaphore(pcd->hSemaforoEscrita,1,NULL);
	}

	return 0;
}



// FUNCAO PARA ESCREVER MENSAGEM NA MEMORIA PARTILHADA
unsigned writeMensagem(Shared_MEM * shared, TCHAR * msgtext)
{
	unsigned myNum;
	int numchars = _tcslen(msgtext);

	if (numchars > MSGSIZE - 1) {
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');
	}

	// Fechar mutex
	shared->mensagens[ shared->indice_leitura].msgnum++;
	myNum = shared->mensagens[shared->indice_leitura].msgnum;
	_tcscpy(shared->mensagens[shared->indice_leitura].szMessage, msgtext);
	// Abrir mutex
	return myNum;
}


// FUNCAO PARA INICIALIZAR ESTRUTURA SHARED_MSG
void iniMsgArea(Shared_MEM * shared) { // sem validação de tamanho
									   // fechar mutex
	TCHAR inicio[] = TEXT("empty");
	shared->mensagens[shared->indice_leitura].msgnum = 0;
	CopyMemory((PVOID)shared->mensagens[shared->indice_leitura].szMessage, inicio, _tcslen(inicio) * sizeof(TCHAR));

	// Abrir Mutex
}



////////////////////////          MAIN       //////////////////////////////////////



int _tmain() {
	

	// VARIAVEIS
	DWORD tid;
	HANDLE thnd;

	

	// AGORA ESTAS VARIAVEIS ESTÃO NO CONTROLDATA QUE E' PASSADO A THREAD LISTENER
	/*     
	HANDLE hMapFile;
	Shared_MSG * pBuf;
	int ThreadDeveContinuar;
	*/
	ContrData cdata;



	// UNICODE
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif



	// INICIO 
	_tprintf(TEXT("Servidor de Msg a iniciar .\n"));
	_tprintf(TEXT("Vou criar a memoria partilhada. \n"));

	
	cdata.hSemaforoEscrita = CreateSemaphore(NULL, N, N, semaforoEscritaName); //Semaforo
	cdata.hSemaforoLeitura = CreateSemaphore(NULL, 0, N, semaforoLeituraName); //Semaforo


	cdata.hEventoLe =  CreateEvent(NULL, TRUE, FALSE, nomeEvento);

	


	// ALOCA MEMORIA PARTILHADA
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


	// CRIA VISTA PARA A MEMORIA PARTILHADA 
	cdata.pBuf = (Shared_MEM *)MapViewOfFile(cdata.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, MSGBUFSIZE);

	if (cdata.pBuf == NULL) {
		_tprintf(TEXT("ERROOO  %d  \n "), GetLastError());
		CloseHandle(cdata.hMapFile);
		return 1;
	}

	_tprintf(TEXT("OK, a enviar mensg \n"));

	// TEM DE ESTAR AQUI A INICIALIZAÇÂO 
	cdata.pBuf->indice_escrita = 0;
	cdata.pBuf->indice_leitura = 0;

	for (int i = 0; i < N; i++)
		cdata.pBuf->mensagens[i].nMSG = i;

	// INICIALIZA A ESTRUTURA DO TIPO SHARED_MSG DA MEMORIA PARTILHADA
	iniMsgArea(cdata.pBuf); // serve para colocar uma mensgaem
							// inicial com numero 0


	// ESCREVE OUTRA MENSGAEM NA MEMORIA PARTILHADA
	writeMensagem(cdata.pBuf, TEXT("Server: zona de msg criada \n"));
	cdata.ThreadDeveContinuar = 1;



	// CRIA THREAD LISTENER E PASSA-LHE UM PONTEIRO PARA A ESTRUTURA CDATA QUE CONTEM INFORMACAO SOBRE A MEMORIA PARTILHADA
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
