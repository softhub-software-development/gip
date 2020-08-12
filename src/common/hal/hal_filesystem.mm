
//
//  hal_filesystem.mm
//
//  Created by Christian Lehner on 4/27/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#import "hal_filesystem.h"
#import "hal_utils.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSURL.h>
#ifdef PLATFORM_MAC
#import <AppKit/AppKit.h>
#import <IOKit/IOCFPlugIn.h>
#endif

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace HAL {

#if FEATURE_HAL_FS

void File_system::mounted_volumes(String_vector& volumes)
{
#ifdef PLATFORM_MAC
/*
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSArray* urls = [fileManager mountedVolumeURLsIncludingResourceValuesForKeys : nil options : NSVolumeEnumerationProduceFileReferenceURLs];
    for (NSURL* url in urls) {
        NSString* path = [url path];
        string name = encode_utf8(path);
        volumes.append(name);
    }
*/
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    NSArray* volume_path = [workspace mountedLocalVolumePaths];
    NSFileManager* fm = [NSFileManager defaultManager];
    for (NSString* path in volume_path) {
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
        if (res && ![description isCaseInsensitiveLike : @"autofs"]) {
            fsAttributes = [fm fileSystemAttributesAtPath : path];
            displayName = [fm displayNameAtPath : path];
            size = [fsAttributes objectForKey : NSFileSystemSize];
            string name = encode_utf8(displayName);
            volumes.append(name);
        }
    }
#endif
}

#endif

#ifdef PLATFORM_MAC

bool File_path::app_support_path(string& path, bool create_flag)
{
    NSError* error;
    NSFileManager* manager = [NSFileManager defaultManager];
    NSURL* appSupport = [
        manager URLForDirectory : NSApplicationSupportDirectory
        inDomain : NSUserDomainMask appropriateForURL : nil create : false error : &error];
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* identifier = [bundle bundleIdentifier];
    if (!identifier)
        identifier = @"com.softhub";
    NSURL* folder = [appSupport URLByAppendingPathComponent : identifier];
    if (create_flag)
        [manager createDirectoryAtURL : folder withIntermediateDirectories : true attributes : nil error : &error];
    NSString* url_path = [folder path];
    path = encode_utf8(url_path);
    return [manager fileExistsAtPath : url_path];
}

#endif

}}
