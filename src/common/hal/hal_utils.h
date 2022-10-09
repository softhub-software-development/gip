
//
//  hal_utils.h
//
//  Created by Christian Lehner on 17/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_UTILS_H
#define SOFTHUB_HAL_UTILS_H

#include "hal.h"
#ifdef PLATFORM_WIN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace SOFTHUB {
namespace HAL {

#ifdef PLATFORM_WIN

std::string encode_utf8(const std::string& str);
std::string encode_utf8(const std::wstring& wstr);
std::wstring decode_utf8(const std::string& str);
const TCHAR* decode_str(const std::string& str);
std::string format_last_error(DWORD err);
DWORD version_of(LPCTSTR dll_name);
DWORD version_of_shell_dll();
bool shell_version_lt(int major, int minor);
bool shell_version_ge(int major, int minor);
ularge cpuid();

#elif defined PLATFORM_APPLE
#ifdef __OBJC__

std::string encode_utf8(NSString* s);
NSString* decode_utf8(const std::string& s);
NSString* decode_mac_roman(const std::string& s);
NSDictionary* decode_json_dictionary(NSString* str, NSError* err = nil);
NSString* encode_json_dictionary(NSDictionary* dict, NSError* err = nil);
std::string find_resource(const std::string& filename, const std::string& type);

#endif
#else
// TODO: PLATFORM_LINUX
#endif

std::string get_platform();
std::string get_host_name();
std::string get_login_name();
std::string full_user_name();
std::string shell_exec(const std::string& cmd);
void generate_platform_id(char** buf, int* bufSize);
void os_version(std::string& platform, std::string& major, std::string& minor);
bool launch_browser(const std::string& url);

}}

#endif
