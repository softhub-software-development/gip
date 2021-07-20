
//
//  hal_filesystem.cpp
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_filesystem.h"
#include "hal_strings.h"
#include "hal_utils.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#ifdef PLATFORM_WIN
#include "hal_directory_win.h"
#include <shlobj.h>
#include <atlbase.h>
#include <comdef.h>
#include <wbemidl.h>
#define chmod _chmod
#define rmdir _rmdir
#define ssize_t SSIZE_T
#pragma comment(lib, "wbemuuid.lib")
#else
#include <errno.h>
#include <unistd.h>
#define O_RAW 0
#endif
#ifdef PLATFORM_MAC
#include <sys/mount.h>
#elif defined(PLATFORM_UNIX)
#include <fstab.h>
#endif

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

//
// class File_path
//

bool File_path::ensure_tail(const string& filename, bool& create_flag, mode_t mode)
{
    bool success = true;
    struct stat st;
    int result = ::stat(filename.c_str(), &st);
    if (result != 0)
        success = make_directory(filename, create_flag, mode);
    return success;
}

bool File_path::ensure(mode_t mode)
{
    bool create_flag = false;
    return ensure_dir(path, "", create_flag, mode);
}

bool File_path::ensure(const string& sub_path, mode_t mode)
{
    bool create_flag = false;
    return ensure_dir(path, sub_path, create_flag, mode);
}

bool File_path::ensure(const string& sub_path, bool& create_flag, mode_t mode)
{
    return ensure_dir(path, sub_path, create_flag, mode);
}

bool File_path::ensure(const string& filename, const string& content, mode_t mode)
{
    return ensure_dir(path, filename, content, mode);
}

bool File_path::ensure_dir(const string& path, mode_t mode)
{
    bool create_flag = false;
    return ensure_dir(path, "", create_flag, mode);
}

bool File_path::ensure_dir(const string& path, const string& sub_path, bool& create_flag, mode_t mode)
{
    string full_path = path;
    if (!sub_path.empty())
        full_path = concat(path, sub_path);
    struct stat st;
    int result = ::stat(full_path.c_str(), &st);
    if (result == 0) {
        create_flag = false;
        return true;
    }
    bool success = true;
    size_t head = 0;
#ifdef PLATFORM_WIN
    if ((head = path.find_first_of(":", 0)) != string::npos)
        head = head + 1;
#endif
    size_t a = head, b;
    while ((b = full_path.find_first_of("/\\", a)) != string::npos) {
        string element = full_path.substr(head, b-head);
        if (!element.empty()) {
            if (!ensure_tail(element, create_flag, mode))
                return false;
        }
        a = b + 1;
    }
    string tail = full_path.substr(a);
    if (!tail.empty())
        success = ensure_tail(full_path, create_flag, mode);
    return success;
}

bool File_path::ensure_dir(const string& path, const string& filename, const string& content, mode_t mode)
{
    string full_path = path + filename;
    ifstream ifs;
    ifs.open(full_path.c_str(), fstream::in);
    if (!ifs.fail())
        return false;
    ofstream ofs;
    const char* s = full_path.c_str();
    ofs.open(s, fstream::out);
    if (ofs.fail())
        return false;
    chmod(s, mode);
    ofs << content << endl;
    ofs.close();
    return true;
}

string File_path::concat(const string& filename)
{
    string result = path;
    if (!Strings::ends_with(path, separator))
        result += separator;
    result += filename;
    return result;
}

string File_path::get_base_path() const
{
    return basepath_of(path);
}

string File_path::get_filename() const
{
    return filename_of(path);
}

string File_path::get_extension() const
{
    return extension_of(path);
}

// TODO: make this return the correct create_flag state
bool File_path::make_directory(const string& path, bool& create_flag, mode_t mode)
{
#ifdef PLATFORM_WIN
    bool success = create_flag = CreateDirectory(decode_str(path), NULL) == TRUE;
    if (!success) {
        DWORD err = GetLastError();
        success = err == ERROR_ALREADY_EXISTS;
    }
    return success;
#else
    bool success = create_flag = ::mkdir(path.c_str(), mode) == 0;
    if (!success)
        success = errno == EEXIST;
    return success;
#endif
}

bool File_path::copy_file(const string& src, const string& dst)
{
#ifdef PLATFORM_WIN
    assert(!"TODO");
    return false;
#else
    int src_fd = open(src.c_str(), O_RDONLY | O_RAW, 0);
    if (src_fd < 0)
        return false;
    int dst_fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_RAW, 0644);
    if (dst_fd < 0)
        return false;
    bool success = true;
    ssize_t num_read, num_written;
    const int buf_size = 65536;
    char buf[buf_size];
    while ((num_read = read(src_fd, buf, buf_size)) > 0) {
        num_written = write(dst_fd, buf, num_read);
        if (num_written != num_read) {
            success = false;
            break;
        }
    }
    close(src_fd);
    close(dst_fd);
    return success;
#endif
}

bool File_path::clone_file(const string& src, const string& dst)
{
    File_info src_info(src);
    File_time src_mod_time;
    bool success = src_info.read_file_modification_time(src_mod_time);
    if (!success)
        return false;
    File_info dst_info(dst);
    File_time dst_mod_time;
    success = dst_info.read_file_modification_time(dst_mod_time);
    if (success && FILE_MOD_TIME_EQUAL(src_mod_time, dst_mod_time))
        return true;
    success = copy_file(src, dst);
    if (!success)
        return false;
    return dst_info.write_file_modification_time(src_mod_time);
}

bool File_path::remove_file(const string& filepath)
{
    return remove(filepath.c_str()) == 0;
}

bool File_path::rename_file(const string& src, const string& dst)
{
#if 1 //_DEBUG
    int err = rename(src.c_str(), dst.c_str());
    if (err < 0)
        cout << "error renaming file: " << strerror(errno) << endl;
    return err == 0;
#else
    return rename(src.c_str(), dst.c_str()) == 0;
#endif
}

bool File_path::exists(const string& filepath)
{
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    return result == 0;
}

bool File_path::unique_filepath(string& filepath)
{
    if (!File_path::exists(filepath))
        return false;
    int num = 1;
    string path, name, ext;
    bool is_dir;
    parse(filepath, path, name, ext, is_dir);
    do {
        stringstream stream;
        stream << path;
        if (!name.empty())
            stream << separator << name;
        stream << num++;
        if (!ext.empty())
            stream << "." << ext;
        if (is_dir)
            stream << separator;
        filepath = stream.str();
    } while (File_path::exists(filepath));
    return true;
}

void File_path::parse(const string& filepath, string& path, string& basename, string& ext, bool& is_dir)
{
    // TODO: needs rewrite
    size_t len = filepath.length();
    size_t dot_pos = filepath.find_last_of(".");
    size_t end_pos = dot_pos == string::npos ? len : dot_pos;
    size_t sep_pos = filepath.find_last_of(separator);
    if (sep_pos == string::npos) {
        is_dir = false;
        path = "";
        basename = filepath.substr(0, end_pos);
    } else {
        is_dir = sep_pos == len - 1;
        if (is_dir)
            sep_pos = filepath.find_last_of(separator, len - 2);
        if (sep_pos == string::npos) {
            path = "";
            basename = filepath.substr(0, len - 1);
        } else {
            path = filepath.substr(0, sep_pos);
            basename = filepath.substr(sep_pos + 1, end_pos - sep_pos - 1);
        }
    }
    if (dot_pos == string::npos) {
        ext = "";
    } else {
        size_t ext_len = len - dot_pos - 1;
        if (is_dir)
            ext_len -= 1;
        ext = filepath.substr(dot_pos + 1, ext_len);
    }
}

string File_path::make_directory_path(const string& path)
{
    string dir = path;
    size_t dir_len = dir.length();
    if (dir_len > 0 && dir[dir_len-1] != separator)
        dir += separator;
    return dir;
}

string File_path::basepath_of(const string& path)
{
    size_t sep_pos = path.find_last_of(separator);
    if (sep_pos == string::npos)
        return "";
    return path.substr(0, sep_pos + 1);
}

string File_path::filename_of(const string& path)
{
    size_t sep_pos = path.find_last_of(separator);
    if (sep_pos == string::npos)
        return path;
    return path.substr(sep_pos + 1);
}

string File_path::basename_of(const string& path)
{
    size_t dot_pos = path.find_last_of(".");
    if (dot_pos == string::npos)
        dot_pos = path.length();
    size_t sep_pos = path.find_last_of(separator);
    if (sep_pos == string::npos)
        return path.substr(0, dot_pos);
    return path.substr(sep_pos + 1, dot_pos - sep_pos - 1);
}

string File_path::lastname_of(const string& path)
{
    size_t tail_pos = path.find_last_of(separator);
    if (tail_pos == string::npos)
        return path;
    size_t len = path.length();
    if (tail_pos < len - 1)
        return path.substr(tail_pos + 1);
    size_t head_pos = path.find_last_of(separator, tail_pos - 1);
    if (head_pos == string::npos)
        return path.substr(0, tail_pos);
    return path.substr(head_pos + 1, tail_pos - head_pos);
}

string File_path::extension_of(const string& path)
{
    size_t dot_pos = path.find_last_of(".");
    if (dot_pos == string::npos)
        return "";
    return path.substr(dot_pos + 1);
}

string File_path::concat(const string& path, const string& filename)
{
    string result = path;
    size_t len = path.length();
    if (len > 0 && path[len-1] != separator)
        result += separator;
    result += filename;
    return result;
}

string File_path::user_home()
{
#ifdef PLATFORM_WIN
    string home_drive = getenv("HOMEDRIVE");
    string home_dir = getenv("HOMEPATH");
    string home = home_drive + home_dir;
#else
    string home = getenv("HOME");
#endif
    return home;
}

string File_path::get_cwd()
{
#ifdef PLATFORM_WIN
    TCHAR buf[MAX_PATH];
    DWORD ret = GetCurrentDirectory(MAX_PATH, buf);
    string cwd;
    if (0 < ret && ret <= MAX_PATH)
        cwd = encode_utf8(buf);
#else
    char buf[PATH_MAX];
    const char* cwd = getcwd(buf, PATH_MAX);
#endif
    return cwd;
}

#ifdef PLATFORM_WIN

bool File_path::special_path(int cid, string& path, bool create_flag)
{
    TCHAR buf[MAX_PATH];
    BOOL success = SHGetSpecialFolderPath(0, buf, cid, (BOOL) create_flag);
    path = encode_utf8(buf);
    return success == TRUE;
}

#elif defined(PLATFORM_MAC)

bool File_path::special_path(const string& base, const string& filename, string& path, bool create_flag)
{
    bool success = true;
    path = base + separator + filename;
    if (create_flag) {
        File_path doc_path(path);
        success = doc_path.ensure();
    }
    return success;
}

#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
#else
#error unknown platform
#endif

bool File_path::app_data_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_APPDATA, path, create_flag);
#elif defined(PLATFORM_MAC)
    return app_support_path(path, create_flag);
#elif defined(PLATFORM_UNIX)
    // TODO: implement create request
    path = "/var/tmp";
    return true;
#elif defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::app_log_path(string& path, bool create_flag)
{
    string app_path;
    bool success = File_path::app_data_path(app_path, create_flag);
    if (!success)
        return false;
#ifdef PLATFORM_LINUX
    path = File_path::concat(app_path, "/gip/log/");
#else
    path = File_path::concat(app_path, "log/");
#endif
    return true;
}

bool File_path::my_documents_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_MYDOCUMENTS, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path(user_home(), "Documents", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::my_pictures_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_MYPICTURES, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path(user_home(), "Pictures", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::my_music_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_MYMUSIC, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path(user_home(), "Music", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::my_video_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_MYVIDEO, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path(user_home(), "Video", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::common_documents_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_COMMON_DOCUMENTS, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path("/Users/Shared", "Documents", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::common_pictures_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_COMMON_PICTURES, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path("/Users/Shared", "Pictures", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::common_music_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_COMMON_MUSIC, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path("/Users/Shared", "Music", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

bool File_path::common_video_path(string& path, bool create_flag)
{
#ifdef PLATFORM_WIN
    return special_path(CSIDL_COMMON_VIDEO, path, create_flag);
#elif defined(PLATFORM_MAC)
    return special_path("/Users/Shared", "Video", path, create_flag);
#elif defined(PLATFORM_UNIX) || defined(PLATFORM_IOS)
    assert(!"path operation not yet implemented");
    return false;
#else
#error unknown platform
#endif
}

#ifdef PLATFORM_WIN
const char File_path::separator = '\\';
#else
const char File_path::separator = '/';
#endif

//
// class File_info
//

bool File_info::read_file_characteristics(File_time& mod_time, large& size)
{
#ifdef PLATFORM_WIN
    assert(!"TODO");
    return false;
#else
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result != 0)
        return false;
    mod_time.actime = st.st_atime;
    mod_time.modtime = st.st_mtime;
    size = st.st_size;
    return true;
#endif
}

bool File_info::read_file_modification_time(File_time& mod_time)
{
#ifdef PLATFORM_WIN
    HANDLE handle = CreateFile(decode_str(filepath), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return false;
#ifdef WINDOWS_XP_COMPATIBLE
    BOOL result = GetFileTime(handle, 0, 0, &mod_time);
#else
    BOOL result = GetFileInformationByHandleEx(handle, FileBasicInfo, &mod_time, sizeof(FILE_BASIC_INFO));
#endif
    CloseHandle(handle);
    return result == TRUE;
#else
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result != 0)
        return false;
    mod_time.actime = st.st_atime;
    mod_time.modtime = st.st_mtime;
    return true;
#endif
}

bool File_info::write_file_modification_time(const File_time& mod_time)
{
#ifdef PLATFORM_WIN
    HANDLE handle = CreateFile(decode_str(filepath), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return false;
#ifdef WINDOWS_XP_COMPATIBLE
    BOOL result = SetFileTime(handle, 0, 0, &mod_time);
#else
    BOOL result = SetFileInformationByHandle(handle, FileBasicInfo, (LPVOID) &mod_time, sizeof(FILE_BASIC_INFO));
#endif
    CloseHandle(handle);
    return result == TRUE;
#else
    return utime(filepath.c_str(), &mod_time) == 0;
#endif
}

large File_info::get_file_size() const
{
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result != 0)
        return -1;
    return st.st_size;
}

int File_info::is_regular_file() const
{
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result != 0)
        return -1;
    return S_ISREG(st.st_mode) ? 0 : 1;
}

int File_info::is_directory() const
{
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result != 0)
        return -1;
    return S_ISDIR(st.st_mode) ? 0 : 1;
}

int File_info::is_link() const
{
    struct stat st;
#ifdef PLATFORM_WIN
    int result = ::stat(filepath.c_str(), &st);
#else
    int result = ::lstat(filepath.c_str(), &st);
#endif
    if (result != 0)
        return -1;
    return S_ISLNK(st.st_mode) ? 0 : 1;
}

//
// class File_system
//

bool File_system::read_volume_info(Volume_info& info)
{
#ifdef PLATFORM_WIN
    TCHAR name_buf[MAX_PATH + 1];
    TCHAR type_buf[MAX_PATH + 1];
    DWORD serno;
    DWORD max_comp_len;
    DWORD flags;
    string silly_c_vol_exception = volume.length() == 2 && toupper(volume[0]) == 'C' ? "C:\\" : volume;
    const TCHAR* s = decode_str(silly_c_vol_exception);
    BOOL success = GetVolumeInformation(s, name_buf, MAX_PATH, &serno, &max_comp_len, &flags, type_buf, MAX_PATH);
    if (!success)
        return false;
    TCHAR path_buf[MAX_PATH + 1];
    DWORD dw = QueryDosDevice(s, path_buf, MAX_PATH);
    if (dw == 0)
        return false;
    string mnt_name;
    string dos_dev = encode_utf8(path_buf);
    size_t sep_pos = dos_dev.find_last_of(";");
    if (sep_pos != string::npos)
        mnt_name = dos_dev.substr(sep_pos + 1);
//  only seems to work with C:\\ 
//  success = GetVolumeNameForVolumeMountPoint(s.c_str(), path_buf, MAX_PATH);
//  if (!success)
//      return false;
    info.logical_name = volume;
    info.physical_name = encode_utf8(name_buf);
    info.fs_type = encode_utf8(type_buf);
    info.mount = mnt_name;
    return true;
#elif defined(PLATFORM_MAC)
    assert(!"TODO");
    return false;
#else
    assert(!"TODO");
    return false;
#endif
}

void File_system::mounted_filesystems(String_vector& volumes)
{
#ifdef PLATFORM_WIN
    DWORD mask = GetLogicalDrives();
    if (mask == 0)
        return;
    for (int i = 0, bit = 1; i < 32; i++, bit <<= 1) {
        if (mask & bit) {
            char drive[3];
            drive[0] = 'A' + i;
            drive[1] = ':';
            drive[2] = 0;
            volumes.append(drive);
        }
    }
#elif defined(PLATFORM_MAC)
    struct statfs* entry;
    int num_mounts = getmntinfo(&entry, MNT_WAIT);
    for (int i = 0; i < num_mounts; i++) {
        string volume = entry[i].f_mntonname;
        volumes.append(volume);
    }
#elif defined(PLATFORM_UNIX)
    struct fstab* entry;
    setfsent();
    while ((entry = getfsent()) != 0) {
        string volume = entry->fs_file;
        volumes.append(volume);
    }
    endfsent();
#elif defined(PLATFORM_IOS)
    assert(!"mounted_volumes not implemented for ios");
#else
#error unknown platform
#endif
}

#ifdef PLATFORM_APPLE
// defined in ObjC .mm file
#else

void File_system::mounted_volumes(String_vector& volumes)
{
    mounted_filesystems(volumes);
}

#endif

string File_system::logical_volume_name(const string& name)
{
#ifdef PLATFORM_WIN
    Volume_info info;
    File_system fs(name);
    bool success = fs.read_volume_info(info);
    if (!success)
        return name;
    const string& mnt = info.get_mount();
    if (mnt.empty())
        return info.get_logical_name().substr(0, 1);
    return File_path::basename_of(mnt);
#else
    return File_path::basename_of(name);
#endif
}

bool File_system::read_serial_number(const string& device, string& serno)
{
#ifdef PLATFORM_WIN
    IWbemLocator* locator = 0;
    HRESULT res = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*) &locator);
    if (FAILED(res))
        return false;
    string lower_case_device = device;
    std::transform(device.begin(), device.end(), lower_case_device.begin(), ::tolower);
    IWbemServices* service = 0;
    res = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), 0, 0, 0, 0, 0, 0, &service);
    if (!FAILED(res)) {
        res = CoSetProxyBlanket(service, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 0, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
        if (!FAILED(res)) {
            IEnumWbemClassObject* enumerator = 0;
//          res = service->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Tag, SerialNumber FROM Win32_PhysicalMedia"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 0, &enumerator);
            res = service->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 0, &enumerator);
            if (!FAILED(res)) {
                IWbemClassObject* obj;
                ULONG result = 0;
                while (enumerator) {
                    res = enumerator->Next(WBEM_INFINITE, 1, &obj, &result);
                    if (!FAILED(res) && result) {
                        VARIANT tag, prop;
                        res = obj->Get(L"Name", 0, &tag, NULL, NULL);
                        if (!FAILED(res)) {
                            string tag_value = encode_utf8(tag.bstrVal);
                            std::transform(tag_value.begin(), tag_value.end(), tag_value.begin(), ::tolower);
                            if (tag_value == lower_case_device) {
                                res = obj->Get(L"SerialNumber", 0, &prop, 0, 0);
                                if (!FAILED(res)) {
                                    serno = encode_utf8(prop.bstrVal);
                                    VariantClear(&prop);
                                    break;
                                }
                            }
                        }
                    }
                    obj->Release();
                }
            }
            enumerator->Release();
        }
        service->Release();
    }
    locator->Release();
    return false;
#else
    // TODO
    serno = File_path::basename_of(device);
    return true;
#endif
}

}}

//
// null stream
//

nullbuf null_obj;
wnullbuf wnull_obj;
ostream cnull(&null_obj);
wostream wcnull(&wnull_obj);
