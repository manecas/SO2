#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#include <Windows.h>
#define MSGSIZE 75

extern "C"
{
	DLL_IMP_API TCHAR szName[]; // Nome da zona de memória partilhada

	DLL_IMP_API typedef struct _MSG {
		unsigned msgnum;
		TCHAR szMessage[MSGSIZE];
	} Shared_MSG;

	#define MSGBUFSIZE sizeof(Shared_MSG)

	DLL_IMP_API unsigned writeMensagem(Shared_MSG * shared, TCHAR * msgtext);
	DLL_IMP_API void readMensagem(Shared_MSG * shared, Shared_MSG * msg);
	DLL_IMP_API unsigned peekMensagem(Shared_MSG * shared);
} 