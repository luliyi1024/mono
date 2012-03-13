// lulu FileHookCustom.cpp

#include <windows.h>
#include <set>

extern "C"{
#include "lululog.h"
}

struct CUSTOMFILE{
	HANDLE realFile;
};

std::set<CUSTOMFILE*> g_customFileSet;

HANDLE WINAPI ApiCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
BOOL WINAPI ApiCloseHandle(HANDLE hObject);
BOOL WINAPI ApiReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
DWORD WINAPI ApiGetFileType(HANDLE hFile);
DWORD WINAPI ApiSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod);
DWORD WINAPI ApiGetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh);

void WINAPI CustomInit()
{
	x_log(XLOG_LOG,"FileHook CustomInit\n");
}

void WINAPI CustomRelease()
{
	std::set<CUSTOMFILE*>::iterator item = g_customFileSet.begin();
	while(item != g_customFileSet.end()){
		if ((*item)->realFile) {
			CloseHandle((*item)->realFile);
		}
		delete (*item);
		item++;
	}
	g_customFileSet.clear();

	x_log(XLOG_LOG,"FileHook CustomRelease\n");
}

BOOL WINAPI IsCustomFile(LPCWSTR lpFileName)
{
	BOOL custom = false;
	size_t len = wcslen(lpFileName);
	if (len > 8){
		if (0 == _wcsnicmp(lpFileName+len-8,L".unity3d",8)){
			custom = true;
		}
	}
	return custom;
}

BOOL WINAPI IsCustomHandle(HANDLE hObject)
{
	return (g_customFileSet.find((CUSTOMFILE*)hObject) != g_customFileSet.end());
}

HANDLE WINAPI CustomCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	CUSTOMFILE *c = new CUSTOMFILE;
	c->realFile = ApiCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	g_customFileSet.insert(c);
	HANDLE h = (HANDLE)c;
	x_log(XLOG_LOG,"CreateFileW %S 0x%x\n",lpFileName,h);
	return h;
}

BOOL WINAPI CustomCloseHandle(HANDLE hObject)
{
	x_log(XLOG_LOG,"CloseHandle 0x%x\n",hObject);

	CUSTOMFILE *custom = (CUSTOMFILE*)hObject;
	BOOL ret = ApiCloseHandle(custom->realFile);
	delete custom;
	g_customFileSet.erase(custom);
	return ret;
}

BOOL WINAPI CustomReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hFile;
	BOOL ret = ApiReadFile(custom->realFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
	x_log(XLOG_LOG,"ReadFile (hFile:0x%x, Bytes:%d)\n",hFile,nNumberOfBytesToRead);
	return ret;
}

DWORD WINAPI CustomGetFileType(HANDLE hFile)
{
	x_log(XLOG_LOG,"GetFileType 0x%x\n",hFile);
	return ApiGetFileType(((CUSTOMFILE*)hFile)->realFile);
}

DWORD WINAPI CustomGetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh)
{
	x_log(XLOG_LOG,"GetFileSize 0x%x\n",hFile);
	return ApiGetFileSize(((CUSTOMFILE*)hFile)->realFile,lpFileSizeHigh);
}

DWORD WINAPI CustomSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	x_log(XLOG_LOG,"SetFilePointer 0x%x\n",hFile);
	return ApiSetFilePointer(((CUSTOMFILE*)hFile)->realFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod);
}

