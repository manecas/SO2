#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#include <Windows.h>
#include <string>

using std::wstring;

#define MSGSIZE 75
#define MSGBUFSIZE sizeof(FileMapping)

DLL_IMP_API TCHAR fileMappingName[];

extern "C"
{
	DLL_IMP_API class Command {
		public:
			wstring message;
	};

	DLL_IMP_API class FileMapping {
	public:
		LPWSTR nomeSemaforoPodeEscrever;
		LPWSTR nomeSemaforoPodeLer;
		HANDLE podeEscrever;
		HANDLE podeLer;

		HANDLE hMapFile;
		HANDLE hMapView;
		wstring name; // Nome da zona de memória partilhada
		Command *cmd;
		unsigned readIndex;
		unsigned writeIndex;
	
		FileMapping();
		FileMapping(TCHAR name[], Command * cmd);
		~FileMapping();

		unsigned writeMensagem(wstring msgtext);
		void readMensagem(FileMapping * msg);
		unsigned peekMensagem();
	};
} 