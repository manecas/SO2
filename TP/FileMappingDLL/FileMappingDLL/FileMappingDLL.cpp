#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include "FileMappingDLL.h"

TCHAR fileMappingName[] = TEXT("Memory");

FileMapping::FileMapping()
{
}

FileMapping::FileMapping(TCHAR name[], Command * cmd) {
	_tprintf(TEXT("Cliente de Msg a iniciar.\n"));
	_tprintf(TEXT("Vou abrir a memoria partilhada.\n"));

	if (cmd == nullptr) {
		return;
	}

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,		// read/write access
		FALSE,						// n�o herdar o nome
		name);					// nome do objecto fich. mapeado

	if (hMapFile == NULL) {
		_tprintf(TEXT("A mem�ria partilhada deu complica��es (%d). At� amanh�.\n"), GetLastError());
		return;
	}

	_tprintf(TEXT("Vou criar a view da memoria partilhada.\n"));

	cmd = (Command *)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,				// Permiss�es read/write
		0,
		0,
		sizeof(Command));

	if (hMapView == NULL) {
		_tprintf(TEXT("A view da mem�ria partilhada deu azar (erro %d).\n"), GetLastError());
		CloseHandle(hMapFile);
		return;
	}
}

FileMapping::~FileMapping(){
	UnmapViewOfFile(hMapView);
	CloseHandle(hMapFile);
}

unsigned FileMapping::writeMensagem(wstring msgtext)
{
	unsigned myNum;
	int numchars = msgtext.size;
	
	// Fechar mutex
	readIndex++;
	myNum = readIndex;
	cmd->message = msgtext;
	// Abrir mutex
	
	return myNum;
}

void FileMapping::readMensagem(FileMapping * msg)
{
	// Fechar mutex
	readIndex--;
	CopyMemory(msg, this, sizeof(FileMapping));
	// Abrir mutex
}

unsigned FileMapping::peekMensagem()
{
	unsigned msgnum;
	// Fechar mutex
	msgnum = readIndex; // simples atribui��o (c�pia de estrututas)
	// Abrir mutex
	
	return msgnum;
}
