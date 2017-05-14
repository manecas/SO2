#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#include <io.h>
#include <fcntl.h>

#define MAX 100
#define N 4 //Quantos semaforos

//includes dos exercícios anteriores
DWORD WINAPI ThreadProdutor(LPVOID param);
DWORD WINAPI ThreadConsumidor(LPVOID param);
HANDLE hSemaforo; //
HANDLE hEventoNovidade;
TCHAR frase[MAX];
BOOL nova = 0;

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR resp;
	DWORD threadId;
	HANDLE hThreadProd, hThreadCons[N];
	hSemaforo = CreateSemaphore(NULL, N, N, NULL); //Semaforo
	hEventoNovidade = CreateEvent(NULL, TRUE, FALSE, NULL);
	//3º false - true, começa a falso
	//UNICODE: Por defeito, a consola Windows não processe caracteres wide.
	//A maneira mais fácil para ter esta funcionalidade é chamar _setmode:
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("Lançar threads produtor-consumidor?"));
	_tscanf_s(TEXT("%c"), &resp, 1);
	if (resp == 'S' || resp == 's') {

		hThreadProd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProdutor, NULL, 0, &threadId);
		if (hThreadProd != NULL) {
			_tprintf(TEXT("Lancei uma thread com id %d\n"), threadId);
		}else {
			_tprintf(TEXT("Erro ao criar Thread\n"));
			return -1;
		}

		for (int i = 0; i < N; i++){
			hThreadCons[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadConsumidor, NULL, 0, &threadId);
			if (hThreadCons[i] != NULL) {
				_tprintf(TEXT("Lancei uma thread com id %d\n"), threadId);
			}else {
				_tprintf(TEXT("Erro ao criar Thread\n"));
				return -1;
			}
		}
		
		WaitForSingleObject(hThreadProd, INFINITE);
		WaitForSingleObject(hThreadCons, INFINITE);
	}
	_tprintf(TEXT("[Thread Principal %d]Finalmente vou terminar..."), GetCurrentThreadId());
	return 0;
}

DWORD WINAPI ThreadProdutor(LPVOID param) {
	TCHAR strLocal[MAX];
	_tprintf(TEXT("[Produtor]Sou a thread %d e vou começar a trabalhar ...\n Prima \'fim\' para terminar..."), GetCurrentThreadId());
	/*Sleep(100);*/

	do {
		_fgetts(strLocal, MAX, stdin);
		fflush(stdin);

		for (int i = 0; i < N; i++){
			WaitForSingleObject(hSemaforo, INFINITE);
		}
	
		_tcscpy_s(frase, MAX, strLocal);
		/*nova = TRUE;*/
		SetEvent(hEventoNovidade); //Mete a true
		ResetEvent(hEventoNovidade); //Mete a false
		ReleaseSemaphore(hSemaforo, N, NULL);
	} while (_tcsncmp(strLocal, TEXT("fim"), 3));

	return 0;
}

DWORD WINAPI ThreadConsumidor(LPVOID param) {
	TCHAR strLocal[MAX];
	_tprintf(TEXT("[Consumidor]Sou a thread %d e vou começar a trabalhar ...\n Prima \'fim\' para terminar..."), GetCurrentThreadId());
	/*Sleep(100);*/

	do {
		WaitForSingleObject(hEventoNovidade, INFINITE); //Espera pelo evento, e acorda, para não estar sempre a rodar...
	
		WaitForSingleObject(hSemaforo, INFINITE);

		/*if (nova) {*/
		_tprintf(TEXT("C"));
		_tcscpy_s(strLocal, MAX, frase);
		/*nova = FALSE;*/
		_tprintf(TEXT("[Consumidor-%d]:%s"), GetCurrentThreadId(), strLocal);
		/*}*/
		ReleaseSemaphore(hSemaforo, 1, NULL);
		/*Sleep(1000);*/
	} while (_tcsncmp(strLocal, TEXT("fim"), 3));

	return 0;
}