
#include "stdafx.h"
#include "hal_usb.h"
#include "hal_utils.h"
#include <iostream>
#include <assert.h>
#ifdef PLATFORM_WIN
#include <windows.h>
#include <setupapi.h>
#include <dbt.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "setupapi.lib")
#endif

#if FEATURE_HAL_USB_DISCOVERY

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

USB_discovery* USB_discovery::instance;

void USB_discovery::initialize()
{
    assert(!instance);
    instance = new USB_discovery();
}

void USB_discovery::finalize()
{
    assert(instance);
    delete instance;
    instance = 0;
}

#ifdef PLATFORM_WIN

HWND hwnd;
HDEVNOTIFY notification_status;

static void plug(void* dev);
static void unplug(void* dev);
static void device_change(void* dev);
static void plug_device_interface(DEV_BROADCAST_DEVICEINTERFACE* device);
static void plug_handle(DEV_BROADCAST_HANDLE* device);
static void plug_oem(DEV_BROADCAST_OEM* device);
static void plug_port(DEV_BROADCAST_PORT* device);
static void plug_volume(DEV_BROADCAST_VOLUME* device);
static void unplug_device_interface(DEV_BROADCAST_DEVICEINTERFACE* device);
static void unplug_handle(DEV_BROADCAST_HANDLE* device);
static void unplug_oem(DEV_BROADCAST_OEM* device);
static void unplug_port(DEV_BROADCAST_PORT* device);
static void unplug_volume(DEV_BROADCAST_VOLUME* device);

LRESULT CALLBACK usb_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#else
    // TODO
#endif

#ifdef PLATFORM_WIN
void USB_discovery::start()
{
    register_all_devices();
    assert(hwnd == 0);
    assert(notification_status == 0);
    HINSTANCE inst = GetModuleHandle(NULL);
    WNDCLASSEX wcx; 
    ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = usb_window_proc;
    wcx.hInstance = inst;
    wcx.lpszClassName = L"WindowClassUSB";
    ATOM atom = RegisterClassEx(&wcx);
    assert(atom);
    hwnd = CreateWindow(L"WindowClassUSB", L"USB", WS_CAPTION, 0, 0, 100, 100, 0, 0, 0, 0);
    assert(hwnd);
    DEV_BROADCAST_DEVICEINTERFACE notification_filter;
    ZeroMemory(&notification_filter, sizeof(notification_filter));
    notification_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notification_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notification_status = RegisterDeviceNotification(
        hwnd, &notification_filter, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
    assert(notification_status);
}
#elif defined PLATFORM_MAC
// implemented in ObjC .mm file
#else
// TODO
#endif

#ifdef PLATFORM_WIN
void USB_discovery::stop()
{
    unregister_all_devices();
    UnregisterDeviceNotification(notification_status);
    notification_status = 0;
    DestroyWindow(hwnd);
    hwnd = 0;
}
#elif defined PLATFORM_MAC
// implemented in ObjC .mm file
#else
// TODO
#endif

#ifdef PLATFORM_WIN
static void plug(void* dev)
{
    DEV_BROADCAST_HDR* device = reinterpret_cast<DEV_BROADCAST_HDR*>(dev);
    if (!device)
        return;
    switch (device->dbch_devicetype) {
    case DBT_DEVTYP_DEVICEINTERFACE:
        plug_device_interface(reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(dev));
        break;
    case DBT_DEVTYP_HANDLE:
        plug_handle(reinterpret_cast<DEV_BROADCAST_HANDLE*>(dev));
        break;
    case DBT_DEVTYP_OEM:
        plug_oem(reinterpret_cast<DEV_BROADCAST_OEM*>(dev));
        break;
    case DBT_DEVTYP_PORT:
        plug_port(reinterpret_cast<DEV_BROADCAST_PORT*>(dev));
        break;
    case DBT_DEVTYP_VOLUME:
        plug_volume(reinterpret_cast<DEV_BROADCAST_VOLUME*>(dev));
        break;
    default:
        assert(!"invalid device type");
    }
}

static void unplug(void* dev)
{
    DEV_BROADCAST_HDR* device = reinterpret_cast<DEV_BROADCAST_HDR*>(dev);
    if (!device)
        return;
    switch (device->dbch_devicetype) {
    case DBT_DEVTYP_DEVICEINTERFACE:
        unplug_device_interface(reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(dev));
        break;
    case DBT_DEVTYP_HANDLE:
        unplug_handle(reinterpret_cast<DEV_BROADCAST_HANDLE*>(dev));
        break;
    case DBT_DEVTYP_OEM:
        unplug_oem(reinterpret_cast<DEV_BROADCAST_OEM*>(dev));
        break;
    case DBT_DEVTYP_PORT:
        unplug_port(reinterpret_cast<DEV_BROADCAST_PORT*>(dev));
        break;
    case DBT_DEVTYP_VOLUME:
        unplug_volume(reinterpret_cast<DEV_BROADCAST_VOLUME*>(dev));
        break;
    default:
        assert(!"invalid device type");
    }
}

static void device_change(void* dev)
{
    DEV_BROADCAST_HDR* device = reinterpret_cast<DEV_BROADCAST_HDR*>(dev);
    if (!device)
        return;
    switch (device->dbch_devicetype) {
    case DBT_DEVTYP_DEVICEINTERFACE:
//      plug_device_interface(reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(dev));
        break;
    case DBT_DEVTYP_HANDLE:
//      plug_handle(reinterpret_cast<DEV_BROADCAST_HANDLE*>(dev));
        break;
    case DBT_DEVTYP_OEM:
//      plug_oem(reinterpret_cast<DEV_BROADCAST_OEM*>(dev));
        break;
    case DBT_DEVTYP_PORT:
//      plug_port(reinterpret_cast<DEV_BROADCAST_PORT*>(dev));
        break;
    case DBT_DEVTYP_VOLUME:
//      plug_volume(reinterpret_cast<DEV_BROADCAST_VOLUME*>(dev));
        break;
    default:
        assert(!"invalid device type");
    }
}
#endif

USB_device_ref USB_discovery::find_device(const string& name)
{
    USB_device_map::iterator it = device_map.find(name);
    return it != device_map.end() ? it->second : null;
}

USB_devices_const_ref USB_discovery::find_all_devices()
{
    USB_devices* devices = new USB_devices();
    USB_device_map::iterator it = device_map.begin();
    USB_device_map::iterator tail = device_map.end();
    while (it != tail) {
        USB_device_map::value_type val = *it++;
        USB_device* device = val.second;
        devices->append(device);
    }
    return devices;
}

void USB_discovery::device_names_of(USB_devices* devices, String_vector& names)
{
    USB_devices::iterator it = devices->begin();
    USB_devices::iterator tail = devices->end();
    while (it != tail) {
        USB_device* device = *it++;
        const string& name = device->get_name();
        names.append(name);
    }
}

void USB_discovery::register_device(const string& name, USB_device* device)
{
    USB_device_map::iterator it = device_map.find(name);
    if (it != device_map.end()) {
        device = it->second;
    } else {
        device_map.insert(name, device);
#ifdef _DEBUG
        clog << "register: " << device->get_name() << endl;
#endif
    }
}

void USB_discovery::unregister_device(const string& name)
{
    USB_device_map::iterator it = device_map.find(name);
    if (it != device_map.end()) {
        USB_device* device = it->second;
#ifdef _DEBUG
        clog << "unregister: " << device->get_name() << endl;
#endif
        device_map.erase(it);
    }
}

#ifdef PLATFORM_WIN

void USB_discovery::register_all_devices()
{
    // from http://msdn.microsoft.com/en-us/library/ff540174(VS.85).aspx#filehandle
    GUID guid_device_interface = { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
//    HANDLE device_handle;
    BOOL result = TRUE;
    PSP_DEVICE_INTERFACE_DETAIL_DATA interface_detail_data = NULL;
    ULONG requiredLength = 0;
    LPTSTR device_path = NULL;
//  HDEVINFO device_info = SetupDiGetClassDevs(&guid_device_interface, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    HDEVINFO device_info = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    assert(device_info != INVALID_HANDLE_VALUE);
    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD index = 0; SetupDiEnumDeviceInfo(device_info, index, &device_info_data); index++) {
        //Reset for this iteration
        if (device_path)
            LocalFree(device_path);
        if (interface_detail_data)
            LocalFree(interface_detail_data);

        SP_DEVICE_INTERFACE_DATA device_interface_data;
        device_interface_data.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        // Get information about the device interface.
        result = SetupDiEnumDeviceInterfaces(device_info, &device_info_data, &guid_device_interface, index, &device_interface_data);

        // Check if last item
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
            break;
 
        // Check for some other error
        if (!result) {
            printf("Error SetupDiEnumDeviceInterfaces: %d.\n", GetLastError());
            goto done;
        }

        //Interface data is returned in SP_DEVICE_INTERFACE_DETAIL_DATA
        //which we need to allocate, so we have to call this function twice.
        //First to get the size so that we know how much to allocate
        //Second, the actual call with the allocated buffer
        
        result = SetupDiGetDeviceInterfaceDetail(device_info, &device_interface_data, NULL, 0, &requiredLength, NULL);

        // Check for some other error.
        if (!result) {
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError() && requiredLength > 0) {
                interface_detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);
                if (!interface_detail_data) { 
                    printf("Error allocating memory for the device detail buffer.\n");
                    goto done;
                }
            } else {
                printf("Error SetupDiEnumDeviceInterfaces: %d.\n", GetLastError());
                goto done;
            }
        }
        interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        result = SetupDiGetDeviceInterfaceDetail(device_info, &device_interface_data, interface_detail_data, requiredLength, NULL, &device_info_data);
        if (!result) {
            printf("Error SetupDiGetDeviceInterfaceDetail: %d.\n", GetLastError());
            goto done;
        }

        string device_name = encode_utf8(interface_detail_data->DevicePath);
        USB_device_ref usb_device = USB_discovery::find_device(device_name);
        if (usb_device) {
            usb_device = new USB_device(device_name);
            USB_discovery::instance->register_device(device_name, usb_device);
        }

        //size_t nLength = wcslen(interface_detail_data->DevicePath) + 1;  
        //device_path = (TCHAR*) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
        //StringCchCopy(device_path, nLength, interface_detail_data->DevicePath);
        //device_path[nLength-1] = 0;
                        
        printf("Device path:  %s\n", device_name.c_str());
    }
/*
    if (!device_path) {
        int err = GetLastError();
        printf("Error %d.", err);
        goto done;
    }

    // Open the device
    device_handle = CreateFile(
        device_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (device_handle == INVALID_HANDLE_VALUE) {
        int err = GetLastError();
        printf("Error %d.", err);
        goto done;
    }
*/
done:
    LocalFree(device_path);
    LocalFree(interface_detail_data);    
    result = SetupDiDestroyDeviceInfoList(device_info);

/* this shit from http://msdn.microsoft.com/en-us/library/ff551069(VS.85).aspx
DeviceInfoSet = SetupDiGetClassDevs(
                                    NULL,
                                    NULL,
                                    NULL,
                                    DIGCF_ALLCLASSES | DIGCF_PRESENT);

ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
DeviceIndex = 0;
    
while (SetupDiEnumDeviceInfo(
                             DeviceInfoSet,
                             DeviceIndex,
                             &DeviceInfoData)) {
    DeviceIndex++;

    if (!SetupDiGetDeviceProperty(
                                  DeviceInfoSet,
                                  &DeviceInfoData,
                                  &DEVPKEY_Device_Class,
                                  &PropType,
                                  (PBYTE)&DevGuid,
                                  sizeof(GUID),
                                  &Size,
                                  0) || PropType != DEVPROP_TYPE_GUID) {

        Error = GetLastError();

        if (Error == ERROR_NOT_FOUND) {
            \\
            \\ This device has an unknown device setup class.
            \\
            }
        }                 
    }

if (DeviceInfoSet) {
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
*/
}

#elif defined PLATFORM_APPLE
// defined in ObjC .mm file
#else
// TODO
#endif

void USB_discovery::unregister_all_devices()
{
    device_map.clear();
}

#ifdef PLATFORM_WIN

static void plug_device_interface(DEV_BROADCAST_DEVICEINTERFACE* device)
{
    string name = encode_utf8(device->dbcc_name);
    USB_device_ref usb_device = USB_discovery::find_device(name);
    if (usb_device) {
        usb_device = new USB_device(name);
        USB_discovery::instance->register_device(name, usb_device);
    }
}

static void plug_handle(DEV_BROADCAST_HANDLE* device)
{
}

static void plug_oem(DEV_BROADCAST_OEM* device)
{
}

static void plug_port(DEV_BROADCAST_PORT* device)
{
}

static void plug_volume(DEV_BROADCAST_VOLUME* device)
{
}

static void unplug_device_interface(DEV_BROADCAST_DEVICEINTERFACE* device)
{
    string name = encode_utf8(device->dbcc_name);
    USB_discovery::instance->unregister_device(name);
}

static void unplug_handle(DEV_BROADCAST_HANDLE* device)
{
}

static void unplug_oem(DEV_BROADCAST_OEM* device)
{
}

static void unplug_port(DEV_BROADCAST_PORT* device)
{
}

static void unplug_volume(DEV_BROADCAST_VOLUME* device)
{
}

LRESULT CALLBACK usb_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DEVICECHANGE:
        switch (wParam) {
        case DBT_DEVICEARRIVAL:
            plug((void*) lParam);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            unplug((void*) lParam);
            break;
        case DBT_DEVNODES_CHANGED:
            device_change((void*) lParam);
            break;
        default:
            break;
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#else
    // TODO
#endif
}}

#endif
