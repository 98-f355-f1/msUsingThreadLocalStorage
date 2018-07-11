/*
USING THREAD LOCAL STORAGE
Thread Local Storage (TLS) enables multiple threads of the same process to use an index
allocated by the TlsAlloc function to store and retrieve a value that is local to the
thread. In this example, an index is allocated when the process starts. When each thread
starts, it allocates a block of dynamic memory and stores a pointer to this memory in
the TLS slot using the TlsSetValue function. The CommonFunc function uses the
TlsGetValue function to access the data associated with the index that is local to the
calling thread. Before each thread terminates, it releases its dynamic memory. Before the
process terminates, it calls TlsFree to release the index.
*/

#include <windows.h>
#include <stdio.h>
//#include <unistd.h>

#define THREADCOUNT 4
DWORD dwTlsIndex;

VOID ErrorExit(LPSTR);

VOID CommonFunc(VOID)
{
	LPVOID lpvData;
	static const char* allocfail = "TlsSetValue error\n";

	// retrieve a data pointer for the thread	
	lpvData = TlsGetValue(dwTlsIndex);
	if ((lpvData == 0) && (GetLastError() != ERROR_SUCCESS))
		ErrorExit((LPSTR)allocfail);

	// use the data stored for the current thread
	printf("common: thread %d: lpvData-%lx\n", GetCurrentThreadId(), (ULONG)lpvData);

	Sleep(50); // was 5000
}

DWORD WINAPI ThreadFunc(VOID)
{
	LPVOID lpvData;
	static const char* allocfail = "TlsSetValue error\n";

	// initialize the TLS index for this thread
	lpvData = (LPVOID)LocalAlloc(LPTR, 256);
	if (!TlsSetValue(dwTlsIndex, lpvData))
		ErrorExit((LPSTR)allocfail);

	printf("thread %d: lpvData-%lx\n", GetCurrentThreadId(), (ULONG)lpvData);

	CommonFunc();

	// release the dynamic memor before thread returns
	lpvData = TlsGetValue(dwTlsIndex);
	if (lpvData != 0)
		LocalFree((HLOCAL)lpvData);

	return (EXIT_SUCCESS);
}

int main(VOID)
{
	DWORD IDThread;
	HANDLE hThread[THREADCOUNT];
	static const char* allocfail = "TlsAlloc failed\n";
	static const char* threadfail = "CreateThread failed\n";
	int i;

	// allocate a TLS index
	if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		ErrorExit((LPSTR)allocfail);


	// create multiple threads
	for (i = 0; i < THREADCOUNT; i++)
	{
		hThread[i] = CreateThread(NULL, 						// default security attributes
			0,								// use default stack size
			(LPTHREAD_START_ROUTINE)ThreadFunc,
			NULL, 						// no thread function argument
			0,								// use default creation flags
			&IDThread);

		// check the return value for success
		if (hThread[i] == NULL)
			ErrorExit((LPSTR)threadfail);
	}

	for (i = 0; i < THREADCOUNT; i++)
		WaitForSingleObject(hThread[i], INFINITE);

	TlsFree(dwTlsIndex);
	getchar();
	return (EXIT_SUCCESS);
}

VOID ErrorExit(LPSTR lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);
	ExitProcess(0);
}
