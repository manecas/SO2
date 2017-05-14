#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include "FileMapping.h"

TCHAR szName[] = TEXT("fmMsgSpace");

unsigned writeMensagem(Shared_MSG * shared, TCHAR * msgtext)
{
	unsigned myNum;
	int numchars = _tcslen(msgtext);
	
	if(numchars > MSGSIZE - 1){
		numchars = MSGSIZE - 1;
		msgtext[MSGSIZE - 1] = _T('\0');
	}
	
	// Fechar mutex
	shared->msgnum++;
	myNum = shared->msgnum;
	_tcscpy(shared->szMessage, msgtext);
	// Abrir mutex
	
	return myNum;
}

void readMensagem(Shared_MSG * shared, Shared_MSG * msg)
{
	// Fechar mutex
	shared->msgnum;
	CopyMemory(msg, shared, sizeof(Shared_MSG));
	// Abrir mutex
}

unsigned peekMensagem(Shared_MSG * shared)
{
	unsigned msgnum;
	// Fechar mutex
	msgnum = shared->msgnum; // simples atribuição (cópia de estrututas)
	// Abrir mutex
	
	return msgnum;
}
