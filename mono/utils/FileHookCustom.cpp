// lulu FileHookCustom.cpp

#include <windows.h>
#include <Shlwapi.h>
#include <set>

extern "C"{
#include "lululog.h"
}

struct CUSTOMFILE{
	HANDLE realFile;
	WCHAR fileName[MAX_PATH];
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
	x_log(XLOG_OK,"FileHook CustomInit\n");
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

	x_log(XLOG_OK,"FileHook CustomRelease\n");
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

	wcsncpy(c->fileName,lpFileName,MAX_PATH);
	PathStripPathW(c->fileName);

	x_log(XLOG_LOG,"FileHook CreateFileW %S 0x%x\n",lpFileName,h);
	return h;
}

BOOL WINAPI CustomCloseHandle(HANDLE hObject)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hObject;
	x_log(XLOG_DEBUG,"FileHook CloseHandle 0x%x %S\n",hObject,custom->fileName);

	BOOL ret = ApiCloseHandle(custom->realFile);
	delete custom;
	g_customFileSet.erase(custom);
	return ret;
}

BOOL WINAPI CustomReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hFile;
	BOOL ret = ApiReadFile(custom->realFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
	x_log(XLOG_DEBUG,"FileHook ReadFile 0x%x Bytes:%d/%d %S\n",hFile,*lpNumberOfBytesRead,nNumberOfBytesToRead,custom->fileName);
	return ret;
}

DWORD WINAPI CustomGetFileType(HANDLE hFile)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hFile;
	x_log(XLOG_DEBUG,"FileHook GetFileType 0x%x %S\n",hFile,custom->fileName);
	return ApiGetFileType(custom->realFile);
}

DWORD WINAPI CustomGetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hFile;
	DWORD s = ApiGetFileSize(custom->realFile,lpFileSizeHigh);
	x_log(XLOG_DEBUG,"FileHook GetFileSize 0x%x size:%d %S\n",hFile,s,custom->fileName);
	return s;
}

DWORD WINAPI CustomSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	CUSTOMFILE *custom = (CUSTOMFILE*)hFile;
	x_log(XLOG_DEBUG,"FileHook SetFilePointer 0x%x dis:%d mode:%d %S\n",hFile,lDistanceToMove,dwMoveMethod,custom->fileName);
	return ApiSetFilePointer(custom->realFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod);
}

