
//
//  hal_audio.mm
//
//  Created by Christian Lehner on 3/19/18.
//  Copyright © 2019 softhub. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface AudioIO : NSObject

// TODO

@end

@implementation AudioIO

- (void) process {
    OSStatus status;
    void* data = 0;
    UInt32 dataSize = 0;
    NSURL* audioInURL = [NSURL URLWithString : @"file:///Users/chris/Documents/tmp/sax.mp3"];
    status = [self read : audioInURL : &data : &dataSize];
    if (status)
        return;
    NSURL* audioOutURL = [NSURL URLWithString : @"file:///Users/lehner/Documents/tmp/tmp.mp3"];
    status = [self write : audioOutURL : data : dataSize];
    free(data);
}

- (OSStatus) read : (NSURL*) url : (void**) data : (UInt32*) dataSize {
    AudioFileID audioFileId;
    OSStatus status = AudioFileOpenURL((__bridge CFURLRef) url, kAudioFileReadPermission, 0, &audioFileId);
    if (status)
        return status;
    UInt64 fileDataSize = 0;
    UInt32 propertySize = sizeof(fileDataSize);
    status = AudioFileGetProperty(audioFileId, kAudioFilePropertyAudioDataByteCount, &propertySize, &fileDataSize);
    if (status)
        return status;
    *dataSize = (UInt32) fileDataSize;
    *data = malloc(*dataSize);
    if (*data)
        status = AudioFileReadBytes(audioFileId, false, 0, dataSize, *data);
    AudioFileClose(audioFileId);
    return status;
}

- (OSStatus) write : (NSURL*) url : (void*) data : (UInt32) dataSize {
    AudioFileID audioFileId;
    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    asbd.mSampleRate = 44100;
    asbd.mFormatID = kAudioFormatMPEGLayer3;
    asbd.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    asbd.mChannelsPerFrame = 1;
    asbd.mFramesPerPacket = 1;
    asbd.mBitsPerChannel = 16;
    asbd.mBytesPerFrame = 2;
    asbd.mBytesPerPacket = 2;
    OSStatus status = AudioFileCreateWithURL((__bridge CFURLRef) url, kAudioFileMP3Type, &asbd, kAudioFileFlags_EraseFile, &audioFileId);
    if (status)
        return status;
    UInt32 dataWriteSize = dataSize / 2;
    status = AudioFileWriteBytes(audioFileId, false, 0, &dataWriteSize, data);
    AudioFileClose(audioFileId);
    return status;
}

@end
