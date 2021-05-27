
//
//  hal_utils.cpp
//
//  Created by Christian Lehner on 17/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_utils.h"
#include <array>
#ifdef PLATFORM_WIN
#include "hal_strings.h"
#include <windows.h>
#include <winbase.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tchar.h>
#include "NB30.H"
#include <intrin.h>
#include <iphlpapi.h>
#include <lmcons.h>
#define PACKVERSION(major, minor) MAKELONG(minor, major)
#define PLATFORM_ID_SIZE (4 * sizeof(int))
#define HOST_NAME_MAX 32
#elif defined PLATFORM_MAC
#include <IOKit/IOKitLib.h>
#elif defined PLATFORM_LINUX
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <iostream>
#elif defined PLATFORM_IOS
#else
#error undefined platform
#endif
#include <time.h>
#ifdef PLATFORM_MAC
#include <libproc.h>
#include <stdio.h>
#include <string.h>
#endif

using namespace std;

namespace SOFTHUB {
namespace HAL {

#if FEATURE_HAL_UTILS
#ifdef PLATFORM_WIN

string get_login_name()
{
    TCHAR name[UNLEN + 1];
    DWORD name_len = UNLEN + 1;
    GetUserName(name, &name_len);
    return encode_utf8(name);
}

string full_user_name()
{
    return "TODO";
}

void os_version(string& platform, string& major, string& minor)
{
    OSVERSIONINFOEX info;
    ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
#pragma warning(disable : 4996)
    GetVersionEx((LPOSVERSIONINFO) &info);
    printf("Windows version: %u.%u\n", info.dwMajorVersion, info.dwMinorVersion);
    platform = "Windows";
    major = "TODO";
    minor = "TODO";
}

string encode_utf8(const string& str)
{
    return str;
}

string encode_utf8(const wstring& wstr)
{
    if (wstr.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int) wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int) wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

wstring decode_utf8(const string& str)
{
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

const TCHAR* decode_str(const string& str)
{
#ifdef _MBCS
    return str.c_str();
#else
    return decode_utf8(str).c_str();
#endif
}

string format_last_error(DWORD err)
{
    string result;
    TCHAR* buf;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD buf_len = FormatMessage(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buf, 0, 0);
    if (buf_len) {
        result = encode_utf8(buf);
        LocalFree(buf);
    }
    return result;
}

DWORD version_of(LPCTSTR dll_name)
{
    HINSTANCE hinst;
    DWORD version = 0;
    hinst = LoadLibrary(dll_name);
    if (hinst) {
        DLLGETVERSIONPROC version_func = (DLLGETVERSIONPROC) GetProcAddress(hinst, "DllGetVersion");
        if (version_func) {
            DLLVERSIONINFO dvi;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            HRESULT hr = (*version_func)(&dvi);
            if (SUCCEEDED(hr))
               version = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
        }
        FreeLibrary(hinst);
    }
    return version;
}

DWORD version_of_shell_dll()
{
    return version_of(_T("C:\\Windows\\System32\\Shell32.dll"));
}

bool shell_version_lt(int major, int minor)
{
    DWORD version = version_of_shell_dll();
    DWORD target_version = PACKVERSION(major, minor);
    return version < target_version;
}

bool shell_version_ge(int major, int minor)
{
    DWORD version = version_of_shell_dll();
    DWORD target_version = PACKVERSION(major, minor);
    return version >= target_version;
}

#if 0
ularge cpuid()
{
    DWORD dw1 = 0, dw2 = 0;
    DWORD support = 0;
    __asm {
       xor eax, eax
       inc eax
       cpuid
       mov support, edx
    }
    if (!(support & 0x00040000))
       throw exception("Processor/system does not provide a serial number");
    __asm {
       mov eax, 3
       cpuid
       mov dw1, ecx
       mov dw2, edx
    }
    ULARGE_INTEGER ul = {};
    ul.HighPart = dw2;
    ul.LowPart = dw1;
    if (!ul.QuadPart)
       throw exception("Processor/system does not provide a serial number");
    return ul.QuadPart;
}
#endif

ULONG hashMacAddress(PIP_ADAPTER_INFO info)
{
    ULONG i, hash = 0;
    for (i = 0; i < info->AddressLength; i++)
        hash += (info->Address[i] << ((i & 1) * 8));
    return hash;
}

BOOL readMacAddress(ULONG* mac)
{
    IP_ADAPTER_INFO info[32];
    DWORD dwBufLen = sizeof(info);

    DWORD dwStatus = GetAdaptersInfo(info, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
        return FALSE;
    *mac = hashMacAddress(info);
    return TRUE;
}

void generate_platform_id(char** buf, int* bufSize)
{
    const int id_size = 12;
    ULONG mac = 0;
	char name_buf[1024];
	u_long name_buf_len = 1024;
	union {
        int ival[4];
        char cval[PLATFORM_ID_SIZE];
    } info;

    memset(info.ival, 0, sizeof(info.ival));
    __cpuid(info.ival, 0);
    readMacAddress(&mac);
	memset(name_buf, 0, sizeof(name_buf));
    GetComputerNameA(name_buf, &name_buf_len);
	name_buf[0] ^= 'C';
	name_buf[1] ^= 'W';
    *buf = (char*) calloc(id_size+1, sizeof(char));
    (*buf)[0] = HEX((mac >> 28) & 0x0f);
    (*buf)[1] = HEX((mac >> 24) & 0x0f);
    (*buf)[2] = HEX((mac >> 20) & 0x0f);
    (*buf)[3] = HEX((mac >> 16) & 0x0f);
    (*buf)[4] = HEX((mac >> 12) & 0x0f);
    (*buf)[5] = HEX((mac >> 8) & 0x0f);
    (*buf)[6] = HEX((mac >> 4) & 0x0f);
    (*buf)[7] = HEX(mac & 0x0f);
    (*buf)[8] = HEX((name_buf[0] >> 4) & 0x0f);
    (*buf)[9] = HEX((info.cval[PLATFORM_ID_SIZE - 2] >> 4) & 0x0f);
    (*buf)[10] = HEX((name_buf[1] >> 4) & 0x0f);
    (*buf)[11] = HEX((info.cval[PLATFORM_ID_SIZE - 1] >> 4) & 0x0f);
    (*buf)[12] = '\0';
    assert((*buf)[id_size] == '\0');
    *bufSize = id_size;
}

#elif defined PLATFORM_MAC

void generate_platform_id(char** buf, int* bufSize)
{
    size_t slen;
    char* str;
    char tmp[128];
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    CFStringRef uuid = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);
    CFStringGetCString(uuid, tmp, sizeof(tmp), kCFStringEncodingMacRoman);
    str = strrchr(tmp, '-') + 1;
    slen = strlen(str);
    *buf = (char*) calloc(slen+1, sizeof(char));
    strncpy(*buf, str, slen);
    (*buf)[slen] = '\0';
    *bufSize = (int) slen;
    CFRelease(uuid);
}

#elif !defined PLATFORM_IOS

string full_user_name()
{
    return "TODO";
}

void generate_platform_id(char** buf, int* bufSize)
{
    char* tmp = (char*) calloc(5, sizeof(char));
    strcpy(tmp, "TODO");
    *buf = tmp;
    *bufSize = 4;
}

#endif

string get_platform()
{
#if defined PLATFORM_MAC
    return "macOS";
#elif defined PLATFORM_IOS
    return "iOS";
#elif defined PLATFORM_WIN
    return "Windows";
#elif defined PLATFORM_LINUX
    return "Linux";
#else
    assert(!"unknown platform");
    return "";
#endif
}

#ifdef PLATFORM_APPLE
#define HOST_NAME_MAX 128
#define LOGIN_NAME_MAX 128
#endif

string get_host_name()
{
    char buf[HOST_NAME_MAX];
    int err = gethostname(buf, HOST_NAME_MAX);
    return err ? "" : buf;
}

#ifdef PLATFORM_UNIX

string get_login_name()
{
    char buf[LOGIN_NAME_MAX];
    int err = getlogin_r(buf, sizeof(buf));
    return err ? "" : buf;
}

#endif

bool launch_browser(const string& url)
{
#ifdef PLATFORM_WIN
    return ShellExecute(NULL, _T("open"), decode_str(url), NULL, NULL, SW_SHOWNORMAL) == 0;
#elif defined PLATFORM_MAC
    string s = string("open \"") + url + "\"";
    return system(s.c_str()) == 0;
#elif defined PLATFORM_UNIX
    const char* browser = getenv("BROWSER");
    if (browser)
        return false;
    char* args[3];
    args[0] = (char*) browser;
    args[1] = (char*) url.c_str();
    args[2] = 0;
    pid_t pid = fork();
    if (!pid)
        execvp(browser, args);
#elif defined PLATFORM_IOS
    return false;
#else
#error "launch browser not supported"
#endif
}

#endif

#ifndef PLATFORM_WIN

string shell_exec(const string& cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

#endif

#ifdef PLATFORM_LINUX

void os_version(string& platform, string& major, string& minor)
{
    struct utsname buf;
    int success = uname(&buf);
    platform = buf.sysname;
    major = buf.release;
    minor = buf.version;
}

#endif

#ifdef PLATFORM_MAC

void find_processes_by_name(const string& process_name)
{
    pid_t pids[2048];
    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    int n = bytes / sizeof(pids[0]);
    for (int i = 0; i < n; i++) {
        struct proc_bsdinfo proc;
        int siz = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, &proc, PROC_PIDTBSDINFO_SIZE);
        if (siz == PROC_PIDTBSDINFO_SIZE) {
            if (process_name == proc.pbi_name) {
                printf("%d [%s] [%s]\n", pids[i], proc.pbi_comm, proc.pbi_name);                
            }
        }
    }
}

#endif
}}
