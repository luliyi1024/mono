// lulu FileHook.cpp

#include <windows.h>

const int HSIZE = 6;
bool g_Inited = false;

enum HOOKINDEX{
	HOOK_CreateFileW = 0,
	HOOK_CloseHandle,
	HOOK_ReadFile,
	HOOK_GetFileType,
	HOOK_SetFilePointer,
	HOOK_GetFileSize,
	HOOKCOUNT
};

struct HOOKFUNCTION{
	LPVOID pOrigFunction;
	BYTE oldBytes[HSIZE];
	BYTE JMP[HSIZE];
	DWORD oldProtect;
	DWORD _Protect;
};

HOOKFUNCTION g_Hooks[HOOKCOUNT];

void WINAPI CustomInit();
void WINAPI CustomRelease();
BOOL WINAPI IsCustomFile(LPCWSTR lpFileName);
BOOL WINAPI IsCustomHandle(HANDLE hObject);
HANDLE WINAPI CustomCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
BOOL WINAPI CustomCloseHandle(HANDLE hObject);
BOOL WINAPI CustomReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
DWORD WINAPI CustomGetFileType(HANDLE hFile);
DWORD WINAPI CustomSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod);
DWORD WINAPI CustomGetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh);

void DetourFunc(HOOKFUNCTION *pHook, LPCWSTR dll, LPCSTR funcname, LPVOID newFunction)
{
	pHook->pOrigFunction = GetProcAddress(GetModuleHandle(dll),funcname);
	if (pHook->pOrigFunction){
		pHook->oldProtect = PAGE_EXECUTE_READWRITE;
		pHook->_Protect = PAGE_EXECUTE_READWRITE;

		BYTE tempJMP[HSIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3};
		memcpy(pHook->JMP, tempJMP, HSIZE);
		DWORD JMPSize = ((DWORD)newFunction - (DWORD)pHook->pOrigFunction - 5);
		VirtualProtect(pHook->pOrigFunction, HSIZE, 
						PAGE_EXECUTE_READWRITE, &pHook->oldProtect);
		memcpy(pHook->oldBytes, pHook->pOrigFunction, HSIZE);
		memcpy(&(pHook->JMP[1]), &JMPSize, 4);
		memcpy(pHook->pOrigFunction, pHook->JMP, HSIZE);
		VirtualProtect(pHook->pOrigFunction, HSIZE, pHook->oldProtect, NULL);
	}
}

void BeginOrig(HOOKFUNCTION *pHook)
{
	VirtualProtect(pHook->pOrigFunction, HSIZE, pHook->_Protect, NULL);
    memcpy(pHook->pOrigFunction, pHook->oldBytes, HSIZE);
}

void EndOrig(HOOKFUNCTION *pHook)
{
    memcpy(pHook->pOrigFunction, pHook->JMP, HSIZE);
	VirtualProtect(pHook->pOrigFunction, HSIZE, pHook->oldProtect, NULL);
}

////////////////////
// orig api
HANDLE WINAPI ApiCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	BeginOrig(&g_Hooks[HOOK_CreateFileW]);
	HANDLE h = CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	EndOrig(&g_Hooks[HOOK_CreateFileW]);
	return h;
}

BOOL WINAPI ApiCloseHandle(HANDLE hObject)
{
	BeginOrig(&g_Hooks[HOOK_CloseHandle]);
	BOOL ret = CloseHandle(hObject);
	EndOrig(&g_Hooks[HOOK_CloseHandle]);
	return ret;
}

BOOL WINAPI ApiReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	BeginOrig(&g_Hooks[HOOK_ReadFile]);
	BOOL ret = ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
	EndOrig(&g_Hooks[HOOK_ReadFile]);
	return ret;
}

DWORD WINAPI ApiGetFileType(HANDLE hFile)
{
	BeginOrig(&g_Hooks[HOOK_GetFileType]);
	DWORD ret = GetFileType(hFile);
	EndOrig(&g_Hooks[HOOK_GetFileType]);
	return ret;
}

DWORD WINAPI ApiSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	BeginOrig(&g_Hooks[HOOK_SetFilePointer]);
	DWORD ret = SetFilePointer(hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod);
	EndOrig(&g_Hooks[HOOK_SetFilePointer]);
	return ret;
}

DWORD WINAPI ApiGetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh)
{
	BeginOrig(&g_Hooks[HOOK_GetFileSize]);
	DWORD ret = GetFileSize(hFile,lpFileSizeHigh);
	EndOrig(&g_Hooks[HOOK_GetFileSize]);
	return ret;
}

////////////////
HANDLE WINAPI _CreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	return IsCustomFile(lpFileName) ?
		CustomCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile)
		: ApiCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

BOOL WINAPI _CloseHandle(HANDLE hObject)
{
	return IsCustomHandle(hObject) ? CustomCloseHandle(hObject)	: ApiCloseHandle(hObject);
}

BOOL WINAPI _ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	return IsCustomHandle(hFile) ? 
		CustomReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)	
		: ApiReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
}

DWORD WINAPI _GetFileType(HANDLE hFile)
{
	return IsCustomHandle(hFile) ? CustomGetFileType(hFile) : ApiGetFileType(hFile);
}

DWORD WINAPI _GetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh)
{
	return IsCustomHandle(hFile) ? CustomGetFileSize(hFile,lpFileSizeHigh) : ApiGetFileSize(hFile,lpFileSizeHigh);
}

DWORD WINAPI _SetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	return IsCustomHandle(hFile) ? 
		CustomSetFilePointer(hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod)
		: ApiSetFilePointer(hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod);
}

/////////////
extern "C" void FileHookInit()
{
	if (g_Inited) return;

	CustomInit();

	memset(&g_Hooks,0,sizeof(g_Hooks));
	DetourFunc(&g_Hooks[HOOK_CreateFileW],L"kernel32.dll","CreateFileW",_CreateFileW);
	DetourFunc(&g_Hooks[HOOK_CloseHandle],L"kernel32.dll","CloseHandle",_CloseHandle);
	DetourFunc(&g_Hooks[HOOK_ReadFile],L"kernel32.dll","ReadFile",_ReadFile);
	DetourFunc(&g_Hooks[HOOK_GetFileType],L"kernel32.dll","GetFileType",_GetFileType);
	DetourFunc(&g_Hooks[HOOK_SetFilePointer],L"kernel32.dll","SetFilePointer",_SetFilePointer);
	DetourFunc(&g_Hooks[HOOK_GetFileSize],L"kernel32.dll","GetFileSize",_GetFileSize);
	g_Inited = true;
}

extern "C" void FileHookRelease()
{
	if (!g_Inited) return;
	for(int i=0;i<HOOKCOUNT;++i){
		HOOKFUNCTION *pHook = &(g_Hooks[i]);
		if (pHook && pHook->pOrigFunction){
			memcpy(pHook->pOrigFunction,pHook->oldBytes,HSIZE);
		}
	}

	CustomRelease();
	g_Inited = false;
}
