
//
//  hal_usb.mm
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//
//  Source: https://tewarid.github.io/2012/04/27/access-usb-device-on-mac-os-x-using-i-o-kit.html
//

#import <Foundation/Foundation.h>
#import <IOKit/usb/IOUSBLib.h>
#import <AppKit/AppKit.h>
#import <IOKit/IOCFPlugIn.h>
#import "hal_usb.h"
#import "hal_utils.h"

#if FEATURE_HAL_USB_DISCOVERY

#define USB_EXPERIMENTAL 0

// Arduino USB info
#define matchVendorID 0x2341
#define matchProductID 0x0043

using namespace SOFTHUB::HAL;
using namespace std;

@interface USB_DiscoveryHelper : NSObject
@end

@implementation USB_DiscoveryHelper

+ (void) startDiscovery {
    NSNotificationCenter* notificationCenter = [[NSWorkspace sharedWorkspace] notificationCenter];
    [notificationCenter addObserver : self selector : @selector(deviceMounted:)  name : NSWorkspaceDidMountNotification object : nil];
    [notificationCenter addObserver : self selector : @selector(deviceUnmounted:)  name : NSWorkspaceDidUnmountNotification object : nil];
}

+ (void) stopDiscovery {
    NSNotificationCenter* notificationCenter = [[NSWorkspace sharedWorkspace] notificationCenter];
    [notificationCenter removeObserver : self name : NSWorkspaceDidMountNotification object : nil];
    [notificationCenter removeObserver : self name : NSWorkspaceDidUnmountNotification object : nil];
}

- (void) deviceMounted : (id) sender {
    NSDictionary* info = [sender userInfo];
    NSString* volume = info[@"NSDevicePath"];
    string name = encode_utf8(volume);
    USB_device* device = new USB_device(name);
    USB_discovery::instance->register_device(name, device);
}

- (void) deviceUnmounted : (id) sender {
    NSDictionary* info = [sender userInfo];
    NSString* volume = info[@"NSDevicePath"];
    string name = encode_utf8(volume);
    USB_discovery::instance->unregister_device(name);
}

#if USB_EXPERIMENTAL

int test_device(io_service_t usb_ref)
{
    IOCFPlugInInterface** plugin = 0;
    SInt32 score = 0;
    IOUSBDeviceInterface300** device = 0;
    IOCreatePlugInInterfaceForService(usb_ref, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
    IOObjectRelease(usb_ref);
    (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300), (void**) &device);
    (*plugin)->Release(plugin);
    IOUSBConfigurationDescriptorPtr config = 0;
    IOReturn ret = (*device)->USBDeviceOpen(device);
    if (ret == kIOReturnSuccess) {
        // set first configuration as active
        ret = (*device)->GetConfigurationDescriptorPtr(device, 0, &config);
        if (ret != kIOReturnSuccess) {
            printf("Could not set active configuration (error: %x)\n", ret);
            return -1;
        }
        (*device)->SetConfiguration(device, config->bConfigurationValue);
    } else if (ret == kIOReturnExclusiveAccess) {
        // this is not a problem as we can still do some things
    } else {
        printf("Could not open device (error: %x)\n", ret);
        return -1;
    }
    IOUSBFindInterfaceRequest ifreq;
    ifreq.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    ifreq.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    ifreq.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    ifreq.bAlternateSetting = kIOUSBFindInterfaceDontCare;
    io_iterator_t iterator = 0;
    (*device)->CreateInterfaceIterator(device, &ifreq, &iterator);
    //IOIteratorNext(iterator); // skip interface #0
    usb_ref = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    IOCreatePlugInInterfaceForService(usb_ref, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
    IOObjectRelease(usb_ref);
    IOUSBInterfaceInterface300** usb_if = 0;
    (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID300), (void**) &usb_if);
    (*plugin)->Release(plugin);
    if (!usb_if)
        return -1;
    ret = (*usb_if)->USBInterfaceOpen(usb_if);
    if (ret != kIOReturnSuccess) {
        printf("Could not open interface (error: %x)\n", ret);
        return -1;
    }
    UInt8 dir_out, number, transfer_type, interval;
    UInt16 max_packet_size;
    ret = (*usb_if)->GetPipeProperties(usb_if, 1, &dir_out, &number, &transfer_type, &max_packet_size, &interval);
    char out_buf[] = { 0x02, 0x00, 0x02, 0x4a, 0x30, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//  char out_buf[] = { 0xA0, 0x00, 0x02, 0x80, 0x01, 0xE0 };
    ret = (*usb_if)->WritePipe(usb_if, 3, out_buf, sizeof(out_buf));
//  char* out_buf = (char*) calloc(64, 1);
//  out_buf[0] = 0xA0;
//  ret = (*usb_if)->WritePipe(usb_if, 1, out_buf, 1);
    UInt32 num_bytes = 640 * 24 / 8;
    int in_buf_size = 640 * 480 * 24 / 8;
    char* in_buf = (char*) malloc(num_bytes);
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, &in_buf, in_buf_size, NULL);//freeBitmapBuffer);
    CGImageRef image = CGImageCreate(640, 480, 8, 24, 640 * 24 / 8, rgb, kCGBitmapByteOrderDefault, provider, 0, FALSE, kCGRenderingIntentDefault);
    int x = 0, y = 0;
    do {
        char* buf = in_buf + x * 24 / 8 + y * 640 * 24 / 8;
        ret = (*usb_if)->ReadPipe(usb_if, 2, buf, &num_bytes);
        x += num_bytes / (24 / 8);
        if (x >= 640) {
            x = 0;
            y++;
        }
        if (y >= 480) {
            CGContextDrawImage(context, CGRectMake(0, 0, 640, 480), image);
            y = 0;
        }
    } while (ret == kIOReturnSuccess);
    CGImageRelease(image);
    CGColorSpaceRelease(rgb);
    CGDataProviderRelease(provider);
    delete in_buf;
    (*usb_if)->USBInterfaceClose(usb_if);
    if (ret == kIOReturnSuccess)
        return -1;
    return 0;
}

void plug(void* ref_con, io_iterator_t iterator)
{
    NSLog(@"Matching USB device appeared");
    io_object_t obj;
    while ((obj = IOIteratorNext(iterator)) != 0) {
        test_device((io_service_t) obj);
        IOObjectRelease(obj);
    }
}

void unplug(void* ref_con, io_iterator_t iterator)
{
    NSLog(@"Matching USB device disappeared");
    io_object_t obj;
    while ((obj = IOIteratorNext(iterator)) != 0) {
        IOObjectRelease(obj);
    }
}

CFMutableDictionaryRef usbMakeMachingDict(ushort vendorId, ushort productId)
{
    CFMutableDictionaryRef dict = IOServiceMatching(kIOUSBDeviceClassName);
    NSMutableDictionary* bridgeDict = (__bridge NSMutableDictionary*) dict;
    if (vendorId)
        [bridgeDict setObject : [NSNumber numberWithShort : vendorId] forKey : (NSString*) CFSTR(kUSBVendorID)];
    if (productId)
        [bridgeDict setObject : [NSNumber numberWithShort : productId] forKey : (NSString*) CFSTR(kUSBProductID)];
    return dict;
}

void setupDeviceNotification(void* refCon, ushort vendorId, ushort productId)
{
    io_iterator_t publishIterator = 0;
    io_iterator_t removeIterator = 0;
    CFMutableDictionaryRef matchingPublishDict = usbMakeMachingDict(vendorId, productId);
    IONotificationPortRef port = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(port);
    CFRunLoopRef runLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
    CFRunLoopAddSource(runLoop, runLoopSource, kCFRunLoopDefaultMode);
    kern_return_t err = IOServiceAddMatchingNotification(port, kIOMatchedNotification, matchingPublishDict, plug, refCon, &publishIterator);
    if (err)
        NSLog(@"error adding publish notification");
    matchingDevicesAdded(publishIterator);
    CFMutableDictionaryRef matchingRemoveDict = usbMakeMachingDict(vendorId, productId);
    err = IOServiceAddMatchingNotification(port, kIOTerminatedNotification, matchingRemoveDict, unplug, refCon, &removeIterator);
    if (err)
        NSLog(@"error adding removed notification");
    matchingDevicesRemoved(removeIterator);
}

void matchingDevicesAdded(io_iterator_t devices)
{
    io_object_t obj;
    while ((obj = IOIteratorNext(devices))) {
        NSLog(@"new Matching device added ");
        IOObjectRelease(obj);
    }
}

void matchingDevicesRemoved(io_iterator_t devices)
{
    io_object_t obj;
    while ((obj = IOIteratorNext(devices))) {
        NSLog(@"A matching device was removed ");
        IOObjectRelease(obj);
    }
}

#endif

@end

namespace SOFTHUB {
namespace HAL {

void USB_discovery::start()
{
	register_all_devices();
    [USB_DiscoveryHelper startDiscovery];
}

void USB_discovery::stop()
{
    [USB_DiscoveryHelper stopDiscovery];
	unregister_all_devices();
}

void USB_discovery::register_all_devices()
{
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    NSArray* volumes = [workspace mountedLocalVolumePaths];
    NSFileManager* fm = [NSFileManager defaultManager];
    for (NSString* path in volumes) {
        NSDictionary* fsAttributes;
        NSString* description, *type, *displayName;
        BOOL removable, writable, unmountable;
        NSNumber* size;
        BOOL res = [workspace getFileSystemInfoForPath : path
            isRemovable : &removable
            isWritable : &writable
            isUnmountable : &unmountable
            description : &description
            type : &type];
        if (res) {
            fsAttributes = [fm fileSystemAttributesAtPath : path];
            displayName = [fm displayNameAtPath : path];
            size = [fsAttributes objectForKey : NSFileSystemSize];
            string name = encode_utf8(displayName);
            USB_device* device = new USB_device(name);
            USB_discovery::register_device(name, device);
        }
    }
#if USB_EXPERIMENTAL
    setupDeviceNotification(this, 0, 0);
#endif
}

}}

#endif
