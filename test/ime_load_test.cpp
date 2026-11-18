#include "../targetver.h"
#include <windows.h>
#include <stdio.h>
#include "../ime/immdev.h"

typedef BOOL (WINAPI *ImeInquireProc)(LPIMEINFO, LPTSTR, DWORD);

struct LoadData {
    const wchar_t *path;
    HMODULE module;
};

static DWORD WINAPI LoadIme(void *parameter)
{
    LoadData *data = static_cast<LoadData *>(parameter);
    data->module = LoadLibraryW(data->path);
    return data->module != NULL;
}

static DWORD WINAPI UnloadIme(void *parameter)
{
    return FreeLibrary(static_cast<HMODULE>(parameter));
}

static DWORD WINAPI InquireIme(void *parameter)
{
    ImeInquireProc inquire = reinterpret_cast<ImeInquireProc>(parameter);
    IMEINFO info = {};
    TCHAR className[64] = {};
    return inquire(&info, className, IME_SYSINFO_WINLOGON) &&
           lstrcmp(className, TEXT("MZIMEUI")) == 0 &&
           (info.fdwProperty & IME_PROP_UNICODE) != 0;
}

static void WaitForSuccess(HANDLE thread, DWORD timeout, const char *operation)
{
    DWORD result = WaitForSingleObject(thread, timeout);
    DWORD exitCode = 0;
    if (result != WAIT_OBJECT_0 || !GetExitCodeThread(thread, &exitCode) || !exitCode) {
        fprintf(stderr, "%s failed or timed out (%lu)\n", operation, result);
        TerminateProcess(GetCurrentProcess(), 1);
    }
    CloseHandle(thread);
}

int wmain(int argc, wchar_t **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Expected the path to mzimeja.ime\n");
        return 1;
    }

    HANDLE mutex = CreateMutexW(NULL, FALSE, L"BasicDictObject");
    if (!mutex || WaitForSingleObject(mutex, 3000) != WAIT_OBJECT_0)
        return 1;

    LoadData data = { argv[1], NULL };
    HANDLE thread = CreateThread(NULL, 0, LoadIme, &data, 0, NULL);
    if (!thread)
        return 1;
    WaitForSuccess(thread, 3000, "LoadLibrary while dictionary mutex is held");
    ReleaseMutex(mutex);

    ImeInquireProc inquire = reinterpret_cast<ImeInquireProc>(
        GetProcAddress(data.module, "ImeInquire"));
    if (!inquire)
        return 1;
    HANDLE threads[2];
    for (int i = 0; i < 2; ++i) {
        threads[i] = CreateThread(NULL, 0, InquireIme,
                                  reinterpret_cast<void *>(inquire), 0, NULL);
        if (!threads[i])
            return 1;
    }
    for (int i = 0; i < 2; ++i)
        WaitForSuccess(threads[i], 30000, "Concurrent ImeInquire");

    HANDLE mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, L"BasicDictObjectFileMapping");
    if (!mapping) {
        fprintf(stderr, "Basic dictionary was not loaded\n");
        return 1;
    }
    CloseHandle(mapping);

    if (WaitForSingleObject(mutex, 3000) != WAIT_OBJECT_0)
        return 1;
    thread = CreateThread(NULL, 0, UnloadIme, data.module, 0, NULL);
    if (!thread)
        return 1;
    WaitForSuccess(thread, 3000, "FreeLibrary while dictionary mutex is held");
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return 0;
}
